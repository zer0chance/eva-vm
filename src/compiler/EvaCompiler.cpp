#include "compiler/EvaCompiler.hpp"
#include "logging/Logger.hpp"
#include "vm/OpCode.hpp"

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

std::map<std::string, uint8_t> EvaCompiler::cmpOps = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5}
};

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
            // Booleans:
            if (exp.string == "true" || exp.string == "false") {
                emit(OP_CONST);
                emit(booleanConstIdx(exp.string == "true" ? true : false));
            } else {
                // Variables:
            }
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

                // Comparison operations:
                else if (cmpOps.count(op) != 0) {
                    gen(exp.list[1]);
                    gen(exp.list[2]);

                    emit(OP_CMP);
                    emit(cmpOps[op]);
                }

                // Branch instructions:
                // (if <test> <consequent> <alternate>)
                else if (op == "if") {
                    // Emit <test>
                    gen(exp.list[1]);

                    // Else branch initialized with 0 and patched later
                    // Takes 2 bytes
                    emit(OP_JMP_IF_FALSE);
                    emit(0);
                    emit(0);

                    auto elseJmpAddr = getCurrentOffset() - 2;

                    // Emit <consequent>
                    gen(exp.list[2]);
                    emit(OP_JMP);
                    emit(0);
                    emit(0);

                    auto endJmpAddr = getCurrentOffset() - 2;

                    auto elseBranchAddr = getCurrentOffset();
                    patchJumpAddres(elseJmpAddr, elseBranchAddr);

                    // Emit alternate if we have it
                    if (exp.list.size() == 4) {
                        gen(exp.list[3]);
                    }

                    auto endBranchAddr = getCurrentOffset();
                    patchJumpAddres(endJmpAddr, endBranchAddr);
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

size_t EvaCompiler::booleanConstIdx(bool value) {
    ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
    return codeObj->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
    ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
    return codeObj->constants.size() - 1;
}

void EvaCompiler::emit(uint8_t code) {
    codeObj->code.push_back(code);
}

void EvaCompiler::patchJumpAddres(size_t offset, uint16_t value) {
    writeByteAtOffset(value >> 8 & 0xFF, offset);
    writeByteAtOffset(value & 0xFF, offset + 1);
}
