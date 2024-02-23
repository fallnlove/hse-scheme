#pragma once

#include <memory>
#include <string>
#include "object.h"

class Interpreter {
public:
    Interpreter();

    Interpreter(const Interpreter& other) = delete;

    Interpreter(Interpreter&& other) = delete;

    Interpreter& operator=(const Interpreter& other) = delete;

    Interpreter& operator=(Interpreter&& other) = delete;

    ~Interpreter();

    std::string Run(const std::string& str);

private:
    Object* scope_;
};
