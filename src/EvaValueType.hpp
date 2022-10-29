
#ifndef SRC_EVAVALUETYPE_HPP
#define SRC_EVAVALUETYPE_HPP

#include <string>

/**
 * Types of values
*/
enum class EvaValueType {
    NUMBER,
    OBJECT
};

enum class ObjectType {
    STRING
};

struct Object {
    Object (ObjectType type) : type(type) {}
    ObjectType type;
};

struct StringObject : public Object {
    StringObject(const std::string &string) 
        : Object(ObjectType::STRING), string(string) {}
    std::string string;
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

// Type constructors:
#define NUMBER(value) EvaValue(EvaValueType::NUMBER, static_cast<double>(value))

#define ALLOC_STRING(value) \
    EvaValue(EvaValueType::OBJECT, (Object*) new StringObject(value))

// Accessors:
#define AS_NUMBER(evaValue) ((double)((evaValue).value.number))
#define AS_OBJECT(evaValue) ((Object*)((evaValue).value.object))
#define AS_STRING(evaValue) ((StringObject*)((evaValue).value.object))
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)

// Testers:
#define IS_NUMBER(evaValue) ((evaValue).type == EvaValueType::NUMBER)
#define IS_OBJECT(evaValue) ((evaValue).type == EvaValueType::OBJECT)

#define IS_OBJECT_TYPE(evaValue, objectType) \
    (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)

#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)

#endif
