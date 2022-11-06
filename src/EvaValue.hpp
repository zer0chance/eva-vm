
#ifndef SRC_EVAVALUETYPE_HPP
#define SRC_EVAVALUETYPE_HPP

#include "Logger.hpp"

#include <string>
#include <vector>

/**
 * Types of values
*/
enum class EvaValueType {
    NUMBER,
    OBJECT
};

enum class ObjectType {
    STRING,
    CODE
};

struct Object {
    Object (ObjectType type) : type(type) {}
    ObjectType type;
};

/*
* Value - union with a tag
*/
struct EvaValue {
    EvaValue() {};
    EvaValue(EvaValueType t, double v) :
        type(t) { value.number = v; };
    EvaValue(EvaValueType t, Object* v) :
        type(t) { value.object = v; };

    EvaValueType type;
    union {
        double number;
        Object* object;
    } value;
};

struct StringObject : public Object {
    StringObject(const std::string &string) 
        : Object(ObjectType::STRING), string(string) {}
    
    /**
     * String value.
    */
    std::string string;
};

/**
 * CodeObject contains compiled bytecode, locals and
 * other state needed to function execution.
*/
struct CodeObject : public Object {
    CodeObject(const std::string &name) 
        : Object(ObjectType::CODE), name(name) {}

    /**
     * Unit name (function name in most cases).
    */
    std::string name;

    /**
     * Bytecode instructions.
    */
    std::vector<uint8_t> code;

    /**
     * Constant pool.
    */
    std::vector<EvaValue> constants;
};

// Type constructors:
#define NUMBER(value) EvaValue(EvaValueType::NUMBER, static_cast<double>(value))

#define ALLOC_STRING(value) \
    EvaValue(EvaValueType::OBJECT, (Object*) new StringObject(value))
#define ALLOC_CODE(name) \
    EvaValue(EvaValueType::OBJECT, (Object*) new CodeObject(name))

// Accessors:
#define AS_NUMBER(evaValue)    ((double)((evaValue).value.number))
#define AS_OBJECT(evaValue)    ((Object*)((evaValue).value.object))
#define AS_STRING(evaValue)    ((StringObject*)((evaValue).value.object))
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)
#define AS_CODE(evaValue)      ((CodeObject*)((evaValue).value.object))

// Testers:
#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)

#define IS_OBJECT_TYPE(evaValue, objectType) \
    (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)

#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue)   IS_OBJECT_TYPE(evaValue, ObjectType::CODE)

std::string evaValueToTypeStr(const EvaValue& evaValue);
std::string evaValueToConstantString(const EvaValue& evaValue);
std::ostream& operator<<(std::ostream& os, const EvaValue& evaValue);

#endif
