module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <exception>
#include <memory>

module avl_tree_impl;

import prog_error;

namespace {
bool IsEqual(int first_key, int second_key) {
  return !(first_key < second_key || second_key < first_key);
}
} // namespace

namespace dynamic_order {

//strong exception guarantee

AvlTreeImpl::AvlTreeImpl(const AvlTreeImpl& other) {

  AvlTreeImpl new_tree;

  RecursiveCopy(new_tree, other.root_);

  *this = std::move(new_tree);
}

AvlTreeImpl& AvlTreeImpl::operator=(const AvlTreeImpl& other) {
  if (this == &other) {
    return *this;
  }

  AvlTreeImpl new_tree;

  RecursiveCopy(new_tree, other.root_);

  *this = std::move(new_tree);

  return *this;
}


void AvlTreeImpl::RecursiveCopy(AvlTreeImpl& tree, const Node* node) {

  if (!node) {
    return;
  }

  tree.Insert(node->key);

  RecursiveCopy(tree, node->left);
  RecursiveCopy(tree, node->right);
}

void AvlTreeImpl::Insert(int key) {

  std::pair<Node*, bool> insert_status = InsertNode(root_, key);
  root_ = insert_status.first;
}

std::pair<Node*, bool> AvlTreeImpl::InsertNode(Node* parent, int key) {

  if (!parent) {
    std::unique_ptr<Node, Node::NodeDeleter> new_node{new Node{key}};
    nodes_ptr_.push_back(std::move(new_node));
    Node* raw_ptr = nodes_ptr_.back().get();
    ++sz_;
    return {raw_ptr, kNotFind};
  }

  std::pair<Node*, bool> insert_status;
  if (IsEqual(key, parent->key)) {
    return {parent, kFind};
  } else if (key < parent->key) {
    insert_status = InsertNode(parent->left, key);
    parent->left = insert_status.first;
  } else {
    insert_status = InsertNode(parent->right, key);
    parent->right = insert_status.first;
  }

  if (!insert_status.second) {
    return {parent->BalanceNode(), insert_status.second};
  }
  return {parent, insert_status.second};
}

size_t Node::GetHeight(const Node* node) {

  return (node ? node->height : 0);
}

int Node::GetBalanceFactor() const {

  int left_h = static_cast<int>(GetHeight(left));
  int right_h = static_cast<int>(GetHeight(right));

  return right_h - left_h;
}

void Node::FixHeight() {

  size_t left_h = GetHeight(left);
  size_t right_h = GetHeight(right);

  height = std::max(left_h, right_h) + 1;
}

size_t Node::GetSubtreeSz(const Node* node) {

  return (node ? node->left_subtree_sz + node->right_subtree_sz + 1
               : 0);
}

void Node::FixSubtreeSz() {

  left_subtree_sz = GetSubtreeSz(left);
  right_subtree_sz = GetSubtreeSz(right);
}

size_t AvlTreeImpl::Query(int left, int right) const {

  if (!root_ || (right < left || IsEqual(left, right))) {
    return 0;
  }

  std::pair<Node*, size_t> left_res = FindLess(root_, left);
  if (left_res.first && IsEqual(left_res.first->key, left)) {
    ++left_res.second;  //take border itself
  }

  std::pair<Node*, size_t> right_res = FindLess(root_, right);
  if (right_res.first && IsEqual(right_res.first->key, right)) {
    ++right_res.second;  // take border itself
  }

  return right_res.second - left_res.second;
}

int AvlTreeImpl::Min(size_t index) const {

  if (index == 0 || sz_ < index) {
    throw prog_error::ProgError{"Incorrect index!",
                                prog_error::ErrorCode::kOutOfRange};
  }

  size_t current_ind = 0;
  return FindMin(root_, index, current_ind);
}

int AvlTreeImpl::FindMin(Node* node, size_t ind, size_t& current_ind) const {

  if (!node) {
    return 0;
  }

  size_t old_current_ind = current_ind;

  current_ind += (node->left_subtree_sz + 1);

  if (current_ind == ind) {
    return node->key;
  }

  if (current_ind > ind) {
    current_ind = old_current_ind;
    return FindMin(node->left, ind, current_ind);
  }

  return FindMin(node->right, ind, current_ind);
}

size_t AvlTreeImpl::LessKeysCounter(int key) const {

  std::pair<Node*, size_t> res = FindLess(root_, key);

  return res.second;
}

std::pair<Node*, size_t> AvlTreeImpl::FindLess(
    dynamic_order::Node* node, int key) const {

  if (!node) {
    return {nullptr, 0};
  }

  if (key < node->key) {
    return FindLess(node->left, key);
  }

  if (IsEqual(node->key, key)) {
    return {node, node->left_subtree_sz};
  }

  if (node->key < key) {
    std::pair<Node*, size_t> right_res = FindLess(node->right, key);
    return {right_res.first, node->left_subtree_sz + right_res.second + 1};
  }

  std::unreachable();
}

Node* Node::BalanceNode() {

  FixHeight();
  FixSubtreeSz();

  switch (GetBalanceFactor()) {
    case kPositiveDelta: {
      if (right->GetBalanceFactor() < 0) {
        right = right->SmallRightRotation();
      }
      return SmallLeftRotation();
    }
    case kNegativeDelta: {
      if (left->GetBalanceFactor() > 0) {
        left = left->SmallLeftRotation();
      }
      return SmallRightRotation();
    }
    default:
      break;
  }

  return this;
}

/*
    this                         q
     / \                        / \
    a   q          —>         this   c
       / \                    / \
      b   c                  a   b
*/

Node* Node::SmallLeftRotation() {

  Node* q = right;
  Node* b = q->left;

  q->left = this;
  right = b;

  FixHeight();
  FixSubtreeSz();

  q->FixHeight();
  q->FixSubtreeSz();

  return q;
}

/*
      this                       q
       / \                      / \
      q   c          —>        a  this
     / \                          / \
    a   b                        b   c
*/

Node* Node::SmallRightRotation() {

  Node* q = left;
  Node* b = q->right;

  q->right = this;
  left = b;

  FixHeight();
  FixSubtreeSz();

  q->FixHeight();
  q->FixSubtreeSz();

  return q;
}
} // namespace order
