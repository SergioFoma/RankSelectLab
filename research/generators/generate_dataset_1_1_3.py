#!/usr/bin/env python3
"""
Генератор датасета research/datasets/data_1_1_3.txt
для Измерения №1.1.3 из research/REPORT.md.

Схема имён: data_{распределение}_{коэффициент}_{N}
  * распределение 1 -- по возрастанию, сначала все ключи, потом запросы;
  * коэффициент   1 -- запросы / вставки = 0.3;
  * N             3 -- 1 000 000 уникальных ключей.

Параметры опыта:
  * ключи из диапазона [-0.5N, 0.5N] = [-500000, 500000];
  * дубликаты: round(0.35 * N) = 350000 повторов существующих ключей,
    каждый повтор стоит сразу за своим ключом;
  * структура файла: сначала блок вставок (prefill), затем блок запросов;
  * mix типов запросов q : m : n = 1 : 1 : 1 (остаток -- в 'q');
  * ширина диапазона в `q l r` -- log-uniform от 1 до N ключей;
  * индекс в `m i` берётся из числа уже вставленных уникальных ключей
    (для перемешанных вариантов это важно: запрос может стоять до
    части вставок).

Формат команд (как в materials/input_*.txt):
  k x    -- вставить ключ x
  q l r  -- число ключей в (l, r]
  m i    -- i-й наименьший ключ (1-based)
  n x    -- число ключей, строго меньших x

Генерация детерминирована (seed = 20261021): один и тот же файл
используется для всех структур данных.

Запуск:  python3 research/generators/generate_dataset_1_1_3.py
"""

import math
import os
import random
from collections import Counter

# ==== параметры Измерения №1.1.3 ====
DIST = "ascending"
INTERLEAVED = False
RATIO = 0.3            # число запросов / число вставок
N = 1000000                # размер множества (уникальных ключей)
DUP_RATE = 0.35          # доля повторов: round(0.35 * N) = 350000
SEED = 20261021
OUT = os.path.join("research", "datasets", "data_1_1_3.txt")


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
    """Блок вставок: N уникальных ключей + round(dup_rate*N) повторов.

    Каждый повтор стоит сразу за своим ключом, поэтому уникальные ключи
    идут строго по порядку распределения.
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
    """Один запрос нужного типа; индекс m -- в пределах текущего размера."""
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


def build_interleaved(inserts, n_queries, n, rng):
    """Вставки и запросы равномерно перемешаны по файлу.

    Запрос `m i` должен быть валиден в момент выполнения, поэтому i
    выбирается из числа уже вставленных уникальных ключей.
    """
    base = n_queries // 3
    n_q = base + (n_queries - 3 * base)
    types = ["q"] * n_q + ["m"] * base + ["n"] * base
    rng.shuffle(types)

    order = [0] * len(inserts) + [1] * n_queries
    rng.shuffle(order)
    if order[0] == 1:  # первая строка -- вставка
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


def main():
    rng = random.Random(SEED)

    inserts, n_dups = build_inserts(DIST, N, DUP_RATE, rng)
    n_inserts = len(inserts)
    n_queries = round(RATIO * n_inserts)

    if INTERLEAVED:
        lines = format_lines(build_interleaved(inserts, n_queries, N, rng))
    else:
        queries = build_queries(N, n_queries, rng)
        ins = [f"k {x}\n" for _, x in inserts]
        qs = [f"q {q[1]} {q[2]}\n" if q[0] == "q" else f"{q[0]} {q[1]}\n"
              for q in queries]
        lines = ins + qs

    out_dir = os.path.dirname(OUT)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        f.writelines(lines)

    total = len(lines)
    print(f"data_1_1_3.txt: inserts={n_inserts}, queries={n_queries}, "
          f"total={total}, query_share={n_queries / total * 100:.1f}%")


if __name__ == "__main__":
    main()
