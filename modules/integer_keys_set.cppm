module;

#include <cstddef>
#include <type_traits>

export module integer_keys_set;

import std_set_impl;
import sorted_vector_impl;

export namespace dynamic_order {

template <typename T>
class IntegerKeysSet {
 public:
  IntegerKeysSet() {
    static_assert(!std::is_const<T>::value);
    static_assert(std::is_same<T, dynamic_order::StdSetImpl>::value ||
                  std::is_same<T, dynamic_order::SortedVectorImpl>::value);
  };

  void Insert(int key) {
    return algorithm_.Insert();
  }

  size_t Query(int left, int right) {
    return algorithm_.Query();
  }

  int Min(size_t index) {
    return algorithm_.Min(index);
  }

  size_t LessKeysCounter(int key) {
    return algorithm_.LessKeysCounter(key);
  }

 private:
  T algorithm_;
};

} // namespace dynamic_order
