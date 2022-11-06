/**
 * Eva compiler.
*/

#ifndef SRC_COMPILER_EVACOMPILER_HPP
#define SRC_COMPILER_EVACOMPILER_HPP

#include "parser/EvaParser.hpp"
#include "vm/EvaValue.hpp"

#include <map>

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

  public:
    EvaCompiler() = default;

    /**
     * Main compiling API.
    */
    CodeObject* compile(const Exp& exp);
  
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
};

#endif