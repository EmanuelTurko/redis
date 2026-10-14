#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// hashtable node, should be embedded into the
// payload
struct HashNode {
  HashNode *next{nullptr};
  uint64_t hashCode{0};
};
using NodeEqualFn = bool (*)(HashNode *, HashNode *);
using NodeVisitorFn = bool (*)(HashNode *, void *);
inline constexpr size_t rehashingWork{128};
inline constexpr size_t maxLoadFactor{8};
inline constexpr size_t initialCapacity{4};
class HashMap {
private:
  // a simple fixed-sized hashtable
  struct HashTable {
    std::vector<HashNode *> _table{}; // array of slots
    size_t _bitMask{};                // power of 2 array size // 2^n - 1
    size_t _keyCount{};               // number of keys

    HashTable() = default;

    HashTable(const HashTable &other) = delete;
    HashTable &operator=(const HashTable &other) = delete;

    HashTable(HashTable &&other) = default;
    HashTable &operator=(HashTable &&other) = default;

    auto init(size_t capacity) -> void;
    auto insert(HashNode *node) -> void;
    auto lookup(HashNode *key, NodeEqualFn eq) -> HashNode **;
    auto detach(HashNode **from) -> HashNode *;
    auto forEach(NodeVisitorFn visit, void *arg) -> bool;
  };

  // the real hashtable interface.
  // it uses 2 hashtables for progressive
  // rehashing.
  HashTable _newer{};
  HashTable _older{};
  size_t _migratePosition{};

  auto helpRehashing() -> void;
  auto rehash() -> void;

public:
  auto insert(HashNode *node) -> void;
  auto lookup(HashNode *key, NodeEqualFn eq) -> HashNode *;
  auto remove(HashNode *key, NodeEqualFn eq) -> HashNode *;
  auto clear() -> void;
  auto size() const -> size_t;
  // invoke the callback on each node until it returns false
  auto forEach(NodeVisitorFn visit, void *arg) -> void;
};
