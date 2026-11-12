module;

#include <cstddef>
#include <memory>
#include <utility>
#include <iterator>
#include <ranges>

module std_set_impl;

import prog_error;

namespace {
bool IsEqual(int first_key, int second_key) {
  return !(first_key < second_key || second_key < first_key);
}
} // namespace

namespace dynamic_order {

void StdSetImpl::Insert(int key) {
  keys_.insert(key);
}

size_t StdSetImpl::Query(int left, int right) const {

  if (right < left || IsEqual(left, right)) {
    return 0;
  }

  auto lower_bound = keys_.upper_bound(left);
  auto upper_bound = keys_.upper_bound(right);

  size_t counter = static_cast<size_t>(std::distance(lower_bound, upper_bound));
  return counter;
}

int StdSetImpl::Min(size_t index) const {

  if (index == 0 || index > keys_.size()) {
    throw prog_error::ProgError{"Incorrect index!",
                                prog_error::ErrorCode::kOutOfRange};
  }

  /*
  * std::views - lazy arithmetic
  * std::views::zip - synchronous passage of n cycles
  * std::views::iota - range generator
  * std::filter - function, that call predicate
      for (....) {
        filter(iota)    -- pipline
      }

      filter argument is view::zip pair
  */

  auto view = std::views::zip(std::views::iota(0), keys_) | std::views::filter([&](const auto& tup) {
    return (std::get<0>(tup) + 1 == index);
  });

  return std::get<1>(*view.begin());
}

size_t StdSetImpl::LessKeysCounter(int key) const {

  size_t counter = static_cast<size_t>(std::distance(keys_.begin(), keys_.lower_bound(key)));

  return counter;
}
} // namespace dynamic_order
