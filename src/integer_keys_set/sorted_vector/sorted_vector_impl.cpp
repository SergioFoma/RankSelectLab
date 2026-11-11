module;

#include <algorithm>
#include <cstddef>
#include <memory>
#include <algorithm>
#include <flat_set>
#include <ranges>

module sorted_vector_impl;

import prog_error;

namespace {
bool IsEqual(int first_key, int second_key) {
  return !(first_key < second_key || second_key < first_key);
}
} // namespace

namespace dynamic_order {

SortedVectorImpl::SortedVectorImpl(std::sorted_unique_t sorted_unique,
                                   std::vector<int> keys)
  : keys_(std::move(keys)) {}

SortedVectorImpl::SortedVectorImpl(std::vector<int> keys)
    : keys_(std::move(keys)) {

  SortNewKeys();

  RemoveCopies();

  sort_violation_ind_ = keys_.size();
}

void dynamic_order::SortedVectorImpl::MergeVectors() {

  SortNewKeys();

  std::inplace_merge(keys_.begin(),
                     keys_.begin() + sort_violation_ind_,
                     keys_.end());

  RemoveCopies();

  sort_violation_ind_ = keys_.size();
}

void dynamic_order::SortedVectorImpl::Insert(int key) {
  bool is_empty = keys_.empty();
  size_t keys_sz = keys_.size();
  const int& keys_back = keys_.back();

  if (is_empty ||
      (sort_violation_ind_ == keys_sz && keys_back < key)) {
    ++sort_violation_ind_;
  }

  if (is_empty || !IsEqual(key,keys_back)) {
    keys_.push_back(key);
  }
}

size_t dynamic_order::SortedVectorImpl::Query(int left, int right) {

  if (right < left || IsEqual(left, right)) {
    return 0;
  }

  FixOrderViolation();

  auto lower_bound = std::ranges::upper_bound(keys_, left);
  if (lower_bound == keys_.end()) {
    return 0;
  }

  auto upper_bound = std::ranges::upper_bound(keys_, right);

  auto distance = std::distance(lower_bound, upper_bound);
  size_t res_distance = static_cast<size_t>(distance);

  return res_distance;
}

int dynamic_order::SortedVectorImpl::Min(size_t index) {

  if (index == 0 || index > keys_.size()) {
    throw prog_error::ProgError{"Incorrect index!",
                                prog_error::ErrorCode::kOutOfRange};
  }

  FixOrderViolation();

  return keys_[index - 1];
}

size_t dynamic_order::SortedVectorImpl::LessKeysCounter(int key) {

  FixOrderViolation();

  if (keys_.empty() || key <= keys_.front()) {
    return 0;
  }

  auto left = keys_.begin();
  auto right = std::ranges::lower_bound(keys_, key);

  auto distance = std::distance(left, right);
  size_t res_distance = static_cast<size_t>(distance);

  return res_distance;
}
} // namespace dynamic_order
