#ifndef INTEGER_KEYS_SET_
#define INTEGER_KEYS_SET_

#include <cstddef>

namespace dynamic_order {

template <typename T>
class IntegerKeysSet {
  public:
    IntegerKeysSet() = default;

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





#endif
