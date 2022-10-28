cmake_minimum_required(VERSION 3.5)
project(eva-vm)

add_compile_options(-std=gnu++17 -Wall -ggdb3)

include_directories(src)

add_executable(
  evm
  eva.cpp
  src/EvaVM.cpp
)
