/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
    EvaVM evm;

    auto result = evm.exec(R"(
        (var i 10)
        (var count 0)

        (while (> i 0)
            (begin
                (set i (- i 1))
                (set count (+ count 1))
            )
        )
        
        count
    )");

    std::cout << "\nVM exited gracefully with value: " << result;
    return 0;
}

        // (begin 
        //   (var x 100)
        //   x
        // )

        // x