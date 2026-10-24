module;

#include <vector>
#include <cstddef>

export module sorted_vector_impl;

export namespace dynamic_order {

class SortedVectorImpl {
 public:
  SortedVectorImpl() = default;

  explicit SortedVectorImpl(std::vector<int>&& keys);

  void Insert(int key);

  size_t Query(int left, int right);

  int Min(size_t index);

  size_t LessKeysCounter(int key);

  private:
    std::vector<int> sorted_keys_;
    std::vector<int> new_keys_;
    bool is_sorted_ = false;

    bool IsSorted() const;

    static void Sort(std::vector<int>& keys);

    void MergeVectors();

};
} // namespace dynamic_order
