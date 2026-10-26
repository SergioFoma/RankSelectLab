#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <iostream>

import integer_keys_set;
import parser;
import configuring_paths;

namespace {

constexpr size_t kTestsNumber = 4;
constexpr size_t kDegree = 5;


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

void ComparingValues(const std::vector<int64_t>& res,
                     const std::vector<int64_t>& correct_res,
                     const std::string& file_name) {
  bool flag = true;
  size_t status_numbers = correct_res.size();

  for (size_t ind = 0; ind < status_numbers && flag; ++ind) {
    flag = (res[ind] == correct_res[ind]);
  }

  std::cout << "RUNNING:  " << file_name << '\n';
  if (flag) {
    std::cout << "STATUS: \033[32m[PASSED]\033[0m\n\n";
  } else {
    std::cout << "STATUS: \033[31m[FAILED]\033[0m\n\n";
  }
}

void TestingSet(const std::string& input_file,
                const std::string& output_file,
                std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::StdSetImpl> set_impl{};

  std::vector<int64_t> res =
      parser::ParseData(input_file, set_impl);

  std::cout << "Base: \033[34m[StdSet]\033[0m\n";
  ComparingValues(res, correct_res, input_file);
}

void TestingVector(const std::string& input_file,
                   const std::string& output_file,
                   std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::SortedVectorImpl> vector_impl{};

  std::vector<int64_t> res =
      parser::ParseData(input_file, vector_impl);

  std::cout << "Base: \033[34m[SortedVector]\033[0m\n";
  ComparingValues(res, correct_res, input_file);
}

void TestingAvlTree(const std::string& input_file,
                    const std::string& output_file,
                    std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::AvlTreeImpl> avl_tree_impl{};

  std::vector<int64_t> res =
      parser::ParseData(input_file, avl_tree_impl);

  std::cout << "Base: \033[34m[AvlTree]\033[0m\n";
  ComparingValues(res, correct_res, input_file);
}

void TestingBTree(const std::string& input_file,
                  const std::string& output_file,
                  std::vector<int64_t>& correct_res) {

  dynamic_order::IntegerKeysSet<dynamic_order::BTreeImpl> b_tree_impl{kDegree};

  std::vector<int64_t> res =
      parser::ParseData(input_file, b_tree_impl);

  std::cout << "Base: \033[34m[BTree]\033[0m\n";
  ComparingValues(res, correct_res, input_file);
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

  for (size_t ind = 0; ind < kTestsNumber; ++ind) {
    const std::string& output_file = output_data[ind];
    const std::string& input_file = input_data[ind];

    std::vector<int64_t> correct_res =
      ReadCorrectStatuses(output_file);
    TestingSet(input_file, output_file, correct_res);
    TestingVector(input_file, output_file, correct_res);
    TestingAvlTree(input_file, output_file, correct_res);
    TestingBTree(input_file, output_file, correct_res);
  }

}
