
#ifndef SRC_EVA_VM_HPP
#define SRC_EVA_VM_HPP

#include <string>
#include <vector>

class EvaVM final {
    /**
     * Bytecode instructions.
    */
    std::vector<uint8_t> code;

    /**
     * Instruction pointer
    */
    uint8_t* ip;

  public:
    /**
     * Executes passed program.
    */
    void exec(const std::string& program);

    /**
     * Main interpreter loop.
    */
    void eval();

    const uint8_t next_opcode() {
        // Return current opcode and increment ip
        return *ip++;
    }
};

#endif
