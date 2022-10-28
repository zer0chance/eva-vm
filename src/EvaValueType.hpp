
#ifndef SRC_EVAVALUETYPE_HPP
#define SRC_EVAVALUETYPE_HPP

/**
 * Types of values
*/
enum class EvaValueType {
    NUMBER
};

/*
* Value - union with a tag
*/
struct EvaValue {
    EvaValue() {};
    EvaValue(EvaValueType t, double v) :
        type(t) { value.number = v; };

    EvaValueType type;
    union {
        double number;
    } value;
};

// Type constructors:
#define NUMBER(value) EvaValue(EvaValueType::NUMBER, static_cast<double>(value))

// Accessors:
#define AS_NUMBER(evaValue) static_cast<double>((evaValue).value.number);

#endif
