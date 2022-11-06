
#ifndef SRC_EVA_VM_HPP
#define SRC_EVA_VM_HPP

#include "EvaValue.hpp"
#include "Logger.hpp"
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
     * Stack.
    */
    static constexpr size_t STACK_LIMIT = 1024;

    EvaValue stack[STACK_LIMIT];

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
    EvaVM() : parser(std::make_unique<EvaParser>()),
              compiler(std::make_unique<EvaCompiler>()) {};

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
        if (sp == &stack[0]) {
            DIE << "pop(): empty stack";
        }

        --sp;
        return *sp;
    }
};

#endif
