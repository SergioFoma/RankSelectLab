module;

#include <cstddef>
#include <vector>
#include <utility>

export module avl_tree_impl;

export namespace dynamic_order {

struct Node {
  Node* left_ = nullptr;
  Node* right_ = nullptr;
  size_t left_subtree_sz_ = 0;
  size_t right_subtree_sz_ = 0;
  size_t height_;
  int key_;

  explicit Node(int key = 0, size_t height = 1)
    : height_(height), key_(key) {}
};

class AvlTreeImpl {
 public:
  AvlTreeImpl() = default;

  void Insert(int key);

  size_t Query(int left, int right) const;

  int Min(size_t index) const;

  size_t LessKeysCounter(int key) const;

  ~AvlTreeImpl() {
    for (auto* node_ptr: nodes_ptr_) {
      delete node_ptr;
    }
  }
 private:
  Node* root_ = nullptr;
  std::vector<Node*> nodes_ptr_;
  size_t sz_ = 0;

  static constexpr size_t kZeroHeight = 0;
  static constexpr size_t kZeroSubtreeSz = 0;
  static constexpr int kPositiveDelta = 2;
  static constexpr int kNegativeDelta = -2;

  Node* InsertNode(Node* parent, int key, bool& is_find);

  int FindMin(Node* node, size_t ind, size_t& current_ind) const;

  Node* FindLess(Node* node, int key, size_t& counter) const;

  size_t GetHeight(const Node* node) const;

  int GetBalanceFactor(const Node* parent) const;

  void FixHeight(Node* node);

  size_t GetSubtreeSz(Node* node) const;

  void FixSubtreeSz(Node* node);

  Node* BalanceTree(Node* node);

  Node* SmallLeftRotation(Node* p);
  Node* SmallRightRotation(Node* p);

};
} // namespace dynamic_order;
