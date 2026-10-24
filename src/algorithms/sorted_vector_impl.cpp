module;

#include <cstddef>
#include <algorithm>
#include <memory>

#include <tbb/parallel_sort.h>

module sorted_vector_impl;

import prog_error;

dynamic_order::SortedVectorImpl::SortedVectorImpl(std::vector<int>&& keys)
   : sorted_keys_(std::move(keys)) {
    if (!is_sorted_) {
      Sort(sorted_keys_);
      is_sorted_ = true;
    }

    auto new_end = std::unique(sorted_keys_.begin(), sorted_keys_.end());
    sorted_keys_.resize(new_end - sorted_keys_.begin());
}

bool dynamic_order::SortedVectorImpl::IsSorted() const {
  return is_sorted_;
}

void dynamic_order::SortedVectorImpl::Sort(std::vector<int>& keys) {
  tbb::parallel_sort(keys.begin(), keys.end());
}

void dynamic_order::SortedVectorImpl::MergeVectors() {

  Sort(new_keys_);

  auto new_end = std::unique(new_keys_.begin(), new_keys_.end());  // removing
  new_keys_.resize(new_end - new_keys_.begin());                   // duplicates

  std::vector<int> merged_keys(sorted_keys_.size() + new_keys_.size());

  new_end = std::set_union(sorted_keys_.begin(), sorted_keys_.end(),
             new_keys_.begin(), new_keys_.end(),
             merged_keys.begin());

  merged_keys.resize(new_end - merged_keys.begin());

  new_keys_.clear();
  sorted_keys_ = std::move(merged_keys);
  is_sorted_ = true;
}

void dynamic_order::SortedVectorImpl::Insert(int key) {
  new_keys_.push_back(key);
  is_sorted_ = false;
}

size_t dynamic_order::SortedVectorImpl::Query(int left, int right) {
  if (!IsSorted()) {
    MergeVectors();
  }

  if (left >= right) {
    return 0;
  }

  auto lower_bound = std::upper_bound(sorted_keys_.begin(),
                                      sorted_keys_.end(), left);
  auto upper_bound = std::upper_bound(sorted_keys_.begin(),
                                      sorted_keys_.end(), right);

  if (lower_bound == sorted_keys_.end()) {
    return 0;
  }

  auto distance = std::abs(std::distance(lower_bound, upper_bound));
  size_t res_distance = static_cast<size_t>(distance);

  return res_distance;
}

int dynamic_order::SortedVectorImpl::Min(size_t index) {

  if (!IsSorted()) {
    MergeVectors();
  }

  if (index == 0 || index > sorted_keys_.size()) {
    prog_error::ErrorInfo err_info = {prog_error::ErrorCode::kOutOfRange,
                                      "Incorrect index!",
                                      "dynamic_order::SortedVectorImpl::Min"};
    throw prog_error::ProgError(std::move(err_info));
  }

  return sorted_keys_[index - 1];
}

size_t dynamic_order::SortedVectorImpl::LessKeysCounter(int key) {

  if (!IsSorted()) {
    MergeVectors();
  }

  if (sorted_keys_.empty() || key <= sorted_keys_.front()) {
    return 0;
  }

  auto left = sorted_keys_.begin();

  auto right = std::lower_bound(sorted_keys_.begin(), sorted_keys_.end(), key);

  auto distance = std::abs(std::distance(left, right));
  size_t res_distance = static_cast<size_t>(distance);

  return res_distance;
}
