/**
 * EVM Launcher
*/

#include "EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        ( + "Hello, "  "world!" )
    )");

    std::cout << "\nVM exited gracefully with value: " << AS_NUMBER(result);
    return 0;
}
