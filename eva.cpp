/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (set x (+ x 1))
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
