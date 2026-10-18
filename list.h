#pragma once

#include <cstddef>

class DList{

    public:
    DList(){_prev = _next = this;};
    DList(DList *node) { node->_prev = node->_next = node; }
    ~DList(){detach();}
    auto getNext() -> DList* {return _next;}
    auto getPrev() -> DList* {return _prev;}
    auto isEmpty()->bool{return _next == this;}
    auto detach() -> void{
        auto* prev = _prev;
        auto* next = _next;
        prev->_next = next;
        next->_prev = prev;
    }
    auto insertBefore(DList* target)->void{
        auto *prev = _prev; // [prev][target][this]
        prev->_next = target;
        target->_next = this;
        target->_prev = prev;
        _prev = target;
    }  
    private:
    DList *_prev{};
    DList *_next{};
};