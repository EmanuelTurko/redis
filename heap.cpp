#include "heap.h"

auto Heap::up(size_t pos) -> void {

  auto item = _heap[pos];
  while (pos > 0 && _heap[getParent(pos)].val > item.val) {

    // swap with the parent
    _heap[pos] = _heap[getParent(pos)];
    *_heap[pos].ref = pos;
    pos = getParent(pos);
  }
  _heap[pos] = item;
  *_heap[pos].ref = pos;
}
auto Heap::down(size_t pos) -> void {

  auto item = _heap[pos];
  while (true) {

    // find the smallest one among the parent and their kids

    auto left = getLeft(pos);
    auto right = getRight(pos);
    auto minPos = pos;
    auto minVal = item.val;

    if (left < _heap.size() && _heap[left].val < minVal) {
      minPos = left;
      minVal = _heap[left].val;
    }
    if (right < _heap.size() && _heap[right].val < minVal)
      minPos = right;
    if (minPos == pos)
      break;

    // swap with the kid
    _heap[pos] = _heap[minPos];
    *_heap[pos].ref = pos;
    pos = minPos;
  }
  _heap[pos] = item;
  *_heap[pos].ref = pos;
}
auto Heap::upsert(size_t pos,HeapItem item) -> void{

    if(pos < _heap.size()) _heap[pos] = item; // update an existing item
    else{
        pos = _heap.size();
        _heap.push_back(item); // or add a new item
    }
    update(pos);
}
auto Heap::remove(size_t pos) -> void{

    //swap the erased item with the last item
    _heap[pos] = _heap.back();
    _heap.pop_back();

    //update the swapped item
    if (pos < _heap.size()) update(pos);
}

auto Heap::update(size_t pos) -> void {

  if (pos > 0 && _heap[getParent(pos)].val > _heap[pos].val)
    up(pos);
  else
    down(pos);
}