#include "EvaVM.hpp"
#include "OpCode.hpp"

#include <iostream>
#include <cstdlib>

void EvaVM::exec(const std::string& program) {
    // 1. Parse AST
    // auto ast = parser->parse(program);

    // 2. Compile AST to bytecode.
    // compiler->compile(ast);
    code = {OP_HALT};

    ip = &code[0];

    return eval();
}

void EvaVM::eval() {
    while(true) {
        uint8_t bytecode = next_opcode();
        switch (bytecode) {
        case OP_HALT:
            return;   
        default:
            std::cout << "Illegal bytecode: " << std::hex << bytecode << '\n';
            exit(-1);
            break;
        }
    }
}
