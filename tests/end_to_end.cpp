#include <fmt/color.h>
#include <CLI/CLI.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

import integer_keys_set;
import parser;
import prog_error;

namespace {

constexpr size_t kTestsNumber = 4;
constexpr size_t kDegree = 5;

enum class TestStatus : int { kCorrect, kError };

std::vector<int64_t> ReadCorrectStatuses(
    const std::filesystem::path& file_path) {
  std::ifstream file(file_path, std::ios::binary);

  if (!file.is_open()) {
    throw prog_error::ProgError{"Error of opening file",
                                prog_error::ErrorCode::kTests};
  }

  std::vector<int64_t> correct_status(std::istream_iterator<int64_t>{file},
                                      std::istream_iterator<int64_t>());

  return correct_status;
}

TestStatus ComparingValues(const std::vector<int64_t>& res,
                           const std::vector<int64_t>& correct_res,
                           const std::string& file_name) {
  size_t correct_res_sz = correct_res.size();
  size_t current_res_sz = res.size();

  if (correct_res_sz != current_res_sz) {
    throw prog_error::ProgError{"Incorrect output result size",
                                prog_error::ErrorCode::kTests};
  }

  bool flag = true;
  for (size_t ind = 0; ind < correct_res_sz && flag; ++ind) {
    flag = (res[ind] == correct_res[ind]);
  }

  std::cout << "RUNNING:  " << file_name << '\n';
  if (flag) {
    fmt::print(stdout, "STATUS: {}\n\n",
               fmt::styled("[PASSED]", fg(fmt::color::green)));
    return TestStatus::kCorrect;
  } else {
    fmt::print(stdout, "STATUS: {}\n\n",
               fmt::styled("[FAILED]", fg(fmt::color::red)));
    return TestStatus::kError;
  }

  std::unreachable();
}

TestStatus TestingSet(const std::string& input_file,
                      std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> set_impl{};

  std::vector<int64_t> res = parser::ExecuteRequests(input_file, set_impl);

  fmt::print(stdout, "Base: {}",
             fmt::styled("[StdSet]\n", fg(fmt::color::blue)));
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingVector(const std::string& input_file,
                         std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl> vector_impl{};

  std::vector<int64_t> res = parser::ExecuteRequests(input_file, vector_impl);

  fmt::print(stdout, "Base: {}\n",
             fmt::styled("[SortedVector]", fg(fmt::color::blue)));
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingAvlTree(const std::string& input_file,
                          std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree_impl{};

  std::vector<int64_t> res = parser::ExecuteRequests(input_file, avl_tree_impl);

  fmt::print(stdout, "Base: {}\n",
             fmt::styled("[AvlTree]", fg(fmt::color::blue)));
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingBTree(const std::string& input_file,
                        std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree_impl{kDegree};

  std::vector<int64_t> res = parser::ExecuteRequests(input_file, b_tree_impl);

  fmt::print(stdout, "Base: {}\n",
             fmt::styled("[BTree]", fg(fmt::color::blue)));
  return ComparingValues(res, correct_res, input_file);
}

int RunCompareResults(
    std::array<const std::filesystem::path, kTestsNumber>& input_data,
    std::array<const std::filesystem::path, kTestsNumber>& output_data) {

  for (size_t ind = 0; ind < kTestsNumber; ++ind) {
    const std::filesystem::path& output_path = output_data[ind];
    const std::filesystem::path& input_path = input_data[ind];
    const std::string input_file =
        std::filesystem::absolute(input_path).string();

    std::vector<int64_t> correct_res = ReadCorrectStatuses(output_path);

    if (TestingSet(input_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    }
    if (TestingVector(input_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    }
    if (TestingAvlTree(input_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    };
    if (TestingBTree(input_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    };
  }

  return static_cast<int>(TestStatus::kCorrect);
}

}  // namespace

int main(int argc, char** argv) {

  CLI::App app("End - to - end test");
  argv = app.ensure_utf8(argv);
  std::string materials_path;

  app.add_option("-p,--materials-path,materials_path", materials_path,
                 "The path to folder with tests materials")
      ->required();

  CLI11_PARSE(app, argc, argv);

  std::array<const std::filesystem::path, kTestsNumber> input_data = {
      materials_path + "input_100.txt", materials_path + "input_1000.txt",
      materials_path + "input_10000.txt", materials_path + "input_100000.txt"};

  std::array<const std::filesystem::path, kTestsNumber> output_data = {
      materials_path + "output_100.txt", materials_path + "output_1000.txt",
      materials_path + "output_10000.txt",
      materials_path + "output_100000.txt"};

  return RunCompareResults(input_data, output_data);
}
