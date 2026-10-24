module;

#include <exception>
#include <string>

export module prog_error;

export namespace prog_error {

enum class ErrorCode {
  kCorrect      = 0,
  kOutOfRange   = 1,
  kParseData    = 2,
  kTests        = 3
};

struct ErrorInfo {
  ErrorCode code;
  std::string err_msg;
  std::string func_name;
};

class ProgError : public std::exception {
 public:
  explicit ProgError(const ErrorInfo& error_info)
    : error_info_(error_info) {}

  explicit ProgError(ErrorInfo&& error_info)
    : error_info_(std::move(error_info)) {}

  const ErrorInfo& GetErrorInfo() const noexcept {
    return error_info_;
  }

  const char* what() const noexcept override {
    return error_info_.err_msg.c_str();
  }

 private:
  ErrorInfo error_info_;
};
} // namespace prog_except
