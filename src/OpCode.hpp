
#ifndef SRC_OPCODE_HPP
#define SRC_OPCODE_HPP

using bytecode = uint8_t;

// Stops the VM
constexpr bytecode OP_HALT  = 0x00;

// Push a constant on stack
constexpr bytecode OP_CONST = 0x01;

// Binary math operations
constexpr bytecode OP_ADD   = 0x02;
constexpr bytecode OP_SUB   = 0x03;
constexpr bytecode OP_MUL   = 0x04;
constexpr bytecode OP_DIV   = 0x05;

#endif