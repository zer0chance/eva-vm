
#ifndef SRC_EVAVALUETYPE_HPP
#define SRC_EVAVALUETYPE_HPP

#include "logging/Logger.hpp"

#include <string>
#include <vector>
#include <functional>

/**
 * Types of values
 */
enum class EvaValueType {
  NUMBER,
  BOOLEAN,
  OBJECT
};

enum class ObjectType {
  STRING,
  CODE,
  NATIVE,
  FUNCTION
};

struct Object {
  Object (ObjectType type) : type(type) {}
  ObjectType type;
};

/**
 * Value - union with a tag
 */
struct EvaValue {
  EvaValue() {};
  EvaValue(EvaValueType t, bool v) :
    type(t) { value.boolean = v; };
  EvaValue(EvaValueType t, double v) :
    type(t) { value.number = v; };
  EvaValue(EvaValueType t, Object* v) :
    type(t) { value.object = v; };

  EvaValueType type;
  union {
    double number;
    bool boolean;
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

struct LocalVar {
  std::string name;
  size_t scopeLevel;
};

/**
 * CodeObject contains compiled bytecode, locals and
 * other state needed to function execution.
 */
struct CodeObject : public Object {
  CodeObject(const std::string &name, size_t arity) 
    : Object(ObjectType::CODE), name(name), arity(arity) {}

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

  /**
   * Current scope level.
   */
  size_t scopeLevel = 0;

  /**
   * Local variables and functions.
   */
  std::vector<LocalVar> locals;

  /**
   * Amount of arguments.
   */
  size_t arity;

  /**
   * Defines new local variable.
   */
  void addLocal(const std::string& name) {
    locals.push_back({name, scopeLevel});
  }

  /**
   * Creates new constant.
   */
  void addConst(const EvaValue& value) {
    constants.push_back(value);
  }

  /**
   * Returns inxed of the local variabel or -1 if not found.
   */
  int getLocalIndex(const std::string& name);
};

using NativeFun = std::function<void()>;

struct NativeObject : public Object {
  NativeObject(NativeFun fn, const std::string& name, size_t arity) 
    : Object(ObjectType::NATIVE), 
    function(fn),
    name(name),
    arity(arity) {}

  /**
   * Function handler.
   */
  NativeFun function;

  /**
   * Native name.
   */
  std::string name;

  /**
   * Amount of arguments.
   */
  size_t arity;
};

struct FunctionObject : public Object {
  FunctionObject(CodeObject* co) 
    : Object(ObjectType::FUNCTION), co(co) {}

  /**
   * Reference to the code object that contais
   * functions code and locals.
   */
  CodeObject* co;

  /**
   * Native name.
   */
  std::string name;
};

// Type constructors:
#define NUMBER(value)  EvaValue(EvaValueType::NUMBER, static_cast<double>(value))
#define BOOLEAN(value) EvaValue(EvaValueType::BOOLEAN, static_cast<bool>(value))

#define ALLOC_STRING(value) \
  EvaValue(EvaValueType::OBJECT, (Object*) new StringObject(value))
#define ALLOC_CODE(name, arity) \
  EvaValue(EvaValueType::OBJECT, (Object*) new CodeObject(name, arity))
#define ALLOC_NATIVE(fn, name, arity) \
  EvaValue(EvaValueType::OBJECT, (Object*) new NativeObject(fn, name, arity))
#define ALLOC_FUNCTION(co) \
  EvaValue(EvaValueType::OBJECT, (Object*) new FunctionObject(co))

// Accessors:
#define AS_NUMBER(evaValue)    ((double)((evaValue).value.number))
#define AS_BOOLEAN(evaValue)   ((bool)((evaValue).value.boolean))
#define AS_OBJECT(evaValue)    ((Object*)((evaValue).value.object))
#define AS_STRING(evaValue)    ((StringObject*)((evaValue).value.object))
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue)->string)
#define AS_CODE(evaValue)      ((CodeObject*)((evaValue).value.object))
#define AS_NATIVE(evaValue)    ((NativeObject*)((evaValue).value.object))
#define AS_FUNCTION(evaValue)  ((FunctionObject*)((evaValue).value.object))

// Testers:
#define IS_NUMBER(evaValue)    ((evaValue).type == EvaValueType::NUMBER)
#define IS_BOOLEAN(evaValue)   ((evaValue).type == EvaValueType::BOOLEAN)
#define IS_OBJECT(evaValue)    ((evaValue).type == EvaValueType::OBJECT)

#define IS_OBJECT_TYPE(evaValue, objectType) \
  (IS_OBJECT(evaValue) && AS_OBJECT(evaValue)->type == objectType)

#define IS_STRING(evaValue)    IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue)      IS_OBJECT_TYPE(evaValue, ObjectType::CODE)
#define IS_NATIVE(evaValue)    IS_OBJECT_TYPE(evaValue, ObjectType::NATIVE)
#define IS_FUNCTION(evaValue)  IS_OBJECT_TYPE(evaValue, ObjectType::FUNCTION)

std::string evaValueToTypeStr(const EvaValue& evaValue);
std::string evaValueToConstantString(const EvaValue& evaValue);
std::ostream& operator<<(std::ostream& os, const EvaValue& evaValue);

#endif

