/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (def square (x) (* x x))
        // (square 2)

        (def sum (a b) (begin
            (var x 10)
            (+ x (+ a b))
        ))

        // (def factorial (x)
        //     (if (== x 1)
        //         1
        //         (* x (factorial (- x 1)))
        //     )
        // )
        //
        // (factorial 5) // 120
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}
