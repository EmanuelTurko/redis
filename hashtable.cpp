#include "hashtable.h"
#include <cassert>
#include <cstdlib>
#include <hashtable.h>
inline constexpr size_t rehashingWork{128};
inline constexpr size_t maxLoadFactor{8};

// cap must be a power of 2
auto hashInit(HashTable *hashTable, size_t cap) -> void {

  assert(cap > 0 && ((cap - 1) & cap) == 0);

  hashTable->table.resize(cap);
  hashTable->bitMask = cap - 1;
  hashTable->keyCount = 0;
}
// hashtable insertion
auto hashInsert(HashTable *hashTable, HashNode *node) -> void {

  size_t position = node->hashCode & hashTable->bitMask;

  HashNode *next = hashTable->table[position];
  node->next = next;
  hashTable->table[position] = node;
  hashTable->keyCount++;
}
// hashtable look up subroutine.
// Pay attention to the return value. It returns the address of
// the parent pointer that owns the target node,
// which can be used to delete the target node.
auto hashLookup(HashTable *hashTable, HashNode *key,
                bool (*eq)(HashNode *, HashNode *)) -> HashNode ** {

  if (hashTable->table.empty())
    return nullptr;

  size_t position = key->hashCode & hashTable->bitMask;
  HashNode **from =
      &hashTable->table[position]; // incoming pointer to the target

  for (HashNode *current; (current = *from) != nullptr; from = &current->next) {
    if (current->hashCode == key->hashCode && eq(current, key))
      return from; // may be a node, may be a slot
  }
  return nullptr;
}
// remove a node from the chain
auto hashDetach(HashTable *hashTable, HashNode **from) -> HashNode * {

  HashNode *node = *from; // the target node
  *from = node->next;     // update the incoming pointer to the target
  hashTable->keyCount--;
  return node;
}
auto mapHelpRehashing(HashMap *hashMap) -> void {

  size_t nodesMoved{};

  while (nodesMoved < rehashingWork && hashMap->older.keyCount > 0) {

    // find a non-empty slot
    HashNode **from = &hashMap->older.table[hashMap->migratePosition];
    if (!*from) {
      hashMap->migratePosition++;
      continue; // empty slot
    }

    // move the first list item to the newer table

    hashInsert(&hashMap->newer, hashDetach(&hashMap->older, from));
    nodesMoved++;
  }
  // discard the old table if done
  if (hashMap->older.keyCount == 0 && !hashMap->older.table.empty())
    hashMap->older = HashTable{};
}
auto mapTriggerRehashing(HashMap *hashMap) -> void {

  assert(hashMap->older.table.empty());

  // (newer, older) <- (new_table, newer)
  hashMap->older = hashMap->newer;
  hashInit(&hashMap->newer, (hashMap->newer.bitMask + 1) * 2);
  hashMap->migratePosition = 0;
}
auto mapLookup(HashMap *hashMap, HashNode *key,
               bool (*eq)(HashNode *, HashNode *)) -> HashNode * {
  mapHelpRehashing(hashMap);

  HashNode **from = hashLookup(&hashMap->newer, key, eq);
  if (!from)
    from = hashLookup(&hashMap->older, key, eq);

  return from ? *from : nullptr;
}
auto mapInsert(HashMap *hashMap, HashNode *node) -> void {
  if (hashMap->newer.table.empty())
    hashInit(&hashMap->newer, 4); // initialize it if empty

  hashInsert(&hashMap->newer, node); // always insert to the newer table

  if (hashMap->older.table.empty()) { // check whether we need to rehash
    size_t threshold = (hashMap->newer.bitMask + 1) * maxLoadFactor;

    if (hashMap->newer.keyCount >= threshold)
      mapTriggerRehashing(hashMap);
  }

  mapHelpRehashing(hashMap); // migrate some keys
}
auto mapDelete(HashMap *hashMap, HashNode *key,
               bool (*eq)(HashNode *, HashNode *)) -> HashNode * {

  mapHelpRehashing(hashMap);

  if (HashNode **from = hashLookup(&hashMap->newer, key, eq))
    return hashDetach(&hashMap->newer, from);

  if (HashNode **from = hashLookup(&hashMap->older, key, eq))
    return hashDetach(&hashMap->older, from);

  return nullptr;
}
auto mapClear(HashMap *hashMap) -> void { *hashMap = HashMap{}; }
auto mapSize(const HashMap *hashMap) -> size_t {
  return hashMap->newer.keyCount + hashMap->older.keyCount;
}
auto hashForEach(HashTable *hashtable, bool (*f)(HashNode *, void *),
                 void *arg) -> bool {

  for (size_t i = 0; hashtable->bitMask != 0 && i <= hashtable->bitMask; ++i) {
    for (HashNode *node = hashtable->table[i]; node != nullptr;
         node = node->next) {
      if (!f(node, arg))
        return false;
    }
  }
  return true;
}
auto mapForEach(HashMap *hashMap, bool (*f)(HashNode *, void *),
                void *arg) -> void {
  hashForEach(&hashMap->newer, f, arg) && hashForEach(&hashMap->older, f, arg);
}
