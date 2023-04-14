#include "EvaValue.hpp"

std::string evaValueToTypeStr(const EvaValue& evaValue) {
  if (IS_NUMBER(evaValue)) {
    return "NUMBER";
  } else if (IS_BOOLEAN(evaValue)) {
    return "BOOLEAN";
  } else if (IS_STRING(evaValue)) {
    return "STRING";
  } else if (IS_CODE(evaValue)) {
    return "CODE";
  } else if (IS_NATIVE(evaValue)) {
    return "NATIVE";
  } else if (IS_FUNCTION(evaValue)) {
    return "FUNCTION";
  }
  DIE << "evaValueToTypeStr: unknown type";
  return "";
}

std::string evaValueToConstantString(const EvaValue& evaValue) {
  std::stringstream ss;
  if (IS_NUMBER(evaValue)) {
    ss << evaValue.value.number;
  } else if (IS_BOOLEAN(evaValue)) {
    ss << (evaValue.value.boolean ? "true" : "false");
  } else if (IS_STRING(evaValue)) {
    ss << '"' << AS_CPPSTRING(evaValue) << '"';
  } else if (IS_CODE(evaValue)) {
    auto code = AS_CODE(evaValue);
    ss << "code " << code << ": " << code->name << '/' << code->arity;
  } else if (IS_NATIVE(evaValue)) {
    auto fn = AS_NATIVE(evaValue);
    ss << fn->name << '/' << fn->arity;
  } else if (IS_FUNCTION(evaValue)) {
    auto fn = AS_FUNCTION(evaValue);
    ss << fn->co->name << '/' << fn->co->arity;
  } else {
    DIE << "evaValueToConstantString: unknown type";
  }

  return ss.str();
}

std::ostream& operator<<(std::ostream& os, const EvaValue& evaValue) {
  return os << "EvaValue (" << evaValueToTypeStr(evaValue) << "): "
    << evaValueToConstantString(evaValue);
}

int CodeObject::getLocalIndex(const std::string& name) {
  if (locals.size() > 0) {
    for (int i = 0; i < (int)locals.size(); i++) {
      if (locals[i].name == name) {
        return i;
      }
    }
  }

  return -1; // Not found
}
