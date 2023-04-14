#include "compiler/EvaCompiler.hpp"
#include "logging/Logger.hpp"
#include "vm/OpCode.hpp"

#define ALLOC_CONST(tester, converter, allocator, value) do {   \
    /* Checking if constant is already defined */               \
    for (size_t i = 0; i < codeObj->constants.size(); i++) {    \
        if (!tester(codeObj->constants[i])) continue;           \
                                                                \
        if (converter(codeObj->constants[i]) == value)          \
            return i;                                           \
    }                                                           \
                                                                \
    /* Defining new constant */                                 \
    codeObj->addConst(allocator(value));                        \
} while (0)

#define GEN_BINARY_OPERATOR(op) do {                            \
    gen(exp.list[1]);                                           \
    gen(exp.list[2]);                                           \
    emit(op);                                                   \
} while(0)

std::map<std::string, uint8_t> EvaCompiler::cmpOps = {
  {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5}
};

void EvaCompiler::compile(const Exp& exp) {
  // Allocate new code object and set it as a main function
  codeObj = AS_CODE(createCodeObjectValue("main"));
  main = AS_FUNCTION(ALLOC_FUNCTION(codeObj));

  gen(exp);

  emit(OP_HALT);
}

void EvaCompiler::compileFunction(const Exp& exp, const std::string& fnName,
                                  const Exp& params, const Exp& body) {
  auto arity = params.list.size();
  // Save current CodeObject to restore it later.
  auto prevCodeObj = codeObj;

  // New function CodeObject.
  auto coValue = createCodeObjectValue(fnName, arity);
  codeObj = AS_CODE(coValue);

  // Store new CodeObject as a constant of the previous CodeObject.
  prevCodeObj->addConst(coValue);
  codeObj->addLocal(fnName);

  // Adding arguments to the list of locals.
  for (size_t i = 0; i < arity; i++) {
    auto argName = params.list[i].string;
    codeObj->addLocal(argName);
  }

  // Compile function body in the new code object.
  gen(body);

  if (!isBlock(body)) {
    emit(OP_SCOPE_EXIT);
    // Number of params + function itself.
    emit(arity + 1);
  }

  // Explicit return to restore caller address.
  emit(OP_RETURN);

  // Creating a function and adding it as a constant to the
  // previous code object.
  auto fn = ALLOC_FUNCTION(codeObj);

  codeObj = prevCodeObj;
  codeObj->addConst(fn);

  emit(OP_CONST);
  emit(codeObj->constants.size() - 1);
}

#define FUNCTION_CALL(exp) do {                     \
    gen(exp.list[0]);                               \
    for (size_t i = 1; i < exp.list.size(); i++) {  \
        gen(exp.list[i]);                           \
    }                                               \
    emit(OP_CALL);                                  \
    emit(exp.list.size() - 1);                      \
    } while(0)

