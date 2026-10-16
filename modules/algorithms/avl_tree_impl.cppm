module;

#include <cstddef>
#include <vector>
#include <utility>

export module avl_tree_impl;

export namespace dynamic_order {

enum class InsertStatus {
  kFound,
  kInserted,
  kBalanced
};

struct Node {
  Node* left_ = nullptr;
  Node* right_ = nullptr;
  size_t height_ = 0;
  size_t subtree_size_ = 0;
  int key_;

  explicit Node(int key = 0, size_t height = 0)
    : height_(height), key_(key) {}
};

using search_pair = std::pair<Node*, InsertStatus>;

class AvlTreeImpl {
 public:
  AvlTreeImpl() = default;

  void Insert(int key);

  //size_t Query(int left, int right);

  //int Min(size_t index);

  //size_t LessKeysCounter(int key);

  ~AvlTreeImpl() {
    for (auto* node_ptr: nodes_ptr_) {
      delete node_ptr;
    }
  }
 private:
  Node* root_ = nullptr;
  std::vector<Node*> nodes_ptr_;
  size_t size_ = 0;

  static constexpr bool kLeft = true;
  static constexpr bool kRight = false;

  search_pair FindAndInsert(Node* current_node,
                           int key, size_t parent_height);

  dynamic_order::search_pair
  HeightChecking(search_pair& search_pair, Node* parent, bool node_type);

  size_t ReceiveHeight(Node* node);

  void BalanceTree(Node* perent, Node* left, Node* right, bool node_type);

  void SmallLeftRotation(Node* parent, Node* left, Node* right);
  void BigLeftRotation(Node* parent, Node* left, Node* right);

  void SmallRightRotation(Node* parent, Node* left, Node* right);
  void BigRightRotation(Node* parent, Node* left, Node* right);
};
} // namespace dynamic_order
