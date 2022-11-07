
#ifndef SRC_OPCODE_HPP
#define SRC_OPCODE_HPP

using ByteCode = uint8_t;

// Stops the VM
constexpr ByteCode OP_HALT          = 0x00;

// Push a constant on stack
constexpr ByteCode OP_CONST         = 0x01;

// Binary math operations
constexpr ByteCode OP_ADD           = 0x02;
constexpr ByteCode OP_SUB           = 0x03;
constexpr ByteCode OP_MUL           = 0x04;
constexpr ByteCode OP_DIV           = 0x05;

// Comparison operation
constexpr ByteCode OP_CMP           = 0x06;

// Branch instructions
constexpr ByteCode OP_JMP           = 0x07;
constexpr ByteCode OP_JMP_IF_FALSE  = 0x08;

#endif