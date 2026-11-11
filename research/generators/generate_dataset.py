#!/usr/bin/env python3
"""
Unified dataset + histogram generator for research/REPORT.md
(replaces 54 generate_dataset_*.py + 54 plot_histogram_*.py scripts).

File naming: data_{distribution}_{ratio}_{N}

Arguments (order as specified in the task):
  ratio 1-3  -- queries / inserts ratio:
                 1 -> 3/7 (fewer queries), 2 -> 1 (equal counts),
                 3 -> 7/3 (more queries);
  num   1-3  -- set size N: 1 -> 100, 2 -> 10000, 3 -> 1000000;
  dist  1-6  -- key distribution and ordering:
                 1 ascending (keys -> queries),
                 2 ascending (interleaved),
                 3 descending (keys -> queries),
                 4 descending (interleaved),
                 5 random (keys -> queries),
                 6 random (interleaved).

Modes:
  python3 generate_dataset.py gen <ratio> <num> <dist> [mode]
      -- one dataset; mode: all (default) | dataset | plot
  python3 generate_dataset.py genall [mode] [--only-numbers 1,2]
      -- all combinations; --only-numbers 1,2 limits the N indices
         (to regenerate data_x_y_1/2 cases without the million-size _3)

Histograms read their numbers from googlebenchmark JSON
(--benchmark_out=results.json); path via the --json flag.
In plot mode without --json the script exits with an error.

Deterministic: seed 20261021, one dataset shared by all structures.
"""

import argparse
import json
import math
import os
import random
import sys
from collections import Counter

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir,
                                    os.pardir))
DATASETS_DIR = os.path.join(ROOT, "research", "datasets")
PICTURES_DIR = os.path.join(ROOT, "research", "pictures")

SEED = 20261021
DUP_RATE = 0.35
DEGREE = 50

RATIOS = {1: 3.0 / 7.0, 2: 1.0, 3: 7.0 / 3.0}
RATIO_LABELS = {1: "3/7", 2: "1", 3: "7/3"}

NUMBERS = {1: 100, 2: 10_000, 3: 1_000_000}

# dist -> (DIST, INTERLEAVED, title phrase)
DISTS = {
    1: ("ascending", False, "возрастающая (ключи → запросы)"),
    2: ("ascending", True, "возрастающая (перемешано)"),
    3: ("descending", False, "убывающая (ключи → запросы)"),
    4: ("descending", True, "убывающая (перемешано)"),
    5: ("random", False, "случайная (ключи → запросы)"),
    6: ("random", True, "случайная (перемешано)"),
}


# ============================ DATA GENERATION ============================

