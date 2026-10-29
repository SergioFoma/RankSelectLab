module;

#include <istream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <utility>

export module parser;

import integer_keys_set;

export struct Token {
  char command_ = 0;
  int key_ = 0;
  int left_ = 0;
  int right_ = 0;
  size_t index_ = 0;

  Token(char command, int key)
    : command_(command), key_(key) {}

  Token(char command, int left, int right)
    : command_(command), left_(left), right_(right) {}

  Token(char command, size_t index)
    : command_(command), index_(index) {}
};

export namespace parser {

struct SaveResult {

  static void save(std::vector<int64_t>& result,
                   int64_t res) {
    result.push_back(res);
  }
};

struct VoidSave {

  static void save(std::vector<int64_t>& result,
                   int64_t res) {}
};
} // namespace parser

void InsertKey(std::istream& in,
               char command,
               std::vector<Token>& tokens) {
  int key = 0;

  in >> key;

  if (in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of reading command",
                                   "parser::InsertKey"};
    throw prog_error::ProgError(std::move(inf));
  }

  tokens.push_back(Token{command, key});
}

void GetQuery(std::istream& in,
              char command,
              std::vector<Token>& result) {

  int left = 0, right = 0;

  in >> left >> right;

  if (in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of reading command",
                                   "parser::GetQuery"};
    throw prog_error::ProgError(std::move(inf));
  }

  result.push_back(Token{command, left, right});
}

void GetMin(std::istream& in,
            char command,
            std::vector<Token>& tokens) {

  size_t index = 0;

  in >> index;

  if (in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of reading command",
                                   "parser::GetMin"};
    throw prog_error::ProgError(std::move(inf));
  }

  tokens.push_back(Token{command, index});
}

void GetLessKeysCounter(std::istream& in,
                        char command,
                        std::vector<Token>& tokens) {

  int key = 0;

  in >> key;

  if (in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of reading command",
                                   "parser::GetLessKeysCounter"};
    throw prog_error::ProgError(std::move(inf));
  }

  tokens.push_back(Token{command, key});
}

template <typename T>
void ExecuteInsert(const Token& token,
                   dynamic_order::IntegerKeysSet<T>& keys) {

  keys.Insert(token.key_);
}

template <typename Saver = parser::VoidSave, typename T>
void ExecuteQuery(const Token& token,
                  std::vector<int64_t>& result,
                  dynamic_order::IntegerKeysSet<T>& keys){

  int64_t query_res = static_cast<int64_t>(keys.Query(token.left_, token.right_));

  Saver::save(result, query_res);
}

template <typename Saver = parser::VoidSave, typename T>
void ExecuteGettingLessCounter(const Token& token,
                              std::vector<int64_t>& result,
                              dynamic_order::IntegerKeysSet<T>& keys) {

  int64_t count = static_cast<int64_t>(keys.LessKeysCounter(token.key_));

  Saver::save(result, count);
}

template <typename Saver = parser::VoidSave, typename T>
void ExecuteGettingMin(const Token& token,
                       std::vector<int64_t>& result,
                      dynamic_order::IntegerKeysSet<T>& keys){

  int64_t key = static_cast<int64_t>(keys.Min(token.index_));

  Saver::save(result, key);
}

export namespace parser {

template <typename Saver = parser::VoidSave, typename T>
std::vector<int64_t> RunRequests(const std::vector<Token>& tokens,
                                 dynamic_order::IntegerKeysSet<T>& keys) {
  if (tokens.empty()) {
    return {};
  }

  std::vector<int64_t> result;

  for (const auto& token: tokens) {
    switch(token.command_) {
      case 'k':
        ExecuteInsert(token, keys);
        break;
      case 'q':
        ExecuteQuery<Saver>(token, result, keys);
        break;
      case 'n':
        ExecuteGettingLessCounter<Saver>(token, result, keys);
        break;
      case 'm':
        ExecuteGettingMin<Saver>(token, result, keys);
        break;
      default:
        std::unreachable();
    }
  }

  return result;
}


std::vector<Token> Tokenize(std::istream& in) {
  // badbit  - system error
  in.exceptions(std::istream::badbit);

  char command = 0;
  bool flag = true;
  std::vector<Token> tokens;

  while (flag && in >> command) {
    switch (command) {
      case 'k':
       InsertKey(in, command, tokens);
       break;
      case 'q':
        GetQuery(in, command, tokens);
        break;
      case 'm':
        GetMin(in, command, tokens);
        break;
      case 'n':
        GetLessKeysCounter(in, command, tokens);
        break;
      default:
        flag = false;
        break;
    }
  }

  if (!in.eof()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of tokenizing data",
                                   "parser::Tokenize"};
    throw prog_error::ProgError(std::move(inf));
  }
  return tokens;
}

std::vector<Token> Tokenize(const std::string& file_name) {
  std::ifstream file_stream(file_name);

  if (!file_stream.is_open()) {
    prog_error::ErrorInfo inf = {prog_error::ErrorCode::kParseData,
                                   "Error of opening file",
                                   "parser::ReadData"};
    throw prog_error::ProgError(std::move(inf));
  }

  std::vector<Token> tokens = Tokenize(file_stream);

  return tokens;
}

template <typename T>
std::vector<int64_t> ExecuteRequests(const std::string& file_name,
                                     dynamic_order::IntegerKeysSet<T>& keys) {

  std::vector<Token> tokens = Tokenize(file_name);

  std::vector<int64_t> result = RunRequests<SaveResult>(tokens, keys);

  return result;
}

} // namespace parser




