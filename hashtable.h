#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// hashtable node, should be embedded into the payload
struct HashNode {
  HashNode *next{nullptr};
  uint64_t hashCode{0};
};
// a simple fixed-sized hashtable
struct HashTable {
  std::vector<HashNode *> table{}; // array of slots
  size_t bitMask{};                // power of 2 array size, 2^n - 1
  size_t keyCount{};               // number of keys
};
// the real hashtable interface.
// it uses 2 hashtables for progressive rehashing.
struct HashMap {
  HashTable newer{};
  HashTable older{};
  size_t migratePosition{};
};
auto mapLookup(HashMap *hashMap, HashNode *key,
               bool (*eq)(HashNode *, HashNode *)) -> HashNode *;
auto mapInsert(HashMap *hashMap, HashNode *node) -> void;
auto mapDelete(HashMap *hashMap, HashNode *key,
               bool (*eq)(HashNode *, HashNode *)) -> HashNode *;
auto mapClear(HashMap *hashMap) -> void;
auto mapSize(const HashMap *hashMap) -> size_t;
// invoke the callback on each node until it returns false
auto mapForEach(HashMap *hashmap, bool (*f)(HashNode *, void *),
                void *arg) -> void;