def key_bounds(n):
    """Key sampling range [-0.5N, 0.5N] (report methodology)."""
    return -(n // 2), n // 2


def make_unique_keys(dist, n, rng):
    """N unique keys in the order of the given distribution."""
    lo, hi = key_bounds(n)
    if dist == "ascending":
        return list(range(lo, lo + n))
    if dist == "descending":
        return list(range(lo, lo + n))[::-1]
    return rng.sample(range(lo, hi + 1), n)


def build_inserts(dist, n, dup_rate, rng):
    """Insert block: N unique keys + round(dup_rate*N) duplicates.

    Each duplicate sits right after its key, so unique keys
    keep the exact distribution order.
    """
    keys = make_unique_keys(dist, n, rng)
    n_dups = round(n * dup_rate)

    counts = Counter(rng.choice(keys) for _ in range(n_dups))

    inserts = []
    for key in keys:
        inserts.append(key)
        inserts.extend([key] * counts.get(key, 0))

    return [("k", x) for x in inserts], n_dups


def _make_query(types, n, lo, hit, spacing, log_n, inserted, rng):
    """One query of the requested type; m index -- within the current size."""
    t = types.pop()
    if t == "m":
        return ("m", rng.randint(1, len(inserted)))
    if t == "q":
        left = rng.randint(lo, hit - 1)
        width_keys = 10 ** rng.uniform(0, log_n)
        right = min(hit, left + max(1, int(width_keys * spacing)))
        return ("q", left, right)
    return ("n", rng.randint(lo, hit))


def build_queries(n, n_queries, rng):
    """Query block: q/m/n in equal thirds (remainder into 'q'), shuffled."""
    base = n_queries // 3
    n_q = base + (n_queries - 3 * base)

    lo, hi = key_bounds(n)
    spacing = (hi - lo) / n
    log_n = math.log10(n)

    queries = []
    for _ in range(n_q):
        left = rng.randint(lo, hi - 1)
        width_keys = 10 ** rng.uniform(0, log_n)
        right = min(hi, left + max(1, int(width_keys * spacing)))
        queries.append(("q", left, right))
    for _ in range(base):
        queries.append(("m", rng.randint(1, n)))
    for _ in range(base):
        queries.append(("n", rng.randint(lo, hi)))

    rng.shuffle(queries)
    return queries


def build_interleaved(inserts, n_queries, n, rng):
    """Inserts and queries uniformly interleaved across the file.

    An `m i` query must be valid at run time, so i
    is drawn from the unique keys inserted so far.
    """
    base = n_queries // 3
    n_q = base + (n_queries - 3 * base)
    types = ["q"] * n_q + ["m"] * base + ["n"] * base
    rng.shuffle(types)

    order = [0] * len(inserts) + [1] * n_queries
    rng.shuffle(order)
    if order[0] == 1:  # the first line must be an insert
        j = order.index(0)
        order[0], order[j] = order[j], order[0]

    lo, hi = key_bounds(n)
    spacing = (hi - lo) / n
    log_n = math.log10(n)

    seq = []
    inserted = set()
    ins_it = iter(inserts)
    for kind in order:
        if kind == 0:
            key = next(ins_it)[1]
            seq.append(("k", key))
            inserted.add(key)
        else:
            seq.append(_make_query(types, n, lo, hi, spacing, log_n,
                                   inserted, rng))
    return seq


def format_lines(seq):
    out = []
    for item in seq:
        if item[0] == "q":
            out.append(f"q {item[1]} {item[2]}\n")
        else:
            out.append(f"{item[0]} {item[1]}\n")
    return out


def dataset_path(d, r, n):
    return os.path.join(DATASETS_DIR, f"data_{d}_{r}_{n}.txt")


def generate_dataset(d, r, n):
    """Writes research/datasets/data_{d}_{r}_{n}.txt; returns a summary."""
    dist, interleaved, _ = DISTS[d]
    ratio = RATIOS[r]
    size = NUMBERS[n]

    rng = random.Random(SEED)

    inserts, n_dups = build_inserts(dist, size, DUP_RATE, rng)
    n_inserts = len(inserts)
    n_queries = round(ratio * n_inserts)

    if interleaved:
        lines = format_lines(build_interleaved(inserts, n_queries, size, rng))
    else:
        queries = build_queries(size, n_queries, rng)
        ins = [f"k {x}\n" for _, x in inserts]
        qs = [f"q {q[1]} {q[2]}\n" if q[0] == "q" else f"{q[0]} {q[1]}\n"
              for q in queries]
        lines = ins + qs

    os.makedirs(DATASETS_DIR, exist_ok=True)
    out = dataset_path(d, r, n)
    with open(out, "w", encoding="utf-8") as f:
        f.writelines(lines)

    total = len(lines)
    print(f"data_{d}_{r}_{n}.txt: inserts={n_inserts}, queries={n_queries}, "
          f"total={total}, query_share={n_queries / total * 100:.1f}%")
    return out


# ============================== HISTOGRAMS ===============================

ORDER = ["StdSet", "SortedVector", "AvlTree", "BTree"]
NAME_MAP = {"Set": "StdSet", "Vector": "SortedVector",
            "AvlTree": "AvlTree", "BTree": "BTree"}


def load_benchmarks_json(path):
    """{experiment_key: {label: time_ms}} from googlebenchmark JSON."""
    with open(path, encoding="utf-8") as f:
        data = json.load(f)

    result = {}
    for bm in data["benchmarks"]:
        name = bm["name"]
        for prefix, label in NAME_MAP.items():
            if name.startswith(prefix):
                key = name[len(prefix):]  # "111" -> (d, r, n)
                t = bm["real_time"]
                unit = bm.get("time_unit", "ms")
                if unit == "us":
                    t /= 1000.0
                elif unit == "ns":
                    t /= 1_000_000.0
                result.setdefault(key, {})[label] = t
    return result


def plot_histogram(d, r, n, times_ms, out_path=None):
    """Style ported from plot_histogram_*.py: bars, winner, log scale."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    size = NUMBERS[n]
    dist, _, phrase = DISTS[d]
    title = (f"Измерение №{d}.{r}.{n}: N = {size}, t = {DEGREE}\n"
             f"{phrase}, запросы/вставки = {RATIO_LABELS[r]}")

    times = [times_ms[label] for label in ORDER]

    fig, ax = plt.subplots(figsize=(6, 5))
    bars = ax.bar(ORDER, times, color="#4C72B0", edgecolor="black", width=0.6)

    for bar, t in zip(bars, times):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                f"{t:.4g}", ha="center", va="bottom", fontsize=10)

    best = times.index(min(times))
    bars[best].set_color("#55A868")

    ax.set_xlabel("Структура данных")
    ax.set_title(title, fontsize=11)

    use_log = max(times) / min(times) >= 10
    if use_log:
        ax.set_yscale("log")
        ax.set_ylim(min(times) * 0.6, max(times) * 3.0)
        ax.grid(axis="y", which="both", linestyle="--", alpha=0.4)
        ax.set_ylabel("Время, ms (лог. шкала)")
    else:
        ax.set_ylabel("Время, ms")
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.set_ylim(0, max(times) * 1.15)

    out = out_path or os.path.join(PICTURES_DIR, f"histogram_{d}_{r}_{n}.png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    fig.tight_layout()
    fig.savefig(out)
    plt.close(fig)
    print(f"histogram_{d}_{r}_{n}.png <- {title.splitlines()[-1]}")


def plot_from_json(d, r, n, json_path):
    bench = load_benchmarks_json(json_path)
    key = f"{d}{r}{n}"
    if key not in bench:
        print(f"!! experiment {key} not found in {json_path}",
              file=sys.stderr)
        return None
    plot_histogram(d, r, n, bench[key])
    return bench[key]


# ================================= CLI ===================================

def check_bounds(ratio, num, dist):
    if not (1 <= ratio <= 3 and 1 <= num <= 3 and 1 <= dist <= 6):
        raise SystemExit(f"bad args ratio={ratio} num={num} dist={dist}; "
                         "expected ratio,num in 1..3 and dist in 1..6")


def cmd_gen(args):
    ratio, num, dist = args.ratio, args.num, args.dist
    check_bounds(ratio, num, dist)
    if args.mode in ("all", "dataset"):
        generate_dataset(dist, ratio, num)
    if args.mode in ("all", "plot"):
        if not args.json:
            raise SystemExit("plot mode requires --json <googlebenchmark "
                             "results.json>")
        plot_from_json(dist, ratio, num, args.json)


def cmd_genall(args):
    nums = ([int(x) for x in args.only_numbers.split(",")]
            if args.only_numbers else [1, 2, 3])
    for dist in range(1, 7):
        for ratio in range(1, 4):
            for num in nums:
                if args.mode in ("all", "dataset"):
                    generate_dataset(dist, ratio, num)
                if args.mode in ("all", "plot"):
                    if not args.json:
                        raise SystemExit("plot mode requires --json")
                    plot_from_json(dist, ratio, num, args.json)


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--json", type=str, default=None,
                        help="JSON googlebenchmark для гистограмм")
    common.add_argument("--mode", choices=["all", "dataset", "plot"],
                        default="all")

    g = sub.add_parser("gen", parents=[common],
                       help="один кейс: gen <ratio> <num> <dist>")
    g.add_argument("ratio", type=int)
    g.add_argument("num", type=int)
    g.add_argument("dist", type=int)
    g.set_defaults(func=cmd_gen)

    ga = sub.add_parser("genall", parents=[common],
                        help="все комбинации")
    ga.add_argument("--only-numbers", type=str, default=None,
                    help="список N-индексов через запятую, напр. 1,2")
    ga.set_defaults(func=cmd_genall)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
