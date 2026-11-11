# Реализации динамического множества целых ключей с запросами 🔑

This is C++ Konstantin Vladimirov task.

![C++](https://img.shields.io/badge/C++-23-blue?logo=cplusplus)
![CMake](https://img.shields.io/badge/CMake-3.28-064F8C?logo=cmake)
![GoogleTest](https://img.shields.io/badge/GoogleTest-passing-4285F4?logo=googletest&logoColor=white)


## Исследование 📄

[Отчёт по исследованию множества ключей с запросами](./research/REPORT.md): 3 значения размера
множества, 3 разных соотношения числа вставок и запросов, 6 распределений.
Датасеты находятся в папке ```research/datasets/```, генераторы в ```research/generators/```

## Библиотеки, необходимые для работы программы 📚

![TBB](https://img.shields.io/badge/TBB-oneAPI-0071C5?logo=logoColor=white)

## Опции main ⚙

Для просмотра возможных команд вызовите help:

```bash
Build/src/target --help
```

## Опции тестов ⚙

Для просмотра возможных команд вызовите help:

```bash
Build/tests/end_to_end_tests --help
```

## Опции бенчмарков ⚙

Для просмотра возможных команд вызовите help:

```bash
Build/benchmarks/benchmark_analysis --help
```








