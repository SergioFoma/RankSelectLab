#include <CLI/CLI.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <variant>
#include <array>
#include <print>
#include <cstdio>
#include <execution>

import integer_keys_set;
import parser;


namespace {

static constexpr uint8_t kStructNumbers = 4;

void PrintRes(const std::vector<int64_t>& result) {
  for (auto& res : result) {
    std::print("{} ", res);
  }

  std::cout << '\n';
}

enum class DataStructType: uint8_t {
  kStdSet       = 0,
  kSortedVector = 1,
  kAvlTree      = 2,
  kBTree        = 3
};

prog_error::ErrorCode FlagsChecking(
  size_t min_degree,
  CLI::Option* degree_opt,
  DataStructType struct_type) {
    assert(degree_opt);

    if (!degree_opt->empty() &&
        struct_type != DataStructType::kBTree) {
      return prog_error::ErrorCode::kCommandFlags;
    }

    return prog_error::ErrorCode::kCorrect;
}

using StdSet = dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl>;
using SortedVector = dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl>;
using AvlTree = dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl>;
using BTree = dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl>;

class DataStructVisitor {
 public:
  explicit DataStructVisitor(std::string input_file)
    : input_file_{input_file} {}

  void operator()(BTree& b_tree) {
    CallExecution<BTree>(b_tree);
  }

  void operator()(SortedVector& sorted_vector) {
    CallExecution<SortedVector>(sorted_vector);
  }

  void operator()(AvlTree& avl_tree) {
    CallExecution<AvlTree>(avl_tree);
  }

  void operator()(StdSet& std_set) {
    CallExecution<StdSet>(std_set);
  }

 private:
  std::string input_file_;

  template <typename T>
  void CallExecution(T& data_struct) {
    std::vector<int64_t> result = parser::ExecuteRequests(input_file_, data_struct);
    PrintRes(result);
  }
};

using StructVariant = std::variant<BTree, SortedVector, AvlTree, StdSet> ;

using DataStructFunc = StructVariant (*)(size_t);
}  // namespace

int main(int argc, char** argv) try {

    CLI::App app("Rank Select Lab");
    argv = app.ensure_utf8(argv);

    DataStructType struct_type{DataStructType::kStdSet};
    size_t min_degree = dynamic_order::kMinDegree;
    std::string input_file;

    const std::map<std::string, DataStructType> map = {
      {"b_tree"          , DataStructType::kBTree},
      {"sorted_vector"   , DataStructType::kSortedVector},
      {"avl_tree"        , DataStructType::kAvlTree},
      {"std_set"         , DataStructType::kStdSet}
    };
    app.add_option("-f,--input-file,input_file", input_file,
                   "The path to commands")
        ->required();

    app.add_option("-t,--struct-type", struct_type,
                   "The data structure which will be based for keys set")
        ->transform(CLI::CheckedTransformer(map, CLI::ignore_case))
        ->required();

    CLI::Option* degree_opt = app.add_option("-k,--min-degree", min_degree,
                   "The min degree for b - tree");
    CLI11_PARSE(app, argc, argv);

    if (FlagsChecking(min_degree, degree_opt, struct_type) != prog_error::ErrorCode::kCorrect) {
      std::println(stderr, "Incorrect use of command line flags");
      return static_cast<int>(prog_error::ErrorCode::kCommandFlags);
    }

    const std::array<DataStructFunc, kStructNumbers> structs = {
      [](size_t min_degree)->StructVariant {return BTree{min_degree};},
      [](size_t min_degree)->StructVariant {return SortedVector{};},
      [](size_t min_degree)->StructVariant {return AvlTree{};},
      [](size_t min_degree)->StructVariant {return StdSet{};}
    };

    StructVariant data_struct = structs[static_cast<uint8_t>(struct_type)](min_degree);
    std::visit(DataStructVisitor(input_file), data_struct);
} catch (const prog_error::ProgError& e) {
    e.Log();
    std::println(stderr, "Message: {}", e.what());
    return static_cast<int>(e.GetErrorCode());
}
