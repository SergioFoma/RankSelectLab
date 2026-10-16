module;

#include <cstddef>
#include <cassert>
#include <algorithm>

module avl_tree_impl;

import prog_error;

void dynamic_order::AvlTreeImpl::Insert(int key) {

  if (size_ == 0) {
    root_ = new Node(key);
    nodes_ptr_.push_back(root_);
    ++size_;
    return ;
  }

  Node* current_node = root_;
  if (key < current_node->key_) {
    FindAndInsert(current_node->left_, key, current_node->height_);
  } else if (key > current_node->key_) {
    FindAndInsert(current_node->right_, key, current_node->height_);
  }
}

//size_t dynamic_order::AvlTreeImpl::Query(int left, int right);

//int dynamic_order::AvlTreeImpl::Min(size_t index);

//size_t dynamic_order::AvlTreeImpl::LessKeysCounter(int key);

dynamic_order::search_pair
dynamic_order::AvlTreeImpl::FindAndInsert(dynamic_order::Node* current_node,
                                                 int key, size_t parent_height) {
  if (current_node != nullptr && current_node->key_ == key) {
    return {nullptr, dynamic_order::InsertStatus::kFound};
  }

  if (current_node == nullptr) {
    size_t child_height = parent_height + 1;
    Node* child_node = new Node(key, child_height);
    nodes_ptr_.push_back(child_node);
    ++size_;
    return {child_node, dynamic_order::InsertStatus::kInserted};
  }

  if (key < current_node->key_) {
    dynamic_order::search_pair left_child = FindAndInsert(current_node->left_,
                                                          key, current_node->height_);
    return HeightChecking(left_child, current_node, kLeft);
  }

  dynamic_order::search_pair right_child = FindAndInsert(current_node->right_,
                                                         key, current_node->height_);
  return HeightChecking(right_child, current_node, kRight);
}

dynamic_order::search_pair
dynamic_order::AvlTreeImpl::HeightChecking(dynamic_order::search_pair& pair,
                                           dynamic_order::Node* parent,
                                           bool node_type) {
  if (pair.second != dynamic_order::InsertStatus::kInserted) {
    return pair;
  }
  if (kLeft) {
    size_t right_height = std::max(ReceiveHeight(parent->right_),
                                   parent->height_);
    size_t delta = (pair.first->height_ > right_height ?
                    pair.first->height_ - right_height :
                    right_height - pair.first->height_);
    if (delta > 1) {
      BalanceTree(parent, pair.first, parent->right_, kLeft);
    }

    pair.second = dynamic_order::InsertStatus::kBalanced;
    return pair;
  }

  size_t left_height = std::max(ReceiveHeight(parent->left_),
                                  parent->height_);
  size_t delta = (pair.first->height_ > left_height ?
                  pair.first->height_ - left_height :
                  left_height - pair.first->height_);
  if (delta > 1) {
    BalanceTree(parent, parent->left_, pair.first, kRight);
  }

  pair.second = dynamic_order::InsertStatus::kBalanced;
  return pair;
}

size_t dynamic_order::AvlTreeImpl::ReceiveHeight(dynamic_order::Node* node) {
  if (node == nullptr) {
    return 0;
  }

  size_t left_height = 0, right_height = 0;
  if (node->left_ != nullptr) {
    left_height = ReceiveHeight(node->left_);
  }
  if (node->right_ != nullptr) {
    right_height = ReceiveHeight(node->right_);
  }

  return std::max(left_height, right_height);
}

void dynamic_order::AvlTreeImpl::BalanceTree(dynamic_order::Node* parent,
                                                 dynamic_order::Node* left,
                                                 dynamic_order::Node* right,
                                                 bool node_type) {
  assert(parent);
  assert(left);
  assert(right);

  if (node_type == kLeft) {
    size_t right_left_h = std::max(ReceiveHeight(right->left_), right->height_);
    size_t right_right_h = std::max(ReceiveHeight(right->right_), right->height_);

    if (right_left_h <= right_right_h) {
      SmallLeftRotation(parent, left, right);
    } else {
      BigLeftRotation(parent, left, right);
    }
  } else {
    size_t left_left_h = std::max(ReceiveHeight(left->left_), left->height_);
    size_t left_right_h = std::max(ReceiveHeight(left->right_), left->height_);

    if (left_right_h <= left_left_h) {
      SmallRightRotation(parent, left, right);
    } else {
      BigRightRotation(parent, left, right);
    }
  }
}

/*
      a                          b
     / \                        / \
    L   b          —>          a   R
       / \                    / \
      C   R                  L   C
*/

void dynamic_order::AvlTreeImpl::SmallLeftRotation(dynamic_order::Node* a,
                                                   dynamic_order::Node* l,
                                                   dynamic_order::Node* b) {
  Node* c = b->left_;

  b->left_ = a;
  a->right_ = c;
}

/*
        a                                    c
       / \                                 /   \
      L   b                               a     b
         / \              —>             / \   / \
        c   R                           L   M N   R
       / \
      M   N

*/
void dynamic_order::AvlTreeImpl::BigLeftRotation(dynamic_order::Node* a,
                                                 dynamic_order::Node* l,
                                                 dynamic_order::Node* b) {
  Node* c = b->left_;
  Node* m = c->left_;
  Node* n = c->right_;

  a->right_ = m;
  c->left_ = a;
  c->right_ = b;
  b->left_ = n;
}

/*
        a                        b
       / \                      / \
      b   R          —>        L   a
     / \                          / \
    L   C                        C   R
*/

void dynamic_order::AvlTreeImpl::SmallRightRotation(dynamic_order::Node* a,
                                                    dynamic_order::Node* l,
                                                    dynamic_order::Node* b) {
  Node* c = b->right_;

  b->right_ = a;
  a->left_ = c;
}

/*
        a                                    c
       / \                                 /   \
      b   R                               b     a
     / \                  —>             / \   / \
    L   c                               L   M N   R
       / \
      M   N
*/

void dynamic_order::AvlTreeImpl::BigRightRotation(dynamic_order::Node* a,
                                                  dynamic_order::Node* l,
                                                  dynamic_order::Node* b) {
  Node* c = b->right_;
  Node* n = c->right_;
  Node* m = c->left_;

  b->right_ = m;
  c->left_ = b;
  c->right_ = a;
  a->left_ = n;
}
