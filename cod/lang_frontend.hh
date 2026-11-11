//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <llvm/ADT/ImmutableMap.h>
#include <clang/Basic/Stack.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <compare>
#include <cstdint>
#include <deque>
#include <fstream>
#include <exception>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <string_view>
#include <variant>

namespace cod::language {

struct Module;
struct Procedure;
struct CaptureReplay;
struct Conformance {
  Type data, interface;
  bool operator==(const Conformance &) const = default;
};
struct StorageView {
  llvm::Value *address;
  std::optional<size_t> scope;
  std::set<size_t> cells; // Possible lexical allocations, for definite initialization.
  bool initialized_alternatives = false; // Other targets are initialized, but have no exact allocation identity.
};
struct Value {
  Type type = Type::void_type;
  llvm::Value *native = nullptr; // Procedure bindings are compile-time metadata.
  std::shared_ptr<const Procedure> procedure;
  const Module *owner = nullptr;
  std::optional<size_t> region_id = {};
  std::optional<size_t> holding_id = {};
  std::vector<Conformance> conformances = {}; // Selected type facts carried by a borrowed view, never runtime metadata.
  std::shared_ptr<const std::vector<Value>> arguments = {}; // WPO argument pack; no user allocation or runtime container.
  llvm::Value *exception_category = nullptr; // Propagation context, separate from the scalar payload.
  bool read_only = false;
  std::optional<StorageView> storage = {};
  std::optional<size_t> pointer = {}; // Lexical binding, distinct from its current referent.
  bool nullable = false;
  bool pending_owner = false;
  std::optional<std::pair<size_t, bool>> nonnull_when = {};
  std::optional<LLVMEmission::Place> reference_field = {};
  bool required_reference = false;
  bool pointer_view = false;
  llvm::Value *referent = nullptr; // Stable object expression identity across lexical pointer loads.
};
using Bindings = llvm::ImmutableMap<size_t, const Value *>;
struct Environment {
  Bindings bindings{nullptr};
  Bindings captured_bindings{nullptr};
  bool captured_context = false;
  std::shared_ptr<CaptureReplay> replay;

  const Value *lookup(size_t id) const {
    const auto found = bindings.lookup(id);
    return found ? *found : nullptr;
  }
  bool captured(size_t id, const Value *value) const {
    const auto found = captured_bindings.lookup(id);
    return found && *found == value;
  }
};
struct CaptureReplay {
  Environment caller;
  Bindings captured_values{nullptr};
  std::shared_ptr<CaptureReplay> defining_replay;
  std::set<size_t> used;
};
struct Locals {
  std::map<std::string, Value> visible;
  std::set<std::string> declared;
  std::map<std::string, Location> locations;

  Locals nested() const { return {visible, {}, locations}; }
  bool declare(const std::string &name, Value value, const Location &at) {
    if (!declared.insert(name).second) return false;
    visible.insert_or_assign(name, std::move(value));
    locations.insert_or_assign(name, at);
    return true;
  }
};
using ImplementationContext = std::vector<const Module *>;
struct Method {
  Form *form;
  const Module *lexical;
  ImplementationContext implementation{}; // Source-to-receiver interpretation, entirely compile-time.
  bool operator==(const Method &) const = default;
};
struct CallableDeclaration {
  const Form *form;
  const Module *lexical;
  bool operator==(const CallableDeclaration &) const = default;
};
struct Procedure {
  Form *form = nullptr;
  const Module *lexical = nullptr;
  std::string builtin;
  Environment captures;
  ImplementationContext implementation{};
  std::vector<CallableDeclaration> declarations;
};
using MethodOrigin = std::pair<const Module *, std::string>;
struct CallableIdentity {
  const Module *module;
  // Declarations/explicit unions use their defining name. Imported identities
  // use a receiver-local ordinal, independent of every alias exposing them.
  std::variant<std::string, size_t> definition;
  auto operator<=>(const CallableIdentity &) const = default;
};
struct CallableInformation {
  std::vector<Method> methods;
  std::set<MethodOrigin> origins;
  bool implicit = false;
  std::set<CallableIdentity> identities;
  std::vector<CallableDeclaration> declarations;
};
struct Callable {
  std::vector<Method> methods;
  std::set<MethodOrigin> origins;
  std::set<CallableIdentity> members; // Receiver-local virtual union, by identity.
  std::vector<CallableDeclaration> declarations; // Signatures promise behavior; only methods have bodies.
};
struct GlobalIdentity {
  MethodOrigin origin;
  bool operator==(const GlobalIdentity &) const = default;
};
using PublishedIdentity = std::variant<size_t, Type, CallableIdentity, GlobalIdentity>;
// Each module owns its selected semantic information. Declaration identities
// can be shared with other worlds; the selection containers are never shared.
// Native values here describe the demanded program, not a runtime world ABI.
struct ModuleWorld {
  std::map<std::string, size_t> symbols;
  std::map<std::string, Form *> symbol_aliases;
  std::set<std::string> available_symbols;
  std::map<CallableIdentity, Callable> callables;
  std::map<CallableIdentity, CallableIdentity> imported_callables;
  std::map<std::string, CallableIdentity> callable_aliases;
  std::map<std::string, CallableIdentity> associated_aliases;
  std::map<std::string, CallableIdentity> local_callables;
  std::map<std::string, bool> explicit_method_imports;
  std::set<std::string> lexical_names;
  std::map<std::string, PublishedIdentity> exports;
  std::map<std::string, Type> types;
  std::set<size_t> imported_data;
  std::vector<Conformance> conformances;
  std::vector<Method> conversions;
  std::set<const Module *> unit_sources;
  std::set<size_t> selected_units;
  std::map<MethodOrigin, Value> global_values;
  std::map<std::string, MethodOrigin> global_aliases;
  std::map<const Form *, Value> procedures;
  std::map<size_t, Value> symbolic_resources;
};
struct Module {
  std::string path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, Module *> dependencies;
  ModuleWorld world;
  enum class State { registered, executing, complete } state = State::registered;
  bool builtin = false;
};
struct Compilation {
  std::vector<std::string> scanned_modules;
  std::vector<std::string> execution_order;
  std::vector<std::string> warnings;
  std::string llvm_ir;
  std::string bitcode;
  bool uses_regions = false;
};
enum class Warning { unit_conversion, multi_alias_extension, shadow };
struct WarningOptions {
  bool as_errors = false;
  std::set<Warning> errors;
};

class Compiler {
  // Shared compiler snapshots retain binding values until lowering finishes.
  // The factory outlives every map; none of this storage enters the program.
  Bindings::Factory binding_factory_{false};
  std::deque<Value> binding_values_;
  std::map<std::string, std::unique_ptr<Module>> modules_;
  std::vector<std::string> symbol_names_;
  std::map<size_t, Type> symbol_types_;
  std::map<size_t, CallableDeclaration> symbol_declarations_;
  struct BindingCheck { Environment environment; Location at; };
  std::vector<BindingCheck> binding_checks_;
  std::vector<std::string> scanned_;
  std::vector<std::string> execution_order_;
  std::set<const Form *> active_calls_;
  struct CallSite { const Form *form; Location at; };
  std::vector<CallSite> call_path_;
  ImplementationContext implementation_context_;
  std::vector<Conformance> caller_conformances_;
  struct TypeContext {
    const Module *lexical = nullptr;
    std::map<std::string, Type> bindings;
  } type_context_;
  struct Signature {
    std::map<std::string, Type> bindings;
    std::vector<Type> parameters;
    std::optional<Type> result;
  };
  std::vector<std::unique_ptr<Form>> mixed_forms_;
  struct Product {
    Form *form;
    Module *module;
    std::vector<Parameter> fields;
    bool resolved = false;
  };
  std::vector<Product> products_;
  struct Abstract {
    Form *form;
    Module *module;
    std::optional<Type> parent;
    enum State { unresolved, resolving, complete } state = unresolved;
  };
  std::vector<Abstract> abstracts_;
  struct Array {
    Form *form;
    Module *module;
    Type element, index;
    bool resolved = false;
  };
  std::vector<Array> arrays_;
  enum class Presence { absent, present, conditional };
  struct Region {
    llvm::Value *owner;
    Location acquired;
    bool live = true;
    std::set<size_t> origins;
    bool symbolic = false;
    std::optional<Type> root = {};
  };
  std::vector<Region> regions_;
  struct Holding {
    size_t region;
    bool module_lifetime;
    llvm::Value *slot;
    Presence presence = Presence::present;
    bool movable = false;
  };
  std::vector<Holding> holdings_;
  struct LocalCell { size_t scope; Location at; bool initialized = false; };
  // The slot is the runtime address; the view carries checked owner/type/access
  // constraints and initialization facts, not an exact runtime target identity.
  struct Pointer { llvm::Value *slot; Value view; bool read_only = false; bool fixed_binding = false; bool required = false; Location at; };
  std::vector<LocalCell> cells_;
  std::vector<Pointer> pointers_;
  struct LoopPointers { size_t boundary; std::vector<bool> initialized; };
  std::vector<LoopPointers> loop_pointers_;
  std::vector<bool> scopes_{true};
  size_t local_scope_ = 0;
  bool module_top_level_ = false;
  bool region_initializer_ = false;
  bool npe_allowed_ = false;
  bool regional_runtime_ = false;
  size_t next_unit_ = 0;
  LLVMEmission ir_;
  WarningOptions warning_options_;
  std::vector<std::string> warnings_;
  bool scope_rebound_ = false; // Includes bindings with no native TLS slot.
  struct EndPath {};
  struct FlowState {
    std::vector<bool> live;
    std::vector<Presence> holdings;
    std::vector<bool> initialized;
    std::vector<Value> pointers;
  };
  struct ConversionPoint {
    LLVMEmission::ConversionPoint native;
    FlowState regions;
    bool raw_view = false;
  };
  ConversionPoint conversion_point() { return {ir_.conversion_point(), flow_state()}; }
  struct ExceptionalEdge { Value error; llvm::BasicBlock *from; FlowState regions; };
  struct Landing { llvm::BasicBlock *block = nullptr; std::vector<ExceptionalEdge> incoming; };
  struct HandlerScope {
    size_t cleanup, holding_begin;
    std::array<Landing, 4> landing;
  };
  std::vector<HandlerScope *> handlers_;
  std::optional<Value> caught_exception_;

  static Type exception_type(size_t category) {
    static const std::array names{"SystemException", "OutOfMemory", "RegionCapacityExceeded", "InvalidCapacity", "NullPointerException"};
    return {Type::exception, names.at(category), category};
  }
  void require_npe_handler(const Location &at) {
    if (!npe_allowed_)
      fail(at, "checked NullPointerException requires a local catch or throws NullPointerException on the immediate func/proc; module top level must catch it");
  }
  bool declares_npe(const std::vector<Type> &types, const Module &module, const Location &at) {
    for (const auto &type : types)
      if (resolve(type, module, at) != exception_type(4))
        fail(at, "throws requires the NullPointerException identity from std:exceptions");
    return !types.empty();
  }
  void check_nullable(const Value &value, const Location &at) {
    if (!value.nullable) return;
    if (value.pending_owner && value.pointer && std::any_of(loop_pointers_.begin(), loop_pointers_.end(),
        [&](const LoopPointers &loop) { return *value.pointer < loop.boundary; }))
      fail(at, "establish a pointer owner before a repeated loop dereference");
    require_npe_handler(at);
    ir_.null_check(value.storage ? value.storage->address : value.native);
    if (value.pending_owner) { ir_.unreachable(); throw EndPath{}; }
  }
  size_t new_region(llvm::Value *owner, const Location &at, std::set<size_t> origins = {}) {
    const auto id = regions_.size();
    if (origins.empty()) origins.insert(id);
    regions_.push_back({owner, at, true, std::move(origins)});
    return id;
  }
  FlowState flow_state() const {
    FlowState result;
    for (const auto &region : regions_) result.live.push_back(region.live);
    for (const auto &holding : holdings_) result.holdings.push_back(holding.presence);
    for (const auto &cell : cells_) result.initialized.push_back(cell.initialized);
    for (const auto &pointer : pointers_) result.pointers.push_back(pointer.view);
    return result;
  }
  void restore_flow(const FlowState &state) {
    for (size_t i = 0; i < regions_.size(); ++i) regions_[i].live = i < state.live.size() && state.live[i];
    for (size_t i = 0; i < holdings_.size(); ++i)
      holdings_[i].presence = i < state.holdings.size() ? state.holdings[i] : Presence::absent;
    for (size_t i = 0; i < cells_.size(); ++i)
      cells_[i].initialized = i < state.initialized.size() && state.initialized[i];
    for (size_t i = 0; i < state.pointers.size(); ++i) pointers_[i].view = state.pointers[i];
  }
  void intersect_flow(FlowState &left, const FlowState &right) {
    left.live.resize(std::max(left.live.size(), right.live.size()), false);
    for (size_t i = 0; i < left.live.size(); ++i) left.live[i] = left.live[i] && i < right.live.size() && right.live[i];
    left.holdings.resize(std::max(left.holdings.size(), right.holdings.size()), Presence::absent);
    for (size_t i = 0; i < left.holdings.size(); ++i) {
      const auto other = i < right.holdings.size() ? right.holdings[i] : Presence::absent;
      if (left.holdings[i] != other) left.holdings[i] = Presence::conditional;
    }
    left.initialized.resize(std::max(left.initialized.size(), right.initialized.size()), false);
    for (size_t i = 0; i < left.initialized.size(); ++i)
      left.initialized[i] = left.initialized[i] && i < right.initialized.size() && right.initialized[i];
    for (size_t i = 0; i < std::min(left.pointers.size(), right.pointers.size()); ++i) {
      auto &target = left.pointers[i];
      const auto &other = right.pointers[i];
      if (target.pending_owner && !other.pending_owner) {
        target = other;
        target.nullable = true;
      } else if (!target.pending_owner && !other.pending_owner && storage_owner(target) != storage_owner(other)) {
        fail(pointers_[i].at, "pointer paths establish different owners; the first non-null assignment must establish one consistent owner");
      }
      target.nullable = target.nullable || other.nullable;
      target.read_only = target.read_only || other.read_only;
      if (target.referent != other.referent) target.referent = nullptr;
      if (target.storage && other.storage) {
        target.storage->cells.insert(other.storage->cells.begin(), other.storage->cells.end());
        target.storage->initialized_alternatives |= other.storage->initialized_alternatives;
      }
    }
  }
  void close_state(FlowState &state, size_t begin, std::optional<size_t> result_owner = {}) {
    state.live.resize(regions_.size(), false);
    state.holdings.resize(holdings_.size(), Presence::absent);
    std::set<size_t> released;
    for (size_t i = begin; i < holdings_.size(); ++i) {
      const auto &holding = holdings_[i];
      if (holding.module_lifetime || state.holdings[i] == Presence::absent) continue;
      const auto &origins = regions_[holding.region].origins;
      released.insert(origins.begin(), origins.end());
      state.holdings[i] = Presence::absent;
    }
    for (size_t i = 0; i < regions_.size(); ++i) {
      if (result_owner == i) continue;
      const auto protected_owner = std::any_of(holdings_.begin(), holdings_.end(), [&](const Holding &holding) {
        return holding.region == i && state.holdings[&holding - holdings_.data()] == Presence::present;
      });
      if (!protected_owner && std::any_of(regions_[i].origins.begin(), regions_[i].origins.end(),
                                         [&](size_t origin) { return released.contains(origin); }))
        state.live[i] = false;
    }
  }
  Value hold_region(Value value, bool module_lifetime, bool movable) {
    value.holding_id = holdings_.size();
    holdings_.push_back({*value.region_id, module_lifetime, ir_.hold_region(value.native, module_lifetime),
                         Presence::present, movable});
    return value;
  }
  Value take_owner(Value value, const Location &at) {
    if (value.type.kind != Type::region || !value.type.owned || !value.holding_id)
      fail(at, "ownership transfer requires an owning region value; dynamic resources and borrows cannot be extracted");
    region_pointer(value, at);
    auto &holding = holdings_.at(*value.holding_id);
    if (!holding.movable) fail(at, "cannot transfer ownership out of a dynamic region binding");
    holding.presence = Presence::absent;
    ir_.relinquish_region(holding.slot);
    value.holding_id.reset();
    return value;
  }
  static Value borrowed(Value value) {
    value.type.owned = false;
    value.holding_id.reset();
    return value;
  }
  llvm::Value *exception_category(const Value &value) {
    return value.type.identity ? ir_.bits(Type::uint64, value.type.identity) : value.exception_category;
  }
  static bool may_be_npe(llvm::Value *category) {
    std::vector<const llvm::Value *> pending{category};
    std::set<const llvm::Value *> visited;
    while (!pending.empty()) {
      auto *next = pending.back(); pending.pop_back();
      if (!visited.insert(next).second) continue;
      if (const auto *constant = llvm::dyn_cast<llvm::ConstantInt>(next)) {
        if (constant->getZExtValue() == 4) return true;
      } else if (const auto *phi = llvm::dyn_cast<llvm::PHINode>(next)) {
        for (auto &incoming : phi->incoming_values()) pending.push_back(incoming.get());
      } else if (const auto *select = llvm::dyn_cast<llvm::SelectInst>(next)) {
        pending.push_back(select->getTrueValue()); pending.push_back(select->getFalseValue());
      } else return true;
    }
    return false;
  }
  void raise_exception(Value error) {
    ir_.dispatch_exception(exception_category(error), [&](size_t category) {
      if (handlers_.empty()) { ir_.uncaught(category, error.native); return; }
      auto &scope = *handlers_.back();
      auto &landing = scope.landing.at(category - 1);
      if (!landing.block) landing.block = ir_.label("exception.handler");
      auto state = flow_state();
      close_state(state, scope.holding_begin);
      auto incoming = error;
      incoming.exception_category = ir_.bits(Type::uint64, category);
      ir_.unwind(scope.cleanup);
      landing.incoming.push_back({incoming, ir_.here(), std::move(state)});
      ir_.jump(landing.block);
    });
  }

  void warn(Warning kind, const Location &at, std::string message) {
    const auto category = kind == Warning::unit_conversion ? "unit-conversion" :
                          kind == Warning::shadow ? "shadow" : "multi-alias-extension";
    const auto newline = message.find('\n');
    message.insert(newline == std::string::npos ? message.size() : newline,
                   " [-W" + std::string(category) + "]");
    if (warning_options_.as_errors || warning_options_.errors.contains(kind))
      fail(at, "warning treated as error: " + message);
    auto diagnostic = location_text(at) + ": warning: " + message;
    if (kind != Warning::shadow || std::find(warnings_.begin(), warnings_.end(), diagnostic) == warnings_.end())
      warnings_.push_back(std::move(diagnostic));
  }

  void declare_local(Locals &locals, const std::string &name, Value value,
                     const Module &module, const Location &at) {
    if (locals.declared.contains(name)) fail(at, "duplicate lexical value '" + name + "'");
    if (const auto previous = locals.locations.find(name); previous != locals.locations.end()) {
      warn(Warning::shadow, at, "local '" + name + "' shadows an outer binding\n" +
           location_text(previous->second) + ": note: shadowed binding declared here");
    } else if (module.world.callable_aliases.contains(name) || module.world.associated_aliases.contains(name)) {
      warn(Warning::shadow, at, "local '" + name + "' shadows a callable in module '" + module.path + "'");
    } else if (module.world.global_aliases.contains(name) || module.world.types.contains(name)) {
      warn(Warning::shadow, at, "local '" + name + "' shadows a module binding in '" + module.path + "'");
    }
    locals.declare(name, std::move(value), at);
  }

  static void require_user_type_name(const std::string &name, const Location &at) {
    static const std::set<std::string> reserved{"i64", "u8", "u32", "u64", "f32", "f64", "bool", "void", "region", "ref"};
    if (reserved.contains(name)) fail(at, "reserved scalar type name '" + name + "'");
    if (intrinsic_name(name) || expression_keyword(name) || module_keyword(name))
      fail(at, "reserved keyword '" + name + "' cannot be used as an ordinary name");
  }

