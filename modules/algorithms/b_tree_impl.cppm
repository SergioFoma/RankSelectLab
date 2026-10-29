module;

#include <cstddef>
#include <vector>
#include <cassert>
#include <utility>

export module b_tree_impl;

export namespace dynamic_order {

constexpr size_t kMinDegree = 2;
constexpr size_t kRangeBegin = 0;

class BTreeNode {
 public:
  std::vector<int> keys_;
  std::vector<BTreeNode*> children_;
  size_t subtree_sz_ = 0;
  size_t t_;
  bool leaf_;

  explicit BTreeNode(size_t min_degree, bool leaf = true)
            : t_(min_degree), leaf_(leaf){
    assert(t_ >= kMinDegree);
  }

  BTreeNode(const BTreeNode& other) = delete;

  BTreeNode(BTreeNode&& other) noexcept
    : keys_(std::move(other.keys_)),
      children_(std::move(other.children_)),
      subtree_sz_(other.subtree_sz_),
      t_(other.t_),
      leaf_(other.leaf_) {

    other.subtree_sz_ = 0;
    other.leaf_ = true;
  }

  void SplitChild(size_t child_index, BTreeNode* child);

  bool InsertNonFull(int key);

  void FixSubtreeSz();

  BTreeNode& operator=(const BTreeNode& other) = delete;

  BTreeNode& operator=(BTreeNode&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    DropChildren();
    keys_ = std::move(other.keys_);
    children_ = std::move(other.children_);
    subtree_sz_ = other.subtree_sz_;
    t_ = other.t_;
    leaf_ = other.leaf_;

    other.subtree_sz_ = 0;
    other.leaf_ = true;

    return *this;
  }

  ~BTreeNode() {
    DropChildren();
  }

 private:
  void DropChildren();

  // range [l, r)
  template<typename T>
  static std::vector<T> Copy(const std::vector<T>& keys,
                        size_t left, size_t right) {
    std::vector<T> copy(right - left);

    for (size_t ind = left; ind < right; ++ind) {
      copy[ind - left] = keys[ind];
    }

    return copy;
  }

  static constexpr size_t kZeroSubtreeSz = 0;

  size_t GetSubtreeSz() const;
};

class BTreeImpl {
 public:
  explicit BTreeImpl(size_t t = kMinDegree)
    : root_(new BTreeNode(t)), t_(t) {}

  BTreeImpl(const BTreeImpl& other) = delete;

  BTreeImpl(BTreeImpl&& other) noexcept
    : root_(other.root_), t_(other.t_) {
      other.root_ = nullptr;
  }

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

  BTreeImpl& operator=(const BTreeImpl& other) = delete;

  BTreeImpl& operator=(BTreeImpl&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    delete root_;
    root_ = other.root_;
    t_ = other.t_;
    other.root_ = nullptr;

    return *this;
  }

  ~BTreeImpl() {
    delete root_;
  }

 private:
  BTreeNode* root_;
  size_t t_;

  void FindLess(BTreeNode* node, int key, size_t& counter) const;

  int FindMin(BTreeNode* node, size_t ind,
              size_t& current_ind) const;
};
} // namespace dynamic_order
