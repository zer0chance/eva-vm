#include "EvaVM.hpp"
#include "OpCode.hpp"

#define BINARY_OP(op) do {              \
    auto op2 = AS_NUMBER(pop());        \
    auto op1 = AS_NUMBER(pop());        \
    push(NUMBER(op1 op op2));           \
} while (0)

#define COMPARE_VALUES(op, v1, v2) do { \
    bool res;                           \
    switch (op) {                       \
    case 0:                             \
        res = v1 < v2;                  \
        break;                          \
    case 1:                             \
        res = v1 > v2;                  \
        break;                          \
    case 2:                             \
        res = v1 == v2;                 \
        break;                          \
    case 3:                             \
        res = v1 >= v2;                 \
        break;                          \
    case 4:                             \
        res = v1 <= v2;                 \
        break;                          \
    case 5:                             \
        res = v1 != v2;                 \
        break;                          \
    default:                            \
        DIE << "Bad comparison op!";    \
    }                                   \
    push(BOOLEAN(res));                 \
} while (0)

#define TO_ADDRESS(index) (&codeObj->code[index])

EvaValue EvaVM::exec(const std::string& program) {
    // 1. Parse AST
    auto ast = parser->parse(program);

    // 2. Compile AST to bytecode.
    codeObj = compiler->compile(ast);

    // constants.push_back(NUMBER(3));
    // constants.push_back(NUMBER(1));

    // code = { OP_CONST, 0, OP_CONST, 1, OP_ADD, OP_HALT };

    ip = &codeObj->code[0];
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
            push(codeObj->constants[next_opcode()]);
            break;
        
        case OP_ADD: {
            auto op2 = pop();
            auto op1 = pop();

            if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                auto n1 = AS_NUMBER(op1);
                auto n2 = AS_NUMBER(op2);
                push(NUMBER(n1 + n2));
            } else if (IS_STRING(op1) && IS_STRING(op2)) {
                auto s1 = AS_CPPSTRING(op1);
                auto s2 = AS_CPPSTRING(op2);
                push(ALLOC_STRING(s1 + s2));
            } else {
                DIE << "Incompatible types in addition \n";
            }

            break;
        }
        
        case OP_SUB:
            BINARY_OP(-);
            break;
        
        case OP_MUL:
            BINARY_OP(*);
            break;
        
        case OP_DIV:
            BINARY_OP(/);
            break;
        
        case OP_CMP: {
            auto op = next_opcode();

            auto op2 = pop();
            auto op1 = pop();

            if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
                auto v1 = AS_NUMBER(op1);
                auto v2 = AS_NUMBER(op2);
                COMPARE_VALUES(op, v1, v2);
            } else if (IS_STRING(op1) && IS_STRING(op2)) {
                auto s1 = AS_CPPSTRING(op1);
                auto s2 = AS_CPPSTRING(op2);
                COMPARE_VALUES(op, s1, s2);
            }

            break;
        }

        case OP_JMP: {
            ip = TO_ADDRESS(next_short());
            break;
        }

        case OP_JMP_IF_FALSE: {
            auto cond = AS_BOOLEAN(pop()); // TODO: TO_BOOLEAN converter

            auto address = next_short();

            if (!cond) {
                ip = TO_ADDRESS(address);
            }
            break;
        }

        default:
            DIE << "Illegal bytecode: " << HEX(bytecode) << '\n';
            break;
        }
    }
}
