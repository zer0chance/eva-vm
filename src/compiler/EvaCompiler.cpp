#include "compiler/EvaCompiler.hpp"
#include "OpCode.hpp"
#include "Logger.hpp"

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
        
        case ExpType::LIST:
            DIE << "Unimplemented.";
            break;

        default:
            DIE << "Unknown expression type met, must be a parser error.";
    }
}

size_t EvaCompiler::numericConstIdx(double value) {
    // Checking if constant is already defined
    for (size_t i = 0; i < codeObj->constants.size(); i++) {
        if (!IS_NUMBER(codeObj->constants[i])) continue;

        if (AS_NUMBER(codeObj->constants[i]) == value) {
            return i;
        }
    }

    // Defining new constant
    codeObj->constants.push_back(NUMBER(value));
    return codeObj->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
    // Checking if constant is already defined
    for (size_t i = 0; i < codeObj->constants.size(); i++) {
        if (!IS_STRING(codeObj->constants[i])) continue;

        if (AS_CPPSTRING(codeObj->constants[i]) == value) {
            return i;
        }
    }
    
    // Defining new constant
    codeObj->constants.push_back(ALLOC_STRING(value));
    return codeObj->constants.size() - 1;
}

void EvaCompiler::emit(uint8_t code) {
    codeObj->code.push_back(code);
}
