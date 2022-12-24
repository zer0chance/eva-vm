/**
 * Eva compiler.
*/

#ifndef SRC_COMPILER_EVACOMPILER_HPP
#define SRC_COMPILER_EVACOMPILER_HPP

#include "vm/EvaValue.hpp"
#include "parser/EvaParser.hpp"
#include "disassembler/EvaDisassembler.hpp"

#include <map>
#include <memory>

/**
 * Compiler class, emits bytecodes, records constant pools, vars, etc.
*/
class EvaCompiler {
   /**
    * Compiling code object.
   */
   CodeObject* codeObj;

   /**
    * Comparing operations.
   */
   static std::map<std::string, uint8_t> cmpOps;

    /**
     * Global object. Shared with VM.
    */
    std::shared_ptr<Global> global;

   /**
    * Disassembler.
   */
   std::unique_ptr<EvaDisassembler> disassembler;

  public:
    EvaCompiler(std::shared_ptr<Global> global) :
        global(global),
        disassembler(std::make_unique<EvaDisassembler>(global)) {};

    /**
     * Main compiling API.
    */
    CodeObject* compile(const Exp& exp);

    /**
     * Disassembly method.
    */
    void disassembleBytecode() {
      disassembler->disassemble(codeObj);
    }
  
  private:
    /**
     * Recursive code generation.
    */
    void gen(const Exp& exp);

    /**
     * Code emission.
    */
    void emit(uint8_t code);

    /**
     * Allocates numeric constant in constant pool.
    */
    size_t numericConstIdx(double value);

    /**
     * Allocates boolean constant in constant pool.
    */
    size_t booleanConstIdx(bool value);

    /**
     * Allocates string constant in constant pool.
    */
    size_t stringConstIdx(const std::string& value);

    /**
     * Returns current bytecode offset.
    */
    size_t getCurrentOffset() { return codeObj->code.size(); }

    /**
     * Writing byte in specified offset.
    */
    void writeByteAtOffset(uint8_t byte, size_t offset) { codeObj->code[offset] = byte; }

    /**
     * Patches jump address.
    */
    void patchJumpAddres(size_t offset, uint16_t value);

    /**
     * Enter and exit block scope.
    */
    void scopeEnter() { codeObj->scopeLevel++; }
    void scopeExit ();
    bool isGlobalScope() { return codeObj->name == "main" && codeObj->scopeLevel == 1; }

    /**
     * Whether the expression is declaration.
    */
    bool isDeclaration(const Exp& exp) { return isVarDeclaration(exp); }

    /**
     * (var <name> <value>)
    */
    bool isVarDeclaration(const Exp& exp) { return isTaggedList(exp, "var"); }

    /**
     * Tagged list.
    */
    bool isTaggedList(const Exp& exp, const std::string& tag) {
      return exp.type == ExpType::LIST && exp.list[0].type == ExpType::SYMBOL
             && exp.list[0].string == tag;
    }

    /**
     * Pops variables from current scope and return the amount of them.
    */
    size_t getVarCountOnScopeExit() {
      size_t varsCount = 0;
      if (codeObj->locals.size() > 0) {
        while(codeObj->locals.back().scopeLevel == codeObj->scopeLevel) {
          codeObj->locals.pop_back();
          varsCount++;
        }
      }
      
      return varsCount;
    }
    
};

#endif