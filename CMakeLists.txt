cmake_minimum_required(VERSION 3.5)
project(eva-vm)

add_compile_options(-std=c++2a)

include_directories(src)

add_executable(
  evm
  src/EvaVM.cpp
)
