#include <benchmark/benchmark.h>
#include <string>
#include <iostream>

import integer_keys_set;
import parser;
import configuring_paths;


namespace {

constexpr size_t kDefaultDegree = 50;
constexpr size_t kDegree5   = 5;
constexpr size_t kDegree10  = 10;
constexpr size_t kDegree20  = 20;
constexpr size_t kDegree30  = 30;
constexpr size_t kDegree40  = 40;
constexpr size_t kDegree50  = 50;
constexpr size_t kDegree60  = 60;
constexpr size_t kDegree70  = 70;
constexpr size_t kDegree80  = 80;
constexpr size_t kDegree90  = 90;
constexpr size_t kDegree100 = 100;

void StdSet(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> std_set{};
    std::vector<int64_t> result = parser::ParseData(data, std_set);

    benchmark::DoNotOptimize(result);
  }
}

void SortedVector(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl> sorted_vector{};
    std::vector<int64_t> result = parser::ParseData(data, sorted_vector);

    benchmark::DoNotOptimize(result);
  }
}

void AvlTree(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree{};
    std::vector<int64_t> result = parser::ParseData(data, avl_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDefaultDegree};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree5(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree5};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree10(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree10};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree20(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree20};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree30(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree30};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree40(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree40};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree50(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree50};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree60(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree60};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree70(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree70};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree80(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree80};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}


