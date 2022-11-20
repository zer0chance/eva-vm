
#include "disassembler/EvaDisassembler.hpp"

void EvaDisassembler::disassemble(CodeObject* co) {
    std::cout << "\n---------------- Disassembly: " << co->name << " ----------------\n\n";
    size_t offset = 0;
    while (offset < co->code.size()) {
        offset = disassembleInstruction(co, offset);
        std::cout << '\n';
    }
}

size_t EvaDisassembler::disassembleInstruction(CodeObject* co, size_t offset) {
    std::ios_base::fmtflags f(std::cout.flags());

    // Print bytecode offset
    std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
              << offset << "    ";
    
    auto opcode = co->code[offset];

    switch (opcode) {
        case OP_HALT:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            return disassembleSimple(co, opcode, offset);
        case OP_CONST:
            return disassembleConst(co, opcode, offset);
        case OP_CMP:
            return disassembleCompare(co, opcode, offset);
        case OP_JMP:
        case OP_JMP_IF_FALSE:
            return disassembleJump(co, opcode, offset);
        default:
            DIE << "disassemblyInstruction: no disassembly for "
                << opcodeToString(opcode);
            return 0;
    }

    std::cout.flags(f);
}

size_t EvaDisassembler::disassembleSimple(CodeObject* co, ByteCode opcode, size_t offset) {
    dumpBytes(co, offset, 1);
    printOpcode(opcode);
    return offset + 1;
}

size_t EvaDisassembler::disassembleConst(CodeObject* co, ByteCode opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpcode(opcode);
    auto constIndex = co->code[offset + 1];
    std::cout << (int)constIndex << " (" << evaValueToConstantString(co->constants[constIndex]) << ')'; 

    return offset + 2;
}

std::array<std::string, 6> EvaDisassembler::inverseCompareOps = {
    "<", ">", "==", ">=", "<=", "!="
};

size_t EvaDisassembler::disassembleCompare(CodeObject* co, ByteCode opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpcode(opcode);
    auto compareOp = co->code[offset + 1];
    std::cout << (int)compareOp << " (";
    std::cout << inverseCompareOps[compareOp] << ')'; 

    return offset + 2;
}

uint16_t readWordAtOffset(CodeObject* co, size_t offset) {
    return (uint16_t)((co->code[offset]) << 8 | co->code[offset + 1]);
}

size_t EvaDisassembler::disassembleJump(CodeObject* co, ByteCode opcode, size_t offset) {
    std::ios_base::fmtflags f(std::cout.flags());

    dumpBytes(co, offset, 2);
    printOpcode(opcode);
    uint16_t address = readWordAtOffset(co, offset + 1);

    std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
              << (int)address << ' ';

    std::cout.flags(f);

    return offset + 3;
}

void EvaDisassembler::dumpBytes(CodeObject* co, size_t offset, size_t count) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::stringstream ss;

    for (size_t i = 0; i < count; i++) {
        ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
              << ((int)co->code[offset + i] && 0xFF) << " ";
    }
    
    std::cout << std::left << std::setfill(' ') << std::setw(12) << ss.str();

    std::cout.flags(f);
}

void EvaDisassembler::printOpcode(ByteCode opcode) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << std::left << std::setfill(' ') << std::setw(20) << opcodeToString(opcode);
    std::cout.flags(f);
}