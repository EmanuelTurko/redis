#pragma once
#include "avl.h"
#include "hashtable.h"
#include <string>

struct ZNode {

  AVLNode avlNode{};
  HashNode hashNode{};
  double score{};
  std::string name{};

  static auto newNode(std::string &name, double score) -> ZNode *;
  static auto deleteNode(ZNode *node) -> void;
};
class ZSet {
public:

  ZSet() = default;
  ~ZSet(){clear();}
  auto update(ZNode* node,double score) -> void;
  auto insert(std::string &name, double score) -> bool;
  auto lookup(std::string &name) -> ZNode *;
  auto remove(ZNode *node) -> void;
  auto seekGreaterOrEqual(std::string &name, double score) -> ZNode *;
  auto clear() -> void;
  auto offset(ZNode* node,std::int64_t offset) -> ZNode *;

  static auto disposeNode(AVLNode* node) -> void;
  static auto hashCompare(HashNode *node, HashNode *key) -> bool;
  static auto isLess(AVLNode *left, std::string &name, double score) -> bool;
  static auto isLess(AVLNode *left, AVLNode *right) -> bool;

private:
  AVLTree _tree{}; // index by (score, name)
  HashMap _map{};   // index by name
};

// a helper structure for the hashtable lookup
struct HashKey{
  HashNode node{};
  std::string name{};
};