  static Type reference(Type target) {
    Type type{Type::reference};
    type.target = std::make_shared<Type>(std::move(target));
    return type;
  }
  std::string region_note(std::optional<size_t> region, std::string_view message) const {
    if (!region) return {};
    return "\n" + location_text(regions_.at(*region).acquired) + ": note: " + std::string(message);
  }
  llvm::Value *region_pointer(const Value &value, const Location &at) {
    if (value.type.owned && (!value.holding_id || holdings_.at(*value.holding_id).presence != Presence::present))
      fail(at, "region owner has been consumed on a path reaching this use" +
                   region_note(value.region_id, "the region was acquired here"));
    if (!value.region_id || !regions_.at(*value.region_id).live)
      fail(at, "regional value requires a live owning region in this caller scope" +
                   region_note(value.region_id, "the referenced region was acquired here"));
    return regions_.at(*value.region_id).owner;
  }
  static bool local_bits(Type type) { return numeric(representation(type)) || type == Type::boolean || type.kind == Type::exception; }
  static std::optional<std::pair<bool, size_t>> storage_owner(const Value &value) {
    if (value.region_id) return std::pair(true, *value.region_id);
    if (value.storage && value.storage->scope) return std::pair(false, *value.storage->scope);
    return {};
  }
  void valid_view(const Value &value, const Location &at) {
    if (value.region_id) region_pointer(value, at);
    if (value.storage && value.storage->scope && !scopes_.at(*value.storage->scope))
      fail(at, "borrowed value would outlive its lexical storage owner");
  }
  LLVMEmission::Place place(const Value &value, const Location &at) {
    valid_view(value, at);
    check_nullable(value, at);
    if (!value.storage) fail(at, "operation requires addressable target storage");
    return {value.type, value.region_id ? region_pointer(value, at) : nullptr, value.storage->address};
  }
  Value target_view(Value value, const Location &at) {
    if (value.type.kind != Type::reference) return value;
    const auto target = *value.type.target;
    if (local_bits(target)) {
      value.storage = StorageView{ir_.nullable_address(region_pointer(value, at), value.native), {}, {}};
      value.native = nullptr;
    }
    value.type = target;
    return value;
  }
  Value null_target(Type type, std::optional<Value> owner, const Location &at) {
    Value value; value.type = type; value.nullable = true; value.pointer_view = true;
    value.pending_owner = !owner || !storage_owner(*owner);
    if (owner) value.region_id = owner->region_id;
    if (local_bits(type)) {
      value.storage = StorageView{ir_.null_pointer(), owner && owner->storage ? owner->storage->scope : std::nullopt, {}};
    } else if (type.kind == Type::regional || type.kind == Type::array || type.kind == Type::reference) {
      value.native = ir_.bits(Type::null_ref, 0);
    } else fail(at, "null requires a pointer to bits or regional data");
    return value;
  }
  Value read_value(Value value, const Location &at) {
    valid_view(value, at);
    if (value.type.kind != Type::reference) {
      check_nullable(value, at);
      value.nullable = false;
    }
    if (value.storage) {
      for (const auto cell : value.storage->cells)
        if (!cells_.at(cell).initialized)
          fail(at, "read of storage before definite initialization\n" + location_text(cells_[cell].at) +
                       ": note: local storage allocated here");
      const auto loaded = loaded_value(value.type, ir_.read(place(value, at)));
      value.native = loaded.native;
      value.exception_category = loaded.exception_category;
    }
    return value;
  }
  static Value bits_value(Value value) {
    value.storage.reset(); value.pointer.reset(); value.region_id.reset(); value.read_only = false; value.referent = nullptr; value.nullable = false; value.reference_field.reset(); value.nonnull_when.reset(); value.pointer_view = false;
    return value;
  }
  Value local_storage(Type type, const Location &at) {
    if (!local_bits(type)) fail(at, "make local requires a bits type; data objects must be allocated in a region");
    Value value; value.type = type;
    value.storage = StorageView{ir_.local_storage(type), local_scope_, {cells_.size()}};
    cells_.push_back({local_scope_, at});
    return value;
  }
  void write_bits(Value destination, Value source, const Location &at) {
    if (destination.read_only) fail(at, "cannot transfer into a read-only destination view");
    source = read_value(std::move(source), at);
    require(source.type, destination.type, at);
    ir_.write(place(destination, at), stored_value(source, destination.type, at));
    if (destination.storage->cells.size() == 1 && !destination.storage->initialized_alternatives)
      cells_.at(*destination.storage->cells.begin()).initialized = true;
  }
  Value pointer_value(size_t id, const Location &at) {
    auto value = pointers_.at(id).view;
    valid_view(value, at);
    auto *current = ir_.read_pointer(pointers_[id].slot, value.storage ? value.storage->address : value.native);
    if (value.storage) value.storage->address = current;
    else value.native = current;
    value.pointer = id;
    return value;
  }
  void declare_pointer(Locals &locals, const std::string &name, Value value,
                       const Module &module, const Location &at, bool read_only = false, bool fixed_binding = false, bool required = false) {
    value.pointer.reset();
    value.reference_field.reset();
    value.required_reference = false;
    value.pointer_view = true;
    valid_view(value, at);
    if (!storage_owner(value) && !value.pending_owner) fail(at, "ptr requires storage with a known owner");
    const auto id = pointers_.size();
    if (!value.referent) value.referent = value.storage ? value.storage->address : value.native;
    pointers_.push_back({ir_.pointer_storage(value.storage ? value.storage->address : value.native), value,
                         read_only || value.read_only, fixed_binding, required, at});
    value.pointer = id;
    declare_local(locals, name, std::move(value), module, at);
  }
  void close_holdings(size_t begin, const Value &result, const Location &at) {
    auto state = flow_state();
    close_state(state, begin, result.type.owned ? result.region_id : std::nullopt);
    if (result.region_id && !state.live.at(*result.region_id))
      fail(at, "regional result would outlive its owning scope" +
                   region_note(result.region_id, "the escaping value depends on this region acquisition"));
    restore_flow(state);
  }
  Value finish_scope(size_t holding_begin, size_t native_scope, Value result, const Location &at,
                     bool parent_module) {
    const bool owns = result.type.kind == Type::region && result.type.owned;
    if (owns) result = take_owner(result, at);
    close_holdings(holding_begin, result, at);
    ir_.restore(native_scope);
    if (owns) result = hold_region(std::move(result), parent_module, true);
    return result;
  }
  Type resolve_name(Type type, const Module &module, const Location &at) {
    if (type.kind != Type::named) return type;
    if (!type.target && type_context_.lexical == &module)
      if (const auto found = type_context_.bindings.find(type.name); found != type_context_.bindings.end())
        return found->second;
    const auto found = module.world.types.find(type.name);
    if (found == module.world.types.end()) fail(at, "unavailable type '" + type.name + "'");
    if (type.target) {
      if (found->second.kind != Type::unit) fail(at, "a quantity annotation requires a UoM identity");
      auto number = resolve_name(*type.target, module, at);
      if (!numeric(number) && number.kind != Type::parameter)
        fail(at, "a quantity requires a numeric representation");
      return quantity(found->second, std::move(number));
    }
    return found->second;
  }
  void resolve_abstract(Type type, const Location &at) {
    auto &definition = abstracts_.at(type.identity);
    if (definition.state == Abstract::complete) return;
    if (definition.state == Abstract::resolving) fail(at, "cyclic abstract type inheritance");
    definition.state = Abstract::resolving;
    if (definition.form->parent_type) {
      auto parent = resolve_name(*definition.form->parent_type, *definition.module, definition.form->at);
      if (parent.kind != Type::abstract_type) fail(definition.form->at, "an abstract parent must be an abstract type");
      resolve_abstract(parent, definition.form->at);
      definition.parent = parent;
    }
    definition.state = Abstract::complete;
  }
  bool nominal_subtype(Type actual, Type expected, const Location &at) {
    if (actual == expected) return true;
    if (actual.kind != Type::abstract_type || expected.kind != Type::abstract_type) return false;
    resolve_abstract(actual, at);
    const auto &parent = abstracts_.at(actual.identity).parent;
    return parent && nominal_subtype(*parent, expected, at);
  }
  bool subtype(Type actual, Type expected, const Module &visible, const Location &at) {
    if (nominal_subtype(actual, expected, at)) return true;
    if (actual.kind != Type::regional || expected.kind != Type::abstract_type) return false;
    const auto matches = [&](const Conformance &relation) {
      return relation.data == actual && nominal_subtype(relation.interface, expected, at);
    };
    return std::any_of(visible.world.conformances.begin(), visible.world.conformances.end(), matches) ||
           std::any_of(caller_conformances_.begin(), caller_conformances_.end(), matches);
  }
  static void append_conformance(std::vector<Conformance> &relations, Conformance relation) {
    if (std::find(relations.begin(), relations.end(), relation) == relations.end()) relations.push_back(relation);
  }
  static void merge_conformances(std::vector<Conformance> &into, const std::vector<Conformance> &from) {
    for (const auto &relation : from) append_conformance(into, relation);
  }
  bool referenced_data(Type type, std::set<size_t> &reachable) const {
    if ((type.kind == Type::reference || type.kind == Type::region) && type.target)
      return referenced_data(*type.target, reachable);
    if (type.kind != Type::regional || !reachable.insert(type.identity).second) return true;
    const auto &product = products_.at(type.identity);
    // Unknown layouts retain their facts; checking a fold must not demand them.
    if (!product.resolved) return false;
    return std::all_of(product.fields.begin(), product.fields.end(), [&](const auto &field) {
      return referenced_data(field.type, reachable);
    });
  }
  Value contextual(Value value, const Module &module) {
    if (value.type.kind == Type::regional || value.type.kind == Type::reference || value.type.kind == Type::array ||
        value.type.kind == Type::region) {
      merge_conformances(value.conformances, module.world.conformances);
      merge_conformances(value.conformances, caller_conformances_);
    }
    return value;
  }
  bool establishes(const Value &value, Conformance required, const Location &at) {
    return std::any_of(value.conformances.begin(), value.conformances.end(), [&](const Conformance &relation) {
      return relation.data == required.data && nominal_subtype(relation.interface, required.interface, at);
    });
  }
  bool accepts(const Value &actual, Type expected, const Module &visible, const Location &at) {
    if (actual.type == Type::never) return true;
    if (actual.type.kind == Type::region && expected.kind == Type::region) {
      if (expected.owned && !actual.type.owned) return false;
      return !expected.target || (actual.type.target &&
          (subtype(*actual.type.target, *expected.target, visible, at) ||
           establishes(actual, {*actual.type.target, *expected.target}, at)));
    }
    if (actual.type.kind == Type::procedure && expected.kind == Type::procedure) {
      if (!actual.type.throws_types.empty() && expected.throws_types.empty()) return false;
      auto provided = actual.type;
      provided.throws_types.clear(); expected.throws_types.clear();
      return provided == expected;
    }
    if (actual.type.kind == Type::exception && expected.kind == Type::exception && !expected.identity) return true;
    return subtype(actual.type, expected, visible, at) ||
           (actual.type.kind == Type::regional && establishes(actual, {actual.type, expected}, at));
  }
  Value admit(Value actual, Type expected, const Module &visible, const Location &at) {
    actual = contextual(std::move(actual), visible);
    if (!accepts(actual, expected, visible, at)) {
      auto message = "expected " + type_name(expected) + ", got " + type_name(actual.type);
      if (expected.kind == Type::abstract_type) {
        message += "\n" + location_text(at) + ": note: required interface is not established in module '" + visible.path + "'";
        for (const auto &relation : actual.conformances)
          if (relation.data == actual.type)
            message += "\n" + location_text(at) + ": note: available value guarantee is " + type_name(relation.interface);
      }
      fail(at, message);
    }
    if (actual.type.kind == Type::procedure) actual.type = expected;
    if (actual.type.kind == Type::region && !expected.owned) actual = borrowed(std::move(actual));
    return actual;
  }
  void declare_conformance(Module &module, const Form &form) {
    const auto data = resolve_name({Type::named, form.name}, module, form.at);
    if (data.kind != Type::regional) fail(form.at, "a data conformance requires a concrete regional data type");
    if (products_.at(data.identity).module != &module && !module.world.imported_data.contains(data.identity))
      fail(form.at, "declaring external data conformance requires import of the data type");
    const auto interface = resolve_name(*form.parent_type, module, form.at);
    if (interface.kind != Type::abstract_type) fail(form.at, "a data conformance requires an abstract interface");
    resolve_abstract(interface, form.at);
    const Conformance relation{data, interface};
    if (std::find(module.world.conformances.begin(), module.world.conformances.end(), relation) != module.world.conformances.end())
      fail(form.at, "repeated data conformance declaration is outside this prototype");
    module.world.conformances.push_back(relation);
  }
  Type resolve(Type type, const Module &module, const Location &at) {
    type = resolve_name(type, module, at);
    if (type.kind == Type::region && type.target) {
      auto root = resolve(*type.target, module, at);
      if (root.kind != Type::regional && root.kind != Type::abstract_type && root.kind != Type::parameter)
        fail(at, "a region root requires a data type or an abstract root interface");
      type.target = std::make_shared<Type>(std::move(root));
    }
    if (type.kind == Type::unit) fail(at, "a UoM identity requires a numeric representation, such as f64(" + type.name + ")");
    if (type.kind == Type::abstract_type) resolve_abstract(type, at);
    if (type.kind == Type::procedure) {
      if (!type.target) fail(at, "a procedure type requires its parameter and result signature");
      for (auto &parameter : type.parameters) {
        parameter = resolve(parameter, module, at);
        if (parameter == Type::void_type) fail(at, "a procedure parameter must have a value type");
      }
      type.target = std::make_shared<Type>(resolve(*type.target, module, at));
      if (declares_npe(type.throws_types, module, at)) type.throws_types = {exception_type(4)};
      return type;
    }
    if (type.kind == Type::reference) {
      auto target = resolve_name(*type.target, module, at);
      if (target.kind == Type::reference || target.kind == Type::region || target == Type::void_type ||
          target.kind == Type::procedure || target.kind == Type::abstract_type)
        fail(at, "a regional reference requires a bits or regional target type");
      if (target.kind != Type::regional) target = resolve(target, module, at);
      regional_runtime_ = true;
      return reference(target);
    }
    if (type.kind == Type::array) {
      auto &array = arrays_.at(type.identity);
      if (!array.resolved) {
        array.element = resolve_name(*array.form->result_type, *array.module, array.form->at);
        array.index = resolve_name(*array.form->index_type, *array.module, array.form->at);
        if (!numeric(array.element) && array.element != Type::boolean && array.element.kind != Type::quantity)
          fail(at, "initial regional arrays store scalar bits; use parallel arrays for product fields");
        if (!integral(array.index) && !(array.index.kind == Type::quantity && integral(representation(array.index))))
          fail(at, "an array index type requires an integer representation");
        array.resolved = true;
      }
      regional_runtime_ = true;
    }
    if (type.kind == Type::regional) {
      auto &product = products_.at(type.identity);
      if (!product.resolved) {
        std::set<std::string> fields;
        for (const auto &field : product.form->parameters) {
          if (!fields.insert(field.name).second) fail(field.at, "duplicate product field '" + field.name + "'");
          auto field_type = resolve_name(field.type, *product.module, field.at);
          if (field.declaration_kind == "val" && field_type.kind == Type::reference)
            fail(field.at, "val fields store direct bits; use ref or ptr for a stored reference");
          if (field.declaration_kind == "ptr" || field.declaration_kind == "ref") field_type = reference(field_type);
          if (field_type.kind == Type::procedure) fail(field.at, "procedure fields are outside the prototype");
          if (field_type.kind == Type::region || field_type.kind == Type::regional || field_type.kind == Type::array ||
              field_type.kind == Type::abstract_type)
            fail(field.at, "data object fields require a ref or ptr declaration");
          field_type = resolve(field_type, *product.module, field.at);
          product.fields.push_back({field.name, field_type, field.at, false, {}});
          product.fields.back().declaration_kind = field.declaration_kind;
        }
        regional_runtime_ = true;
        std::vector<Type> field_types;
        for (const auto &field : product.fields) field_types.push_back(field.type);
        ir_.record(type, field_types);
        product.resolved = true;
      }
    }
    return type;
  }
  Value scalar(Type type, llvm::Value *native, std::optional<size_t> region_id = {}) {
    return {type, native, {}, nullptr, region_id};
  }
  llvm::Value *stored_value(const Value &value, Type destination, const Location &at) {
    if (destination.kind == Type::reference && value.read_only)
      fail(at, "cannot store a read-only view in a writable reference field");
    if (destination.kind == Type::exception && !destination.identity)
      return ir_.store_exception(value.native, exception_category(value));
    return value.native;
  }
  Value loaded_value(Type type, llvm::Value *stored, std::optional<size_t> region_id = {}) {
    if (type.kind == Type::exception && !type.identity) {
      const auto [payload, category] = ir_.load_exception(stored);
      auto value = scalar(type, payload);
      value.exception_category = category;
      return value;
    }
    return scalar(type, stored, region_id);
  }
  Value widened(Value value, Type target, const Location &at) {
    if (value.type == target) return value;
    if (!widens(value.type, target))
      fail(at, "no lossless numeric widening from " + type_name(value.type) + " to " + type_name(target));
    if (!value.native) fail(at, "read of storage before definite initialization in a conversion");
    value.native = ir_.cast(value.type, target, value.native);
    value = bits_value(std::move(value));
    value.type = std::move(target);
    return value;
  }
  static Type common_number(Type left, Type right, const Location &at) {
    const std::array<Type, 6> numbers{Type::uint8, Type::uint32, Type::uint64, Type::integer, Type::float32, Type::float64};
    for (const auto &candidate : numbers) {
      if (!widens(left, candidate) || !widens(right, candidate)) continue;
      if (std::all_of(numbers.begin(), numbers.end(), [&](Type other) {
            return !widens(left, other) || !widens(right, other) || widens(candidate, other);
          })) return candidate;
    }
    fail(at, "numeric operands have no unique lossless common representation; use explicit conversion");
  }
  static bool variadic(const Form &form) {
    return !form.parameters.empty() && form.parameters.back().variadic;
  }
  static const Parameter &parameter_at(const Form &form, size_t index) {
    return form.parameters[std::min(index, form.parameters.size() - 1)];
  }
  std::optional<Signature> signature(const Form &form, const Module &lexical, const std::vector<Value> &args,
                                      const Module &caller, const Location &at, bool required = false) {
    const auto reject = [&](const std::string &reason) -> std::optional<Signature> {
      if (required) fail(at, reason + " for '" + form.name + "'\n" + location_text(form.at) + ": note: signature declared here");
      return {};
    };
    const auto minimum = form.parameters.size() - (variadic(form) ? 1 : 0);
    if (args.size() < minimum || (!variadic(form) && args.size() != minimum)) return reject("wrong argument count");
    const auto context = llvm::SaveAndRestore(type_context_, TypeContext{&lexical, {}});
    const auto infer = [&](auto &&self, const Type &pattern, const Type &actual) -> bool {
      if (pattern.kind == Type::named && !pattern.target && std::any_of(form.type_parameters.begin(), form.type_parameters.end(),
          [&](const TypeParameter &p) { return p.name == pattern.name; })) {
        auto inferred = actual;
        if (inferred.kind == Type::region) inferred.owned = false;
        const auto [found, added] = type_context_.bindings.emplace(pattern.name, inferred);
        return added || found->second == inferred;
      }
      if (pattern.target && actual.target && !self(self, *pattern.target, *actual.target)) return false;
      if (pattern.parameters.size() == actual.parameters.size())
        for (size_t i = 0; i < pattern.parameters.size(); ++i)
          if (!self(self, pattern.parameters[i], actual.parameters[i])) return false;
      return true;
    };
    for (size_t i = 0; i < args.size(); ++i)
      if (!infer(infer, parameter_at(form, i).type, args[i].type)) return reject("inconsistent type-parameter arguments");
    for (const auto &parameter : form.type_parameters) {
      const auto found = type_context_.bindings.find(parameter.name);
      if (found == type_context_.bindings.end()) return reject("cannot infer type parameter '" + parameter.name + "'");
      if (parameter.bound) {
        Value value; value.type = found->second;
        for (const auto &arg : args) merge_conformances(value.conformances, arg.conformances);
        if (!accepts(value, resolve(*parameter.bound, lexical, parameter.at), caller, at))
          return reject("type argument does not satisfy bound for '" + parameter.name + "'");
      }
    }
    Signature result{type_context_.bindings, {}, {}};
    std::map<std::string, std::pair<bool, size_t>> owners;
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &parameter = parameter_at(form, i);
      if (form.operation && ((form.name == "=" && i == 0) || swap_operation(form.name)) && args[i].read_only != parameter.read_only)
        return reject("reference-assignment destination requires an exact const qualifier");
      if (args[i].read_only && !parameter.read_only &&
          (object_type(args[i].type) || !parameter.owner.empty()))
        return reject("read-only object cannot satisfy a writable parameter");
      if (!parameter.owner.empty()) {
        const auto declaration = std::find_if(form.owner_parameters.begin(), form.owner_parameters.end(),
            [&](const auto &owner) { return owner.name == parameter.owner; });
        if (declaration == form.owner_parameters.end()) return reject("undeclared parameter owner '" + parameter.owner + "'");
        const auto actual = storage_owner(args[i]);
        if (!actual) return reject("external owner requires target storage with known provenance");
        const auto [found, inserted] = owners.emplace(parameter.owner, *actual);
        if (!inserted && found->second != *actual) return reject("arguments require the same actual owner");
        if (declaration->root) {
          if (!args[i].region_id) return reject("a region owner cannot bind lexical storage");
          const auto &root = regions_.at(*args[i].region_id).root;
          if (!root || !accepts(contextual(scalar(*root, nullptr), caller),
                               resolve(*declaration->root, lexical, declaration->at), caller, at))
            return reject("argument region does not satisfy its root constraint");
        }
      }
      result.parameters.push_back(resolve(parameter.type, lexical, parameter.at));
    }
    if (form.result_type) result.result = resolve(*form.result_type, lexical, form.at);
    return result;
  }
  static void require(Type actual, Type expected, const Location &at) {
    if (actual != Type::never && actual != expected)
      fail(at, "expected " + type_name(expected) + ", got " + type_name(actual));
  }
  size_t new_symbol(Module &module, const std::string &name) {
    const auto id = symbol_names_.size();
    symbol_names_.push_back(module.path + "::@" + name);
    module.world.symbols.emplace(name, id);
    return id;
  }
  Module &load(const std::string &path, const Location &at) {
    const bool builtin = path == "std:io" || path == "std:exceptions";
    std::error_code error;
    const auto canonical = builtin ? path : std::filesystem::canonical(path, error).string();
    if (error) fail(at, "cannot load module '" + path + "': " + error.message());
    if (const auto found = modules_.find(canonical); found != modules_.end()) return *found->second;
    auto module = std::make_unique<Module>();
    module->path = canonical;
    module->builtin = builtin;
    auto &result = *module;
    modules_.emplace(canonical, std::move(module));
    scanned_.push_back(canonical);
    if (builtin) {
      if (path == "std:exceptions") {
        for (size_t category = 0; category <= 4; ++category) {
          const auto type = exception_type(category);
          result.world.types.emplace(type.name, type);
          result.world.exports.emplace(type.name, type);
        }
        return result;
      }
      new_symbol(result, "print");
      new_symbol(result, "assert");
      result.world.exports.emplace("@print", result.world.symbols.at("print"));
      result.world.exports.emplace("@assert", result.world.symbols.at("assert"));
      return result;
    }
    std::ifstream input(canonical);
    if (!input) fail(at, "cannot read module '" + canonical + "'");
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    result.forms = Parser(source, canonical).parse();
    auto &lexical_names = result.world.lexical_names;
    std::set<std::string> lexical_selections;
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind == Form::Kind::export_resource) {
        continue; // Publication resolves identities after this module is constructed.
      } else if (form.kind == Form::Kind::symbol) {
        if (result.world.symbols.contains(form.name) || result.world.symbol_aliases.contains(form.name))
          fail(form.at, "duplicate symbol name @" + form.name);
        symbol_declarations_.emplace(new_symbol(result, form.name), CallableDeclaration{&form, &result});
      } else if (form.kind == Form::Kind::procedure && form.symbolic) {
        if (!result.world.symbols.contains(form.name) && !result.world.symbol_aliases.contains(form.name))
          symbol_declarations_.emplace(new_symbol(result, form.name), CallableDeclaration{&form, &result});
      } else if (form.kind == Form::Kind::unit) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::unit, result.path + "::" + form.name,
                                            next_unit_++});
      } else if (form.kind == Form::Kind::array) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::array, result.path + "::" + form.name, arrays_.size()});
        arrays_.push_back({&form, &result, {}, {}, false});
      } else if (form.kind == Form::Kind::abstract_type) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        result.world.types.emplace(form.name, Type{Type::abstract_type, result.path + "::" + form.name, abstracts_.size()});
        abstracts_.push_back({&form, &result, {}});
      } else if (form.kind == Form::Kind::data) {
        require_user_type_name(form.name, form.at);
        if (!lexical_names.insert(form.name).second) fail(form.at, "duplicate lexical name '" + form.name + "'");
        const Type type{Type::regional, result.path + "::" + form.name, products_.size()};
        result.world.types.emplace(form.name, type);
        products_.push_back({&form, &result, {}});
      } else if (form.kind == Form::Kind::function || (form.kind == Form::Kind::procedure && !form.symbolic) || form.kind == Form::Kind::global) {
        if (!lexical_names.insert(form.name).second &&
            (form.kind == Form::Kind::global ||
             (!result.world.callable_aliases.contains(form.name) && !lexical_selections.contains(form.name))))
          fail(form.at, "duplicate lexical name '" + form.name + "'");
        if (form.kind != Form::Kind::global) {
          if (!form.operation) require_user_type_name(form.name, form.at);
          if (form.operation && form.name == "setfield")
            fail(form.at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
          declare_callable(result, form);
          if (form.conversion) result.world.conversions.push_back({&form, &result});
        }
      } else if (form.kind == Form::Kind::using_units) {
        for (const auto &selection : form.units) {
          require_user_type_name(selection.local, selection.at);
          if (!lexical_names.insert(selection.local).second)
            fail(selection.at, "duplicate lexical name '" + selection.local + "'");
        }
      } else if (form.kind == Form::Kind::symbol_from) {
        if (result.world.symbols.contains(form.alias) || !result.world.symbol_aliases.emplace(form.alias, &form).second)
          fail(form.at, "duplicate symbol alias @" + form.alias);
      } else if (form.kind == Form::Kind::using_resource || form.kind == Form::Kind::import_resource ||
                 form.kind == Form::Kind::mixin_resource) {
        if (form.using_symbol && !result.world.symbols.contains(form.alias))
          result.world.symbol_aliases.try_emplace(form.alias, &form);
        if (!form.symbolic) {
          if (!lexical_names.insert(form.alias).second &&
              !result.world.callable_aliases.contains(form.alias) && !lexical_selections.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          lexical_selections.insert(form.alias);
        }
      }
    }
    const auto register_inline = [&](auto &&self, const Expression &expression) -> void {
      if (expression.definition) {
        const auto &definition = *expression.definition;
        if (!result.world.symbols.contains(definition.name) && !result.world.symbol_aliases.contains(definition.name)) {
          symbol_declarations_.emplace(new_symbol(result, definition.name), CallableDeclaration{&definition, &result});
          result.world.available_symbols.insert(definition.name);
        }
        self(self, *definition.body);
      }
      for (const auto &child : expression.children) self(self, *child);
    };
    for (const auto &owned : result.forms)
      if (owned->body) register_inline(register_inline, *owned->body);
    for (const auto &owned : result.forms) {
      auto &form = *owned;
      if (form.kind != Form::Kind::symbol_from && form.kind != Form::Kind::using_resource &&
          form.kind != Form::Kind::import_resource && form.kind != Form::Kind::mixin_resource &&
          form.kind != Form::Kind::using_units) continue;
      const auto dependency = form.path.starts_with("std:") ? form.path :
          (std::filesystem::path(canonical).parent_path() / form.path).string();
      result.dependencies.emplace(&form, &load(dependency, form.at));
    }
    return result;
  }
  size_t symbol(const Module &module, const std::string &name, const Location &at) {
    if (const auto found = module.world.symbols.find(name); found != module.world.symbols.end()) return found->second;
    if (const auto alias = module.world.symbol_aliases.find(name); alias != module.world.symbol_aliases.end())
      fail(at, "symbol alias @" + name + " requires its " +
                   (alias->second->using_symbol ? "using symbol" : "symbol from") + " encounter first in this prototype");
    fail(at, "undeclared symbol @" + name);
  }
  size_t expression_symbol(const Expression &expr, const Module &module) {
    if (expr.symbol_id) return *expr.symbol_id;
    return symbol(module, expr.text, expr.at);
  }
  static bool uses_symbol(Expression::Kind kind) {
    using K = Expression::Kind;
    return kind == K::symbol || kind == K::effect || kind == K::perform || kind == K::behave || kind == K::touch ||
           kind == K::procedure_effect;
  }
  static std::unique_ptr<Form> procedure_header(const Form &source) {
    auto form = std::make_unique<Form>();
    form->kind = Form::Kind::procedure;
    form->at = source.at;
    form->name = source.name;
    form->symbolic = source.symbolic;
    form->operation = source.operation;
    form->parameters = source.parameters;
    form->type_parameters = source.type_parameters;
    form->owner_parameters = source.owner_parameters;
    form->result_type = source.result_type;
    form->result_owner = source.result_owner;
    form->throws_types = source.throws_types;
    form->result_const = source.result_const;
    form->result_kind = source.result_kind;
    return form;
  }
  Expr mixin_expression(const Expression &source_expr, const Module &source, const Module &receiver,
                         const std::map<std::string, std::string> &names,
                         std::map<size_t, size_t> &identities, std::set<std::string> &used) {
    auto expr = std::make_unique<Expression>(Expression{
        source_expr.kind, source_expr.at, source_expr.text, {}, source_expr.symbol_id, source_expr.binding,
        source_expr.spread, source_expr.annotation});
    expr->intrinsic = source_expr.intrinsic;
    expr->fixed_binding = source_expr.fixed_binding;
    expr->declaration_kind = source_expr.declaration_kind;
    expr->owner_name = source_expr.owner_name;
    if (uses_symbol(expr->kind)) {
      used.insert(source_expr.text);
      const auto renamed = names.find(source_expr.text);
      expr->text = renamed == names.end() ? source_expr.text : renamed->second;
      expr->symbol_id = symbol(receiver, expr->text, expr->at);
      const auto source_id = expression_symbol(source_expr, source);
      const auto [found, added] = identities.emplace(source_id, *expr->symbol_id);
      if (!added && found->second != *expr->symbol_id)
        fail(expr->at, "splitting one source symbol identity across destinations is outside the prototype");
    }
    for (const auto &child : source_expr.children)
      expr->children.push_back(mixin_expression(*child, source, receiver, names, identities, used));
    if (source_expr.definition) {
      expr->definition = procedure_header(*source_expr.definition);
      expr->definition->name = expr->text;
      expr->definition->body = mixin_expression(*source_expr.definition->body, source, receiver, names, identities, used);
    }
    return expr;
  }
  Form &mixin_body(const Procedure &source, const Form &selection, const Module &receiver) {
    if (!source.form) fail(selection.at, "a built-in procedure has no mixin body; use using or import");
    std::map<std::string, std::string> names;
    for (const auto &mapping : selection.mappings) {
      if (!names.emplace(mapping.source, mapping.local).second)
        fail(mapping.at, "duplicate source role @" + mapping.source + " in symbol map");
      symbol(receiver, mapping.local, mapping.at);
    }
    auto form = procedure_header(*source.form);
    form->at = selection.at;
    form->name = selection.alias;
    form->symbolic = selection.symbolic;
    std::map<size_t, size_t> identities;
    std::set<std::string> used;
    form->body = mixin_expression(*source.form->body, *source.lexical, receiver, names, identities, used);
    for (const auto &mapping : selection.mappings)
      if (!used.contains(mapping.source))
        fail(mapping.at, "unknown source-body role @" + mapping.source + " in symbol map");
    auto &result = *form;
    mixed_forms_.push_back(std::move(form));
    return result;
  }
  Type procedure_type(const Form &form, const Module &module) {
    Type type{Type::procedure};
    const auto context = llvm::SaveAndRestore(type_context_, TypeContext{&module, {}});
    for (size_t i = 0; i < form.type_parameters.size(); ++i)
      type_context_.bindings.emplace(form.type_parameters[i].name, Type{Type::parameter, {}, i});
    for (const auto &parameter : form.type_parameters)
      type.bounds.push_back(parameter.bound ? resolve(*parameter.bound, module, parameter.at) : Type::void_type);
    if (declares_npe(form.throws_types, module, form.at)) type.throws_types = {exception_type(4)};
    type.variadic = variadic(form);
    for (const auto &parameter : form.parameters)
      type.parameters.push_back(resolve(parameter.type, module, parameter.at));
    type.target = std::make_shared<Type>(form.result_type ? resolve(*form.result_type, module, form.at) : Type::void_type);
    std::map<std::string, size_t> owners;
    const auto owner_index = [&](const std::string &name) -> size_t {
      if (name.empty()) return 0;
      if (const auto found = owners.find(name); found != owners.end()) return found->second;
      const auto declaration = std::find_if(form.owner_parameters.begin(), form.owner_parameters.end(),
          [&](const auto &owner) { return owner.name == name; });
      if (declaration == form.owner_parameters.end()) fail(form.at, "undeclared signature owner '" + name + "'");
      type.owner_bounds.push_back(declaration->root ? resolve(*declaration->root, module, declaration->at) : Type::void_type);
      const auto index = type.owner_bounds.size();
      owners.emplace(name, index);
      return index;
    };
    for (const auto &parameter : form.parameters) {
      type.owners.push_back(owner_index(parameter.owner));
      type.const_views.push_back(parameter.read_only);
      type.nullable_views.push_back(parameter.declaration_kind == "ptr" && !parameter.owner.empty());
    }
    type.owners.push_back(owner_index(form.result_owner));
    type.const_views.push_back(form.result_const);
    type.nullable_views.push_back(form.result_kind == "ptr");
    if (std::none_of(type.nullable_views.begin(), type.nullable_views.end(), [](bool x) { return x; })) type.nullable_views.clear();
    if (type.owner_bounds.empty()) type.owners.clear();
    if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; }))
      type.const_views.clear();
    return type;
  }
  bool signature_subset(const Method &left, const Method &right, const Module &caller,
                        const Location &at, const std::vector<Value> &args, bool widening = false) {
    const auto l = procedure_type(*left.form, *left.lexical);
    const auto r = procedure_type(*right.form, *right.lexical);
    if (l.parameters.size() - l.variadic < r.parameters.size() - r.variadic) return false;
    std::map<size_t, Type> substitutions;
    const auto covers = [&](auto &&self, Type a, Type b, bool allow_widening) -> bool {
      if (b.kind == Type::parameter) {
        const auto [found, inserted] = substitutions.emplace(b.identity, a);
        if (!inserted && found->second != a) return false;
        const auto bound = r.bounds.at(b.identity);
        if (bound == Type::void_type) return true;
        if (bound.kind == Type::parameter) return false;
        return self(self, a, bound, allow_widening);
      }
      if (a.kind == Type::parameter) {
        const auto bound = l.bounds.at(a.identity);
        if (bound == Type::void_type || bound.kind == Type::parameter) return false;
        return self(self, bound, b, allow_widening);
      }
      if (a.kind == b.kind && a.target && b.target) {
        if (a.kind == Type::quantity && a.identity != b.identity) return false;
        if (a.parameters.size() != b.parameters.size() || a.variadic != b.variadic) return false;
        for (size_t i = 0; i < a.parameters.size(); ++i)
          if (!self(self, a.parameters[i], b.parameters[i], false)) return false;
        return self(self, *a.target, *b.target, allow_widening && a.kind == Type::quantity);
      }
      return (allow_widening && widens(a, b)) || subtype(a, b, caller, at) || std::any_of(args.begin(), args.end(), [&](const Value &value) {
        return value.type == a && accepts(value, b, caller, at);
      });
    };
    for (size_t i = 0; i < args.size(); ++i)
      if (!covers(covers, l.parameters[std::min(i, l.parameters.size() - 1)],
                          r.parameters[std::min(i, r.parameters.size() - 1)], widening)) return false;
    const auto root_constraint = [&](const Method &method, size_t index) -> std::optional<Type> {
      const auto &parameter = parameter_at(*method.form, index);
      for (const auto &owner : method.form->owner_parameters)
        if (owner.name == parameter.owner && owner.root)
          return resolve(*owner.root, *method.lexical, owner.at);
      return {};
    };
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &left_parameter = parameter_at(*left.form, i);
      const auto &right_parameter = parameter_at(*right.form, i);
      if (left_parameter.read_only != right_parameter.read_only &&
          left_parameter.read_only != args[i].read_only) return false;
      const auto right_root = root_constraint(right, i);
      if (right_root) {
        const auto left_root = root_constraint(left, i);
        if (!left_root || !subtype(*left_root, *right_root, caller, at)) return false;
      }
      const auto &right_owner = parameter_at(*right.form, i).owner;
      if (right_owner.empty()) continue;
      if (parameter_at(*left.form, i).owner.empty()) return false;
      for (size_t j = 0; j < i; ++j)
        if (parameter_at(*right.form, j).owner == right_owner &&
            parameter_at(*left.form, j).owner != parameter_at(*left.form, i).owner) return false;
    }
    // A fixed arity is a subset of a matching variadic signature, not vice versa.
    if (l.variadic != r.variadic) return !l.variadic;
    return true;
  }
  void declare_symbol_type(size_t id) {
    if (symbol_types_.contains(id)) return;
    const auto declaration = symbol_declarations_.find(id);
    if (declaration == symbol_declarations_.end()) return; // Compiler-provided intrinsic family.
    const auto *form = declaration->second.form;
    auto *module = declaration->second.lexical;
    auto type = form->kind == Form::Kind::symbol ? resolve(*form->result_type, *module, form->at)
                                               : procedure_type(*form, *module);
    if (type == Type::void_type) fail(form->at, "a symbol cannot have void type");
    if (type.owned) fail(form->at, "a dynamic resource uses 'region symbol @name:Root', without an owned qualifier");
    if (form->symbol_owner && !object_type(type))
      fail(form->at, "a shared region owner requires object-reference symbols, not TLS bits or whole-region symbols");
    symbol_types_.emplace(id, std::move(type));
  }
  static bool object_type(Type type) {
    return type.kind == Type::reference || type.kind == Type::regional || type.kind == Type::array ||
           type.kind == Type::abstract_type;
  }
  const Value *effective_binding(const Environment &env, size_t id) const {
    const auto *value = env.lookup(id);
    if (env.replay && !env.replay->used.contains(id) && (!value || env.captured(id, value)))
      return effective_binding(env.replay->caller, id);
    return value;
  }
  void check_binding_owners(const Environment &env, const Location &at) {
    std::map<const SymbolOwner *, std::pair<size_t, size_t>> owners;
    for (const auto &[id, declaration] : symbol_declarations_) {
      const auto *group = declaration.form->symbol_owner.get();
      if (!group) continue;
      const auto *value = effective_binding(env, id);
      if (!value) continue;
      if (!value->region_id) fail(at, "symbolic object has no proven region owner: " + symbol_names_[id]);
      const auto [found, inserted] = owners.emplace(group, std::pair{*value->region_id, id});
      if (!inserted && found->second.first != *value->region_id)
        fail(at, "shared-region constraint violated by " + symbol_names_[found->second.second] + " and " + symbol_names_[id] +
            "; a partial perform replay may require explicit touch of its companion symbol\n" +
            location_text(group->at) + ": note: shared region '" + group->name + "' declared here");
    }
  }
  void binding_check(const Environment &env, const Location &at) {
    // A demanded call discovers its actual dependencies after method selection.
    // Validate captured overlays after those reads/touches are known, without
    // treating the complete definition-time metadata snapshot as a capture set.
    if (env.replay) binding_checks_.push_back({env, at});
    else check_binding_owners(env, at);
  }
  void fix_type(size_t id, Type type, const Location &at) {
    declare_symbol_type(id);
    const auto [found, added] = symbol_types_.emplace(id, type);
    if (!added && found->second != type)
      fail(at, "incompatible value types for " + symbol_names_[id] +
                   ": expected " + type_name(found->second) + ", got " + type_name(type));
  }
  Value read_symbol(size_t id, const Environment &env, const Location &at, bool materialize = true) {
    const auto *found = env.lookup(id);
    if (!found) fail(at, (env.captured_context
        ? "perform requires an unavailable definition-time capture " : "missing required binding ") + symbol_names_[id]);
    if (env.replay && env.captured(id, found)) {
      for (auto replay = env.replay; replay; replay = replay->defining_replay) {
        const auto origin = replay->captured_values.lookup(id);
        if (!origin || *origin != found) break;
        replay->used.insert(id);
      }
    }
    if (found->region_id) region_pointer(*found, at);
    if (!materialize || found->type.kind == Type::procedure || found->type.kind == Type::region) return *found;
    const auto type = found->type;
    auto result = scalar(type, env.captured(id, found) ? found->native : ir_.read_symbol(id, type), found->region_id);
    result.conformances = found->conformances;
    result.exception_category = found->exception_category;
    result.read_only = found->read_only;
    return result;
  }
  Value bind(size_t id, Value value, const Module &owner, Environment &env, const Location &at, bool grouped = false) {
    if (local_bits(value.type)) value = bits_value(std::move(value));
    if (value.type == Type::void_type || value.arguments) fail(at, "a symbol cannot be bound to void or a variadic argument pack");
    if (value.type == Type::null_ref) fail(at, "a symbolic object binding requires a live region-backed object");
    if (value.region_id) region_pointer(value, at);
    declare_symbol_type(id);
    if (value.procedure && !value.procedure->builtin.empty()) {
      const auto declared = symbol_types_.find(id);
      if (declared != symbol_types_.end() && declared->second.kind == Type::procedure && declared->second.target) {
        const auto &type = declared->second;
        if (type.parameters.size() != 1 || *type.target != Type::void_type ||
            (value.procedure->builtin == "assert" ? type.parameters[0] != Type::boolean :
             !numeric(type.parameters[0]) && type.parameters[0] != Type::boolean))
          fail(at, "signature is incompatible with intrinsic @" + value.procedure->builtin);
        value.type = type;
      }
    }
    if (value.type.kind == Type::region) {
      if (!value.type.owned) {
        fail(at, "cannot transfer ownership out of a dynamic region binding; use a new resource to shadow it" +
                     region_note(value.region_id, "the non-extractable region was acquired here"));
      }
      value = take_owner(value, at);
      value.type.owned = false;
      value = hold_region(value, module_top_level_, false);
      regions_.at(*value.region_id).symbolic = true;
      if (value.type.target) regions_.at(*value.region_id).root = *value.type.target;
    }
    if (object_type(value.type)) {
      if (!value.region_id || !regions_.at(*value.region_id).symbolic)
        fail(at, "a symbolic object reference requires a region supplied by a region symbol");
      if (const auto declaration = symbol_declarations_.find(id); declaration != symbol_declarations_.end()) {
        if (const auto &group = declaration->second.form->symbol_owner) {
          const auto &root = regions_.at(*value.region_id).root;
          const auto expected = resolve(group->root, *declaration->second.lexical, group->at);
          if (!root || !accepts(contextual(scalar(*root, nullptr), owner), expected, owner, at))
            fail(at, "symbolic object's region root does not satisfy " + type_name(expected));
        }
      }
      const auto declared = symbol_types_.find(id);
      if (declared == symbol_types_.end() || !accepts(value, declared->second, owner, at))
        fix_type(id, value.type, at);
    } else if (value.type.kind == Type::procedure && symbol_types_.contains(id)) {
      auto actual = value.type;
      auto declared = symbol_types_.at(id);
      const bool permitted = actual.throws_types.empty() || !declared.throws_types.empty();
      actual.throws_types.clear(); declared.throws_types.clear();
      if (actual != declared) fix_type(id, value.type, at);
      if (!permitted) fail(at, "procedure checked NullPointerException exceeds symbolic declaration");
      value.type = symbol_types_.at(id);
    } else fix_type(id, value.type, at);
    ir_.effect(id, value.type, value.native);
    value.owner = &owner;
    binding_values_.push_back(std::move(value));
    env.bindings = binding_factory_.add(env.bindings, id, &binding_values_.back());
    if (!grouped) binding_check(env, at);
    return binding_values_.back();
  }

  void resolve_procedure_symbols(Expression &expr, const Module &module) {
    if (uses_symbol(expr.kind) && !expr.symbol_id)
      expr.symbol_id = symbol(module, expr.text, expr.at);
    for (const auto &child : expr.children) resolve_procedure_symbols(*child, module);
    if (expr.definition) resolve_procedure_symbols(*expr.definition->body, module);
  }
  Value define_procedure(Form &form, const Module &lexical, const Environment &env) {
    resolve_procedure_symbols(*form.body, lexical);
    auto proc = std::make_shared<Procedure>();
    proc->form = &form;
    proc->lexical = &lexical;
    // This is compiler metadata, not a runtime closure or a retained region.
    // SSA values already represent each binding's value at its definition.
    // Only demanded reads generate native uses, after typed method selection.
    proc->captures = env;
    proc->captures.captured_context = true;
    proc->captures.captured_bindings = env.bindings;
    return {procedure_type(form, lexical), {}, std::move(proc), &lexical};
  }
  Value block(const Expression &expr, const Module &module, Environment env, Locals locals,
              bool function_context, bool discard_result = false, std::optional<Type> result_type = {}, bool copy_result = false,
              std::optional<size_t> destination_binding = {}, bool read_result = true,
              const Value *null_owner = nullptr) {
    const bool parent_module = module_top_level_;
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);
    const auto holding_begin = holdings_.size();
    const auto native_scope = ir_.scope();
    const auto scope_id = scopes_.size();
    scopes_.push_back(true);
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);

    Value result;
    try {
      for (size_t i = 0; i < expr.children.size(); ++i) {
        const auto &child = expr.children[i];
        const bool assignment = child->kind == Expression::Kind::rebind ||
            (child->kind == Expression::Kind::call && child->text == "=");
        result = expression(*child, module, env, locals, function_context,
                            (!assignment || (!discard_result && i + 1 == expr.children.size())) &&
                                (read_result || i + 1 != expr.children.size()),
                            copy_result && i + 1 == expr.children.size(),
                            read_result || i + 1 != expr.children.size());
      }
    } catch (const EndPath &) { result = scalar(Type::never, nullptr); }
    if (discard_result && result.type != Type::never) result = {};
    if (result.type == Type::null_ref && null_owner && result_type)
      result = null_target(*result_type, *null_owner, expr.at);
    if (result_type && result.type != Type::never) result = admit(std::move(result), *result_type, module, expr.at);
    if (copy_result && local_bits(result.type)) result = bits_value(read_value(std::move(result), expr.at));
    if (destination_binding && result.type != Type::never && result.pointer != destination_binding)
      fail(expr.at, "a (=) overload must return its destination binding's resulting view; finish the body with the lhs parameter");
    result.pointer.reset();
    scopes_[scope_id] = false;
    valid_view(result, expr.at);
    result = finish_scope(holding_begin, native_scope, std::move(result), expr.at, parent_module);
    if (result.type.kind == Type::procedure) fail(expr.at, "procedure-valued scope results are not supported");
    if (result.arguments) fail(expr.at, "a variadic argument pack cannot escape its call scope");
    return result;
  }
  std::vector<Value> arguments(const Expression &expr, const Module &module, Environment &env,
                               Locals &locals, bool function_context,
                               std::vector<ConversionPoint> *points = nullptr,
                               const std::vector<Method> &nullable_candidates = {}) {
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    std::vector<Value> values;
    for (const auto &arg : expr.children) {
      const bool forward = std::any_of(nullable_candidates.begin(), nullable_candidates.end(), [&](const Method &method) {
        return !method.form->parameters.empty() && (variadic(*method.form) || values.size() < method.form->parameters.size()) &&
               parameter_at(*method.form, values.size()).declaration_kind == "ptr" &&
               !parameter_at(*method.form, values.size()).owner.empty();
      });
      auto value = expression(*arg, module, env, locals, function_context, false, false, !forward);
      if (!forward && value.storage) {
        const bool initialized = std::all_of(value.storage->cells.begin(), value.storage->cells.end(),
            [&](size_t cell) { return cells_.at(cell).initialized; });
        if (initialized) value = read_value(std::move(value), arg->at);
        else value.native = nullptr;
      } else if (!forward && value.type.kind != Type::reference && value.nullable) {
        value = read_value(std::move(value), arg->at);
      }
      if (arg->spread) {
        if (!value.arguments) fail(arg->at, "argument expansion requires a variadic parameter pack");
        for (const auto &element : *value.arguments) {
          values.push_back(forward ? element : read_value(element, arg->at));
          if (points) { points->push_back(conversion_point()); points->back().raw_view = forward; }
        }
      } else {
        values.push_back(std::move(value));
        if (points) { points->push_back(conversion_point()); points->back().raw_view = forward; }
      }
    }
    return values;
  }
  static bool converting_parameters(const Form &form) {
    return std::any_of(form.parameters.begin(), form.parameters.end(),
                       [](const Parameter &parameter) { return parameter.automatic_conversion; });
  }
  bool converting_method(const std::string &name, const Module &module,
                         const Location &at) {
    const auto methods = lookup_callable(module, name, at).methods;
    return std::any_of(methods.begin(), methods.end(), [&](const Method &function) {
      return converting_parameters(*function.form);
    });
  }
  Value call(Form &form, const Module &lexical, Environment env, const std::vector<Value> &args,
             bool function_context, const Location &at, const Module &caller, ImplementationContext implementation = {}) {
    const bool throws_npe = declares_npe(form.throws_types, lexical, form.at);
    if (throws_npe) require_npe_handler(at);
    const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, throws_npe);
    const auto instantiated = *signature(form, lexical, args, caller, at, true);
    const auto types = llvm::SaveAndRestore(type_context_, TypeContext{&lexical, instantiated.bindings});
    if (!form.body) fail(at, "callable '" + form.name + "' has no method implementation");
    if (!function_context) {
      const auto touches = [&](auto &&self, const Expression &expression) -> void {
        if (expression.kind == Expression::Kind::touch)
          read_symbol(expression_symbol(expression, lexical), env, expression.at, false);
        // An inline definition's touches belong to that procedure, not this body.
        for (const auto &child : expression.children) self(self, *child);
      };
      touches(touches, *form.body);
    }
    // A using selection retains this context; import selects the receiving module.
    // The source body still resolves types, globals and symbols in lexical.
    const auto saved_context = llvm::SaveAndRestore(implementation_context_,
        implementation.empty() ? ImplementationContext{&lexical} : std::move(implementation));
    const auto saved_conformances = llvm::SaveAndRestore(caller_conformances_);
    for (const auto &arg : args) {
      const auto value = contextual(arg, caller);
      const auto type = (value.type.kind == Type::reference || value.type.kind == Type::region) && value.type.target ?
          *value.type.target : value.type;
      for (const auto &relation : value.conformances)
        if (relation.data == type) append_conformance(caller_conformances_, relation);
    }
    call_path_.push_back({&form, at});
    if (form.conversion) conversion({&form, &lexical});
    if (!active_calls_.insert(&form).second) fail(at, "recursive call is outside the first prototype");
    const bool parent_module = module_top_level_;
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto holding_begin = holdings_.size();
    const auto native_scope = ir_.scope();
    const auto scope_id = scopes_.size();
    scopes_.push_back(true);
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);
    std::vector<Value> parameters;
    std::set<size_t> transferring;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i].region_id) region_pointer(args[i], at);
      auto parameter = admit(args[i], instantiated.parameters[i], caller, at);
      const auto &formal = parameter_at(form, i);
      if (parameter.type.kind == Type::region && formal.type.kind != Type::region)
        fail(formal.at, "a whole-region parameter requires a region declaration");
      if (formal.owner.empty() && object_type(parameter.type))
        fail(formal.at, "a non-bits parameter requires an explicit external owner: use 'in O' and 'where {owner O}'");
      if (formal.declaration_kind == "val" && !local_bits(parameter.type))
        fail(formal.at, "val parameters require bits; regional objects are borrowed through an explicit owner");
      if (formal.owner.empty() && local_bits(parameter.type)) {
        if (!parameter.native) fail(at, "read of storage before definite initialization in a local parameter argument");
        const auto source = bits_value(parameter);
        parameter = local_storage(parameter.type, formal.at);
        write_bits(parameter, source, at);
      }
      if (formal.read_only && storage_owner(parameter)) parameter.read_only = true;
      if (parameter.type.owned && (!parameter.holding_id || !transferring.insert(*parameter.holding_id).second))
        fail(at, "one region owner cannot be consumed by multiple parameters");
      parameters.push_back(std::move(parameter));
    }
    // Revalidation precedes every move: later arguments may have changed an
    // earlier owner or view. Transfers begin only at this callee-entry boundary.
    for (auto &parameter : parameters)
      if (parameter.type.owned) parameter = hold_region(take_owner(parameter, at), false, true);
    Locals locals;
    std::map<std::string, Value> owner_values;
    for (const auto &owner : form.owner_parameters) {
      std::optional<Value> region;
      for (size_t i = 0; i < parameters.size(); ++i) {
        if (parameter_at(form, i).owner != owner.name) continue;
        if (!owner.root) { region = parameters[i]; break; }
        auto &metadata = regions_.at(*parameters[i].region_id);
        if (!region) {
          Type type{Type::region};
          if (metadata.root) type.target = std::make_shared<Type>(*metadata.root);
          region = scalar(type, region_pointer(parameters[i], at), parameters[i].region_id);
          region->read_only = parameters[i].read_only;
          region->conformances = parameters[i].conformances;
        } else region->read_only = region->read_only && parameters[i].read_only;
      }
      if (!region) fail(owner.at, "cannot infer owner '" + owner.name + "' from the arguments");
      owner_values.emplace(owner.name, *region);
      if (owner.root && !owner.explicit_parameter) declare_local(locals, owner.name, *region, lexical, owner.at);
    }
    const auto fixed = form.parameters.size() - (variadic(form) ? 1 : 0);
    const bool rebind = form.operation && form.name == "=";
    for (size_t i = 0; i < fixed; ++i) {
      auto argument = parameters[i];
      if ((rebind && i == 0) || (form.operation && form.name == "<=>")) {
        if (!argument.pointer) fail(at, "a reference operation overload requires an ordinary ptr destination");
        // Reference-operation destinations denote the caller's bindings; other
        // parameters borrow storage through their own lexical pointer bindings.
        declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
      } else if (form.parameters[i].declaration_kind == "val")
        declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
      else if (argument.storage || (!form.parameters[i].owner.empty() && object_type(argument.type)))
        declare_pointer(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at,
                        form.parameters[i].read_only, false, form.parameters[i].declaration_kind == "ref");
      else declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
    }
    if (variadic(form)) {
      std::vector<Value> tail;
      for (size_t i = fixed; i < args.size(); ++i) {
        tail.push_back(parameters[i]);
      }
      Value pack; pack.type = Type::arguments;
      pack.type.target = std::make_shared<Type>(resolve(form.parameters.back().type, lexical, form.at));
      pack.arguments = std::make_shared<const std::vector<Value>>(std::move(tail));
      declare_local(locals, form.parameters.back().name, std::move(pack), lexical, form.parameters.back().at);
    }
    const bool assignment = form.operation && (form.name == "=" || form.name == "<-" || form.name == "[]=" || form.name == "[]<-");
    const Value *result_owner = form.result_owner.empty() ? nullptr : &owner_values.at(form.result_owner);
    auto result = block(*form.body, lexical, std::move(env), std::move(locals), function_context, false,
                        instantiated.result, form.result_const && form.result_owner.empty() && !assignment,
                        rebind ? args.front().pointer : std::nullopt, form.result_kind != "ptr", result_owner);
    if (form.result_kind != "ptr" && result.nullable) result = read_value(std::move(result), at);
    scopes_[scope_id] = false;
    valid_view(result, at);
    result = finish_scope(holding_begin, native_scope, std::move(result), at, parent_module);
    active_calls_.erase(&form);
    if (instantiated.result) result = admit(std::move(result), *instantiated.result, lexical, form.at);
    if (!form.result_owner.empty()) {
      const auto owner = owner_values.find(form.result_owner);
      if (owner == owner_values.end() || storage_owner(result) != storage_owner(owner->second))
        fail(form.at, "result does not belong to its declared owner");
    }
    if (form.result_const && storage_owner(result)) result.read_only = true;
    if (!form.result_const && !form.result_owner.empty() && result.read_only)
      fail(form.at, "a writable borrowed result cannot return a read-only view");
    result.pointer.reset();
    call_path_.pop_back();
    return result;
  }
  struct Conversion {
    Method function;
    Type source, target;
  };
  struct Route {
    std::vector<Method> steps;
    std::vector<Method> alternative;
    std::optional<Type> target = {};
  };
  struct MethodMatch {
    Method function;
    std::vector<Route> routes;
    std::vector<CallableDeclaration> declarations;
  };
  Conversion conversion(Method function) {
    auto &form = *function.form;
    if (form.parameters.size() != 1 || !form.result_type || form.parameters[0].automatic_conversion)
      fail(form.at, "a conversion function requires one exact UoM input and an explicit UoM result");
    auto source = resolve(form.parameters[0].type, *function.lexical, form.at);
    auto target = resolve(*form.result_type, *function.lexical, form.at);
    if (source.kind != Type::quantity || target.kind != Type::quantity)
      fail(form.at, "a conversion function must connect UoM types");
    return {function, source, target};
  }
  std::vector<Conversion> conversion_graph(const Module &module) {
    std::vector<Conversion> result;
    for (const auto &function : module.world.conversions) result.push_back(conversion(function));
    return result;
  }
  static bool conversion_applicable(const Conversion &edge, const std::vector<Conversion> &graph, Type actual) {
    if (!widens(actual, edge.source)) return false;
    // Overloads of one conversion retain argument-directed specificity. Distinct
    // conversion functions remain alternative routes, including equal inputs.
    return !std::any_of(graph.begin(), graph.end(), [&](const Conversion &other) {
      return other.function.lexical == edge.function.lexical &&
             other.function.form->name == edge.function.form->name &&
             other.function.implementation == edge.function.implementation &&
             widens(actual, other.source) && widens(other.source, edge.source) &&
             !widens(edge.source, other.source);
    });
  }
  static std::optional<std::vector<Method>> shortest_route(
      const std::vector<Conversion> &graph, Type source, Type target,
      const std::vector<size_t> &itinerary = {}, const std::optional<Method> &excluded = {}) {
    struct Step { Type type; std::vector<Method> path; };
    std::queue<Step> pending;
    std::vector<Type> visited{source};
    pending.push({source, {}});
    while (!pending.empty()) {
      auto step = std::move(pending.front());
      pending.pop();
      for (const auto &edge : graph) {
        if (!conversion_applicable(edge, graph, step.type) || (excluded && edge.function == *excluded)) continue;
        const auto depth = step.path.size() + 1;
        if (!itinerary.empty() && (depth >= itinerary.size() || edge.target.identity != itinerary[depth])) continue;
        auto path = step.path;
        path.push_back(edge.function);
        if (widens(edge.target, target) && (itinerary.empty() || depth + 1 == itinerary.size())) return path;
        if (std::find(visited.begin(), visited.end(), edge.target) != visited.end()) continue;
        visited.push_back(edge.target);
        pending.push({edge.target, std::move(path)});
      }
    }
    return {};
  }
  std::optional<Route> route(Type actual, const Parameter &parameter, const Module &lexical,
                              const Module &caller, const Location &at, bool diagnose_unavailable = true) {
    const auto target = resolve(parameter.type, lexical, parameter.at);
    if (parameter.automatic_conversion && target.kind != Type::quantity)
      fail(parameter.at, "a starred parameter requires a UoM quantity type");
    if (widens(actual, target)) return Route{{}, {}, target};
    if (!parameter.automatic_conversion || actual.kind != Type::quantity) return {};
    const auto graph = conversion_graph(caller);
    Route result{{}, {}, target};
    for (const auto &edge : graph) {
      if (!conversion_applicable(edge, graph, actual) || !widens(edge.target, target)) continue;
      if (result.steps.empty()) result.steps = {edge.function};
      else if (result.alternative.empty()) result.alternative = {edge.function};
    }
    if (!result.steps.empty()) return result;
    std::vector<size_t> itinerary;
    if (!parameter.conversion_route.empty()) {
      std::vector<size_t> declared{target.identity};
      std::set<size_t> visited{target.identity};
      for (const auto &name : parameter.conversion_route) {
        const auto unit = resolve_name({Type::named, name}, lexical, parameter.at);
        if (unit.kind != Type::unit || !visited.insert(unit.identity).second)
          fail(parameter.at, "declared conversion route requires distinct UoM identities");
        declared.push_back(unit.identity);
      }
      const auto start = std::find(declared.begin() + 1, declared.end(), actual.identity);
      if (start == declared.end()) return {};
      for (auto i = start;; --i) {
        itinerary.push_back(*i);
        if (i == declared.begin()) break;
      }
    }
    const auto chosen = shortest_route(graph, actual, target, itinerary);
    if (!chosen) {
      if (!itinerary.empty() && diagnose_unavailable)
        fail(at, "declared conversion route has an unavailable typed step from " +
                     type_name(actual) + " to " + type_name(target));
      return {};
    }
    result.steps = *chosen;
    // A distinct simple route omits an edge from the selected route. Numeric
    // widening is compatibility, not extra graph edges or ambiguous paths.
    for (const auto &edge : result.steps) {
      if (auto alternative = shortest_route(graph, actual, target, itinerary, edge)) {
        result.alternative = std::move(*alternative);
        break;
      }
    }
    return result;
  }
  static std::string callable_name(const Form &form) {
    if (form.operation) return "(" + form.name + ")";
    return form.symbolic ? "@" + form.name : form.name;
  }
  [[noreturn]] static void method_failure(const std::string &name, const std::vector<Value> &args,
                                         const std::vector<Method> &candidates, const Location &at,
                                         bool ambiguous, const Module &visible) {
    std::string message = (ambiguous ? "ambiguous method '" : "no matching method '") + name +
                          "' for supplied argument types (";
    for (size_t i = 0; i < args.size(); ++i)
      message += (i ? ", " : "") + std::string(args[i].read_only ? "const " : "") + type_name(args[i].type);
    message += "): " + std::to_string(candidates.size()) + (ambiguous ? " matching" : " available") + " candidates";
    message += "\n" + location_text(at) + ": note: method lookup uses module '" + visible.path + "'";
    for (size_t i = 0; i < args.size(); ++i)
      for (const auto &relation : args[i].conformances)
        if (relation.data == args[i].type)
          message += "\n" + location_text(at) + ": note: argument " + std::to_string(i + 1) +
                     " guarantees " + type_name(relation.interface);
    const auto shown = std::min<size_t>(candidates.size(), 4);
    for (size_t i = 0; i < shown; ++i) {
      const auto &form = *candidates[i].form;
      message += "\n" + location_text(form.at) + ": note: candidate " +
                 (form.kind == Form::Kind::procedure ? "procedure '" : "function '") + callable_name(form) + "' (";
      for (size_t parameter = 0; parameter < form.parameters.size(); ++parameter) {
        const auto &declared = form.parameters[parameter];
        message += (parameter ? ", " : "") + std::string(declared.read_only ? "const " : "") + type_name(declared.type);
        if (!declared.owner.empty()) message += " in " + declared.owner;
        if (declared.automatic_conversion) message += "*";
        for (const auto &step : declared.conversion_route) message += "<-" + step;
      }
      message += ")";
      if (form.result_type) message += " -> " + type_name(*form.result_type);
      if (!candidates[i].implementation.empty() && candidates[i].implementation.back() != candidates[i].lexical)
        message += "; implementation context '" + candidates[i].implementation.back()->path + "'";
    }
    if (shown < candidates.size())
      message += "\nnote: " + std::to_string(candidates.size() - shown) + " additional candidates omitted";
    fail(at, message);
  }
  std::optional<MethodMatch> method(const std::string &name, const Module &module,
                                    const std::vector<Value> &args, const Location &at) {
    const auto information = lookup_callable(module, name, at);
    const auto &candidates = information.methods;
    if (candidates.empty()) return {};
    for (size_t i = 0; i < candidates.size(); ++i) {
      const auto first = signature(*candidates[i].form, *candidates[i].lexical, args, module, at);
      if (!first) continue;
      for (size_t j = 0; j < i; ++j) {
        const auto second = signature(*candidates[j].form, *candidates[j].lexical, args, module, at);
        if (!second) continue;
        bool same = true, different_mode = false;
        for (size_t k = 0; k < first->parameters.size(); ++k) {
          auto left = first->parameters[k], right = second->parameters[k];
          different_mode = different_mode || left.owned != right.owned;
          left.owned = right.owned = false;
          same = same && left == right;
        }
        if (same && different_mode)
          fail(at, "overloads differing only in owning versus borrowed mode are not supported");
      }
    }
    std::vector<MethodMatch> exact, converted;
    const auto choose_specific = [&](std::vector<MethodMatch> &matches, bool widening) {
      if (matches.size() < 2) return;
      const auto more_specific = [&](const MethodMatch &left, const MethodMatch &right) {
        return signature_subset(left.function, right.function, module, at, args, widening) &&
               !signature_subset(right.function, left.function, module, at, args, widening);
      };
      for (const auto &candidate : matches) {
        if (std::all_of(matches.begin(), matches.end(), [&](const MethodMatch &other) {
              return &candidate == &other || more_specific(candidate, other);
            })) {
          auto selected = candidate;
          matches = {std::move(selected)};
          return;
        }
      }
      std::vector<Method> candidates;
      for (const auto &match : matches) candidates.push_back(match.function);
      method_failure(name, args, candidates, at, true, module);
    };
    for (const auto &candidate : candidates) {
      const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
      if (!instantiated) continue;
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &parameter = parameter_at(*candidate.form, i);
        const auto type = instantiated->parameters[i];
        if (parameter.automatic_conversion && type.kind != Type::quantity)
          fail(parameter.at, "a starred parameter requires a UoM type");
        matches = matches && accepts(args[i], type, module, at);
      }
      if (matches) exact.push_back({candidate, std::vector<Route>(args.size()), information.declarations});
    }
    choose_specific(exact, false);
    if (!exact.empty()) return exact.front();
    for (const auto &candidate : candidates) {
      if (candidate.form->operation && (candidate.form->name == "=" || swap_operation(candidate.form->name))) continue;
      const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
      if (!instantiated) continue;
      const auto context = llvm::SaveAndRestore(type_context_, TypeContext{candidate.lexical, instantiated->bindings});
      MethodMatch match{candidate, {}, information.declarations};
      bool matches = true;
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &parameter = parameter_at(*candidate.form, i);
        if (accepts(args[i], instantiated->parameters[i], module, at)) {
          match.routes.push_back({});
          continue;
        }
        if (i == 0 && candidate.form->operation &&
            (candidate.form->name == "<-" || candidate.form->name == "[]<-")) { matches = false; break; }
        auto path = route(args[i].type, parameter, *candidate.lexical, module, at, false);
        if (!path) { matches = false; break; }
        match.routes.push_back(std::move(*path));
      }
      if (matches) converted.push_back(std::move(match));
    }
    choose_specific(converted, true);
    if (converted.empty()) {
      // A missing edge makes an overload inapplicable. Preserve the detailed
      // diagnostic for an unambiguous declaration after considering alternatives.
      if (candidates.size() == 1) {
        const auto &candidate = candidates.front();
        const auto instantiated = signature(*candidate.form, *candidate.lexical, args, module, at);
        if (instantiated && !(candidate.form->operation && (candidate.form->name == "=" || candidate.form->name == "<-" ||
            candidate.form->name == "[]<-" || swap_operation(candidate.form->name)))) {
          const auto context = llvm::SaveAndRestore(type_context_, TypeContext{candidate.lexical, instantiated->bindings});
          for (size_t i = 0; i < args.size(); ++i)
            route(args[i].type, parameter_at(*candidate.form, i), *candidate.lexical, module, at);
        }
      }
      return {};
    }
    return converted.front();
  }
  Value converted(Value value, const Route &route, const Location &at, const Module &caller,
                  const ConversionPoint *point = nullptr, bool read_argument = false) {
    if (route.steps.empty() && !route.target && !read_argument) return value;
    if (!route.alternative.empty()) {
      auto describe = [&](const std::vector<Method> &path) {
        std::string result = type_name(value.type);
        for (const auto &step : path)
          result += " --" + step.form->name + "--> " + type_name(conversion(step).target);
        return result;
      };
      const auto message = "ambiguous unit conversion; selected " + describe(route.steps) +
                           "; alternative " + describe(route.alternative);
      warn(Warning::unit_conversion, at, message);
    }
    // Dispatch needs every argument's type, but a selected conversion executes
    // immediately after its own argument, before any later argument effects.
    const auto state = flow_state();
    const auto resume = point ? std::optional(ir_.begin_conversion(point->native)) : std::nullopt;
    if (point) restore_flow(point->regions);
    if (read_argument) {
      const bool initialized = !value.storage || std::all_of(value.storage->cells.begin(), value.storage->cells.end(),
          [&](size_t cell) { return cells_.at(cell).initialized; });
      if (initialized) value = read_value(std::move(value), at);
      else value.native = nullptr;
    }
    for (const auto &step : route.steps) {
      value = widened(value, conversion(step).source, at);
      value = call(*step.form, *step.lexical, {}, {value}, true, at, caller, step.implementation);
      if (value.type == Type::never) break;
    }
    if (route.target && value.type != Type::never) value = widened(value, *route.target, at);
    if (resume) {
      ir_.end_conversion(point->native, *resume);
      restore_flow(state);
      if (value.type == Type::never) ir_.unreachable();
    }
    if (value.type == Type::never) throw EndPath{};
    return value;
  }
  void check_declarations(const std::vector<CallableDeclaration> &declarations, Form::Kind kind,
                           const std::vector<Value> &args, const Value &result,
                           const Module &caller, const Location &at, bool throws_npe) {
    for (const auto &declaration : declarations) {
      const auto contract = signature(*declaration.form, *declaration.lexical, args, caller, at);
      if (!contract) continue;
      bool applicable = true;
      for (size_t i = 0; i < args.size(); ++i)
        applicable = applicable && accepts(args[i], contract->parameters[i], caller, at);
      if (!applicable) continue;
      if (throws_npe && !declares_npe(declaration.form->throws_types, *declaration.lexical, declaration.form->at))
        fail(at, "method checked NullPointerException exceeds callable declaration");
      if (declaration.form->kind != kind)
        fail(at, "method callable kind violates declaration\n" + location_text(declaration.form->at) + ": note: callable signature declared here");
      if (result.nullable && declaration.form->result_kind != "ptr")
        fail(at, "nullable method result violates non-null callable declaration");
      if (contract->result && !accepts(result, *contract->result, caller, at))
        fail(at, "method result violates callable declaration: expected " + type_name(*contract->result) +
                     ", got " + type_name(result.type) + "\n" + location_text(declaration.form->at) + ": note: callable signature declared here");
      if (!declaration.form->result_const && !declaration.form->result_owner.empty() && result.read_only)
        fail(at, "read-only method result violates writable callable declaration\n" +
                     location_text(declaration.form->at) + ": note: result access promised here");
      if (!declaration.form->result_owner.empty()) {
        bool inferred = false;
        for (size_t i = 0; i < args.size(); ++i)
          if (parameter_at(*declaration.form, i).owner == declaration.form->result_owner) {
            inferred = true;
            if (storage_owner(result) != storage_owner(args[i]))
              fail(at, "method result violates callable declaration owner\n" +
                           location_text(declaration.form->at) + ": note: result owner promised here");
          }
        if (!inferred) fail(declaration.form->at, "cannot infer callable declaration result owner from its arguments");
      }
    }
  }
  Value call_method(const MethodMatch &match, std::vector<Value> args, const Location &at,
                    const Environment &env, bool function_context, const Module &caller,
                    const std::vector<ConversionPoint> &points = {}) {
    const auto &form = *match.function.form;
    if (form.operation && (form.name == "=" || form.name == "<-" || form.name == "[]=" || form.name == "[]<-") &&
        (form.parameters.empty() || form.parameters.front().owner.empty()))
      fail(form.at, "an assignment destination requires an explicit external owner: use 'in O' and 'where {owner O}'");
    if (form.operation && swap_operation(form.name) &&
        (form.parameters.size() != 2 || form.parameters[0].owner.empty() || form.parameters[1].owner.empty() || variadic(form)))
      fail(form.at, "a swap overload requires two destinations with explicit external owners");
    if (form.operation && (form.name == "=" || form.name == "<=>"))
      for (size_t i = 0; i < (form.name == "=" ? 1u : 2u) && i < args.size(); ++i)
        if (args[i].pointer && pointers_.at(*args[i].pointer).fixed_binding)
          fail(at, "cannot rebind a const ptr binding");
    for (size_t i = 0; i < args.size(); ++i)
      args[i] = converted(args[i], match.routes[i], at, caller, points.empty() ? nullptr : &points[i],
          !points.empty() && points[i].raw_view && !(parameter_at(form, i).declaration_kind == "ptr" && !parameter_at(form, i).owner.empty()));
    const bool function = match.function.form->kind == Form::Kind::function;
    if (!function && function_context) {
      const auto &form = *match.function.form;
      const std::string message = form.operation ? "a function cannot invoke a procedure overload"
                                                 : "a function cannot invoke a procedure";
      fail(at, message + "\n" + location_text(form.at) + ": note: selected procedure '" +
                   callable_name(form) + "' declared here");
    }
    for (const auto &declaration : match.declarations) {
      const auto contract = signature(*declaration.form, *declaration.lexical, args, caller, at);
      if (!contract) continue;
      bool applicable = true;
      for (size_t i = 0; i < args.size(); ++i)
        applicable &= accepts(args[i], contract->parameters[i], caller, at);
      if (applicable && declares_npe(declaration.form->throws_types, *declaration.lexical, declaration.form->at))
        require_npe_handler(at);
    }
    // Lexical procedure calls use current bindings; perform uses captured values.
    auto result = call(*match.function.form, *match.function.lexical, function ? Environment{} : env, args, function, at, caller,
                       match.function.implementation);
    if (form.operation && swap_operation(form.name) && result.type != Type::never)
      require(result.type, Type::void_type, at);
    if (form.operation && (form.name == "<-" || form.name == "[]<-") && result.type != Type::never && result.type != Type::void_type) {
      if (!form.result_const || !storage_owner(result))
        fail(form.at, "a transfer overload must return void or a const destination view");
      if (storage_owner(result) != storage_owner(args.front()))
        fail(at, "a transfer result must belong to the destination owner");
      if (form.name == "<-" &&
          (result.referent ? result.referent : result.storage ? result.storage->address : result.native) !=
          (args.front().referent ? args.front().referent : args.front().storage ? args.front().storage->address : args.front().native))
        fail(at, "a (<-) overload must return the original destination, not another object");
      result.read_only = true;
    }
    check_declarations(match.declarations, match.function.form->kind, args, result, caller, at, declares_npe(form.throws_types, *match.function.lexical, form.at));
    return result;
  }
  Value transfer_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                            bool function_context) {
    if (expr.children.size() != 2) fail(expr.at, "object transfer requires a destination and source");
    auto destination = expression(*expr.children[0], module, env, locals, function_context, false);
    if (destination.read_only) fail(expr.at, "cannot transfer into a read-only destination view");
    auto source = expression(*expr.children[1], module, env, locals, function_context);
    if (!expr.intrinsic)
      if (const auto selected = method("<-", module, {destination, source}, expr.at))
        return call_method(*selected, {destination, source}, expr.at, env, function_context, module);
    if (local_bits(destination.type) && destination.storage) {
      write_bits(destination, source, expr.at);
      destination.read_only = true;
      destination.pointer.reset();
      return destination;
    }
    if (!object_type(destination.type) || !destination.region_id)
      fail(expr.at, "object transfer requires a regional object destination; symbolic bits require effect '<-'");
    if (expr.intrinsic) fail(expr.at, "intrinsic (<-) requires a bits destination; object transfer requires a user-defined overload");
    fail(expr.at, "object transfer requires a user-defined (<-) overload");
  }
  Value reference_source(Value value, Type target, size_t owner, bool required, const Location &at) {
    if (value.type == Type::null_ref) {
      if (required) fail(at, "a ref field cannot contain null");
      value = scalar(reference(target), ir_.bits(Type::null_ref, 0), owner);
      value.nullable = true;
      return value;
    }
    if (required && value.nullable) fail(at, "a ref field requires a proven non-null target");
    if (value.read_only) fail(at, "cannot store a read-only view in a writable reference field");
    if (value.region_id != owner) fail(at, "cross-region reference fields require a target in the same owner; use '<-' for bits transfer");
    valid_view(value, at);
    if (value.type == target) {
      if (local_bits(target)) {
        if (!value.storage) fail(at, "a reference field requires existing target storage");
        auto *address = value.storage->address;
        value.native = ir_.reference({target, regions_.at(owner).owner, address});
        if (value.nullable) value.native = ir_.nullable_offset(regions_.at(owner).owner, address);
      }
      value.type = reference(target);
      value.storage.reset();
    }
    require(value.type, reference(target), at);
    return value;
  }
  Value rebind_field(Value destination, Value source, const Location &at) {
    if (destination.read_only) fail(at, "cannot rebind a field through a read-only view");
    auto stored = reference_source(source, *destination.reference_field->type.target,
                                   *destination.region_id, destination.required_reference, at);
    if (destination.required_reference) ir_.require_representable_reference(*destination.reference_field, stored.native);
    ir_.write(*destination.reference_field, stored.native);
    destination.native = stored.native;
    destination.nullable = stored.nullable;
    if (destination.storage) {
      destination.storage->address = ir_.nullable_address(destination.reference_field->owner, stored.native);
      destination.native = nullptr;
    }
    return destination;
  }
  Value rebind_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    if (expr.children.size() != 2) fail(expr.at, "reference assignment requires a destination and source");
    auto destination = expression(*expr.children[0], module, env, locals, function_context, false);
    auto source = expression(*expr.children[1], module, env, locals, function_context, false);
    if (destination.reference_field) return rebind_field(destination, source, expr.at);
    if (source.type == Type::null_ref && destination.pointer) source = null_target(destination.type, destination, expr.at);
    if (!destination.pointer) fail(expr.at, "reference assignment requires an ordinary ptr binding; use '<-' for bits transfer");
    if (pointers_.at(*destination.pointer).fixed_binding) fail(expr.at, "cannot rebind a const ptr binding");
    if (!source.nullable && source.storage && std::all_of(source.storage->cells.begin(), source.storage->cells.end(),
        [&](size_t cell) { return cells_.at(cell).initialized; })) source = read_value(std::move(source), expr.at);
    // Select user methods before checking the builtin alias operation. A method
    // may use a read-only source to choose an independently writable target.
    if (!expr.intrinsic) {
      if (const auto selected = method("=", module, {destination, source}, expr.at))
        return call_method(*selected, {destination, source}, expr.at, env, function_context, module);
    }
    check_rebind(destination, source, expr.at);
    return rebind_pointer(destination, source);
  }
  void check_rebind(const Value &destination, const Value &source, const Location &at) {
    if (!destination.pointer) fail(at, "reference swap requires ordinary ptr bindings");
    if (pointers_.at(*destination.pointer).fixed_binding) fail(at, "cannot rebind a const ptr binding");
    require(source.type, destination.type, at);
    if (pointers_.at(*destination.pointer).required && source.nullable) fail(at, "ref cannot be rebound to a nullable target");
    if (!destination.pending_owner && storage_owner(destination) != storage_owner(source))
      fail(at, "reference assignment cannot cross owners; you may want '<-' for bits transfer");
    valid_view(source, at);
    if (!destination.read_only && source.read_only)
      fail(at, "a writable ptr cannot rebind to a read-only view; use '<-' to copy bits or declare a separate read-only alias");
    for (const auto &loop : loop_pointers_)
      if (*destination.pointer < loop.boundary && !loop.initialized[*destination.pointer])
        fail(at, "loop-carried rebinding of an initially uninitialized target is not yet supported; initialize it before the loop");
  }
  Value rebind_pointer(const Value &destination, Value source) {
    source.pointer.reset();
    source.reference_field.reset();
    source.pointer_view = true;
    auto &binding = pointers_.at(*destination.pointer);
    source.read_only = binding.read_only;
    ir_.write_pointer(binding.slot, source.storage ? source.storage->address : source.native);
    binding.view = source;
    return source;
  }
  Value swap_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                        bool function_context) {
    if (expr.children.size() != 2) fail(expr.at, "swap requires two destinations");
    auto left = expression(*expr.children[0], module, env, locals, function_context, false);
    auto right = expression(*expr.children[1], module, env, locals, function_context, false);
    if (expr.text == "<=>") {
      if (!left.pointer || !right.pointer) fail(expr.at, "reference swap requires ordinary ptr bindings");
      left = pointer_value(*left.pointer, expr.at);
      right = pointer_value(*right.pointer, expr.at);
      if (pointers_.at(*left.pointer).fixed_binding || pointers_.at(*right.pointer).fixed_binding)
        fail(expr.at, "cannot rebind a const ptr binding");
    }
    if (!expr.intrinsic) {
      if (const auto selected = method(expr.text, module, {left, right}, expr.at))
        return call_method(*selected, {left, right}, expr.at, env, function_context, module);
    }
    if (expr.text == "<=>") {
      check_rebind(left, right, expr.at);
      check_rebind(right, left, expr.at);
      rebind_pointer(left, right);
      rebind_pointer(right, left);
    } else {
      if (!local_bits(left.type) || !local_bits(right.type) || !left.storage || !right.storage)
        fail(expr.at, "builtin bits swap requires two bits storage destinations");
      if (left.read_only || right.read_only) fail(expr.at, "cannot swap bits through a read-only view");
      require(left.type, right.type, expr.at);
      const auto left_bits = bits_value(read_value(left, expr.at));
      const auto right_bits = bits_value(read_value(right, expr.at));
      write_bits(left, right_bits, expr.at);
      write_bits(right, left_bits, expr.at);
    }
    return {};
  }
  Value query_value(const Expression &expr, const Module &module, const Locals &locals) {
    // Type queries inspect declarations and operand types, never execute a body
    // or emit an operand's loads, allocation, transfer or dynamic effects.
    using K = Expression::Kind;
    if (expr.kind == K::name) {
      if (const auto found = locals.visible.find(expr.text); found != locals.visible.end())
        return found->second.pointer ? pointers_.at(*found->second.pointer).view : found->second;
      if (const auto found = module.world.global_aliases.find(expr.text); found != module.world.global_aliases.end())
        return module.world.global_values.at(found->second);
    }
    if (expr.kind == K::symbol) {
      const auto id = expression_symbol(expr, module);
      declare_symbol_type(id);
      return scalar(symbol_types_.at(id), nullptr);
    }
    if (expr.kind == K::integer) return scalar(expr.text.ends_with("u8") ? Type::uint8 :
        expr.text.ends_with("u32") ? Type::uint32 : expr.text.ends_with("u64") ? Type::uint64 : Type::integer, nullptr);
    if (expr.kind == K::real) return scalar(expr.text.ends_with("f32") ? Type::float32 : Type::float64, nullptr);
    if (expr.kind == K::boolean) return scalar(Type::boolean, nullptr);
    if (expr.kind == K::null_ref) return scalar(Type::null_ref, nullptr);
    if (expr.kind == K::field) {
      auto value = query_value(*expr.children.front(), module, locals);
      if (value.type.kind == Type::regional)
        for (const auto &field : products_.at(value.type.identity).fields)
          if (field.name == expr.text) {
            const bool view = field.declaration_kind == "ref" || field.declaration_kind == "ptr";
            auto result = scalar(view ? *field.type.target : field.type, nullptr, value.region_id);
            result.pointer_view = view;
            return result;
          }
      fail(expr.at, "typeof: unavailable field '" + expr.text + "'");
    }
    if (expr.kind == K::index) {
      auto value = query_value(*expr.children.front(), module, locals);
      if (value.type.kind == Type::reference) value.type = *value.type.target;
      if (value.type.kind == Type::array) {
        const auto &array = arrays_.at(value.type.identity);
        require(query_value(*expr.children[1], module, locals).type, array.index, expr.at);
        value.type = array.element; return value;
      }
    }
    if (expr.kind == K::unary || expr.kind == K::binary || expr.kind == K::call) {
      std::vector<Value> args;
      for (const auto &child : expr.children) args.push_back(query_value(*child, module, locals));
      if (const auto selected = expr.intrinsic ? std::nullopt : method(expr.text, module, args, expr.at)) {
        const auto signature_value = signature(*selected->function.form, *selected->function.lexical, args, module, expr.at, true);
        if (!signature_value->result) fail(expr.at, "typeof requires a declared callable result type; the operand is not executed");
        return scalar(*signature_value->result, nullptr);
      }
      if (expr.kind == K::unary || (expr.kind == K::call && args.size() == 1 && (expr.text == "-" || expr.text == "!"))) {
        if (expr.text == "!") require(args[0].type, Type::boolean, expr.at);
        else if (!floating(args[0].type)) require(args[0].type, Type::integer, expr.at);
        return scalar(args[0].type, nullptr);
      }
      if (expr.kind == K::binary || (expr.kind == K::call && args.size() == 2 && binary_precedence(expr.text) >= 0)) {
        auto type = args[0].type;
        if (type != args[1].type && numeric(type) && numeric(args[1].type)) type = common_number(type, args[1].type, expr.at);
        else require(args[1].type, type, expr.at);
        if (expr.text == "&&" || expr.text == "||") require(type, Type::boolean, expr.at);
        else if (expr.text == "==" || expr.text == "!=") {
          if (!numeric(type) && type != Type::boolean) fail(expr.at, "typeof: equality requires scalar bits");
        } else if (!numeric(type)) fail(expr.at, "arithmetic requires numeric operands");
        if (expr.text == "%" && floating(type)) fail(expr.at, "remainder requires integer operands");
        if (expr.text == "==" || expr.text == "!=" || expr.text == "<" || expr.text == ">" ||
            expr.text == "<=" || expr.text == ">=" || expr.text == "&&" || expr.text == "||") return scalar(Type::boolean, nullptr);
        return scalar(type, nullptr);
      }
      if (expr.kind == K::call) {
        if (expr.text == "isnull") {
          if (args.size() != 1) fail(expr.at, "isnull requires one pointer operand");
          if (args[0].type != Type::null_ref && !args[0].pointer_view && args[0].type.kind != Type::reference)
            fail(expr.at, "isnull requires a ptr variable or reference field, not a copied payload");
          return scalar(Type::boolean, nullptr);
        }
        if (expr.text == "len") {
          if (args.size() != 1 || !(args[0].type.kind == Type::array || args[0].type.kind == Type::arguments ||
              (args[0].type.kind == Type::reference && args[0].type.target->kind == Type::array)))
            fail(expr.at, "typeof: len requires an array or argument pack");
          return scalar(Type::uint64, nullptr);
        }
        if (expr.text == "deref" && args.size() == 1) return args[0];
        if (expr.text == "root") {
          if (args.size() == 1 && args[0].type.target &&
              args[0].type.kind == (expr.text == "root" ? Type::region : Type::reference)) {
            auto value = args[0]; value.type = *value.type.target; return value;
          }
          fail(expr.at, "typeof: root/deref requires its declared reference kind");
        }
        if (const auto found = module.world.types.find(expr.text); found != module.world.types.end()) {
          auto type = found->second;
          if (type.kind != Type::unit || args.size() != 1 || !numeric(args[0].type))
            fail(expr.at, "typeof: this type has no applicable value constructor");
          type = quantity(type, args[0].type);
          return scalar(type, nullptr);
        }
        for (const auto &[name, type] : std::array<std::pair<std::string_view, Type>, 6>{{
            {"i64", Type::integer}, {"u8", Type::uint8}, {"u32", Type::uint32}, {"u64", Type::uint64},
            {"f32", Type::float32}, {"f64", Type::float64}}})
          if (expr.text == name) {
            if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
            return scalar(type, nullptr);
          }
      }
    }
    fail(expr.at, "typeof requires an expression with a statically declared or inferred type; its operand is never executed");
  }
  Type query_type(const Expression &expr, const Module &module, const Locals &locals, const Location &at) {
    const auto value = query_value(expr, module, locals);
    if (value.type == Type::void_type) fail(at, "typeof(void) cannot allocate local storage");
    return value.type;
  }
  Type array_type(const Value &value, const Location &at) {
    const auto type = value.type.kind == Type::reference ? *value.type.target : value.type;
    if (type.kind != Type::array) fail(at, "expected a regional array");
    check_nullable(value, at);
    region_pointer(value, at);
    return type;
  }
  Value array_element(Value array, Value index, const Location &at, bool as_reference = false) {
    if (array.arguments) {
      if (as_reference) fail(at, "a variadic argument pack has no regional element address");
      require(index.type, Type::uint64, at);
      const auto &elements = *array.arguments;
      if (const auto *constant = llvm::dyn_cast<llvm::ConstantInt>(index.native)) {
        if (constant->getZExtValue() >= elements.size()) fail(at, "variadic argument index out of bounds");
        const auto result = elements[constant->getZExtValue()];
        if (result.region_id) region_pointer(result, at);
        return result;
      }
      if (elements.empty()) {
        const auto type = *array.type.target;
        if (!numeric(type) && type != Type::boolean && type.kind != Type::quantity)
          fail(at, "an empty argument pack has no regional owner to borrow");
        return scalar(type, ir_.argument_element(type, index.native, {}));
      }
      auto result = elements.front();
      if (result.procedure) fail(at, "a dynamic argument index cannot select a procedure");
      std::vector<llvm::Value *> native, categories;
      for (const auto &stored : elements) {
        const auto element = read_value(stored, at);
        require(element.type, result.type, at);
        if (element.region_id != result.region_id) fail(at, "a dynamic argument index requires one owning region");
        if (element.region_id) region_pointer(element, at);
        std::erase_if(result.conformances, [&](const Conformance &relation) {
          return !establishes(element, relation, at);
        });
        native.push_back(element.native);
        if (result.type.kind == Type::exception) categories.push_back(exception_category(element));
      }
      result.native = ir_.argument_element(result.type, index.native, native);
      if (!categories.empty()) result.exception_category = ir_.argument_element(Type::uint64, index.native, categories);
      if (local_bits(result.type)) result = bits_value(std::move(result));
      return result;
    }
    const auto type = array_type(array, at);
    const auto &layout = arrays_.at(type.identity);
    require(index.type, layout.index, at);
    const auto place = ir_.element(region_pointer(array, at), array.native, index.type, index.native, layout.element);
    auto result = scalar(layout.element, nullptr, array.region_id);
    result.storage = StorageView{place.address, {}, {}};
    result.pointer_view = true;
    result.read_only = array.read_only;
    return result;
  }
  Value invoke_procedure(const Value &value, const Expression &expr, const Module &module,
                         Environment &env, Locals &locals, bool function_context, bool captured) {
    if (function_context) fail(expr.at, "a function cannot invoke a procedure");
    if (!value.procedure) fail(expr.at, "binding '" + expr.text + "' is not callable");
    if (!value.type.throws_types.empty()) require_npe_handler(expr.at);
    const auto proc = value.procedure; // Resolve before arguments/capture installation.
    std::vector<ConversionPoint> points;
    auto args = arguments(expr, module, env, locals, function_context, proc->form ? &points : nullptr,
        proc->form ? std::vector<Method>{{proc->form, proc->lexical}} : std::vector<Method>{});
    if (!proc->builtin.empty()) {
      if (args.size() != 1) fail(expr.at, "@" + proc->builtin + " expects one argument");
      if (!args[0].native) fail(expr.at, "read of storage before definite initialization in an argument");
      if (value.type.target) require(args[0].type, value.type.parameters[0], expr.at);
      if (proc->builtin == "assert") {
        require(args[0].type, Type::boolean, expr.at);
        ir_.assert_value(args[0].native);
      } else {
        if (!numeric(args[0].type) && args[0].type != Type::boolean)
          fail(expr.at, "@print expects a numeric scalar or bool");
        ir_.print(args[0].type, args[0].native);
      }
      return {};
    }
    {
      const auto instantiated = *signature(*proc->form, *proc->lexical, args, module, expr.at, true);
      const auto types = llvm::SaveAndRestore(type_context_, TypeContext{proc->lexical, instantiated.bindings});
      for (size_t i = 0; i < args.size(); ++i) {
        if (const auto path = route(args[i].type, parameter_at(*proc->form, i), *proc->lexical, module, expr.at))
          args[i] = converted(args[i], *path, expr.at, module, points.empty() ? nullptr : &points[i],
              !points.empty() && points[i].raw_view && !(parameter_at(*proc->form, i).declaration_kind == "ptr" && !parameter_at(*proc->form, i).owner.empty()));
      }
    }
    auto call_env = captured ? proc->captures : env;
    if (captured) {
      call_env.replay = std::make_shared<CaptureReplay>(CaptureReplay{
          env, proc->captures.captured_bindings, proc->captures.replay, {}});
      binding_check(call_env, expr.at);
    }
    auto result = call(*proc->form, *proc->lexical, std::move(call_env), args, false, expr.at, module, proc->implementation);
    check_declarations(proc->declarations, Form::Kind::procedure, args, result, module, expr.at, declares_npe(proc->form->throws_types, *proc->lexical, proc->form->at));
    return result;
  }
  Value invoke(const Expression &expr, const Module &module, Environment &env, Locals &locals,
               bool function_context) {
    const auto id = expression_symbol(expr, module);
    const auto value = read_symbol(id, env, expr.at, false);
    return invoke_procedure(value, expr, module, env, locals, function_context,
                            expr.kind == Expression::Kind::perform);
  }
  Value make_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                        bool function_context) {
    const auto root = resolve({Type::named, expr.text}, module, expr.at);
    if (root.kind != Type::regional) fail(expr.at, "make requires a concrete data root type");
    const auto &fields = products_.at(root.identity).fields;
    std::vector<size_t> order;
    std::set<size_t> initialized;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &initializer = *expr.children[i];
      const auto field = std::find_if(fields.begin(), fields.end(), [&](const auto &field) {
        return field.name == initializer.text;
      });
      if (field == fields.end()) fail(initializer.at, "unknown root field '" + initializer.text + "'");
      const auto index = size_t(field - fields.begin());
      if (!initialized.insert(index).second) fail(initializer.at, "duplicate root initializer '" + initializer.text + "'");
      order.push_back(index);
    }
    if (initialized.size() != fields.size()) fail(expr.at, "make must initialize every root field exactly once");
    const auto capacity = expression(*expr.children.front(), module, env, locals, function_context);
    if (!integral(capacity.type)) fail(expr.at, "make requires an integer byte capacity");
    regional_runtime_ = true;
    auto *native = ir_.region_new(capacity.type, capacity.native);
    Type type{Type::region}; type.owned = true; type.target = std::make_shared<Type>(root);
    auto owner = hold_region(scalar(type, native, new_region(native, expr.at)), module_top_level_, true);
    regions_.at(*owner.region_id).root = root;
    auto *offset = ir_.reserve_root(native, root);
    for (size_t i = 0; i < order.size(); ++i) {
      const auto &initializer = *expr.children[i + 1];
      const auto &field = fields[order[i]];
      const bool edge = field.type.kind == Type::reference;
      auto value = expression(*initializer.children.front(), module, env, locals, function_context, !edge);
      if (!field.declaration_kind.empty() && (field.declaration_kind == "ref" || field.declaration_kind == "ptr") && initializer.binding != "=")
        fail(initializer.at, "initialize a reference field with '='; '<-' writes an existing target");
      if (!edge && initializer.binding == "=") fail(initializer.at, "a bits field uses '<-' for payload initialization");
      if (edge) value = reference_source(std::move(value), *field.type.target, *owner.region_id, field.declaration_kind == "ref", initializer.at);
      else if (field.type.kind == Type::exception) value = admit(std::move(value), field.type, module, initializer.at);
      else require(value.type, field.type, initializer.at);
      const auto slot = ir_.field(native, offset, root, order[i], field.type);
      if (field.declaration_kind == "ref") ir_.require_representable_reference(slot, value.native);
      ir_.write(slot, stored_value(value, field.type, initializer.at));
    }
    return contextual(std::move(owner), module);
  }
  Value allocate_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    auto owner = expression(*expr.children[0], module, env, locals, function_context);
    if (owner.type.kind != Type::region) fail(expr.at, "alloc requires a whole region");
    if (owner.read_only) fail(expr.at, "cannot allocate through a read-only region view");
    region_pointer(owner, expr.at);
    auto type = resolve({Type::named, expr.text}, module, expr.at);
    if (type.kind != Type::array && type.kind != Type::regional)
      fail(expr.at, "alloc requires a regional type or regional array");
    std::vector<Value> args;
    for (size_t i = 1; i < expr.children.size(); ++i)
      args.push_back(expression(*expr.children[i], module, env, locals, function_context,
          type.kind == Type::array || i > products_.at(type.identity).fields.size() ||
          products_.at(type.identity).fields[i - 1].type.kind != Type::reference));

    if (type.kind == Type::array) {
      if (args.size() != 2 || !integral(args[0].type))
        fail(expr.at, "array allocation requires an integer length and initial element value");
      require(args[1].type, arrays_.at(type.identity).element, expr.at);

    } else {
      const auto &fields = products_.at(type.identity).fields;
      if (args.size() != fields.size()) fail(expr.at, "wrong field count for regional allocation");
      for (size_t i = 0; i < args.size(); ++i) {
        if (fields[i].type.kind == Type::reference)
          args[i] = reference_source(std::move(args[i]), *fields[i].type.target, *owner.region_id,
                                     fields[i].declaration_kind == "ref", expr.at);
        if (fields[i].type.kind == Type::exception) args[i] = admit(std::move(args[i]), fields[i].type, module, expr.at);
        else if (!(args[i].type == Type::null_ref && fields[i].type.kind == Type::reference))
          require(args[i].type, fields[i].type, expr.at);
        if (fields[i].type.kind == Type::reference && args[i].region_id && args[i].region_id != owner.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype" +
                           region_note(owner.region_id, "destination object's region acquired here") +
                           region_note(args[i].region_id, "referenced target's region acquired here"));

      }
    }
    std::vector<llvm::Value *> native_args;
    for (size_t i = 0; i < args.size(); ++i)
      native_args.push_back(type.kind == Type::regional ? stored_value(args[i], products_.at(type.identity).fields[i].type, expr.at) : args[i].native);
    auto *native = type.kind == Type::array ?
        ir_.array_new(region_pointer(owner, expr.at), arrays_.at(type.identity).element,
                      args[0].type, args[0].native, args[1].native) :
        ir_.record_new(region_pointer(owner, expr.at), type, native_args);
    auto result = scalar(type, native, owner.region_id);
    result.pointer_view = true;
    if (type.kind == Type::regional) {
      const auto &fields = products_.at(type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i)
        if (fields[i].declaration_kind == "ref")
          ir_.require_representable_reference(ir_.field(region_pointer(owner, expr.at), result.native, type, i, fields[i].type), args[i].native);
    }
    for (const auto &arg : args) merge_conformances(result.conformances, arg.conformances);
    return contextual(std::move(result), module);
  }
  Value loop_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    using K = Expression::Kind;
    const auto first = expression(*expr.children[0], module, env, locals, function_context);
    const auto last = expression(*expr.children[1], module, env, locals, function_context);
    require(last.type, first.type, expr.at);
    if (!integral(first.type) && !(first.type.kind == Type::quantity && integral(representation(first.type))))
      fail(expr.at, "a range requires integer endpoints or one integer-backed UoM identity");
    Value accumulator;
    if (expr.kind == K::fold) {
      accumulator = contextual(expression(*expr.children[2], module, env, locals, function_context), module);
      accumulator.pointer.reset();
      if (local_bits(accumulator.type)) accumulator = bits_value(std::move(accumulator));
      if (accumulator.type == Type::void_type || accumulator.type.kind == Type::procedure || accumulator.arguments ||
          accumulator.type == Type::null_ref)
        fail(expr.at, "a fold accumulator requires bits or a borrowed regional value");
      std::set<size_t> reachable;
      if (referenced_data(accumulator.type, reachable))
        std::erase_if(accumulator.conformances, [&](const Conformance &relation) {
          return !reachable.contains(relation.data.identity);
        });
    }
    const auto before = flow_state();
    if (accumulator.type.owned) accumulator = borrowed(std::move(accumulator));
    const auto *start = llvm::dyn_cast<llvm::ConstantInt>(first.native);
    const auto *end = llvm::dyn_cast<llvm::ConstantInt>(last.native);
    const bool signed_range = representation(first.type) == Type::integer;
    const bool empty = start && end && (signed_range ? start->getValue().sge(end->getValue()) :
                                                                     start->getValue().uge(end->getValue()));
    const bool one = start && end && !empty && end->getValue() - start->getValue() == 1;
    const size_t body_index = expr.kind == K::fold ? 3 : 2;
    const bool has_else = expr.kind == K::range_for && expr.children.size() == 4;
    const auto native_scope = ir_.scope();
    std::optional<LLVMEmission::Branch> empty_branch;
    if (has_else) empty_branch = ir_.branch(ir_.binary(first.type, "<", first.native, last.native));
    auto native_loop = ir_.loop(first.type, first.native, last.native, accumulator.native,
        accumulator.type.kind == Type::exception ? exception_category(accumulator) : nullptr);
    auto inner_locals = locals.nested();
    declare_local(inner_locals, expr.text, {first.type, native_loop.index, {}, nullptr}, module, expr.at);
    if (expr.kind == K::fold) {
      accumulator.native = native_loop.accumulator;
      accumulator.referent = native_loop.accumulator;
      accumulator.exception_category = native_loop.exception_category;
      declare_local(inner_locals, expr.binding, accumulator, module, expr.at);
    }

    std::set<size_t> rebound_pointers;
    const auto changed = [&](auto &&self, const Expression &item) -> void {
      const bool assignment = item.kind == K::rebind || (item.kind == K::call && item.text == "=");
      const bool swap = item.kind == K::call && item.text == "<=>";
      for (size_t i = 0; i < (swap ? 2u : assignment ? 1u : 0u) && i < item.children.size(); ++i) {
        const auto &target = *item.children[i];
        if (target.kind == K::name)
          if (const auto found = locals.visible.find(target.text); found != locals.visible.end() && found->second.pointer)
            rebound_pointers.insert(*found->second.pointer);
      }
      for (const auto &child : item.children) self(self, *child);
    };
    changed(changed, *expr.children[body_index]);
    LoopPointers loop_pointers{pointers_.size(), {}};
    for (auto &pointer : pointers_) {
      auto &view = pointer.view;
      const bool initialized = !view.storage || std::all_of(view.storage->cells.begin(), view.storage->cells.end(),
          [&](size_t cell) { return cells_.at(cell).initialized; });
      loop_pointers.initialized.push_back(initialized);
      if (!one && !empty && !pointer.fixed_binding && initialized) {
        if (!pointer.required && rebound_pointers.contains(size_t(&pointer - pointers_.data()))) view.nullable = true;
        view.referent = nullptr;
        if (view.storage) {
          view.storage->cells.clear();
          view.storage->initialized_alternatives = true;
        }
      }
    }
    if (!one && !empty) loop_pointers_.push_back(loop_pointers);
    const auto next = block(*expr.children[body_index], module, env, std::move(inner_locals), function_context,
                            expr.kind == K::range_for, {}, local_bits(accumulator.type));
    if (!one && !empty) {
      loop_pointers_.pop_back();
      if (next.type != Type::never)
        for (size_t i = 0; i < loop_pointers.boundary; ++i) {
          const auto &view = pointers_[i].view;
          if (loop_pointers.initialized[i] && view.storage &&
              !std::all_of(view.storage->cells.begin(), view.storage->cells.end(),
                  [&](size_t cell) { return cells_.at(cell).initialized; }))
            fail(expr.at, "loop-carried pointer target must remain definitely initialized on every continuing path");
        }
    }

    if (expr.kind == K::fold && next.type != Type::never) {
      require(next.type, accumulator.type, expr.at);
      if (next.region_id != accumulator.region_id)
        fail(expr.at, "a fold reference must retain its owning region" +
                         region_note(accumulator.region_id, "initial accumulator's region acquired here") +
                         region_note(next.region_id, "iteration result's region acquired here"));
      if (next.read_only && !accumulator.read_only)
        fail(expr.at, "a fold cannot feed a read-only view into a writable accumulator");
      for (const auto &relation : accumulator.conformances)
        if (!establishes(next, relation, expr.at) && !subtype(relation.data, relation.interface, module, expr.at))
          fail(expr.at, "a fold result must retain established conformance " + type_name(relation.data) +
                           " <: " + type_name(relation.interface));
    }
    auto after = flow_state();
    if (!empty && !one && next.type != Type::never)
      for (size_t i = 0; i < before.holdings.size(); ++i)
        if (before.holdings[i] != after.holdings[i])
          fail(expr.at, "a repeating loop cannot consume an owner from outside its iteration scope");
    ir_.end_loop(native_loop, next.native, next.type.kind == Type::exception ? exception_category(next) : nullptr);
    if (has_else) {
      const bool body_completes = !empty && next.type != Type::never;
      if (!body_completes) ir_.unreachable();
      ir_.restore(native_scope);
      ir_.otherwise(*empty_branch);
      restore_flow(before);
      const auto fallback = block(*expr.children[3], module, env, locals.nested(), function_context, true);
      const bool else_completes = (!(start && end) || empty) && fallback.type != Type::never;
      if (!else_completes && ir_.reachable()) ir_.unreachable();
      ir_.merge(*empty_branch, Type::void_type);
      if (body_completes) {
        if (else_completes) intersect_flow(after, flow_state());
        restore_flow(after);
      } else restore_flow(else_completes ? flow_state() : before);
      if (!body_completes && !else_completes) {
        if (ir_.reachable()) ir_.unreachable();
        return scalar(Type::never, nullptr);
      }
      return {};
    }
    if (empty || next.type == Type::never) restore_flow(before);
    else {
      if (!start || !end) intersect_flow(after, before);
      restore_flow(after);
    }
    if (next.type == Type::never && start && end && !empty) {
      ir_.unreachable();
      return scalar(Type::never, nullptr);
    }
    return accumulator;
  }
  Value call_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool function_result = false) {
    if (swap_operation(expr.text)) return swap_expression(expr, module, env, locals, function_context || function_result);
    if (expr.text == "=") return rebind_expression(expr, module, env, locals, function_context || function_result);
    if (expr.text == "<-") return transfer_expression(expr, module, env, locals, function_context || function_result);
    if (const auto local = locals.visible.find(expr.text); !expr.intrinsic && local != locals.visible.end()) {
      const auto callee = local->second;
      return invoke_procedure(callee, expr, module, env, locals, function_context || function_result, false);
    }
    if (expr.children.size() == 2 && binary_precedence(expr.text) >= 0)
      return binary_expression(expr, module, env, locals, function_context, function_result);
    if (expr.children.size() == 1 && (expr.text == "-" || expr.text == "!"))
      return unary_expression(expr, module, env, locals, function_context, function_result);
    if (expr.intrinsic || intrinsic_name(expr.text) || expr.text == "[]<-" || module.world.types.contains(expr.text) ||
        (type_context_.lexical == &module && type_context_.bindings.contains(expr.text)))
      return builtin_expression(expr, module, env, locals, function_context, function_result);
    const auto information = lookup_callable(module, expr.text, expr.at);
    const auto &functions = information.methods;
    if (functions.empty()) {
      if (!information.identities.empty())
        fail(expr.at, "callable '" + expr.text + "' has no method implementation" +
                     (information.declarations.empty() ? std::string{} :
                      "\n" + location_text(information.declarations.front().form->at) + ": note: callable signature declared here"));
      if (expr.text == "=" || expr.text == "[]=")
        fail(expr.at, "unavailable operation overload '(" + expr.text + ")'");
      fail(expr.at, "unavailable lexical function '" + expr.text + "'");
    }
    std::vector<ConversionPoint> points;
    const auto args = arguments(expr, module, env, locals, function_context,
        &points, functions);
    auto selected = method(expr.text, module, args, expr.at);
    if (!selected && functions.size() == 1) {
      const auto &candidate = functions.front();
      return call_method({candidate, std::vector<Route>(args.size()), information.declarations}, args, expr.at, env, function_context || function_result, module, points);
    }
    if (!selected) method_failure(expr.text, args, functions, expr.at, false, module);
    return call_method(*selected, args, expr.at, env, function_context || function_result, module, points);
  }
  Value builtin_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                           bool function_context, bool function_result) {
    using K = Expression::Kind;
    if (expr.text == "isnull") {
      if (expr.children.size() != 1) fail(expr.at, "isnull requires one pointer operand");
      const auto value = expression(*expr.children.front(), module, env, locals, function_context, false);
      if (!expr.intrinsic)
        if (const auto selected = method("isnull", module, {value}, expr.at))
          return call_method(*selected, {value}, expr.at, env, function_context || function_result, module);
      if (value.type == Type::null_ref) return scalar(Type::boolean, ir_.bits(Type::boolean, 1));
      if (!value.pointer_view && value.type.kind != Type::reference)
        fail(expr.at, "isnull requires a ptr variable or reference field, not a copied payload");
      valid_view(value, expr.at);
      auto result = scalar(Type::boolean, ir_.is_null(value.storage ? value.storage->address : value.native));
      if (value.pointer) result.nonnull_when = std::pair{*value.pointer, false};
      return result;
    }
    std::vector<ConversionPoint> points;
    std::optional<std::vector<Value>> evaluated;
    const auto values = [&](bool read = true) -> const std::vector<Value> & {
      if (!evaluated) evaluated = arguments(expr, module, env, locals, function_context,
          !expr.intrinsic && converting_method(expr.text, module, expr.at) ? &points : nullptr);
      if (read) for (const auto &value : *evaluated)
        if (value.storage && !value.native) fail(expr.at, "read of storage before definite initialization in an argument");
      return *evaluated;
    };
    if (!expr.intrinsic && intrinsic_name(expr.text) && expr.text != "setfield") {
      const auto &args = values(false);
      if (const auto selected = method(expr.text, module, args, expr.at))
        return call_method(*selected, args, expr.at, env, function_context || function_result, module, points);
    }
    if (expr.text == "[]<-") {
      const auto &args = values(false);
      if (args.size() != 3) fail(expr.at, "indexed transfer requires container, index and source");
      if (!expr.intrinsic)
        if (const auto selected = method(expr.text, module, args, expr.at))
          return call_method(*selected, args, expr.at, env, function_context || function_result, module, points);
      auto destination = array_element(args[0], args[1], expr.at);
      write_bits(destination, args[2], expr.at);
      destination.read_only = true;
      return destination;
    }
    if (expr.text == "cause_at") {
      const auto &args = values();
      if (args.size() != 2 || args[0].type.kind != Type::exception)
        fail(expr.at, "cause_at requires an exception and a u64 index");
      require(args[1].type, Type::uint64, expr.at);
      const auto [payload, category] = ir_.cause_at(args[0].native, args[1].native);
      auto result = scalar(exception_type(0), payload);
      result.exception_category = category;
      return result;
    }
    if (expr.text == "root") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::region || !args[0].type.target)
        fail(expr.at, "root requires one typed region");
      const auto &owner = args[0];
      auto value = scalar(*owner.type.target, ir_.region_root(region_pointer(owner, expr.at)), owner.region_id);
      value.conformances = owner.conformances;
      value.read_only = owner.read_only;
      return contextual(std::move(value), module);
    }
    if (expr.text == "region") {
      const auto &args = values();
      if (args.size() != 1 || !integral(args[0].type)) fail(expr.at, "region requires an integer byte capacity");
      regional_runtime_ = true;
      if (!region_initializer_) fail(expr.at, "a new region must be acquired by an effect or owning region declaration");
      auto *owner = ir_.region_new(args[0].type, args[0].native);
      const auto id = new_region(owner, expr.at);
      Type type{Type::region}; type.owned = true;
      return hold_region({type, owner, {}, nullptr, id}, module_top_level_, true);
    }
    if (expr.text == "used_bytes" || expr.text == "capacity_bytes") {
      const auto &args = values();
      if (args.size() != 1) fail(expr.at, "region size requires one resource");
      if (args[0].type.kind != Type::region) fail(expr.at, "region size requires a whole region");
      return scalar(Type::uint64, ir_.region_size(region_pointer(args[0], expr.at), expr.text == "capacity_bytes"));
    }
    if (expr.text == "deref") {
      const auto &args = values();
      if (args.size() != 1 || (!args[0].storage && !object_type(args[0].type)))
        fail(expr.at, "deref requires an object or storage view");
      return read_value(args[0], expr.at);
    }
    if (expr.text == "len" || expr.text == "ref_at") {
      const auto &args = values();
      if (args.size() != (expr.text == "len" ? 1u : 2u)) fail(expr.at, "wrong array argument count");
      if (expr.text == "ref_at") return array_element(args[0], args[1], expr.at, true);
      if (args[0].arguments) return scalar(Type::uint64, ir_.bits(Type::uint64, args[0].arguments->size()));
      array_type(args[0], expr.at);
      return scalar(Type::uint64, ir_.array_length(region_pointer(args[0], expr.at), args[0].native));
    }
    if (expr.text == "store") {
      const auto &args = values();
      if (!args.empty() && args[0].read_only) fail(expr.at, "cannot store through a read-only view");
      if (args.size() == 2 && args[0].storage && local_bits(args[0].type)) {
        write_bits(args[0], args[1], expr.at);
        return args[1];
      }
      if (args.size() != 3) fail(expr.at, "store requires an array/index/value or scalar reference/value");
      const auto type = array_type(args[0], expr.at);
      const auto &layout = arrays_.at(type.identity);
      require(args[1].type, layout.index, expr.at);
      require(args[2].type, layout.element, expr.at);
      ir_.write(ir_.element(region_pointer(args[0], expr.at), args[0].native, args[1].type, args[1].native, layout.element), args[2].native);
      return args[2];
    }
    if (expr.text == "setfield") {
      if (expr.children.size() != 3 || expr.children[1]->kind != K::name)
        fail(expr.at, "setfield requires a reference, field name and value");
      const auto object = expression(*expr.children[0], module, env, locals, function_context);
      if (object.read_only) fail(expr.at, "cannot change fields through a read-only view");
      const auto target = object.type.kind == Type::reference ? *object.type.target : object.type;
      if (target.kind != Type::regional)
        fail(expr.at, "setfield requires a regional record view or reference");
      check_nullable(object, expr.at);
      const auto type = resolve(target, module, expr.at);
      auto value = expression(*expr.children[2], module, env, locals, function_context, false);
      const auto &fields = products_.at(type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.children[1]->text) continue;
        const auto field = fields[i].type;
        if (field.kind == Type::reference) value = reference_source(std::move(value), *field.target, *object.region_id, fields[i].declaration_kind == "ref", expr.at);
        else value = read_value(std::move(value), expr.at);
        if (field.kind == Type::exception) value = admit(std::move(value), field, module, expr.at);
        else require(value.type, field, expr.at);
        if (field.kind == Type::reference && value.region_id && value.region_id != object.region_id)
          fail(expr.at, "cross-region reference fields are outside this prototype" +
                           region_note(object.region_id, "destination object's region acquired here") +
                           region_note(value.region_id, "referenced target's region acquired here"));
        region_pointer(object, expr.at);

        if (field.kind == Type::reference) {
          value.region_id = object.region_id;
          merge_conformances(value.conformances, object.conformances);
          value = contextual(std::move(value), module);
        }

        const auto slot = ir_.field(region_pointer(object, expr.at), object.native, type, i, field);
        if (fields[i].declaration_kind == "ref") ir_.require_representable_reference(slot, value.native);
        ir_.write(slot, stored_value(value, field, expr.at));
        return field.kind == Type::reference ? target_view(std::move(value), expr.at) : value;
      }
      fail(expr.at, "no such regional field '" + expr.children[1]->text + "'");
    }
    static const std::map<std::string, Type> casts{{"i64", Type::integer}, {"u8", Type::uint8},
        {"u32", Type::uint32}, {"u64", Type::uint64}, {"f32", Type::float32}, {"f64", Type::float64}};
    if (const auto cast = casts.find(expr.text); cast != casts.end()) {
      const auto &args = values();
      if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
      return scalar(cast->second, ir_.cast(args[0].type, cast->second, args[0].native));
    }
    if (module.world.types.contains(expr.text) ||
        (type_context_.lexical == &module && type_context_.bindings.contains(expr.text))) {
      const auto type = resolve_name({Type::named, expr.text}, module, expr.at);
      if (type.kind == Type::abstract_type) fail(expr.at, "an abstract type has no constructor or storage layout");
      if (type.kind == Type::regional || type.kind == Type::array)
        fail(expr.at, "regional objects require alloc in a region resource");
      const auto &args = values();
      if (numeric(type)) {
        if (args.size() != 1 || !numeric(args[0].type)) fail(expr.at, "numeric conversion requires one scalar value");
        return scalar(type, ir_.cast(args[0].type, type, args[0].native));
      }
      if (type.kind == Type::unit) {
        if (args.size() != 1) fail(expr.at, "a UoM constructor requires one scalar value");
        if (!numeric(args[0].type)) fail(expr.at, "a UoM constructor requires one scalar value");
        return scalar(quantity(type, args[0].type), args[0].native);
      }
      fail(expr.at, "this type has no value constructor");
    }
    if (expr.text == "scalar") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::quantity)
        fail(expr.at, "scalar requires one UoM value");
      return {representation(args[0].type), args[0].native, {}, nullptr};
    }
    fail(expr.at, "no builtin implementation for intrinsic (" + expr.text + ") with these arguments");
  }
  Value unary_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context, bool function_result = false) {
    using K = Expression::Kind;
    if (expr.text == "-" && expr.children[0]->kind == K::integer) {
      std::string_view magnitude = expr.children[0]->text;
      while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
      if (magnitude == "9223372036854775808")
        return scalar(Type::integer, ir_.bits(Type::integer, UINT64_C(1) << 63));
    }
    auto value = expression(*expr.children[0], module, env, locals, function_context);
    if (!expr.intrinsic) {
      if (const auto selected = method(expr.text, module, {value}, expr.at))
        return call_method(*selected, {value}, expr.at, env, function_context || function_result, module);
    }
    if (expr.text == "-" && floating(value.type))
      return scalar(value.type, ir_.unary(value.type, expr.text, value.native));
    require(value.type, expr.text == "!" ? Type::boolean : Type::integer, expr.at);
    auto result = scalar(value.type, ir_.unary(value.type, expr.text, value.native));
    if (expr.text == "!" && value.nonnull_when) result.nonnull_when = std::pair{value.nonnull_when->first, !value.nonnull_when->second};
    return result;
  }
  Value binary_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool function_result = false) {
    auto left = expression(*expr.children[0], module, env, locals, function_context);
    if (expr.text == "&&" || expr.text == "||") {
      require(left.type, Type::boolean, expr.at);
      auto right_env = env;
      auto right_locals = locals.nested();
      const auto rebound = llvm::SaveAndRestore(scope_rebound_, false);

      const auto native_scope = ir_.scope();
      const auto before = flow_state();
      auto native_branch = ir_.branch(left.native, expr.text == "||");
      Value right;
      try { right = expression(*expr.children[1], module, right_env, right_locals, function_context); }
      catch (const EndPath &) { right = scalar(Type::never, nullptr); }
      require(right.type, Type::boolean, expr.at);
      if (scope_rebound_ || ir_.scope() != native_scope)
        fail(expr.at, "conditional rebinding in a logical operand needs an explicit scope");
      ir_.otherwise(native_branch, right.native);
      auto joined = right.type == Type::never ? before : flow_state();
      intersect_flow(joined, before);
      restore_flow(joined);
      return scalar(Type::boolean, ir_.merge(native_branch, Type::boolean, left.native));
    }
    std::vector<ConversionPoint> points;
    if (!expr.intrinsic && converting_method(expr.text, module, expr.at)) points.push_back(conversion_point());
    auto right = expression(*expr.children[1], module, env, locals, function_context);
    if (!points.empty()) points.push_back(conversion_point());
    if (!expr.intrinsic) {
      if (const auto selected = method(expr.text, module, {left, right}, expr.at))
        return call_method(*selected, {left, right}, expr.at, env, function_context || function_result, module, points);
    }
    if ((expr.text == "==" || expr.text == "!=") &&
        (left.type.kind == Type::reference || left.type == Type::null_ref) &&
        (right.type.kind == Type::reference || right.type == Type::null_ref)) {
      if (left.type != Type::null_ref && right.type != Type::null_ref) require(right.type, left.type, expr.at);
      // The right operand can end an owner that the left operand borrowed.
      if (left.region_id) region_pointer(left, expr.at);
      if (right.region_id) region_pointer(right, expr.at);
      return scalar(Type::boolean, ir_.reference_equal(expr.text, left.native, right.native,
                      left.region_id && right.region_id && left.region_id != right.region_id));
    }
    if (left.type != right.type && numeric(left.type) && numeric(right.type)) {
      const auto common = common_number(left.type, right.type, expr.at);
      left = widened(left, common, expr.at);
      right = widened(right, common, expr.at);
    }
    require(right.type, left.type, expr.at);
    if (expr.text == "==" || expr.text == "!=") {
      if (!numeric(left.type) && left.type != Type::boolean) fail(expr.at, "equality requires scalar bits");
      return scalar(Type::boolean, ir_.binary(left.type, expr.text, left.native, right.native));
    }
    if (!numeric(left.type)) fail(expr.at, "arithmetic requires numeric operands");
    if (expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=")
      return scalar(Type::boolean, ir_.binary(left.type, expr.text, left.native, right.native));
    if (floating(left.type) && expr.text == "%") fail(expr.at, "remainder requires integer operands");
    return scalar(left.type, ir_.binary(left.type, expr.text, left.native, right.native));
  }
  struct Completion { Value value; llvm::BasicBlock *from; FlowState regions; };
  Value join_completions(const std::vector<Completion> &completions, const Location &at, bool module_lifetime) {
    if (completions.empty()) { ir_.unreachable(); return scalar(Type::never, nullptr); }
    if (local_bits(completions.front().value.type) &&
        std::any_of(completions.begin(), completions.end(), [](const Completion &completion) { return bool(completion.value.storage); }) &&
        std::any_of(completions.begin(), completions.end(), [](const Completion &completion) { return !completion.value.storage; })) {
      auto values = completions;
      for (auto &completion : values) completion.value = bits_value(std::move(completion.value));
      return join_completions(values, at, module_lifetime);
    }
    auto result = completions.front().value;
    if (result.storage && std::any_of(completions.begin(), completions.end(),
        [](const Completion &completion) { return !completion.value.native; })) result.native = nullptr;
    result.pointer.reset();
    auto state = completions.front().regions;
    std::set<size_t> origins;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> values, categories;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> addresses;
    for (const auto &completion : completions) {
      require(completion.value.type, result.type, at);
      result.read_only = result.read_only || completion.value.read_only;
      result.nullable = result.nullable || completion.value.nullable;
      result.nonnull_when.reset();
      if (!result.type.owned && (storage_owner(completion.value) != storage_owner(result) ||
                                bool(completion.value.storage) != bool(result.storage)))
        fail(at, "borrowed control-flow results require one storage owner and reference category");
      if (result.storage) {
        addresses.emplace_back(completion.value.storage->address, completion.from);
        result.storage->cells.insert(completion.value.storage->cells.begin(), completion.value.storage->cells.end());
        result.storage->initialized_alternatives |= completion.value.storage->initialized_alternatives;
      }
      if (result.type.owned) {
        const auto &source = regions_.at(*completion.value.region_id).origins;
        origins.insert(source.begin(), source.end());
      }
      intersect_flow(state, completion.regions);
      std::vector<Conformance> common;
      for (const auto &relation : result.conformances) {
        auto interface = std::optional(relation.interface);
        while (interface && !establishes(completion.value, {relation.data, *interface}, at)) {
          resolve_abstract(*interface, at);
          interface = abstracts_.at(interface->identity).parent;
        }
        if (interface) append_conformance(common, {relation.data, *interface});
      }
      result.conformances = std::move(common);
      if (result.native) values.emplace_back(completion.value.native, completion.from);
      if (result.type.kind == Type::exception && result.native)
        categories.emplace_back(exception_category(completion.value), completion.from);
    }
    restore_flow(state);
    if (!values.empty()) result.native = ir_.phi(ir_.type_of(result.type), values);
    if (!addresses.empty()) result.storage->address = ir_.phi(addresses.front().first->getType(), addresses);
    if (!categories.empty()) result.exception_category = ir_.phi(ir_.type_of(Type::uint64), categories);
    if (result.type.owned) {
      if (std::any_of(completions.begin(), completions.end(), [&](const Completion &completion) {
            return completion.value.region_id != result.region_id;
          })) result.region_id = new_region(result.native, at, std::move(origins));
      regions_.at(*result.region_id).live = true;
      if (result.type.target) regions_.at(*result.region_id).root = *result.type.target;
      result = hold_region(std::move(result), module_lifetime, true);
    }
    return result;
  }
  Value try_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                       bool function_context, bool copy_bits = false, bool read_result = true) {
    const bool parent_module = module_top_level_;
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    std::array<const Expression *, 4> catches{};
    const Expression *finally = nullptr;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &clause = *expr.children[i];
      if (clause.kind == Expression::Kind::block) { finally = &clause; continue; }
      if (clause.annotation) {
        const auto type = resolve(*clause.annotation, module, clause.at);
        if (type.kind != Type::exception || !type.identity)
          fail(clause.at, "a typed catch requires an exact intrinsic exception category");
        auto &destination = catches.at(type.identity - 1);
        if (destination) fail(clause.at, "duplicate exception category in catch clauses");
        destination = &clause;
      } else {
        for (auto &destination : catches) if (!destination) destination = &clause;
      }
    }
    HandlerScope protected_scope{ir_.scope(), holdings_.size(), {}};
    handlers_.push_back(&protected_scope);
    Value normal;
    {
      const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, npe_allowed_ || catches[3]);
      normal = block(*expr.children.front(), module, env, locals.nested(), function_context, false, {}, copy_bits, {}, read_result);
    }
    handlers_.pop_back();

    auto *join = ir_.label("try.result");
    std::vector<Completion> completions;
    const auto complete = [&](Value result) {
      if (result.type == Type::never) { ir_.restore(protected_scope.cleanup); return; }
      if (finally) {
        const auto finished = block(*finally, module, env, locals.nested(), function_context);
        require(finished.type, Type::void_type, finally->at);
        if (finished.type == Type::never) { ir_.restore(protected_scope.cleanup); return; }
      }
      if (result.region_id) region_pointer(result, expr.at);
      if (result.type.owned) result = take_owner(result, expr.at);
      ir_.restore(protected_scope.cleanup);
      completions.push_back({result, ir_.here(), flow_state()});
      ir_.jump(join);
    };
    complete(normal);

    const auto enter_landing = [&](Landing &landing, size_t category) {
      ir_.enter(landing.block);
      auto state = landing.incoming.front().regions;
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> edges;
      for (const auto &edge : landing.incoming) {
        intersect_flow(state, edge.regions);
        edges.emplace_back(edge.error.native, edge.from);
      }
      restore_flow(state);
      Value error = scalar(exception_type(category), ir_.phi(ir_.type_of(exception_type(category)), edges));
      error.exception_category = ir_.bits(Type::uint64, category);
      return error;
    };
    const auto propagate = [&](Value error) {
      if (finally) {
        auto finished = block(*finally, module, env, locals.nested(), function_context);
        require(finished.type, Type::void_type, finally->at);
        if (finished.type == Type::never) return;
      }
      raise_exception(error);
    };
    for (size_t i = 0; i < protected_scope.landing.size(); ++i) {
      auto &landing = protected_scope.landing[i];
      if (!landing.block) continue;
      auto error = enter_landing(landing, i + 1);
      const auto *clause = catches[i];
      if (!clause) { propagate(error); continue; }
      // Catch bodies do not see sibling handlers. Their exceptional exits must
      // still run this try's finally before proceeding to an outer handler.
      HandlerScope catch_scope{ir_.scope(), holdings_.size(), {}};
      if (finally) handlers_.push_back(&catch_scope);
      if (!clause->annotation) error.type = exception_type(0);
      auto inner_locals = locals.nested();
      declare_local(inner_locals, clause->text, error, module, clause->at);
      Value result;
      {
        const auto active = llvm::SaveAndRestore(caught_exception_, std::optional(error));
        result = block(*clause->children.front(), module, env, std::move(inner_locals), function_context, false, {}, copy_bits, {}, read_result);
      }
      if (finally) handlers_.pop_back();
      complete(result);
      for (size_t j = 0; j < catch_scope.landing.size(); ++j)
        if (catch_scope.landing[j].block) propagate(enter_landing(catch_scope.landing[j], j + 1));
    }
    ir_.enter(join);
    return join_completions(completions, expr.at, parent_module);
  }
  Value conditional_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool copy_bits = false, bool read_result = true) {
    const auto condition = expression(*expr.children[0], module, env, locals, function_context);
    require(condition.type, Type::boolean, expr.at);

    const bool parent_module = module_top_level_;
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto before = flow_state();
    const auto native_scope = ir_.scope();
    auto native_branch = ir_.branch(condition.native);
    if (condition.nonnull_when && condition.nonnull_when->second) pointers_.at(condition.nonnull_when->first).view.nullable = false;
    auto first = block(*expr.children[1], module, env, locals.nested(), function_context, false, {}, copy_bits, {}, read_result);
    if (first.type.owned) first = take_owner(first, expr.at);
    ir_.restore(native_scope);
    auto first_state = flow_state();
    ir_.otherwise(native_branch);
    restore_flow(before);
    if (condition.nonnull_when && !condition.nonnull_when->second) pointers_.at(condition.nonnull_when->first).view.nullable = false;
    auto second = block(*expr.children[2], module, env, locals.nested(), function_context, false, {}, copy_bits, {}, read_result);
    if (second.type.owned) second = take_owner(second, expr.at);
    ir_.restore(native_scope);
    if (first.type == Type::null_ref && storage_owner(second))
      first = null_target(second.type, second, expr.at);
    else if (second.type == Type::null_ref && storage_owner(first))
      second = null_target(first.type, first, expr.at);
    std::vector<Completion> completions;
    if (first.type != Type::never) completions.push_back({first, native_branch.first_end, std::move(first_state)});
    if (second.type != Type::never) {
      completions.push_back({second, ir_.here(), flow_state()});
      ir_.jump(native_branch.merge);
    }
    ir_.enter(native_branch.merge);
    return join_completions(completions, expr.at, parent_module);
  }
  Value expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                   bool function_context, bool read = true, bool copy_bits = false, bool read_nested = true) {
    if (clang::isStackNearlyExhausted()) {
#if LLVM_ENABLE_THREADS
      Value result;
      std::exception_ptr error;
      // WPO can have deep source call paths. Use Clang's existing stack
      // support, and resume C++ diagnostics on the originating stack.
      clang::runWithSufficientStackSpace([] {}, [&] {
        try { result = expression(expr, module, env, locals, function_context, read, copy_bits, read_nested); }
        catch (...) { error = std::current_exception(); }
      });
      if (error) std::rethrow_exception(error);
      return result;
#else
      fail(expr.at, "compiler stack capacity exhausted while elaborating this expression");
#endif
    }
    auto result = evaluate(expr, module, env, locals, function_context, copy_bits, read_nested);
    if (result.type == Type::never) throw EndPath{};
    if (copy_bits && local_bits(result.type)) return bits_value(read_value(std::move(result), expr.at));
    return read ? read_value(std::move(result), expr.at) : result;
  }
  Value declaration_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context) {
    using K = Expression::Kind;
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) {
          block(*declaration, module, env, locals.nested(), function_context, true);
          continue;
        }
        // Resolve the complete RHS before introducing this name, including the
        // transfer shorthand's source when it shadows an outer binding.
        auto value = expression(*declaration->children.front(), module, env, locals, function_context, false, expr.declaration_kind == "val" && declaration->children.front()->kind != K::make_local);
        if (value.type.kind == Type::region) fail(declaration->at, "a whole-region binding requires a region declaration, not ptr");
        if (expr.declaration_kind == "ref" && value.type.kind == Type::reference)
          value = target_view(std::move(value), declaration->at);
        if (expr.declaration_kind == "ptr" && value.type.kind == Type::reference && declaration->annotation &&
            declaration->annotation->kind != Type::reference)
          value = target_view(std::move(value), declaration->at);
        if (value.procedure && expr.declaration_kind == "ref" && expr.fixed_binding) {
          declare_local(locals, declaration->text, std::move(value), module, declaration->at);
          continue;
        }
        std::optional<Value> owner;
        if (!declaration->owner_name.empty()) {
          const auto found = locals.visible.find(declaration->owner_name);
          if (found == locals.visible.end()) fail(declaration->at, "unavailable pointer owner '" + declaration->owner_name + "'");
          owner = found->second;
          if (owner->type.kind != Type::region) fail(declaration->at, "an explicit pointer owner must name a region");
        }
        if (value.type == Type::null_ref) {
          if (expr.declaration_kind != "ptr" || declaration->binding != "=" || !declaration->annotation)
            fail(declaration->at, "null initialization requires ptr, an explicit target type, and '='");
          value = null_target(resolve(*declaration->annotation, module, declaration->at), owner, declaration->at);
        } else {
          if (declaration->annotation) value = admit(std::move(value), resolve(*declaration->annotation, module, declaration->at), module, declaration->at);
          if (owner && storage_owner(value) != storage_owner(*owner)) fail(declaration->at, "pointer initializer does not belong to its declared owner");
          const bool explicit_local = declaration->children.front()->kind == K::make_local;
          if (declaration->binding == "<-" || (expr.declaration_kind == "val" && !explicit_local) ||
              (!value.storage && local_bits(value.type))) {
            auto target = local_storage(value.type, declaration->at);
            write_bits(target, value, declaration->at);
            value = std::move(target);
          }
        }
        if (expr.declaration_kind == "val") {
          if (!local_bits(value.type)) fail(declaration->at, "val declarations require bits storage");
          if (owner) fail(declaration->at, "val local storage belongs to its lexical scope");
          value.read_only = expr.fixed_binding;
          value.pointer.reset();
          declare_local(locals, declaration->text, std::move(value), module, declaration->at);
        } else {
          if (expr.declaration_kind == "ref" && value.nullable)
            fail(declaration->at, "ref requires a proven non-null target");
          declare_pointer(locals, declaration->text, std::move(value), module, declaration->at, false, expr.fixed_binding,
                          expr.declaration_kind == "ref");
        }
      }
      return {};
  }
  Value field_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context) {
      const auto value = expression(*expr.children[0], module, env, locals, function_context);
      if (value.type.kind == Type::exception) {
        if (expr.text == "category") return scalar(Type::uint64, exception_category(value));
        if (expr.text == "cause_count") return scalar(Type::uint64, ir_.cause_count(value.native));
        const std::array<std::string_view, 3> fields{"site", "requested", "available"};
        const auto field = std::find(fields.begin(), fields.end(), expr.text);
        if (field == fields.end()) fail(expr.at, "unknown intrinsic exception field '" + expr.text + "'");
        return scalar(Type::uint64, ir_.exception_field(value.native, unsigned(field - fields.begin())));
      }
      if (value.type.kind != Type::regional)
        fail(expr.at, "field access requires a product value; dereference regional pointers first");
      const auto &fields = products_.at(value.type.identity).fields;
      for (size_t i = 0; i < fields.size(); ++i) {
        if (fields[i].name != expr.text) continue;
        if (fields[i].type.kind == Type::reference) {
          const auto slot = ir_.field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type);
          const auto offset = ir_.read(slot);
          auto result = scalar(fields[i].type, offset, value.region_id);
          result.nullable = true;
          result.reference_field = slot;
          result.pointer_view = true;
          result.required_reference = fields[i].declaration_kind == "ref";
          if (fields[i].declaration_kind == "ref" || fields[i].declaration_kind == "ptr") {
            result.type = *fields[i].type.target;
            result.nullable = fields[i].declaration_kind == "ptr";
            if (local_bits(result.type)) {
              result.storage = StorageView{ir_.nullable_address(slot.owner, offset), {}, {}};
              result.native = nullptr;
            }
          }
          result.conformances = value.conformances;
          result.read_only = value.read_only;
          return contextual(std::move(result), module);
        }
        const auto target = ir_.field(region_pointer(value, expr.at), value.native, value.type, i, fields[i].type);
        if (local_bits(fields[i].type)) {
          auto result = scalar(fields[i].type, nullptr, value.region_id);
          result.storage = StorageView{target.address, {}, {}};
          result.read_only = value.read_only;
          return result;
        }
        return loaded_value(fields[i].type, ir_.read(target));
      }
      fail(expr.at, "no field '" + expr.text + "' in " + type_name(value.type));
  }
  Value evaluate(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                 bool function_context, bool copy_bits = false, bool read_result = true) {
    const auto location = ir_.source_location(expr.at);
    using K = Expression::Kind;
    switch (expr.kind) {
    case K::try_scope: return try_expression(expr, module, env, locals, function_context, copy_bits, read_result);
    case K::handler: fail(expr.at, "catch requires an enclosing try");
    case K::raise: {
      Value error;
      if (expr.children.empty()) {
        if (!caught_exception_) fail(expr.at, "bare throw requires an active catch");
        error = *caught_exception_;
      } else error = expression(*expr.children.front(), module, env, locals, function_context);
      if (error.type.kind != Type::exception) fail(expr.at, "throw requires an intrinsic exception value");
      if (expr.children.size() == 2) {
        const auto cause = expression(*expr.children[1], module, env, locals, function_context);
        if (cause.type.kind != Type::exception) fail(expr.at, "an explicit cause requires an intrinsic exception value");
        error.native = ir_.exception_chain(error.native, cause.native, exception_category(cause));
      }
      if (may_be_npe(exception_category(error))) require_npe_handler(expr.at);
      raise_exception(error);
      return scalar(Type::never, nullptr);
    }
    case K::null_ref: return scalar(Type::null_ref, ir_.bits(Type::null_ref, 0));
    case K::integer: {
      auto spelling = expr.text;
      const auto type = spelling.ends_with("u8") ? Type::uint8 : spelling.ends_with("u32") ? Type::uint32 :
                        spelling.ends_with("u64") ? Type::uint64 : Type::integer;
      if (type != Type::integer) {
        spelling.resize(spelling.size() - (type == Type::uint8 ? 2 : 3));
        std::uint64_t value{};
        const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != spelling.data() + spelling.size() ||
            (type == Type::uint8 && value > UINT8_MAX) || (type == Type::uint32 && value > UINT32_MAX))
          fail(expr.at, "integer literal is outside " + type_name(type) + " range");
        return scalar(type, ir_.bits(type, value));
      }
      std::int64_t value{};
      const auto parsed = std::from_chars(expr.text.data(), expr.text.data() + expr.text.size(), value);
      if (parsed.ec != std::errc{} || parsed.ptr != expr.text.data() + expr.text.size())
        fail(expr.at, "integer literal is outside i64 range");
      return scalar(Type::integer, ir_.bits(Type::integer, value));
    }
    case K::real: {
      auto spelling = expr.text;
      const auto type = spelling.ends_with("f32") ? Type::float32 : Type::float64;
      if (spelling.ends_with("f32") || spelling.ends_with("f64")) spelling.resize(spelling.size() - 3);
      const auto in_range = [&](auto value) {
        const auto parsed = std::from_chars(spelling.data(), spelling.data() + spelling.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == spelling.data() + spelling.size() && std::isfinite(value);
      };
      // Validate in the declared precision: a double intermediate can round
      // a finite f32 literal onto an underflow/overflow boundary.
      if (!(type == Type::float32 ? in_range(float{}) : in_range(double{})))
        fail(expr.at, "floating literal is outside " + type_name(type) + " range");
      if (spelling.find_first_of(".eE") == std::string::npos) spelling += ".0";
      return scalar(type, ir_.floating_literal(type, spelling));
    }
    case K::boolean: return scalar(Type::boolean, ir_.bits(Type::boolean, expr.text == "true"));
    case K::pointer_group: return declaration_expression(expr, module, env, locals, function_context);
    case K::pointer: fail(expr.at, "ptr declarator requires a declaration group");
    case K::make_local: {
      const auto type = expr.annotation ? resolve(*expr.annotation, module, expr.at) :
          query_type(*expr.children.front(), module, locals, expr.at);
      auto value = local_storage(type, expr.at);
      const auto initial = expr.annotation ? 0u : 1u;
      if (expr.children.size() > initial) {
        auto source = block(*expr.children[initial], module, env, locals.nested(), function_context, false, type, true);
        write_bits(value, source, expr.at);
      }
      return value;
    }
    case K::region_binding: {
      const auto type = resolve(*expr.annotation, module, expr.at);
      const auto initializer = llvm::SaveAndRestore(region_initializer_, type.owned);
      auto value = admit(expression(*expr.children[0], module, env, locals, function_context), type, module, expr.at);
      if (type.owned) value = hold_region(take_owner(value, expr.at), module_top_level_, true);
      else value = borrowed(std::move(value));
      value.pointer.reset();
      declare_local(locals, expr.text, value, module, expr.at);
      return {};
    }

    case K::field: return field_expression(expr, module, env, locals, function_context);
    case K::index: {
      auto array = expression(*expr.children[0], module, env, locals, function_context);
      auto index = expression(*expr.children[1], module, env, locals, function_context);
      return array_element(array, index, expr.at);
    }
    case K::name: {
      if (const auto local = locals.visible.find(expr.text); local != locals.visible.end()) {
        if (local->second.pointer) return pointer_value(*local->second.pointer, expr.at);
        if (local->second.region_id) region_pointer(local->second, expr.at);
        return local->second;
      }
      if (const auto global = module.world.global_aliases.find(expr.text); global != module.world.global_aliases.end())
        return module.world.global_values.at(global->second);
      if (const auto information = lookup_callable(module, expr.text, expr.at); !information.methods.empty()) {
        const auto &methods = information.methods;
        if (methods.size() != 1) fail(expr.at, "a procedure value requires an unambiguous lexical declaration");
        const auto &method = methods.front();
        if (method.form->kind == Form::Kind::procedure && !method.form->operation) {
          const auto value = method.lexical->world.procedures.find(method.form);
          if (value == method.lexical->world.procedures.end()) fail(expr.at, "lexical procedure used before its definition");
          auto selected = value->second;
          auto procedure = std::make_shared<Procedure>(*selected.procedure);
          procedure->implementation = method.implementation;
          procedure->declarations = information.declarations;
          selected.procedure = std::move(procedure);
          return selected;
        }
      }
      fail(expr.at, "unavailable lexical value '" + expr.text + "'");
    }
    case K::symbol:
      if (function_context) fail(expr.at, "a function cannot read symbolic effects");
      return read_symbol(expression_symbol(expr, module), env, expr.at);
    case K::touch:
      if (function_context) fail(expr.at, "a function cannot touch symbolic effects");
      read_symbol(expression_symbol(expr, module), env, expr.at, false);
      return {};
    case K::procedure_effect: {
      if (function_context) fail(expr.at, "a function cannot establish procedure effects");
      const auto id = expression_symbol(expr, module);
      auto value = define_procedure(*expr.definition, module, env);
      bind(id, std::move(value), module, env, expr.at);
      scope_rebound_ = true;
      return {};
    }
    case K::effect: case K::effect_group: {
      if (function_context) fail(expr.at, "a function cannot establish effects");
      std::vector<const Expression *> bindings;
      if (expr.kind == K::effect) bindings.push_back(&expr);
      else for (const auto &child : expr.children) bindings.push_back(child.get());
      std::vector<std::pair<size_t, Value>> values;
      std::set<size_t> assigned;
      for (const auto *binding : bindings) {
        const auto id = expression_symbol(*binding, module);
        if (!assigned.insert(id).second) fail(binding->at, "duplicate symbol identity in effect group");
        declare_symbol_type(id);
        const auto type = symbol_types_.at(id);
        if (binding->binding == "=" && !object_type(type) && type.kind != Type::procedure)
          fail(binding->at, "effect '=' requires an object or procedure symbol; bits and region resources use '<-'");
        if (binding->binding == "<-" && (object_type(type) || type.kind == Type::procedure))
          fail(binding->at, "effect '<-' is limited to bits and whole-region acquisition; use '=' for object/procedure rebinding");
        const auto region_initializer = llvm::SaveAndRestore(region_initializer_,
            binding->children[0]->kind == K::call && binding->children[0]->text == "region");
        values.emplace_back(id, expression(*binding->children[0], module, env, locals, function_context));
      }
      for (auto &[id, value] : values) bind(id, value, module, env, expr.at, true);
      binding_check(env, expr.at);
      scope_rebound_ = true;
      return {};
    }
    case K::make_region: return make_expression(expr, module, env, locals, function_context);
    case K::transfer: return transfer_expression(expr, module, env, locals, function_context);
    case K::rebind: return rebind_expression(expr, module, env, locals, function_context);
    case K::allocate: return allocate_expression(expr, module, env, locals, function_context);
    case K::range_for: case K::fold: return loop_expression(expr, module, env, locals, function_context);
    case K::block: return block(expr, module, env, locals.nested(), function_context, false, {}, copy_bits, {}, read_result);
    case K::call: return call_expression(expr, module, env, locals, function_context);
    case K::perform: case K::behave: return invoke(expr, module, env, locals, function_context);
    case K::unary: return unary_expression(expr, module, env, locals, function_context);
    case K::binary: return binary_expression(expr, module, env, locals, function_context);
    case K::conditional: return conditional_expression(expr, module, env, locals, function_context, copy_bits, read_result);
    }
    fail(expr.at, "unsupported expression");
  }
  static void append_method(std::vector<Method> &methods, Method method) {
    if (std::find(methods.begin(), methods.end(), method) == methods.end()) methods.push_back(method);
  }
  static void append_declaration(std::vector<CallableDeclaration> &declarations, CallableDeclaration declaration) {
    if (std::find(declarations.begin(), declarations.end(), declaration) == declarations.end())
      declarations.push_back(declaration);
  }
  static bool operation_name(const std::string &name) {
    return intrinsic_name(name) || binary_precedence(name) >= 0 || name == "!" || name == "=" || name == "[]=" || name == "[]<-" || name == "<-" || swap_operation(name);
  }
  static CallableIdentity local_callable(const Module &module, const std::string &name) {
    return {operation_name(name) ? nullptr : &module, name};
  }
  static void declare_callable(Module &module, Form &form) {
    const auto identity = module.world.local_callables.emplace(form.name, local_callable(module, form.name)).first->second;
    auto &family = module.world.callables[identity];
    if (form.body) append_method(family.methods, {&form, &module});
    else append_declaration(family.declarations, {&form, &module});
    family.origins.insert({identity.module, form.name});
    module.world.callable_aliases[form.name] = identity;
  }
  static CallableInformation callable_information(const Module &module, const CallableIdentity &identity) {
    const auto &family = module.world.callables.at(identity);
    CallableInformation result{family.methods, family.origins, false, {identity}, family.declarations};
    for (const auto &member : family.members) {
      const auto contribution = callable_information(module, member);
      result.origins.insert(contribution.origins.begin(), contribution.origins.end());
      for (const auto &method : contribution.methods) append_method(result.methods, method);
      for (const auto &declaration : contribution.declarations) append_declaration(result.declarations, declaration);
    }
    return result;
  }
  // Integrating information changes only the receiving world's identity record.
  static void merge_callable(Module &module, const CallableIdentity &identity, const CallableInformation &information) {
    auto &family = module.world.callables[identity];
    family.origins.insert(information.origins.begin(), information.origins.end());
    for (const auto &method : information.methods) append_method(family.methods, method);
    for (const auto &declaration : information.declarations) append_declaration(family.declarations, declaration);
  }
  static void extend_imported_callable(Module &module, const std::string &alias, const CallableIdentity &identity) {
    const auto local = module.world.local_callables.find(alias);
    if (local == module.world.local_callables.end() || local->second == identity) return;
    const auto declaration = local->second;
    const auto &family = module.world.callables.at(declaration);
    // Scanning registers local overloads before their import is encountered.
    // Attach that declaration group to the selected identity, without merging
    // independently selected families or extending a different source world.
    if (!family.members.empty() || family.origins != std::set<MethodOrigin>{{&module, alias}}) return;
    merge_callable(module, identity, callable_information(module, declaration));
    for (auto *aliases : {&module.world.callable_aliases, &module.world.associated_aliases,
                         &module.world.local_callables})
      for (auto &[name, selected] : *aliases)
        if (selected == declaration) selected = identity;
    for (auto &[source, selected] : module.world.imported_callables)
      if (selected == declaration) selected = identity;
    for (auto &[name, published] : module.world.exports)
      if (auto *selected = std::get_if<CallableIdentity>(&published); selected && *selected == declaration)
        *selected = identity;
    for (auto &[owner, information] : module.world.callables)
      if (information.members.erase(declaration) && owner != identity) information.members.insert(identity);
    module.world.callables.erase(declaration);
  }
  static CallableIdentity combine_alias(Module &module, const std::string &name,
                                       const CallableIdentity &left, const CallableIdentity &right) {
    if (left == right) return left;
    const auto identity = local_callable(module, name);
    auto &family = module.world.callables[identity];
    if (left != identity) family.members.insert(left);
    if (right != identity) family.members.insert(right);
    return identity;
  }
  static std::set<std::string> method_names(const Module &module) {
    std::set<std::string> names;
    for (const auto &[name, identity] : module.world.callable_aliases) names.insert(name);
    for (const auto &[name, identity] : module.world.associated_aliases) names.insert(name);
    return names;
  }
  static CallableInformation lexical_callable(const Module &module, const std::string &name) {
    if (const auto found = module.world.callable_aliases.find(name); found != module.world.callable_aliases.end())
      return callable_information(module, found->second);
    if (module.world.lexical_names.contains(name)) return {};
    if (const auto found = module.world.associated_aliases.find(name); found != module.world.associated_aliases.end()) {
      auto result = callable_information(module, found->second);
      result.implicit = true;
      return result;
    }
    return {};
  }
  static void check_implicit_identity(const CallableInformation &set, const std::string &name, const Location &at) {
    if (!set.implicit || set.origins.size() < 2) return;
    std::string message = "ambiguous implicit callable '" + name +
                          "'; select its family with explicit using/import as";
    size_t shown = 0;
    for (const auto &[source, original] : set.origins) {
      if (shown++ == 4) break;
      if (!source) {
        message += "\n" + location_text(at) + ": note: compiler operation family '(" + original + ")'";
        continue;
      }
      const auto declaration = source->world.callables.find({source, original});
      const auto location = declaration != source->world.callables.end() && !declaration->second.methods.empty()
          ? location_text(declaration->second.methods.front().form->at) : source->path;
      message += "\n" + location + ": note: original callable family '" + original + "'";
    }
    if (set.origins.size() > 4)
      message += "\nnote: " + std::to_string(set.origins.size() - 4) + " additional callable families omitted";
    fail(at, message);
  }
  static bool related(const std::set<MethodOrigin> &left, const std::set<MethodOrigin> &right) {
    return std::any_of(left.begin(), left.end(), [&](const MethodOrigin &origin) { return right.contains(origin); });
  }
  static void select_type(Module &module, Type type, const std::string &alias, const Location &at) {
    require_user_type_name(alias, at);
    if (module.world.callable_aliases.contains(alias) || module.world.global_aliases.contains(alias))
      fail(at, "duplicate lexical alias '" + alias + "'");
    const auto [found, inserted] = module.world.types.emplace(alias, type);
    if (!inserted && found->second != type)
      fail(at, "duplicate lexical alias '" + alias + "'");
  }
  static CallableIdentity select_callable(Module &module, CallableInformation information, const std::string &alias,
                                          bool importing, bool implicit, const Location &at) {
    if (information.identities.size() != 1) fail(at, "a callable selection requires one identity");
    if (!implicit) {
      if (alias == "setfield")
        fail(at, "setfield overloads need a compile-time field-selector contract; outside this prototype");
      if (!intrinsic_name(alias)) require_user_type_name(alias, at);
      if (module.world.types.contains(alias) || module.world.global_aliases.contains(alias))
        fail(at, "duplicate lexical alias '" + alias + "'");
      for (const auto &method : information.methods)
        if (method.form->kind == Form::Kind::procedure && method.form->operation && !operation_name(alias))
          fail(at, "a procedure overload requires an operator or intrinsic destination");
      module.world.explicit_method_imports[alias] = module.world.explicit_method_imports[alias] || importing;
    }
    const auto source_identity = *information.identities.begin();
    auto identity = source_identity;
    if (operation_name(alias)) {
      identity = local_callable(module, alias);
    } else if (importing) {
      // Import adapts one source identity to this module's implementation
      // context. Another alias selects that same adaptation, not a new one.
      const auto proposed = CallableIdentity{&module, module.world.imported_callables.size()};
      identity = module.world.imported_callables.emplace(source_identity, proposed).first->second;
      extend_imported_callable(module, alias, identity);
    }
    auto &family = module.world.callables[identity];
    if (identity != source_identity && module.world.local_callables.contains(alias))
      family.origins.erase({&module, alias});
    merge_callable(module, identity, information);
    auto &aliases = implicit ? module.world.associated_aliases : module.world.callable_aliases;
    const auto [found, inserted] = aliases.emplace(alias, identity);
    if (!inserted) found->second = combine_alias(module, alias, found->second, identity);
    return found->second;
  }
  static bool context_dependent(const Expression &expr, const Module &lexical,
                                std::set<std::string> &locals) {
    if (expr.kind == Expression::Kind::call && !locals.contains(expr.text) && !operation_name(expr.text) &&
        !lexical.world.types.contains(expr.text)) return true;
    if (expr.kind == Expression::Kind::name && !locals.contains(expr.text) &&
        !lexical_callable(lexical, expr.text).methods.empty()) return true;
    if (expr.kind == Expression::Kind::block) {
      auto nested = locals;
      for (const auto &child : expr.children)
        if (context_dependent(*child, lexical, nested)) return true;
      return false;
    }
    const bool loop = expr.kind == Expression::Kind::range_for || expr.kind == Expression::Kind::fold;
    const auto count = loop ? (expr.kind == Expression::Kind::fold ? 3u : 2u) : expr.children.size();
    for (size_t i = 0; i < count; ++i) {
      // The field selector is a compile-time label, not a lexical value lookup.
      if (expr.kind == Expression::Kind::call && expr.text == "setfield" && i == 1) continue;
      if (context_dependent(*expr.children[i], lexical, locals)) return true;
    }
    if (loop) {
      auto nested = locals;
      nested.insert(expr.text);
      if (expr.kind == Expression::Kind::fold) nested.insert(expr.binding);
      if (context_dependent(*expr.children[count], lexical, nested)) return true;
      return expr.kind == Expression::Kind::range_for && expr.children.size() == 4 &&
             context_dependent(*expr.children[3], lexical, locals);
    }
    if (expr.kind == Expression::Kind::region_binding || expr.kind == Expression::Kind::pointer) locals.insert(expr.text);
    return false;
  }
  static bool context_dependent(const Method &method) {
    std::set<std::string> locals;
    for (const auto &parameter : method.form->parameters) locals.insert(parameter.name);
    for (const auto &parameter : method.form->type_parameters) locals.insert(parameter.name);
    return context_dependent(*method.form->body, *method.lexical, locals);
  }
  CallableInformation lookup_callable(const Module &module, const std::string &name, const Location &at) {
    auto source = lexical_callable(module, name);
    check_implicit_identity(source, name, at);
    for (auto &method : source.methods)
      if (method.implementation.empty()) method.implementation = {&module};
    std::map<CallableIdentity, CallableInformation> interpretations;
    for (const auto &identity : source.identities) interpretations.emplace(identity, source);
    const auto merge_information = [](CallableInformation &target, const CallableInformation &information) {
      target.origins.insert(information.origins.begin(), information.origins.end());
      target.identities.insert(information.identities.begin(), information.identities.end());
      for (const auto &method : information.methods) append_method(target.methods, method);
      for (const auto &declaration : information.declarations) append_declaration(target.declarations, declaration);
    };
    const Module *previous_world = &module;
    if (!operation_name(name)) for (const auto *world : implementation_context_) {
      if (world == &module) { previous_world = world; continue; }
      std::set<MethodOrigin> origins;
      for (const auto &[identity, information] : interpretations)
        origins.insert(information.origins.begin(), information.origins.end());
      std::map<CallableIdentity, CallableInformation> visible;
      for (const auto &alias : method_names(*world)) {
        if (operation_name(alias)) continue;
        auto candidate = lexical_callable(*world, alias);
        if (!related(origins, candidate.origins)) continue;
        for (const auto &identity : candidate.identities) {
          const auto [found, inserted] = visible.emplace(identity, candidate);
          if (!inserted && !candidate.implicit) found->second.implicit = false;
        }
      }
      std::map<CallableIdentity, CallableInformation> next;
      std::set<CallableIdentity> incorporated;
      for (auto &[identity, inherited] : interpretations) {
        auto selected_identity = identity;
        if (const auto imported = world->world.imported_callables.find(identity);
            imported != world->world.imported_callables.end() && visible.contains(imported->second))
          selected_identity = imported->second;
        const auto selected = visible.find(selected_identity);
        if (selected != visible.end()) {
          // Only the same identity or its explicit receiver import may replace
          // an inherited declaration's interpretation. A sibling sharing its
          // source body remains a distinct candidate, including nested helpers.
          check_implicit_identity(selected->second, name, at);
          std::erase_if(inherited.methods, [&](const Method &method) {
            return std::any_of(selected->second.methods.begin(), selected->second.methods.end(),
                [&](const Method &replacement) {
                  return method.form == replacement.form && method.lexical == replacement.lexical;
                });
          });
        }
        for (auto &method : inherited.methods)
          if (method.implementation.back() == previous_world && previous_world != world)
            method.implementation.push_back(world);
        inherited.identities = {selected_identity};
        merge_information(next[selected_identity], inherited);
        if (selected != visible.end()) {
          auto information = selected->second;
          for (auto &method : information.methods)
            if (method.implementation.empty()) method.implementation = {world};
          merge_information(next[selected_identity], information);
          incorporated.insert(selected_identity);
        }
      }
      for (auto &[identity, information] : visible) {
        if (incorporated.contains(identity)) continue;
        check_implicit_identity(information, name, at);
        for (auto &method : information.methods)
          if (method.implementation.empty()) method.implementation = {world};
        merge_information(next[identity], information);
      }
      interpretations = std::move(next);
      previous_world = world;
    }
    CallableInformation result;
    for (const auto &[identity, information] : interpretations) {
      for (const auto &declaration : information.declarations) append_declaration(result.declarations, declaration);
      result.identities.insert(identity);
      result.origins.insert(information.origins.begin(), information.origins.end());
      for (auto method : information.methods) {
        if (method.form->operation || !context_dependent(method)) method.implementation = {method.lexical};
        append_method(result.methods, std::move(method));
      }
    }
    return result;
  }
  static Method selected_method(Method method, const Module &source, const Module &receiver, bool importing) {
    if (!method.form->operation) {
      if (method.implementation.empty()) method.implementation = {&source};
      if (importing && method.implementation.back() != &receiver) method.implementation.push_back(&receiver);
    }
    return method;
  }
  bool associated_type(Type signature, Type selected, const Module &lexical, const Module &source,
                       const Location &at, bool applicable) {
    if (signature.kind == Type::named) {
      const auto found = lexical.world.types.find(signature.name);
      if (found == lexical.world.types.end()) return false; // Unrelated unavailable declarations stay demand-checked.
      signature = found->second;
    }
    if (signature == selected || (applicable && signature.kind == Type::abstract_type &&
                                 subtype(selected, signature, source, at))) return true;
    if (signature.target && associated_type(*signature.target, selected, lexical, source, at, false)) return true;
    return std::any_of(signature.parameters.begin(), signature.parameters.end(), [&](const Type &parameter) {
      return associated_type(parameter, selected, lexical, source, at, false);
    });
  }
  bool associated_signature(const Form &form, const Module &lexical, const Module &source, Type selected) {
    const auto associated = [&](auto &&self, const Type &type, bool applicable) -> bool {
      if (type.kind == Type::named) {
        for (const auto &parameter : form.type_parameters)
          if (parameter.name == type.name)
            return parameter.bound && associated_type(*parameter.bound, selected, lexical, source, parameter.at, applicable);
      }
      if (associated_type(type, selected, lexical, source, form.at, applicable)) return true;
      if (type.target && self(self, *type.target, false)) return true;
      return std::any_of(type.parameters.begin(), type.parameters.end(),
          [&](const Type &parameter) { return self(self, parameter, false); });
    };
    for (const auto &parameter : form.parameters)
      if (associated(associated, parameter.type, true)) return true;
    return form.result_type && associated(associated, *form.result_type, false);
  }
  void select_associated_methods(Module &module, const Module &source, Type selected, const Form &selection) {
    const bool importing = selection.kind == Form::Kind::import_resource;
    for (const auto &[alias, published] : source.world.exports) {
      const auto *identity = std::get_if<CallableIdentity>(&published);
      if (!identity) continue;
      auto information = callable_information(source, *identity);
      std::vector<Method> associated;
      for (const auto &method : information.methods) {
        if (associated_signature(*method.form, *method.lexical, source, selected))
          append_method(associated, selected_method(method, source, module, importing));
      }
      std::vector<CallableDeclaration> declarations;
      for (const auto &declaration : information.declarations)
        if (associated_signature(*declaration.form, *declaration.lexical, source, selected))
          append_declaration(declarations, declaration);
      if (associated.empty() && declarations.empty()) continue;
      information.methods = std::move(associated);
      information.declarations = std::move(declarations);
      const auto local = select_callable(module, std::move(information), alias, importing, true, selection.at);
      if (selection.exporting) publish(module, alias, local, selection.at);
    }
  }
  void check_method_extensions(const Module &module) {
    for (const auto &[name, identity] : module.world.local_callables) {
      const auto explicit_selection = module.world.explicit_method_imports.find(name);
      if (explicit_selection == module.world.explicit_method_imports.end() || explicit_selection->second || operation_name(name)) continue;
      const auto &family = module.world.callables.at(identity);
      fail(family.methods.empty() ? family.declarations.front().form->at : family.methods.front().form->at,
           "extending selected callable '" + name + "' requires import rather than using");
    }
    std::map<CallableIdentity, std::vector<std::pair<std::string, const Form *>>> declarations;
    for (const auto &[alias, identity] : module.world.local_callables) {
      const auto &methods = module.world.callables.at(identity).methods;
      const auto local = std::find_if(methods.begin(), methods.end(), [&](const Method &method) {
        return method.lexical == &module && method.form->name == alias;
      });
      if (local != methods.end()) declarations[identity].emplace_back(alias, local->form);
    }
    for (auto &[identity, aliases] : declarations) {
      if (aliases.size() < 2) continue;
      std::sort(aliases.begin(), aliases.end(), [](const auto &a, const auto &b) {
        const auto &left = a.second->at, &right = b.second->at;
        if (left.file != right.file) return left.file < right.file;
        if (left.line != right.line) return left.line < right.line;
        return left.column < right.column;
      });
      std::string message = "overloads declared through multiple aliases of one callable identity:";
      for (const auto &[alias, form] : aliases) message += " '" + alias + "'";
      for (const auto &[alias, form] : aliases)
        message += "\n" + location_text(form->at) + ": note: alias '" + alias + "' adds overloads here";
      warn(Warning::multi_alias_extension, aliases.back().second->at, std::move(message));
    }
  }
  static void publish(Module &module, const std::string &alias, PublishedIdentity identity, const Location &at) {
    const auto [found, inserted] = module.world.exports.emplace(alias, identity);
    if (!inserted && found->second != identity)
      fail(at, "export alias '" + alias + "' already publishes a different identity");
  }
  static const PublishedIdentity &published(const Module &source, const std::string &name,
                                             bool symbolic, const Location &at) {
    const auto alias = (symbolic ? "@" : "") + name;
    const auto found = source.world.exports.find(alias);
    if (found == source.world.exports.end())
      fail(at, "module '" + source.path + "' does not export '" + alias + "'");
    return found->second;
  }
  static size_t published_symbol(const Module &source, const std::string &name, const Location &at) {
    const auto &identity = published(source, name, true, at);
    const auto *symbol = std::get_if<size_t>(&identity);
    if (!symbol) fail(at, "published resource @" + name + " is not a symbol");
    return *symbol;
  }
  void publish_declarations(Module &module) {
    for (const auto &form : module.forms) {
      if (form->kind != Form::Kind::export_resource) continue;
      if (form->symbolic) {
        if (!module.world.available_symbols.contains(form->name))
          fail(form->at, "cannot export unavailable resource '@" + form->name + "'");
        publish(module, "@" + form->name, symbol(module, form->name, form->at), form->at);
      } else if (const auto type = module.world.types.find(form->name); type != module.world.types.end()) {
        publish(module, form->name, type->second, form->at);
      } else if (const auto global = module.world.global_aliases.find(form->name); global != module.world.global_aliases.end()) {
        publish(module, form->name, GlobalIdentity{global->second}, form->at);
      } else {
        const auto methods = lexical_callable(module, form->name);
        check_implicit_identity(methods, form->name, form->at);
        if (methods.identities.size() != 1)
          fail(form->at, "cannot export unavailable resource '" + form->name + "'");
        publish(module, form->name, *methods.identities.begin(), form->at);
      }
    }
  }
  const Module &dependency(const Module &receiver, const Form &selection, const Environment &env) {
    auto &source = *receiver.dependencies.at(&selection);
    execute_module(source, env, selection.at);
    return source;
  }
  void execute_module(Module &module, const Environment &caller, const Location &at) {
    // This composes runtime code after the complete module scan. A repeated
    // encounter reuses the incorporated module's resources, never its effects.
    if (module.state == Module::State::complete) return;
    if (module.state == Module::State::executing) fail(at, "cyclic module execution is outside the first prototype");
    const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, false);
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, true);
    const auto scope_id = scopes_.size();
    scopes_.push_back(true); // Module storage remains live through process exit.
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);
    const auto holding_begin = holdings_.size();
    module.state = Module::State::executing;
    execution_order_.push_back(module.path);
    if (module.builtin) {
      for (const auto &[name, id] : module.world.symbols) {
        auto proc = std::make_shared<Procedure>();
        proc->builtin = name;
        proc->lexical = &module;
        module.world.symbolic_resources.emplace(id, Value{Type::procedure, {}, std::move(proc), &module});
      }
      module.state = Module::State::complete;
      return;
    }
    const auto native_scope = ir_.scope();
    auto env = caller;
    Locals locals;

    try {
    for (const auto &owned : module.forms) {
      auto &form = *owned;
      switch (form.kind) {
      case Form::Kind::symbol:
        declare_symbol_type(symbol(module, form.name, form.at));
        module.world.available_symbols.insert(form.name);
        break;
      case Form::Kind::function: case Form::Kind::array: case Form::Kind::unit:
      case Form::Kind::export_resource: break;
      case Form::Kind::abstract_type:
        resolve_abstract(module.world.types.at(form.name), form.at);
        break;
      case Form::Kind::data:
        if (form.parent_type) declare_conformance(module, form);
        break;
      case Form::Kind::conformance:
        declare_conformance(module, form);
        break;
      case Form::Kind::using_units: {
        const auto &source = dependency(module, form, env);
        if (!module.world.unit_sources.insert(&source).second)
          fail(form.at, "UoMs from a source module must be selected in one using bundle");
        std::set<size_t> selected;
        for (const auto &selection : form.units) {
          const auto *selected_type = std::get_if<Type>(&published(source, selection.source, false, selection.at));
          if (!selected_type) fail(selection.at, "using units requires UoM definitions");
          const auto type = resolve_name(*selected_type, source, selection.at);
          if (type.kind != Type::unit) fail(selection.at, "using units requires UoM definitions");
          if (!module.world.selected_units.insert(type.identity).second)
            fail(selection.at, "repeated using of UoM identity " + type_name(type));
          selected.insert(type.identity);
          select_type(module, type, selection.local, selection.at);
        }
        for (const auto &eligible : source.world.conversions) {
          const auto bundle_context = eligible.implementation.empty() ? ImplementationContext{eligible.lexical} : eligible.implementation;
          for (const auto &[name, publication] : source.world.exports) {
            const auto *identity = std::get_if<CallableIdentity>(&publication);
            if (!identity) continue;
            const auto methods = callable_information(source, *identity);
            for (const auto &function : methods.methods) {
              if (function.form != eligible.form || function.lexical != eligible.lexical) continue;
              const auto visible_context = function.implementation.empty() ? ImplementationContext{function.lexical} : function.implementation;
              if (visible_context != bundle_context) continue;
              const auto edge = conversion(function);
              if (!selected.contains(edge.source.identity) || !selected.contains(edge.target.identity)) continue;
              check_implicit_identity(methods, name, form.at);
              const auto selected_conversion = selected_method(function, source, module, false);
              append_method(module.world.conversions, selected_conversion);
              auto information = methods;
              information.methods = {selected_conversion};
              select_callable(module, std::move(information), name, false, true, form.at);
            }
          }
        }
        break;
      }
      case Form::Kind::symbol_from: {
        const auto &source = dependency(module, form, env);
        module.world.symbols.emplace(form.alias, published_symbol(source, form.name, form.at));
        module.world.available_symbols.insert(form.alias);
        break;
      }
      case Form::Kind::procedure: {
        if (!form.body) {
          if (form.symbolic) {
            declare_symbol_type(symbol(module, form.name, form.at));
            module.world.available_symbols.insert(form.name);
          }
          break;
        }
        if (form.operation) break;
        auto value = define_procedure(form, module, env);
        if (form.symbolic) {
          const auto id = symbol(module, form.name, form.at);
          module.world.available_symbols.insert(form.name);
          bind(id, std::move(value), module, env, form.at);
        } else {
          module.world.procedures.emplace(&form, std::move(value));
        }
        break;
      }
      case Form::Kind::global: {
        if (module.world.callable_aliases.contains(form.name)) fail(form.at, "duplicate lexical name '" + form.name + "'");
        auto value = form.body->kind == Expression::Kind::call
            ? call_expression(*form.body, module, env, locals, false, true)
            : form.body->kind == Expression::Kind::unary
            ? unary_expression(*form.body, module, env, locals, false, true)
            : expression(*form.body, module, env, locals, false);
        if (value.region_id || value.type.kind == Type::reference || value.type.kind == Type::region)
          fail(form.at, "module globals contain bits values, not regional resources or references");
        require(value.type, resolve(*form.result_type, module, form.at), form.at);
        const MethodOrigin identity{&module, form.name};
        module.world.global_values.emplace(identity, std::move(value));
        module.world.global_aliases.emplace(form.name, identity);
        break;
      }
      case Form::Kind::expression:
        expression(*form.body, module, env, locals, false, form.body->kind != Expression::Kind::rebind &&
                   !(form.body->kind == Expression::Kind::call && form.body->text == "="));
        break;
      case Form::Kind::using_resource: case Form::Kind::import_resource: case Form::Kind::mixin_resource: {
        const bool mixing = form.kind == Form::Kind::mixin_resource;
        if (form.symbolic && !form.using_symbol && !module.world.available_symbols.contains(form.alias))
          fail(form.at, "procedure destination @" + form.alias + " must be declared before using/import/mixin");
        const auto &source = dependency(module, form, env);
        const auto &publication = published(source, form.name, form.symbolic, form.at);
        if (form.symbolic) {
          const auto source_id = published_symbol(source, form.name, form.at);
          const auto resource = source.world.symbolic_resources.find(source_id);
          if (resource == source.world.symbolic_resources.end() || !resource->second.procedure)
            fail(form.at, "source resource @" + form.name + " has no procedure value; use symbol from for identity");
          if (form.using_symbol) {
            const auto alias = module.world.symbol_aliases.find(form.alias);
            if (alias != module.world.symbol_aliases.end() && alias->second == &form) {
              module.world.symbols.emplace(form.alias, source_id);
              module.world.available_symbols.insert(form.alias);
            }
          }
          const auto destination = symbol(module, form.alias, form.at);
          if (!mixing && (form.using_symbol || !form.explicit_alias) &&
              destination != source_id)
            fail(form.at, "symbol identity mismatch: source @" + form.name + " is " + symbol_names_[source_id] +
                             ", local @" + form.alias + " is " + symbol_names_[destination] +
                             "; write using \"" + form.path + "\" @" + form.name + " as @" + form.alias +
                             "; to select that destination explicitly");
          auto value = resource->second;
          if (mixing)
            value = define_procedure(mixin_body(*value.procedure, form, module), module, env);
          else if (form.kind == Form::Kind::import_resource) {
            auto imported = std::make_shared<Procedure>(*value.procedure);
            if (imported->implementation.empty()) imported->implementation = {imported->lexical};
            if (imported->implementation.back() != &module) imported->implementation.push_back(&module);
            value.procedure = std::move(imported);
          }
          bind(destination, std::move(value), module, env, form.at);
          if (form.exporting) publish(module, "@" + form.alias, destination, form.at);
        } else if (const auto *type = std::get_if<Type>(&publication)) {
          if (mixing) fail(form.at, "mixin requires a procedure body, not a type definition");
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (type->kind == Type::unit)
            fail(form.at, "select UoMs and their conversions together with using units");
          select_type(module, *type, form.alias, form.at);
          if (type->kind == Type::regional) {
            if (form.kind == Form::Kind::import_resource) module.world.imported_data.insert(type->identity);
            for (const auto &relation : source.world.conformances) {
              if (relation.data == *type &&
                  std::find(module.world.conformances.begin(), module.world.conformances.end(), relation) == module.world.conformances.end())
                module.world.conformances.push_back(relation);
            }
          }
          select_associated_methods(module, source, *type, form);
          if (form.exporting) publish(module, form.alias, *type, form.at);
        } else if (const auto *identity = std::get_if<CallableIdentity>(&publication)) {
          const auto function = callable_information(source, *identity);
          check_implicit_identity(function, form.name, form.at);
          if (!form.alias_operation) require_user_type_name(form.alias, form.at);
          if (!mixing) {
            auto information = function;
            for (auto &method : information.methods)
              method = selected_method(method, source, module, form.kind == Form::Kind::import_resource);
            const auto local = select_callable(module, std::move(information), form.alias,
                            form.kind == Form::Kind::import_resource, false, form.at);
            if (form.exporting) publish(module, form.alias, local, form.at);
          } else {
            if (function.methods.empty()) fail(form.at, "mixin requires an implemented procedure method");
            for (const auto &candidate : function.methods) {
              if (candidate.form->kind != Form::Kind::procedure)
                fail(form.at, "mixin requires a procedure body, not a function definition");
              if (candidate.form->operation || form.alias_operation)
                fail(form.at, "mixin of operation overloads is outside this prototype");
              const auto original = candidate.lexical->world.procedures.find(candidate.form);
              if (original == candidate.lexical->world.procedures.end())
                fail(form.at, "procedure body requires its definition encounter before mixin");
              auto &body = mixin_body(*original->second.procedure, form, module);
              auto value = define_procedure(body, module, env);
              declare_callable(module, body);
              module.world.procedures.emplace(&body, std::move(value));
            }
          }
        } else if (const auto *identity = std::get_if<GlobalIdentity>(&publication)) {
          if (mixing) fail(form.at, "mixin requires a procedure body, not a global value");
          if (form.alias_operation) fail(form.at, "an operation name requires a callable overload");
          if (module.world.types.contains(form.alias) || module.world.callable_aliases.contains(form.alias))
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          const auto [alias, inserted] = module.world.global_aliases.emplace(form.alias, identity->origin);
          if (!inserted && alias->second != identity->origin)
            fail(form.at, "duplicate lexical alias '" + form.alias + "'");
          module.world.global_values.emplace(identity->origin, source.world.global_values.at(identity->origin));
          if (form.exporting) publish(module, form.alias, *identity, form.at);
        } else {
          fail(form.at, "unavailable module resource '" + form.name + "'");
        }
        break;
      }
      }
    }
    } catch (const EndPath &) {
      close_holdings(holding_begin, {}, at);
      ir_.restore(native_scope);
      module.state = Module::State::complete;
      throw;
    }
    check_method_extensions(module);
    publish_declarations(module);
    for (const auto &[id, current] : env.bindings) {
      if (current->owner != &module) continue;
      auto value = *current;
      if (value.type.kind != Type::procedure && value.type.kind != Type::region) {
        value.native = ir_.read_symbol(id, value.type);
      }
      module.world.symbolic_resources.emplace(id, std::move(value));
    }

    close_holdings(holding_begin, {}, at);
    ir_.restore(native_scope);
    module.state = Module::State::complete;
  }

