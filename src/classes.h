#pragma once

#include <memory>
#include <type_traits>

class Object;
class Number;
class Symbol;
class Cell;
class Interpreter;
class Heap;

template <class T>
requires(std::is_convertible_v<T, Object>) T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(Object* obj) {
    return dynamic_cast<T*>(obj) != nullptr;
}

template <class T>
T& GetInstance() {
    static T instance;
    return instance;
}
