#include "compiler/EvaCompiler.hpp"
#include "OpCode.hpp"
#include "Logger.hpp"

#define ALLOC_CONST(tester, converter, allocator, value) do {   \
    /* Checking if constant is already defined */               \
    for (size_t i = 0; i < codeObj->constants.size(); i++) {    \
        if (!tester(codeObj->constants[i])) continue;           \
                                                                \
        if (converter(codeObj->constants[i]) == value)          \
            return i;                                           \
    }                                                           \
                                                                \
    /* Defining new constant */                                 \
    codeObj->constants.push_back(allocator(value));             \
} while (0)

#define GEN_BINARY_OPERATOR(op) do {                            \
    gen(exp.list[1]);                                           \
    gen(exp.list[2]);                                           \
    emit(op);                                                   \
} while(0)

CodeObject* EvaCompiler::compile(const Exp& exp) {
    codeObj = AS_CODE(ALLOC_CODE("main"));

    gen(exp);

    emit(OP_HALT);

    return codeObj;
}

void EvaCompiler::gen(const Exp& exp) {
    switch (exp.type) {
        case ExpType::NUMBER:
            emit(OP_CONST);
            emit(numericConstIdx(exp.number));
            break;
        
        case ExpType::STRING:
            emit(OP_CONST);
            emit(stringConstIdx(exp.string));
            break;
        
        case ExpType::SYMBOL:
            DIE << "Unimplemented.";
            break;
        
        case ExpType::LIST: {
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;

                // Binary math operations:
                if (op == "+") {
                    GEN_BINARY_OPERATOR(OP_ADD);
                } else if (op == "-") {
                    GEN_BINARY_OPERATOR(OP_SUB);
                } else if (op == "*") {
                    GEN_BINARY_OPERATOR(OP_MUL);
                } else if (op == "/") {
                    GEN_BINARY_OPERATOR(OP_DIV);
                }
            }
            break;
        }

        default:
            DIE << "Unknown expression type met, must be a parser error.";
    }
}

size_t EvaCompiler::numericConstIdx(double value) {
    ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
    return codeObj->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
    ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
    return codeObj->constants.size() - 1;
}

void EvaCompiler::emit(uint8_t code) {
    codeObj->code.push_back(code);
}
