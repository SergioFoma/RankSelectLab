module;

#include <cstddef>
#include <type_traits>

export module integer_keys_set;

export import std_set_impl;
export import sorted_vector_impl;
export import avl_tree_impl;
export import b_tree_impl;
export import prog_error;

export namespace dynamic_order {

template <typename T>
class IntegerKeysSet {
 public:
  IntegerKeysSet() {
    static_assert(!std::is_const<T>::value);
    static_assert(std::is_same<T, dynamic_order::StdSetImpl>::value ||
                  std::is_same<T, dynamic_order::SortedVectorImpl>::value ||
                  std::is_same<T, dynamic_order::AvlTreeImpl>::value);
  };

  void Insert(int key) {
    return algorithm_.Insert(key);
  }

  size_t Query(int left, int right) {
    return algorithm_.Query(left, right);
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

template <>
class IntegerKeysSet<dynamic_order::BTreeImpl> {
 public:
  explicit IntegerKeysSet(size_t min_degree = dynamic_order::kMinDegree)
    : algorithm_(min_degree) {}

  void Insert(int key) {
    return algorithm_.Insert(key);
  }

  size_t Query(int left, int right) {
    return algorithm_.Query(left, right);
  }

  int Min(size_t index) {
    return algorithm_.Min(index);
  }

  size_t LessKeysCounter(int key) {
    return algorithm_.LessKeysCounter(key);
  }

 private:
  dynamic_order::BTreeImpl algorithm_;
};

} // namespace dynamic_order
