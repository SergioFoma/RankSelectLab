#!/usr/bin/env python3
"""
Гистограмма результатов Измерения №1.1 (случай N = 10000).

Оси:
  Ox -- структура данных (StdSet, SortedVector, AvlTree, BTree);
  Oy -- время обработки потока команд, ms;
Заголовок -- параметры опыта из REPORT.md.

Источник чисел (по приоритету):
  1) --json <файл> -- JSON googlebenchmark (--benchmark_out=results.json);
     имена бенчмарков ожидаются вида Set12 / Vector12 / AvlTree12 / BTree12
     (или любые -- берутся как есть), поле real_time в ms;
  2) значения по умолчанию -- прогон от 21.10.2026 (N=10000,
     data_1_2.txt, t=50): Set 39.1, Vector 1.64, Avl 2.43, BTree 3.66.

Запуск:
  python3 research/generators/plot_histogram_1_2.py
  python3 research/generators/plot_histogram_1_2.py --json results_1_2.json \
      --out research/pictures/histogram_1_2.png
"""

import argparse
import json
import os

import matplotlib
matplotlib.use("Agg")  # сохранение в файл без GUI
import matplotlib.pyplot as plt

# ==== параметры опыта (заголовок гистограммы) ====
TITLE = ("Измерение №1.1: N = 10000, t = 50\n"
         "возрастающая последовательность, запросы/вставки = 0.3")

# ==== результаты прогона по умолчанию (Time, ms) ====
DEFAULT_LABELS = ["StdSet", "SortedVector", "AvlTree", "BTree"]
DEFAULT_TIMES_MS = [39.1, 1.64, 2.43, 3.66]

# соответствие имён бенчмарков из JSON понятным подписям
JSON_NAME_MAP = {
    "Set12": "StdSet",
    "Vector12": "SortedVector",
    "AvlTree12": "AvlTree",
    "BTree12": "BTree",
}

DEFAULT_OUT = os.path.join("research", "pictures", "histogram_1_2.png")


def load_from_json(path):
    """Читает JSON googlebenchmark: [(label, real_time_ms), ...]."""
    with open(path, encoding="utf-8") as f:
        data = json.load(f)

    labels, times = [], []
    for bm in data["benchmarks"]:
        name = bm["name"]
        labels.append(JSON_NAME_MAP.get(name, name))
        t = bm["real_time"]
        unit = bm.get("time_unit", "ms")
        if unit == "us":
            t /= 1000.0
        elif unit == "ns":
            t /= 1_000_000.0
        times.append(t)
    return labels, times


def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--json", type=str, default=None,
                   help="JSON googlebenchmark (--benchmark_out) вместо значений по умолчанию")
    p.add_argument("--out", type=str, default=DEFAULT_OUT,
                   help="путь к сохраняемому PNG")
    return p.parse_args()


def main():
    args = parse_args()

    if args.json:
        labels, times = load_from_json(args.json)
    else:
        labels, times = DEFAULT_LABELS, DEFAULT_TIMES_MS

    fig, ax = plt.subplots(figsize=(6, 5))
    bars = ax.bar(labels, times, color="#4C72B0", edgecolor="black", width=0.6)

    # подписи значений над столбцами
    for bar, t in zip(bars, times):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                f"{t:.2f}", ha="center", va="bottom", fontsize=10)

    # отметим победителя
    best = times.index(min(times))
    bars[best].set_color("#55A868")

    ax.set_xlabel("Структура данных")
    ax.set_ylabel("Время, ms")
    ax.set_title(TITLE, fontsize=12)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.set_ylim(0, max(times) * 1.15)

    out_dir = os.path.dirname(args.out)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    fig.tight_layout()
    fig.savefig(args.out, dpi=200)
    print(f"histogram saved to {args.out}")
    for label, t in zip(labels, times):
        print(f"  {label:<14} {t:.2f} ms")


if __name__ == "__main__":
    main()
