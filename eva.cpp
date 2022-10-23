/**
 * EVM Launcher
*/

#include "EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    evm.exec("Dummy");

    std::cout << "\nVM exited gracefully.";
    return 0;
}
