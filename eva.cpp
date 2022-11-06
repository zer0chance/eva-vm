/**
 * EVM Launcher
*/

#include "EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        "Hello"
    )");

    std::cout << "\nVM exited gracefully with value: " << AS_CPPSTRING(result);
    return 0;
}
