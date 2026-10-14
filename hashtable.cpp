#include "hashtable.h"
#include <cassert>
#include <cstdlib>


// cap must be a power of 2
auto HashMap::HashTable::init(size_t capacity) -> void {

  assert(capacity > 0 && ((capacity - 1) & capacity) == 0);

  _table.assign(capacity, nullptr);
  _bitMask = capacity - 1;
  _keyCount = 0;
}
// hashtable insertion
auto HashMap::HashTable::insert(HashNode *node) -> void {

  size_t position = node->hashCode & _bitMask;

  HashNode *next = _table[position];
  node->next = next;
  _table[position] = node;
  _keyCount++;
}
// hashtable look up subroutine.
// Pay attention to the return value. It returns the address of
// the parent pointer that owns the target node,
// which can be used to delete the target node.
auto HashMap::HashTable::lookup(HashNode *key, NodeEqualFn eq) -> HashNode ** {

  if (_table.empty())
    return nullptr;

  size_t position = key->hashCode & _bitMask;
  HashNode **from = &_table[position]; // incoming pointer to the target

  for (HashNode *current; (current = *from) != nullptr; from = &current->next) {
    if (current->hashCode == key->hashCode && eq(current, key))
      return from; // may be a node, may be a slot
  }
  return nullptr;
}
// remove a node from the chain
auto HashMap::HashTable::detach(HashNode **from) -> HashNode * {

  HashNode *node = *from; // the target node
  *from = node->next;     // update the incoming pointer to the target
  _keyCount--;
  return node;
}
auto HashMap::HashTable::forEach(NodeVisitorFn visit, void *arg) -> bool {

  for (auto *node : _table) {
    while (node != nullptr) {
      if (!visit(node, arg))
        return false;
      node = node->next;
    }
  }
  return true;
}
auto HashMap::helpRehashing() -> void {

  size_t nodesMoved{};
  while (nodesMoved < rehashingWork && _older._keyCount > 0) {

    // find a non-empty slot
    HashNode **from = &_older._table[_migratePosition];
    if (!*from) {
      _migratePosition++;
      continue; // empty slot
    }

    // move the first list item to the newer table
    _newer.insert(_older.detach(from));
    nodesMoved++;
  }
  // discard the old table if done
  if (_older._keyCount == 0 && !_older._table.empty())
    _older = HashTable{};
}
auto HashMap::rehash() -> void {

  assert(_older._table.empty());

  // (newer, older) <- (new_table, newer)
  _older = std::move(_newer);
  _newer.init((_older._bitMask + 1) * 2);
  _migratePosition = 0;
}
auto HashMap::insert(HashNode *node) -> void {
  if (_newer._table.empty())
    _newer.init(initialCapacity); // initialize it if empty

  _newer.insert(node); // always insert to the newer table

  if (_older._table.empty()) { // check whether we need to rehash
    size_t threshold = (_newer._bitMask + 1) * maxLoadFactor;

    if (_newer._keyCount >= threshold)
      rehash();
  }
  helpRehashing(); // migrate some keys
}
auto HashMap::lookup(HashNode *key, NodeEqualFn eq) -> HashNode * {
  helpRehashing();

  HashNode **from = _newer.lookup(key, eq);
  if (!from)
    from = _older.lookup(key, eq);

  return from ? *from : nullptr;
}
auto HashMap::remove(HashNode *key, NodeEqualFn eq) -> HashNode * {

  helpRehashing();

  if (HashNode **from = _newer.lookup(key, eq))
    return _newer.detach(from);
  if (HashNode **from = _older.lookup(key, eq))
    return _older.detach(from);
  return nullptr;
}
auto HashMap::clear() -> void { *this = HashMap{}; }
auto HashMap::size() const -> size_t { return _newer._keyCount + _older._keyCount; }
auto HashMap::forEach(NodeVisitorFn visit, void *arg) -> void {
  _newer.forEach(visit, arg) && _older.forEach(visit, arg);
}
