#ifndef STD_SET_IMPL_
#define STD_SET_IMPL_

#include <set>
#include <cstddef>

namespace dynamic_order {

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

#endif
