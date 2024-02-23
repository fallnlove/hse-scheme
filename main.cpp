#include <iostream>
#include <string>

#include "src/scheme.h"

int main() {
    Interpreter interpreter;
    std::string input;

    while (std::getline(std::cin, input)) {
        std::cout << interpreter.Run(input) << std::endl;
    }

    return 0;
}
