/**
 * EVM Launcher
*/

#include "EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (<= 5 5)
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
