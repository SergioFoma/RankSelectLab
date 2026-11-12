module;

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

export module avl_tree_impl;

namespace dynamic_order {

export struct Node {
  Node* left = nullptr;
  Node* right = nullptr;
  size_t left_subtree_sz = 0;
  size_t right_subtree_sz = 0;
  size_t height;
  int key;

  struct NodeDeleter {
    void operator()(Node* node) { delete node; }
  };

  static constexpr int kPositiveDelta = 2;
  static constexpr int kNegativeDelta = -2;

  explicit Node(int key, size_t height = 1) : height(height), key(key) {}

  explicit Node(const Node& other) = delete;

  explicit Node(Node&& other) noexcept
      : left{std::exchange(other.left, nullptr)},
        right{std::exchange(other.right, nullptr)},
        left_subtree_sz{std::exchange(other.left_subtree_sz, 0)},
        right_subtree_sz{std::exchange(other.right_subtree_sz, 0)},
        height{std::exchange(other.height, 0)},
        key{other.key} {}

  Node& operator=(const Node& other) = delete;

  Node& operator=(Node&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    delete left;
    delete right;

    left = std::exchange(other.left, nullptr);
    right = std::exchange(other.right, nullptr);
    left_subtree_sz = std::exchange(other.left_subtree_sz, 0);
    right_subtree_sz = std::exchange(other.right_subtree_sz, 0);
    height = std::exchange(other.height, 0);
    key = other.key;

    return *this;
  }

  void FixHeight();

  void FixSubtreeSz();

  int GetBalanceFactor() const;

  Node* BalanceNode();

  Node* SmallLeftRotation();

  Node* SmallRightRotation();

  static size_t GetHeight(const Node* node);

  static size_t GetSubtreeSz(const Node* node);
};

export class AvlTreeImpl {
 public:
  AvlTreeImpl() = default;

  AvlTreeImpl(const AvlTreeImpl& other);

  AvlTreeImpl(AvlTreeImpl&& other) noexcept
      : root_(std::exchange(other.root_, nullptr)),
        sz_(std::exchange(other.sz_, 0)),
        nodes_ptr_(std::move(other.nodes_ptr_)) {}

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

  AvlTreeImpl& operator=(const AvlTreeImpl& other);

  AvlTreeImpl& operator=(AvlTreeImpl&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    root_ = std::exchange(other.root_, nullptr);
    sz_ = std::exchange(other.sz_, 0);
    nodes_ptr_ = std::move(other.nodes_ptr_);

    return *this;
  }

  ~AvlTreeImpl() = default;

 private:
  Node* root_ = nullptr;
  size_t sz_ = 0;
  std::vector<std::unique_ptr<Node, Node::NodeDeleter>> nodes_ptr_;

  static constexpr bool kNotFind = false;
  static constexpr bool kFind = true;

  std::pair<Node*, bool> InsertNode(Node* parent, int key);

  int FindMin(Node* node, size_t ind, size_t& current_ind) const;

  std::pair<Node*, size_t> FindLess(Node* node, int key) const;

  void RecursiveCopy(AvlTreeImpl& tree, const Node* node);
};
}  // namespace dynamic_order
