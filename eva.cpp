/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (var r (square 2))
        (max 3 r)
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
