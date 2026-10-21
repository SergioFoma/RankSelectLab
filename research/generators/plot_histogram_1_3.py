#!/usr/bin/env python3
"""
Гистограмма результатов Измерения №1.3 (случай N = 1000000).

Оси:
  Ox -- структура данных (StdSet, SortedVector, AvlTree, BTree);
  Oy -- время обработки потока команд, ms (линейная или log-шкала, --log);
Заголовок -- параметры опыта из REPORT.md.

Источник чисел (по приоритету):
  1) --json <файл> -- JSON googlebenchmark (--benchmark_out=results.json);
     имена бенчмарков ожидаются вида Set13 / Vector13 / AvlTree13 / BTree13
     (или любые -- берутся как есть), поле real_time в ms;
  2) значения по умолчанию -- прогон от 21.10.2026 (N=1000000,
     data_1_3.txt, t=50): Set 492273, Vector 207, Avl 467, BTree 526.

ВНИМАНИЕ: разброс результатов ~2400x, на линейной шкале SortedVector/
AvlTree/BTree практически не видны -- для статьи используйте --log.

Запуск:
  python3 research/generators/plot_histogram_1_3.py --log
  python3 research/generators/plot_histogram_1_3.py --json results_1_3.json \
      --out research/pictures/histogram_1_3.png
"""

import argparse
import json
import os

import matplotlib
matplotlib.use("Agg")  # сохранение в файл без GUI
import matplotlib.pyplot as plt

# ==== параметры опыта (заголовок гистограммы) ====
TITLE = ("Измерение №1.3: N = 1000000, t = 50\n"
         "возрастающая последовательность,\n"
         "запросы/вставки = 0.3")

# ==== результаты прогона по умолчанию (Time, ms) ====
DEFAULT_LABELS = ["StdSet", "SortedVector", "AvlTree", "BTree"]
DEFAULT_TIMES_MS = [492273.0, 207.0, 467.0, 526.0]

# соответствие имён бенчмарков из JSON понятным подписям
JSON_NAME_MAP = {
    "Set13": "StdSet",
    "Vector13": "SortedVector",
    "AvlTree13": "AvlTree",
    "BTree13": "BTree",
}

DEFAULT_OUT = os.path.join("research", "pictures", "histogram_1_3.png")


def load_from_json(path):
    """Читает JSON googlebenchmark: ([labels], [real_time_ms])."""
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


def fmt_ms(t):
    """Подпись значения: крупные числа -- без дробной части."""
    return f"{t:.2f}" if t < 100 else f"{t:,.0f}"


def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--json", type=str, default=None,
                   help="JSON googlebenchmark (--benchmark_out) вместо значений по умолчанию")
    p.add_argument("--log", action="store_true",
                   help="логарифмическая шкала Oy (рекомендуется: разброс ~2400x)")
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
                fmt_ms(t), ha="center", va="bottom", fontsize=10)

    # отметим победителя
    best = times.index(min(times))
    bars[best].set_color("#55A868")

    ax.set_xlabel("Структура данных")
    ax.set_ylabel("Время, ms" + (" (log)" if args.log else ""))
    ax.set_title(TITLE, fontsize=11)
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    if args.log:
        ax.set_yscale("log")
        ax.set_ylim(min(times) / 2, max(times) * 4)
    else:
        ax.set_ylim(0, max(times) * 1.15)

    out_dir = os.path.dirname(args.out)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    fig.tight_layout()
    fig.savefig(args.out, dpi=200)
    print(f"histogram saved to {args.out} (log={args.log})")
    for label, t in zip(labels, times):
        print(f"  {label:<14} {fmt_ms(t)} ms")


if __name__ == "__main__":
    main()
