#!/usr/bin/env python3
"""
Генератор датасета research/datasets/data_1_1.txt
для Измерения №1.1 из research/REPORT.md.

Параметры (зафиксированы в статье):
  * распределение  : возрастающая последовательность;
  * число запросов / число вставок = 0.3;
  * N = 100 (уникальных ключей);
  * ключи из диапазона [-0.5N, 0.5N] = [-50, 50] (методика отчёта);
  * дубликаты: round(0.35 * N) = 35 повторных вставок существующих
    ключей, размещённых в случайные позиции потока;
  * структура файла: сначала блок вставок (prefill), затем блок
    запросов (методика: "сначала заполнялись ... затем выполняли запросы");
  * mix типов запросов q : m : n = 1 : 1 : 1 (остаток -- в 'q');
  * ширина диапазона в `q l r` -- log-uniform от 1 до N ключей.

Формат команд (как в materials/input_*.txt):
  k x    -- вставить ключ x
  q l r  -- число ключей в (l, r]
  m i    -- i-й наименьший ключ (1-based)
  n x    -- число ключей, строго меньших x

Генерация детерминирована (seed = 20261021): один и тот же файл
используется для всех четырёх структур данных (требование отчёта).

Запуск:  python3 research/generators/generate_dataset_1_1.py
"""

import math
import os
import random

# ==== параметры Измерения №1.1 (из REPORT.md) ====
DIST = "ascending"     # возрастающая последовательность
RATIO = 0.3            # число запросов / число вставок
N = 100                # размер множества (уникальных ключей)
DUP_RATE = 0.35        # доля повторов: round(0.35 * N) = 35
SEED = 20261021
OUT = os.path.join("research", "datasets", "data_1_1.txt")


def key_bounds(n):
    """Диапазон выборки ключей [-0.5N, 0.5N] (методика отчёта)."""
    return -(n // 2), n // 2


def make_unique_keys(dist, n, rng):
    """N уникальных ключей в порядке заданного распределения."""
    lo, hi = key_bounds(n)
    if dist == "ascending":
        return list(range(lo, lo + n))
    if dist == "descending":
        return list(range(lo, lo + n))[::-1]
    return rng.sample(range(lo, hi + 1), n)


def build_inserts(dist, n, dup_rate, rng):
    """Блок вставок: N уникальных ключей + round(dup_rate*N) повторов."""
    keys = make_unique_keys(dist, n, rng)
    n_dups = round(n * dup_rate)
    dups = [rng.choice(keys) for _ in range(n_dups)]

    inserts = list(keys)
    for d in dups:
        pos = rng.randint(0, len(inserts))
        inserts.insert(pos, d)

    return [("k", x) for x in inserts], n_dups


def build_queries(n, n_queries, rng):
    """Блок запросов: q/m/n равными третями (остаток в 'q'), перемешаны."""
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


def main():
    rng = random.Random(SEED)

    inserts, n_dups = build_inserts(DIST, N, DUP_RATE, rng)
    n_inserts = len(inserts)
    n_queries = round(RATIO * n_inserts)
    queries = build_queries(N, n_queries, rng)

    out_dir = os.path.dirname(OUT)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        for _, x in inserts:
            f.write(f"k {x}\n")
        for q in queries:
            if q[0] == "q":
                f.write(f"q {q[1]} {q[2]}\n")
            else:
                f.write(f"{q[0]} {q[1]}\n")

    lo, hi = key_bounds(N)
    print(f"seed            = {SEED}")
    print(f"distribution    = {DIST}")
    print(f"key range       = [{lo}, {hi}]")
    print(f"N (unique keys) = {N}")
    print(f"duplicates      = {n_dups}  ({DUP_RATE} * N)")
    print(f"inserts total   = {n_inserts}")
    print(f"queries total   = {n_queries}  (ratio = {RATIO})")
    print(f"lines total     = {n_inserts + n_queries}")
    print(f"output          = {OUT}")


if __name__ == "__main__":
    main()
