/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (var square (lambda (x) (* x x)))
        (square 2)
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
