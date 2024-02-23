#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include "classes.h"

class Heap {
public:
    template <class T, class... Args>
    requires(std::is_convertible_v<T, Object>) Object* Make(Args&&... args) {
        memory_.emplace_back(new T(std::forward<Args>(args)...));
        return memory_.back().get();
    }

    void Check(Object* root);

private:
    std::vector<std::unique_ptr<Object>> memory_;

    friend Object;

    void PopBack();
};
