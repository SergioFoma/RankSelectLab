#!/usr/bin/env python3
"""
Столбчатая диаграмма: время обработки B-дерева от его степени t.

Данные по умолчанию -- медианы 10 повторов google benchmark на датасете
research/datasets/data_6_1_3.txt (N = 1 000 000, случайное распределение,
ключи и запросы перемешаны, запросы/вставки = 0.3, t = 50 в основном
эксперименте).

Источник чисел:
  1) --json <файл> -- JSON googlebenchmark (--benchmark_out=...);
     берутся aggregate_name == "median", если они есть, иначе медиана
     сырых прогонов;
  2) значения по умолчанию -- медианы из research/degree_results.json.

Запуск:
  python3 research/generators/plot_btree_degree.py
  python3 research/generators/plot_btree_degree.py \
      --json research/degree_results.json \
      --out research/pictures/btree_degree.png
"""

import argparse
import json
import os
import re
import statistics

import matplotlib
matplotlib.use("Agg")  # сохранение в файл без GUI
import matplotlib.pyplot as plt

TITLE = ("B-дерево: зависимость времени от степени t\n"
         "N = 1 000 000, случайная (перемешано), запросы/вставки = 0.3, "
         "медиана 10 повторов")

DEFAULT_DEGREES = [5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
DEFAULT_TIMES_MS = [290.4, 192.2, 161.8, 149.4, 163.2, 159.8, 161.1, 163.8, 168.8, 168.2, 174.1]

DEFAULT_OUT = os.path.join("research", "pictures", "btree_degree.png")


def _to_ms(value, unit):
    if unit == "us":
        return value / 1000.0
    if unit == "ns":
        return value / 1_000_000.0
    if unit == "s":
        return value * 1000.0
    return value


def load_from_json(path):
    """Возвращает (degrees, times_ms), отсортированные по степени t."""
    with open(path, encoding="utf-8") as f:
        data = json.load(f)

    medians = {}
    raw = {}
    for bm in data["benchmarks"]:
        name = bm.get("run_name") or bm["name"]
        m = re.fullmatch(r"Degree\s+(\d+)", name)
        if not m:
            continue
        degree = int(m.group(1))
        t = _to_ms(bm["real_time"], bm.get("time_unit", "ms"))

        if bm.get("run_type") == "aggregate":
            if bm.get("aggregate_name") == "median":
                medians[degree] = t
        else:
            raw.setdefault(degree, []).append(t)

    if medians:
        degrees = sorted(medians)
        times = [medians[d] for d in degrees]
    else:
        degrees = sorted(raw)
        times = [statistics.median(raw[d]) for d in degrees]
    return degrees, times


def parse_args():
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--json", type=str, default=None,
                   help="JSON googlebenchmark вместо значений по умолчанию")
    p.add_argument("--out", type=str, default=DEFAULT_OUT,
                   help="путь к сохраняемому PNG")
    return p.parse_args()


def main():
    args = parse_args()

    if args.json:
        degrees, times = load_from_json(args.json)
    else:
        degrees, times = DEFAULT_DEGREES, DEFAULT_TIMES_MS

    fig, ax = plt.subplots(figsize=(8, 5))
    positions = list(range(len(degrees)))
    bars = ax.bar(positions, times, color="#4C72B0",
                  edgecolor="black", width=0.7)

    # подписи значений над столбцами
    for bar, t in zip(bars, times):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                f"{t:.0f}", ha="center", va="bottom", fontsize=9)

    # отметим лучшую степень
    best = times.index(min(times))
    bars[best].set_color("#55A868")

    ax.set_xticks(positions)
    ax.set_xticklabels([str(d) for d in degrees])
    ax.set_xlabel("Степень B-дерева t")
    ax.set_ylabel("Время, ms")
    ax.set_title(TITLE, fontsize=11)
    ax.set_ylim(0, max(times) * 1.15)
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    out_dir = os.path.dirname(args.out)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    fig.tight_layout()
    fig.savefig(args.out, dpi=200)
    print(f"bar chart saved to {args.out}")
    for d, t in zip(degrees, times):
        print(f"  t={d:<4} {t:.1f} ms")


if __name__ == "__main__":
    main()
