
#ifndef SRC_OPCODE_HPP
#define SRC_OPCODE_HPP

#include <string>

using ByteCode = uint8_t;

enum ByteCodes : ByteCode {
  OP_HALT          = 0x00,

  // Push a constant on stack
  OP_CONST         = 0x01,

  // Binary math operations
  OP_ADD           = 0x02,
  OP_SUB           = 0x03,
  OP_MUL           = 0x04,
  OP_DIV           = 0x05,

  // Comparison operation
  OP_CMP           = 0x06,

  // Branch instructions
  OP_JMP           = 0x07,
  OP_JMP_IF_FALSE  = 0x08,

  // Globals
  OP_GET_GLOBAL    = 0x09,
  OP_SET_GLOBAL    = 0x0A,

  // Pop value from the stack
  OP_POP           = 0x0B,
  OP_SET_LOCAL     = 0x0C,
  OP_GET_LOCAL     = 0x0D,
  OP_SCOPE_EXIT    = 0x0E,

  // Function call
  OP_CALL          = 0x0F,
  OP_RETURN        = 0x10,

  // Cells
  OP_SET_CELL      = 0x11,
  OP_GET_CELL      = 0x12,
  OP_LOAD_CELL     = 0x13,

  // Function construction
  OP_MAKE_FUNCTION = 0x14,
};

std::string opcodeToString(ByteCode opcode);

#endif

