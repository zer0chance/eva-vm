#include "EvaVM.hpp"
#include "OpCode.hpp"
#include "Logger.hpp"

void EvaVM::exec(const std::string& program) {
    // 1. Parse AST
    // auto ast = parser->parse(program);

    // 2. Compile AST to bytecode.
    // compiler->compile(ast);
    code = {0x1, OP_HALT};

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
            DIE << "Illegal bytecode: " << HEX(bytecode) << '\n';
            break;
        }
    }
}
