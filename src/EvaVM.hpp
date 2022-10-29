
#ifndef SRC_EVA_VM_HPP
#define SRC_EVA_VM_HPP

#include "EvaValueType.hpp"
#include "Logger.hpp"

#include <string>
#include <vector>
#include <array>


class EvaVM final {
    /**
     * Instruction pointer.
    */
    uint8_t* ip;

    /**
     * Stack pointer.
    */
    EvaValue* sp;

    /**
     * Bytecode instructions.
    */
    std::vector<uint8_t> code;

    /**
     * Constant pool.
    */
    std::vector<EvaValue> constants;

    /**
     * Stack.
    */
    static constexpr size_t STACK_LIMIT = 1024;

    EvaValue stack[STACK_LIMIT];

  public:
    EvaVM() = default;
    /**
     * Executes passed program.
    */
    EvaValue exec(const std::string& program);

    /**
     * Main interpreter loop.
    */
    EvaValue eval();

    const uint8_t next_opcode() {
        // Return current opcode and increment SP
        return *ip++;
    }

    const void push(const EvaValue& value) {
        // Push the value on TOS and increment SP
        if ((size_t) (sp - stack) == STACK_LIMIT) {
            DIE << "Stack overflow";
        }

        *sp = value;
        ++sp;
    }

    EvaValue pop() {
        // Return value on TOS and decrement SP
        if (sp == stack) {
            DIE << "pop(): empty stack";
        }

        --sp;
        return *sp;
    }
};

#endif