void BTree90(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree90};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree100(benchmark::State& state, const std::string& data) {
  for (auto st: state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree100};
    std::vector<int64_t> result = parser::ParseData(data, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

// ===================== Group of experiments ================================

void Experiment(const char* set_name, const char* vector_name,
                const char* avl_name, const char* b_name,
                const std::string& data) {

  benchmark::RegisterBenchmark(set_name, [data](benchmark::State& state) {
    StdSet(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark(vector_name, [data](benchmark::State& state) {
    SortedVector(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark(avl_name, [data](benchmark::State& state) {
    AvlTree(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark(b_name, [data](benchmark::State& state) {
    BTree(state, data);
  })->Unit(benchmark::kMillisecond);
}

void Experiment111(const std::string& datasets_str) {
  const std::string data_1_1_1 = datasets_str + "/data_1_1_1.txt";

  Experiment("Set111", "Vector111", "AvlTree111", "BTree111", data_1_1_1);
}

void Experiment112(const std::string& datasets_str) {
  const std::string data_1_1_2 = datasets_str + "/data_1_1_2.txt";

  Experiment("Set112", "Vector112", "AvlTree112", "BTree112", data_1_1_2);
}

void Experiment113(const std::string& datasets_str) {
  const std::string data_1_1_3 = datasets_str + "/data_1_1_3.txt";

  Experiment("Set113", "Vector113", "AvlTree113", "BTree113", data_1_1_3);
}

void Experiment121(const std::string& datasets_str) {
  const std::string data_1_2_1 = datasets_str + "/data_1_2_1.txt";

  Experiment("Set121", "Vector121", "AvlTree121", "BTree121", data_1_2_1);
}

void Experiment122(const std::string& datasets_str) {
  const std::string data_1_2_2 = datasets_str + "/data_1_2_2.txt";

  Experiment("Set122", "Vector122", "AvlTree122", "BTree122", data_1_2_2);
}

void Experiment123(const std::string& datasets_str) {
  const std::string data_1_2_3 = datasets_str + "/data_1_2_3.txt";

  Experiment("Set123", "Vector123", "AvlTree123", "BTree123", data_1_2_3);
}

void Experiment131(const std::string& datasets_str) {
  const std::string data_1_3_1 = datasets_str + "/data_1_3_1.txt";

  Experiment("Set131", "Vector131", "AvlTree131", "BTree131", data_1_3_1);
}

void Experiment132(const std::string& datasets_str) {
  const std::string data_1_3_2 = datasets_str + "/data_1_3_2.txt";

  Experiment("Set132", "Vector132", "AvlTree132", "BTree132", data_1_3_2);
}

void Experiment133(const std::string& datasets_str) {
  const std::string data_1_3_3 = datasets_str + "/data_1_3_3.txt";

  Experiment("Set133", "Vector133", "AvlTree133", "BTree133", data_1_3_3);
}

void Experiment211(const std::string& datasets_str) {
  const std::string data_2_1_1 = datasets_str + "/data_2_1_1.txt";

  Experiment("Set211", "Vector211", "AvlTree211", "BTree211", data_2_1_1);
}

void Experiment212(const std::string& datasets_str) {
  const std::string data_2_1_2 = datasets_str + "/data_2_1_2.txt";

  Experiment("Set212", "Vector212", "AvlTree212", "BTree212", data_2_1_2);
}

void Experiment213(const std::string& datasets_str) {
  const std::string data_2_1_3 = datasets_str + "/data_2_1_3.txt";

  Experiment("Set213", "Vector213", "AvlTree213", "BTree213", data_2_1_3);
}

void Experiment221(const std::string& datasets_str) {
  const std::string data_2_2_1 = datasets_str + "/data_2_2_1.txt";

  Experiment("Set221", "Vector221", "AvlTree221", "BTree221", data_2_2_1);
}

void Experiment222(const std::string& datasets_str) {
  const std::string data_2_2_2 = datasets_str + "/data_2_2_2.txt";

  Experiment("Set222", "Vector222", "AvlTree222", "BTree222", data_2_2_2);
}

void Experiment223(const std::string& datasets_str) {
  const std::string data_2_2_3 = datasets_str + "/data_2_2_3.txt";

  Experiment("Set223", "Vector223", "AvlTree223", "BTree223", data_2_2_3);
}

void Experiment231(const std::string& datasets_str) {
  const std::string data_2_3_1 = datasets_str + "/data_2_3_1.txt";

  Experiment("Set231", "Vector231", "AvlTree231", "BTree231", data_2_3_1);
}

void Experiment232(const std::string& datasets_str) {
  const std::string data_2_3_2 = datasets_str + "/data_2_3_2.txt";

  Experiment("Set232", "Vector232", "AvlTree232", "BTree232", data_2_3_2);
}

void Experiment233(const std::string& datasets_str) {
  const std::string data_2_3_3 = datasets_str + "/data_2_3_3.txt";

  Experiment("Set233", "Vector233", "AvlTree233", "BTree233", data_2_3_3);
}

void Experiment311(const std::string& datasets_str) {
  const std::string data_3_1_1 = datasets_str + "/data_3_1_1.txt";

  Experiment("Set311", "Vector311", "AvlTree311", "BTree311", data_3_1_1);
}

void Experiment312(const std::string& datasets_str) {
  const std::string data_3_1_2 = datasets_str + "/data_3_1_2.txt";

  Experiment("Set312", "Vector312", "AvlTree312", "BTree312", data_3_1_2);
}

void Experiment313(const std::string& datasets_str) {
  const std::string data_3_1_3 = datasets_str + "/data_3_1_3.txt";

  Experiment("Set313", "Vector313", "AvlTree313", "BTree313", data_3_1_3);
}

void Experiment321(const std::string& datasets_str) {
  const std::string data_3_2_1 = datasets_str + "/data_3_2_1.txt";

  Experiment("Set321", "Vector321", "AvlTree321", "BTree321", data_3_2_1);
}

void Experiment322(const std::string& datasets_str) {
  const std::string data_3_2_2 = datasets_str + "/data_3_2_2.txt";

  Experiment("Set322", "Vector322", "AvlTree322", "BTree322", data_3_2_2);
}

void Experiment323(const std::string& datasets_str) {
  const std::string data_3_2_3 = datasets_str + "/data_3_2_3.txt";

  Experiment("Set323", "Vector323", "AvlTree323", "BTree323", data_3_2_3);
}

void Experiment331(const std::string& datasets_str) {
  const std::string data_3_3_1 = datasets_str + "/data_3_3_1.txt";

  Experiment("Set331", "Vector331", "AvlTree331", "BTree331", data_3_3_1);
}

void Experiment332(const std::string& datasets_str) {
  const std::string data_3_3_2 = datasets_str + "/data_3_3_2.txt";

  Experiment("Set332", "Vector332", "AvlTree332", "BTree332", data_3_3_2);
}

void Experiment333(const std::string& datasets_str) {
  const std::string data_3_3_3 = datasets_str + "/data_3_3_3.txt";

  Experiment("Set333", "Vector333", "AvlTree333", "BTree333", data_3_3_3);
}

void Experiment411(const std::string& datasets_str) {
  const std::string data_4_1_1 = datasets_str + "/data_4_1_1.txt";

  Experiment("Set411", "Vector411", "AvlTree411", "BTree411", data_4_1_1);
}

void Experiment412(const std::string& datasets_str) {
  const std::string data_4_1_2 = datasets_str + "/data_4_1_2.txt";

  Experiment("Set412", "Vector412", "AvlTree412", "BTree412", data_4_1_2);
}

void Experiment413(const std::string& datasets_str) {
  const std::string data_4_1_3 = datasets_str + "/data_4_1_3.txt";

  Experiment("Set413", "Vector413", "AvlTree413", "BTree413", data_4_1_3);
}

void Experiment421(const std::string& datasets_str) {
  const std::string data_4_2_1 = datasets_str + "/data_4_2_1.txt";

  Experiment("Set421", "Vector421", "AvlTree421", "BTree421", data_4_2_1);
}

void Experiment422(const std::string& datasets_str) {
  const std::string data_4_2_2 = datasets_str + "/data_4_2_2.txt";

  Experiment("Set422", "Vector422", "AvlTree422", "BTree422", data_4_2_2);
}

void Experiment423(const std::string& datasets_str) {
  const std::string data_4_2_3 = datasets_str + "/data_4_2_3.txt";

  Experiment("Set423", "Vector423", "AvlTree423", "BTree423", data_4_2_3);
}

void Experiment431(const std::string& datasets_str) {
  const std::string data_4_3_1 = datasets_str + "/data_4_3_1.txt";

  Experiment("Set431", "Vector431", "AvlTree431", "BTree431", data_4_3_1);
}

void Experiment432(const std::string& datasets_str) {
  const std::string data_4_3_2 = datasets_str + "/data_4_3_2.txt";

  Experiment("Set432", "Vector432", "AvlTree432", "BTree432", data_4_3_2);
}

void Experiment433(const std::string& datasets_str) {
  const std::string data_4_3_3 = datasets_str + "/data_4_3_3.txt";

  Experiment("Set433", "Vector433", "AvlTree433", "BTree433", data_4_3_3);
}

void Experiment511(const std::string& datasets_str) {
  const std::string data_5_1_1 = datasets_str + "/data_5_1_1.txt";

  Experiment("Set511", "Vector511", "AvlTree511", "BTree511", data_5_1_1);
}

void Experiment512(const std::string& datasets_str) {
  const std::string data_5_1_2 = datasets_str + "/data_5_1_2.txt";

  Experiment("Set512", "Vector512", "AvlTree512", "BTree512", data_5_1_2);
}

void Experiment513(const std::string& datasets_str) {
  const std::string data_5_1_3 = datasets_str + "/data_5_1_3.txt";

  Experiment("Set513", "Vector513", "AvlTree513", "BTree513", data_5_1_3);
}

void Experiment521(const std::string& datasets_str) {
  const std::string data_5_2_1 = datasets_str + "/data_5_2_1.txt";

  Experiment("Set521", "Vector521", "AvlTree521", "BTree521", data_5_2_1);
}

void Experiment522(const std::string& datasets_str) {
  const std::string data_5_2_2 = datasets_str + "/data_5_2_2.txt";

  Experiment("Set522", "Vector522", "AvlTree522", "BTree522", data_5_2_2);
}

void Experiment523(const std::string& datasets_str) {
  const std::string data_5_2_3 = datasets_str + "/data_5_2_3.txt";

  Experiment("Set523", "Vector523", "AvlTree523", "BTree523", data_5_2_3);
}

void Experiment531(const std::string& datasets_str) {
  const std::string data_5_3_1 = datasets_str + "/data_5_3_1.txt";

  Experiment("Set531", "Vector531", "AvlTree531", "BTree531", data_5_3_1);
}

void Experiment532(const std::string& datasets_str) {
  const std::string data_5_3_2 = datasets_str + "/data_5_3_2.txt";

  Experiment("Set532", "Vector532", "AvlTree532", "BTree532", data_5_3_2);
}

void Experiment533(const std::string& datasets_str) {
  const std::string data_5_3_3 = datasets_str + "/data_5_3_3.txt";

  Experiment("Set533", "Vector533", "AvlTree533", "BTree533", data_5_3_3);
}

void Experiment611(const std::string& datasets_str) {
  const std::string data_6_1_1 = datasets_str + "/data_6_1_1.txt";

  Experiment("Set611", "Vector611", "AvlTree611", "BTree611", data_6_1_1);
}

void Experiment612(const std::string& datasets_str) {
  const std::string data_6_1_2 = datasets_str + "/data_6_1_2.txt";

  Experiment("Set612", "Vector612", "AvlTree612", "BTree612", data_6_1_2);
}

void Experiment613(const std::string& datasets_str) {
  const std::string data_6_1_3 = datasets_str + "/data_6_1_3.txt";

  Experiment("Set613", "Vector613", "AvlTree613", "BTree613", data_6_1_3);
}

void Experiment621(const std::string& datasets_str) {
  const std::string data_6_2_1 = datasets_str + "/data_6_2_1.txt";

  Experiment("Set621", "Vector621", "AvlTree621", "BTree621", data_6_2_1);
}

void Experiment622(const std::string& datasets_str) {
  const std::string data_6_2_2 = datasets_str + "/data_6_2_2.txt";

  Experiment("Set622", "Vector622", "AvlTree622", "BTree622", data_6_2_2);
}

void Experiment623(const std::string& datasets_str) {
  const std::string data_6_2_3 = datasets_str + "/data_6_2_3.txt";

  Experiment("Set623", "Vector623", "AvlTree623", "BTree623", data_6_2_3);
}

void Experiment631(const std::string& datasets_str) {
  const std::string data_6_3_1 = datasets_str + "/data_6_3_1.txt";

  Experiment("Set631", "Vector631", "AvlTree631", "BTree631", data_6_3_1);
}

void Experiment632(const std::string& datasets_str) {
  const std::string data_6_3_2 = datasets_str + "/data_6_3_2.txt";

  Experiment("Set632", "Vector632", "AvlTree632", "BTree632", data_6_3_2);
}

void Experiment633(const std::string& datasets_str) {
  const std::string data_6_3_3 = datasets_str + "/data_6_3_3.txt";

  Experiment("Set633", "Vector633", "AvlTree633", "BTree633", data_6_3_3);
}

void BTreeExperiment(const std::string& datasets_str) {
  const std::string data = datasets_str + "/data_6_1_3.txt";

  benchmark::RegisterBenchmark("Degree 5", [data](benchmark::State& state) {
    BTree5(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 10", [data](benchmark::State& state) {
    BTree10(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 20", [data](benchmark::State& state) {
    BTree20(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 30", [data](benchmark::State& state) {
    BTree30(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 40", [data](benchmark::State& state) {
    BTree40(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 50", [data](benchmark::State& state) {
    BTree50(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 60", [data](benchmark::State& state) {
    BTree60(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 70", [data](benchmark::State& state) {
    BTree70(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 80", [data](benchmark::State& state) {
    BTree80(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 90", [data](benchmark::State& state) {
    BTree90(state, data);
  })->Unit(benchmark::kMillisecond);

  benchmark::RegisterBenchmark("Degree 100", [data](benchmark::State& state) {
    BTree100(state, data);
  })->Unit(benchmark::kMillisecond);
}

} // namespace

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);

  const char* bin_relative_path = argv[0];
  const std::string datasets_str = config_paths::GetAbsDatasets(bin_relative_path);

  /*
  Experiment111(datasets_str);
  Experiment112(datasets_str);
  Experiment113(datasets_str);
  Experiment121(datasets_str);
  Experiment122(datasets_str);
  Experiment123(datasets_str);
  Experiment131(datasets_str);
  Experiment132(datasets_str);
  Experiment133(datasets_str);
  Experiment211(datasets_str);
  Experiment212(datasets_str);
  Experiment213(datasets_str);
  Experiment221(datasets_str);
  Experiment222(datasets_str);
  Experiment223(datasets_str);
  Experiment231(datasets_str);
  Experiment232(datasets_str);
  Experiment233(datasets_str);
  Experiment311(datasets_str);
  Experiment312(datasets_str);
  Experiment313(datasets_str);
  Experiment321(datasets_str);
  Experiment322(datasets_str);
  Experiment323(datasets_str);
  Experiment331(datasets_str);
  Experiment332(datasets_str);
  Experiment333(datasets_str);
  Experiment411(datasets_str);
  Experiment412(datasets_str);
  Experiment413(datasets_str);
  Experiment421(datasets_str);
  Experiment422(datasets_str);
  Experiment423(datasets_str);
  Experiment431(datasets_str);
  Experiment432(datasets_str);
  Experiment433(datasets_str);
  Experiment511(datasets_str);
  Experiment512(datasets_str);
  Experiment513(datasets_str);
  Experiment521(datasets_str);
  Experiment522(datasets_str);
  Experiment523(datasets_str);
  Experiment531(datasets_str);
  Experiment532(datasets_str);
  Experiment533(datasets_str);
  Experiment611(datasets_str);
  Experiment612(datasets_str);
  Experiment613(datasets_str);
  Experiment621(datasets_str);
  Experiment622(datasets_str);
  Experiment623(datasets_str);
  Experiment631(datasets_str);
  Experiment632(datasets_str);
  Experiment633(datasets_str);
  */

  BTreeExperiment(datasets_str);

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
}
