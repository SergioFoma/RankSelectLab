# Реализации динамического множества целых ключей с запросами 🔑

This is C++ Vladimirov task.

![C++](https://img.shields.io/badge/C++-23-blue?logo=cplusplus)
![CMake](https://img.shields.io/badge/CMake-3.28-064F8C?logo=cmake)
![GoogleTest](https://img.shields.io/badge/GoogleTest-passing-4285F4?logo=googletest&logoColor=white)


## Исследование 📄

[Отчёт по исследованию множества ключей с запросами](./research/REPORT.md): 3 значения размера
множества, 3 разных соотношения числа вставок и запросов, 6 распределений.
Датасеты находятся в папке ```research/datasets/```, генераторы в ```research/generators/```

## Библиотеки, необходимые для работы программы 📚

![TBB](https://img.shields.io/badge/TBB-oneAPI-0071C5?logo=logoColor=white)

## Структура проекта 📝

```

├── 📂benchmarks
│   ├── benchmarks.cpp              # измерения времени обработки
│   └── CMakeLists.txt              # сборка бенч марков
├── CMakeLists.txt                  # сборка всего проекта
├── 📂materials                     # входные данные
├── 📂modules                       # C++ модули
│   ├── 📂algorithms                # модули алгоритмов
│   ├── configuring_paths.cppm      # настройка путей к папкам
│   ├── integer_keys_set.cppm       # user interface
│   ├── parser.cppm                 # обработка входных данных
│   └── prog_error.cppm             # собственные исключения
├── README.md
├── 📂research                      # исследование
│   ├── 📂datasets                  # входные данные
│   ├── 📂generators                # генераторы входных данных
│   ├── 📂pictures                  # гистограммы и тепловые карты
│   └── REPORT.md                   # отчет
├── 📂src
│   ├── 📂algorithms                # исходники алгоритмов
│   ├── CMakeLists.txt              # сборка src
│   └── main.cpp                    # main
└── 📂tests
    ├── CMakeLists.txt              # сборка тестов
    ├── end_to_end.cpp              # end to end тесты
    └── unit_tests.cpp              # unit тесты

```

## Скачивание репозитория 👾

### **Чтобы скачать весь проект, нужно:**

1. В Вашем терминале необходимо перейти в директорию, куда Вы хотите скачать данный проект.

2. Затем в терминале введите:

SSH:

```bash
git clone git@github.com:SergioFoma/RankSelectLab.git
```

HTTPS:

```bash
git clone https://github.com/SergioFoma/RankSelectLab.git
```

3. Дождитесь скачивания.

4. После чего будет создана рабочая директория с названием репозитория (RankSelectLab).

## Запуск основой программы 🚀

Для сборки проект необходимо перейти в его корень и выполнить команду:

```bash
cmake -S . -B Build -G Ninja
```

> ❗ Используется система сборки Ninja ввиду использования C++ модулей

Для компиляции проекта выполните команду:

```bash
cmake --build Build
```

**Опции программы**:

Для просмотра возможных команд выполните:

```bash
Build/src/target --help

Rank select lab


Build/src/target [OPTIONS] input_file


POSITIONALS:
  input_file TEXT REQUIRED    The path to commands

OPTIONS:
  -h,     --help              Print this help message and exit
          --std-set           The algorithm is based on a std base
          --sorted-vector     The algorithm is base on sorted vector
          --avl-tree          The algorithm is base on avl - tree
          --b-tree            The algorithm is base on b - tree
  -f,     --input-file TEXT REQUIRED
                              The path to commands
  -k,     --min-degree UINT   The min degree for b - tree
```

Запуск обработки последовательности запросов:

```bash
Build/src/target -f materials/input_10000.txt
```

## Запуск тестов ⚙

Соберите проект с флагом ```-DBUILD_TESTS=ON```:

```bash
cmake -S . -B Build -G Ninja -DBUILD_TESTS=ON
```

Компиляции unit и end-to-end тестирования:

```bash
cmake --build Build
```

Запуск ```google тестов```:

```bash
Build/tests/unit_tests
```

Запуск ```end-to-end тестов```:

```bash
Build/tests/end_to_end_tests
```



