#include "EvaValue.hpp"

std::string evaValueToTypeStr(const EvaValue& evaValue) {
    if (IS_NUMBER(evaValue)) {
        return "NUMBER";
    }
    if (IS_STRING(evaValue)) {
        return "STRING";
    }
    if (IS_CODE(evaValue)) {
        return "CODE";
    }
    DIE << "evaValueToTypeStr: unknown type";
    return "";
}

std::string evaValueToConstantString(const EvaValue& evaValue) {
    std::stringstream ss;
    if (IS_NUMBER(evaValue)) {
        ss << evaValue.value.number;
    } else if (IS_STRING(evaValue)) {
        ss << '"' << AS_CPPSTRING(evaValue) << '"';
    } else if (IS_CODE(evaValue)) {
        auto code = AS_CODE(evaValue);
        ss << "code: " << code << code->name;
    } else {
        DIE << "evaValueToConstantString: unknown type";
    }

    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const EvaValue& evaValue) {
    return os << "EvaValue (" << evaValueToTypeStr(evaValue) << "): "
              << evaValueToConstantString(evaValue);
}
