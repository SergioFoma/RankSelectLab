module;

#include <cstddef>
#include <vector>
#include <flat_set>

export module sorted_vector_impl;

namespace dynamic_order {

export class SortedVectorImpl {
 public:
  SortedVectorImpl() = default;

  explicit SortedVectorImpl(std::vector<int> keys);

  explicit SortedVectorImpl(std::sorted_unique_t sorted_unique,
                            std::vector<int> keys);

  void Insert(int key);

  size_t Query(int left, int right);

  int Min(size_t index);

  size_t LessKeysCounter(int key);

 private:
  std::vector<int> keys_;
  size_t sort_violation_ind_ = 0;

  void FixOrderViolation() {
    if (sort_violation_ind_ < keys_.size()) {
      MergeVectors();
    }
  }

 void SortNewKeys();

  void RemoveCopies() {
    auto new_end = std::ranges::unique(keys_).begin();
    keys_.resize(static_cast<size_t>(new_end - keys_.begin()));
  }

  void MergeVectors();
};
}  // namespace dynamic_order
