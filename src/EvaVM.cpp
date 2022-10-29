#include "EvaVM.hpp"
#include "OpCode.hpp"

#define BINARY_OP(op) do {       \
    auto op2 = AS_NUMBER(pop()); \
    auto op1 = AS_NUMBER(pop()); \
    push(NUMBER(op1 op op2));    \
} while (0)

EvaValue EvaVM::exec(const std::string& program) {
    // 1. Parse AST
    // auto ast = parser->parse(program);

    // 2. Compile AST to bytecode.
    // compiler->compile(ast);
    constants.push_back(NUMBER(1));
    constants.push_back(NUMBER(12));
    constants.push_back(NUMBER(3));

    code = { OP_CONST, 0, OP_CONST, 1, OP_CONST, 2, OP_DIV, OP_ADD, OP_HALT };

    ip = &code[0];
    sp = &stack[0];

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
        
        case OP_ADD:
            BINARY_OP(+);
            break;
        
        case OP_SUB:
            BINARY_OP(-);
            break;
        
        case OP_MUL:
            BINARY_OP(*);
            break;
        
        case OP_DIV:
            BINARY_OP(/);
            break;

        default:
            DIE << "Illegal bytecode: " << HEX(bytecode) << '\n';
            break;
        }
    }
}
