module;

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
#include <memory>

export module b_tree_impl;

namespace dynamic_order {

export constexpr size_t kMinDegree = 2;
constexpr size_t kRangeBegin = 0;

struct BTreeNode {
 public:
  struct NodeDeleter {
    void operator()(BTreeNode* node) {
      delete node;
    }
  };

  std::vector<int> keys;
  std::vector<std::unique_ptr<BTreeNode, NodeDeleter>> children;
  size_t subtree_sz = 0;
  size_t t;
  bool leaf;

  explicit BTreeNode(size_t min_degree, bool leaf = true)
      : t(min_degree), leaf(leaf) {
    assert(t >= kMinDegree);
  }

  BTreeNode(const BTreeNode& other) = delete;

  BTreeNode(BTreeNode&& other) noexcept
      : keys(std::move(other.keys)),
        children(std::move(other.children)),
        subtree_sz(std::exchange(other.subtree_sz, 0)),
        t(other.t),
        leaf(std::exchange(other.leaf, true)) {}

  void SplitChild(size_t child_index, BTreeNode* child);

  bool InsertNonFull(int key);

  void FixSubtreeSz();

  BTreeNode& operator=(const BTreeNode& other) = delete;

  BTreeNode& operator=(BTreeNode&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    keys = std::move(other.keys);
    children = std::move(other.children);
    subtree_sz = std::exchange(other.subtree_sz, 0);
    t = other.t;
    leaf = std::exchange(other.leaf, true);

    return *this;
  }

  ~BTreeNode() = default;

 private:

  // range [l, r)
  template <typename T>
  static std::vector<T> Move(std::vector<T>& arr, size_t left,
                             size_t right) {
    std::vector<T> copy(right - left);

    for (size_t ind = left; ind < right; ++ind) {
      copy[ind - left] = std::move(arr[ind]);
    }

    return copy;
  }

  size_t GetSubtreeSz() const;
};

export class BTreeImpl {
 public:
  explicit BTreeImpl(size_t t = kMinDegree)
    : root_{new BTreeNode(t)}, t_(t) {}

  BTreeImpl(const BTreeImpl& other);

  BTreeImpl(BTreeImpl&& other) noexcept = default;

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

  BTreeImpl& operator=(const BTreeImpl& other);

  BTreeImpl& operator=(BTreeImpl&& other) noexcept = default;

  ~BTreeImpl() = default;

 private:
  std::unique_ptr<BTreeNode, BTreeNode::NodeDeleter> root_;
  size_t t_;

  size_t FindLess(BTreeNode* node, int key) const;

  int FindMin(BTreeNode* node, size_t ind, size_t& current_ind) const;

  void RecursiveCopy(BTreeImpl& tree, const BTreeNode* node);
};
}  // namespace dynamic_order
