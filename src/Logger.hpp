
#ifndef SRC_LOGGER_HPP
#define SRC_LOGGER_HPP

#include <sstream>
#include <iomanip>
#include <iostream>

#define HEX(x) "0x" << std::setw(2) << std::setfill('0') << std::hex << (unsigned int)(x)

#define log(value) std::cout << #value << " = " << (value) << '\n'
#define log_bc(bytecode) std::cout << HEX(bytecode) << '\n'

class FatalErrorMessage : public std::basic_ostringstream<char> {
  public:
    ~FatalErrorMessage() {
        fprintf(stderr, "\nFatal error occured: %s", str().c_str());
        exit(EXIT_FAILURE);
    }
};

#define DIE FatalErrorMessage()

#endif
