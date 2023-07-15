/**
 * Scope analysis.
 */
#ifndef SRC_VM_SCOPE_HPP
#define SRC_VM_SCOPE_HPP

#include "logging/Logger.hpp"
#include "vm/OpCode.hpp"

#include <set>
#include <map>
#include <map>

/**
 * Type of the scope.
 */
enum class ScopeType {
  GLOBAL,
  FUNCTION,
  BLOCK,
};

/**
 * Allocation type.
 */
enum class AllocType {
  GLOBAL,
  LOCAL,
  CELL,
};

struct Scope {
  Scope(ScopeType t, std::shared_ptr<Scope> p) :
    type(t), parent(p) {}

  /**
   * Type of the scope.
   */
  ScopeType type;

  /**
   * Parent scope.
   */
  std::shared_ptr<Scope> parent;

  /**
   * Allocation info.
   */
  std::map<std::string, AllocType> allocInfo;

  /**
   * Set of free variables.
   */
  std::set<std::string> free;

  /**
   * Set of own cells.
   */

  std::set<std::string> cells;
  /**
   * Register a local.
   */

  void addLocal(const std::string& name) {
    allocInfo[name] =
      type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;
  }

  /**
   * Register a free variable.
   */
  void addFree(const std::string& name) {
    free.insert(name);
    allocInfo[name] = AllocType::CELL;
  }

  /**
   * Register a cell.
   */
  void addCell(const std::string& name) {
    cells.insert(name);
    allocInfo[name] = AllocType::CELL;
  }

  void maybePromote(const std::string& name) {
    auto initAllocType = type == ScopeType::GLOBAL ? AllocType::GLOBAL : AllocType::LOCAL;

    if (allocInfo.count(name) != 0) {
      // We already have info about this var.
      initAllocType = allocInfo[name];
    }

    auto [ownerScope, allocType] = resolve(name, initAllocType);

    // Update the alloc type based on resolution.
    allocInfo[name] = allocType;

    // If resolved as a cell, promote it to heap.
    if (allocType == AllocType::CELL) {
      promote(name, ownerScope);
    }
  }

  /**
   * Resovle the variable in the Scope chain.
   *
   * Initially all variable are threated as a local, however if during the
   * resolution we passed local scope boundary, it is free and should be
   * promoted to the cell unless it is global.
   */
  std::pair<Scope*, AllocType> resolve(const std::string& name, AllocType allocType) {
    if (allocInfo.count(name) != 0) {
      // Found in the local scope.
      return std::make_pair(this, allocType);
    }

    // We crossed the boundary of the function and still have not found the variable.
    // Further resolution should be free.
    if (type == ScopeType::FUNCTION) {
      allocType = AllocType::CELL;
    }

    if (parent == nullptr) {
      DIE << "[Scope] Refference error: " << name << " is not defined." << std::endl;
    }

    if (parent->type == ScopeType::GLOBAL) {
      allocType = AllocType::GLOBAL;
    }

    return parent->resolve(name, allocType);
  }

  /**
   * Promote variable from local (stack) to cell (heap).
   */
  void promote(const std::string& name, Scope* ownerScope) {
    ownerScope->addCell(name);

    // Make the variable free at all scopes in the chain
    // to propogate it to our scope.
    auto scope = this;
    while (scope != ownerScope) {
      scope->addFree(name);
      scope = scope->parent.get();
    }
  }

  /**
   * Returns get opcode based on allocation type.
   */
  ByteCode getNameGetter(const std::string& name) {
    switch (allocInfo[name]) {
      case AllocType::GLOBAL:
        return OP_GET_GLOBAL;
      case AllocType::LOCAL:
        return OP_GET_LOCAL;
      case AllocType::CELL:
        return OP_GET_CELL;
    }
  }

  /**
   * Returns set opcode based on allocation type.
   */
  ByteCode getNameSetter(const std::string& name) {
    switch (allocInfo[name]) {
      case AllocType::GLOBAL:
        return OP_SET_GLOBAL;
      case AllocType::LOCAL:
        return OP_SET_LOCAL;
      case AllocType::CELL:
        return OP_SET_CELL;
    }
  }
};

#endif

