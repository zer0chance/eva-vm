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

  // Scope analysis.
  analyze(exp, nullptr);

  gen(exp);

  emit(OP_HALT);
}

void EvaCompiler::analyze(const Exp& exp, std::shared_ptr<Scope> scope) {
  if (exp.type == ExpType::SYMBOL) {
    // Variables
    if (exp.string != "true" && exp.string != "false") {
      scope->maybePromote(exp.string);
    }
  } else if (exp.type == ExpType::LIST) {
    // Lists
    auto tag = exp.list[0];

    if (tag.type == ExpType::SYMBOL) {
      auto op = tag.string;

      if (op == "begin") {
        auto newScope = std::make_shared<Scope>(
            scope == nullptr ? ScopeType::GLOBAL : ScopeType::BLOCK, scope);

        scopeInfo_[&exp] = newScope;

        for (int i = 1; i < exp.list.size(); ++i) {
          analyze(exp.list[i], newScope);
        }
      }

      else if (op == "var") {
        scope->addLocal(exp.list[1].string);
        analyze(exp.list[2], scope);
      }

      else if (op == "def") {
        auto fnName = exp.list[1].string;
        scope->addLocal(fnName);
        auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);
        scopeInfo_[&exp] = newScope;

        newScope->addLocal(fnName);

        // Params
        auto arity = exp.list[2].list.size();
        for (int i = 0; i < arity; ++i) {
          scope->addLocal(exp.list[2].list[i].string);
        }

        analyze(exp.list[3], newScope);
      }

      else if (op == "lambda") {
        auto newScope = std::make_shared<Scope>(ScopeType::FUNCTION, scope);
        scopeInfo_[&exp] = newScope;

        // Params
        auto arity = exp.list[1].list.size();
        for (int i = 0; i < arity; ++i) {
          scope->addLocal(exp.list[1].list[i].string);
        }

        analyze(exp.list[2], newScope);
      }

      else {
        for (int i = 1; i < exp.list.size(); ++i) {
          analyze(exp.list[i], scope);
        }
      }
    } else {
      for (int i = 0; i < exp.list.size(); ++i) {
        analyze(exp.list[i], scope);
      }
    }
  }
}

void EvaCompiler::compileFunction(const Exp& exp, const std::string& fnName,
                                  const Exp& params, const Exp& body) {
  auto scopeInfo = scopeInfo_.at(&exp);
  scopeStack_.push(scopeInfo);

  auto arity = params.list.size();
  // Save current CodeObject to restore it later.
  auto prevCodeObj = codeObj;

  // New function CodeObject.
  auto coValue = createCodeObjectValue(fnName, arity);
  codeObj = AS_CODE(coValue);

  // Put free and cell variables from the scope into the
  // cellNames of the codeObj.
  codeObj->freeCount = scopeInfo->free.size();

  codeObj->cellNames.reserve(scopeInfo->free.size() + scopeInfo->cells.size());

  codeObj->cellNames.insert(codeObj->cellNames.end(), scopeInfo->free.begin(), scopeInfo->free.end());
  codeObj->cellNames.insert(codeObj->cellNames.end(), scopeInfo->cells.begin(), scopeInfo->cells.end());

  // Store new CodeObject as a constant of the previous CodeObject.
  prevCodeObj->addConst(coValue);
  codeObj->addLocal(fnName);

  // Adding arguments to the list of locals.
  for (size_t i = 0; i < arity; i++) {
    auto argName = params.list[i].string;
    codeObj->addLocal(argName);

    // NOTE: if param is captured by cell, emit the code for it.
    // We also don't pop the param value at this case since 
    // OP_SCOPE_EXIT will pop it.
    auto cellIndex = codeObj->getCellIndex(argName);
    if (cellIndex != -1) {
      emit(OP_SET_CELL);
      emit(cellIndex);
    }
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

  if (scopeInfo->free.size() == 0) {
    // Creating a function and adding it as a constant to the
    // previous code object.
    auto fn = ALLOC_FUNCTION(codeObj);

    codeObj = prevCodeObj;

    codeObj->addConst(fn);

    emit(OP_CONST);
    emit(codeObj->constants.size() - 1);
  }
  
  // Closures:
  // 1. Load free variables to capture.
  // 2. Load code object for current funtcion.
  // 3. Make function.
  
  else {
    codeObj = prevCodeObj;

    // Load free vars.
    for (const auto& freeVar : scopeInfo->free) {
      emit(OP_LOAD_CELL);
      emit(prevCodeObj->getCellIndex(freeVar));
    }

    // Load code object.
    emit(OP_CONST);
    emit(codeObj->constants.size() - 1);

    // Create the function.
    emit(OP_MAKE_FUNCTION);

    // How many cells to capture.
    emit(scopeInfo->free.size());
  }
  
  scopeStack_.pop();
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

        auto opCodeGetter = scopeStack_.top()->getNameGetter(varName);
        emit(opCodeGetter);

        // Local variables
        if (opCodeGetter == OP_GET_LOCAL) {
          emit(codeObj->getLocalIndex(varName));
        }

        // Cell variables
        else if (opCodeGetter == OP_GET_CELL) {
          emit(codeObj->getCellIndex(varName));
        }

        // Global variables
        else {
          if (!global->exists(varName))
            DIE << "[EvaCompiler] Reference error: " << varName << std::endl;
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
          
          auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

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
          if (opCodeSetter == OP_SET_GLOBAL) {
            global->define(varName);
            emit(OP_SET_GLOBAL);
            emit(global->getGlobalIndex(varName));
          }

          // Cells
          else if (opCodeSetter == OP_SET_CELL) {
            codeObj->cellNames.push_back(varName);
            emit(OP_SET_CELL);
            emit(codeObj->cellNames.size() - 1);
            // Explicitly pop the value of initializer from the stack
            // since it was promoted to the heap.
            emit(OP_POP);
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

          auto opCodeSetter = scopeStack_.top()->getNameSetter(varName);

          // Value:
          gen(exp.list[2]);

          // Local variables
          if (opCodeSetter == OP_SET_LOCAL) {
            emit(OP_SET_LOCAL);
            emit(codeObj->getLocalIndex(varName));
          }

          // Cell variables
          else if (opCodeSetter == OP_SET_CELL) {
            emit(OP_SET_CELL);
            emit(codeObj->getCellIndex(varName));
          }

          // Global variables
          else {
            auto globalIndex = global->getGlobalIndex(varName);
            if (globalIndex == -1) {
              DIE << "Reference error: " << varName << " is not defined." << std::endl;
            }

            // Initializer
            gen(exp.list[2]);

            emit(OP_SET_GLOBAL);
            emit(globalIndex);
          }
        }

        else if (op == "begin") {
          scopeStack_.push(scopeInfo_.at(&exp));
          blockEnter();

          for (size_t i = 1; i < exp.list.size(); i++) {
            // The value of the last expression is kept on the stack as the
            // result of the block. Everything else must be popped from stack.
            bool isLast = i == exp.list.size() - 1;

            // Local variables of functions should not be popped.
            auto isLocalDeclaration = isDeclaration(exp.list[i]) && !isGlobalScope();
            gen(exp.list[i]);

            if (!isLast && !isLocalDeclaration) emit(OP_POP);
          }

          blockExit();
          scopeStack_.pop();
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

void EvaCompiler::blockExit() {
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

