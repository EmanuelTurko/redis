#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>


struct HeapItem{
    std::uint64_t val{};
    size_t *ref{};
};
class Heap{
public:
    Heap() = default;
    ~Heap() = default;

    auto getParent(size_t i)-> size_t {return (i+1)/2-1;}
    auto getLeft(size_t i) -> size_t {return i*2+1;}
    auto getRight(size_t i)->size_t {return i*2+2;}
    auto operator[](size_t pos) const -> const HeapItem & { return _heap[pos]; }
    auto empty(){return _heap.empty();}
    auto remove(size_t pos) -> void;
    auto upsert(size_t pos, HeapItem item) -> void;
    auto up(size_t pos) -> void;
    auto down(size_t pos) -> void;
    auto update(size_t pos) -> void;

  private:
    std::vector<HeapItem> _heap;
};


