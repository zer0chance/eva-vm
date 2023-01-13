/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (var x 0)
        (for x (< x 10) (set x (+ x 2)))

        // (for (var x 0) (< x 10) (set x (+ x 1)))
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
