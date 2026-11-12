#include <benchmark/benchmark.h>
#include <CLI/CLI.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <string>
#include <vector>

import integer_keys_set;
import parser;

namespace {

constexpr uint8_t kDistribLowerBound = 1;
constexpr uint8_t kDistribUpperBound = 6;
constexpr uint8_t kFractionLowerBound = 1;
constexpr uint8_t kFractionUpperBound = 3;
constexpr uint8_t kNumLowerBound = 1;
constexpr uint8_t kNumUpperBound = 3;

constexpr uint8_t kDistrib = 6;   // for b - tree research
constexpr uint8_t kFraction = 1;  // for b - tree research
constexpr uint8_t kNum = 3;       // for b - tree research

constexpr size_t kDegree5 = 5;
constexpr size_t kDegree10 = 10;
constexpr size_t kDefaultDegree = 50;
constexpr size_t kDegree100 = 100;
constexpr size_t kStep = 10;

void StdSet(benchmark::State& state, const std::string& input_file) {

  std::vector<Token> tokens = parser::Tokenize(input_file);

  for (auto st : state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> std_set{};
    std::vector<int64_t> result =
        parser::RunRequests<parser::VoidSave>(tokens, std_set);

    benchmark::DoNotOptimize(result);
  }
}

void SortedVector(benchmark::State& state, const std::string& input_file) {

  std::vector<Token> tokens = parser::Tokenize(input_file);

  for (auto st : state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl>
        sorted_vector{};
    std::vector<int64_t> result =
        parser::RunRequests<parser::VoidSave>(tokens, sorted_vector);

    benchmark::DoNotOptimize(result);
  }
}

void AvlTree(benchmark::State& state, const std::string& input_file) {

  std::vector<Token> tokens = parser::Tokenize(input_file);

  for (auto st : state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree{};
    std::vector<int64_t> result =
        parser::RunRequests<parser::VoidSave>(tokens, avl_tree);

    benchmark::DoNotOptimize(result);
  }
}

void BTree(benchmark::State& state, const std::string& input_file,
           size_t degree) {

  std::vector<Token> tokens = parser::Tokenize(input_file);

  for (auto st : state) {
    benchmark::ClobberMemory();

    dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{degree};
    std::vector<int64_t> result =
        parser::RunRequests<parser::VoidSave>(tokens, b_tree);

    benchmark::DoNotOptimize(result);
  }
}

void Experiment(const std::string& set_name, const std::string& vector_name,
                const std::string& avl_name, const std::string& b_name,
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
    BTree(state, data, kDefaultDegree);
  })->Unit(benchmark::kMillisecond);
}

void BTreeExperiment(const std::string& set_name, size_t degree,
                     const std::string& data) {

  benchmark::RegisterBenchmark(set_name, [data,
                                          degree](benchmark::State& state) {
    BTree(state, data, degree);
  })->Unit(benchmark::kMillisecond);
}

class BenchmarkInfo {
 public:
  BenchmarkInfo(const std::string& datasets_path, uint8_t distribution,
                uint8_t fraction, uint8_t num)
      : file_path_(std::format("{}/data_{}_{}_{}.txt", datasets_path,
                               distribution, fraction, num)),
        set_name_(std::format("Set{}{}{}", distribution, fraction, num)),
        vector_name_(std::format("Vector{}{}{}", distribution, fraction, num)),
        avl_tree_name_(
            std::format("AvlTree{}{}{}", distribution, fraction, num)),
        b_tree_name_(std::format("BTree{}{}{}", distribution, fraction, num)) {

    assert(kDistribLowerBound <= distribution &&
           distribution <= kDistribUpperBound);
    assert(kFractionLowerBound <= fraction && fraction <= kFractionUpperBound);
    assert(kNumLowerBound <= num && num <= kNumUpperBound);
  }

  const std::string& GetFilePath() const { return file_path_; }

  const std::string& GetSetName() const { return set_name_; }

  const std::string& GetVectorName() const { return vector_name_; }

  const std::string& GetAvlTreeName() const { return avl_tree_name_; }

  const std::string& GetBTreeName() const { return b_tree_name_; }

 private:
  std::string file_path_;
  std::string set_name_;
  std::string vector_name_;
  std::string avl_tree_name_;
  std::string b_tree_name_;
};

class BTreeResearchInfo {
 public:
  BTreeResearchInfo(const std::string& datasets_path, size_t degree,
                    uint8_t distribution, uint8_t fraction, uint8_t num)
      : file_path_(std::format("{}/data_{}_{}_{}.txt", datasets_path,
                               distribution, fraction, num)),
        set_name_(std::format("Degree {}", degree)),
        degree_(degree) {

    assert(kDistribLowerBound <= distribution &&
           distribution <= kDistribUpperBound);
    assert(kFractionLowerBound <= fraction && fraction <= kFractionUpperBound);
    assert(kNumLowerBound <= num && num <= kNumUpperBound);
  }

  const std::string& GetFilePath() const { return file_path_; }

  const std::string& GetSetName() const { return set_name_; }

  size_t GetDegree() const { return degree_; }

 private:
  std::string file_path_;
  std::string set_name_;
  size_t degree_;
};

}  // namespace

int main(int argc, char** argv) try {

  CLI::App app("Benchmarks");
  argv = app.ensure_utf8(argv);
  std::string datasets_path;

  app.add_option("-p,--datasets-path,datasets_path", datasets_path,
                 "The path to folder with datasets")
      ->required();

  CLI11_PARSE(app, argc, argv);

  benchmark::Initialize(&argc, argv);

  std::vector<BenchmarkInfo> input_data{};
  std::vector<BTreeResearchInfo> b_tree_res{
      {datasets_path, kDegree5, kDistrib, kFraction, kNum}};
  // ============================ GENERATORS ===================================================
  for (uint8_t distrib = kDistribLowerBound; distrib <= kDistribUpperBound;
       ++distrib) {
    for (uint8_t fraction = kFractionLowerBound;
         fraction <= kFractionUpperBound; ++fraction) {
      for (uint8_t num = kNumLowerBound; num <= kNumUpperBound; ++num) {
        input_data.push_back({datasets_path, distrib, fraction, num});
      }
    }
  }

  for (size_t degree = kDegree10; degree <= kDegree100; degree += kStep) {
    b_tree_res.push_back({datasets_path, degree, kDistrib, kFraction, kNum});
  }

  // ============================= RUN BENCHMARKS ==============================================
  for (const auto& set : input_data) {
    Experiment(set.GetSetName(), set.GetVectorName(), set.GetAvlTreeName(),
               set.GetBTreeName(), set.GetFilePath());
  }

  for (const auto& set : b_tree_res) {
    BTreeExperiment(set.GetSetName(), set.GetDegree(), set.GetFilePath());
  }

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
} catch (const prog_error::ProgError& e) {
  e.Log();
  std::println(stderr, "Message: {}", e.what());
  return static_cast<int>(e.GetErrorCode());
}
