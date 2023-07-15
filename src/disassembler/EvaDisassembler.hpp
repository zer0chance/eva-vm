/**
 * EvaDisassembler
 */
#ifndef SRC_DISASSEMBLER_EVADISASSEMBLER_HPP
#define SRC_DISASSEMBLER_EVADISASSEMBLER_HPP

#include "vm/EvaValue.hpp"
#include "vm/OpCode.hpp"
#include "vm/Global.hpp"

#include <array>
#include <memory>

class EvaDisassembler {
  public:
    EvaDisassembler(std::shared_ptr<Global> global) : global(global) {}

    /**
     * Disassembles the code unit.
     */
    void disassemble(CodeObject* co);

  private:
    /**
     * Global object. Shared with VM.
     */
    std::shared_ptr<Global> global;

    /**
     * Comparison operators as a strings.
     */
    static std::array<std::string, 6> inverseCompareOps;

    /**
     * Disassembles individual instruction and updates the offset.
     */
    size_t disassembleInstruction(CodeObject* co, size_t offset);

    /**
     * Disassembles one byte.
     */
    size_t disassembleSimple(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles two bytes.
     */
    size_t disassembleWord(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles const instruction.
     */
    size_t disassembleConst(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles global variables.
     */
    size_t disassembleGlobal(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles local variables.
     */
    size_t disassembleLocal(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles cells.
     */
    size_t disassembleCell(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles compare instruction.
     */
    size_t disassembleCompare(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Disassembles jump instruction.
     */
    size_t disassembleJump(CodeObject* co, ByteCode opcode, size_t offset);

    /**
     * Prints raw bytes.
     */
    void dumpBytes(CodeObject* co, size_t offset, size_t count);

    /**
     * Prints opcode as string.
     */
    void printOpcode(ByteCode opcode);
};

#endif
