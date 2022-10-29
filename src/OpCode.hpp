
#ifndef SRC_OPCODE_HPP
#define SRC_OPCODE_HPP

// Stops the VM
#define OP_HALT  0x00

// Push a constant on stack
#define OP_CONST 0x01

// Binary math operations
#define OP_ADD   0x02
#define OP_SUB   0x03
#define OP_MUL   0x04
#define OP_DIV   0x05

#endif