module;

#include <cstddef>
#include <cassert>
#include <algorithm>
#include <utility>

module avl_tree_impl;

import prog_error;

void dynamic_order::AvlTreeImpl::Insert(int key) {

  bool is_find = false;
  root_ = InsertNode(root_, key, is_find);
}

dynamic_order::Node* dynamic_order::AvlTreeImpl::InsertNode(
  dynamic_order::Node* parent,
  int key,
  bool& is_find
) {

  if (!parent){
    Node* new_node = new Node(key);
    nodes_ptr_.push_back(new_node);
    ++sz_;
    return new_node;
  }

  if (key == parent->key_){
    is_find = true;
    return parent;
  } else if (key < parent->key_) {
    parent->left_ = InsertNode(parent->left_, key, is_find);
  } else {
    parent->right_ = InsertNode(parent->right_, key, is_find);
  }

  if (!is_find) {
    return BalanceTree(parent);
  }
  return parent;
}

size_t dynamic_order::AvlTreeImpl::GetHeight(
  const dynamic_order::Node* node
) const {

  return (node ? node->height_ : kZeroHeight);
}

int dynamic_order::AvlTreeImpl::GetBalanceFactor(
  const dynamic_order::Node* parent
) const {

  int left_h = static_cast<int>(GetHeight(parent->left_));
  int right_h = static_cast<int>(GetHeight(parent->right_));

  return right_h - left_h;
}

void dynamic_order::AvlTreeImpl::FixHeight(
  dynamic_order::Node* node
) {

  assert(node);

  size_t left_h = GetHeight(node->left_);
  size_t right_h = GetHeight(node->right_);

  node->height_ = std::max(left_h, right_h) + 1;
}

size_t dynamic_order::AvlTreeImpl::GetSubtreeSz(
  dynamic_order::Node* node
) const {

  return (node ?
          node->left_subtree_sz_ + node->right_subtree_sz_ + 1:
          kZeroSubtreeSz);
}

void dynamic_order::AvlTreeImpl::FixSubtreeSz(
  dynamic_order::Node* node
) {

  assert(node);

  node->left_subtree_sz_ = GetSubtreeSz(node->left_);
  node->right_subtree_sz_ = GetSubtreeSz(node->right_);
}

size_t dynamic_order::AvlTreeImpl::Query(int left, int right) const {

  if (left >= right) {
    return 0;
  }

  size_t left_counter = 0;
  Node* left_less = FindLess(root_, left, left_counter);
  if (left_less && left_less->key_ == left) {
    ++left_counter;                         //take border itself
  }

  size_t right_counter = 0;
  Node* right_less = FindLess(root_, right, right_counter);
  if (right_less && right_less->key_ == right) {
    ++right_counter;                        // take border itself
  }

  return right_counter - left_counter;
}

int dynamic_order::AvlTreeImpl::Min(size_t index) const {

  if (index == 0 || index > sz_) {
    prog_error::ErrorInfo err_info = {prog_error::ErrorCode::kOutOfRange,
                                      "Incorrect index!",
                                      "dynamic_order::AvlTreeImpl::Min"};
    throw prog_error::ProgError(std::move(err_info));
  }

  size_t current_ind = 0;
  return FindMin(root_, index, current_ind);
}

int dynamic_order::AvlTreeImpl::FindMin(
  dynamic_order::Node* node,
  size_t ind,
  size_t& current_ind
) const {

  if (!node) {
    return 0;
  }

  size_t old_current_ind = current_ind;
  current_ind = current_ind + node->left_subtree_sz_ + 1;

  if (current_ind == ind) {
    return node->key_;
  }

  if (current_ind > ind) {
    current_ind = old_current_ind;
    return FindMin(node->left_, ind, current_ind);
  }

  return FindMin(node->right_, ind, current_ind);
}

size_t dynamic_order::AvlTreeImpl::LessKeysCounter(int key) const {

  size_t counter = 0;
  FindLess(root_, key, counter);

  return counter;
}

dynamic_order::Node* dynamic_order::AvlTreeImpl::FindLess(
  dynamic_order::Node* node,
  int key,
  size_t& counter
) const {

  if (!node) {
    return nullptr;
  }

  if (node->key_ == key) {
    counter += node->left_subtree_sz_;
    return node;
  }

  if (node->key_ < key) {
    counter += (node->left_subtree_sz_ + 1);
    return FindLess(node->right_, key, counter);
  }

  return FindLess(node->left_, key, counter);
}

dynamic_order::Node* dynamic_order::AvlTreeImpl::BalanceTree(
  dynamic_order::Node* node
) {

  assert(node);

  FixHeight(node);
  FixSubtreeSz(node);

  if (GetBalanceFactor(node) == kPositiveDelta) {
    if (GetBalanceFactor(node->right_) < 0) {
      node->right_ = SmallRightRotation(node->right_);
    }
    return SmallLeftRotation(node);
  }
  if (GetBalanceFactor(node) == kNegativeDelta) {
    if (GetBalanceFactor(node->left_) > 0) {
      node->left_ = SmallLeftRotation(node->left_);
    }
    return SmallRightRotation(node);
  }

  return node;
}

/*
      p                          q
     / \                        / \
    a   q          —>          p   c
       / \                    / \
      b   c                  a   b
*/

dynamic_order::Node* dynamic_order::AvlTreeImpl::SmallLeftRotation(
  dynamic_order::Node* p
) {

  Node* q = p->right_;
  Node* b = q->left_;

  q->left_ = p;
  p->right_ = b;

  FixHeight(p);
  FixSubtreeSz(p);

  FixHeight(q);
  FixSubtreeSz(q);

  return q;
}

/*
        p                        q
       / \                      / \
      q   c          —>        a   p
     / \                          / \
    a   b                        b   c
*/

dynamic_order::Node* dynamic_order::AvlTreeImpl::SmallRightRotation(
  dynamic_order::Node* p
) {

  assert(p);

  Node* q = p->left_;
  Node* b = q->right_;

  q->right_ = p;
  p->left_ = b;

  FixHeight(p);
  FixSubtreeSz(p);

  FixHeight(q);
  FixSubtreeSz(q);

  return q;
}

void dynamic_order::AvlTreeImpl::DropTree() {
  for (auto* node_ptr: nodes_ptr_) {
    delete node_ptr;
  }
}
