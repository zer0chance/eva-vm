
#include "vm/OpCode.hpp"
#include "logging/Logger.hpp"

#define OP_STR(op)  \
    case OP_##op:   \
        return #op

std::string opcodeToString(ByteCode opcode) {
    switch(opcode) {
        OP_STR(HALT);
        OP_STR(CONST);
        OP_STR(ADD);
        OP_STR(SUB);
        OP_STR(MUL);
        OP_STR(DIV);
        OP_STR(CMP);
        OP_STR(JMP);
        OP_STR(JMP_IF_FALSE);
        default:
            DIE << "Unknown bytecode: " << opcode;
            return "";
    }
}
