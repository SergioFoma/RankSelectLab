module;

#include <exception>
#include <iostream>
#include <print>
#include <source_location>
#include <string>

export module prog_error;

export namespace prog_error {

enum class ErrorCode {
  kCorrect = 0,
  kOutOfRange = 1,
  kParseData = 2,
  kTests = 3,
  kConfigPaths = 4,
  kCommandFlags = 5
};

class ProgError : public std::runtime_error {
 public:
  explicit ProgError(const char* message, ErrorCode err_code)
      : std::runtime_error(message), err_code_{err_code} {}

  const ErrorCode& GetErrorCode() const noexcept { return err_code_; }

  void Log(const std::source_location location =
               std::source_location::current()) const {

    std::println(stderr, "file: {} ({} : {}) {}", location.file_name(),
                 location.line(), location.column(), location.function_name());
  }

 private:
  ErrorCode err_code_;
};
}  // namespace prog_error
