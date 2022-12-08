cmake_minimum_required(VERSION 3.5)
project(eva-vm)

add_compile_options(-std=gnu++17 -Wall -Werror -ggdb3)

include_directories(src src/parser src/compiler)

add_executable(
  evm
  eva.cpp
  src/vm/EvaVM.cpp
  src/vm/OpCode.cpp
  src/vm/Global.cpp
  src/vm/EvaValue.cpp
  src/parser/EvaParser.cpp
  src/compiler/EvaCompiler.cpp
  src/disassembler/EvaDisassembler.cpp
)
