#include "heap.h"
#include <cstddef>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <vector>
#include "object.h"

void Heap::PopBack() {
    while (!memory_.empty() && !memory_.back()->is_achivable_) {
        memory_.pop_back();
    }
}

void Heap::Check(Object* root) {
    for (auto& obj : memory_) {
        obj->is_achivable_ = false;
    }
    root->Mark();

    std::vector<size_t> indexes;
    size_t size_to_del = 0;

    for (auto& obj : memory_) {
        if (!obj->is_achivable_) {
            ++size_to_del;
        }
    }

    for (size_t i = 0; i < size_to_del; ++i) {
        if (memory_[memory_.size() - i - 1]->is_achivable_) {
            indexes.push_back(memory_.size() - i - 1);
        }
    }

    for (size_t i = 0; i < memory_.size() && !indexes.empty(); ++i) {
        if (!memory_[i]->is_achivable_) {
            std::swap(memory_[indexes.back()], memory_[i]);
            indexes.pop_back();
        }
    }

    PopBack();
}
