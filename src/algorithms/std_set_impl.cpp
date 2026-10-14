#include <utility>

#include "algorithms/std_set_impl.hpp"
#include "prog_error.hpp"

void dynamic_order::StdSetImpl::Insert(int key) {
  keys_.insert(key);
}

size_t dynamic_order::StdSetImpl::Query(int left, int right) const {

  size_t counter = 0;
  auto lower_bound = keys_.upper_bound(left);

  for (auto it = lower_bound; it != keys_.end() && *it <= right; ++it) {
    ++counter;
  }

  return counter;
}

int dynamic_order::StdSetImpl::Min(size_t index) const {

  if (index == 0 || index > keys_.size()) {
    prog_error::ErrorInfo err_info = {prog_error::ErrorCode::kOutOfRange,
                                      "Incorrect index!",
                                      "dynamic_order::StdSetImpl"};
    throw prog_error::ProgError(std::move(err_info));
  }

  size_t counter = 1;
  for (const int& key: keys_) {
    if (index == counter) {
      return key;
    }
    ++counter;
  }

  std::unreachable();
}

size_t dynamic_order::StdSetImpl::LessKeysCounter(int key) const {

  size_t counter = 0;

  for (const auto& elem: keys_) {
    if (elem >= key) {
      break;
    }
    ++counter;
  }

  return counter;
}
