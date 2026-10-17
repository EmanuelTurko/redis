#pragma once

#include <cassert>
#include <cstdint>

struct AVLNode {
  AVLNode *parent{};
  AVLNode *left{};
  AVLNode *right{};
  std::uint32_t height{1}; // subtree height
  std::uint32_t count{1};  // subtree size
};
using LesserNodeFn = bool (*)(AVLNode *, AVLNode *);
using DeleteNodeFn = void (*)(AVLNode *);

class AVLTree {
public:
  AVLTree(){};
  AVLTree(AVLNode *node){ _root = node;}
  ~AVLTree() = default;

  AVLTree(AVLTree &) = delete;
  AVLTree &operator=(const AVLTree &) = delete;

  AVLTree(AVLTree &&) = default;
  AVLTree &operator=(AVLTree &&) = default;

  auto getRoot() const -> AVLNode * { return _root; }
  auto getHeight(AVLNode *node) const -> std::uint32_t;
  auto getCount(AVLNode *node) const -> std::uint32_t;

  auto init(AVLNode *node) -> void;
  auto update(AVLNode *node) -> void;
  auto insert(AVLNode *node, LesserNodeFn isLess) -> void;
  auto dispose(AVLNode *node, DeleteNodeFn del) -> void;
  auto offset(AVLNode *node, std::int64_t offset) -> AVLNode *;

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