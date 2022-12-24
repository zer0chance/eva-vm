
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
        OP_STR(GET_GLOBAL);
        OP_STR(SET_GLOBAL);
        OP_STR(POP);
        OP_STR(SET_LOCAL);
        OP_STR(GET_LOCAL);
        OP_STR(SCOPE_EXIT);
        default:
            DIE << "Unknown bytecode: " << HEX(opcode);
            return "";
    }
}
