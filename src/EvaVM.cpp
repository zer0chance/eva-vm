#include "EvaVM.hpp"
#include "OpCode.hpp"

EvaValue EvaVM::exec(const std::string& program) {
    // 1. Parse AST
    // auto ast = parser->parse(program);

    // 2. Compile AST to bytecode.
    // compiler->compile(ast);
    constants.push_back(NUMBER(42));

    code = { OP_CONST, 0, OP_HALT };

    ip = &code[0];

    return eval();
}

EvaValue EvaVM::eval() {
    while(true) {
        auto bytecode = next_opcode();
        switch (bytecode) {
        case OP_HALT:
            return pop();

        case OP_CONST:
            push(constants[next_opcode()]);
            break;

        default:
            DIE << "Illegal bytecode: " << HEX(bytecode) << '\n';
            break;
        }
    }
}
