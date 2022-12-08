
#ifndef SRC_VM_GLOBAL_HPP
#define SRC_VM_GLOBAL_HPP

#include "vm/EvaValue.hpp"

/**
 * Gloal variable entry.
*/
struct GlobalVar {
    std::string name;
    EvaValue value;
};

/**
 * Global object.
*/
struct Global {
    /**
     * Returns a global at index.
    */
    GlobalVar& get(size_t index) { return globals[index]; }

    /**
     * Sets global value at index.
    */
    void set(size_t index, EvaValue& value);

    /**
     * Adding constant to global variables.
    */
    void addConst(const std::string& name, double value);

    /**
     * Get an index of the global value.
    */
    int getGlobalIndex(const std::string& name);

    /**
     * Check if the global variable exists.
    */
    bool exists(const std::string& name) { return getGlobalIndex(name) != -1; }

    /**
     * Defines a global with a name.
    */
    void define(const std::string& name);

  private:
    /**
     * Global variables and functions.
    */
    std::vector<GlobalVar> globals;
};

#endif
