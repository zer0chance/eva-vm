#include "EvaVM.hpp"
#include "OpCode.hpp"

#define BINARY_OP(op) do {              \
    auto op2 = AS_NUMBER(pop());        \
    auto op1 = AS_NUMBER(pop());        \
    push(NUMBER(op1 op op2));           \
} while (0)

#define COMPARE_VALUES(op, v1, v2) do { \
    bool res{};                         \
    switch (op) {                       \
    case 0:                             \
        res = v1 < v2;                  \
        break;                          \
    case 1:                             \
        res = v1 > v2;                  \
        break;                          \
    case 2:                             \
        res = v1 == v2;                 \
        break;                          \
    case 3:                             \
        res = v1 >= v2;                 \
        break;                          \
    case 4:                             \
        res = v1 <= v2;                 \
        break;                          \
    case 5:                             \
        res = v1 != v2;                 \
        break;                          \
    default:                            \
        DIE << "Bad comparison op!";    \
    }                                   \
    push(BOOLEAN(res));                 \
} while (0)

#define TO_ADDRESS(index) (&fn->co->code[index])

EvaValue EvaVM::exec(const std::string& program) {
  // 1. Parse AST
  auto ast = parser->parse("(begin " + program + ")");

  // 2. Compile AST to bytecode.
  compiler->compile(ast);
  fn = compiler->getMainFunction();

  ip = &fn->co->code[0];
  sp = &stack[0];
  bp = sp;

  // Debug assembly
  compiler->disassembleBytecode();

  return eval();
}

EvaValue EvaVM::eval() {
  while(true) {
    dumpStack();
    auto bytecode = next_byte();
    switch (bytecode) {
      case OP_HALT:
        return pop();

      case OP_CONST:
        push(fn->co->constants[next_byte()]);
        break;

      case OP_ADD: {
        auto op2 = pop();
        auto op1 = pop();

        if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
          auto n1 = AS_NUMBER(op1);
          auto n2 = AS_NUMBER(op2);
          push(NUMBER(n1 + n2));
        } else if (IS_STRING(op1) && IS_STRING(op2)) {
          auto s1 = AS_CPPSTRING(op1);
          auto s2 = AS_CPPSTRING(op2);
          push(ALLOC_STRING(s1 + s2));
        } else {
          DIE << "Incompatible types in addition \n";
        }
        break;
      }

      case OP_SUB:
        BINARY_OP(-);
        break;

      case OP_MUL:
        BINARY_OP(*);
        break;

      case OP_DIV:
        BINARY_OP(/);
        break;

      case OP_CMP: {
        auto op = next_byte();

        auto op2 = pop();
        auto op1 = pop();

        if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
          auto v1 = AS_NUMBER(op1);
          auto v2 = AS_NUMBER(op2);
          COMPARE_VALUES(op, v1, v2);
        } else if (IS_STRING(op1) && IS_STRING(op2)) {
          auto s1 = AS_CPPSTRING(op1);
          auto s2 = AS_CPPSTRING(op2);
          COMPARE_VALUES(op, s1, s2);
        }

        break;
      }

      case OP_JMP: {
        ip = TO_ADDRESS(next_short());
        break;
      }

      case OP_JMP_IF_FALSE: {
        auto cond = AS_BOOLEAN(pop()); // TODO: TO_BOOLEAN converter

        auto address = next_short();

        if (!cond) {
          ip = TO_ADDRESS(address);
        }
        break;
      }

      case OP_GET_GLOBAL: {
        auto globalIndex = next_byte();
        push(global->get(globalIndex).value);
        break;
      }

      case OP_SET_GLOBAL: {
        auto globalIndex = next_byte();
        auto value = peek();
        global->set(globalIndex, value);
        break;
      }

      case OP_GET_LOCAL: {
        auto localIndex = next_byte();
#if 0
        if (0 < localIndex && localIndex >= STACK_LIMIT) {
          DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
        }
#endif
        push(bp[localIndex]);
        break;
      }

      case OP_SET_LOCAL: {
        auto localIndex = next_byte();
        auto value = peek(0);
#if 0
        if (0 < localIndex && localIndex >= STACK_LIMIT) {
          DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
        }
#endif
        bp[localIndex] = value;
        break;
      }

      case OP_SCOPE_EXIT: {
        auto vars = next_byte();

        *(sp - 1 - vars) = peek();

        popN(vars);
        break;
      }

      case OP_POP: {
        pop();
        break;
      }

      case OP_CALL: {
        auto argc = next_byte();
        auto fnValue = peek(argc);

        // Native functions:
        if (IS_NATIVE(fnValue)) {
          AS_NATIVE(fnValue)->function();

          auto result = pop();

          popN(argc + 1); // Pop all arguments + function
          push(result);

          break;
        }

        // User functions:
        auto callee = AS_FUNCTION(fnValue);

        callStack.push(Frame{ip, bp, fn});

        // To access locals, etc.
        fn = callee;

        // Set the base (frame) pointer for the callee.
        bp = sp - argc - 1;
        // Jump to the function code.
        ip = &callee->co->code[0];

        break;
      }

      case OP_RETURN: {
        auto callerFrame = callStack.top();
        ip = callerFrame.ra;
        bp = callerFrame.bp;
        fn = callerFrame.fn;

        callStack.pop();
        break;
      }

      default:
        DIE << "Illegal bytecode: " << HEX(bytecode) << '\n';
        break;
    }
  }
}

void EvaVM::setGlobalVariables() {
  // Native functions:
  global->addNativeFunction(
      "native-square",
      [&](){
      auto x = AS_NUMBER(peek());
      push(NUMBER(x * x));
      },
      1 // argc
      );

  global->addNativeFunction(
      "max",
      [&](){
      auto x = AS_NUMBER(peek());
      auto y = AS_NUMBER(peek(1));
      push(NUMBER(x > y ? x : y));
      },
      2 // argc
      );

  global->addNativeFunction(
      "min",
      [&](){
      auto x = AS_NUMBER(peek());
      auto y = AS_NUMBER(peek(1));
      push(NUMBER(x < y ? x : y));
      },
      2 // argc
      );

  // GlobalVariables:
  global->addConst("VERSION", 2);
}

