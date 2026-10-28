#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <utility>
#include <array>
#include <cstddef>

import integer_keys_set;
import parser;
import configuring_paths;
import prog_error;

namespace {

constexpr size_t kTestsNumber = 4;
constexpr size_t kDegree = 5;

enum class TestStatus {
  kCorrect,
  kError
};

std::vector<int64_t> ReadCorrectStatuses(const std::string& file_name) {
  std::ifstream file(file_name, std::ios::binary);

  if (!file.is_open()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kTests,
                                   "Error of opening file",
                                   "ReadCorrectStatuses"};
    throw prog_error::ProgError(std::move(inf));
  }

  std::vector<int64_t> correct_status;

  int64_t intersect_status = 0;

  while (file >> intersect_status) {
    correct_status.push_back(intersect_status);
  }

  return correct_status;
}

TestStatus ComparingValues(const std::vector<int64_t>& res,
                     const std::vector<int64_t>& correct_res,
                     const std::string& file_name) {
  size_t correct_res_sz = correct_res.size();
  size_t current_res_sz = res.size();

  if (correct_res_sz != current_res_sz) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kTests,
                                   "Incorrect output result size",
                                   "ComparingValues"};
    throw prog_error::ProgError(std::move(inf));
  }

  bool flag = true;
  for (size_t ind = 0; ind < correct_res_sz && flag; ++ind) {
    flag = (res[ind] == correct_res[ind]);
  }

  std::cout << "RUNNING:  " << file_name << '\n';
  if (flag) {
    std::cout << "STATUS: \033[32m[PASSED]\033[0m\n\n";
    return TestStatus::kCorrect;
  } else {
    std::cout << "STATUS: \033[31m[FAILED]\033[0m\n\n";
    return TestStatus::kError;
  }

  std::unreachable();
}

TestStatus TestingSet(const std::string& input_file,
                const std::string& output_file,
                std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> set_impl{};

  std::vector<int64_t> res =
      parser::ExecuteRequests(input_file, set_impl);

  std::cout << "Base: \033[34m[StdSet]\033[0m\n";
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingVector(const std::string& input_file,
                   const std::string& output_file,
                   std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl> vector_impl{};

  std::vector<int64_t> res =
      parser::ExecuteRequests(input_file, vector_impl);

  std::cout << "Base: \033[34m[SortedVector]\033[0m\n";
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingAvlTree(const std::string& input_file,
                    const std::string& output_file,
                    std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree_impl{};

  std::vector<int64_t> res =
      parser::ExecuteRequests(input_file, avl_tree_impl);

  std::cout << "Base: \033[34m[AvlTree]\033[0m\n";
  return ComparingValues(res, correct_res, input_file);
}

TestStatus TestingBTree(const std::string& input_file,
                  const std::string& output_file,
                  std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree_impl{kDegree};

  std::vector<int64_t> res =
      parser::ExecuteRequests(input_file, b_tree_impl);

  std::cout << "Base: \033[34m[BTree]\033[0m\n";
  return ComparingValues(res, correct_res, input_file);
}

int RunCompareResults(std::array<const std::string, kTestsNumber>& input_data,
                      std::array<const std::string, kTestsNumber>& output_data) {

  for (size_t ind = 0; ind < kTestsNumber; ++ind) {
    const std::string& output_file = output_data[ind];
    const std::string& input_file = input_data[ind];

    std::vector<int64_t> correct_res =
      ReadCorrectStatuses(output_file);

    if (TestingSet(input_file, output_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    }
    if (TestingVector(input_file, output_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    }
    if (TestingAvlTree(input_file, output_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    };
    if (TestingBTree(input_file, output_file, correct_res) == TestStatus::kError) {
      return static_cast<int>(TestStatus::kError);
    };
  }

  return static_cast<int>(TestStatus::kCorrect);
}

} // namespace

int main(int argc, char** argv) {

  const char* bin_relative_path = argv[0];
  const std::string materials_str = config_paths::GetAbsMaterials(bin_relative_path);

  std::array<const std::string, kTestsNumber> input_data = {
    materials_str + "/input_100.txt",
    materials_str + "/input_1000.txt",
    materials_str + "/input_10000.txt",
    materials_str + "/input_100000.txt"
  };

  std::array<const std::string, kTestsNumber> output_data = {
    materials_str + "/output_100.txt",
    materials_str + "/output_1000.txt",
    materials_str + "/output_10000.txt",
    materials_str + "/output_100000.txt"
  };


  return RunCompareResults(input_data, output_data);
}
