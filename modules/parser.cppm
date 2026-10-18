module;

#include <istream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>

export module parser;

import integer_keys_set;


template <typename T>
void InsertKey(std::istream& in,
               dynamic_order::IntegerKeysSet<T>& keys) {
  int key = 0;

  in >> key;

  keys.Insert(key);
}

template <typename T>
void GetQuery(std::istream& in,
              std::vector<int64_t>& results,
              dynamic_order::IntegerKeysSet<T>& keys) {

  int left = 0, right = 0;

  in >> left >> right;

  int64_t res = static_cast<int64_t>(keys.Query(left, right));

  results.push_back(res);
}

template <typename T>
void GetMin(std::istream& in,
              std::vector<int64_t>& results,
              dynamic_order::IntegerKeysSet<T>& keys) {

  size_t index = 0;

  in >> index;

  int64_t res = static_cast<int64_t>(keys.Min(index));

  results.push_back(res);
}

template <typename T>
void GetLessKeysCounter(std::istream& in,
              std::vector<int64_t>& results,
              dynamic_order::IntegerKeysSet<T>& keys) {

  int key = 0;

  in >> key;

  int64_t res = static_cast<int64_t>(keys.LessKeysCounter(key));

  results.push_back(res);
}

export namespace parser {

template <typename T>
std::vector<int64_t>
ParseData(std::istream& in, dynamic_order::IntegerKeysSet<T>& keys) {
  // failbit - format errorr
  // badbit  - system error
  in.exceptions(std::istream::badbit);

  char command = 0;
  std::vector<int64_t> results;

  while (in >> command) {
    switch (command) {
      case 'k':
       InsertKey(in, keys);
       break;
      case 'q':
        GetQuery(in, results, keys);
        break;
      case 'm':
        GetMin(in, results, keys);
        break;
      case 'n':
        GetLessKeysCounter(in, results, keys);
        break;
    }
  }

  if (!in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of reading data",
                                   "parser::ParseData"};
    throw prog_error::ProgError(std::move(inf));
  }

  return results;
}

template <typename T>
std::vector<int64_t>
ParseData(const std::string& file_name,
                dynamic_order::IntegerKeysSet<T>& keys) {
  std::ifstream file_stream(file_name);

  if (!file_stream.is_open()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of opening file",
                                   "parser::ParseData"};
    throw prog_error::ProgError(std::move(inf));
  }

  return ParseData(file_stream, keys);
}
} // namespace parser




