/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (var x 5)
        (set x (+ x 10))

        (begin
            (set x 100)
            (begin
                (var x 200)
                x
            )
            x
        )
        
        x
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}

        // (begin 
        //   (var x 100)
        //   x
        // )

        // x