
#include "vm/Global.hpp"

void Global::set(size_t index, EvaValue& value) {
  if (index >= globals.size()) {
    DIE << "Global " << index << " doesn't exist";
  }
  globals[index].value = value;
}

void Global::addConst(const std::string& name, double value) {
  if (exists(name)) return;
  globals.push_back(GlobalVar{name, NUMBER(value)});
}

void Global::addNativeFunction(const std::string& name, std::function<void()> fn, size_t arity) {
  if (exists(name)) return;
  globals.push_back(GlobalVar{name, ALLOC_NATIVE(fn, name, arity)});
}

int Global::getGlobalIndex(const std::string& name) {
  if (globals.size() > 0) {
    for (int i = 0; i < (int)globals.size(); i++) {
      if (globals[i].name == name) {
        return i;
      }
    }
  }

  return -1; // Not found
}

void Global::define(const std::string& name) {
  if (getGlobalIndex(name) != -1) {
    return; // Already defined
  }

  globals.push_back(GlobalVar{name, NUMBER(0)});
}

