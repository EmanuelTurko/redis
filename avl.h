#pragma once

#include <cstdint>

struct AVLNode {
  AVLNode *parent{};
  AVLNode *left{};
  AVLNode *right{};
  std::uint32_t height{}; // subtree height
  std::uint32_t count{};  // subtree size
};
class AVLTree {
public:
  AVLTree();
  AVLTree(AVLNode *node);
  ~AVLTree() = default;

  AVLTree(AVLTree &) = delete;
  AVLTree &operator=(const AVLTree &) = delete;

  AVLTree(AVLTree &&) = default;
  AVLTree &operator=(AVLTree &&) = default;

  auto getRoot() const -> AVLNode * { return _root; }
  auto getHeight(AVLNode *node) const -> std::uint32_t { return node->height; }
  auto getCount(AVLNode *node) const -> std::uint32_t { return node->count; }

  auto init(AVLNode *node) -> void;
  auto update(AVLNode *node) -> void;

  auto fix(AVLNode *node) -> AVLNode *;
  auto fixLeft(AVLNode *node) -> AVLNode *;
  auto fixRight(AVLNode *node) -> AVLNode *;

  auto remove(AVLNode *node) -> AVLNode *;
  auto removeEasy(AVLNode *node) -> AVLNode *;

  auto rotateLeft(AVLNode *node) -> AVLNode *;
  auto rotateRight(AVLNode *node) -> AVLNode *;

private:
  AVLNode *_root{};
};