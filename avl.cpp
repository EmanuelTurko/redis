#include "avl.h"
#include <algorithm>
#include <cassert>

auto AVLTree::init(AVLNode *node) -> void {
  node->left = node->right = node->parent = nullptr;
  node->height = 1;
  node->count = 1;
}
auto AVLTree::update(AVLNode *node) -> void {
  node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
  node->count = 1 + getCount(node->left) + getCount(node->right);
}
auto AVLTree::rotateLeft(AVLNode *node) -> AVLNode * {

  AVLNode *parent = node->parent;
  AVLNode *newNode = node->right;
  AVLNode *inner = newNode->left;

  // node <-> inner
  node->right = inner;
  if (inner)
    inner->parent = node;

  // parent <- newNode
  newNode->parent = parent;

  // newNode <-> node
  newNode->left = node;
  node->parent = newNode;

  // auxiliary data
  update(node);
  update(newNode);
  return newNode;
}
auto AVLTree::rotateRight(AVLNode *node) -> AVLNode * {

  AVLNode *parent = node->parent;
  AVLNode *newNode = node->left;
  AVLNode *inner = newNode->right;

  // node <-> inner
  node->left = inner;
  if (inner)
    inner->parent = node;

  // parent <- newNode
  newNode->parent = parent;

  // newNode <-> node
  newNode->right = node;
  node->parent = newNode;

  // auxiliary data
  update(node);
  update(newNode);
  return newNode;
}
// the left subtree is taller by 2
auto AVLTree::fixLeft(AVLNode *node) -> AVLNode * {

  if (getHeight(node->left->left) < getHeight(node->left->right))
    node->left = rotateLeft(node->left);
  return rotateRight(node);
}
// the right subtree is taller by 2
auto AVLTree::fixRight(AVLNode *node) -> AVLNode * {

  if (getHeight(node->right->right) < getHeight(node->right->left))
    node->right = rotateRight(node->right);
  return rotateLeft(node);
}
// fix imbalanced nodes and maintain invariants until the root is reached
auto AVLTree::fix(AVLNode *node) -> AVLNode * {

  while (true) {

    AVLNode **from = &node;
    AVLNode *parent = node->parent;

    if (parent)
      // attach the fixed subtree to the parent
      from = parent->left == node ? &parent->left : &parent->right;
    // else : save to the local variable `node`

    // auxiliary data
    update(node);

    // fix the height difference of 2
    std::uint32_t left = getHeight(node->left);
    std::uint32_t right = getHeight(node->right);

    if (left == right + 2)
      *from = fixLeft(node);
    else if (left + 2 == right)
      *from = fixRight(node);

    // root node, stop
    if (!parent) {
      _root = *from;
      return _root;
    }

    // continue to the parent node because its height may be changed
    node = parent;
  }
}
// detach a node where 1 of its children is empty
auto AVLTree::removeEasy(AVLNode *node) -> AVLNode * {

  assert(!node->left || !node->right);                    // at most 1 child
  AVLNode *child = node->left ? node->left : node->right; // can be nullptr
  AVLNode *parent = node->parent;

  // update the child's parent pointer
  if (child)
    child->parent = parent; // can be nullptr

  // attach the child to the grandparent
  if (!parent) {
    _root = child; // removing the root node
    return _root;
  }
  AVLNode **from = parent->left == node ? &parent->left : &parent->right;
  *from = child;

  // rebalance the updated tree
  return fix(parent);
}
// detach a node and returns the new root of the tree
auto AVLTree::remove(AVLNode *node) -> AVLNode * {

  // the easy case of 0 or 1 child
  if (!node->left || !node->right)
    return removeEasy(node);

  // find the successor
  AVLNode *victim = node->right;
  while (victim->left)
    victim = victim->left;

  // detach the successor
  _root = removeEasy(victim);

  // swap with the successor
  *victim = *node; // left, right, parent
  if (victim->left)
    victim->left->parent = victim;
  if (victim->right)
    victim->right->parent = victim;

  // attach the successor to the parent, or update the root pointer
  AVLNode **from = &_root;
  AVLNode *parent = node->parent;
  if (parent)
    from = parent->left == node ? &parent->left : &parent->right;

  *from = victim;
  return _root;
}
auto AVLTree::insert(AVLNode *node, LesserNodeFn isLess) -> void {
  AVLNode *parent = nullptr;
  AVLNode **from = &_root;
  while (*from) {
    parent = *from;
    from = isLess(node, parent) ? &parent->left : &parent->right;
  }
  *from = node;
  node->parent = parent;

  fix(node); // assigns _root
}
auto AVLTree::dispose(AVLNode *node, DeleteNodeFn del) -> void {
  if (!node)
    return;

  dispose(node->left, del);
  dispose(node->right, del);
  del(node);
}
// offset into the succeeding or preceding node.
// note: the worst-case is O(log N) regardless of how long the offset is.
auto AVLTree::offset(AVLNode *node, std::int64_t offset) -> AVLNode * {
  std::int64_t pos{}; // the rank difference from the starting node
  while (offset != pos) {
    if (pos < offset && pos + getCount(node->right) >= offset) {
      // the target is inside the right subtree
      node = node->right;
      pos += getCount(node->left) + 1;
    } else if (pos > offset && pos - getCount(node->left) <= offset) {
      // the target is inside the left subtree
      node = node->left;
      pos -= getCount(node->right) + 1;
    } else {
      // go to the parent
      auto *parent = node->parent;
      if (!parent)
        return nullptr;
      if (parent->right == node)
        pos -= getCount(node->left) + 1;
      else
        pos += getCount(node->right) + 1;

      node = parent;
    }
  }
  return node;
}
auto AVLTree::getCount(AVLNode *node) const -> std::uint32_t {
  if (node)
    return node->count;
  return 0;
}
auto AVLTree::getHeight(AVLNode *node) const -> std::uint32_t {
  if (node)
    return node->height;
  return 0;
}