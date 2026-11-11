module;

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <memory>
#include <utility>
#include <ranges>

module b_tree_impl;

import prog_error;

namespace {
bool IsEqual(int first_key, int second_key) {
  return !(first_key < second_key || second_key < first_key);
}
} // namespace

namespace dynamic_order {

void BTreeNode::SplitChild(size_t child_index, BTreeNode* child) {

  assert(child);
  assert(children[child_index].get() == child);

  std::unique_ptr<BTreeNode, NodeDeleter> new_node{new BTreeNode{child->t, child->leaf}};
  size_t t = child->t;

  new_node->keys = Move(child->keys, t - 1, child->keys.size());
  child->keys = Move(child->keys, kRangeBegin, t - 1);

  if (!child->leaf) {
    new_node->children = Move(child->children, t, child->children.size());
    child->children = Move(child->children, kRangeBegin, t);
  }

  children.insert(children.begin() + child_index + 1, std::move(new_node));

  BTreeNode* raw_new_node = children[child_index + 1].get();

  keys.insert(keys.begin() + child_index, raw_new_node->keys.front());
  raw_new_node->keys.erase(raw_new_node->keys.begin());

  raw_new_node->FixSubtreeSz();
  child->FixSubtreeSz();
  FixSubtreeSz();
}

bool BTreeNode::InsertNonFull(int key) {
  int insert_pos = static_cast<int>(
    std::distance(keys.begin(), std::ranges::upper_bound(keys, key))
  );

  if (insert_pos > 0 && IsEqual(key,keys[insert_pos - 1])) {
    return false;
  }

  if (leaf) {
    keys.insert(keys.begin() + insert_pos, key);
    ++subtree_sz;
    return true;
  }

  if (children[insert_pos]->keys.size() == (2 * t - 1)) {
    SplitChild(insert_pos, children[insert_pos].get());

    if (IsEqual(key, keys[insert_pos])) {
      return false;
    } else if (keys[insert_pos] < key) {
      ++insert_pos;
    }
  }

  if (!children[insert_pos]->InsertNonFull(key)) {
    return false;
  }

  ++subtree_sz;
  return true;
}

size_t BTreeNode::GetSubtreeSz() const {

  size_t counter = 0;

  for (auto& child : children) {
    if (child) {
      counter += child->subtree_sz;
    }
  }

  return counter + keys.size();
}

void BTreeNode::FixSubtreeSz() {

  subtree_sz = GetSubtreeSz();
}

BTreeImpl::BTreeImpl(const BTreeImpl& other) {

  BTreeImpl copy{};

  RecursiveCopy(copy, other.root_.get());

  *this = std::move(copy);
}

BTreeImpl& BTreeImpl::operator=(const BTreeImpl& other) {
  if (this == &other) {
    return *this;
  }

  BTreeImpl copy{};

  RecursiveCopy(copy, other.root_.get());

  *this = std::move(copy);

  return *this;
}

void BTreeImpl::RecursiveCopy(BTreeImpl& tree, const BTreeNode* node) {

  if (!node) {
    return ;
  }

  for (auto key: node->keys) {
    tree.Insert(key);
  }

  size_t children_sz = node->children.size();
  for (size_t ind = 0; ind < children_sz; ++ind) {
    RecursiveCopy(tree, node->children[ind].get());
  }
}

void BTreeImpl::Insert(int key) {

  if (!root_) {
    root_ = std::unique_ptr<BTreeNode, BTreeNode::NodeDeleter> {new BTreeNode(t_)};
  }

  if (root_->keys.size() == (2 * root_->t - 1)) {
    std::unique_ptr<BTreeNode, BTreeNode::NodeDeleter> new_node {new BTreeNode(t_)};
    new_node->children.insert(new_node->children.begin(), std::move(root_));
    new_node->leaf = false;
    new_node->SplitChild(kRangeBegin, new_node->children.begin()->get());
    root_ = std::move(new_node);
  }

  root_->InsertNonFull(key);
}

size_t BTreeImpl::Query(int left, int right) const {

  if (!root_ || (right < left || IsEqual(left, right))) {
    return 0;
  }

  size_t left_counter = FindLess(root_.get(), left + 1);

  size_t right_counter = 0;
  if (right == INT_MAX) {
    right_counter = root_->subtree_sz;
  } else {
    right_counter = FindLess(root_.get(), right + 1);
  }

  return right_counter - left_counter;
}

int BTreeImpl::Min(size_t index) const {

  if (!root_ || index == 0 || index > root_->subtree_sz) {
    throw prog_error::ProgError{"Incorrect index!",
                                prog_error::ErrorCode::kOutOfRange};
  }

  size_t current_ind = 0;
  int key = FindMin(root_.get(), index, current_ind);

  return key;
}

size_t BTreeImpl::LessKeysCounter(int key) const {

  size_t counter = FindLess(root_.get(), key);

  return counter;
}

size_t BTreeImpl::FindLess(BTreeNode* node, int key) const {

  if (!node) {
    return 0;
  }

  size_t counter = 0;

  size_t first_bigger = static_cast<int>(
    std::distance(node->keys.begin(), std::ranges::lower_bound(node->keys, key))
  );
  counter += first_bigger;

  if (node->leaf) {
    return counter;
  }

  for (size_t ind = 0; ind < first_bigger; ++ind) {
    counter += node->children[ind]->subtree_sz;
  }

  if (first_bigger < node->keys.size() && IsEqual(key, node->keys[first_bigger])) {
    counter += node->children[first_bigger]->subtree_sz;
  } else {
    counter += FindLess(node->children[first_bigger].get(), key);
  }

  return counter;
}

int BTreeImpl::FindMin(BTreeNode* node, size_t ind,
                                      size_t& current_ind) const {

  assert(node);

  if (node->leaf) {
    return node->keys[ind - current_ind - 1];
  }

  size_t child_ind = 0;
  for (; child_ind < node->keys.size(); ++child_ind) {
    size_t child_sz = node->children[child_ind]->subtree_sz;

    if (ind <= current_ind + child_sz) {
      return FindMin(node->children[child_ind].get(), ind, current_ind);
    }

    current_ind += child_sz + 1;  // the descendant and the key after it

    if (ind == current_ind) {
      return node->keys[child_ind];
    }
  }

  return FindMin(node->children[child_ind].get(), ind, current_ind);
}
} // namespace dynamic_order
