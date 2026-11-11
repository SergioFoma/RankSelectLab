module;

#include <cstddef>
#include <set>

export module std_set_impl;

namespace dynamic_order {

export class StdSetImpl {
 public:
  StdSetImpl() = default;

  explicit StdSetImpl(std::set<int>&& keys) : keys_(std::move(keys)) {}

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

 private:
  std::set<int> keys_;
};
}  // namespace dynamic_order
