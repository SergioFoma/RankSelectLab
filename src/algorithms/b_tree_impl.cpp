module;

#include <cstddef>
#include <cassert>
#include <utility>
#include <climits>
#include <tbb/parallel_sort.h>

module b_tree_impl;

import prog_error;

void dynamic_order::BTreeNode::SplitChild(
  size_t child_index,
  dynamic_order::BTreeNode* child
) {

  assert(child);
  assert(children_[child_index] == child);

  BTreeNode* new_node = new BTreeNode(child->t_, child->leaf_);
  size_t t = child->t_;
  const size_t kRangeStart = dynamic_order::kRangeBegin;

  new_node->keys_ = Copy(child->keys_, t - 1, child->keys_.size());
  child->keys_ = Copy(child->keys_, kRangeStart, t - 1);

  if (!child->leaf_) {
    new_node->children_ = Copy(child->children_, t, child->children_.size());
    child->children_ = Copy(child->children_, kRangeStart, t);
  }

  children_.insert(children_.begin() + child_index + 1, new_node);
  keys_.insert(keys_.begin() + child_index, new_node->keys_.front());
  new_node->keys_.erase(new_node->keys_.begin());

  new_node->FixSubtreeSz();
  child->FixSubtreeSz();
  FixSubtreeSz();
}

void dynamic_order::BTreeNode::InsertNonFull(
  int key
) {

  if (leaf_) {
    keys_.push_back(key);
    tbb::parallel_sort(keys_.begin(), keys_.end());
  } else {
    int insert_pos = static_cast<int>(keys_.size() - 1);
    while (insert_pos >= 0 && key < keys_[insert_pos]) {
      --insert_pos;
    }
    ++insert_pos;

    if (children_[insert_pos]->keys_.size() ==
        (2 * t_ - 1)) {
      SplitChild(insert_pos, children_[insert_pos]);

      if (key > keys_[insert_pos]) {
        ++insert_pos;
      }
    }
    children_[insert_pos]->InsertNonFull(key);
  }

  FixSubtreeSz();
}

size_t dynamic_order::BTreeNode::GetSubtreeSz() const {

  size_t counter = 0;
  for (auto* child: children_) {
    if (child) {
      counter += child->subtree_sz_;
    }
  }

  return counter + keys_.size();
}

void dynamic_order::BTreeNode::FixSubtreeSz() {

  subtree_sz_ = GetSubtreeSz();
}

void dynamic_order::BTreeNode::DropChildren() {

  for (auto* child: children_) {
    delete child;
  }

  children_.clear();
}

bool dynamic_order::BTreeImpl::Search(int key) const {

  BTreeNode* node = root_;
  while (true) {

    size_t key_numbers = node->keys_.size();
    for (size_t ind = 0; ind < key_numbers; ++ind) {
      if (node->keys_[ind] == key) {
        return true;
      }
    }

    if (node->leaf_) {
      break;
    }

    int ind = static_cast<int>(key_numbers) - 1;
    while (ind >= 0 && key < node->keys_[ind]){
      --ind;
    }
    ++ind;

    node = node->children_[ind];
  }

  return false;
}

void dynamic_order::BTreeImpl::Insert(int key) {

  if (Search(key)){
    return ;
  }

  if (root_->keys_.size() == (2 * root_->t_ - 1)) {
    BTreeNode* new_node = new BTreeNode(t_);
    new_node->children_.insert(new_node->children_.begin(), root_);
    new_node->leaf_ = false;
    new_node->SplitChild(kRangeBegin, root_);
    root_ = new_node;
  }

  root_->InsertNonFull(key);
}

size_t dynamic_order::BTreeImpl::Query(
  int left,
  int right
) const {

  if (left >= right) {
    return 0;
  }

  size_t left_counter = 0;
  FindLess(root_, left + 1, left_counter);

  size_t right_counter = 0;
  if (right == INT_MAX) {
    right_counter = root_->subtree_sz_;
  } else {
    FindLess(root_, right + 1, right_counter);
  }

  return right_counter - left_counter;
}

int dynamic_order::BTreeImpl::Min(
  size_t index
) const {

  if (index == 0 || index > root_->subtree_sz_) {
    prog_error::ErrorInfo err_info = {prog_error::ErrorCode::kOutOfRange,
                                      "Incorrect index!",
                                      "dynamic_order::BTreeImpl::Min"};
    throw prog_error::ProgError(std::move(err_info));
  }

  size_t current_ind = 0;
  int key = FindMin(root_, index, current_ind);

  return key;
}

size_t dynamic_order::BTreeImpl::LessKeysCounter(
  int key
) const {

  size_t counter = 0;
  FindLess(root_, key, counter);

  return counter;
}

void dynamic_order::BTreeImpl::FindLess(
  BTreeNode* node,
  int key,
  size_t& counter) const {

  if (!node) {
    return ;
  }

  size_t first_bigger = 0, key_numbers = node->keys_.size();
  for (; first_bigger < key_numbers && key > node->keys_[first_bigger];
         ++first_bigger) {
  }
  counter += first_bigger;

  if (node->leaf_) {
    return ;
  }

  for (size_t ind = 0; ind < first_bigger; ++ind) {
    counter += node->children_[ind]->subtree_sz_;
  }

  if (first_bigger < key_numbers &&
    key == node->keys_[first_bigger]) {
    counter += node->children_[first_bigger]->subtree_sz_;
  } else {
    FindLess(node->children_[first_bigger], key, counter);
  }
}

int dynamic_order::BTreeImpl::FindMin(
  BTreeNode* node,
  size_t ind,
  size_t& current_ind
) const {

  assert(node);

  if (node->leaf_) {
    return node->keys_[ind - current_ind - 1];
  }

  size_t child_ind = 0;
  for (; child_ind < node->keys_.size(); ++child_ind) {
    size_t child_sz = node->children_[child_ind]->subtree_sz_;

    if (ind <= current_ind + child_sz) {
      return FindMin(node->children_[child_ind], ind, current_ind);
    }

    current_ind += child_sz + 1;          // потомок и ключ после него

    if (ind == current_ind) {
      return node->keys_[child_ind];
    }
  }

  return FindMin(node->children_[child_ind], ind, current_ind);
}