void EvaCompiler::gen(const Exp& exp) {
  switch (exp.type) {
    case ExpType::NUMBER:
      emit(OP_CONST);
      emit(numericConstIdx(exp.number));
      break;

    case ExpType::STRING:
      emit(OP_CONST);
      emit(stringConstIdx(exp.string));
      break;

    case ExpType::SYMBOL:
      // Booleans
      if (exp.string == "true" || exp.string == "false") {
        emit(OP_CONST);
        emit(booleanConstIdx(exp.string == "true" ? true : false));
      } else {
        // Variables
        auto varName = exp.string;

        // Local variables
        auto localIndex = codeObj->getLocalIndex(varName);
        if (localIndex != -1) {
          emit(OP_GET_LOCAL);
          emit(codeObj->getLocalIndex(varName));
        }

        // Global variables
        else {
          if (!global->exists(varName)) {
            DIE << "[EvaCompiler] Reference error: " << varName;
          }

          emit(OP_GET_GLOBAL);
          emit(global->getGlobalIndex(varName));
        }
      }
      break;

    case ExpType::LIST: {
      auto tag = exp.list[0];

      if (tag.type == ExpType::SYMBOL) {
        auto op = tag.string;

        // Binary math operations:
        if (op == "+") {
          GEN_BINARY_OPERATOR(OP_ADD);
        } else if (op == "-") {
          GEN_BINARY_OPERATOR(OP_SUB);
        } else if (op == "*") {
          GEN_BINARY_OPERATOR(OP_MUL);
        } else if (op == "/") {
          GEN_BINARY_OPERATOR(OP_DIV);
        }

        // Comparison operations:
        else if (cmpOps.count(op) != 0) {
          gen(exp.list[1]);
          gen(exp.list[2]);

          emit(OP_CMP);
          emit(cmpOps[op]);
        }

        // Branch instructions:
        // (if <test> <consequent> <alternate>)
        else if (op == "if") {
          // Emit <test>
          gen(exp.list[1]);

          // Else branch initialized with 0 and patched later
          // Takes 2 bytes
          emit(OP_JMP_IF_FALSE);
          emit(0);
          emit(0);

          auto elseJmpAddr = getCurrentOffset() - 2;

          // Emit <consequent>
          gen(exp.list[2]);
          emit(OP_JMP);
          emit(0);
          emit(0);

          auto endJmpAddr = getCurrentOffset() - 2;

          auto elseBranchAddr = getCurrentOffset();
          patchJumpAddres(elseJmpAddr, elseBranchAddr);

          // Emit alternate if we have it
          if (exp.list.size() == 4) {
            gen(exp.list[3]);
          }

          auto endBranchAddr = getCurrentOffset();
          patchJumpAddres(endJmpAddr, endBranchAddr);
        }

        // (while <test> <body>)
        else if (op == "while") {
          auto loopStartAddr = getCurrentOffset();

          // Emit <test>
          gen(exp.list[1]);

          emit(OP_JMP_IF_FALSE);

          // Emit 2 bytes address placeholder
          emit(0);
          emit(0);

          auto loopEndJmpAddress = getCurrentOffset() - 2;

          // Emit <body>
          gen(exp.list[2]);

          emit(OP_JMP);
          emit(0);
          emit(0);

          patchJumpAddres(getCurrentOffset() - 2, loopStartAddr);
          patchJumpAddres(loopEndJmpAddress, getCurrentOffset() + 1);
        }

        // for loop: (for <init> <test> <modifier> <body>)
        //
        // (for (var x 1) (< x 10) (set x (+ x 1)) (print x))
        else if (op == "for") {
          // Emit <init>
          // FIXME: make init to define variables in local scope, not global.
          gen(exp.list[1]);

          auto loopStartAddr = getCurrentOffset();

          // Emit <test>
          gen(exp.list[2]);

          emit(OP_JMP_IF_FALSE);

          // Emit 2 bytes address placeholder
          emit(0);
          emit(0);

          auto loopEndJmpAddress = getCurrentOffset() - 2;

          // Emit <body>
          // Can be empty loop with no body
          if (exp.list.size() == 5) {
            gen(exp.list[4]);
          }

          // FIXME - Initializer of the for loop creates global
          // variables. Poping it's value before calling modifier.
          emit(OP_POP);
          // Emit <modifier>
          gen(exp.list[3]);

          emit(OP_JMP);
          emit(0);
          emit(0);

          patchJumpAddres(getCurrentOffset() - 2, loopStartAddr);
          patchJumpAddres(loopEndJmpAddress, getCurrentOffset());
        }

        else if (op == "var") {
          auto varName = exp.list[1].string;   

          // Initializer or lambda function
          if (isLambda(exp.list[2])) {
            compileFunction(exp.list[2],
                varName, 
                exp.list[2].list[1],
                exp.list[2].list[2]);
          } else {
            gen(exp.list[2]);
          }

          // Global variables
          if (isGlobalScope()) {
            global->define(varName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(varName));
          }

          // Local variables
          else {
            codeObj->addLocal(varName);
            emit(OP_SET_LOCAL);
            emit(codeObj->getLocalIndex(varName));
          }
        }

        else if (op == "set") {
          auto varName = exp.list[1].string;

          // Local variables
          auto localIndex = codeObj->getLocalIndex(varName);
          if (localIndex != -1) {
            emit(OP_SET_LOCAL);
            emit(localIndex);
          }

          // Global variables
          else {
            auto globalIndex = global->getGlobalIndex(varName);
            if (globalIndex == -1) {
              DIE << "Reference error: " << varName << " is not defined.";
            }

            // Initializer
            gen(exp.list[2]);

            emit(OP_SET_GLOBAL);
            emit(globalIndex);
          }
        }

        else if (op == "begin") {
          scopeEnter();
          for (size_t i = 1; i < exp.list.size(); i++) {
            // The value of the last expression is kept on the stack as the
            // result of the block. Everything else must be popped from stack.
            bool isLast = i == exp.list.size() - 1;

            // Local variables of functions should not be popped.
            auto isLocalDeclaration = isDeclaration(exp.list[i]) && !isGlobalScope();
            gen(exp.list[i]);

            if (!isLast && !isLocalDeclaration) emit(OP_POP);
          }
          scopeExit();
        }

        else if (op == "lambda") {
          compileFunction(exp, "lambda", exp.list[1], exp.list[2]);
        }

        else if (op == "def") {
          auto fnName = exp.list[1].string;

          compileFunction(exp, fnName, exp.list[2], exp.list[3]);

          if (isGlobalScope()) {
            global->define(fnName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(fnName));
          } else {
            codeObj->addLocal(fnName);
            emit(OP_SET_LOCAL);
            emit(codeObj->getLocalIndex(fnName));
          }
        }

        // Everything else is treated as a function call.
        else {
          FUNCTION_CALL(exp);
        }
      }

      // Lambda function calls: ((lambda (x) (* x x)) 2)
      else {
        FUNCTION_CALL(exp);
      }

      break;
    }

    default:
      DIE << "Unknown expression type met, must be a parser error.";
  }
}

size_t EvaCompiler::numericConstIdx(double value) {
  ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
  return codeObj->constants.size() - 1;
}

size_t EvaCompiler::booleanConstIdx(bool value) {
  ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
  return codeObj->constants.size() - 1;
}

size_t EvaCompiler::stringConstIdx(const std::string& value) {
  ALLOC_CONST(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
  return codeObj->constants.size() - 1;
}

void EvaCompiler::emit(uint8_t code) {
  codeObj->code.push_back(code);
}

void EvaCompiler::patchJumpAddres(size_t offset, uint16_t value) {
  writeByteAtOffset(value >> 8 & 0xFF, offset);
  writeByteAtOffset(value & 0xFF, offset + 1);
}

void EvaCompiler::scopeExit() {
  // Pops all local variables that was define in this scope.
  auto varCount = getVarCountOnScopeExit();

  if (varCount > 0 || codeObj->arity > 0) {
    emit(OP_SCOPE_EXIT);

    if (isFunctionBody()) {
      // Adding amount of function arguments + function itself.
      varCount += codeObj->arity + 1;
    }
    emit(varCount);
  } 

  codeObj->scopeLevel--;
}

