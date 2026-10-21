#include <benchmark/benchmark.h>
#include <string>
#include <iostream>

import integer_keys_set;
import parser;
import configuring_paths;


namespace {

const size_t kDegree = 50;

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

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{kDegree};
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

void Experiment11(const std::string& datasets_str) {
  const std::string data_1_1 = datasets_str + "/data_1_1.txt";

  Experiment("Set11", "Vector11", "AvlTree11", "BTree11", data_1_1);
}

void Experiment12(const std::string& datasets_str) {
  const std::string data_1_2 = datasets_str + "/data_1_2.txt";

  Experiment("Set12", "Vector12", "AvlTree12", "BTree12", data_1_2);
}

void Experiment13(const std::string& datasets_str) {
  const std::string data_1_3 = datasets_str + "/data_1_3.txt";

  Experiment("Set13", "Vector13", "AvlTree13", "BTree13", data_1_3);
}

} // namespace

int main(int argc, char** argv) {

  const char* bin_relative_path = argv[0];
  const std::string datasets_str = config_paths::GetAbsDatasets(bin_relative_path);

  //Experiment11(datasets_str);
  //Experiment12(datasets_str);
  Experiment13(datasets_str);

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
}
