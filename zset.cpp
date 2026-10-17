#include "zset.h"
#include "avl.h"
#include "hashtable.h"
#include "util.h"
#include <cassert>
#include <cstring>

auto ZNode::newNode(std::string &name, double score) -> ZNode * {

  auto node = new ZNode{};
  assert(node);

  node->hashNode.next = nullptr;
  node->hashNode.hashCode =
      stringHash(reinterpret_cast<std::uint8_t *>(name.data()), name.length());
  node->score = score;
  node->name = name;

  return node;
}
auto ZNode::deleteNode(ZNode *node) -> void { delete(node); }
// compare by the (score, name) tuple
auto ZSet::isLess(AVLNode *left, std::string &name, double score) -> bool {

  auto zLeft = containerOf(left, ZNode, avlNode);
  if (zLeft->score != score)
    return zLeft->score < score;

  int val = zLeft->name.compare(name);
  if (val != 0)
    return val < 0;

  return zLeft->name.length() < name.length();
}
auto ZSet::isLess(AVLNode *left, AVLNode *right) -> bool {

  auto zRight = containerOf(right, ZNode, avlNode);
  return isLess(left, zRight->name, zRight->score);
}
// update the score of an existing node
auto ZSet::update(ZNode *node, double score) -> void {
  if (node->score == score)
    return;

  // detach the tree node
  _tree.remove(&node->avlNode);
  _tree.init(&node->avlNode);

  // reinsert the tree node
  node->score = score;
  _tree.insert(&node->avlNode, isLess);
}
// add a new (score, name) tuple, or update the score of the existing tuple
auto ZSet::insert(std::string &name, double score) -> bool {

  ZNode *node = lookup(name);
  if (node) {
    update(node, score);
    return false;
  }
  node = ZNode::newNode(name, score);
  _map.insert(&node->hashNode);
  _tree.insert(&node->avlNode, isLess);
  return true;
}
auto ZSet::hashCompare(HashNode *node, HashNode *key) -> bool {
  ZNode *zNode = containerOf(node, ZNode, hashNode);
  HashKey *hashKey = containerOf(key, HashKey, node);

  return zNode->name == hashKey->name;
}
// lookup by name
auto ZSet::lookup(std::string &name) -> ZNode * {
  if (!_tree.getRoot())
    return nullptr;

  HashKey key{};
  key.node.hashCode = stringHash(reinterpret_cast<uint8_t *>(name.data()), name.length());
  key.name = name;
  HashNode *found = _map.lookup(&key.node, &hashCompare);
  return found ? containerOf(found, ZNode, hashNode) : nullptr;
}
// delete a node
auto ZSet::remove(ZNode *node) -> void {

  // remove from the hashtable
  HashKey key{};
  key.node.hashCode = node->hashNode.hashCode;
  key.name = node->name;
  HashNode *found = _map.remove(&key.node, &hashCompare);
  assert(found);

  // remove from the tree
  _tree.remove(&node->avlNode);
  // deallocate the node
  ZNode::deleteNode(node);
}
// find the first (score, name) tuple that is >= key.
auto ZSet::seekGreaterOrEqual(std::string &name, double score) -> ZNode * {
  AVLNode *found{};

  for (auto *node = _tree.getRoot(); node;) {
    if (isLess(node, name, score))
      node = node->right; // node < key
    else {
      found = node;
      node = node->left;
    }
  }
  return found ? containerOf(found, ZNode, avlNode) : nullptr;
}
// offset into the succeeding or preceding node.
auto ZSet::offset(ZNode *node, std::int64_t offset) -> ZNode * {
  AVLNode *tNode = node ? _tree.offset(&node->avlNode, offset) : nullptr;
  return tNode ? containerOf(tNode, ZNode, avlNode) : nullptr;
}
auto ZSet::disposeNode(AVLNode *node) -> void {
  ZNode::deleteNode(containerOf(node, ZNode, avlNode));
}
auto ZSet::clear() -> void {
  _map.clear();
  _tree.dispose(_tree.getRoot(), disposeNode);
  _tree = nullptr;
}