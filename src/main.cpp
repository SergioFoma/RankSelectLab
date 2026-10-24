#include <iostream>
#include <CLI/CLI.hpp>
#include <string>
#include <vector>
#include <cstdint>

import integer_keys_set;
import parser;

namespace {

void PrintRes(std::vector<int64_t>& result) {
  for (auto& res: result) {
    std::cout << res << ' ';
  }

  std::cout << '\n';
}

void BaseBTree(const std::string& input_file, size_t min_degree) {

  dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree{min_degree};

  std::vector<int64_t> result = parser::ParseData(input_file, b_tree);

  PrintRes(result);
}

void BaseVector(const std::string& input_file) {

  dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl> sorted_v{};

  std::vector<int64_t> result = parser::ParseData(input_file, sorted_v);

  PrintRes(result);
}

void BaseAvlTree(const std::string& input_file) {

  dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree{};

  std::vector<int64_t> result = parser::ParseData(input_file, avl_tree);

  PrintRes(result);
}

void BaseSet(const std::string& input_file) {

  dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> std_set{};

  std::vector<int64_t> result = parser::ParseData(input_file, std_set);

  PrintRes(result);
}

}

int main(int argc, char** argv) {

  try {
    CLI::App app("Rank select lab");
    argv = app.ensure_utf8(argv);
    bool is_std_set  = false   ,   is_sorted_vector = false;
    bool is_avl_tree = false   ,   is_b_tree        = false;
    size_t min_degree = dynamic_order::kMinDegree;
    std::string input_file;

    app.add_flag("--std-set", is_std_set, "The algorithm is based on a std base");
    app.add_flag("--sorted-vector", is_sorted_vector, "The algorithm is base on sorted vector");
    app.add_flag("--avl-tree", is_avl_tree, "The algorithm is base on avl - tree");
    app.add_flag("--b-tree", is_b_tree, "The algorithm is base on b - tree");
    app.add_option("-f,--input-file,input_file", input_file, "The path to commands")->required();
    app.add_option("-k,--min-degree", min_degree, "The min degree for b - tree");
    CLI11_PARSE(app, argc, argv);

    if (is_b_tree) {
      BaseBTree(input_file, min_degree);
    } else if (is_sorted_vector) {
      BaseVector(input_file);
    } else if (is_avl_tree) {
      BaseAvlTree(input_file);
    } else {
      BaseSet(input_file);
    }
  } catch (const prog_error::ProgError& e) {
    const prog_error::ErrorInfo& info = e.GetErrorInfo();
    std::cerr << "Error in " << info.func_name << "\nLog: " << e.what() << '\n';
    return static_cast<int>(info.code);
  }

  return 0;
}
