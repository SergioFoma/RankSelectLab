module;

#include <set>
#include <cstddef>

export module std_set_impl;

export namespace dynamic_order {

class StdSetImpl {
 public:
  StdSetImpl() = default;

  explicit StdSetImpl(std::set<int>&& keys)
    : keys_(std::move(keys)) {}

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

 private:
  std::set<int> keys_;
};
} // namespace dynamic_order

