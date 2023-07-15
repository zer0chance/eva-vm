/**
 * EVM Launcher
*/

#include "vm/EvaVM.hpp"
#include <iostream>

int main(int argc, char** argv) {
  EvaVM evm;

  auto result = evm.exec(R"(
        (var x 10)
        (def foo () x)

        (begin
          (var y 100)
          (var q 300)
          q
          (begin
            (var z 200)
            z
            (def bar () (+ y z)
            (bar))))
  )");

  std::cout << "\nVM exited gracefully with value: " << result << std::endl;
  return 0;
}

