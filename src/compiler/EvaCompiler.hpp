/**
 * Eva compiler.
*/

#ifndef SRC_COMPILER_EVACOMPILER_HPP
#define SRC_COMPILER_EVACOMPILER_HPP

#include "vm/Scope.hpp"
#include "vm/EvaValue.hpp"
#include "parser/EvaParser.hpp"
#include "disassembler/EvaDisassembler.hpp"

#include <map>
#include <stack>
#include <memory>

/**
 * Compiler class, emits bytecodes, records constant pools, vars, etc.
 */
class EvaCompiler {
  private:
    /**
     * Compiling code object.
     */
    CodeObject* codeObj;

    /**
     * Main entry point (function).
     */
    FunctionObject* main;

    /**
     * All code objects.
     */
    std::vector<CodeObject*> codeObjects_;

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

    /**
     * Scope info.
     */
    std::map<const Exp*, std::shared_ptr<Scope>> scopeInfo_;

    /**
     * Scope stack.
     */
    std::stack<std::shared_ptr<Scope>> scopeStack_;

  public:
    EvaCompiler(std::shared_ptr<Global> global) :
      global(global),
      disassembler(std::make_unique<EvaDisassembler>(global)) {};

    /**
     * Main compiling API.
     */
    void compile(const Exp& exp);

    /**
     * Disassembly method.
     */
    void disassembleBytecode() {
      for (auto& co_ : codeObjects_) {
        disassembler->disassemble(co_);
      }
    }

    /**
     * Getter for the main function.
     */
    FunctionObject* getMainFunction() { return this->main; }
  
  private:
    /**
     * Scope analysis.
     */
    void analyze(const Exp& exp, std::shared_ptr<Scope> scope);

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
    void blockEnter() { codeObj->scopeLevel++; }
    void blockExit ();

    /**
     * Check if we are in global scope.
     */
    bool isGlobalScope() { return codeObj->name == "main" && codeObj->scopeLevel == 1; }

    /**
     * Check if we are in function body.
     */
    bool isFunctionBody() { return codeObj->name != "main" && codeObj->scopeLevel == 1; }

    bool isDeclaration(const Exp& exp) { return isVarDeclaration(exp); }
    bool isVarDeclaration(const Exp& exp) { return isTaggedList(exp, "var"); }
    bool isBlock(const Exp& exp) { return isTaggedList(exp, "begin"); }
    bool isLambda(const Exp& exp) { return isTaggedList(exp, "lambda"); }

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

    /**
     * Helper function to create new CodeObject value.
     */
    EvaValue createCodeObjectValue(const std::string& name, size_t arity = 0) {
      auto coValue = ALLOC_CODE(name, arity);
      auto co = AS_CODE(coValue);
      codeObjects_.push_back(co);
      return coValue;
    }

    /**
     * Compiling a function body.
     */
    void compileFunction(const Exp& exp, const std::string& fnName, const Exp& params, const Exp& body);
};

#endif