public:
  explicit Compiler(WarningOptions warnings = {}) : warning_options_(std::move(warnings)) {
    ir_.on_exception([this](size_t category, llvm::Value *payload) {
      raise_exception(scalar(exception_type(category), payload));
    });
  }
  Compilation compile(const std::filesystem::path &entry) {
    clang::noteBottomOfStack(true);
    auto &module = load(entry.string(), {entry.string()});
    // All source files have been parsed before any execution semantics are lowered.

    try {
      execute_module(module, {}, {module.path});
    } catch (const EndPath &) {
      // The emitted program exits exceptionally; no later runtime forms execute.
    } catch (const std::runtime_error &error) {
      // Successful calls pop their sites. An error leaves the demanded path
      // intact, so format it once without copying a growing message on unwind.
      std::string message = error.what();
      for (size_t depth = 0; depth < call_path_.size(); ++depth) {
        if (depth == 4 && call_path_.size() > 8) {
          message += "\nnote: " + std::to_string(call_path_.size() - 8) + " intervening calls omitted";
          depth = call_path_.size() - 4;
        }
        const auto &site = call_path_[call_path_.size() - 1 - depth];
        const bool procedure = site.form->kind == Form::Kind::procedure;
        message += "\n" + location_text(site.at) + ": note: in call to " +
            (procedure ? "procedure '" : "function '") + callable_name(*site.form) +
            "' defined at " + location_text(site.form->at);
      }
      throw std::runtime_error(message);
    }
    for (const auto &check : binding_checks_) check_binding_owners(check.environment, check.at);
    auto [llvm_ir, bitcode] = ir_.finish();
    return {scanned_, execution_order_, warnings_, std::move(llvm_ir), std::move(bitcode), regional_runtime_};
  }
};

} // namespace cod::language
