
#ifndef SRC_EVA_VM_HPP
#define SRC_EVA_VM_HPP

#include "vm/Global.hpp"
#include "vm/EvaValue.hpp"
#include "logging/Logger.hpp"
#include "parser/EvaParser.hpp"
#include "compiler/EvaCompiler.hpp"

#include <string>

using syntax::EvaParser;

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
     * Base pointer.
    */
    EvaValue* bp;

    /**
     * Stack.
    */
    static constexpr size_t STACK_LIMIT = 1024;

    EvaValue stack[STACK_LIMIT];

    /**
     * Global object. Shared with compiler.
    */
    std::shared_ptr<Global> global;

    /**
     * Pareser.
    */
    std::unique_ptr<EvaParser> parser;

    /**
     * Compiler.
    */
    std::unique_ptr<EvaCompiler> compiler;

    /**
     * Code object.
    */
    CodeObject* codeObj;

  public:
    EvaVM() : global(std::make_shared<Global>()),
              parser(std::make_unique<EvaParser>()),
              compiler(std::make_unique<EvaCompiler>(global)) {
        setGlobalVariables();
    };

    /**
     * Executes passed program.
    */
    EvaValue exec(const std::string& program);

    /**
     * Main interpreter loop.
    */
    EvaValue eval();

    const uint8_t next_opcode() {
        // Return current opcode and increment IP
        return *ip++;
    }

    const uint16_t next_short() {
        // Return current opcode and increment IP
        ip += 2;
        return (uint16_t)((ip[-2] << 8) | ip[-1]);
    }

    const void push(const EvaValue& value) {
        // Push the value on TOS and increment SP
        if ((size_t) (sp - stack) == STACK_LIMIT) {
            DIE << "Stack overflow";
        }

        *sp = value;
        ++sp;
    }

    /**
     * Pops the value from the stack.
    */
    EvaValue pop() {
        // Return value on TOS and decrement SP
        if (sp == &stack[0]) {
            DIE << "pop(): empty stack";
        }

        --sp;
        return *sp;
    }

    /**
     * Pops N values from the stack.
    */
    EvaValue popN(size_t N) {
        for (size_t i = 0; i < N; i++) pop();
        return *sp;
    }

    /**
     * Peeks the value from the stack at offset without poping it.
    */
    EvaValue peek(size_t offset = 0) {
        // Return value on TOS and decrement SP
        if (sp == &stack[0]) {
            DIE << "pop(): empty stack";
        }

        return *(sp - 1 - offset);
    }

    /**
     * Sets up global variables.
    */
    void setGlobalVariables();
};

#endif
