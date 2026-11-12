//===--- lang_frontend.hh - Demand-checked CoD native lowering ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_llvm.hh"

#include <llvm/ADT/ImmutableMap.h>
#include <llvm/ADT/ScopeExit.h>
#include <clang/Basic/Stack.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <compare>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <exception>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <string_view>
#include <tuple>
#include <variant>

namespace cod::language {

struct Module;
struct Procedure;
struct CallableCode;
struct CaptureReplay;
struct CaptureUse {
  bool used = false;
  std::shared_ptr<CaptureUse> source;

  void require() {
    for (auto *capture = this; capture && !capture->used; capture = capture->source.get()) capture->used = true;
  }
};
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
struct UnresolvedAccess : std::runtime_error {
  explicit UnresolvedAccess(const Location &at) : std::runtime_error(location_text(at) +
      ": callable owner permissions still depend on an unresolved return or loop flow") {}
};
class FlowCondition {
  struct Deferred {
    std::function<bool()> evaluate;
    bool active = false;
  };
  bool constant_ = false;
  std::shared_ptr<Deferred> deferred_;
public:
  FlowCondition(bool value = false) : constant_(value) {}
  static FlowCondition deferred(std::function<bool()> evaluate) {
    FlowCondition value;
    value.deferred_ = std::make_shared<Deferred>(Deferred{std::move(evaluate)});
    return value;
  }
  bool pending() const { return bool(deferred_); }
  bool same(const FlowCondition &other) const { return constant_ == other.constant_ && deferred_ == other.deferred_; }
  operator bool() const {
    if (!deferred_) return constant_;
    // Positive boolean equations need a concrete fact; a cycle alone cannot
    // establish either an access restriction or a surviving owner.
    if (deferred_->active) return false;
    const auto active = llvm::SaveAndRestore(deferred_->active, true);
    return deferred_->evaluate();
  }
  static FlowCondition either(FlowCondition left, FlowCondition right) {
    if (!left.pending()) return left.constant_ ? left : right;
    if (!right.pending()) return right.constant_ ? right : left;
    return deferred([left, right] { return bool(left) || bool(right); });
  }
  static FlowCondition both(FlowCondition left, FlowCondition right) {
    if (!left.pending()) return left.constant_ ? right : left;
    if (!right.pending()) return right.constant_ ? left : right;
    return deferred([left, right] { return bool(left) && bool(right); });
  }
  FlowCondition &operator|=(FlowCondition other) { return *this = either(*this, std::move(other)); }
};
using ReadOnly = FlowCondition;
using ExceptionPossibilities = std::array<FlowCondition, concrete_exception_count>;
struct InitializationRead { size_t proof, cell; Location at; };
struct Value {
  Type type = Type::void_type;
  llvm::Value *native = nullptr; // Scalar payload, regional offset or native callable image.
  std::shared_ptr<const Procedure> procedure;
  const Module *owner = nullptr;
  std::optional<size_t> region_id = {};
  std::optional<size_t> holding_id = {};
  std::vector<Conformance> conformances = {}; // Selected type facts carried by a borrowed view, never runtime metadata.
  std::shared_ptr<const std::vector<Value>> arguments = {}; // WPO argument pack; no user allocation or runtime container.
  llvm::Value *exception_category = nullptr; // Propagation context, separate from the scalar payload.
  ReadOnly read_only = {};
  std::optional<StorageView> storage = {};
  std::optional<size_t> pointer = {}; // Lexical binding, distinct from its current referent.
  bool nullable = false;
  bool pending_owner = false;
  std::optional<std::pair<size_t, bool>> nonnull_when = {};
  std::optional<LLVMEmission::Place> reference_field = {};
  bool required_reference = false;
  bool pointer_view = false;
  llvm::Value *referent = nullptr; // Stable object expression identity across lexical pointer loads.
  ReadOnly binding_read_only = {}; // Permission to mutate the selected binding, independently of its target.
  bool reference_read_only = false; // Declared target access of a stored reference slot.
  llvm::Value *global_slot = nullptr;
  bool callable_temporary = false;
  size_t callable_address = 0; // Flow of lexical holder addresses, separate from their values.
  size_t callable_flow = 0; // Frozen value-flow node; zero has no runtime callable.
  std::shared_ptr<CaptureUse> capture_use = {}; // Compile-time demand for an enclosing dynamic capture.
  bool replayed = false; // Captured bits/procedure values are installed in the active TLS scope.
  bool provenance_only = false; // Erased owner binding; its actual owner is in Locals::owner_anchors.
  std::shared_ptr<const ExceptionPossibilities> exception_facts = {}; // Compiler reachability, independent of the native category.
  bool borrowed_arguments = false; // An external-owner variadic pack selects target views.
  size_t result_type_origin = 0;
  std::vector<size_t> result_fact_origins = {}; // Compiler-only checked returns, not runtime tags.
  std::vector<InitializationRead> argument_reads = {};
  std::optional<size_t> declaration_cell = {};
  llvm::Value *loop_control = nullptr; // Identity of an enclosing loop binding, never a runtime field.
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
  std::set<std::string> regional_slots; // Binding locations, never a property of a loaded value.
  std::map<std::string, Value> owner_anchors; // Type-level associations, separate from body captures.

  Locals nested() const { return {visible, {}, locations, regional_slots, owner_anchors}; }
  std::map<std::string, Value> owners() const {
    auto result = owner_anchors;
    for (const auto &[name, value] : visible) {
      if (value.provenance_only) continue;
      result.erase(name);
      if (value.type.kind == Type::region) result.emplace(name, value);
    }
    return result;
  }
  bool declare(const std::string &name, Value value, const Location &at) {
    if (!declared.insert(name).second) return false;
    visible.insert_or_assign(name, std::move(value));
    regional_slots.erase(name);
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
  Locals lexical_captures;
  ImplementationContext implementation{};
  std::vector<CallableDeclaration> declarations;
  std::shared_ptr<CallableCode> code;
};
struct CallableCode {
  LLVMEmission::NativeDefinition entry;
  llvm::StructType *layout;
  std::set<size_t> nested;
  std::vector<size_t> regions;
  std::vector<size_t> dynamic_values;
  llvm::Constant *dispatch_key = nullptr;
  std::vector<size_t> dynamic_procedures;
  std::vector<std::pair<size_t, std::shared_ptr<CaptureUse>>> capture_fields;
  std::shared_ptr<LLVMEmission::CallableConstruction> construction;
  std::vector<size_t> dynamic_regions;
  size_t result_flow = 0;
  bool direct = false;
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
// These records describe semantic selection, not lowered execution values.
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
  std::map<std::string, MethodOrigin> global_aliases;
};
// Values produced while compiling a module's once-only initialization. They
// retain runtime storage/capture provenance independently of semantic identity.
struct ModuleExecution {
  enum class State { pending, compiling, complete } state = State::pending;
  std::map<MethodOrigin, Value> global_values;
  std::map<const Form *, Value> procedures;
  std::map<size_t, Value> symbolic_resources;
};
struct Module {
  std::string path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, Module *> dependencies;
  ModuleWorld world;
  ModuleExecution execution;
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
enum class Warning { unit_conversion, multi_alias_extension, shadow, unused_capture, missing_return };
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
    std::map<std::string, Value> owner_values;
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
    FlowCondition live = true;
    std::set<size_t> origins;
    bool symbolic = false;
    std::optional<Type> root = {};
    size_t root_origin = 0;
    size_t owner_origin = 0;
    std::set<size_t> owner_inputs;
  };
  std::vector<Region> regions_;
  struct PendingField {
    Type record;
    size_t result_origin;
    Parameter field;
    llvm::Instruction *offset;
    Location at;
  };
  std::vector<PendingField> pending_fields_;
  struct Holding {
    size_t region;
    bool module_lifetime;
    llvm::Value *slot;
    Presence presence = Presence::present;
    bool movable = false;
    size_t scope = 0;
  };
  std::vector<Holding> holdings_;
  struct LocalCell { size_t scope; Location at; size_t initialized = 0; };
  // The slot is the runtime address; the view carries checked owner/type/access
  // constraints and initialization facts, not an exact runtime target identity.
  struct Pointer { llvm::Value *slot; const Value *view; ReadOnly read_only; bool fixed_binding = false; bool required = false; Location at; };
  struct DeferredCheck { std::function<bool()> rejected; Location at; std::string message; };
  std::vector<DeferredCheck> deferred_checks_;
  std::vector<ReadOnly> projection_permissions_;
  void require_writable(const ReadOnly &permission, const Location &at, std::string message) {
    if (permission.pending()) deferred_checks_.push_back({[permission] { return bool(permission); }, at, std::move(message)});
    else if (permission) fail(at, message);
  }
  void require_access(const ReadOnly &source, const ReadOnly &destination, const Location &at, std::string message) {
    const auto rejected = [source, destination] { return bool(source) && !bool(destination); };
    if (source.pending() || destination.pending()) deferred_checks_.push_back({rejected, at, std::move(message)});
    else if (rejected()) fail(at, message);
  }
  std::vector<LocalCell> cells_;
  struct InitializationFlow { std::vector<size_t> inputs; bool open = false; bool any = false; };
  // IDs 0/1 are false/true. Returning alternatives conjoin; preserving prior
  // initialization alongside a callee guarantee disjoins. Recursion contributes
  // no counterexample without a finite uninitialized exit.
  std::vector<InitializationFlow> initialization_flows_{{}, {}};
  std::vector<InitializationRead> initialization_reads_;
  std::vector<Pointer> pointers_;
  // Control-flow snapshots share unchanged views. A rebind or refinement adds
  // a version, leaving every saved branch/call/exception state intact.
  std::deque<Value> pointer_views_;
  using CallableAlternatives = std::vector<std::shared_ptr<const Procedure>>;
  using CallableMemory = llvm::ImmutableMap<size_t, size_t>;
  CallableMemory::Factory callable_memory_factory_;
  struct NativeCallDemand;
  struct CallableFlow {
    std::shared_ptr<const Procedure> definition;
    std::vector<size_t> inputs;
    bool open = false;
    std::optional<size_t> holder = {};
    size_t address = 0;
    CallableMemory memory{nullptr};
    std::optional<size_t> written_holder = {};
    bool loop = false;
    NativeCallDemand *producer = nullptr;
  };
  std::vector<CallableFlow> callable_flows_{CallableFlow{}};
  CallableMemory callable_holders_{nullptr};
  std::vector<size_t> callable_scopes_;
  struct CallableDemand { size_t flow; const Module *caller; Location at; };
  std::vector<CallableDemand> callable_demands_;
  struct CallableResult { size_t callee, result; };
  std::vector<CallableResult> callable_results_;
  std::vector<Value> direct_callables_;
  struct LoopPointers { size_t boundary; std::vector<bool> initialized; };
  std::vector<LoopPointers> loop_pointers_;
  std::vector<bool> scopes_{true};
  size_t local_scope_ = 0;
  bool module_top_level_ = false;
  bool region_initializer_ = false;
  bool npe_allowed_ = false;
  std::deque<ExceptionPossibilities> exception_flows_;
  bool regional_runtime_ = false;
  size_t next_unit_ = 0;
  LLVMEmission ir_;
  WarningOptions warning_options_;
  std::vector<std::string> warnings_;
  std::vector<std::pair<Location, std::string>> capture_warnings_;
  struct EndPath {};
  struct FlowState {
    std::vector<FlowCondition> live;
    std::vector<Presence> holdings;
    std::vector<size_t> initialized;
    std::vector<const Value *> pointers;
    CallableMemory callables{nullptr};
    std::map<size_t, size_t> deferred;
  };
  struct DeferredFlow {
    std::vector<size_t> inputs;
    std::set<size_t> catches;
    std::shared_ptr<bool> recovers = {};
    std::vector<size_t> continuations = {};
  };
  // Node zero is an empty queue. Registration prepends an action; joins and
  // loop headers retain every possible predecessor without copying stacks.
  std::deque<DeferredFlow> deferred_flows_{{{}, {}}};
  std::map<size_t, size_t> registered_deferred_;
  struct ConversionPoint {
    LLVMEmission::ConversionPoint native;
    FlowState regions;
    bool raw_view = false;
    std::optional<StorageView> callable_destination = {};
    bool callable_prepared = false;
  };
  struct NativeCallDemand {
    size_t flow;
    const Module *caller;
    Location at;
    Environment environment;
    bool captured;
    Type type;
    FlowState state;
    LLVMEmission::NativeDefinition definition;
    llvm::Instruction *context;
    std::vector<Conformance> conformances;
    std::vector<bool> scopes;
    std::vector<CallSite> calls;
    NativeCallDemand *parent = nullptr;
    std::map<size_t, size_t> returns = {};
    std::array<std::map<size_t, size_t>, concrete_exception_count> failures = {};
    std::set<const Procedure *> emitted = {};
    std::map<const Procedure *, LLVMEmission::CallableMatch> matches = {};
    llvm::BasicBlock *tail = nullptr;
    const NativeCallDemand *reused = nullptr;
    size_t result_flow = 0;
    std::vector<Value> arguments = {};
    struct Selection {
      std::string name;
      bool function_context;
      FlowCondition npe_allowed;
      ImplementationContext implementation;
      llvm::Instruction *image;
      Value result;
    };
    std::optional<Selection> selection = {};
    std::optional<size_t> conversion_parameter = {}; // Argument conversion uses the selected body's declaration.
    size_t record_result = 0;
    bool pending_argument_facts = false;
    std::map<size_t, size_t> initialized = {};
    std::array<std::map<size_t, size_t>, concrete_exception_count> failure_initialized = {};
  };
  std::deque<NativeCallDemand> native_call_demands_;
  size_t callable_compilations_ = 0;
  std::set<const NativeCallDemand *> checking_native_calls_;
  const NativeCallDemand *pending_callable_failure_ = nullptr;
  struct ReturnFacts {
    NativeCallDemand *producer = nullptr;
    std::vector<Value> alternatives;
  };
  std::deque<ReturnFacts> return_facts_;
  bool possible_result_facts_ = false;
  std::vector<std::pair<std::shared_ptr<CaptureUse>, std::shared_ptr<LLVMEmission::EffectWrites>>> capture_effects_;
  ConversionPoint conversion_point() { return {ir_.conversion_point(), flow_state()}; }
  struct ExceptionalEdge { Value error; llvm::BasicBlock *from; FlowState regions; Environment environment; };
  struct Landing { llvm::BasicBlock *block = nullptr; std::vector<ExceptionalEdge> incoming; };
  struct HandlerScope {
    size_t cleanup, holding_begin, owner_scope;
    Environment *environment;
    std::array<Landing, concrete_exception_count> landing;
  };
  std::vector<HandlerScope *> handlers_;
  struct DeferredAction {
    llvm::StructType *layout;
    Location at;
    std::vector<size_t> images;
    std::function<Value(llvm::Value *, Environment &, Value)> invoke;
    bool handles_error = false;
    std::shared_ptr<bool> recovers;
    size_t predecessors = 0;
    size_t registration = 0;
  };
  struct DeferredScope {
    llvm::AllocaInst *head;
    size_t owner_scope;
    std::deque<DeferredAction> actions;
    DeferredScope *parent = nullptr;
  };
  DeferredScope *deferred_scope_ = nullptr;
  std::set<llvm::Value *> deferred_loop_controls_;
  struct TryEmission {
    std::function<Value()> body;
    std::function<Value(Value)> cleanup;
    std::optional<Value> incoming;
    bool discard = false;
    std::function<bool(const FlowState &)> recovers;
    bool *recovered = nullptr;
  };
  struct EnvironmentEdge { Environment environment; llvm::BasicBlock *from; FlowState regions; };
  struct Completion { Value value; llvm::BasicBlock *from; FlowState regions; };
  struct ReturnTarget {
    size_t cleanup, holding_begin, owner_scope;
    Environment *environment;
    llvm::BasicBlock *block;
    std::vector<Completion> incoming = {};
    std::vector<EnvironmentEdge> environments = {};
  };
  struct ReturnContext {
    std::optional<Type> type;
    const Value *owner;
    bool copy, read;
    std::optional<size_t> destination;
  };
  ReturnContext *return_context_ = nullptr;
  std::vector<ReturnTarget *> return_targets_;
  std::vector<std::pair<size_t, Environment *>> active_environments_;
  std::optional<Value> caught_exception_;
  struct NativeActivation {
    Form *form;
    const Module *lexical;
    ImplementationContext implementation;
    std::vector<Type> parameters;
    std::map<size_t, std::pair<Type, const Value *>> bindings;
    LLVMEmission::NativeDefinition definition;
    NativeCallDemand *call = nullptr;
  };
  std::deque<NativeActivation> native_activations_;
  struct CheckedNative {
    NativeActivation activation;
    const Module *caller;
    std::vector<Conformance> conformances;
  };
  std::map<const Form *, std::vector<CheckedNative>> checked_natives_;
  struct CheckedFactory {
    NativeActivation activation;
    const Module *caller;
    std::vector<Conformance> conformances;
    std::vector<std::tuple<Type, std::optional<size_t>, size_t, bool>> arguments;
    Value result;
    bool checking = true;
  };
  std::deque<CheckedFactory> factories_;
  NativeActivation *native_context_ = nullptr;

  static Type exception_type(size_t category) {
    return {Type::exception, exception_categories.at(category).name, category};
  }
  static Type exception_type(ExceptionCategory category) {
    return exception_type(exception_id(category));
  }
  FlowCondition npe_coverage() const {
    if (npe_allowed_) return true;
    std::vector<size_t> queues;
    for (auto *scope = deferred_scope_; scope; scope = scope->parent)
      if (const auto found = registered_deferred_.find(scope->owner_scope); found != registered_deferred_.end())
        queues.push_back(found->second);
    if (queues.empty()) return false;
    return FlowCondition::deferred([this, queues = std::move(queues)] {
      return std::any_of(queues.begin(), queues.end(), [&](size_t queue) {
        return deferred_handles(queue, exception_id(ExceptionCategory::null_pointer));
      });
    });
  }
  void require_npe_handler(const Location &at, FlowCondition possible = true) {
    const auto coverage = npe_coverage();
    const std::string message = "checked NullPointerException requires a local catch or throws NullPointerException on the immediate func/proc; module top level must catch it";
    if (possible.pending() || coverage.pending())
      deferred_checks_.push_back({[possible, coverage] { return bool(possible) && !bool(coverage); }, at, message});
    else if (possible && !coverage) fail(at, message);
  }
  bool deferred_handles(size_t queue, size_t category) const {
    // A missing handler is witnessed by a finite path to the empty queue.
    // Revisiting a loop alone cannot create such a path.
    std::set<size_t> visited;
    std::vector<size_t> pending{queue};
    while (!pending.empty()) {
      const auto node = pending.back(); pending.pop_back();
      if (!node) return false;
      if (!visited.insert(node).second) continue;
      const auto &flow = deferred_flows_.at(node);
      if (flow.catches.contains(category)) continue;
      pending.insert(pending.end(), flow.inputs.begin(), flow.inputs.end());
    }
    return true;
  }
  size_t join_deferred_flow(size_t left, size_t right) {
    if (left == right) return left;
    deferred_flows_.push_back({{left, right}, {}});
    return deferred_flows_.size() - 1;
  }
  bool deferred_may_recover(size_t queue) const {
    std::set<size_t> visited;
    std::vector<size_t> pending{queue};
    while (!pending.empty()) {
      const auto node = pending.back(); pending.pop_back();
      if (!node || !visited.insert(node).second) continue;
      const auto &flow = deferred_flows_.at(node);
      if (flow.recovers && *flow.recovers) return true;
      pending.insert(pending.end(), flow.inputs.begin(), flow.inputs.end());
      pending.insert(pending.end(), flow.continuations.begin(), flow.continuations.end());
    }
    return false;
  }
  bool declares_npe(const std::vector<Type> &types, const Module &module, const Location &at) {
    for (const auto &type : types)
      if (resolve(type, module, at) != exception_type(ExceptionCategory::null_pointer))
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
    regions_.push_back({owner, at, true, std::move(origins), false, {}, 0, 0, {}});
    return id;
  }
  const Value *snapshot_pointer(Value value) {
    pointer_views_.push_back(std::move(value));
    return &pointer_views_.back();
  }
  FlowState flow_state() const {
    FlowState result;
    for (const auto &region : regions_) result.live.push_back(region.live);
    for (const auto &holding : holdings_) result.holdings.push_back(holding.presence);
    for (const auto &cell : cells_) result.initialized.push_back(cell.initialized);
    for (const auto &pointer : pointers_) result.pointers.push_back(pointer.view);
    result.callables = callable_holders_;
    result.deferred = registered_deferred_;
    return result;
  }
  void restore_flow(const FlowState &state) {
    for (size_t i = 0; i < regions_.size(); ++i) regions_[i].live = i < state.live.size() ? state.live[i] : FlowCondition{false};
    for (size_t i = 0; i < holdings_.size(); ++i)
      holdings_[i].presence = i < state.holdings.size() ? state.holdings[i] : Presence::absent;
    for (size_t i = 0; i < cells_.size(); ++i)
      cells_[i].initialized = i < state.initialized.size() ? state.initialized[i] : 0;
    for (size_t i = 0; i < state.pointers.size(); ++i) pointers_[i].view = state.pointers[i];
    callable_holders_ = state.callables;
    registered_deferred_ = state.deferred;
    for (const auto &[holder, flow] : state.callables)
      if (!scopes_.at(callable_scopes_.at(holder)))
        callable_holders_ = callable_memory_factory_.remove(callable_holders_, holder);
  }
  size_t join_callable_flow(std::vector<size_t> inputs) {
    std::sort(inputs.begin(), inputs.end());
    inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
    std::erase(inputs, size_t(0));
    if (inputs.empty()) return 0;
    if (inputs.size() == 1) return inputs.front();
    callable_flows_.push_back({{}, std::move(inputs)});
    return callable_flows_.size() - 1;
  }
  struct CallableChoices { CallableAlternatives definitions; bool closed = true; };
  struct CallableTargets { std::set<size_t> holders; bool closed = true; };
  CallableTargets callable_targets(size_t address, std::set<size_t> *unresolved = nullptr) const {
    CallableTargets result;
    std::vector<size_t> pending{address};
    std::set<size_t> visited;
    while (!pending.empty()) {
      const auto id = pending.back(); pending.pop_back();
      if (!id || !visited.insert(id).second) continue;
      const auto &node = callable_flows_.at(id);
      result.closed &= !node.open;
      if (node.open && unresolved) unresolved->insert(id);
      if (node.holder) result.holders.insert(*node.holder);
      pending.insert(pending.end(), node.inputs.begin(), node.inputs.end());
    }
    return result;
  }
  CallableChoices callable_choices(size_t flow, std::set<size_t> *unresolved = nullptr) const {
    CallableChoices result;
    using Read = std::tuple<size_t, size_t, size_t>; // Flow, selected address, selected holder.
    std::vector<Read> pending{{flow, 0, 0}};
    std::set<Read> visited;
    std::set<const Procedure *> definitions;
    while (!pending.empty()) {
      auto [id, address, holder] = pending.back(); pending.pop_back();
      if (!id) continue;
      const auto &node = callable_flows_.at(id);
      // A prior iteration may have selected another holder at this same site.
      if (node.loop) { address = 0; holder = 0; }
      if (!visited.emplace(id, address, holder).second) continue;
      result.closed &= !node.open;
      if (node.open && unresolved) unresolved->insert(id);
      if (node.address) {
        const auto targets = callable_targets(node.address, unresolved);
        result.closed &= targets.closed;
        if (node.written_holder) {
          if (address == node.address && holder == *node.written_holder) {
            pending.emplace_back(node.inputs.at(1), 0, 0);
            continue;
          }
          const bool selected = targets.holders.contains(*node.written_holder);
          if (selected) pending.emplace_back(node.inputs.at(1), 0, 0);
          if (!selected || targets.holders.size() != 1) pending.emplace_back(node.inputs.at(0), address, holder);
        } else for (const auto target : targets.holders)
          if (const auto *value = node.memory.lookup(target)) pending.emplace_back(*value, node.address, target);
        continue;
      }
      if (node.definition && definitions.insert(node.definition.get()).second)
        result.definitions.push_back(node.definition);
      for (const auto input : node.inputs) pending.emplace_back(input, address, holder);
    }
    return result;
  }
  void register_callable_holder(Value &value) {
    const auto holder = callable_scopes_.size();
    callable_scopes_.push_back(*value.storage->scope);
    callable_holders_ = callable_memory_factory_.add(callable_holders_, holder, value.callable_flow);
    CallableFlow node; node.holder = holder;
    callable_flows_.push_back(std::move(node));
    value.callable_address = callable_flows_.size() - 1;
  }
  size_t callable_result_flow(size_t callee) {
    callable_flows_.push_back({{}, {}, true});
    const auto result = callable_flows_.size() - 1;
    callable_results_.push_back({callee, result});
    return result;
  }
  bool resolve_callable_results() {
    bool changed = false;
    for (const auto &[callee, result] : callable_results_) {
      const auto choices = callable_choices(callee);
      auto &output = callable_flows_.at(result);
      for (const auto &candidate : choices.definitions) {
        const auto input = candidate->code->result_flow;
        if (!input) throw std::logic_error("callable result has no value-flow output");
        if (std::find(output.inputs.begin(), output.inputs.end(), input) == output.inputs.end()) {
          output.inputs.push_back(input);
          changed = true;
        }
      }
      if (output.open != !choices.closed) { output.open = !choices.closed; changed = true; }
    }
    return changed;
  }
  Value held_callable(Type type, size_t flow, llvm::Value *image, const StorageView &destination) {
    auto result = scalar(std::move(type), image);
    const auto choices = callable_choices(flow);
    if (!choices.definitions.empty()) result.procedure = choices.definitions.front();
    result.callable_flow = flow;
    result.storage = destination;
    result.callable_temporary = true;
    register_callable_holder(result);
    return result;
  }
  Value native_parameter(Type type, size_t flow, llvm::Value *argument) {
    if (type.kind != Type::procedure) return loaded_value(std::move(type), argument);
    return held_callable(std::move(type), flow, argument,
        StorageView{ir_.hold_callable(argument, false), local_scope_, {}});
  }
  void return_callable(const Value &result, size_t flow, const LLVMEmission::NativeDefinition &definition,
                       const Location &at) {
    if (!native_callable(result) || !result.storage)
      fail(at, "native factory result has no complete callable representation");
    auto &inputs = callable_flows_.at(flow).inputs;
    if (std::find(inputs.begin(), inputs.end(), result.callable_flow) == inputs.end())
      inputs.push_back(result.callable_flow);
    auto *image = ir_.take_callable(result.storage->address);
    ir_.write_pointer(definition.function->getArg(definition.context_argument + 1), image);
  }
  void intersect_flow(FlowState &left, const FlowState &right) {
    for (auto &[scope, queue] : left.deferred) {
      const auto found = right.deferred.find(scope);
      queue = join_deferred_flow(queue, found == right.deferred.end() ? 0 : found->second);
    }
    for (const auto &[scope, queue] : right.deferred)
      if (!left.deferred.contains(scope)) left.deferred.emplace(scope, join_deferred_flow(0, queue));
    left.live.resize(std::max(left.live.size(), right.live.size()), false);
    for (size_t i = 0; i < left.live.size(); ++i) left.live[i] = FlowCondition::both(left.live[i], i < right.live.size() ? right.live[i] : FlowCondition{false});
    left.holdings.resize(std::max(left.holdings.size(), right.holdings.size()), Presence::absent);
    for (size_t i = 0; i < left.holdings.size(); ++i) {
      const auto other = i < right.holdings.size() ? right.holdings[i] : Presence::absent;
      if (left.holdings[i] != other) left.holdings[i] = Presence::conditional;
    }
    left.initialized.resize(std::max(left.initialized.size(), right.initialized.size()), 0);
    for (size_t i = 0; i < left.initialized.size(); ++i)
      left.initialized[i] = join_initialization(left.initialized[i], i < right.initialized.size() ? right.initialized[i] : 0);
    for (size_t i = 0; i < std::min(left.pointers.size(), right.pointers.size()); ++i) {
      if (left.pointers[i] == right.pointers[i]) continue;
      auto target = *left.pointers[i];
      const auto &other = *right.pointers[i];
      if (target.pending_owner && !other.pending_owner) {
        target = other;
        target.nullable = true;
      } else if (!target.pending_owner && !other.pending_owner && storage_owner(target) != storage_owner(other)) {
        fail(pointers_[i].at, "pointer paths establish different owners; the first non-null assignment must establish one consistent owner");
      }
      target.nullable = target.nullable || other.nullable;
      target.read_only |= other.read_only;
      target.callable_address = join_callable_flow({target.callable_address, other.callable_address});
      if (target.referent != other.referent) target.referent = nullptr;
      if (target.storage && other.storage) {
        target.storage->cells.insert(other.storage->cells.begin(), other.storage->cells.end());
        target.storage->initialized_alternatives |= other.storage->initialized_alternatives;
      }
      left.pointers[i] = snapshot_pointer(std::move(target));
    }
    for (const auto &[holder, flow] : right.callables) {
      const auto *previous = left.callables.lookup(holder);
      left.callables = callable_memory_factory_.add(left.callables, holder,
          previous ? join_callable_flow({*previous, flow}) : flow);
    }
  }
  void close_state(FlowState &state, size_t begin, std::optional<size_t> result_owner = {},
                   std::optional<size_t> preserve_scope = {}) {
    state.live.resize(regions_.size(), false);
    state.holdings.resize(holdings_.size(), Presence::absent);
    std::set<size_t> released;
    for (size_t i = begin; i < holdings_.size(); ++i) {
      const auto &holding = holdings_[i];
      if (holding.module_lifetime || holding.scope == preserve_scope || state.holdings[i] == Presence::absent) continue;
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
                                         [&](size_t origin) { return released.contains(origin); })) {
        auto preserved = result_owner ? region_alias(*result_owner, i) : FlowCondition{false};
        for (size_t holder = 0; holder < holdings_.size(); ++holder)
          if (state.holdings[holder] == Presence::present)
            preserved |= region_alias(holdings_[holder].region, i);
        state.live[i] = FlowCondition::both(state.live[i], preserved);
      }
    }
  }
  Value hold_region(Value value, bool module_lifetime, bool movable) {
    value.holding_id = holdings_.size();
    holdings_.push_back({*value.region_id, module_lifetime, ir_.hold_region(value.native, module_lifetime),
                         Presence::present, movable, local_scope_});
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
  static ExceptionPossibilities exception_possibilities(const Value &value) {
    if (value.exception_facts) return *value.exception_facts;
    ExceptionPossibilities result{};
    if (value.type.kind != Type::exception) return result;
    if (value.type.identity) result.at(value.type.identity - 1) = true;
    else for (const auto category : LLVMEmission::possible_exceptions(value.exception_category))
      result.at(category - 1) = true;
    return result;
  }
  static void merge_exception_possibilities(ExceptionPossibilities &into, const Value &value) {
    const auto incoming = exception_possibilities(value);
    for (size_t i = 0; i < into.size(); ++i) into[i] |= incoming[i];
  }
  std::pair<size_t, std::shared_ptr<const ExceptionPossibilities>> pending_exception_flow(const Value &initial) {
    const auto flow = exception_flows_.size();
    exception_flows_.push_back(exception_possibilities(initial));
    auto facts = std::make_shared<ExceptionPossibilities>();
    for (size_t i = 0; i < facts->size(); ++i)
      facts->at(i) = FlowCondition::deferred([this, flow, i] { return bool(exception_flows_.at(flow)[i]); });
    return {flow, std::move(facts)};
  }
  void raise_exception(Value error) {
    const auto possibilities = exception_possibilities(error);
    std::set<size_t> categories;
    for (size_t i = 0; i < possibilities.size(); ++i)
      if (possibilities[i].pending() || possibilities[i]) categories.insert(i + 1);
    ir_.dispatch_exception(exception_category(error), [&](size_t category) {
      const auto possible = possibilities.at(category - 1);
      if (!possible.pending() && !possible) { ir_.unreachable(); return; }
      const auto memory = llvm::SaveAndRestore(callable_holders_);
      std::vector<std::pair<size_t, size_t>> initialized;
      const auto restore_initialization = llvm::scope_exit([&] {
        for (const auto &[cell, proof] : initialized) cells_[cell].initialized = proof;
      });
      if (pending_callable_failure_) {
        for (const auto &[cell, proof] : pending_callable_failure_->failure_initialized.at(category - 1)) {
          initialized.emplace_back(cell, cells_.at(cell).initialized);
          cells_[cell].initialized = preserved_initialization(cells_[cell].initialized, proof);
        }
        callable_holders_ = callable_effects(*pending_callable_failure_,
            pending_callable_failure_->failures.at(category - 1));
      }
      if (handlers_.empty()) {
        if (native_context_) {
          native_context_->definition.failures.insert(category);
          if (native_context_->call) {
            record_callable_effects(native_context_->call->failures.at(category - 1));
            record_initialization(native_context_->call->failure_initialized.at(category - 1));
          }
          ir_.unwind(0);
          ir_.native_return(nullptr, category, error.native);
        } else ir_.uncaught(category, error.native);
        return;
      }
      auto &scope = *handlers_.back();
      auto &landing = scope.landing.at(category - 1);
      if (!landing.block) landing.block = ir_.label("exception.handler");
      auto state = flow_state();
      close_state(state, scope.holding_begin, {}, scope.owner_scope);
      auto environment = *scope.environment;
      for (auto frame = active_environments_.rbegin(); frame != active_environments_.rend(); ++frame)
        if (frame->first == scope.owner_scope) { environment = *frame->second; break; }
      auto incoming = error;
      incoming.exception_category = ir_.bits(Type::uint64, category);
      auto selected = std::make_shared<ExceptionPossibilities>();
      selected->at(category - 1) = possible;
      incoming.exception_facts = std::move(selected);
      ir_.unwind(scope.cleanup, scope.owner_scope);
      landing.incoming.push_back({incoming, ir_.here(), std::move(state), std::move(environment)});
      ir_.jump(landing.block);
    }, std::move(categories));
  }

  void warn(Warning kind, const Location &at, std::string message) {
    const auto category = kind == Warning::unit_conversion ? "unit-conversion" :
                          kind == Warning::shadow ? "shadow" :
                          kind == Warning::unused_capture ? "unused-capture" :
                          kind == Warning::missing_return ? "missing-return" : "multi-alias-extension";
    const auto newline = message.find('\n');
    message.insert(newline == std::string::npos ? message.size() : newline,
                   " [-W" + std::string(category) + "]");
    if (warning_options_.as_errors || warning_options_.errors.contains(kind))
      fail(at, "warning treated as error: " + message);
    auto diagnostic = location_text(at) + ": warning: " + message;
    if (std::find(warnings_.begin(), warnings_.end(), diagnostic) == warnings_.end())
      warnings_.push_back(std::move(diagnostic));
  }

  void require_declaration(const Value &value, const Location &at) {
    if (value.declaration_cell) {
      const auto cell = *value.declaration_cell;
      require_initialization({cells_.at(cell).initialized, cell, at});
    }
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
    value.declaration_cell = cells_.size();
    cells_.push_back({local_scope_, at, 1});
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
  bool possible_region_alias(size_t region, size_t target, std::set<size_t> visited = {}) const {
    if (region == target) return true;
    if (!visited.insert(region).second) return false;
    return std::any_of(regions_.at(region).owner_inputs.begin(), regions_.at(region).owner_inputs.end(),
        [&](size_t input) { return possible_region_alias(input, target, visited); });
  }
  std::optional<size_t> returned_owner(size_t origin, const std::set<size_t> &inputs, const Location &at) {
    // A return is fresh or comes from an owning parameter. Recursive adapters
    // preserve parameter roles, not the allocation IDs of their first caller.
    using Provenance = std::set<size_t>; // Zero is fresh; parameter i is i + 1.
    using InputRoles = std::map<size_t, size_t>;
    std::map<const NativeCallDemand *, Provenance> summaries;
    InputRoles outer;
    for (size_t input : inputs) outer.emplace(input, outer.size() + 1);
    std::function<Provenance(size_t, const InputRoles &, std::set<size_t>)> region_value;
    const auto merge = [](Provenance &into, const Provenance &from) { into.insert(from.begin(), from.end()); };
    const auto result_value = [&](size_t id, const InputRoles &roles, const std::set<size_t> &visited) {
      const auto &returns = checked_returns(id, at);
      Provenance result;
      if (const auto *producer = return_facts_.at(id - 1).producer) {
        const auto summary = summaries.try_emplace(producer).first->second;
        for (size_t role : summary) {
          if (!role) result.insert(0);
          else merge(result, region_value(*producer->arguments.at(role - 1).region_id, roles, visited));
        }
      } else for (const auto &value : returns)
        if (value.region_id) merge(result, region_value(*value.region_id, roles, visited));
      return result;
    };
    region_value = [&](size_t id, const InputRoles &roles, std::set<size_t> visited) -> Provenance {
      if (const auto found = roles.find(id); found != roles.end()) return {found->second};
      if (!visited.insert(id).second) return {};
      const auto &region = regions_.at(id);
      if (!region.owner_origin || region.owner_inputs.empty()) return {0};
      return result_value(region.owner_origin, roles, visited);
    };
    Provenance result;
    for (;;) {
      const auto before = summaries.size();
      result = result_value(origin, outer, {});
      bool changed = false;
      for (auto &[demand, summary] : summaries) {
        const auto &returns = checked_returns(demand->record_result, at);
        Provenance next;
        if (demand->reused) next = summaries.try_emplace(demand->reused).first->second;
        else {
          InputRoles roles;
          for (size_t i = 0; i < demand->arguments.size(); ++i)
            if (demand->type.parameters[i].owned) roles.emplace(*demand->arguments[i].region_id, i + 1);
          for (const auto &value : returns)
            if (value.region_id) merge(next, region_value(*value.region_id, roles, {}));
        }
        const auto size = summary.size();
        merge(summary, next);
        changed |= size != summary.size();
      }
      if (!changed && summaries.size() == before) break;
    }
    if (result.size() != 1 || result.contains(0)) return {};
    for (const auto &[owner, role] : outer) if (result.contains(role)) return owner;
    return {};
  }
  size_t region_identity(size_t id, const Location &at) {
    const auto &region = regions_.at(id);
    if (!region.owner_origin || region.owner_inputs.empty()) return id;
    const auto inputs = region.owner_inputs;
    const auto owner = returned_owner(region.owner_origin, inputs, at);
    if (owner && *owner != id && possible_region_alias(id, *owner)) return region_identity(*owner, at);
    return id;
  }
  FlowCondition region_alias(size_t region, size_t target) {
    if (region == target) return true;
    if (!possible_region_alias(region, target)) return false;
    return FlowCondition::deferred([this, region, target] {
      const auto &at = regions_.at(region).acquired;
      return region_identity(region, at) == region_identity(target, at);
    });
  }
  void require_same_region(size_t first, size_t second, const Location &at, std::string message) {
    if (first == second) return;
    const auto rejected = [this, first, second, at] {
      return region_identity(first, at) != region_identity(second, at);
    };
    if (regions_.at(first).owner_origin || regions_.at(second).owner_origin)
      deferred_checks_.push_back({rejected, at, std::move(message)});
    else if (rejected()) fail(at, message);
  }
  void require_live(FlowCondition live, const Location &at, std::string message) {
    if (live.pending()) deferred_checks_.push_back({[live] { return !bool(live); }, at, std::move(message)});
    else if (!live) fail(at, message);
  }
  llvm::Value *region_pointer(const Value &value, const Location &at) {
    if (value.type.owned && (!value.holding_id || holdings_.at(*value.holding_id).presence != Presence::present))
      fail(at, "region owner has been consumed on a path reaching this use" +
                   region_note(value.region_id, "the region was acquired here"));
    require_live(value.region_id ? regions_.at(*value.region_id).live : FlowCondition{false}, at,
        "regional value requires a live owning region in this caller scope" +
        region_note(value.region_id, "the referenced region was acquired here"));
    return regions_.at(*value.region_id).owner;
  }
  static bool native_callable(const Value &value) {
    return value.type.kind == Type::procedure && (value.callable_flow || value.callable_address);
  }
  static bool local_bits(Type type) { return numeric(representation(type)) || type == Type::boolean || type.kind == Type::exception; }
  std::optional<std::pair<bool, size_t>> storage_owner(const Value &value) {
    if (value.region_id) return std::pair(true, region_identity(*value.region_id, regions_.at(*value.region_id).acquired));
    if (value.storage && value.storage->scope) return std::pair(false, *value.storage->scope);
    return {};
  }
  void valid_view(const Value &value, const Location &at) {
    for (const auto region : value.type.capture_regions)
      require_live(regions_.at(region).live, at,
          "callable owner contract " + type_name(value.type) + " requires a live region owner" +
          region_note(region, "this captured region's owner was acquired here"));
    if (value.procedure)
      for (const auto &[name, capture] : value.procedure->lexical_captures.visible) valid_view(capture, at);
    if (value.region_id) region_pointer(value, at);
    if (value.storage && value.storage->scope && !scopes_.at(*value.storage->scope))
      fail(at, "borrowed value would outlive its lexical storage owner");
  }
  std::map<size_t, Value> closure_regions(const Value &value, std::set<const Procedure *> active = {},
                                        const Location *complete_at = nullptr) {
    std::map<size_t, Value> result;
    if (native_callable(value)) {
      if (value.type.capture_regions.empty()) return result;
      if (complete_at) {
        if (!callable_choices(value.callable_flow).closed)
          throw UnresolvedAccess(*complete_at);
      }
      for (const auto &candidate : callable_choices(value.callable_flow).definitions) {
        auto source = value;
        source.procedure = candidate;
        source.callable_flow = 0;
        source.callable_address = 0;
        for (const auto &[id, access] : closure_regions(source, active, complete_at)) {
          const auto [found, inserted] = result.emplace(id, access);
          if (!inserted) found->second.read_only |= access.read_only;
        }
      }
      return result;
    }
    if (!value.procedure) return result;
    if (!active.insert(value.procedure.get()).second) return result;
    for (const auto &[name, owner] : value.procedure->lexical_captures.owner_anchors)
      if (owner.region_id) result.emplace(*owner.region_id, owner);
    for (const auto &[name, capture] : value.procedure->lexical_captures.visible) {
      auto dependencies = closure_regions(capture, active, complete_at);
      if (capture.region_id) dependencies.emplace(*capture.region_id, borrowed(capture));
      for (const auto &[id, access] : dependencies) {
        const auto [found, inserted] = result.emplace(id, access);
        if (!inserted) found->second.read_only = ReadOnly::both(found->second.read_only, access.read_only);
      }
    }
    return result;
  }
  Value callable_owner(const Value &callable, size_t id, const Location &at) {
    valid_view(callable, at);
    if (native_callable(callable)) {
      std::set<NativeCallDemand *> resolving;
      resolve_native_flow(callable.callable_flow, resolving);
    }
    const auto retained = closure_regions(callable);
    const auto access = retained.find(id);
    Type type{Type::region}; type.target = std::make_shared<Type>(*regions_.at(id).root);
    auto result = scalar(type, nullptr, id);
    if (native_callable(callable)) {
      result.read_only = ReadOnly::deferred([this, callable, id, at] {
        const auto complete = closure_regions(callable, {}, &at);
        const auto permission = complete.find(id);
        if (permission == complete.end()) fail(at, "callable does not retain its declared region owner");
        return bool(permission->second.read_only);
      });
      projection_permissions_.push_back(result.read_only);
    } else {
      if (access == retained.end()) fail(at, "callable does not retain its declared region owner");
      result.read_only = access->second.read_only;
    }
    if (access != retained.end()) {
      result.conformances = access->second.conformances;
      result.result_fact_origins = access->second.result_fact_origins;
    }
    return result;
  }
  Type bind_capture_type(Type type, const std::map<std::string, Value> &owners, const Location &at, bool partial = false) {
    if (type.kind != Type::procedure) return type;
    if (!type.capture_owners.empty()) {
      type.capture_owner_regions.resize(type.capture_owners.size());
      for (size_t i = 0; i < type.capture_owners.size(); ++i) {
        if (type.capture_owner_regions[i] || (!type.capture_formals.empty() && type.capture_formals[i])) continue;
        const auto &name = type.capture_owners[i];
        const auto found = owners.find(name);
        if (found == owners.end() && partial) continue;
        if (found == owners.end() || !found->second.region_id)
          fail(at, "callable environment owner '" + name + "' must name a declared, inferred region owner");
        type.capture_owner_regions[i] = *found->second.region_id;
        type.capture_regions.push_back(*found->second.region_id);
      }
      std::sort(type.capture_regions.begin(), type.capture_regions.end());
      type.capture_regions.erase(std::unique(type.capture_regions.begin(), type.capture_regions.end()), type.capture_regions.end());
    }
    for (auto &parameter : type.parameters) parameter = bind_capture_type(parameter, owners, at, partial);
    if (type.target) type.target = std::make_shared<Type>(bind_capture_type(*type.target, owners, at, partial));
    return type;
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
    if (owner && type.kind == Type::procedure) {
      value.procedure = owner->procedure;
      value.callable_address = owner->callable_address;
      value.callable_flow = owner->callable_flow;
    }
    if (local_bits(type) || type.kind == Type::procedure) {
      value.storage = StorageView{ir_.null_pointer(), owner && owner->storage ? owner->storage->scope : std::nullopt, {}};
    } else if (type.kind == Type::regional || type.kind == Type::array || type.kind == Type::reference) {
      value.native = ir_.bits(Type::null_ref, 0);
    } else fail(at, "null requires a pointer to bits or regional data");
    return value;
  }
  std::optional<bool> initialization_known(size_t proof) const {
    if (proof < 2) return proof == 1;
    std::vector<size_t> pending{proof};
    std::set<size_t> visited;
    std::map<size_t, std::vector<size_t>> dependents;
    bool open = false;
    while (!pending.empty()) {
      const size_t id = pending.back(); pending.pop_back();
      if (id < 2 || !visited.insert(id).second) continue;
      const auto &node = initialization_flows_.at(id);
      open |= node.open;
      for (size_t input : node.inputs) {
        dependents[input].push_back(id);
        pending.push_back(input);
      }
    }
    // Open producers may still add a returning alternative. Check both bounds
    // before accepting a proof or rejecting a read while bodies remain unchecked.
    const auto disproved = [&](bool unknown_exits) {
      std::set<size_t> rejected{0};
      std::vector<size_t> work(visited.begin(), visited.end());
      while (!work.empty()) {
        const size_t id = work.back(); work.pop_back();
        if (rejected.contains(id)) continue;
        const auto &node = initialization_flows_.at(id);
        const auto missing = [&](size_t input) { return rejected.contains(input); };
        const bool fails = (unknown_exits && node.open) || (node.any ?
            std::all_of(node.inputs.begin(), node.inputs.end(), missing) :
            std::any_of(node.inputs.begin(), node.inputs.end(), missing));
        if (fails) {
          rejected.insert(id);
          const auto found = dependents.find(id);
          if (found != dependents.end()) work.insert(work.end(), found->second.begin(), found->second.end());
        }
      }
      return rejected.contains(proof);
    };
    if (disproved(false)) return false;
    if (!open || !disproved(true)) return true;
    return {};
  }
  size_t preserved_initialization(size_t before, size_t returned) {
    if (before == returned || returned == 0) return before;
    if (before == 0) return returned;
    if (before == 1 || returned == 1) return 1;
    initialization_flows_.push_back({{before, returned}, false, true});
    return initialization_flows_.size() - 1;
  }
  size_t join_initialization(size_t a, size_t b) {
    if (a == b || b == 1) return a;
    if (a == 1) return b;
    if (!a || !b) return 0;
    initialization_flows_.push_back({{a, b}});
    return initialization_flows_.size() - 1;
  }
  void require_initialization(const InitializationRead &read) {
    const auto known = initialization_known(read.proof);
    if (known == false)
      fail(read.at, "read of storage before definite initialization\n" + location_text(cells_.at(read.cell).at) +
                   ": note: local storage allocated here");
    if (!known) initialization_reads_.push_back(read);
  }
  void require_argument_reads(const Value &value) {
    for (const auto &read : value.argument_reads) require_initialization(read);
  }
  static void require_runtime_value(const Value &value, const Location &at) {
    if (value.provenance_only)
      fail(at, "generic owner binding carries provenance, not an ordinary runtime value; region access requires a region-specific owner");
  }
  Value read_value(Value value, const Location &at, bool argument = false) {
    require_runtime_value(value, at);
    if (!argument) { require_argument_reads(value); value.argument_reads.clear(); }
    valid_view(value, at);
    if (value.result_type_origin && region_definition(value.type)) {
      try { resolve_result_type(value, at); }
      catch (const UnresolvedAccess &) { /* A reference read does not require its concrete layout. */ }
    }
    if (value.global_slot) {
      value.native = ir_.read({value.type, nullptr, value.global_slot});
      value.global_slot = nullptr;
      value.exception_facts.reset();
    }
    if (value.type.kind != Type::reference) {
      check_nullable(value, at);
      value.nullable = false;
    }
    if (value.storage) {
      for (const auto cell : value.storage->cells) {
        InitializationRead read{cells_.at(cell).initialized, cell, at};
        if (argument) {
          if (initialization_known(read.proof) != true) value.argument_reads.push_back(read);
        } else require_initialization(read);
      }
      const auto loaded = loaded_value(value.type, ir_.read(place(value, at)));
      value.native = loaded.native;
      value.exception_category = loaded.exception_category;
      value.exception_facts.reset();
    }
    if (value.callable_address) {
      CallableFlow load; load.address = value.callable_address; load.memory = callable_holders_;
      callable_flows_.push_back(std::move(load));
      value.callable_flow = callable_flows_.size() - 1;
    }
    return value;
  }
  static Value bits_value(Value value) {
    value.storage.reset(); value.pointer.reset(); value.region_id.reset(); value.read_only = false; value.referent = nullptr; value.nullable = false; value.reference_field.reset(); value.nonnull_when.reset(); value.pointer_view = false;
    value.binding_read_only = false;
    value.reference_read_only = false;
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
    if (!local_bits(destination.type)) fail(at, "bits transfer requires bits storage");
    require_writable(destination.read_only, at, "cannot transfer into a read-only destination view");
    source = read_value(std::move(source), at);
    require(source.type, destination.type, at);
    ir_.write(place(destination, at), stored_value(source, destination.type, at));
    if (destination.storage->cells.size() == 1 && !destination.storage->initialized_alternatives)
      cells_.at(*destination.storage->cells.begin()).initialized = 1;
  }
  Value pointer_value(size_t id, const Location &at) {
    auto value = *pointers_.at(id).view;
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
    value.reference_read_only = false;
    value.required_reference = false;
    value.binding_read_only = false;
    value.pointer_view = true;
    value.callable_temporary = false;
    value.read_only |= read_only;
    valid_view(value, at);
    if (!storage_owner(value) && !value.pending_owner) fail(at, "ptr requires storage with a known owner");
    const auto id = pointers_.size();
    if (!value.referent) value.referent = value.storage ? value.storage->address : value.native;
    pointers_.push_back({ir_.pointer_storage(value.storage ? value.storage->address : value.native), snapshot_pointer(value),
                         value.read_only, fixed_binding, required, at});
    value.pointer = id;
    declare_local(locals, name, std::move(value), module, at);
  }
  void close_holdings(size_t begin, const Value &result, const Location &at) {
    auto state = flow_state();
    close_state(state, begin, result.type.owned ? result.region_id : std::nullopt);
    if (result.region_id) require_live(state.live.at(*result.region_id), at,
        "regional result would outlive its owning scope" +
        region_note(result.region_id, "the escaping value depends on this region acquisition"));
    restore_flow(state);
  }
  Value finish_scope(size_t holding_begin, size_t native_scope, Value result, const Location &at,
                     bool parent_module, size_t parent_scope) {
    bool carry_callable = false;
    if (native_callable(result) && result.storage && result.storage->scope &&
        !scopes_.at(*result.storage->scope) && !result.pointer_view) {
      result.native = ir_.take_callable(result.storage->address);
      result.storage.reset();
      result.callable_address = 0;
      result.callable_temporary = true;
      carry_callable = true;
    }
    valid_view(result, at);
    const bool owns = result.type.kind == Type::region && result.type.owned;
    if (owns) result = take_owner(result, at);
    close_holdings(holding_begin, result, at);
    if (result.type.kind == Type::procedure) valid_view(result, at);
    ir_.restore(native_scope);
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, parent_scope);
    const auto cleanup_scope = llvm::SaveAndRestore(ir_.cleanup_owner(), parent_scope);
    if (carry_callable) {
      result.storage = StorageView{ir_.hold_callable(result.native, parent_module), parent_scope, {}};
      register_callable_holder(result);
    }
    if (owns) result = hold_region(std::move(result), parent_module, true);
    return result;
  }
  Value detach_callable(Value value, const Location &at) {
    value = read_value(std::move(value), at);
    value.native = value.callable_temporary && value.storage
        ? ir_.take_callable(value.storage->address) : ir_.copy_callable(value.native);
    value.storage.reset(); value.pointer.reset(); value.pointer_view = false;
    value.callable_address = 0;
    value.callable_temporary = true;
    return value;
  }
  Value prepare_callable_argument(Value value, const std::optional<StorageView> &destination) {
    // value.native is the already evaluated argument, not a reload of its
    // potentially replaced holder. Fresh unaliased images can transfer directly.
    value.native = value.callable_temporary && value.storage
        ? ir_.take_callable(value.storage->address) : ir_.copy_callable(value.native);
    if (destination) {
      value.storage = destination;
      ir_.write_pointer(value.storage->address, value.native);
    } else value.storage = StorageView{ir_.hold_callable(value.native, module_top_level_), local_scope_, {}};
    value.pointer.reset();
    value.pointer_view = false;
    value.callable_address = 0;
    value.callable_temporary = true;
    return value;
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
  bool region_definition(Type type) const {
    return type.kind == Type::regional && products_.at(type.identity).form->kind == Form::Kind::region_definition;
  }
  bool subtype(Type actual, Type expected, const Module &visible, const Location &at) {
    if (nominal_subtype(actual, expected, at)) return true;
    if (region_definition(actual) && region_definition(expected)) {
      resolve(actual, visible, at);
      resolve(expected, visible, at);
      const auto &available = products_.at(actual.identity).fields;
      const auto &required = products_.at(expected.identity).fields;
      const auto kind = [](const Parameter &field) {
        return field.declaration_kind.empty() ? std::string_view("val") : std::string_view(field.declaration_kind);
      };
      return std::all_of(required.begin(), required.end(), [&](const Parameter &field) {
        return std::any_of(available.begin(), available.end(), [&](const Parameter &candidate) {
          return candidate.name == field.name && candidate.type == field.type && kind(candidate) == kind(field) &&
                 candidate.read_only == field.read_only && candidate.fixed_binding == field.fixed_binding;
        });
      });
    }
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
  bool establishes(const Value &source, Conformance required, const Location &at) {
    if (possible_result_facts_ && !source.result_fact_origins.empty()) return true;
    const auto matches = [&](const Conformance &relation) {
      return relation.data == required.data && nominal_subtype(relation.interface, required.interface, at);
    };
    if (std::any_of(source.conformances.begin(), source.conformances.end(), matches)) return true;
    auto value = source;
    resolve_result_facts(value, at);
    return std::any_of(value.conformances.begin(), value.conformances.end(), matches);
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
      if (actual.type.kind == Type::procedure && expected.kind == Type::procedure &&
          actual.type.capture_regions != expected.capture_regions) {
        message = "callable environment owner mismatch: " + message;
        if (expected.capture_owners.empty())
          message += "\n" + location_text(at) + ": note: declare retained regions in the callable type, for example func()->const u64 with {region r}";
        for (const auto id : actual.type.capture_regions)
          message += region_note(id, "the returned or supplied callable retains this region owner");
        for (const auto id : expected.capture_regions)
          message += region_note(id, "the callable signature requires this actual region owner");
      }
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
      if (declares_npe(type.throws_types, module, at)) type.throws_types = {exception_type(ExceptionCategory::null_pointer)};
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
          product.fields.back().read_only = field.read_only;
          product.fields.back().fixed_binding = field.fixed_binding;
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
  llvm::Value *stored_value(const Value &value, Type destination, const Location &at, bool read_only = false) {
    if (local_bits(destination)) require_argument_reads(value);
    if (destination.kind == Type::reference && !read_only)
      require_writable(value.read_only, at, "cannot store a read-only view in a writable reference field");
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
    require_argument_reads(value);
    value.argument_reads.clear();
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
                                      const Module &caller, const Location &at, bool required = false,
                                      const Locals *captures = nullptr, bool check_access = true) {
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
        for (const auto &arg : args) {
          merge_conformances(value.conformances, arg.conformances);
          merge_result_origins(value, arg);
        }
        if (!accepts(value, resolve(*parameter.bound, lexical, parameter.at), caller, at))
          return reject("type argument does not satisfy bound for '" + parameter.name + "'");
      }
    }
    Signature result{type_context_.bindings, {}, {}, {}};
    if (captures)
      for (const auto &[name, value] : captures->owners()) result.owner_values.emplace(name, borrowed(value));
    const auto bind_owner = [&](const std::string &name, Value value) -> std::optional<std::string> {
      const auto declaration = std::find_if(form.owner_parameters.begin(), form.owner_parameters.end(),
          [&](const auto &owner) { return owner.name == name; });
      if (declaration == form.owner_parameters.end()) return "undeclared owner '" + name + "'";
      const auto actual = storage_owner(value);
      if (!actual) return "owner '" + name + "' requires storage with known provenance";
      if (declaration->root) {
        if (!value.region_id) return "region owner '" + name + "' cannot bind lexical storage";
        if (!possible_result_facts_) resolve_region_root(*value.region_id, at);
        const auto &metadata = regions_.at(*value.region_id);
        if (!metadata.root || !accepts(contextual(scalar(*metadata.root, nullptr), caller),
            resolve(*declaration->root, lexical, declaration->at), caller, at))
          return "region owner '" + name + "' does not satisfy its root constraint";
        Type type{Type::region}; type.target = std::make_shared<Type>(*metadata.root);
        auto region = scalar(type, metadata.owner, value.region_id);
        region.read_only = value.read_only; region.conformances = value.conformances; region.result_fact_origins = value.result_fact_origins;
        value = std::move(region);
      }
      const auto [found, inserted] = result.owner_values.emplace(name, borrowed(value));
      if (!inserted) {
        if (storage_owner(found->second) != actual) return "owner '" + name + "' requires the same actual owner";
        found->second.read_only = ReadOnly::both(found->second.read_only, value.read_only);
      }
      return {};
    };
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &parameter = parameter_at(form, i);
      if (check_access && form.operation && ((form.name == "=" && i == 0) || swap_operation(form.name)) && args[i].read_only != parameter.read_only)
        return reject("reference-assignment destination requires an exact const qualifier");
      if (check_access && !parameter.read_only &&
          (object_type(args[i].type) || args[i].type.kind == Type::region || !parameter.owner.empty()) && args[i].read_only)
        return reject("cannot pass a read-only view to a writable parameter");
      if (!parameter.owner.empty()) {
        auto view = args[i];
        view.read_only |= parameter.read_only;
        if (const auto error = bind_owner(parameter.owner, std::move(view))) return reject(*error);
      }
    }
    // Infer a single unbound environment owner from a callable argument. Multiple
    // unknown roles need explicit owner arguments; never assign roles by slot order.
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &pattern = parameter_at(form, i).type;
      if (pattern.kind != Type::procedure || pattern.capture_owners.empty()) continue;
      std::set<size_t> remaining(args[i].type.capture_regions.begin(), args[i].type.capture_regions.end());
      std::vector<std::string> unknown;
      for (const auto &name : pattern.capture_owners) {
        if (const auto found = result.owner_values.find(name); found != result.owner_values.end()) {
          if (!found->second.region_id) return reject("callable environment owner '" + name + "' must be regional");
          remaining.erase(*found->second.region_id);
        } else unknown.push_back(name);
      }
      if (!unknown.empty()) {
        if (unknown.size() != 1 || remaining.size() != 1)
          return reject("cannot uniquely infer callable environment owners; supply explicit region arguments");
        const auto owner = callable_owner(args[i], *remaining.begin(), at);
        if (const auto error = bind_owner(unknown.front(), owner)) return reject(*error);
      }
    }
    const auto check_names = [&](auto &&self, const Type &type) -> std::optional<std::string> {
      if (type.kind != Type::procedure) return {};
      for (const auto &name : type.capture_owners)
        if (!result.owner_values.contains(name)) return "callable environment owner '" + name + "' has no argument or captured region anchor";
      for (const auto &parameter : type.parameters)
        if (auto error = self(self, parameter)) return error;
      return type.target ? self(self, *type.target) : std::nullopt;
    };
    for (const auto &parameter : form.parameters)
      if (auto error = check_names(check_names, parameter.type)) return reject(*error);
    if (form.result_type)
      if (auto error = check_names(check_names, *form.result_type)) return reject(*error);
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &parameter = parameter_at(form, i);
      result.parameters.push_back(bind_capture_type(resolve(parameter.type, lexical, parameter.at), result.owner_values, parameter.at));
    }
    if (form.result_type) result.result = bind_capture_type(resolve(*form.result_type, lexical, form.at), result.owner_values, form.at);
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
        for (size_t category = 0; category < exception_categories.size(); ++category) {
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
      } else if (form.kind == Form::Kind::data || form.kind == Form::Kind::region_definition) {
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
  static void require_symbol_name(const Expression &expr, bool lexical_binding) {
    if (lexical_binding)
      fail(expr.at, "symbol access @" + expr.text + " resolves to non-symbol lexical binding '" + expr.text + "'");
  }
  size_t expression_symbol(const Expression &expr, const Module &module, const Locals &locals) {
    require_symbol_name(expr, locals.visible.contains(expr.text));
    return expression_symbol(expr, module);
  }
  static bool uses_symbol(Expression::Kind kind) {
    using K = Expression::Kind;
    return kind == K::symbol || kind == K::effect || kind == K::perform || kind == K::behave || kind == K::touch ||
           kind == K::procedure_effect;
  }
  static std::unique_ptr<Form> procedure_header(const Form &source) {
    auto form = std::make_unique<Form>();
    form->kind = source.kind;
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
    form->captures = source.captures;
    form->explicit_captures = source.explicit_captures;
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
    expr->read_only = source_expr.read_only;
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
      if (expr->kind == Expression::Kind::procedure_effect) expr->definition->name = expr->text;
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
    type.pure_callable = form.kind == Form::Kind::function;
    const auto context = llvm::SaveAndRestore(type_context_, TypeContext{&module, {}});
    for (size_t i = 0; i < form.type_parameters.size(); ++i)
      type_context_.bindings.emplace(form.type_parameters[i].name, Type{Type::parameter, {}, i});
    for (const auto &parameter : form.type_parameters)
      type.bounds.push_back(parameter.bound ? resolve(*parameter.bound, module, parameter.at) : Type::void_type);
    if (declares_npe(form.throws_types, module, form.at)) type.throws_types = {exception_type(ExceptionCategory::null_pointer)};
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
    const auto capture_parameters = [&](auto &&self, Type &value) -> void {
      if (value.kind != Type::procedure) return;
      value.capture_formals.resize(value.capture_owners.size());
      for (size_t i = 0; i < value.capture_owners.size(); ++i) {
        const auto &name = value.capture_owners[i];
        if (std::any_of(form.owner_parameters.begin(), form.owner_parameters.end(),
            [&](const auto &owner) { return owner.name == name; }))
          value.capture_formals[i] = owner_index(name);
      }
      if (std::none_of(value.capture_formals.begin(), value.capture_formals.end(), [](size_t id) { return id; }))
        value.capture_formals.clear();
      for (auto &parameter : value.parameters) self(self, parameter);
      if (value.target) {
        value.target = std::make_shared<Type>(*value.target);
        self(self, *value.target);
      }
    };
    for (auto &parameter : type.parameters) capture_parameters(capture_parameters, parameter);
    capture_parameters(capture_parameters, *type.target);
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
      const auto &name = parameter.owner.empty() && parameter.type.capture_owners.size() == 1
          ? parameter.type.capture_owners.front() : parameter.owner;
      for (const auto &owner : method.form->owner_parameters)
        if (owner.name == name && owner.root)
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
      if (!inserted)
        require_same_region(found->second.first, *value->region_id, at,
            "shared-region constraint violated by " + symbol_names_[found->second.second] + " and " + symbol_names_[id] +
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
    if (found->capture_use) found->capture_use->require();
    if (env.replay && env.captured(id, found)) {
      for (auto replay = env.replay; replay; replay = replay->defining_replay) {
        const auto origin = replay->captured_values.lookup(id);
        if (!origin || *origin != found) break;
        replay->used.insert(id);
      }
    }
    if (found->region_id) region_pointer(*found, at);
    if (found->type.kind == Type::region) return *found;
    if (found->type.kind == Type::procedure) {
      auto value = *found;
      if (native_callable(value)) {
        if (value.replayed) value.native = ir_.read({value.type, nullptr, ir_.read_symbol(id, value.type)});
        else if (value.storage && !env.captured(id, found)) {
          value.storage->address = ir_.read_symbol(id, value.type);
          value.pointer.reset();
        }
      }
      return value;
    }
    if (!materialize) return *found;
    const auto type = found->type;
    auto result = scalar(type, env.captured(id, found) && !found->replayed ? found->native : ir_.read_symbol(id, type), found->region_id);
    result.conformances = found->conformances; result.result_fact_origins = found->result_fact_origins;
    result.exception_category = found->exception_category;
    result.exception_facts = found->exception_facts;
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
      if (!value.type.capture_regions.empty())
        fail(at, "symbolic procedure binding cannot erase captured region owners; use an owner-anchored lexical callable");
      auto actual = value.type;
      auto declared = symbol_types_.at(id);
      const bool permitted = actual.throws_types.empty() || !declared.throws_types.empty();
      actual.throws_types.clear(); declared.throws_types.clear();
      if (actual != declared) fix_type(id, value.type, at);
      if (!permitted) fail(at, "procedure checked NullPointerException exceeds symbolic declaration");
      value.type = symbol_types_.at(id);
    } else fix_type(id, value.type, at);
    ir_.effect(id, value.type, value.type.kind == Type::procedure
        ? (native_callable(value) && value.storage ? value.storage->address : nullptr) : value.native);
    if (value.type.kind == Type::procedure) value.pointer.reset();
    value.replayed = false;
    value.owner = &owner;
    value.callable_temporary = false;
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
  struct LexicalReferences {
    std::map<std::string, Location> required;
    std::map<std::string, Value> owners;
    using Names = std::set<std::string>;
    Names bound;
    std::vector<const Expression *> touches;
  };
  LexicalReferences lexical_references(const Expression &body, const Locals &outer, const Form *signature = nullptr) {
    LexicalReferences result;
    auto &required = result.required;
    auto &bound = result.bound;
    const auto owners = outer.owners();
    const auto type_owners = [&](auto &&self, const Type &type, const LexicalReferences::Names &bound) -> void {
      for (const auto &name : type.capture_owners)
        if (!bound.contains(name))
          if (const auto found = owners.find(name); found != owners.end()) {
            valid_view(found->second, body.at);
            result.owners.emplace(name, borrowed(found->second));
          }
      if (type.target) self(self, *type.target, bound);
      for (const auto &parameter : type.parameters) self(self, parameter, bound);
    };
    const auto use = [&](const std::string &name, const Location &at, const LexicalReferences::Names &bound) {
      if (!bound.contains(name) && outer.visible.contains(name)) required.try_emplace(name, at);
    };
    unsigned nested_callables = 0;
    const auto scan = [&](auto &&self, const Expression &expr, LexicalReferences::Names &bound) -> void {
      using K = Expression::Kind;
      if (uses_symbol(expr.kind))
        require_symbol_name(expr, bound.contains(expr.text) || outer.visible.contains(expr.text));
      if (expr.kind == K::touch && !nested_callables) result.touches.push_back(&expr);
      if (expr.kind == K::name || expr.kind == K::call) use(expr.text, expr.at, bound);
      if (!expr.owner_name.empty()) use(expr.owner_name, expr.at, bound);
      if (expr.annotation) type_owners(type_owners, *expr.annotation, bound);
      if (expr.kind == K::block) {
        auto inner = bound;
        for (const auto &child : expr.children) self(self, *child, expr.scoped ? inner : bound);
        return;
      }
      if (expr.kind == K::pointer_group) {
        for (const auto &child : expr.children) {
          self(self, *child, bound);
          if (child->kind == K::pointer) bound.insert(child->text);
        }
        return;
      }
      if (expr.kind == K::range_for || expr.kind == K::fold) {
        const auto body = expr.kind == K::fold ? 3u : 2u;
        for (size_t i = 0; i < body; ++i) self(self, *expr.children[i], bound);
        auto inner = bound;
        inner.insert(expr.text);
        if (expr.kind == K::fold) inner.insert(expr.binding);
        self(self, *expr.children[body], inner);
        if (expr.children.size() > body + 1) self(self, *expr.children.back(), bound);
        return;
      }
      if (expr.definition) {
        // Constructing a nested callable also requires its declared captures,
        // including entries which its own body does not use.
        for (const auto &capture : expr.definition->captures) use(capture.name, capture.at, bound);
        auto inner = bound;
        for (const auto &parameter : expr.definition->parameters) inner.insert(parameter.name);
        for (const auto &owner : expr.definition->owner_parameters)
          inner.insert(owner.name);
        for (const auto &parameter : expr.definition->parameters) type_owners(type_owners, parameter.type, inner);
        if (expr.definition->result_type) type_owners(type_owners, *expr.definition->result_type, inner);
        {
          const auto nested = llvm::SaveAndRestore(nested_callables, nested_callables + 1);
          self(self, *expr.definition->body, inner);
        }
        if (expr.kind == K::callable && !expr.definition->name.empty()) bound.insert(expr.definition->name);
        return;
      }
      if (expr.kind == K::handler) {
        auto inner = bound;
        inner.insert(expr.text);
        for (const auto &child : expr.children) self(self, *child, inner);
        return;
      }
      for (const auto &child : expr.children) self(self, *child, bound);
      if (expr.kind == K::region_binding || expr.kind == K::alias) bound.insert(expr.text);
    };
    if (signature) {
      for (const auto &parameter : signature->parameters) bound.insert(parameter.name);
      for (const auto &owner : signature->owner_parameters)
        bound.insert(owner.name);
      for (const auto &parameter : signature->parameters) type_owners(type_owners, parameter.type, bound);
      if (signature->result_type) type_owners(type_owners, *signature->result_type, bound);
    }
    scan(scan, body, bound);
    return result;
  }
  Locals capture_variables(const Form &form, const Locals &outer) {
    auto references = lexical_references(*form.body, outer, &form);
    const auto &required = references.required;
    const auto &bound = references.bound;
    Locals captures;
    captures.owner_anchors = std::move(references.owners);
    std::map<std::string, Location> selected;
    if (!form.explicit_captures) selected = required;
    else {
      for (const auto &capture : form.captures) {
        if (bound.contains(capture.name)) fail(capture.at, "a capture name must be distinct from parameters and owner declarations");
        if (!selected.emplace(capture.name, capture.at).second) fail(capture.at, "duplicate capture '" + capture.name + "'");
        if (!outer.visible.contains(capture.name)) fail(capture.at, "unavailable captured variable '" + capture.name + "'");
      }
      for (const auto &[name, at] : required)
        if (!selected.contains(name)) fail(at, "explicit capture list is missing '" + name + "'");
    }
    for (const auto &[name, at] : selected) {
      auto value = outer.visible.at(name);
      require_runtime_value(value, at);
      if (deferred_loop_controls_.contains(value.loop_control))
        fail(at, "a delayed defer block cannot capture a loop-control binding; prepare a deferred call argument instead");
      require_declaration(value, at);
      value.declaration_cell.reset();
      const bool slot = outer.regional_slots.contains(name);
      if (value.type.kind == Type::region) value = borrowed(std::move(value));
      else if (value.type.kind != Type::procedure && !slot)
        fail(at, "a closure cannot capture lexical variable storage; declare a var alias to regional storage");
      valid_view(value, at);
      if (native_callable(value)) {
        value = read_value(std::move(value), at);
        std::set<NativeCallDemand *> resolving;
        resolve_native_flow(value.callable_flow, resolving);
        value.pointer.reset(); value.pointer_view = false;
        value.callable_address = 0;
      }
      captures.declare(name, std::move(value), outer.locations.at(name));
      if (slot) captures.regional_slots.insert(name);
      if (form.explicit_captures && !required.contains(name))
        capture_warnings_.emplace_back(at, "capture '" + name + "' is unused by the body but retains its owner dependency");
    }
    return captures;
  }
  Value captured_view(Value value, const Location &at) {
    valid_view(value, at);
    if (value.reference_field) {
      const auto offset = ir_.read(*value.reference_field);
      value.native = offset;
      if (local_bits(value.type)) {
        value.storage = StorageView{ir_.nullable_address(value.reference_field->owner, offset), {}, {}};
        value.native = nullptr;
      }
    }
    return value;
  }
  static void callable_fields(Procedure &procedure, const std::function<void(llvm::Value *&, bool)> &visit) {
    const auto value_fields = [&](Value &value) {
      if (value.procedure && !value.procedure->code) {
        // An unmaterialized generic function has a statically selected body.
        // Its captured fields still belong to the complete enclosing value.
        auto captured = std::make_shared<Procedure>(*value.procedure);
        callable_fields(*captured, visit);
        value.procedure = std::move(captured);
        return;
      }
      const auto field = [&](llvm::Value *&native, bool nested = false) { if (native) visit(native, nested); };
      field(value.native, native_callable(value));
      if (value.type.kind == Type::procedure) return;
      field(value.exception_category);
      if (value.storage) field(value.storage->address);
      if (value.reference_field) {
        field(value.reference_field->owner); field(value.reference_field->address);
      }
    };
    for (auto &[name, value] : procedure.lexical_captures.visible) value_fields(value);
    for (auto &[name, value] : procedure.lexical_captures.owner_anchors) value_fields(value);
  }
  Value materialize_callable(Value value, const Environment &environment, const Location &at) {
    const auto &type = value.type;
    bool native = value.procedure->form->result_type && native_callable_type(type, false);
    if (!type.pure_callable)
      for (const auto &[id, binding] : value.procedure->captures.bindings)
        native &= local_bits(binding->type) || object_type(binding->type) || binding->type.kind == Type::region ||
            (native_callable(*binding) && !binding->nullable) || (binding->procedure && !binding->procedure->builtin.empty());
    for (const auto &[name, capture] : value.procedure->lexical_captures.visible)
      if (capture.procedure && !capture.procedure->code && !capture.type.pure_callable) native = false;
    auto procedure = std::make_shared<Procedure>(*value.procedure);
    for (auto &[name, capture] : procedure->lexical_captures.visible)
      if (native_callable(capture)) {
        if (!native) {
          capture.native = ir_.copy_callable(capture.native);
          capture.storage = StorageView{ir_.hold_callable(capture.native, module_top_level_), local_scope_, {}};
        } else capture.storage.reset();
      }
    value.procedure = procedure;
    if (!native) return value;
    std::vector<llvm::Value *> fields;
    std::set<size_t> nested;
    callable_fields(*procedure, [&](llvm::Value *&field, bool child) {
      if (child) nested.insert(fields.size());
      fields.push_back(field);
    });
    std::vector<size_t> dynamic_values;
    std::set<size_t> dynamic_regions;
    if (!type.pure_callable)
      for (const auto &[id, binding] : procedure->captures.bindings)
        if (local_bits(binding->type) || object_type(binding->type) || binding->type.kind == Type::region) {
          dynamic_values.push_back(id);
          fields.push_back(stored_value(*binding, binding->type, at));
          if (binding->region_id) dynamic_regions.insert(*binding->region_id);
        }
    std::vector<size_t> dynamic_procedures;
    std::vector<std::pair<size_t, std::shared_ptr<CaptureUse>>> capture_fields;
    if (!type.pure_callable) {
      const auto captures = procedure->captures.bindings;
      for (const auto &[id, binding] : captures)
        if (native_callable(*binding)) {
          auto captured = *binding;
          if (captured.replayed) captured.native = ir_.read({captured.type, nullptr, ir_.read_symbol(id, captured.type)});
          else if (captured.storage && !environment.captured(id, binding))
            captured.storage->address = ir_.read_symbol(id, captured.type);
          captured = read_value(std::move(captured), at);
          captured.storage.reset(); captured.pointer.reset(); captured.pointer_view = false;
          captured.callable_address = 0;
          captured.callable_temporary = false;
          captured.replayed = false;
          captured.capture_use = std::make_shared<CaptureUse>(CaptureUse{false, binding->capture_use});
          capture_fields.emplace_back(fields.size(), captured.capture_use);
          dynamic_procedures.push_back(id);
          nested.insert(fields.size());
          fields.push_back(captured.native);
          binding_values_.push_back(std::move(captured));
          procedure->captures.bindings = binding_factory_.add(procedure->captures.bindings, id, &binding_values_.back());
          procedure->captures.captured_bindings = binding_factory_.add(
              procedure->captures.captured_bindings, id, &binding_values_.back());
        }
    }
    std::vector<size_t> regions;
    for (const auto &[id, owner] : closure_regions(value)) {
      regions.push_back(id);
      fields.push_back(region_pointer(owner, at));
    }
    for (const auto id : dynamic_regions) fields.push_back(regions_.at(id).owner);
    LLVMEmission::NativeDefinition entry;
    if (!contextual_callable(type)) entry = ir_.native_declaration("callback", type.parameters, *type.target);
    entry.checked_null = !type.throws_types.empty();
    auto *layout = ir_.callable_layout(fields, nested);
    auto *key = contextual_callable(type) ? ir_.callable_key() : entry.function;
    auto construction = dynamic_procedures.empty() ? nullptr : std::make_shared<LLVMEmission::CallableConstruction>();
    value.native = ir_.create_callable(entry.function, layout, fields, nested, key, construction.get());
    procedure->code = std::make_shared<CallableCode>(CallableCode{
        std::move(entry), layout, std::move(nested), std::move(regions), std::move(dynamic_values), key,
        std::move(dynamic_procedures), std::move(capture_fields), std::move(construction),
        {dynamic_regions.begin(), dynamic_regions.end()}});
    if (type.target->kind == Type::procedure && !borrowed_result(type)) {
      callable_flows_.push_back({{}, {}, true});
      procedure->code->result_flow = callable_flows_.size() - 1;
    }
    value.procedure = std::move(procedure);
    callable_flows_.push_back({value.procedure, {}});
    value.callable_flow = callable_flows_.size() - 1;
    value.storage = StorageView{ir_.hold_callable(value.native, module_top_level_), local_scope_, {}};
    register_callable_holder(value);
    value.callable_temporary = true;
    return value;
  }
  Value define_procedure(Form &form, const Module &lexical, const Environment &env, const Locals &locals = {},
                         ImplementationContext implementation = {}, std::vector<CallableDeclaration> declarations = {}) {
    resolve_procedure_symbols(*form.body, lexical);
    auto proc = std::make_shared<Procedure>();
    proc->form = &form;
    proc->lexical = &lexical;
    proc->implementation = std::move(implementation);
    proc->declarations = std::move(declarations);
    proc->lexical_captures = capture_variables(form, locals);
    // Snapshot dynamic payloads at definition time. Native materialization embeds
    // the procedural snapshots demanded by checked reads or explicit touches.
    if (form.kind == Form::Kind::procedure) {
      proc->captures = env;
      for (const auto &[id, value] : env.bindings) {
        if ((!local_bits(value->type) && !object_type(value->type)) ||
            (env.captured(id, value) && !value->replayed)) continue;
        auto snapshot = *value;
        snapshot.native = ir_.read_symbol(id, value->type);
        snapshot.replayed = false;
        binding_values_.push_back(std::move(snapshot));
        proc->captures.bindings = binding_factory_.add(proc->captures.bindings, id, &binding_values_.back());
      }
      proc->captures.captured_context = true;
      proc->captures.captured_bindings = proc->captures.bindings;
    }
    Value result{procedure_type(form, lexical), {}, std::move(proc), &lexical};
    result.type = bind_capture_type(result.type, result.procedure->lexical_captures.owners(), form.at, true);
    for (const auto &[id, access] : closure_regions(result)) result.type.capture_regions.push_back(id);
    return materialize_callable(std::move(result), env, form.at);
  }
  Value sequence(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                 bool function_context, bool discard_result, std::optional<Type> result_type, bool copy_result,
                 bool read_result, const Value *null_owner) {
    active_environments_.emplace_back(local_scope_, &env);
    const auto environment_frame = llvm::scope_exit([&] { active_environments_.pop_back(); });
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
                            read_result || i + 1 != expr.children.size(),
                            i + 1 == expr.children.size() && result_type && result_type->kind == Type::procedure
                                ? result_type : std::nullopt,
                            i + 1 == expr.children.size() ? null_owner : nullptr);
      }
    } catch (const EndPath &) { result = scalar(Type::never, nullptr); }
    if (discard_result && result.type != Type::never) result = {};
    if (result.type == Type::null_ref && null_owner && result_type)
      result = null_target(*result_type, *null_owner, expr.at);
    if (result_type && result.type != Type::never) result = admit(std::move(result), *result_type, module, expr.at);
    if (copy_result && local_bits(result.type)) result = bits_value(read_value(std::move(result), expr.at));
    if (copy_result && native_callable(result)) {
      result = detach_callable(std::move(result), expr.at);
      result.storage = StorageView{ir_.hold_callable(result.native, module_top_level_), local_scope_, {}};
      register_callable_holder(result);
    }
    return result;
  }
  Value block(const Expression &expr, const Module &module, Environment &env, Locals &locals,
              bool function_context, bool discard_result = false, std::optional<Type> result_type = {}, bool copy_result = false,
              bool read_result = true,
              const Value *null_owner = nullptr, bool iteration = false) {
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    if (!expr.scoped && !iteration)
      return sequence(expr, module, env, locals, function_context, discard_result, result_type, copy_result,
                      read_result, null_owner);
    const bool parent_module = module_top_level_;
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
    const auto holding_begin = holdings_.size();
    const auto native_scope = ir_.scope();
    const auto scope_id = scopes_.size();
    scopes_.push_back(true);
    const auto parent_scope = local_scope_;
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);
    const auto cleanup_scope = llvm::SaveAndRestore(ir_.cleanup_owner(), scope_id);
    auto inner_env = env;
    auto inner_locals = locals.nested();
    const auto body = [&] { return sequence(expr, module, inner_env, inner_locals, function_context,
        discard_result, result_type, copy_result, read_result, null_owner); };
    auto result = expr.scoped ? deferred_scope(expr, module, inner_env, inner_locals, function_context, body) : body();
    scopes_[scope_id] = false;
    result = finish_scope(holding_begin, native_scope, std::move(result), expr.at, parent_module, parent_scope);
    result.pointer.reset();
    if (result.arguments) fail(expr.at, "a variadic argument pack cannot escape its call scope");
    return result;
  }
  std::vector<Value> arguments(const Expression &expr, const Module &module, Environment &env,
                               Locals &locals, bool function_context,
                               std::vector<ConversionPoint> *points = nullptr,
                               const std::vector<Method> &nullable_candidates = {},
                               const Type *native_type = nullptr) {
    const auto region_initializer = llvm::SaveAndRestore(region_initializer_, false);
    std::vector<Value> values;
    auto append = [&](Value value, bool forward) {
      try { resolve_region_type(value, expr.at); }
      catch (const UnresolvedAccess &) { /* Layout-dependent dispatch is checked at the call boundary. */ }
      // Reserve cleanup before lowering later arguments. A selected by-value
      // parameter constructs its snapshot here, even when a later argument
      // replaces the source holder or exits exceptionally.
      std::optional<StorageView> destination;
      bool local = false, borrowed = false;
      if (native_type && (values.size() < native_type->parameters.size() || native_type->variadic)) {
        borrowed = borrowed_parameter(*native_type, values.size());
        local = !borrowed;
      }
      for (const auto &method : nullable_candidates)
        if (!method.form->parameters.empty() && (variadic(*method.form) || values.size() < method.form->parameters.size())) {
          const bool external = !parameter_at(*method.form, values.size()).owner.empty();
          local |= !external;
          borrowed |= external;
        }
      if (points && native_callable(value) && local)
        destination = StorageView{ir_.hold_callable(ir_.null_pointer(), module_top_level_), local_scope_, {}};
      const bool prepared = destination && !borrowed;
      if (prepared) {
        value = prepare_callable_argument(std::move(value), destination);
        register_callable_holder(value);
      }
      values.push_back(std::move(value));
      if (points) {
        points->push_back(conversion_point());
        points->back().raw_view = forward;
        points->back().callable_destination = destination;
        points->back().callable_prepared = prepared;
      }
    };
    const size_t first_argument = expr.kind == Expression::Kind::value_call ? 1 : 0;
    for (size_t index = first_argument; index < expr.children.size(); ++index) {
      const auto &arg = expr.children[index];
      const bool native_forward = native_type && !native_type->nullable_views.empty() &&
          (values.size() < native_type->parameters.size() || native_type->variadic) &&
          native_type->nullable_views[std::min(values.size(), native_type->parameters.size() - 1)];
      const bool forward = native_forward || std::any_of(nullable_candidates.begin(), nullable_candidates.end(), [&](const Method &method) {
        return !method.form->parameters.empty() && (variadic(*method.form) || values.size() < method.form->parameters.size()) &&
               parameter_at(*method.form, values.size()).declaration_kind == "ptr" &&
               !parameter_at(*method.form, values.size()).owner.empty();
      });
      auto value = expression(*arg, module, env, locals, function_context, false, false, !forward);
      if (!forward && value.storage) {
        const bool initialized = std::all_of(value.storage->cells.begin(), value.storage->cells.end(),
            [&](size_t cell) { return initialization_known(cells_.at(cell).initialized) != false; });
        if (initialized) value = read_value(std::move(value), arg->at, true);
        else value.native = nullptr;
      } else if (!forward && value.type.kind != Type::reference && value.nullable) {
        value = read_value(std::move(value), arg->at);
      }
      if (arg->spread) {
        if (!value.arguments) fail(arg->at, "argument expansion requires a variadic parameter pack");
        for (const auto &element : *value.arguments)
          append(forward ? element : read_value(element, arg->at), forward);
      } else append(std::move(value), forward);
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
  static bool borrowed_parameter(const Type &type, size_t index) {
    return !type.owners.empty() && type.owners[std::min(index, type.parameters.size() - 1)] &&
        type.parameters[std::min(index, type.parameters.size() - 1)].kind != Type::region;
  }
  static bool borrowed_result(const Type &type) {
    return !type.owners.empty() && type.owners.back();
  }
  static bool native_callable_type(const Type &type, bool concrete = true) {
    const auto supported = [concrete](const Type &value) {
      return local_bits(value) || (!concrete && representation(value).kind == Type::parameter) ||
          native_callable_type(value, false);
    };
    if (type.kind != Type::procedure || !type.target ||
        !(supported(*type.target) || *type.target == Type::void_type ||
          (type.target->kind == Type::region && type.target->owned) ||
          (borrowed_result(type) && (type.target->kind == Type::array || type.target->kind == Type::regional ||
                                    (type.target->kind == Type::region && !type.target->owned)))) ||
        (concrete && (!type.bounds.empty() || type.variadic))) return false;
    if (borrowed_result(type) &&
        !local_bits(*type.target) && type.target->kind != Type::procedure && type.target->kind != Type::array &&
        type.target->kind != Type::regional && type.target->kind != Type::region &&
        (concrete || representation(*type.target).kind != Type::parameter)) return false;
    // Unqualified borrowed results still need body-derived provenance.
    if (!borrowed_result(type) && !type.owner_bounds.empty() &&
        *type.target != Type::void_type && type.target->kind != Type::procedure &&
        !(type.target->kind == Type::region && type.target->owned) &&
        (type.const_views.empty() || !type.const_views.back())) return false;
    for (size_t i = 0; i < type.parameters.size(); ++i) {
      const auto &parameter = type.parameters[i];
      if (borrowed_parameter(type, i)) {
        if (!supported(parameter) && !object_type(parameter)) return false;
      } else if (!supported(parameter) && parameter.kind != Type::region) return false;
    }
    return true;
  }
  static bool contextual_callable(const Type &type) {
    return !type.pure_callable || !type.bounds.empty() || !type.owner_bounds.empty() || type.variadic ||
        (type.target && type.target->kind == Type::region && type.target->owned) ||
        std::any_of(type.parameters.begin(), type.parameters.end(),
        [](const Type &parameter) { return parameter.kind == Type::procedure || parameter.kind == Type::region; });
  }
  Type instantiate_callable(Type type, const std::vector<Value> &args, const Module &caller, const Location &at) {
    const size_t fixed = type.parameters.size() - size_t(type.variadic);
    if (args.size() < fixed || (!type.variadic && args.size() != fixed))
      fail(at, "callable argument count mismatch");
    const auto parameter = [&](size_t i) -> const Type & { return type.parameters[std::min(i, type.parameters.size() - 1)]; };
    std::vector<std::optional<Type>> bindings(type.bounds.size());
    const auto infer = [&](auto &&self, const Type &pattern, const Type &actual) -> void {
      // A supplied polymorphic value owns its binders; they are not variables
      // of this invocation. Nested signatures without binders can mention ours.
      if (pattern.kind == Type::procedure && (!pattern.bounds.empty() || !pattern.owner_bounds.empty())) return;
      if (pattern.kind == Type::parameter) {
        auto &bound = bindings.at(pattern.identity);
        if (bound && *bound != actual) fail(at, "inconsistent type-parameter arguments");
        bound = actual;
        return;
      }
      if (pattern.target && actual.target) self(self, *pattern.target, *actual.target);
      if (pattern.parameters.size() == actual.parameters.size())
        for (size_t i = 0; i < pattern.parameters.size(); ++i) self(self, pattern.parameters[i], actual.parameters[i]);
    };
    for (size_t i = 0; i < args.size(); ++i) infer(infer, parameter(i), args[i].type);
    const auto substitute = [&](auto &&self, Type value) -> Type {
      if (value.kind == Type::procedure && (!value.bounds.empty() || !value.owner_bounds.empty())) return value;
      if (value.kind == Type::parameter) {
        if (!bindings.at(value.identity)) fail(at, "cannot infer callable type parameter " + std::to_string(value.identity + 1));
        return *bindings[value.identity];
      }
      if (value.target) value.target = std::make_shared<Type>(self(self, *value.target));
      for (auto &parameter : value.parameters) parameter = self(self, parameter);
      return value;
    };
    for (size_t i = 0; i < bindings.size(); ++i) {
      if (!bindings[i]) fail(at, "cannot infer callable type parameter " + std::to_string(i + 1));
      if (type.bounds[i] == Type::void_type) continue;
      auto value = scalar(*bindings[i], nullptr);
      for (const auto &arg : args) {
        merge_conformances(value.conformances, arg.conformances);
        merge_result_origins(value, arg);
      }
      if (!accepts(value, substitute(substitute, type.bounds[i]), caller, at))
        fail(at, "type argument does not satisfy callable bound");
    }
    std::vector<Type> parameters;
    std::vector<bool> qualifiers, nullable;
    std::vector<size_t> owners;
    for (size_t i = 0; i < args.size(); ++i) {
      parameters.push_back(substitute(substitute, parameter(i)));
      const auto formal = std::min(i, type.parameters.size() - 1);
      if (!type.const_views.empty()) qualifiers.push_back(type.const_views[formal]);
      if (!type.nullable_views.empty()) nullable.push_back(type.nullable_views[formal]);
      if (!type.owners.empty()) owners.push_back(type.owners[formal]);
    }
    if (!type.const_views.empty()) qualifiers.push_back(type.const_views.back());
    if (!type.nullable_views.empty()) nullable.push_back(type.nullable_views.back());
    if (!type.owners.empty()) owners.push_back(type.owners.back());
    type.parameters = std::move(parameters);
    type.const_views = std::move(qualifiers);
    type.nullable_views = std::move(nullable);
    type.owners = std::move(owners);
    for (auto &bound : type.owner_bounds) bound = substitute(substitute, bound);
    type.target = std::make_shared<Type>(substitute(substitute, *type.target));
    type.bounds.clear(); type.variadic = false;
    std::map<size_t, std::pair<bool, size_t>> owner_values;
    const auto bind_owner = [&](size_t owner, const Value &value) {
      valid_view(value, at);
      const auto actual = storage_owner(value);
      if (!actual) fail(at, "callable owner parameter requires storage with known provenance");
      if (owner_values.emplace(owner, *actual).first->second != *actual)
        fail(at, "callable owner parameter requires the same actual owner");
      const auto &bound = type.owner_bounds.at(owner - 1);
      if (bound == Type::void_type) return;
      if (!value.region_id) fail(at, "region owner parameter cannot bind lexical storage");
      const auto &region = regions_.at(*value.region_id);
      if (!region.root || !accepts(contextual(scalar(*region.root, nullptr), caller), bound, caller, at))
        fail(at, "region owner parameter does not satisfy its root constraint");
    };
    for (size_t i = 0; i < args.size() && !type.owners.empty(); ++i) {
      const auto owner = type.owners[i];
      if (!owner) continue;
      bind_owner(owner, args[i]);
      if (type.const_views.empty() || !type.const_views[i])
        require_writable(args[i].read_only, at, "cannot pass a read-only view to a writable parameter");
    }
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &pattern = type.parameters[i];
      if (pattern.capture_formals.empty()) continue;
      std::set<size_t> remaining(args[i].type.capture_regions.begin(), args[i].type.capture_regions.end());
      for (const auto id : pattern.capture_regions) remaining.erase(id);
      std::set<size_t> unknown;
      for (const auto formal : pattern.capture_formals) {
        if (!formal) continue;
        if (const auto found = owner_values.find(formal); found != owner_values.end()) {
          if (!found->second.first) fail(at, "callable environment owner must be regional");
          remaining.erase(found->second.second);
        } else unknown.insert(formal);
      }
      if (unknown.empty()) continue;
      if (unknown.size() != 1 || remaining.size() != 1)
        fail(at, "cannot uniquely infer callable environment owners; supply explicit region arguments");
      bind_owner(*unknown.begin(), callable_owner(args[i], *remaining.begin(), at));
    }
    const auto bind_captures = [&](auto &&self, Type &value) -> void {
      // A nested generic callable retains its own owner binder namespace.
      if (value.kind != Type::procedure || !value.bounds.empty() || !value.owner_bounds.empty()) return;
      value.capture_owner_regions.resize(value.capture_owners.size());
      for (size_t i = 0; i < value.capture_formals.size(); ++i) {
        const auto formal = value.capture_formals[i];
        if (!formal) continue;
        const auto found = owner_values.find(formal);
        if (found == owner_values.end()) fail(at, "cannot infer callable result environment owner from the arguments");
        if (!found->second.first) fail(at, "callable environment owner must be regional");
        value.capture_owner_regions[i] = found->second.second;
        value.capture_regions.push_back(found->second.second);
      }
      value.capture_formals.clear();
      std::sort(value.capture_regions.begin(), value.capture_regions.end());
      value.capture_regions.erase(std::unique(value.capture_regions.begin(), value.capture_regions.end()), value.capture_regions.end());
      for (const auto &owner : value.capture_owner_regions)
        if (!owner) fail(at, "callable environment owner has no argument or captured region anchor");
      for (auto &parameter : value.parameters) self(self, parameter);
      if (value.target) {
        value.target = std::make_shared<Type>(*value.target);
        self(self, *value.target);
      }
    };
    for (auto &parameter : type.parameters) bind_captures(bind_captures, parameter);
    bind_captures(bind_captures, *type.target);
    if (!native_callable_type(type)) fail(at, "this instantiated callable signature needs native parameter/result support");
    return type;
  }
  llvm::Value *borrowed_operand(const Value &value, const Location &at) {
    valid_view(value, at);
    if (value.type.kind == Type::region) return region_pointer(value, at);
    if (value.storage) return value.storage->address;
    return ir_.nullable_address(region_pointer(value, at), value.native);
  }
  const std::vector<Value> &checked_returns(size_t id, const Location &at) {
    auto &facts = return_facts_.at(id - 1);
    if (!facts.producer) return facts.alternatives;
    auto &demand = *facts.producer;
    if (active_native_call(demand)) throw UnresolvedAccess(at);
    std::set<NativeCallDemand *> resolving;
    resolve_native_flow(demand.flow, resolving);
    compile_native_call(demand);
    if (demand.reused) return checked_returns(demand.reused->record_result, at);
    const auto choices = callable_choices(demand.flow);
    if (!choices.closed || demand.selection || std::any_of(choices.definitions.begin(), choices.definitions.end(),
        [&](const auto &source) { return !demand.emitted.contains(source.get()); })) throw UnresolvedAccess(at);
    return facts.alternatives;
  }
  std::optional<Type> result_layout(size_t origin, const Location &at, bool complete) {
    std::set<size_t> visited;
    std::optional<Type> concrete;
    const auto visit = [&](auto &&self, size_t id) -> void {
      if (!visited.insert(id).second) return;
      const auto &facts = return_facts_.at(id - 1);
      if (!complete && facts.producer && facts.producer->reused) {
        self(self, facts.producer->reused->record_result);
        return;
      }
      // Partial evidence comes only from already checked bodies. Every use of
      // it retains its origin for validation against the closed return graph.
      const auto &alternatives = complete ? checked_returns(id, at) : facts.alternatives;
      for (const auto &result : alternatives) {
        if (result.result_type_origin) self(self, result.result_type_origin);
        else {
          const auto actual = result.type.kind == Type::region && result.type.target ? *result.type.target : result.type;
          if (!concrete) concrete = actual;
          else if (*concrete != actual)
            fail(at, "native regional results with different concrete types need further result-dispatch support");
        }
      }
    };
    visit(visit, origin);
    return concrete;
  }
  void resolve_result_type(Value &value, const Location &at, bool allow_partial = false) {
    if (!value.result_type_origin) return;
    std::optional<Type> concrete;
    bool partial = false;
    try { concrete = result_layout(value.result_type_origin, at, true); }
    catch (const UnresolvedAccess &) {
      if (!allow_partial) throw;
      concrete = result_layout(value.result_type_origin, at, false);
      partial = true;
    }
    if (!concrete) throw UnresolvedAccess(at);
    deferred_checks_.push_back({[this, origin = value.result_type_origin, concrete, at] {
      return result_layout(origin, at, true) != concrete;
    }, at, "native result layout changed after recursive context refinement"});
    if (value.type.kind == Type::region) {
      value.type.target = std::make_shared<Type>(*concrete);
      if (!partial) {
        regions_.at(*value.region_id).root = *concrete;
        regions_.at(*value.region_id).root_origin = 0;
      }
    } else value.type = *concrete;
    value.result_type_origin = 0;
  }
  void resolve_region_root(size_t id, const Location &at) {
    const auto origin = regions_.at(id).root_origin;
    if (!origin) return;
    auto value = scalar(Type{Type::region}, nullptr, id);
    value.result_type_origin = origin;
    resolve_result_type(value, at);
  }
  void resolve_region_type(Value &value, const Location &at) {
    if (value.type.kind != Type::region || !value.region_id) return;
    resolve_region_root(*value.region_id, at);
    const auto &root = regions_.at(*value.region_id).root;
    value.type.target = root ? std::make_shared<Type>(*root) : nullptr;
    value.result_type_origin = 0;
  }
  std::vector<Conformance> result_conformances(const std::vector<size_t> &origins, const Location &at) {
    std::vector<Conformance> common;
    std::set<size_t> visited;
    std::vector<Conformance> candidates;
    const auto collect = [&](auto &&self, size_t id) -> void {
      if (!visited.insert(id).second) return;
      for (const auto &result : checked_returns(id, at)) {
        for (const auto &fact : result.conformances) {
          auto interface = std::optional(fact.interface);
          while (interface) {
            append_conformance(candidates, {fact.data, *interface});
            resolve_abstract(*interface, at);
            interface = abstracts_.at(interface->identity).parent;
          }
        }
        for (size_t input : result.result_fact_origins) self(self, input);
      }
    };
    for (size_t id : origins) collect(collect, id);
    std::map<size_t, std::vector<size_t>> dependents;
    for (size_t id : visited)
      for (const auto &result : checked_returns(id, at))
        for (size_t input : result.result_fact_origins) dependents[input].push_back(id);
    for (const auto &candidate : candidates) {
      // Begin with the recursive hypothesis, then remove facts contradicted by
      // a returning exit. Shared return graphs are checked as graphs, not paths.
      std::set<size_t> rejected;
      std::vector<size_t> pending(visited.begin(), visited.end());
      while (!pending.empty()) {
        const size_t id = pending.back(); pending.pop_back();
        if (rejected.contains(id)) continue;
        const auto &results = checked_returns(id, at);
        const bool holds = std::all_of(results.begin(), results.end(), [&](const Value &result) {
          return std::any_of(result.conformances.begin(), result.conformances.end(), [&](const Conformance &fact) {
            return fact.data == candidate.data && nominal_subtype(fact.interface, candidate.interface, at);
          }) || std::any_of(result.result_fact_origins.begin(), result.result_fact_origins.end(),
              [&](size_t input) { return !rejected.contains(input); });
        });
        if (!holds) {
          rejected.insert(id);
          const auto &parents = dependents[id];
          pending.insert(pending.end(), parents.begin(), parents.end());
        }
      }
      if (std::any_of(origins.begin(), origins.end(),
          [&](size_t id) { return !rejected.contains(id); })) append_conformance(common, candidate);
    }
    return common;
  }
  void resolve_result_facts(Value &value, const Location &at) {
    if (possible_result_facts_ || value.result_fact_origins.empty()) return;
    if (value.result_type_origin && region_definition(value.type)) resolve_result_type(value, at);
    const auto facts = result_conformances(value.result_fact_origins, at);
    deferred_checks_.push_back({[this, origins = value.result_fact_origins, facts, at] {
      const auto current = result_conformances(origins, at);
      return !std::is_permutation(facts.begin(), facts.end(), current.begin(), current.end());
    }, at, "native result interface facts changed after recursive context refinement"});
    for (const auto &fact : facts) append_conformance(value.conformances, fact);
    value.result_fact_origins.clear();
  }
  static void merge_result_origins(Value &into, const Value &from) {
    for (size_t id : from.result_fact_origins)
      if (std::find(into.result_fact_origins.begin(), into.result_fact_origins.end(), id) == into.result_fact_origins.end())
        into.result_fact_origins.push_back(id);
  }
  void join_result_facts(Value &result, std::vector<Value> alternatives) {
    if (!(result.type.kind == Type::region && result.type.owned && alternatives.size() > 1) &&
        std::none_of(alternatives.begin(), alternatives.end(),
            [](const Value &value) { return !value.result_fact_origins.empty(); })) return;
    return_facts_.push_back({nullptr, std::move(alternatives)});
    result.result_fact_origins = {return_facts_.size()};
    if (std::any_of(return_facts_.back().alternatives.begin(), return_facts_.back().alternatives.end(),
        [](const Value &value) { return value.result_type_origin != 0; }))
      result.result_type_origin = return_facts_.size();
  }
  Value borrowed_call_result(const Type &type, const std::vector<Value> &args,
                             llvm::Value *address, const Location &at, size_t callable_address = 0,
                             size_t record_result = 0) {
    const auto owner = type.owners.back();
    for (size_t i = 0; i < args.size(); ++i) {
      if (type.owners[i] != owner) continue;
      valid_view(args[i], at);
      auto result = scalar(*type.target, nullptr, args[i].region_id);
      result.read_only = !type.const_views.empty() && type.const_views.back();
      if (result.type.kind == Type::region) {
        region_pointer(result, at);
        result.native = address;
        const auto &root = regions_.at(*result.region_id).root;
        result.type.target = root ? std::make_shared<Type>(*root) : nullptr;
      } else if (result.type.kind == Type::array || result.type.kind == Type::regional) {
        result.native = ir_.nullable_offset(region_pointer(result, at), address);
        result.conformances = args[i].conformances; result.result_fact_origins = args[i].result_fact_origins;
      } else result.storage = StorageView{address, args[i].storage ? args[i].storage->scope : std::nullopt, {}, true};
      result.nullable = !type.nullable_views.empty() && type.nullable_views.back();
      result.pointer_view = result.type.kind != Type::region;
      result.callable_address = callable_address;
      if (record_result) {
        result.conformances.clear();
        if (result.type.kind == Type::regional) result.result_type_origin = record_result;
        result.result_fact_origins = {record_result};
      }
      if (result.nullable) {
        // Forwarding a pointer does not establish initialization of its target.
        for (size_t j = 0; result.storage && j < args.size(); ++j)
          if (type.owners[j] == owner && args[j].type == *type.target && args[j].storage)
            result.storage->cells.insert(args[j].storage->cells.begin(), args[j].storage->cells.end());
        return result;
      }
      return read_value(std::move(result), at);
    }
    fail(at, "cannot infer borrowed result owner from the arguments");
  }
  Value call_factory(Form &form, const Module &lexical, Environment env, const std::vector<Value> &args,
                     bool function_context, const Location &at, const Module &caller,
                     ImplementationContext implementation,
                     std::map<size_t, std::pair<Type, const Value *>> binding_shape) {
    const auto instantiated = *signature(form, lexical, args, caller, at, true);
    if (declares_npe(form.throws_types, lexical, form.at)) require_npe_handler(at);
    const StorageView destination{ir_.hold_callable(ir_.null_pointer(), module_top_level_), local_scope_, {}};
    std::vector<Value> parameters;
    std::vector<llvm::Value *> operands;
    std::vector<std::tuple<Type, std::optional<size_t>, size_t, bool>> shapes;
    auto conformances = caller_conformances_;
    for (size_t i = 0; i < args.size(); ++i) {
      valid_view(args[i], at);
      auto value = admit(args[i], instantiated.parameters[i], caller, at);
      if (parameter_at(form, i).read_only && storage_owner(value)) value.read_only = true;
      shapes.emplace_back(value.type, value.region_id, value.callable_flow, value.read_only);
      for (const auto &fact : contextual(value, caller).conformances) append_conformance(conformances, fact);
      if (native_callable(value)) {
        if (!value.callable_temporary || !value.storage)
          fail(at, "internal error: native callable argument has no prepared value");
        operands.push_back(ir_.take_callable(value.storage->address));
      } else operands.push_back(stored_value(value, value.type, at));
      parameters.push_back(std::move(value));
    }
    const auto selected_context = implementation.empty() ? ImplementationContext{&lexical} : implementation;
    const auto invoke = [&](const LLVMEmission::NativeDefinition &definition, const Value &result) {
      auto *image = ir_.native_call(definition, ir_.native_environment(definition), operands, nullptr, destination.address);
      if (!definition.returns) return scalar(Type::never, nullptr);
      return held_callable(result.type, result.callable_flow, image, destination);
    };
    for (const auto &checked : factories_)
      if (checked.activation.form == &form && checked.activation.lexical == &lexical &&
          checked.activation.implementation == selected_context && checked.activation.parameters == instantiated.parameters &&
          checked.activation.bindings == binding_shape &&
          (checked.checking || (checked.caller == &caller && ir_.context_free(checked.activation.definition))) &&
          checked.conformances == conformances && checked.arguments == shapes)
        return invoke(checked.activation.definition, checked.result);
    callable_flows_.push_back({{}, {}, true});
    const auto result_flow = callable_flows_.size() - 1;
    auto pending = scalar(*instantiated.result, nullptr);
    pending.callable_flow = result_flow;
    factories_.push_back({{&form, &lexical, selected_context, instantiated.parameters, std::move(binding_shape),
        ir_.native_declaration(form.name, instantiated.parameters, *instantiated.result)},
        &caller, conformances, shapes, pending});
    auto &factory = factories_.back();
    factory.activation.definition.checked_null = declares_npe(form.throws_types, lexical, form.at);
    native_activations_.push_back(factory.activation);
    auto &active = native_activations_.back();
    auto previous_ir = ir_.begin_native(active.definition);
    const auto call_begin = native_call_demands_.size();
    const auto previous_flow = flow_state();
    std::map<size_t, llvm::Value *> region_bases;
    Value result;
    {
      // Source expansion guards belong to one native frame, not the checker's call stack.
      const auto expansion = llvm::SaveAndRestore(active_calls_, std::set<const Form *>{});
      const auto context = llvm::SaveAndRestore(native_context_, &active);
      const auto handlers = llvm::SaveAndRestore(handlers_, std::vector<HandlerScope *>{});
      const auto loops = llvm::SaveAndRestore(loop_pointers_, std::vector<LoopPointers>{});
      const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
      const auto cleanup = ir_.scope();
      for (size_t i = 0; i < parameters.size(); ++i) {
        auto &value = parameters[i];
        auto *argument = active.definition.function->getArg(unsigned(i + 2));
        if (local_bits(value.type)) value = loaded_value(value.type, argument);
        else {
          value.native = argument;
          value.pointer.reset();
          value.pointer_view = false;
          if (value.type.kind == Type::region) {
            region_bases.try_emplace(*value.region_id, regions_[*value.region_id].owner);
            regions_[*value.region_id].owner = argument;
          } else {
            value.storage = StorageView{ir_.hold_callable(argument, false), local_scope_, {}};
            value.callable_address = 0;
            register_callable_holder(value);
          }
        }
      }
      result = inline_call(form, lexical, std::move(env), parameters, function_context, at, caller,
                           std::move(implementation), nullptr);
      active.definition.returns = result.type != Type::never;
      if (active.definition.returns) {
        return_callable(result, result_flow, active.definition, at);
        ir_.restore(cleanup);
        ir_.native_return();
      } else ir_.restore(cleanup);
    }
    resolve_native_calls(call_begin);
    ir_.end_native(active.definition, std::move(previous_ir));
    for (const auto &[region, base] : region_bases) regions_[region].owner = base;
    restore_flow(previous_flow);
    callable_flows_.at(result_flow).open = false;
    factory.activation = active;
    factory.checking = false;
    if (result.type != Type::never) factory.result = result;
    factory.result.callable_flow = result_flow;
    native_activations_.pop_back();
    return invoke(factory.activation.definition, factory.result);
  }
  Value direct_callable(Form &form, const Module &lexical, Type type,
                        const ImplementationContext &implementation) {
    for (const auto &value : direct_callables_)
      if (value.procedure->form == &form && value.procedure->lexical == &lexical &&
          value.procedure->implementation == implementation) {
        // The immutable descriptor identifies this body/context. Instantiated
        // argument and owner types belong to its checked call, not its image.
        auto selected = value;
        selected.type = std::move(type);
        return selected;
      }
    auto procedure = std::make_shared<Procedure>();
    procedure->form = &form;
    procedure->lexical = &lexical;
    procedure->implementation = implementation;
    auto *layout = ir_.callable_layout({}, {});
    auto *key = ir_.callable_key();
    procedure->code = std::make_shared<CallableCode>();
    procedure->code->layout = layout;
    procedure->code->dispatch_key = key;
    procedure->code->direct = true;
    auto value = scalar(std::move(type), ir_.static_callable(layout, key));
    value.procedure = std::move(procedure);
    callable_flows_.push_back({value.procedure, {}});
    value.callable_flow = callable_flows_.size() - 1;
    direct_callables_.push_back(value);
    return value;
  }
  Type instantiated_callable_type(const Form &form, const Module &lexical,
                                  const Signature &instantiated, size_t arity) {
    auto type = procedure_type(form, lexical);
    const auto bind_type = [&](auto &&self, Type value) -> Type {
      if (value.kind == Type::parameter)
        return instantiated.bindings.at(form.type_parameters.at(value.identity).name);
      if (value.target) value.target = std::make_shared<Type>(self(self, *value.target));
      return value;
    };
    for (auto &bound : type.owner_bounds) bound = bind_type(bind_type, bound);
    type.parameters = instantiated.parameters;
    type.target = std::make_shared<Type>(*instantiated.result);
    type.bounds.clear(); type.variadic = false;
    type.const_views.clear();
    if (!type.owners.empty()) {
      const auto owners = type.owners;
      type.owners.clear();
      for (size_t i = 0; i < arity; ++i) type.owners.push_back(owners[std::min(i, form.parameters.size() - 1)]);
      type.owners.push_back(owners.back());
    }
    if (!type.nullable_views.empty()) {
      const auto nullable = type.nullable_views;
      type.nullable_views.clear();
      for (size_t i = 0; i < arity; ++i)
        type.nullable_views.push_back(nullable[std::min(i, form.parameters.size() - 1)]);
      type.nullable_views.push_back(nullable.back());
    }
    for (size_t i = 0; i < arity; ++i) type.const_views.push_back(parameter_at(form, i).read_only);
    type.const_views.push_back(form.result_const);
    if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; }))
      type.const_views.clear();
    return type;
  }
  Value call(Form &form, const Module &lexical, Environment env, const std::vector<Value> &args,
             bool function_context, const Location &at, const Module &caller, ImplementationContext implementation = {},
             const Locals *captures = nullptr) {
    const auto instantiated = *signature(form, lexical, args, caller, at, true, captures);
    bool native_environment = true;
    std::map<size_t, std::pair<Type, const Value *>> binding_shape;
    for (const auto &[id, value] : env.bindings) {
      native_environment &= local_bits(value->type) || (value->procedure && !value->procedure->builtin.empty());
      native_environment &= value->type.kind != Type::exception || value->type.identity != 0;
      binding_shape.emplace(id, std::pair(value->type, env.captured(id, value) ? value : nullptr));
    }
    bool factory = native_environment && instantiated.result && native_callable_type(*instantiated.result) &&
        form.result_owner.empty() && form.result_kind != "ptr" && form.body && !form.operation &&
        (!captures || (captures->visible.empty() && captures->owner_anchors.empty()));
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &value = args[i];
      factory &= value.native && !value.nullable && !instantiated.parameters[i].owned &&
          ((local_bits(value.type) && parameter_at(form, i).owner.empty()) ||
           value.type.kind == Type::region ||
           (native_callable(value) && parameter_at(form, i).owner.empty()));
    }
    const bool contextual_parameters = (instantiated.result && instantiated.result->kind == Type::region && instantiated.result->owned) ||
        !form.owner_parameters.empty() || std::any_of(args.begin(), args.end(), [](const Value &value) {
      return value.type.kind == Type::procedure || value.type.kind == Type::region;
    });
    // Keep reusable closed factories on their shared-specialization path.
    if (contextual_parameters && !(factory && !form.owner_parameters.empty()) &&
        form.body && !form.operation && form.result_type &&
        (!captures || (captures->visible.empty() && captures->owner_anchors.empty()))) {
      auto type = instantiated_callable_type(form, lexical, instantiated, args.size());
      bool native = native_callable_type(type);
      for (size_t i = 0; i < args.size(); ++i) {
        const auto &value = args[i];
        if (borrowed_parameter(type, i)) {
          native &= value.storage || value.native;
        } else native &= value.native && !value.nullable &&
            (local_bits(value.type) || object_type(value.type) || value.type.kind == Type::region ||
             (native_callable(value) && value.callable_temporary && value.storage));
      }
      if (native) {
        if (!type.throws_types.empty()) require_npe_handler(at);
        auto selected = direct_callable(form, lexical, type,
            implementation.empty() ? ImplementationContext{&lexical} : implementation);
        auto parameters = args;
        std::vector<llvm::Value *> operands;
        for (size_t i = 0; i < args.size(); ++i) {
          valid_view(args[i], at);
          if (type.parameters[i].kind == Type::region)
            parameters[i] = admit(args[i], type.parameters[i], caller, at);
          operands.push_back(borrowed_parameter(type, i) ? borrowed_operand(args[i], at) :
              native_callable(args[i]) ? ir_.take_callable(args[i].storage->address)
              : stored_value(args[i], type.parameters[i], at));
        }
        std::optional<StorageView> destination;
        if (type.target->kind == Type::procedure && !borrowed_result(type))
          destination = StorageView{ir_.hold_callable(ir_.null_pointer(), module_top_level_), local_scope_, {}};
        return contextual_call(selected, parameters, operands, env, caller, at, false, destination);
      }
    }
    if (factory) return call_factory(form, lexical, std::move(env), args, function_context, at, caller,
                                     std::move(implementation), std::move(binding_shape));
    const bool value_result = instantiated.result &&
        (local_bits(*instantiated.result) || *instantiated.result == Type::void_type) && form.result_owner.empty();
    bool native = native_environment && value_result && form.body && !form.operation && !caught_exception_ &&
        (!captures || (captures->visible.empty() && captures->owner_anchors.empty()));
    for (size_t i = 0; i < args.size(); ++i)
      native &= local_bits(args[i].type) && !args[i].nullable && args[i].native &&
                parameter_at(form, i).owner.empty();
    if (!native)
      return inline_call(form, lexical, std::move(env), args, function_context, at, caller,
                         std::move(implementation), captures);
    if (declares_npe(form.throws_types, lexical, form.at)) require_npe_handler(at);
    auto selected_context = implementation.empty() ? ImplementationContext{&lexical} : implementation;
    std::vector<Value> values;
    std::vector<llvm::Value *> operands;
    for (size_t i = 0; i < args.size(); ++i) {
      valid_view(args[i], at);
      auto value = admit(args[i], instantiated.parameters[i], caller, at);
      operands.push_back(stored_value(value, value.type, at));
      values.push_back(bits_value(std::move(value)));
    }
    for (auto &active : native_activations_)
      if (active.form == &form && active.lexical == &lexical && active.implementation == selected_context &&
          active.parameters == instantiated.parameters && active.bindings == binding_shape) {
        auto *returned = ir_.native_call(active.definition, active.definition.function->getArg(0), operands);
        return *instantiated.result == Type::void_type ? Value{} : loaded_value(*instantiated.result, returned);
      }

    auto conformances = caller_conformances_;
    for (const auto &argument : args)
      for (const auto &fact : contextual(argument, caller).conformances)
        append_conformance(conformances, fact);
    for (const auto &checked : checked_natives_[&form]) {
      const auto &active = checked.activation;
      if (active.form != &form || active.lexical != &lexical || active.implementation != selected_context ||
          active.parameters != instantiated.parameters || active.bindings != binding_shape ||
          checked.caller != &caller || checked.conformances != conformances) continue;
      auto *returned = ir_.native_call(active.definition, ir_.null_pointer(), operands);
      if (!active.definition.returns) return scalar(Type::never, nullptr);
      return active.definition.result == Type::void_type ? Value{} : loaded_value(active.definition.result, returned);
    }

    native_activations_.push_back({&form, &lexical, selected_context, instantiated.parameters,
        std::move(binding_shape), ir_.native_declaration(form.name, instantiated.parameters, *instantiated.result)});
    auto &active = native_activations_.back();
    active.definition.checked_null = declares_npe(form.throws_types, lexical, form.at);
    auto previous_ir = ir_.begin_native(active.definition);
    const auto call_begin = native_call_demands_.size();
    const auto previous_flow = flow_state();
    {
      // Source expansion guards belong to one native frame, not the checker's call stack.
      const auto expansion = llvm::SaveAndRestore(active_calls_, std::set<const Form *>{});
      const auto context = llvm::SaveAndRestore(native_context_, &active);
      const auto handlers = llvm::SaveAndRestore(handlers_, std::vector<HandlerScope *>{});
      for (size_t i = 0; i < values.size(); ++i)
        values[i] = loaded_value(values[i].type, active.definition.function->getArg(unsigned(i + 1)));
      auto result = inline_call(form, lexical, std::move(env), values, function_context, at, caller,
                                std::move(implementation), captures);
      active.definition.returns = result.type != Type::never;
      if (active.definition.returns)
        ir_.native_return(result.type == Type::void_type ? nullptr : stored_value(result, result.type, at));
    }
    resolve_native_calls(call_begin);
    ir_.end_native(active.definition, std::move(previous_ir));
    restore_flow(previous_flow);
    // A closed native definition has no SSA dependency on its first caller.
    // Reuse it only under the same semantic view; aliases and extension worlds
    // must not accidentally share a specialization.
    if (function_context && active.bindings.empty() && ir_.context_free(active.definition))
      checked_natives_[&form].push_back({active, &caller, std::move(conformances)});
    auto definition = std::move(active.definition);
    native_activations_.pop_back();
    auto *returned = ir_.native_call(definition, ir_.native_environment(definition), operands);
    if (!definition.returns) return scalar(Type::never, nullptr);
    return definition.result == Type::void_type ? Value{} : loaded_value(definition.result, returned);
  }
  Value inline_call(Form &form, const Module &lexical, Environment env, const std::vector<Value> &args,
                    bool function_context, const Location &at, const Module &caller, ImplementationContext implementation,
                    const Locals *captures) {
    const bool throws_npe = declares_npe(form.throws_types, lexical, form.at);
    if (throws_npe) require_npe_handler(at);
    const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, throws_npe);
    const auto instantiated = *signature(form, lexical, args, caller, at, true, captures);
    const auto types = llvm::SaveAndRestore(type_context_, TypeContext{&lexical, instantiated.bindings});
    if (!form.body) fail(at, "callable '" + form.name + "' has no method implementation");
    const auto references = lexical_references(*form.body, captures ? *captures : Locals{}, &form);
    if (!function_context)
      for (const auto *touch : references.touches)
        read_symbol(expression_symbol(*touch, lexical), env, touch->at, false);
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
    const auto parent_scope = local_scope_;
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);
    const auto cleanup_scope = llvm::SaveAndRestore(ir_.cleanup_owner(), scope_id);
    std::vector<Value> parameters;
    std::set<size_t> transferring;
    for (size_t i = 0; i < args.size(); ++i) {
      valid_view(args[i], at);
      auto parameter = admit(args[i], instantiated.parameters[i], caller, at);
      const auto &formal = parameter_at(form, i);
      if (parameter.type.kind == Type::region && formal.type.kind != Type::region)
        fail(formal.at, "a whole-region parameter requires a region declaration");
      if (formal.owner.empty() && object_type(parameter.type))
        fail(formal.at, "a non-bits parameter requires an explicit external owner: use 'in O' and 'where {owner O}'");
      if (formal.declaration_kind == "val" && !local_bits(parameter.type))
        fail(formal.at, "val parameters require bits; regional objects are borrowed through an explicit owner");
      if (!formal.owner.empty()) parameter.argument_reads.clear();
      if (formal.owner.empty() && local_bits(parameter.type)) {
        if (!parameter.native) fail(at, "read of storage before definite initialization in a local parameter argument");
        const auto source = bits_value(parameter);
        parameter = local_storage(parameter.type, formal.at);
        write_bits(parameter, source, at);
      }
      if (formal.owner.empty() && native_callable(parameter)) {
        if (!parameter.callable_temporary || !parameter.storage)
          fail(at, "internal error: callable argument has no prepared value");
        parameter.native = ir_.take_callable(parameter.storage->address);
        parameter.storage = StorageView{ir_.hold_callable(parameter.native, false), local_scope_, {}};
        parameter.pointer.reset();
        parameter.pointer_view = false;
        parameter.callable_temporary = false;
        parameter.read_only = false;
        register_callable_holder(parameter);
      }
      if (!formal.owner.empty() && native_callable(parameter)) parameter.callable_temporary = false;
      if (formal.read_only && storage_owner(parameter)) parameter.read_only = true;
      if (formal.fixed_binding && (formal.declaration_kind == "val" ||
          (native_callable(parameter) && formal.owner.empty()))) parameter.read_only = true;
      if (parameter.type.owned && (!parameter.holding_id || !transferring.insert(*parameter.holding_id).second))
        fail(at, "one region owner cannot be consumed by multiple parameters");
      parameters.push_back(std::move(parameter));
    }
    // Revalidation precedes every move: later arguments may have changed an
    // earlier owner or view. Transfers begin only at this callee-entry boundary.
    for (auto &parameter : parameters)
      if (parameter.type.owned) parameter = hold_region(take_owner(parameter, at), false, true);
    Locals locals = captures ? captures->nested() : Locals{};
    const auto &owner_values = instantiated.owner_values;
    for (const auto &owner : form.owner_parameters) {
      const auto found = owner_values.find(owner.name);
      if (found == owner_values.end()) fail(owner.at, "cannot infer owner '" + owner.name + "' from the arguments");
      if (!owner.explicit_parameter) {
        Value binding;
        if (owner.root) binding = found->second;
        else {
          binding.provenance_only = true;
          binding.read_only = true;
          locals.owner_anchors.insert_or_assign(owner.name, found->second);
        }
        binding.binding_read_only = true;
        declare_local(locals, owner.name, std::move(binding), lexical, owner.at);
      }
    }
    const auto fixed = form.parameters.size() - (variadic(form) ? 1 : 0);
    const bool rebind = form.operation && form.name == "=";
    for (size_t i = 0; i < fixed; ++i) {
      auto argument = parameters[i];
      if ((rebind && i == 0) || (form.operation && form.name == "<=>")) {
        if (!argument.pointer && !argument.reference_field)
          fail(at, "a reference operation overload requires a pointer binding destination");
        // Destinations denote caller bindings, including regional reference slots.
        const bool regional = bool(argument.reference_field);
        argument.binding_read_only |= form.parameters[i].fixed_binding;
        declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
        if (regional) locals.regional_slots.insert(form.parameters[i].name);
      } else if (form.parameters[i].declaration_kind == "val")
        declare_local(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at);
      else if (argument.storage || (!form.parameters[i].owner.empty() && object_type(argument.type)))
        declare_pointer(locals, form.parameters[i].name, std::move(argument), lexical, form.parameters[i].at,
                        form.parameters[i].read_only, form.parameters[i].fixed_binding, form.parameters[i].declaration_kind == "ref");
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
      pack.borrowed_arguments = !form.parameters.back().owner.empty();
      declare_local(locals, form.parameters.back().name, std::move(pack), lexical, form.parameters.back().at);
    }
    const bool assignment = form.operation && (form.name == "=" || form.name == "<-" || form.name == "[]=" || form.name == "[]<-");
    const Value *result_owner = form.result_owner.empty() ? nullptr : &owner_values.at(form.result_owner);
    ReturnContext context{instantiated.result, result_owner,
        form.result_owner.empty() && !assignment &&
            (form.result_const || (instantiated.result && instantiated.result->kind == Type::procedure && form.result_kind != "ptr")),
        form.result_kind != "ptr", rebind ? args.front().pointer : std::nullopt};
    const auto callable_return = llvm::SaveAndRestore(return_context_, &context);
    const auto caller_returns = llvm::SaveAndRestore(return_targets_, std::vector<ReturnTarget *>{});
    ReturnTarget destination{native_scope, holding_begin, scope_id, &env, ir_.label("callable.return")};
    return_targets_.push_back(&destination);
    const auto enclosing_defer = llvm::SaveAndRestore(deferred_scope_, static_cast<DeferredScope *>(nullptr));
    auto body = deferred_scope(*form.body, lexical, env, locals, function_context,
        [&] { return block(*form.body, lexical, env, locals, function_context, true); }, true);
    if (body.type != Type::never) {
      if (context.type && *context.type != Type::void_type) {
        warn(Warning::missing_return, form.at, "value-returning callable may finish without an explicit return");
        const auto location = ir_.source_location(form.at);
        ir_.missing_return();
      } else route_return({}, form.at);
    }
    return_targets_.pop_back();
    ir_.enter(destination.block);
    auto result = join_completions(destination.incoming, form.at, false);
    if (form.result_kind != "ptr" && result.nullable) result = read_value(std::move(result), at);
    scopes_[scope_id] = false;
    result = finish_scope(holding_begin, native_scope, std::move(result), at, parent_module, parent_scope);
    active_calls_.erase(&form);
    if (instantiated.result) result = admit(std::move(result), *instantiated.result, lexical, form.at);
    if (!form.result_owner.empty()) {
      const auto owner = owner_values.find(form.result_owner);
      if (owner == owner_values.end() || storage_owner(result) != storage_owner(owner->second))
        fail(form.at, "result does not belong to its declared owner");
    }
    if (form.result_const && storage_owner(result)) result.read_only = true;
    if (!form.result_const && !form.result_owner.empty())
      require_writable(result.read_only, form.at, "a writable borrowed result cannot return a read-only view");
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
  std::optional<Route> route_to(Type actual, Type target, const Parameter &parameter, const Module &lexical,
                              const Module &caller, const Location &at, bool diagnose_unavailable = true) {
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
  std::optional<Route> route(Type actual, const Parameter &parameter, const Module &lexical,
                              const Module &caller, const Location &at, bool diagnose_unavailable = true) {
    return route_to(actual, resolve(parameter.type, lexical, parameter.at), parameter,
                    lexical, caller, at, diagnose_unavailable);
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
      if (!form.owner_parameters.empty()) {
        message += " where {";
        for (size_t j = 0; j < form.owner_parameters.size(); ++j) {
          const auto &owner = form.owner_parameters[j];
          message += (j ? ", " : "") + std::string(owner.root ? "region " : "owner ") + owner.name;
          if (owner.root) message += ":" + type_name(*owner.root);
        }
        message += "}";
      }
      if (!candidates[i].implementation.empty() && candidates[i].implementation.back() != candidates[i].lexical)
        message += "; implementation context '" + candidates[i].implementation.back()->path + "'";
    }
    if (shown < candidates.size())
      message += "\nnote: " + std::to_string(candidates.size() - shown) + " additional candidates omitted";
    fail(at, message);
  }
  std::optional<MethodMatch> method(const std::string &name, const Module &module,
                                    const std::vector<Value> &provided, const Location &at) {
    const auto information = lookup_callable(module, name, at);
    const auto &candidates = information.methods;
    if (candidates.empty()) return {};
    auto args = provided;
    for (auto &value : args) {
      try { resolve_region_type(value, at); }
      catch (const UnresolvedAccess &) { resolve_result_type(value, at, true); }
      if (value.result_type_origin && region_definition(value.type)) resolve_result_type(value, at);
    }
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
  static Type method_abi(Type type) {
    for (size_t i = 0; i < type.parameters.size() && !type.const_views.empty(); ++i) type.const_views[i] = false;
    if (std::none_of(type.const_views.begin(), type.const_views.end(), [](bool value) { return value; }))
      type.const_views.clear();
    return type;
  }
  std::optional<Value> defer_method(const std::string &name, const Module &module, const std::vector<Value> &args,
                                    const Environment &env, const Location &at, bool function_context,
                                    const std::vector<ConversionPoint> &points) {
    std::optional<Type> prototype;
    {
      const auto possible = llvm::SaveAndRestore(possible_result_facts_, true);
      for (const auto &candidate : lookup_callable(module, name, at).methods) {
        const auto &form = *candidate.form;
        const auto instantiated = signature(form, *candidate.lexical, args, module, at, false, nullptr, false);
        if (!instantiated) continue;
        bool exact = true;
        for (size_t i = 0; i < args.size(); ++i) exact &= accepts(args[i], instantiated->parameters[i], module, at);
        if (!exact) continue;
        if (!form.result_type || !instantiated->result || form.operation || !form.body) return {};
        if (const auto source = candidate.lexical->execution.procedures.find(&form);
            source != candidate.lexical->execution.procedures.end() &&
            (!source->second.procedure->lexical_captures.visible.empty() ||
             !source->second.procedure->lexical_captures.owner_anchors.empty())) return {};
        auto type = method_abi(instantiated_callable_type(form, *candidate.lexical, *instantiated, args.size()));
        if (!native_callable_type(type) || (prototype && *prototype != type)) return {};
        prototype = std::move(type);
      }
    }
    if (!prototype) return {};
    std::vector<llvm::Value *> operands;
    for (size_t i = 0; i < args.size(); ++i) {
      const auto &value = args[i];
      valid_view(value, at);
      if (borrowed_parameter(*prototype, i)) operands.push_back(borrowed_operand(value, at));
      else if (native_callable(value)) {
        if (!value.storage || !value.callable_temporary || !points.at(i).callable_prepared) return {};
        operands.push_back(ir_.take_callable(value.storage->address));
      } else {
        if (!value.native || value.nullable) return {};
        operands.push_back(stored_value(value, prototype->parameters[i], at));
      }
    }
    if (!prototype->throws_types.empty()) require_npe_handler(at);
    auto *image = ir_.pending_callable();
    auto selected = scalar(*prototype, image);
    callable_flows_.push_back({{}, {}, true});
    selected.callable_flow = callable_flows_.size() - 1;
    std::optional<StorageView> destination;
    if (prototype->target->kind == Type::procedure && !borrowed_result(*prototype))
      destination = StorageView{ir_.hold_callable(ir_.null_pointer(), module_top_level_), local_scope_, {}};
    const auto index = native_call_demands_.size();
    auto result = contextual_call(selected, args, operands, env, module, at, false, destination);
    native_call_demands_.at(index).selection = NativeCallDemand::Selection{
        name, function_context, npe_coverage(), implementation_context_, image, result};
    return result;
  }
  bool select_native_method(NativeCallDemand &demand) {
    try {
      const auto &pending = *demand.selection;
      const auto context = llvm::SaveAndRestore(implementation_context_, pending.implementation);
      const auto conformances = llvm::SaveAndRestore(caller_conformances_, demand.conformances);
      const auto checked = llvm::SaveAndRestore(npe_allowed_, bool(pending.npe_allowed));
      const auto calls = llvm::SaveAndRestore(call_path_, demand.calls);
      const auto selected = method(pending.name, *demand.caller, demand.arguments, demand.at);
      if (!selected) method_failure(pending.name, demand.arguments,
          lookup_callable(*demand.caller, pending.name, demand.at).methods, demand.at, false, *demand.caller);
      const auto &source = selected->function;
      const auto instantiated = *signature(*source.form, *source.lexical, demand.arguments, *demand.caller, demand.at, true);
      auto type = instantiated_callable_type(*source.form, *source.lexical, instantiated, demand.arguments.size());
      if (method_abi(type) != demand.type || std::any_of(selected->routes.begin(), selected->routes.end(),
          [](const Route &route) { return !route.steps.empty() || route.target; }))
        fail(demand.at, "access-dependent result types or argument conversions need further staging");
      if (!type.pure_callable && pending.function_context)
        fail(demand.at, "a function cannot invoke a procedure");
      require_declaration_handlers(selected->declarations, demand.arguments, *demand.caller, demand.at);
      check_declarations(selected->declarations, source.form->kind, demand.arguments, pending.result,
          *demand.caller, demand.at, declares_npe(source.form->throws_types, *source.lexical, source.form->at));
      auto callable = direct_callable(*source.form, *source.lexical, type, source.implementation);
      callable_flows_.at(demand.flow).inputs = {callable.callable_flow};
      callable_flows_.at(demand.flow).open = false;
      pending.image->replaceAllUsesWith(callable.native);
      pending.image->eraseFromParent();
      demand.type = std::move(type);
      for (size_t i = 0; i < demand.arguments.size(); ++i)
        if (!demand.type.const_views.empty() && demand.type.const_views[i] && storage_owner(demand.arguments[i]))
          demand.arguments[i].read_only = true;
      demand.selection.reset();
      return true;
    } catch (const UnresolvedAccess &) { return false; }
  }
  Value converted(Value value, const Route &route, const Location &at, const Module &caller,
                  const ConversionPoint *point = nullptr, bool read_argument = false,
                  bool local_parameter = false) {
    const bool copy_callable = local_parameter && native_callable(value) &&
                               !(point && point->callable_prepared);
    if (route.steps.empty() && !route.target && !read_argument && !copy_callable) return value;
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
          [&](size_t cell) { return initialization_known(cells_.at(cell).initialized) != false; });
      if (initialized) value = read_value(std::move(value), at, true);
      else value.native = nullptr;
    }
    for (const auto &step : route.steps) {
      value = widened(value, conversion(step).source, at);
      value = call(*step.form, *step.lexical, {}, {value}, true, at, caller, step.implementation);
      if (value.type == Type::never) break;
    }
    if (route.target && value.type != Type::never) value = widened(value, *route.target, at);
    if (copy_callable && value.type != Type::never) {
      if (point && !point->callable_destination) fail(at, "internal error: callable argument cleanup was not reserved");
      value = prepare_callable_argument(std::move(value), point ? point->callable_destination : std::nullopt);
    }
    if (resume) {
      ir_.end_conversion(point->native, *resume);
      restore_flow(state);
      if (value.type == Type::never) ir_.unreachable();
    }
    if (value.type == Type::never) throw EndPath{};
    if (copy_callable) register_callable_holder(value);
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
      if (!declaration.form->result_const && !declaration.form->result_owner.empty())
        require_writable(result.read_only, at, "read-only method result violates writable callable declaration\n" +
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
  void require_declaration_handlers(const std::vector<CallableDeclaration> &declarations,
                                     const std::vector<Value> &args, const Module &caller, const Location &at) {
    for (const auto &declaration : declarations) {
      const auto contract = signature(*declaration.form, *declaration.lexical, args, caller, at);
      if (!contract) continue;
      bool applicable = true;
      for (size_t i = 0; i < args.size(); ++i)
        applicable &= accepts(args[i], contract->parameters[i], caller, at);
      if (applicable && declares_npe(declaration.form->throws_types, *declaration.lexical, declaration.form->at))
        require_npe_handler(at);
    }
  }
  struct PreparedMethodCall {
    MethodMatch method;
    std::vector<Value> arguments;
  };
  PreparedMethodCall prepare_method_call(const MethodMatch &match, std::vector<Value> args, const Location &at,
                                         bool function_context, const Module &caller,
                                         const std::vector<ConversionPoint> &points = {}) {
    const auto &form = *match.function.form;
    if (form.operation && (form.name == "=" || form.name == "<-" || form.name == "[]=" || form.name == "[]<-") &&
        (form.parameters.empty() || form.parameters.front().owner.empty()))
      fail(form.at, "an assignment destination requires an explicit external owner: use 'in O' and 'where {owner O}'");
    if (form.operation && swap_operation(form.name) &&
        (form.parameters.size() != 2 || form.parameters[0].owner.empty() || form.parameters[1].owner.empty() || variadic(form)))
      fail(form.at, "a swap overload requires two destinations with explicit external owners");
    if (form.operation && (form.name == "=" || form.name == "<=>"))
      for (size_t i = 0; i < (form.name == "=" ? 1u : 2u) && i < args.size(); ++i) {
        require_writable(args[i].binding_read_only, at, "cannot rebind a const binding or alias");
        if (args[i].pointer && pointers_.at(*args[i].pointer).fixed_binding)
          fail(at, "cannot rebind a const ptr binding");
      }
    for (size_t i = 0; i < args.size(); ++i)
      args[i] = converted(args[i], match.routes[i], at, caller, points.empty() ? nullptr : &points[i],
          !points.empty() && points[i].raw_view && !(parameter_at(form, i).declaration_kind == "ptr" && !parameter_at(form, i).owner.empty()),
          parameter_at(form, i).owner.empty());
    const bool function = match.function.form->kind == Form::Kind::function;
    if (!function && function_context) {
      const auto &form = *match.function.form;
      const std::string message = form.operation ? "a function cannot invoke a procedure overload"
                                                 : "a function cannot invoke a procedure";
      fail(at, message + "\n" + location_text(form.at) + ": note: selected procedure '" +
                   callable_name(form) + "' declared here");
    }
    require_declaration_handlers(match.declarations, args, caller, at);
    return {match, std::move(args)};
  }
  Value invoke_prepared_method(const PreparedMethodCall &prepared, const Location &at,
                                const Environment &env, const Module &caller) {
    const auto &match = prepared.method;
    const auto &args = prepared.arguments;
    const auto &form = *match.function.form;
    const bool function = form.kind == Form::Kind::function;
    for (const auto &argument : args) valid_view(argument, at);
    // Lexical procedure calls use current bindings; perform uses captured values.
    const Locals *captures = nullptr;
    if (!function && !form.operation) {
      const auto definition = match.function.lexical->execution.procedures.find(&form);
      if (definition != match.function.lexical->execution.procedures.end()) {
        valid_view(definition->second, at);
        captures = &definition->second.procedure->lexical_captures;
      }
    }
    auto result = call(*match.function.form, *match.function.lexical, function ? Environment{} : env, args, function, at, caller,
                       match.function.implementation, captures);
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
  Value call_method(const MethodMatch &match, std::vector<Value> args, const Location &at,
                    const Environment &env, bool function_context, const Module &caller,
                    const std::vector<ConversionPoint> &points = {}) {
    auto prepared = prepare_method_call(match, std::move(args), at, function_context, caller, points);
    return invoke_prepared_method(prepared, at, env, caller);
  }
  Value transfer_callable(Value destination, Value source, const Location &at) {
    require_writable(destination.read_only, at, "cannot transfer into a read-only destination view");
    require(source.type, destination.type, at);
    if (!native_callable(source)) fail(at, "callable transfer requires a compatible native value");
    auto *slot = place(destination, at).address;
    auto transferred = detach_callable(std::move(source), at);
    ir_.callable_write();
    ir_.replace_callable(slot, transferred.native);
    if (!destination.callable_address) fail(at, "callable replacement requires a lexical value holder");
    const auto targets = callable_targets(destination.callable_address);
    const auto memory = callable_holders_;
    for (const auto &[holder, previous] : memory) {
      if (targets.closed && !targets.holders.contains(holder)) continue;
      size_t flow = transferred.callable_flow;
      if (!targets.closed) {
        CallableFlow write;
        write.inputs = {previous, transferred.callable_flow};
        write.address = destination.callable_address;
        write.written_holder = holder;
        callable_flows_.push_back(std::move(write));
        flow = callable_flows_.size() - 1;
      } else if (targets.holders.size() != 1) flow = join_callable_flow({previous, flow});
      callable_holders_ = callable_memory_factory_.add(callable_holders_, holder, flow);
    }
    destination.read_only = true;
    destination.pointer.reset();
    return destination;
  }
  Value transfer_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                            bool function_context) {
    if (expr.children.size() != 2) fail(expr.at, "object transfer requires a destination and source");
    auto destination = expression(*expr.children[0], module, env, locals, function_context, false);
    require_writable(destination.read_only, expr.at, "cannot transfer into a read-only destination view");
    auto source = expression(*expr.children[1], module, env, locals, function_context);
    if (!expr.intrinsic)
      if (const auto selected = method("<-", module, {destination, source}, expr.at))
        return call_method(*selected, {destination, source}, expr.at, env, function_context, module);
    if (destination.type.kind == Type::procedure && destination.storage)
      return transfer_callable(std::move(destination), std::move(source), expr.at);
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
  Value reference_source(Value value, Type target, size_t owner, bool required, const Location &at, bool read_only = false) {
    if (value.result_type_origin && region_definition(value.type)) {
      try { resolve_result_type(value, at); }
      catch (const UnresolvedAccess &) {
        const auto origin = value.result_type_origin;
        const auto declared = value.type;
        deferred_checks_.push_back({[this, origin, declared, target, at] {
          auto actual = scalar(declared, nullptr);
          actual.result_type_origin = origin;
          resolve_result_type(actual, at);
          return actual.type != target;
        }, at, "reference field requires exact concrete target type " + type_name(target)});
        value.type = target;
        value.result_type_origin = 0;
      }
    }
    if (value.type == Type::null_ref) {
      if (required) fail(at, "a ref field cannot contain null");
      value = scalar(reference(target), ir_.bits(Type::null_ref, 0), owner);
      value.nullable = true;
      return value;
    }
    if (required && value.nullable) fail(at, "a ref field requires a proven non-null target");
    if (!read_only) require_writable(value.read_only, at, "cannot store a read-only view in a writable reference field");
    if (!value.region_id) fail(at, "a regional reference field requires a regional target");
    require_same_region(*value.region_id, owner, at,
        "cross-region reference fields require a target in the same owner; use '<-' for bits transfer");
    valid_view(value, at);
    value.region_id = owner;
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
    require_writable(destination.binding_read_only, at, "cannot rebind a const field or a field through a read-only view");
    auto stored = reference_source(source, *destination.reference_field->type.target,
                                   *destination.region_id, destination.required_reference, at, destination.reference_read_only);
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
    require_writable(destination.binding_read_only, expr.at, "cannot rebind a const binding or alias");
    if (destination.reference_field) return rebind_field(destination, source, expr.at);
    if (source.type == Type::null_ref && destination.pointer) source = null_target(destination.type, destination, expr.at);
    if (!destination.pointer) fail(expr.at, "reference assignment requires an ordinary ptr binding; use '<-' for bits transfer");
    if (pointers_.at(*destination.pointer).fixed_binding) fail(expr.at, "cannot rebind a const ptr binding");
    if (!source.nullable && source.storage && std::all_of(source.storage->cells.begin(), source.storage->cells.end(),
        [&](size_t cell) { return initialization_known(cells_.at(cell).initialized) == true; })) source = read_value(std::move(source), expr.at);
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
    require_access(source.read_only, pointers_.at(*destination.pointer).read_only, at,
        "a writable ptr cannot rebind to a read-only view; use '<-' to copy bits or declare a separate read-only alias");
    for (const auto &loop : loop_pointers_)
      if (*destination.pointer < loop.boundary && !loop.initialized[*destination.pointer])
        fail(at, "loop-carried rebinding of an initially uninitialized target is not yet supported; initialize it before the loop");
  }
  Value rebind_pointer(const Value &destination, Value source) {
    source.pointer.reset();
    source.reference_field.reset();
    source.reference_read_only = false;
    source.binding_read_only = false;
    source.pointer_view = true;
    auto &binding = pointers_.at(*destination.pointer);
    source.read_only = binding.read_only;
    ir_.write_pointer(binding.slot, source.storage ? source.storage->address : source.native);
    binding.view = snapshot_pointer(source);
    source.read_only |= destination.read_only;
    return source;
  }
  Value swap_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                        bool function_context) {
    if (expr.children.size() != 2) fail(expr.at, "swap requires two destinations");
    auto left = expression(*expr.children[0], module, env, locals, function_context, false);
    auto right = expression(*expr.children[1], module, env, locals, function_context, false);
    if (expr.text == "<=>") {
      require_writable(ReadOnly::either(left.binding_read_only, right.binding_read_only), expr.at,
                       "cannot rebind a const binding or alias");
      const auto reload = [&](Value value) {
        if (value.pointer) {
          if (pointers_.at(*value.pointer).fixed_binding) fail(expr.at, "cannot rebind a const ptr binding");
          auto current = pointer_value(*value.pointer, expr.at);
          current.read_only |= value.read_only;
          current.binding_read_only |= value.binding_read_only;
          return current;
        }
        if (value.reference_field) return captured_view(std::move(value), expr.at);
        fail(expr.at, "reference swap requires pointer binding destinations");
      };
      left = reload(std::move(left));
      right = reload(std::move(right));
    }
    if (!expr.intrinsic) {
      if (const auto selected = method(expr.text, module, {left, right}, expr.at))
        return call_method(*selected, {left, right}, expr.at, env, function_context, module);
    }
    if (expr.text == "<=>") {
      const auto check = [&](const Value &destination, const Value &source) {
        if (!destination.reference_field) { check_rebind(destination, source, expr.at); return; }
        require_writable(destination.binding_read_only, expr.at, "cannot rebind a const field or a field through a read-only view");
        reference_source(source, *destination.reference_field->type.target, *destination.region_id,
                         destination.required_reference, expr.at, destination.reference_read_only);
      };
      check(left, right);
      check(right, left);
      if (left.reference_field) rebind_field(left, right, expr.at); else rebind_pointer(left, right);
      if (right.reference_field) rebind_field(right, left, expr.at); else rebind_pointer(right, left);
    } else {
      if (!local_bits(left.type) || !local_bits(right.type) || !left.storage || !right.storage)
        fail(expr.at, "builtin bits swap requires two bits storage destinations");
      require_writable(ReadOnly::either(left.read_only, right.read_only), expr.at,
          "cannot swap bits through a read-only view");
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
      if (const auto found = locals.visible.find(expr.text); found != locals.visible.end()) {
        require_runtime_value(found->second, expr.at);
        return found->second.pointer ? *pointers_.at(*found->second.pointer).view : found->second;
      }
      if (const auto found = module.world.global_aliases.find(expr.text); found != module.world.global_aliases.end())
        return module.execution.global_values.at(found->second);
    }
    if (expr.kind == K::symbol) {
      const auto id = expression_symbol(expr, module, locals);
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
      const auto [field, place] = record_field(value, expr.text, module, expr.at, false);
      const bool view = field.declaration_kind == "ref" || field.declaration_kind == "ptr";
      auto result = scalar(view ? *field.type.target : field.type, nullptr, value.region_id);
      result.pointer_view = view;
      return result;
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
            auto value = args[0]; value.type = *value.type.target;
            if (value.region_id) value.result_type_origin = regions_.at(*value.region_id).root_origin;
            return value;
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
    require_argument_reads(index);
    if (index.storage && !index.native)
      fail(at, "read of storage before definite initialization in an index");
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
        if (!numeric(type) && type != Type::boolean && type.kind != Type::quantity && type.kind != Type::procedure)
          fail(at, "an empty argument pack has no regional owner to borrow");
        auto result = scalar(type, ir_.argument_element(type, index.native, {}));
        if (type.kind == Type::procedure) {
          callable_flows_.push_back({});
          result.callable_flow = callable_flows_.size() - 1;
          // The bounds guard makes this path unreachable. A local placeholder
          // still permits ordinary view checking in a zero-iteration loop.
          result.storage = StorageView{result.native, local_scope_, {}};
          result.callable_address = result.callable_flow;
          result.pointer_view = array.borrowed_arguments;
        } else if (array.borrowed_arguments) {
          result.storage = StorageView{llvm::PoisonValue::get(ir_.null_pointer()->getType()), local_scope_, {}, true};
          result.pointer_view = true;
        }
        return result;
      }
      auto result = elements.front();
      if (native_callable(result)) {
        std::vector<llvm::Value *> slots;
        std::vector<size_t> addresses;
        for (const auto &element : elements) {
          valid_view(element, at);
          require(element.type, result.type, at);
          if (!element.storage || !element.callable_address || storage_owner(element) != storage_owner(result))
            fail(at, "a dynamic callable argument index requires one live storage owner");
          slots.push_back(element.storage->address);
          addresses.push_back(element.callable_address);
          result.read_only |= element.read_only;
          result.nullable |= element.nullable;
        }
        result.storage->address = ir_.argument_element(Type::null_ref, index.native, slots);
        result.callable_address = join_callable_flow(std::move(addresses));
        result.pointer.reset();
        result.callable_temporary = false;
        if (array.borrowed_arguments) {
          result.native = nullptr;
          result.pointer_view = true;
          return result;
        }
        return read_value(std::move(result), at);
      }
      if (result.procedure) fail(at, "a dynamic argument index cannot select a procedure");
      std::vector<llvm::Value *> native, categories;
      auto facts = result.type.kind == Type::exception ? std::make_shared<ExceptionPossibilities>() : nullptr;
      result.pointer.reset(); result.reference_field.reset(); result.referent = nullptr;
      result.nullable = false; result.read_only = false;
      if (std::any_of(elements.begin(), elements.end(), [](const Value &value) {
            return !value.result_fact_origins.empty();
          })) result.conformances.clear();
      if (result.storage) { result.storage->cells.clear(); result.storage->initialized_alternatives = false; }
      for (const auto &stored : elements) {
        const auto element = array.borrowed_arguments ? stored : read_value(stored, at);
        valid_view(element, at);
        if (facts) merge_exception_possibilities(*facts, element);
        require(element.type, result.type, at);
        if (storage_owner(element) != storage_owner(result) || bool(element.storage) != bool(result.storage))
          fail(at, "a dynamic argument index requires one live storage owner");
        result.nullable |= element.nullable;
        result.read_only |= element.read_only;
        std::erase_if(result.conformances, [&](const Conformance &relation) {
          return !establishes(element, relation, at);
        });
        if (array.borrowed_arguments && element.storage) {
          native.push_back(element.storage->address);
          result.storage->cells.insert(element.storage->cells.begin(), element.storage->cells.end());
          result.storage->initialized_alternatives |= element.storage->initialized_alternatives;
        } else {
          native.push_back(element.native);
          if (result.type.kind == Type::exception) categories.push_back(exception_category(element));
        }
      }
      if (array.borrowed_arguments && result.storage) {
        result.storage->address = ir_.argument_element(Type::null_ref, index.native, native);
        result.native = nullptr;
      } else result.native = ir_.argument_element(result.type, index.native, native);
      if (!categories.empty()) result.exception_category = ir_.argument_element(Type::uint64, index.native, categories);
      result.exception_facts = std::move(facts);
      result.pointer_view = true;
      if (!array.borrowed_arguments && local_bits(result.type)) result = bits_value(std::move(result));
      join_result_facts(result, elements);
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
  bool resolve_native_calls(size_t begin) {
    bool changed = false;
    for (size_t i = begin; i < native_call_demands_.size(); ++i)
      changed |= compile_native_call(native_call_demands_[i]);
    return changed;
  }
  void record_callable_effects(const std::map<size_t, size_t> &outputs) {
    for (const auto &[holder, output] : outputs)
      if (const auto *value = callable_holders_.lookup(holder)) {
        auto &inputs = callable_flows_.at(output).inputs;
        if (std::find(inputs.begin(), inputs.end(), *value) == inputs.end()) inputs.push_back(*value);
      }
  }
  CallableMemory callable_effects(const NativeCallDemand &demand, const std::map<size_t, size_t> &outputs) {
    auto memory = demand.state.callables;
    for (const auto &[holder, output] : outputs)
      memory = callable_memory_factory_.add(memory, holder, output);
    return memory;
  }
  void prepare_initialization(NativeCallDemand &demand) {
    for (size_t i = 0; i < demand.arguments.size(); ++i) {
      auto &argument = demand.arguments[i];
      if (!borrowed_parameter(demand.type, i)) continue;
      argument.argument_reads.clear();
      if (!argument.storage) continue;
      for (size_t cell : argument.storage->cells) {
        if (initialization_known(demand.state.initialized.at(cell)) == true || demand.initialized.contains(cell)) continue;
        const auto create = [&] {
          initialization_flows_.push_back({{}, true});
          return initialization_flows_.size() - 1;
        };
        demand.initialized.emplace(cell, create());
        for (auto &failure : demand.failure_initialized) failure.emplace(cell, create());
      }
    }
  }
  void record_initialization(const std::map<size_t, size_t> &outputs) {
    for (const auto &[cell, id] : outputs) {
      auto &inputs = initialization_flows_.at(id).inputs;
      const auto proof = cells_.at(cell).initialized;
      if (std::find(inputs.begin(), inputs.end(), proof) == inputs.end()) inputs.push_back(proof);
    }
  }
  void apply_initialization(const std::map<size_t, size_t> &outputs) {
    for (const auto &[cell, proof] : outputs)
      cells_.at(cell).initialized = preserved_initialization(cells_.at(cell).initialized, proof);
  }
  void reuse_initialization(NativeCallDemand &to, const NativeCallDemand &from) {
    const auto connect = [&](const auto &target, const auto &source) {
      for (const auto &[cell, id] : target) {
        auto &node = initialization_flows_.at(id);
        node.inputs = {source.contains(cell) ? source.at(cell) : from.state.initialized.at(cell)};
        node.open = false;
      }
    };
    connect(to.initialized, from.initialized);
    for (size_t i = 0; i < to.failure_initialized.size(); ++i)
      connect(to.failure_initialized[i], from.failure_initialized[i]);
  }
  void prepare_callable_effects(NativeCallDemand &demand) {
    std::set<size_t> holders;
    const auto collect = [&](const Value &value) {
      if (!value.storage || !value.callable_address || value.read_only) return;
      const auto targets = callable_targets(value.callable_address);
      if (!targets.closed) {
        for (const auto &[holder, flow] : demand.state.callables) holders.insert(holder);
        return;
      }
      for (const auto holder : targets.holders)
        if (demand.state.callables.lookup(holder)) holders.insert(holder);
    };
    if (!demand.captured)
      for (const auto &[id, value] : demand.environment.bindings) collect(*value);
    for (size_t i = 0; i < demand.arguments.size(); ++i)
      if (borrowed_parameter(demand.type, i) &&
          (demand.type.const_views.empty() || !demand.type.const_views[i])) collect(demand.arguments[i]);
    for (size_t holder : holders) {
      const auto create = [&] {
        callable_flows_.push_back({{}, {}, true});
        callable_flows_.back().producer = &demand;
        return callable_flows_.size() - 1;
      };
      demand.returns.emplace(holder, create());
      for (auto &failure : demand.failures) failure.emplace(holder, create());
    }
  }
  bool close_callable_effects(NativeCallDemand &demand) {
    if (demand.selection) return false;
    bool changed = false;
    const auto choices = callable_choices(demand.flow);
    if (choices.closed && (demand.reused || std::all_of(choices.definitions.begin(), choices.definitions.end(),
        [&](const auto &source) { return demand.emitted.contains(source.get()); }))) {
      const auto close = [&](const auto &outputs) {
        for (const auto &[cell, id] : outputs) {
          changed |= initialization_flows_.at(id).open;
          initialization_flows_.at(id).open = false;
        }
      };
      close(demand.initialized);
      for (const auto &failure : demand.failure_initialized) close(failure);
    }
    const auto close_flow = [&](size_t output) {
      changed |= callable_flows_.at(output).open;
      callable_flows_.at(output).open = false;
    };
    const auto close = [&](const auto &outputs) {
      for (const auto &[holder, output] : outputs) close_flow(output);
    };
    close(demand.returns);
    if (demand.result_flow) close_flow(demand.result_flow);
    for (const auto &failure : demand.failures) close(failure);
    return changed;
  }
  bool active_native_call(const NativeCallDemand &demand) const {
    return checking_native_calls_.contains(&demand) ||
        std::any_of(native_activations_.begin(), native_activations_.end(),
        [&](const auto &active) { return active.call == &demand; });
  }
  bool resolve_native_flow(size_t flow, std::set<NativeCallDemand *> &resolving) {
    bool progressed = false;
    for (;;) {
      std::set<size_t> unresolved;
      if (callable_choices(flow, &unresolved).closed) return progressed;
      bool changed = false;
      for (const auto id : unresolved) {
        auto *demand = callable_flows_.at(id).producer;
        if (!demand || active_native_call(*demand) || !resolving.insert(demand).second) continue;
        changed |= resolve_native_flow(demand->flow, resolving);
        changed |= compile_native_call(*demand);
        const auto choices = callable_choices(demand->flow);
        if (choices.closed && (demand->reused || std::all_of(choices.definitions.begin(), choices.definitions.end(),
            [&](const auto &source) { return demand->emitted.contains(source.get()); })))
          changed |= close_callable_effects(*demand);
        resolving.erase(demand);
      }
      if (!changed) return progressed;
      progressed = true;
    }
  }
  CallableChoices bound_callable_choices(const Value &value, const CallableMemory &memory) {
    if (!value.callable_address) return callable_choices(value.callable_flow);
    const auto targets = callable_targets(value.callable_address);
    CallableChoices result; result.closed = targets.closed;
    for (size_t holder : targets.holders) {
      const auto *flow = memory.lookup(holder);
      if (!flow) { result.closed = false; continue; }
      auto choices = callable_choices(*flow);
      result.closed &= choices.closed;
      for (auto &definition : choices.definitions)
        if (std::find(result.definitions.begin(), result.definitions.end(), definition) == result.definitions.end())
          result.definitions.push_back(std::move(definition));
    }
    return result;
  }
  bool same_native_context(const NativeCallDemand &left, const NativeCallDemand &right, bool values,
                              std::map<size_t, size_t> *correspondence = nullptr, bool pending_facts = false,
                              std::vector<std::pair<const Procedure *, const Procedure *>> *images = nullptr,
                              std::set<const CallableCode *> *escaping_images = nullptr) {
    if (left.caller != right.caller || left.captured != right.captured ||
        left.conformances != right.conformances || left.conversion_parameter != right.conversion_parameter) return false;
    bool recursive = false;
    for (auto *parent = right.parent; parent; parent = parent->parent) recursive |= parent == &left;
    std::map<size_t, size_t> owners, reverse_owners, holders, reverse_holders;
    const auto associate = [](auto &forward, auto &reverse, size_t a, size_t b) {
      return forward.emplace(a, b).first->second == b && reverse.emplace(b, a).first->second == a;
    };
    if (left.arguments.size() != right.arguments.size()) return false;
    std::map<size_t, size_t> regional_owners, reverse_regional_owners;
    const auto identity = [&](size_t owner) {
      try { return region_identity(owner, right.at); }
      catch (const UnresolvedAccess &) { if (!pending_facts || !recursive) throw; return owner; }
    };
    const auto fixed = [&](size_t owner) {
      owner = identity(owner);
      regional_owners.emplace(owner, owner);
      reverse_regional_owners.emplace(owner, owner);
    };
    const auto retain = [&](const Value &value) {
      if (value.region_id) fixed(*value.region_id);
      for (size_t owner : value.type.capture_regions) fixed(owner);
    };
    for (const auto *demand : {&left, &right}) {
      for (const auto &[id, value] : demand->environment.bindings) retain(*value);
      for (const auto &source : callable_choices(demand->flow).definitions) if (source->code) {
        for (size_t owner : source->code->regions) fixed(owner);
        if (demand->captured) for (size_t owner : source->code->dynamic_regions) fixed(owner);
      }
    }
    for (size_t i = 0; i < left.arguments.size(); ++i) if (left.type.parameters[i].owned) {
      const auto a = identity(*left.arguments[i].region_id);
      auto b = identity(*right.arguments[i].region_id);
      if (pending_facts && recursive && regional_owners.contains(a) && regional_owners.at(a) != b &&
          possible_region_alias(*right.arguments[i].region_id, *left.arguments[i].region_id)) b = a;
      if (!associate(regional_owners, reverse_regional_owners, a, b)) return false;
    }
    const auto same_region = [&](size_t first, size_t second) {
      const auto a = identity(first), b = identity(second);
      const auto found = regional_owners.find(a);
      if ((found == regional_owners.end() ? a : found->second) == b) {
        const auto reverse = reverse_regional_owners.find(b);
        return reverse == reverse_regional_owners.end() || reverse->second == a;
      }
      return pending_facts && recursive &&
          (possible_region_alias(first, second) || possible_region_alias(second, first));
    };
    const auto mapped_type = [&](auto &&self, Type type) -> Type {
      const auto mapped_owner = [&](size_t owner) {
        owner = identity(owner);
        const auto found = regional_owners.find(owner);
        return found == regional_owners.end() ? owner : found->second;
      };
      for (auto &owner : type.capture_regions) owner = mapped_owner(owner);
      std::sort(type.capture_regions.begin(), type.capture_regions.end());
      for (auto &owner : type.capture_owner_regions) if (owner) owner = mapped_owner(*owner);
      for (auto &parameter : type.parameters) parameter = self(self, parameter);
      if (type.target) type.target = std::make_shared<Type>(self(self, *type.target));
      return type;
    };
    if (mapped_type(mapped_type, left.type) != right.type) return false;
    std::vector<std::pair<const Procedure *, const Procedure *>> matched_images;
    std::set<const CallableCode *> changed_inputs, escaping;
    const auto pair_images = [&](const CallableChoices &a, const CallableChoices &b, const auto &matches) {
      if (!a.closed || !b.closed || a.definitions.size() != b.definitions.size()) return false;
      std::set<const Procedure *> selected;
      for (const auto &first : a.definitions) {
        bool found = false;
        for (const auto &second : b.definitions) {
          if (selected.contains(second.get())) continue;
          const size_t checkpoint = matched_images.size();
          if (matches(*first, *second)) { selected.insert(second.get()); found = true; break; }
          matched_images.resize(checkpoint);
        }
        if (!found) return false;
      }
      return true;
    };
    // Contextual images are interpreted by this caller's adapters. Compare
    // their complete typed schema under the declared owner correspondence;
    // runtime field contents may differ, but metadata and slot order may not.
    const auto same_image = [&](auto &&self, const Procedure &a, const Procedure &b) -> bool {
      if (&a == &b) return true;
      if (!a.code || !b.code || a.form != b.form || a.lexical != b.lexical ||
          a.implementation != b.implementation || a.declarations != b.declarations) return false;
      const auto pair = std::pair{&a, &b};
      if (std::find(matched_images.begin(), matched_images.end(), pair) != matched_images.end()) return true;
      const auto &ac = *a.code, &bc = *b.code;
      if (ac.direct != bc.direct || ac.capture_fields.size() != bc.capture_fields.size()) return false;
      for (size_t i = 0; i < ac.capture_fields.size(); ++i)
        if (ac.capture_fields[i].first != bc.capture_fields[i].first) return false;
      if (ac.layout->elements() != bc.layout->elements() || ac.nested != bc.nested ||
          ac.dynamic_values != bc.dynamic_values || ac.dynamic_procedures != bc.dynamic_procedures ||
          bool(ac.result_flow) != bool(bc.result_flow) || ac.regions.size() != bc.regions.size() ||
          ac.dynamic_regions.size() != bc.dynamic_regions.size() ||
          a.lexical_captures.regional_slots != b.lexical_captures.regional_slots ||
          a.captures.replay != b.captures.replay) return false;
      for (size_t i = 0; i < ac.regions.size(); ++i) {
        if (!same_region(ac.regions[i], bc.regions[i])) return false;
        resolve_region_root(ac.regions[i], left.at); resolve_region_root(bc.regions[i], right.at);
        if (regions_.at(ac.regions[i]).root != regions_.at(bc.regions[i]).root) return false;
      }
      for (size_t i = 0; i < ac.dynamic_regions.size(); ++i)
        if (!same_region(ac.dynamic_regions[i], bc.dynamic_regions[i])) return false;
      matched_images.push_back(pair);
      const auto same_value = [&](const Value &x, const Value &y) {
        if (mapped_type(mapped_type, x.type) != y.type || x.read_only != y.read_only ||
            x.nullable != y.nullable || x.pending_owner != y.pending_owner ||
            x.pointer_view != y.pointer_view || x.required_reference != y.required_reference ||
            x.binding_read_only != y.binding_read_only || x.reference_read_only != y.reference_read_only || x.replayed != y.replayed ||
            bool(x.native) != bool(y.native) || bool(x.storage) != bool(y.storage) ||
            bool(x.reference_field) != bool(y.reference_field) ||
            bool(x.exception_category) != bool(y.exception_category) ||
            bool(x.region_id) != bool(y.region_id) || x.holding_id || y.holding_id ||
            !x.argument_reads.empty() || !y.argument_reads.empty()) return false;
        if (x.region_id && !same_region(*x.region_id, *y.region_id)) return false;
        if (x.storage && (x.storage->scope || y.storage->scope ||
            x.storage->cells != y.storage->cells ||
            x.storage->initialized_alternatives != y.storage->initialized_alternatives)) return false;
        auto xc = x, yc = y;
        resolve_result_facts(xc, left.at); resolve_result_facts(yc, right.at);
        if (xc.conformances != yc.conformances) return false;
        if (x.type.kind != Type::procedure) return true;
        // Symbolic snapshots are demanded by checked bodies. An unused
        // snapshot may contain an arbitrarily old enclosing image; it supplies
        // no field to execution. Reuse is rechecked after capture demand closes.
        if (x.capture_use && !x.capture_use->used) return true;
        const auto xs = callable_choices(x.callable_flow), ys = callable_choices(y.callable_flow);
        if (!xs.closed || !ys.closed || xs.definitions.size() != ys.definitions.size()) return false;
        if (xs.definitions.empty()) return x.procedure == y.procedure;
        return pair_images(xs, ys, [&](const auto &u, const auto &v) { return self(self, u, v); });
      };
      const auto same_locals = [&](const auto &x, const auto &y) {
        if (x.size() != y.size()) return false;
        auto other = y.begin();
        for (const auto &[name, value] : x) {
          if (name != other->first || !same_value(value, other->second)) return false;
          ++other;
        }
        return true;
      };
      if (!same_locals(a.lexical_captures.visible, b.lexical_captures.visible) ||
          !same_locals(a.lexical_captures.owner_anchors, b.lexical_captures.owner_anchors)) return false;
      auto other = b.captures.bindings.begin();
      for (const auto &[id, value] : a.captures.bindings) {
        if (other == b.captures.bindings.end() || id != other->first ||
            a.captures.captured(id, value) != b.captures.captured(id, other->second) ||
            !same_value(*value, *other->second)) return false;
        ++other;
      }
      if (other != b.captures.bindings.end()) return false;
      if (bool(ac.entry.function) != bool(bc.entry.function)) return false;
      if (ac.entry.function && (ac.result_flow || !ac.entry.function->empty() || !bc.entry.function->empty())) {
        // Reuse must emit an entry reached by its peer, without demanding a
        // returned leaf body that neither checked caller ever invokes.
        compile_callable(a, *left.caller, left.at);
        compile_callable(b, *right.caller, right.at);
        if (!ac.entry.complete || !bc.entry.complete || ac.entry.returns != bc.entry.returns ||
            ac.entry.failures != bc.entry.failures) return false;
      }
      if (!ac.result_flow) return true;
      // A factory's returned images are part of its checked contract. A native
      // entry fixes that result graph independently of this caller's adapter.
      if (!ac.entry.function || !bc.entry.function) return false;
      return pair_images(callable_choices(ac.result_flow), callable_choices(bc.result_flow),
          [&](const auto &x, const auto &y) { return self(self, x, y); });
    };
    const auto preserved_outputs = [&](const NativeCallDemand &demand) {
      const auto preserved = [&](const auto &outputs) {
        for (const auto &[holder, flow] : outputs) {
          const auto *input = demand.state.callables.lookup(holder);
          if (!input) return false;
          const auto initial = callable_choices(*input), returned = callable_choices(flow);
          if ((!initial.closed || !returned.closed) && !active_native_call(demand)) return false;
          for (const auto &source : returned.definitions)
            if (std::find(initial.definitions.begin(), initial.definitions.end(), source) == initial.definitions.end())
              return false;
        }
        return true;
      };
      if (!preserved(demand.returns)) return false;
      return std::all_of(demand.failures.begin(), demand.failures.end(), preserved);
    };
    const auto same_callback = [&](const Value &a, const Value &b,
                                     const CallableChoices &first, const CallableChoices &second) {
      // Non-contextual function images carry their own typed entry. Their
      // bodies/layouts may differ; projection and failure facts must still
      // agree in this caller after formal owner substitution.
      const auto ac = closure_regions(a), bc = closure_regions(b);
      if (ac.size() != bc.size()) return false;
      for (const auto &[owner, access] : ac) {
        const auto other = std::find_if(bc.begin(), bc.end(), [&](const auto &item) {
          return same_region(owner, item.first);
        });
        if (other == bc.end() || access.read_only != other->second.read_only) return false;
        resolve_region_root(owner, left.at);
        resolve_region_root(other->first, right.at);
        auto first_access = access, second_access = other->second;
        resolve_result_facts(first_access, left.at);
        resolve_result_facts(second_access, right.at);
        if (regions_.at(owner).root != regions_.at(other->first).root ||
            first_access.conformances != second_access.conformances) return false;
      }
      const auto contract = [&](const CallableChoices &choices, const NativeCallDemand &demand) {
        std::pair<bool, std::set<size_t>> result;
        for (const auto &source : choices.definitions) {
          if (!source->code || !source->code->entry.function) return std::optional<decltype(result)>{};
          compile_callable(*source, *demand.caller, demand.at);
          const auto &entry = source->code->entry;
          if (!entry.complete) return std::optional<decltype(result)>{};
          result.first |= entry.returns;
          result.second.insert(entry.failures.begin(), entry.failures.end());
        }
        return std::optional(result);
      };
      const auto expected = contract(first, left), actual = contract(second, right);
      if (!expected || !actual || expected != actual) return false;
      if (a.type.target->kind != Type::procedure) return true;
      const auto results = [&](const CallableChoices &sources) {
        CallableChoices result;
        for (const auto &source : sources.definitions) {
          const auto choices = callable_choices(source->code->result_flow);
          result.closed &= choices.closed;
          for (const auto &definition : choices.definitions)
            if (std::find(result.definitions.begin(), result.definitions.end(), definition) == result.definitions.end())
              result.definitions.push_back(definition);
        }
        return result;
      };
      return pair_images(results(first), results(second),
          [&](const auto &x, const auto &y) { return same_image(same_image, x, y); });
    };
    for (size_t i = 0; i < left.arguments.size(); ++i) {
      const auto &first = left.arguments[i], &second = right.arguments[i];
      // Recursive return facts can depend on this very adapter. Its reuse is
      // provisional until the closed facts pass the final context comparison.
      const bool same_facts = first.conformances == second.conformances ||
          (pending_facts && recursive &&
           (!first.result_fact_origins.empty() || !second.result_fact_origins.empty()));
      if (first.type.kind == Type::region) {
        const bool same_owner = first.region_id && second.region_id && same_region(*first.region_id, *second.region_id);
        bool same_access;
        try { same_access = first.read_only == second.read_only; }
        catch (const UnresolvedAccess &) {
          if (!pending_facts || !recursive) throw;
          same_access = true;
        }
        if (first.type != second.type || !same_owner || !same_access || !same_facts) return false;
      }
      if (borrowed_parameter(left.type, i)) {
        const bool same_storage = first.region_id && second.region_id
            ? same_region(*first.region_id, *second.region_id) : storage_owner(first) == storage_owner(second);
        if (first.type != second.type || !same_storage ||
            first.read_only != second.read_only || !same_facts ||
            first.nullable != second.nullable || first.pending_owner != second.pending_owner ||
            bool(first.storage) != bool(second.storage)) return false;
        if (first.storage) {
          if (first.storage->cells != second.storage->cells ||
              first.storage->initialized_alternatives != second.storage->initialized_alternatives) return false;
          // These same live allocations cannot become uninitialized along a
          // recursive call path. Sibling calls have no such entry invariant.
          if (!recursive) for (size_t cell : first.storage->cells)
            if (left.state.initialized.at(cell) != right.state.initialized.at(cell)) return false;
        }
        if (native_callable(first)) {
          const auto a = callable_targets(first.callable_address), b = callable_targets(second.callable_address);
          if (!a.closed || !b.closed || a.holders != b.holders) return false;
          for (size_t holder : a.holders)
            if (!associate(holders, reverse_holders, holder, holder)) return false;
        }
      }
      const auto a = borrowed_parameter(left.type, i) ? bound_callable_choices(first, left.state.callables) :
          callable_choices(first.callable_flow);
      const auto b = borrowed_parameter(right.type, i) ? bound_callable_choices(second, right.state.callables) :
          callable_choices(second.callable_flow);
      if (!a.closed || !b.closed) {
        if (values) return false;
      } else if (!std::is_permutation(a.definitions.begin(), a.definitions.end(),
          b.definitions.begin(), b.definitions.end())) {
        // Function images carry their own entry; contextual images need a
        // matching checked schema and caller-local dispatch adaptation.
        if (!preserved_outputs(left) || !preserved_outputs(right)) return false;
        for (const auto *choices : {&a, &b})
          for (const auto &source : choices->definitions) changed_inputs.insert(source->code.get());
        if (contextual_callable(first.type)) {
          if (!pair_images(a, b, [&](const auto &x, const auto &y) { return same_image(same_image, x, y); }))
            return false;
        } else {
          if (!same_callback(first, second, a, b)) return false;
          if (left.result_flow || right.result_flow) {
            const size_t checkpoint = matched_images.size();
            if (!pair_images(a, b, [&](const auto &x, const auto &y) { return same_image(same_image, x, y); }))
              matched_images.resize(checkpoint);
          }
        }
      }
    }
    if (!changed_inputs.empty() && (left.result_flow || right.result_flow)) {
      // A returned environment must not retain substituted scratch owners.
      // Inspect complete nested values and demanded dynamic captures; forwarded
      // equivalent inputs also require dispatch support after this call exits.
      const auto independent = [&](const NativeCallDemand &demand) {
        std::set<const Procedure *> visited;
        const auto check = [&](auto &&self, size_t flow) -> bool {
          const auto choices = callable_choices(flow);
          if (!choices.closed && !active_native_call(demand)) return false;
          for (const auto &source : choices.definitions) {
            if (!visited.insert(source.get()).second) continue;
            if (!source->code) return false;
            // A forwarded image, including a nested environment, retains its
            // key after this caller exits. Its schema must match and all owners
            // must remain unchanged below before downstream calls accept it.
            const bool matched = std::any_of(matched_images.begin(), matched_images.end(), [&](const auto &pair) {
              return pair.first->code == source->code || pair.second->code == source->code;
            });
            if (changed_inputs.contains(source->code.get()) && !matched) return false;
            if (matched) escaping.insert(source->code.get());
            const auto &code = *source->code;
            for (size_t owner : code.regions) if (!same_region(owner, owner)) return false;
            for (size_t owner : code.dynamic_regions) if (!same_region(owner, owner)) return false;
            const auto value = [&](const Value &capture) {
              if (mapped_type(mapped_type, capture.type) != capture.type ||
                  (capture.region_id && !same_region(*capture.region_id, *capture.region_id))) return false;
              if (capture.type.kind != Type::procedure ||
                  (capture.capture_use && !capture.capture_use->used)) return true;
              if (native_callable(capture)) return self(self, capture.callable_flow);
              return capture.procedure && !capture.procedure->builtin.empty();
            };
            for (const auto &[name, capture] : source->lexical_captures.visible)
              if (!value(capture)) return false;
            for (const auto &[name, capture] : source->lexical_captures.owner_anchors)
              if (!value(capture)) return false;
            for (const auto &[id, capture] : source->captures.bindings)
              if (!value(*capture)) return false;
          }
          return true;
        };
        return check(check, demand.result_flow);
      };
      if (!independent(left) || !independent(right)) return false;
    }
    const auto image_context = [](const Environment &environment) {
      for (auto *current = &environment; current; current = current->replay ? &current->replay->caller : nullptr)
        for (const auto &[id, value] : current->bindings)
          if (value->region_id || value->type.kind == Type::region || object_type(value->type) ||
              !value->type.capture_regions.empty()) return false;
      return true;
    };
    // A perform adapter obtains its bits/procedure captures from the selected
    // image on each invocation. Caller values are irrelevant to that body;
    // regional overlays still require the caller-dependent owner proof below.
    if (left.captured && image_context(left.environment) && image_context(right.environment)) {
      if (correspondence) *correspondence = std::move(holders);
      if (images) *images = std::move(matched_images);
      if (escaping_images) *escaping_images = std::move(escaping);
      return true;
    }
    if (left.environment.captured_context != right.environment.captured_context) return false;
    if (values) {
      if (left.environment.replay != right.environment.replay) return false;
      // A recursive frame may construct local callable values. It cannot
      // reuse this context if any pre-existing holder has changed or expired.
      for (const auto &[holder, flow] : left.state.callables) {
        const auto *current = right.state.callables.lookup(holder);
        if (!current) return false;
        if (*current != flow) {
          const auto a = callable_choices(flow), b = callable_choices(*current);
          if (!a.closed || !b.closed || !std::is_permutation(a.definitions.begin(), a.definitions.end(),
              b.definitions.begin(), b.definitions.end())) return false;
        }
      }
    }
    auto l = left.environment.bindings.begin(), r = right.environment.bindings.begin();
    for (; l != left.environment.bindings.end() && r != right.environment.bindings.end(); ++l, ++r) {
      if (l->first != r->first) return false;
      const auto *a = l->second, *b = r->second;
      if (a->type != b->type || left.environment.captured(l->first, a) !=
          right.environment.captured(r->first, b)) return false;
      if (native_callable(*a) && native_callable(*b) &&
          a->storage && b->storage && !left.environment.captured(l->first, a)) {
        if (a->read_only != b->read_only || a->nullable != b->nullable ||
            a->pointer_view != b->pointer_view || a->binding_read_only != b->binding_read_only ||
            a->required_reference != b->required_reference || a->reference_read_only != b->reference_read_only ||
            !a->storage->scope || !b->storage->scope) return false;
        if (values && (*b->storage->scope >= right.scopes.size() || !right.scopes[*b->storage->scope])) return false;
        const auto ac = bound_callable_choices(*a, left.state.callables);
        const auto bc = bound_callable_choices(*b, right.state.callables);
        if (!ac.closed || !bc.closed) {
          if (values) return false;
        } else if (!std::is_permutation(ac.definitions.begin(), ac.definitions.end(),
            bc.definitions.begin(), bc.definitions.end(), [values](const auto &x, const auto &y) {
              return values ? x == y : x->form == y->form && x->lexical == y->lexical && x->implementation == y->implementation;
            })) return false;
        const auto at = callable_targets(a->callable_address), bt = callable_targets(b->callable_address);
        if (!at.closed || !bt.closed || at.holders.size() != 1 || bt.holders.size() != 1) {
          if (values) {
            if (!at.closed || !bt.closed || a != b) return false;
            for (size_t holder : at.holders)
              if (!associate(holders, reverse_holders, holder, holder)) return false;
          }
        } else if (!associate(holders, reverse_holders, *at.holders.begin(), *bt.holders.begin())) return false;
        if (!associate(owners, reverse_owners, *a->storage->scope, *b->storage->scope)) return false;
        continue;
      }
      if (!values || a == b) continue;
      // Live numeric effects are read from TLS in every activation. Captured
      // values and other resources must retain the same checked provenance.
      if (!local_bits(a->type) || a->type.kind == Type::exception ||
          left.environment.captured(l->first, a) || right.environment.captured(r->first, b)) return false;
    }
    if (l != left.environment.bindings.end() || r != right.environment.bindings.end()) return false;
    if (correspondence) *correspondence = std::move(holders);
    if (images) *images = std::move(matched_images);
    if (escaping_images) *escaping_images = std::move(escaping);
    return true;
  }
  bool refine_native_reuse() {
    bool changed = false;
    const auto compiled = callable_compilations_;
    for (size_t i = 0; i < native_call_demands_.size(); ++i) {
      auto &demand = native_call_demands_[i];
      if (!demand.reused) continue;
      for (auto &argument : demand.arguments) resolve_result_facts(argument, demand.at);
      std::vector<std::pair<const Procedure *, const Procedure *>> images;
      if (same_native_context(*demand.reused, demand, true, nullptr, false, &images)) {
        for (const auto &[first, second] : images)
          for (size_t j = 0; j < first->code->capture_fields.size(); ++j) {
            const auto &a = first->code->capture_fields[j].second, &b = second->code->capture_fields[j].second;
            if (a->used && !b->used) { b->require(); changed = true; }
          }
        continue;
      }
      // The recursive hypothesis has supplied enough return information to
      // check this distinct context. Its IR declaration has not been replaced.
      demand.reused = nullptr;
      demand.pending_argument_facts = false;
      const auto reopen = [&](const auto &outputs) {
        for (const auto &[holder, id] : outputs) {
          auto &flow = callable_flows_.at(id);
          flow.inputs.clear(); flow.open = true;
        }
      };
      reopen(demand.returns);
      for (const auto &failure : demand.failures) reopen(failure);
      if (demand.result_flow) {
        auto &flow = callable_flows_.at(demand.result_flow);
        flow.inputs.clear(); flow.open = true;
      }
      const auto reopen_initialization = [&](const auto &outputs) {
        for (const auto &[cell, id] : outputs) {
          auto &flow = initialization_flows_.at(id);
          flow.inputs.clear(); flow.open = true;
        }
      };
      reopen_initialization(demand.initialized);
      for (const auto &failure : demand.failure_initialized) reopen_initialization(failure);
      changed = true;
    }
    return changed || compiled != callable_compilations_;
  }
  bool compile_native_call(NativeCallDemand &demand) {
    if (demand.reused || active_native_call(demand)) return false;
    checking_native_calls_.insert(&demand);
    const auto checking = llvm::scope_exit([&] { checking_native_calls_.erase(&demand); });
    if (demand.selection && !select_native_method(demand)) return false;
    const auto choices = callable_choices(demand.flow);
    if (choices.definitions.empty() || std::all_of(choices.definitions.begin(), choices.definitions.end(),
        [&](const auto &source) { return demand.emitted.contains(source.get()); })) return false;
    bool pending_facts = demand.pending_argument_facts;
    if (!pending_facts) for (auto &argument : demand.arguments) {
      try {
        if (argument.result_type_origin &&
            (argument.type.kind == Type::region || region_definition(argument.type)))
          resolve_result_type(argument, demand.at, true);
        resolve_result_facts(argument, demand.at);
      }
      catch (const UnresolvedAccess &) { pending_facts = true; }
    }
    // Repeated calls in one native caller can share a checked record adapter.
    // Its lexical context is rebuilt at each call, not borrowed from its frame.
    if (!pending_facts && demand.type.pure_callable && demand.record_result && demand.emitted.empty() && choices.closed &&
        demand.returns.empty() && std::none_of(demand.arguments.begin(), demand.arguments.end(), native_callable)) {
      for (auto &checked : native_call_demands_) {
        if (&checked == &demand) break;
        if (checked.parent != demand.parent || checked.reused || checked.selection ||
            !checked.definition.complete || active_native_call(checked) || !checked.returns.empty()) continue;
        const auto reaching = callable_choices(checked.flow);
        if (!reaching.closed || !std::is_permutation(choices.definitions.begin(), choices.definitions.end(),
            reaching.definitions.begin(), reaching.definitions.end()) ||
            !std::all_of(reaching.definitions.begin(), reaching.definitions.end(),
                [&](const auto &source) { return checked.emitted.contains(source.get()); })) continue;
        if (!same_native_context(checked, demand, true)) continue;
        demand.reused = &checked;
        reuse_initialization(demand, checked);
        return true;
      }
    }
    auto previous_calls = std::move(call_path_);
    call_path_ = demand.calls;
    std::vector<const NativeCallDemand *> ancestors;
    for (auto *parent = demand.parent; parent; parent = parent->parent) ancestors.push_back(parent);
    std::reverse(ancestors.begin(), ancestors.end());
    for (const auto *ancestor : ancestors) {
      const auto reaching = callable_choices(ancestor->flow);
      if (!std::is_permutation(choices.definitions.begin(), choices.definitions.end(),
                               reaching.definitions.begin(), reaching.definitions.end())) continue;
      if (!demand.emitted.empty()) continue;
      std::map<size_t, size_t> holders;
      if (!same_native_context(*ancestor, demand, true, &holders, pending_facts)) continue;
      const auto connect = [&](const auto &from, const auto &to) {
        for (const auto &[a, b] : holders)
          if (from.contains(a) && to.contains(b)) callable_flows_.at(to.at(b)).inputs = {from.at(a)};
      };
      connect(ancestor->returns, demand.returns);
      if (demand.result_flow) callable_flows_.at(demand.result_flow).inputs = {ancestor->result_flow};
      for (size_t i = 0; i < demand.failures.size(); ++i) connect(ancestor->failures[i], demand.failures[i]);
      demand.reused = ancestor;
      reuse_initialization(demand, *ancestor);
      call_path_ = std::move(previous_calls);
      return true;
    }
    for (const auto *ancestor : ancestors) {
      const auto reaching = callable_choices(ancestor->flow);
      if (std::is_permutation(choices.definitions.begin(), choices.definitions.end(),
          reaching.definitions.begin(), reaching.definitions.end(), [](const auto &a, const auto &b) {
            // A named recursive call can leave an image adapter for its
            // static descriptor once; that is not changing captured state.
            return a->code->direct == b->code->direct &&
                a->form == b->form && a->lexical == b->lexical && a->implementation == b->implementation;
          }) && same_native_context(*ancestor, demand, false)) {
        const auto unresolved = [&](const FlowState &state) {
          return std::any_of(state.callables.begin(), state.callables.end(),
              [&](const auto &entry) { return !callable_choices(entry.second).closed; });
        };
        if (!choices.closed || unresolved(ancestor->state) || unresolved(demand.state)) {
          call_path_ = std::move(previous_calls);
          return false;
        }
        fail(demand.at, "recursive callable adaptation with changing captures or callable storage is not yet implemented");
      }
    }
    demand.pending_argument_facts = pending_facts;
    native_activations_.push_back({nullptr, demand.caller, {}, demand.type.parameters, {}, demand.definition});
    auto &active = native_activations_.back();
    active.call = &demand;
    auto previous_ir = ir_.begin_native(active.definition, demand.tail);
    const auto previous_flow = flow_state();
    const auto previous_scopes = scopes_;
    for (size_t i = 0; i < scopes_.size(); ++i)
      scopes_[i] = i < demand.scopes.size() && demand.scopes[i];
    {
      // Source expansion guards belong to one native frame, not the checker's call stack.
      const auto expansion = llvm::SaveAndRestore(active_calls_, std::set<const Form *>{});
      const auto context = llvm::SaveAndRestore(native_context_, &active);
      const auto handlers = llvm::SaveAndRestore(handlers_, std::vector<HandlerScope *>{});
      const auto checked = llvm::SaveAndRestore(npe_allowed_, !demand.type.throws_types.empty());
      const auto loops = llvm::SaveAndRestore(loop_pointers_, std::vector<LoopPointers>{});
      const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
      const auto conformances = llvm::SaveAndRestore(caller_conformances_, demand.conformances);
      auto *image = active.definition.function->getArg(0);
      for (const auto &source : choices.definitions) {
        if (!demand.emitted.insert(source.get()).second) continue;
        if (!source->code) fail(demand.at, "procedure alternatives require compatible native representations");
        const auto match = ir_.match_callable(image, source->code->dispatch_key);
        demand.matches.emplace(source.get(), match);
        auto *next = match.next;
        const auto capture_scope = ir_.scope();
        restore_flow(demand.state);
        if (demand.conversion_parameter) {
          const auto &parameter = parameter_at(*source->form, *demand.conversion_parameter);
          const auto path = route_to(demand.type.parameters.front(), *demand.type.target, parameter,
                                      *source->lexical, *demand.caller, demand.at);
          if (!path) fail(demand.at, "selected callable body has no applicable argument conversion from " +
              type_name(demand.type.parameters.front()) + " to " + type_name(*demand.type.target) +
              "\n" + location_text(parameter.at) + ": note: parameter declared here");
          auto argument = loaded_value(demand.type.parameters.front(), active.definition.function->getArg(2));
          try {
            auto result = converted(std::move(argument), *path, demand.at, *demand.caller);
            ir_.restore(capture_scope);
            ir_.native_return(stored_value(result, *demand.type.target, demand.at));
          } catch (const EndPath &) { /* The conversion has no successful continuation. */ }
          ir_.enter(next);
          continue;
        }
        auto procedure = *source;
        const auto &code = *source->code;
        size_t index = 0;
        callable_fields(procedure, [&](llvm::Value *&field, bool nested) {
          field = ir_.callable_field(image, code.layout, index++, nested);
        });
        for (size_t id : code.dynamic_values) {
          if (!demand.captured) { ++index; continue; }
          const auto *original = procedure.captures.lookup(id);
          auto value = *original;
          auto loaded = loaded_value(value.type, ir_.callable_field(image, code.layout, index++, false));
          value.native = loaded.native;
          value.exception_category = loaded.exception_category;
          value.replayed = true;
          ir_.effect(id, value.type, value.native);
          binding_values_.push_back(std::move(value));
          procedure.captures.bindings = binding_factory_.add(procedure.captures.bindings, id, &binding_values_.back());
          procedure.captures.captured_bindings = binding_factory_.add(
              procedure.captures.captured_bindings, id, &binding_values_.back());
        }
        for (size_t id : code.dynamic_procedures) {
          if (!demand.captured) { ++index; continue; }
          auto value = *procedure.captures.lookup(id);
          value.native = ir_.callable_field(image, code.layout, index++, true);
          value.replayed = true;
          auto writes = std::make_shared<LLVMEmission::EffectWrites>();
          ir_.captured_procedure(id, value.native, writes);
          capture_effects_.emplace_back(value.capture_use, std::move(writes));
          binding_values_.push_back(std::move(value));
          procedure.captures.bindings = binding_factory_.add(procedure.captures.bindings, id, &binding_values_.back());
          procedure.captures.captured_bindings = binding_factory_.add(
              procedure.captures.captured_bindings, id, &binding_values_.back());
        }
        std::map<size_t, llvm::Value *> owners;
        for (size_t region : code.regions) {
          owners.try_emplace(region, regions_[region].owner);
          regions_[region].live = true;
          regions_[region].owner = ir_.callable_field(image, code.layout, index++, false);
        }
        for (size_t region : code.dynamic_regions) {
          if (!demand.captured) { ++index; continue; }
          owners.try_emplace(region, regions_[region].owner);
          // Captured dynamic resources remain borrowed. A demanded read must
          // still prove this region live in the invocation's flow state.
          regions_[region].owner = ir_.callable_field(image, code.layout, index++, false);
        }
        auto environment = demand.type.pure_callable ? Environment{} :
                           demand.captured ? procedure.captures : demand.environment;
        if (demand.captured) {
          environment.replay = std::make_shared<CaptureReplay>(CaptureReplay{
              demand.environment, procedure.captures.captured_bindings, procedure.captures.replay, {}});
          binding_check(environment, demand.at);
        }
        // Owner bases must already be formal inputs when constructing an
        // interior argument, independently of parameter declaration order.
        for (size_t i = 0; i < demand.type.parameters.size(); ++i) {
          if (demand.type.parameters[i].kind != Type::region) continue;
          const auto &value = demand.arguments[i];
          region_pointer(borrowed(value), demand.at);
          auto *argument = active.definition.function->getArg(
              unsigned(i + 2 + bool(demand.result_flow && !active.definition.borrowed_result)));
          for (const auto &input : demand.arguments) if (input.region_id) {
            bool same = input.region_id == value.region_id;
            if (!same) try {
              same = region_identity(*input.region_id, demand.at) == region_identity(*value.region_id, demand.at);
            } catch (const UnresolvedAccess &) { /* Unresolved aliases retain their final owner proof. */ }
            if (!same) continue;
            owners.try_emplace(*input.region_id, regions_[*input.region_id].owner);
            regions_[*input.region_id].owner = argument;
          }
        }
        std::vector<Value> parameters;
        for (size_t i = 0; i < demand.type.parameters.size(); ++i) {
          auto *argument = active.definition.function->getArg(
              unsigned(i + 2 + bool(demand.result_flow && !active.definition.borrowed_result)));
          if (borrowed_parameter(demand.type, i)) {
            auto value = demand.arguments[i];
            value.pointer.reset(); value.reference_field.reset();
            if (value.storage) { value.storage->address = argument; value.native = nullptr; }
            else value.native = ir_.nullable_offset(region_pointer(value, demand.at), argument);
            parameters.push_back(std::move(value));
          } else if (demand.type.parameters[i].kind == Type::region) {
            auto value = demand.arguments[i];
            value.native = argument;
            if (value.type.owned) value = hold_region(std::move(value), false, true);
            parameters.push_back(std::move(value));
          } else parameters.push_back(native_parameter(demand.type.parameters[i], demand.arguments[i].callable_flow, argument));
        }
        auto result = inline_call(*procedure.form, *procedure.lexical, std::move(environment), parameters,
                                  demand.type.pure_callable, demand.at, *demand.caller,
                                  procedure.implementation, &procedure.lexical_captures);
        check_declarations(procedure.declarations,
                            demand.type.pure_callable ? Form::Kind::function : Form::Kind::procedure, parameters, result,
                            *demand.caller, demand.at, declares_npe(procedure.form->throws_types,
                                                                 *procedure.lexical, procedure.form->at));
        if (result.type != Type::never && demand.record_result)
          return_facts_.at(demand.record_result - 1).alternatives.push_back(result);
        if (result.type != Type::never && demand.result_flow) {
          if (active.definition.borrowed_result) {
            if (!result.callable_address) fail(demand.at, "borrowed callable result requires a live holder view");
            auto &inputs = callable_flows_.at(demand.result_flow).inputs;
            if (std::find(inputs.begin(), inputs.end(), result.callable_address) == inputs.end())
              inputs.push_back(result.callable_address);
          } else return_callable(result, demand.result_flow, active.definition, demand.at);
        }
        if (result.type.kind == Type::region && result.type.owned) result = take_owner(result, demand.at);
        ir_.restore(capture_scope);
        if (result.type != Type::never) {
          record_callable_effects(demand.returns);
          record_initialization(demand.initialized);
          ir_.native_return(active.definition.borrowed_result ? borrowed_operand(result, demand.at) :
              result.type == Type::void_type || demand.result_flow ? nullptr : stored_value(result, result.type, demand.at));
        }
        for (const auto &[region, owner] : owners) regions_[region].owner = owner;
        ir_.enter(next);
      }
      demand.tail = ir_.here();
      ir_.unreachable(); // Additional reaching definitions can extend this dispatch.
    }
    ir_.end_native(active.definition, std::move(previous_ir));
    restore_flow(previous_flow);
    for (size_t i = 0; i < scopes_.size(); ++i)
      scopes_[i] = i < previous_scopes.size() && previous_scopes[i];
    demand.definition = std::move(active.definition);
    native_activations_.pop_back();
    if (demand.context) {
      ir_.resolve_context(demand.context, demand.definition);
      demand.context = nullptr;
    }
    call_path_ = std::move(previous_calls);
    return true;
  }
  bool compile_callable(const Procedure &source, const Module &caller, const Location &at) {
    auto &code = *source.code;
    if (code.entry.complete || !code.entry.function->empty()) return false;
    auto procedure = source;
    const auto type = procedure_type(*source.form, *source.lexical);
    native_activations_.push_back({source.form, source.lexical,
        source.implementation.empty() ? ImplementationContext{source.lexical} : source.implementation,
        type.parameters, {}, code.entry});
    auto &activation = native_activations_.back();
    auto previous_ir = ir_.begin_native(activation.definition);
    const auto call_begin = native_call_demands_.size();
    const auto previous_flow = flow_state();
    std::vector<llvm::Value *> owner_values;
    {
      const auto expansion = llvm::SaveAndRestore(active_calls_, std::set<const Form *>{});
      const auto context = llvm::SaveAndRestore(native_context_, &activation);
      const auto handlers = llvm::SaveAndRestore(handlers_, std::vector<HandlerScope *>{});
      const auto checked = llvm::SaveAndRestore(npe_allowed_, code.entry.checked_null);
      const auto loops = llvm::SaveAndRestore(loop_pointers_, std::vector<LoopPointers>{});
      const auto module_scope = llvm::SaveAndRestore(module_top_level_, false);
      const auto cleanup = ir_.scope();
      size_t index = 0;
      auto *image = code.entry.function->getArg(0);
      callable_fields(procedure, [&](llvm::Value *&field, bool nested) {
        field = ir_.callable_field(image, code.layout, index++, nested);
      });
      for (size_t region : code.regions) {
        owner_values.push_back(regions_[region].owner);
        regions_[region].live = true; // Invocation checks the captured owner's live view.
        regions_[region].owner = ir_.callable_field(image, code.layout, index++, false);
      }
      std::vector<Value> parameters;
      for (size_t i = 0; i < type.parameters.size(); ++i)
        parameters.push_back(loaded_value(type.parameters[i],
            code.entry.function->getArg(unsigned(i + 1 + bool(code.result_flow)))));
      auto result = inline_call(*procedure.form, *procedure.lexical, {}, parameters, true, at, caller,
                                procedure.implementation, &procedure.lexical_captures);
      check_declarations(procedure.declarations, Form::Kind::function, parameters, result, caller, at,
                          code.entry.checked_null);
      activation.definition.returns = result.type != Type::never;
      if (activation.definition.returns && code.result_flow)
        return_callable(result, code.result_flow, activation.definition, at);
      ir_.restore(cleanup);
      if (activation.definition.returns)
        ir_.native_return(result.type == Type::void_type || code.result_flow ? nullptr : stored_value(result, result.type, at));
    }
    resolve_native_calls(call_begin);
    ir_.end_native(activation.definition, std::move(previous_ir));
    for (size_t i = 0; i < code.regions.size(); ++i) regions_[code.regions[i]].owner = owner_values[i];
    restore_flow(previous_flow);
    ir_.require_context_free(activation.definition);
    code.entry = std::move(activation.definition);
    native_activations_.pop_back();
    if (code.result_flow) callable_flows_.at(code.result_flow).open = false;
    ++callable_compilations_;
    return true;
  }
  Value contextual_call(const Value &selected, const std::vector<Value> &args,
                         const std::vector<llvm::Value *> &operands, const Environment &env,
                         const Module &module, const Location &at, bool captured,
                         const std::optional<StorageView> &destination,
                         std::optional<size_t> conversion_parameter = {}) {
    const auto &type = selected.type;
    std::vector<bool> borrowed;
    for (size_t i = 0; i < type.parameters.size(); ++i) borrowed.push_back(borrowed_parameter(type, i));
    auto definition = ir_.native_declaration(type.pure_callable ? "function.context" : "procedure.context",
        type.parameters, *type.target, true, borrowed, borrowed_result(type));
    definition.checked_null = !type.throws_types.empty();
    auto *context = ir_.pending_context();
    auto parameters = args;
    std::set<size_t> transferred, inputs;
    for (size_t i = 0; i < parameters.size(); ++i) {
      valid_view(parameters[i], at);
      // Preserve the supplied callable's owner evidence until admission. The
      // callee's formal type must not overwrite a mismatched environment.
      if (parameters[i].type.kind == Type::procedure || type.parameters[i].kind == Type::procedure)
        parameters[i] = admit(std::move(parameters[i]), type.parameters[i], module, at);
      if (type.parameters[i].owned) {
        if (!parameters[i].holding_id || !transferred.insert(*parameters[i].holding_id).second)
          fail(at, "one region owner cannot be consumed by multiple parameters");
        inputs.insert(*parameters[i].region_id);
      }
    }
    for (size_t i = 0; i < parameters.size(); ++i)
      if (type.parameters[i].owned) parameters[i] = take_owner(std::move(parameters[i]), at);
    native_call_demands_.push_back({selected.callable_flow, &module, at,
        type.pure_callable ? Environment{} : env, captured, type,
        flow_state(), definition, context, caller_conformances_, scopes_, call_path_});
    auto &demand = native_call_demands_.back();
    demand.conversion_parameter = conversion_parameter;
    demand.arguments = parameters;
    for (size_t i = 0; i < demand.arguments.size(); ++i)
      if (!type.const_views.empty() && type.const_views[i] && storage_owner(demand.arguments[i]))
        demand.arguments[i].read_only = true;
    demand.parent = native_context_ ? native_context_->call : nullptr;
    if ((definition.borrowed_result && definition.result.kind == Type::regional) ||
        definition.result.kind == Type::region) {
      return_facts_.push_back({&demand, {}});
      demand.record_result = return_facts_.size();
    }
    prepare_callable_effects(demand);
    prepare_initialization(demand);
    if (destination || (definition.borrowed_result && definition.result.kind == Type::procedure)) {
      callable_flows_.push_back({{}, {}, true});
      demand.result_flow = callable_flows_.size() - 1;
      callable_flows_.back().producer = &demand;
    }
    if (!inputs.empty() && callable_choices(demand.flow).closed) {
      compile_native_call(demand);
      if (!demand.reused) definition = demand.definition;
    }
    // The exceptional caller edge cannot retain a consumed region. Normal
    // return may restore access through a returned owning value of that region.
    for (size_t region : inputs) regions_[region].live = false;
    llvm::Value *returned;
    {
      const auto failures = llvm::SaveAndRestore(pending_callable_failure_,
          static_cast<const NativeCallDemand *>(&demand));
      returned = ir_.native_call(definition, selected.native, operands, nullptr,
                                 destination ? destination->address : nullptr, context);
    }
    callable_holders_ = callable_effects(demand, demand.returns);
    apply_initialization(demand.initialized);
    if (destination) return held_callable(definition.result, demand.result_flow, returned, *destination);
    if (definition.borrowed_result) return borrowed_call_result(type, args, returned, at, demand.result_flow, demand.record_result);
    if (definition.result.kind == Type::region && definition.result.owned) {
      const auto id = new_region(returned, at);
      regions_[id].root = definition.result.target ? std::optional(*definition.result.target) : std::nullopt;
      regions_[id].root_origin = demand.record_result;
      regions_[id].owner_origin = demand.record_result;
      regions_[id].owner_inputs = inputs;
      for (size_t input : inputs) {
        const auto origins = regions_[input].origins;
        regions_[id].origins.insert(origins.begin(), origins.end());
        regions_[input].live = FlowCondition::both(demand.state.live[input], region_alias(id, input));
      }
      auto result = scalar(definition.result, returned, id);
      result.result_type_origin = demand.record_result;
      result.result_fact_origins = {demand.record_result};
      result.read_only = !type.const_views.empty() && type.const_views.back();
      if (!result.read_only) {
        result.read_only = ReadOnly::deferred([this, origin = demand.record_result, at] {
          const auto &results = checked_returns(origin, at);
          return std::any_of(results.begin(), results.end(), [](const Value &value) { return bool(value.read_only); });
        });
        projection_permissions_.push_back(result.read_only);
      }
      return hold_region(std::move(result), module_top_level_, true);
    }
    return definition.result == Type::void_type ? Value{} : loaded_value(definition.result, returned);
  }
  Value callable_conversion(const Value &selected, Value value, Type target, size_t index,
                             const Module &caller, const Location &at, const ConversionPoint &point) {
    const auto state = flow_state();
    const auto resume = ir_.begin_conversion(point.native);
    restore_flow(point.regions);
    Type type{Type::procedure};
    type.pure_callable = true;
    type.parameters = {value.type};
    type.target = std::make_shared<Type>(target);
    if (npe_allowed_) type.throws_types = {exception_type(ExceptionCategory::null_pointer)};
    auto conversion = selected;
    conversion.type = std::move(type);
    auto result = contextual_call(conversion, {value}, {stored_value(value, value.type, at)},
                                   {}, caller, at, false, {}, index);
    ir_.end_conversion(point.native, resume);
    restore_flow(state);
    return result;
  }
  struct PreparedCallableCall {
    Value callee;
    std::vector<Value> arguments;
    bool contextual;
  };
  PreparedCallableCall prepare_callable_call(Value selected, const Expression &expr,
                                               const Module &module, Environment &env,
                                               Locals &locals, bool function_context) {
    if (!native_callable(selected)) {
      const auto proc = selected.procedure; // Resolve before arguments/capture installation.
      std::vector<ConversionPoint> points;
      auto args = arguments(expr, module, env, locals, function_context, proc->form ? &points : nullptr,
          proc->form ? std::vector<Method>{{proc->form, proc->lexical}} : std::vector<Method>{});
      if (!proc->builtin.empty()) {
        if (args.size() != 1) fail(expr.at, "@" + proc->builtin + " expects one argument");
        require_argument_reads(args[0]);
        if (!args[0].native) fail(expr.at, "read of storage before definite initialization in an argument");
        if (selected.type.target) require(args[0].type, selected.type.parameters[0], expr.at);
        if (proc->builtin == "assert") require(args[0].type, Type::boolean, expr.at);
        else if (!numeric(args[0].type) && args[0].type != Type::boolean)
          fail(expr.at, "@print expects a numeric scalar or bool");
        return {std::move(selected), std::move(args), false};
      }
      {
        const auto instantiated = *signature(*proc->form, *proc->lexical, args, module, expr.at, true, &proc->lexical_captures);
        const auto types = llvm::SaveAndRestore(type_context_, TypeContext{proc->lexical, instantiated.bindings});
        for (size_t i = 0; i < args.size(); ++i) {
          if (const auto path = route(args[i].type, parameter_at(*proc->form, i), *proc->lexical, module, expr.at))
            args[i] = converted(args[i], *path, expr.at, module, points.empty() ? nullptr : &points[i],
                !points.empty() && points[i].raw_view && !(parameter_at(*proc->form, i).declaration_kind == "ptr" && !parameter_at(*proc->form, i).owner.empty()),
                parameter_at(*proc->form, i).owner.empty());
        }
      }
      return {std::move(selected), std::move(args), false};
    }
    const bool contextual = contextual_callable(selected.type);
    std::vector<ConversionPoint> points;
    auto args = arguments(expr, module, env, locals, function_context, &points, {}, &selected.type);
    selected.type = instantiate_callable(selected.type, args, module, expr.at);
    for (size_t i = 0; i < args.size(); ++i) {
      const auto target = selected.type.parameters[i];
      if (target.kind == Type::region || borrowed_parameter(selected.type, i))
        args[i] = admit(std::move(args[i]), target, module, expr.at);
      else if (args[i].type.kind == Type::quantity && target.kind == Type::quantity &&
               !widens(args[i].type, target))
        args[i] = callable_conversion(selected, std::move(args[i]), target, i, module, expr.at, points[i]);
      else {
        if (!widens(args[i].type, target)) require(args[i].type, target, expr.at);
        args[i] = converted(std::move(args[i]), {{}, {}, target}, expr.at, module, &points[i]);
      }
    }
    return {std::move(selected), std::move(args), contextual};
  }
  Value invoke_prepared_callable(const PreparedCallableCall &prepared, const Module &module,
                                  const Environment &env, const Location &at, bool captured) {
    const auto &selected = prepared.callee;
    const auto &args = prepared.arguments;
    valid_view(selected, at);
    for (const auto &argument : args) valid_view(argument, at);
    if (!native_callable(selected)) {
      const auto &proc = selected.procedure;
      if (!proc->builtin.empty()) {
        if (proc->builtin == "assert") ir_.assert_value(args[0].native);
        else ir_.print(args[0].type, args[0].native);
        return {};
      }
      auto call_env = captured ? proc->captures : env;
      if (captured) {
        call_env.replay = std::make_shared<CaptureReplay>(CaptureReplay{
            env, proc->captures.captured_bindings, proc->captures.replay, {}});
        binding_check(call_env, at);
      }
      const bool pure = selected.type.pure_callable;
      auto *native_body = ir_.begin_callable();
      auto result = call(*proc->form, *proc->lexical, pure ? Environment{} : std::move(call_env), args, pure, at, module, proc->implementation, &proc->lexical_captures);
      ir_.end_callable(native_body);
      check_declarations(proc->declarations, pure ? Form::Kind::function : Form::Kind::procedure, args, result, module, at, declares_npe(proc->form->throws_types, *proc->lexical, proc->form->at));
      return result;
    }
    std::vector<llvm::Value *> operands;
    for (size_t i = 0; i < args.size(); ++i) {
      const auto target = selected.type.parameters[i];
      if (borrowed_parameter(selected.type, i)) operands.push_back(borrowed_operand(args[i], at));
      else if (!args[i].native) fail(at, "read of storage before definite initialization in a local parameter argument");
      else if (target.kind == Type::procedure) {
        if (!native_callable(args[i]) || !args[i].storage || !args[i].callable_temporary)
          fail(at, "native callable parameter requires a complete prepared value");
        operands.push_back(ir_.take_callable(args[i].storage->address));
      } else operands.push_back(stored_value(args[i], target, at));
    }
    const bool returns_callable = selected.type.target->kind == Type::procedure && !borrowed_result(selected.type);
    std::optional<StorageView> destination;
    if (returns_callable) destination = StorageView{ir_.hold_callable(ir_.null_pointer(), module_top_level_), local_scope_, {}};
    if (prepared.contextual)
      return contextual_call(selected, args, operands, env, module, at, captured, destination);
    auto choices = callable_choices(selected.callable_flow);
    if (choices.closed) {
      for (const auto &candidate : choices.definitions) compile_callable(*candidate, module, at);
    }
    callable_demands_.push_back({selected.callable_flow, &module, at});
    auto signature = ir_.indirect_signature(selected.type.parameters, *selected.type.target, !selected.type.throws_types.empty());
    signature.complete = choices.closed;
    signature.returns = !choices.closed;
    signature.checked_null = !selected.type.throws_types.empty();
    signature.failures.clear();
    for (const auto &candidate : choices.definitions) {
      signature.complete &= candidate->code->entry.complete;
      signature.failures.insert(candidate->code->entry.failures.begin(), candidate->code->entry.failures.end());
      signature.returns |= candidate->code->entry.returns;
    }
    const auto result_flow = destination ? callable_result_flow(selected.callable_flow) : 0;
    resolve_callable_results();
    auto *returned = ir_.native_call(signature, selected.native, operands, ir_.callable_entry(selected.native),
                                     destination ? destination->address : nullptr);
    if (!signature.returns) return scalar(Type::never, nullptr);
    if (destination) return held_callable(signature.result, result_flow, returned, *destination);
    return signature.result == Type::void_type ? Value{} : loaded_value(signature.result, returned);
  }
  static bool contains_deferred(const Expression &expr, bool enter_scope = false) {
    if (expr.kind == Expression::Kind::deferred) return true;
    if (expr.scoped && !enter_scope) return false;
    return std::any_of(expr.children.begin(), expr.children.end(),
        [](const auto &child) { return contains_deferred(*child); });
  }
  Value deferred_scope(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                        bool function_context, const std::function<Value()> &body, bool callable = false) {
    if (!contains_deferred(expr, !callable)) return body();
    DeferredScope scope{ir_.deferred_stack(), local_scope_, {}, deferred_scope_};
    registered_deferred_[scope.owner_scope] = 0;
    const auto active = llvm::SaveAndRestore(deferred_scope_, &scope);
    Expression guarded{};
    guarded.kind = Expression::Kind::try_scope;
    guarded.at = expr.at;
    for (size_t i = 0; i < 2; ++i) {
      auto clause = std::make_unique<Expression>();
      clause->kind = Expression::Kind::block;
      clause->at = expr.at;
      guarded.children.push_back(std::move(clause));
    }
    TryEmission emission;
    emission.body = body;
    emission.cleanup = [&](Value incoming) {
      return drain_deferred(scope, env, std::move(incoming));
    };
    emission.recovers = [&](const FlowState &path) {
      const auto queue = path.deferred.find(scope.owner_scope);
      return queue != path.deferred.end() && deferred_may_recover(queue->second);
    };
    return try_expression(guarded, module, env, locals, function_context, false, true, {}, nullptr, &emission);
  }
  Value drain_deferred(DeferredScope &scope, Environment &env, Value incoming) {
    const auto [flow, pending_facts] = pending_exception_flow(incoming);
    auto *error_payload = ir_.native_slot(ir_.type_of(exception_type(ExceptionCategory::system)), "defer.error");
    auto *error_category = ir_.native_slot(ir_.type_of(Type::uint64), "defer.category");
    ir_.write_native(error_payload, incoming.native);
    ir_.write_native(error_category, exception_category(incoming));
    auto *return_tag = ir_.native_slot(ir_.type_of(Type::uint64), "defer.return.tag");
    ir_.write_native(return_tag, ir_.bits(Type::uint64, 0));
    struct SavedReturn { Value value; std::vector<llvm::AllocaInst *> fields; Location at; };
    std::vector<SavedReturn> returned;
    llvm::AllocaInst *return_resource = nullptr;
    bool callable_resource = false;
    const auto clear_return_resource = [&] {
      if (!return_resource) return;
      if (callable_resource) ir_.replace_callable(return_resource, ir_.null_pointer());
      else ir_.replace_region(return_resource, ir_.null_pointer());
    };
    const auto save_return = [&](Value value, const Location &at) {
      if (native_callable(value)) value = detach_callable(std::move(value), at);
      if (value.type.owned) value = take_owner(std::move(value), at);
      if ((native_callable(value) || value.type.owned) && !return_resource) {
        return_resource = ir_.native_slot(value.native->getType(), "defer.return.owner");
        callable_resource = native_callable(value);
      }
      clear_return_resource();
      SavedReturn saved{std::move(value), {}, at};
      deferred_fields(saved.value, [&](llvm::Value *&native) {
        auto *slot = &native == &saved.value.native && return_resource ? return_resource :
            ir_.native_slot(native->getType(), "defer.return.value");
        saved.fields.push_back(slot);
        ir_.write_native(slot, native);
      });
      returned.push_back(std::move(saved));
      ir_.write_native(return_tag, ir_.bits(Type::uint64, returned.size()));
      ir_.write_native(error_category, ir_.bits(Type::uint64, 0));
    };
    auto *next = ir_.label("defer.next");
    auto *dispatch = ir_.label("defer.dispatch");
    auto *done = ir_.label("defer.done");
    ir_.jump(next);
    ir_.enter(next);
    auto *record = ir_.read_native(scope.head);
    ir_.choose(ir_.is_null(record), done, dispatch);
    ir_.enter(dispatch);
    auto *site = ir_.pop_deferred(scope.head, record);
    const auto entry_flow = flow_state();
    const auto entry_env = env;
    const auto check_flow = [&](const FlowState &exit, const DeferredAction &action) {
      const auto queue = exit.deferred.find(scope.owner_scope);
      if (queue != exit.deferred.end())
        deferred_flows_[action.registration].continuations.push_back(queue->second);
      bool changed = false;
      for (size_t i = 0; i < entry_flow.live.size(); ++i) changed |= !entry_flow.live[i].same(exit.live[i]);
      for (size_t i = 0; i < entry_flow.holdings.size(); ++i) changed |= entry_flow.holdings[i] != exit.holdings[i];
      for (size_t i = 0; i < entry_flow.initialized.size(); ++i) changed |= entry_flow.initialized[i] != exit.initialized[i];
      for (size_t i = 0; i < entry_flow.pointers.size(); ++i) changed |= entry_flow.pointers[i] != exit.pointers[i];
      for (const auto &[holder, value] : entry_flow.callables) {
        const auto *after = exit.callables.lookup(holder);
        changed |= !after || *after != value;
      }
      if (changed) fail(action.at, "deferred action changes flow facts; deferred backedge checking is not implemented yet");
    };
    for (size_t i = 0; i < scope.actions.size(); ++i) {
      const auto &action = scope.actions[i];
      auto *run = ir_.label("defer.invoke");
      auto *other = ir_.label("defer.other");
      ir_.choose(ir_.binary(Type::uint64, "==", site, ir_.bits(Type::uint64, i)), run, other);
      ir_.enter(run);
      restore_flow(entry_flow);
      registered_deferred_[scope.owner_scope] = action.predecessors;
      env = entry_env;
      const auto holding_begin = holdings_.size();
      const auto cleanup = ir_.scope();
      const auto action_scope = scopes_.size();
      scopes_.push_back(true);
      const auto release = [&] {
        for (auto index : action.images)
          ir_.replace_callable(ir_.deferred_field(action.layout, record, index), ir_.null_pointer());
        ir_.free_deferred(record);
        ir_.jump(next);
      };
      HandlerScope failures{cleanup, holding_begin, local_scope_, &env, {}};
      ReturnTarget replacement{cleanup, holding_begin, action_scope, &env, ir_.label("defer.return")};
      {
        const auto local = llvm::SaveAndRestore(local_scope_, action_scope);
        const auto owner = llvm::SaveAndRestore(ir_.cleanup_owner(), action_scope);
        const auto top = llvm::SaveAndRestore(module_top_level_, false);
        handlers_.push_back(&failures);
        return_targets_.push_back(&replacement);
        auto pending = scalar(exception_type(ExceptionCategory::system), ir_.read_native(error_payload));
        pending.exception_category = ir_.read_native(error_category);
        pending.exception_facts = pending_facts;
        if (action.handles_error) ir_.write_native(error_category, ir_.bits(Type::uint64, 0));
        const auto result = action.invoke(record, env, std::move(pending));
        return_targets_.pop_back();
        handlers_.pop_back();
        if (result.type != Type::never) {
          check_flow(flow_state(), action);
          close_holdings(holding_begin, {}, action.at);
          ir_.unwind(cleanup);
          release();
        }
        ir_.enter(replacement.block);
        auto replaced = join_completions(replacement.incoming, action.at, false);
        if (replaced.type != Type::never) {
          check_flow(flow_state(), action);
          save_return(std::move(replaced), action.at);
          close_holdings(holding_begin, {}, action.at);
          ir_.unwind(cleanup);
          release();
        }
        for (auto &landing : failures.landing) if (landing.block) {
          ir_.enter(landing.block);
          std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> payloads, categories;
          for (const auto &edge : landing.incoming) {
            merge_exception_possibilities(exception_flows_.at(flow), edge.error);
            check_flow(edge.regions, action);
            payloads.emplace_back(edge.error.native, edge.from);
            categories.emplace_back(exception_category(edge.error), edge.from);
          }
          ir_.write_native(error_payload, ir_.phi(error_payload->getAllocatedType(), payloads));
          ir_.write_native(error_category, ir_.phi(error_category->getAllocatedType(), categories));
          release();
        }
      }
      scopes_[action_scope] = false;
      ir_.restore(cleanup);
      ir_.enter(other);
    }
    ir_.unreachable();
    ir_.enter(done);
    restore_flow(entry_flow);
    registered_deferred_.erase(scope.owner_scope);
    env = entry_env;
    auto *resume = ir_.label("defer.resume");
    auto *failed = ir_.label("defer.failed");
    auto *category = ir_.read_native(error_category);
    ir_.choose(ir_.is_null(category), resume, failed);
    ir_.enter(failed);
    auto error = scalar(exception_type(ExceptionCategory::system), ir_.read_native(error_payload));
    error.exception_category = category;
    error.exception_facts = pending_facts;
    clear_return_resource();
    raise_exception(std::move(error));
    ir_.enter(resume);
    for (size_t i = 0; i < returned.size(); ++i) {
      auto *selected = ir_.label("defer.return.selected");
      auto *other = ir_.label("defer.return.other");
      ir_.choose(ir_.binary(Type::uint64, "==", ir_.read_native(return_tag), ir_.bits(Type::uint64, i + 1)), selected, other);
      ir_.enter(selected);
      restore_flow(entry_flow);
      registered_deferred_.erase(scope.owner_scope);
      auto value = returned[i].value;
      size_t field = 0;
      deferred_fields(value, [&](llvm::Value *&native) { native = ir_.read_native(returned[i].fields.at(field++)); });
      if (value.type.owned) {
        ir_.write_native(return_resource, ir_.null_pointer());
        auto region = regions_.at(*value.region_id);
        value.region_id = new_region(value.native, returned[i].at, region.origins);
        region.owner = value.native;
        region.live = true;
        regions_.at(*value.region_id) = std::move(region);
        value = hold_region(std::move(value), false, true);
      } else if (native_callable(value)) {
        ir_.write_native(return_resource, ir_.null_pointer());
        value.storage = StorageView{ir_.hold_callable(value.native, false), local_scope_, {}};
        value.callable_temporary = true;
        register_callable_holder(value);
      }
      route_return(std::move(value), returned[i].at);
      ir_.enter(other);
    }
    restore_flow(entry_flow);
    registered_deferred_.erase(scope.owner_scope);
    return {};
  }
  struct ExceptionHandlers {
    std::array<const Expression *, concrete_exception_count> catches{};
    const Expression *finally = nullptr;
  };
  ExceptionHandlers exception_handlers(const Expression &expr, const Module &module) {
    ExceptionHandlers result;
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto &clause = *expr.children[i];
      if (clause.kind == Expression::Kind::block) { result.finally = &clause; continue; }
      if (clause.annotation) {
        const auto type = resolve(*clause.annotation, module, clause.at);
        if (type.kind != Type::exception || !type.identity)
          fail(clause.at, "a typed catch requires an exact intrinsic exception category");
        auto &destination = result.catches.at(type.identity - 1);
        if (destination) fail(clause.at, "duplicate exception category in catch clauses");
        destination = &clause;
      } else for (auto &destination : result.catches) if (!destination) destination = &clause;
    }
    return result;
  }
  void register_deferred(DeferredScope &scope, const std::vector<llvm::Value *> &values,
                         std::set<size_t> catches = {}) {
    auto &action = scope.actions.back();
    action.predecessors = registered_deferred_[scope.owner_scope];
    ir_.register_deferred(scope.head, action.layout, scope.actions.size() - 1, values);
    deferred_flows_.push_back({{action.predecessors}, std::move(catches), action.recovers, {}});
    action.registration = deferred_flows_.size() - 1;
    registered_deferred_[scope.owner_scope] = action.registration;
  }
  void deferred_fields(Value &value, const std::function<void(llvm::Value *&)> &visit) {
    const auto field = [&](llvm::Value *&native) { if (native) visit(native); };
    field(value.native);
    field(value.exception_category);
    field(value.global_slot);
    if (value.storage) field(value.storage->address);
    if (value.reference_field) {
      field(value.reference_field->owner);
      field(value.reference_field->address);
    }
    if (value.procedure && !native_callable(value)) {
      auto procedure = std::make_shared<Procedure>(*value.procedure);
      callable_fields(*procedure, [&](llvm::Value *&native, bool) { visit(native); });
      value.procedure = std::move(procedure);
    }
  }
  Value deferred_block(const Expression &expr, const Module &module, const Locals &locals, bool function_context) {
    auto selected = locals;
    const auto references = lexical_references(expr, locals);
    auto caught = caught_exception_;
    std::set<llvm::Value *> controls = deferred_loop_controls_;
    std::vector<llvm::Value *> values;
    std::map<size_t, size_t> owners;
    const auto save = [&](Value &value) {
      if (value.loop_control) { controls.insert(value.loop_control); return; }
      deferred_fields(value, [&](llvm::Value *&native) { values.push_back(native); });
      if (value.region_id && !owners.contains(*value.region_id)) {
        owners.emplace(*value.region_id, values.size());
        values.push_back(regions_.at(*value.region_id).owner);
      }
    };
    for (auto &[name, value] : selected.visible)
      if (references.required.contains(name)) save(value);
    for (auto &[name, value] : selected.owner_anchors)
      if (references.owners.contains(name)) save(value);
    if (caught) save(*caught);
    auto *layout = ir_.deferred_layout(values);
    auto recovered = std::make_shared<bool>(false);
    auto invoke = [this, selected, references, caught, controls, recovered, layout, &expr, &module, function_context]
                  (llvm::Value *record, Environment &env, Value incoming) -> Value {
      auto locals = selected;
      auto active_error = caught;
      size_t field = 0;
      std::map<size_t, llvm::Value *> previous_owners;
      const auto restore = [&](Value &value) {
        if (value.loop_control) return;
        deferred_fields(value, [&](llvm::Value *&native) { native = ir_.read_deferred_field(layout, record, field++); });
        if (value.region_id && !previous_owners.contains(*value.region_id)) {
          auto &owner = regions_.at(*value.region_id).owner;
          previous_owners.emplace(*value.region_id, owner);
          owner = ir_.read_deferred_field(layout, record, field++);
        }
      };
      for (auto &[name, value] : locals.visible)
        if (references.required.contains(name)) restore(value);
      for (auto &[name, value] : locals.owner_anchors)
        if (references.owners.contains(name)) restore(value);
      if (active_error) restore(*active_error);
      const auto owner_restore = llvm::scope_exit([&] {
        for (const auto &[id, owner] : previous_owners) regions_.at(id).owner = owner;
      });
      const auto blocked_controls = llvm::SaveAndRestore(deferred_loop_controls_, controls);
      const auto active_catch = llvm::SaveAndRestore(caught_exception_, active_error);
      if (expr.children.size() == 1)
        return block(*expr.children.front(), module, env, locals, function_context, true);
      TryEmission emission;
      emission.incoming = std::move(incoming);
      emission.discard = true;
      emission.recovered = recovered.get();
      return try_expression(expr, module, env, locals, function_context, false, true, {}, nullptr, &emission);
    };
    auto &scope = *deferred_scope_;
    scope.actions.push_back({layout, expr.at, {}, std::move(invoke), expr.children.size() > 1, std::move(recovered)});
    std::set<size_t> catches;
    const auto handlers = exception_handlers(expr, module);
    for (size_t i = 0; i < handlers.catches.size(); ++i) if (handlers.catches[i]) catches.insert(i + 1);
    register_deferred(scope, values, std::move(catches));
    return {};
  }
  Value deferred_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                             bool function_context) {
    using K = Expression::Kind;
    const auto &invocation = *expr.children.front();
    if (!deferred_scope_)
      fail(expr.at, "process-lifetime module defer lowering is not implemented yet");
    if (invocation.kind == K::block)
      return deferred_block(expr, module, locals, function_context);
    std::optional<PreparedMethodCall> method_call;
    std::optional<PreparedCallableCall> callable_call;
    bool captured = false;
    std::optional<Value> selected;
    if (invocation.kind == K::value_call)
      selected = expression(*invocation.children.front(), module, env, locals, function_context);
    else if (invocation.kind == K::perform || invocation.kind == K::behave) {
      if (function_context) fail(expr.at, "a function cannot invoke a procedure");
      selected = read_symbol(expression_symbol(invocation, module, locals), env, expr.at, false);
      captured = invocation.kind == K::perform;
    } else if (const auto found = locals.visible.find(invocation.text); found != locals.visible.end()) {
      selected = found->second;
      require_declaration(*selected, expr.at);
    }
    if (selected) {
      if (selected->pointer) *selected = pointer_value(*selected->pointer, expr.at);
      *selected = read_value(std::move(*selected), expr.at);
      if (selected->type.kind != Type::procedure) fail(expr.at, "defer requires a callable invocation");
      if ((invocation.kind == K::perform || invocation.kind == K::behave) && selected->type.pure_callable)
        fail(expr.at, "perform/behave require a procedure, not a function");
      if (function_context && !selected->type.pure_callable) fail(expr.at, "a function cannot invoke a procedure");
      if (!selected->type.throws_types.empty()) require_npe_handler(expr.at);
      if (native_callable(*selected)) *selected = prepare_callable_argument(std::move(*selected), {});
      callable_call = prepare_callable_call(std::move(*selected), invocation, module, env, locals, function_context);
    } else {
      const auto information = lookup_callable(module, invocation.text, expr.at);
      if (information.methods.empty()) fail(expr.at, "unavailable deferred callable '" + invocation.text + "'");
      std::vector<ConversionPoint> points;
      auto args = arguments(invocation, module, env, locals, function_context, &points, information.methods);
      auto match = method(invocation.text, module, args, expr.at);
      if (!match && information.methods.size() == 1)
        match = MethodMatch{information.methods.front(), std::vector<Route>(args.size()), information.declarations};
      if (!match) method_failure(invocation.text, args, information.methods, expr.at, false, module);
      method_call = prepare_method_call(*match, std::move(args), expr.at, function_context, module, points);
    }
    auto &args = method_call ? method_call->arguments : callable_call->arguments;
    std::vector<llvm::Value *> values, image_slots;
    std::vector<size_t> images;
    const auto save = [&](Value &value, bool by_value) {
      valid_view(value, expr.at);
      if (value.type.owned)
        fail(expr.at, "deferred owning-argument flow checking is not implemented yet");
      if (!by_value)
        fail(expr.at, "deferred borrowed-argument flow checking is not implemented yet");
      if (native_callable(value)) {
        if (!value.storage || !value.callable_temporary)
          value = prepare_callable_argument(std::move(value), {});
        images.push_back(values.size());
        image_slots.push_back(value.storage->address);
      } else if (local_bits(value.type)) value = bits_value(std::move(value));
      else if (value.procedure && (!value.procedure->lexical_captures.visible.empty() ||
                                  (captured && !value.procedure->captures.bindings.isEmpty())))
        fail(expr.at, "deferred generic callable capture lowering is not implemented yet");
      if (value.native) values.push_back(value.native);
      if (value.exception_category) values.push_back(value.exception_category);
      value.pointer.reset();
      value.callable_address = 0;
    };
    if (callable_call) save(callable_call->callee, true);
    for (size_t i = 0; i < args.size(); ++i) {
      const bool by_value = method_call ? parameter_at(*method_call->method.function.form, i).owner.empty() :
          (!native_callable(callable_call->callee) && callable_call->callee.procedure->form ?
              parameter_at(*callable_call->callee.procedure->form, i).owner.empty() :
              !borrowed_parameter(callable_call->callee.type, i));
      save(args[i], by_value);
    }
    auto *layout = ir_.deferred_layout(values);
    const auto scope_id = deferred_scope_->owner_scope;
    auto invoke = [this, method_call, callable_call, captured, layout, scope_id, &module, at = expr.at]
                  (llvm::Value *record, Environment &current, Value) -> Value {
      auto method = method_call;
      auto callable = callable_call;
      size_t field = 0;
      const auto restore = [&](Value &value) {
        if (value.native) {
          const auto index = field++;
          value.native = ir_.read_deferred_field(layout, record, index);
          if (native_callable(value))
            value.storage = StorageView{ir_.deferred_field(layout, record, index), scope_id, {}};
        }
        if (value.exception_category) value.exception_category = ir_.read_deferred_field(layout, record, field++);
      };
      if (callable) restore(callable->callee);
      for (auto &value : method ? method->arguments : callable->arguments) restore(value);
      return method ? invoke_prepared_method(*method, at, current, module) :
                      invoke_prepared_callable(*callable, module, current, at, captured);
    };
    auto &scope = *deferred_scope_;
    scope.actions.push_back({layout, expr.at, std::move(images), std::move(invoke), false, {}});
    register_deferred(scope, values);
    for (auto *slot : image_slots) ir_.take_callable(slot);
    return {};
  }

  Value invoke_procedure(Value value, const Expression &expr, const Module &module,
                         Environment &env, Locals &locals, bool function_context, bool captured) {
    if (value.pointer) value = pointer_value(*value.pointer, expr.at);
    if (value.type.kind == Type::procedure && value.storage) value = read_value(std::move(value), expr.at);
    if (value.type.kind != Type::procedure)
      fail(expr.at, expr.kind == Expression::Kind::value_call ? "call target must be a function or procedure value" :
           "binding '" + expr.text + "' is not callable");
    if (function_context && !value.type.pure_callable) fail(expr.at, "a function cannot invoke a procedure");
    valid_view(value, expr.at);
    if (!value.procedure && !native_callable(value)) fail(expr.at, "binding '" + expr.text + "' is not callable");
    if (!value.type.throws_types.empty()) require_npe_handler(expr.at);
    if (native_callable(value)) {
      auto selected = value;
      std::optional<size_t> protection;
      if (selected.callable_address) {
        protection = ir_.protect_callable(selected.native);
        selected.native = ir_.protected_callable(*protection);
      }
      const auto leave_protection = llvm::scope_exit([&] {
        if (protection) ir_.leave_callable_protection(*protection);
      });
      auto prepared = prepare_callable_call(std::move(selected), expr, module, env, locals, function_context);
      auto result = invoke_prepared_callable(prepared, module, env, expr.at, captured);
      if (protection && result.type != Type::never) ir_.release_callable_protection(*protection);
      return result;
    }
    auto prepared = prepare_callable_call(std::move(value), expr, module, env, locals, function_context);
    return invoke_prepared_callable(prepared, module, env, expr.at, captured);
  }

  Value invoke(const Expression &expr, const Module &module, Environment &env, Locals &locals,
               bool function_context) {
    if (function_context) fail(expr.at, "a function cannot invoke a procedure");
    const auto id = expression_symbol(expr, module, locals);
    const auto value = read_symbol(id, env, expr.at, false);
    if (value.type.pure_callable) fail(expr.at, "perform/behave require a procedure, not a function");
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
      if (edge) value = reference_source(std::move(value), *field.type.target, *owner.region_id, field.declaration_kind == "ref", initializer.at, field.read_only);
      else if (field.type.kind == Type::exception) value = admit(std::move(value), field.type, module, initializer.at);
      else require(value.type, field.type, initializer.at);
      const auto slot = ir_.field(native, offset, root, order[i], field.type);
      if (field.declaration_kind == "ref") ir_.require_representable_reference(slot, value.native);
      ir_.write(slot, stored_value(value, field.type, initializer.at, field.read_only));
    }
    return contextual(std::move(owner), module);
  }
  Value allocate_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    auto owner = expression(*expr.children[0], module, env, locals, function_context);
    if (owner.type.kind != Type::region) fail(expr.at, "alloc requires a whole region");
    require_writable(owner.read_only, expr.at, "cannot allocate through a read-only region view");
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
                                     fields[i].declaration_kind == "ref", expr.at, fields[i].read_only);
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
      native_args.push_back(type.kind == Type::regional ? stored_value(args[i], products_.at(type.identity).fields[i].type, expr.at,
          products_.at(type.identity).fields[i].read_only) : args[i].native);
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
    for (const auto &arg : args) {
      merge_conformances(result.conformances, arg.conformances);
      merge_result_origins(result, arg);
    }
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
      resolve_result_facts(accumulator, expr.at);
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
    std::optional<size_t> exception_flow;
    if (accumulator.type.kind == Type::exception) {
      auto [flow, facts] = pending_exception_flow(accumulator);
      exception_flow = flow;
      accumulator.exception_facts = std::move(facts);
    }
    auto inner_locals = locals.nested();
    auto index_value = scalar(first.type, native_loop.index);
    index_value.loop_control = native_loop.index;
    declare_local(inner_locals, expr.text, std::move(index_value), module, expr.at);
    if (expr.kind == K::fold) {
      accumulator.native = native_loop.accumulator;
      accumulator.loop_control = native_loop.accumulator;
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
    std::vector<std::pair<size_t, size_t>> callable_headers, address_headers, deferred_headers;
    if (!one && !empty) {
      for (auto &[scope, queue] : registered_deferred_) {
        deferred_flows_.push_back({{queue}, {}});
        queue = deferred_flows_.size() - 1;
        deferred_headers.emplace_back(scope, queue);
      }
      const auto incoming = callable_holders_;
      for (const auto &[holder, flow] : incoming) {
        callable_flows_.push_back({{}, {flow}, true});
        callable_flows_.back().loop = true;
        const auto header = callable_flows_.size() - 1;
        callable_holders_ = callable_memory_factory_.add(callable_holders_, holder, header);
        callable_headers.emplace_back(holder, header);
      }
    }
    for (auto &pointer : pointers_) {
      const auto &current = *pointer.view;
      const bool initialized = !current.storage || std::all_of(current.storage->cells.begin(), current.storage->cells.end(),
          [&](size_t cell) { return initialization_known(cells_.at(cell).initialized) == true; });
      loop_pointers.initialized.push_back(initialized);
      if (!one && !empty && !pointer.fixed_binding && initialized) {
        auto view = current;
        if (view.callable_address) {
          callable_flows_.push_back({{}, {view.callable_address}, true});
          callable_flows_.back().loop = true;
          view.callable_address = callable_flows_.size() - 1;
          address_headers.emplace_back(size_t(&pointer - pointers_.data()), view.callable_address);
        }
        if (!pointer.required && rebound_pointers.contains(size_t(&pointer - pointers_.data()))) view.nullable = true;
        view.referent = nullptr;
        if (view.storage) {
          view.storage->cells.clear();
          view.storage->initialized_alternatives = true;
        }
        pointer.view = snapshot_pointer(std::move(view));
      }
    }
    if (!one && !empty) loop_pointers_.push_back(loop_pointers);
    const auto next = block(*expr.children[body_index], module, env, inner_locals, function_context,
                            expr.kind == K::range_for, {}, local_bits(accumulator.type), true, nullptr, true);
    if (next.type != Type::never) for (const auto &[scope, header] : deferred_headers) {
      const auto back = registered_deferred_.at(scope);
      if (back != header) deferred_flows_[header].inputs.push_back(back);
    }
    for (const auto &[holder, id] : callable_headers) {
      auto &header = callable_flows_.at(id);
      const auto *back = callable_holders_.lookup(holder);
      if (next.type != Type::never && back && *back != id) header.inputs.push_back(*back);
      header.open = false;
    }
    for (const auto &[pointer, id] : address_headers) {
      auto &header = callable_flows_.at(id);
      const auto back = pointers_[pointer].view->callable_address;
      if (next.type != Type::never && back != id) header.inputs.push_back(back);
      header.open = false;
    }
    if (!one && !empty) {
      loop_pointers_.pop_back();
      if (next.type != Type::never)
        for (size_t i = 0; i < loop_pointers.boundary; ++i) {
          const auto &view = *pointers_[i].view;
          if (loop_pointers.initialized[i] && view.storage &&
              !std::all_of(view.storage->cells.begin(), view.storage->cells.end(),
                  [&](size_t cell) { return initialization_known(cells_.at(cell).initialized) == true; }))
            fail(expr.at, "loop-carried pointer target must remain definitely initialized on every continuing path");
        }
    }

    if (expr.kind == K::fold && next.type != Type::never) {
      require(next.type, accumulator.type, expr.at);
      if (next.region_id != accumulator.region_id)
        fail(expr.at, "a fold reference must retain its owning region" +
                         region_note(accumulator.region_id, "initial accumulator's region acquired here") +
                         region_note(next.region_id, "iteration result's region acquired here"));
      require_access(next.read_only, accumulator.read_only, expr.at,
          "a fold cannot feed a read-only view into a writable accumulator");
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
    if (exception_flow && !empty && next.type != Type::never)
      merge_exception_possibilities(exception_flows_.at(*exception_flow), next);
    ir_.end_loop(native_loop, next.native, next.type.kind == Type::exception ? exception_category(next) : nullptr);
    if (has_else) {
      const bool body_completes = !empty && next.type != Type::never;
      if (!body_completes) ir_.unreachable();
      ir_.restore(native_scope);
      ir_.otherwise(*empty_branch);
      restore_flow(before);
      const auto fallback = block(*expr.children[3], module, env, locals, function_context, true, {}, false, true, nullptr, true);
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
    accumulator.loop_control = nullptr;
    return accumulator;
  }
  Value call_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool function_result = false) {
    if (swap_operation(expr.text)) return swap_expression(expr, module, env, locals, function_context || function_result);
    if (expr.text == "=") return rebind_expression(expr, module, env, locals, function_context || function_result);
    if (expr.text == "<-") return transfer_expression(expr, module, env, locals, function_context || function_result);
    if (const auto local = locals.visible.find(expr.text); !expr.intrinsic && local != locals.visible.end()) {
      const auto callee = local->second;
      require_declaration(callee, expr.at);
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
    std::optional<MethodMatch> selected;
    try { selected = method(expr.text, module, args, expr.at); }
    catch (const UnresolvedAccess &) {
      if (auto result = defer_method(expr.text, module, args, env, expr.at,
                                     function_context || function_result, points)) return *result;
      throw;
    }
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
      if (read) for (const auto &value : *evaluated) {
        require_argument_reads(value);
        if (value.storage && !value.native) fail(expr.at, "read of storage before definite initialization in an argument");
      }
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
      if (destination.type.kind == Type::procedure && destination.storage)
        return transfer_callable(std::move(destination), args[2], expr.at);
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
      auto result = scalar(exception_type(ExceptionCategory::system), payload);
      result.exception_category = category;
      return result;
    }
    if (expr.text == "root") {
      const auto &args = values();
      if (args.size() != 1 || args[0].type.kind != Type::region || !args[0].type.target)
        fail(expr.at, "root requires one typed region");
      const auto &owner = args[0];
      auto value = scalar(*owner.type.target, ir_.region_root(region_pointer(owner, expr.at)), owner.region_id);
      value.conformances = owner.conformances; value.result_fact_origins = owner.result_fact_origins;
      value.result_type_origin = regions_.at(*owner.region_id).root_origin;
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
      if (!args.empty()) require_writable(args[0].read_only, expr.at, "cannot store through a read-only view");
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
      require_writable(object.read_only, expr.at, "cannot change fields through a read-only view");
      const auto target = object.type.kind == Type::reference ? *object.type.target : object.type;
      if (target.kind != Type::regional)
        fail(expr.at, "setfield requires a regional record view or reference");
      check_nullable(object, expr.at);
      auto value = expression(*expr.children[2], module, env, locals, function_context, false);
      const auto [declaration, slot] = record_field(object, expr.children[1]->text, module, expr.at);
      const auto field = declaration.type;
      if (declaration.fixed_binding || (field.kind != Type::reference && declaration.read_only))
        fail(expr.at, "cannot mutate a const field after initialization");
      if (field.kind == Type::reference)
        value = reference_source(std::move(value), *field.target, *object.region_id, declaration.declaration_kind == "ref", expr.at, declaration.read_only);
      else value = read_value(std::move(value), expr.at);
      if (field.kind == Type::exception) value = admit(std::move(value), field, module, expr.at);
      else require(value.type, field, expr.at);
      region_pointer(object, expr.at);
      if (field.kind == Type::reference) {
        value.region_id = object.region_id;
        merge_conformances(value.conformances, object.conformances);
        merge_result_origins(value, object);
        value = contextual(std::move(value), module);
      }
      if (declaration.declaration_kind == "ref") ir_.require_representable_reference(slot, value.native);
      ir_.write(slot, stored_value(value, field, expr.at, declaration.read_only));
      value.read_only |= declaration.read_only;
      return field.kind == Type::reference ? target_view(std::move(value), expr.at) : value;
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
      const auto before = flow_state();
      auto native_branch = ir_.branch(left.native, expr.text == "||");
      Value right;
      try { right = expression(*expr.children[1], module, right_env, locals, function_context); }
      catch (const EndPath &) { right = scalar(Type::never, nullptr); }
      require(right.type, Type::boolean, expr.at);
      ir_.otherwise(native_branch, right.native);
      const auto right_state = flow_state();
      auto joined = right.type == Type::never ? before : right_state;
      intersect_flow(joined, before);
      restore_flow(joined);
      std::vector<EnvironmentEdge> environments{{env, ir_.here(), before}};
      if (right.type != Type::never) environments.push_back({right_env, native_branch.first_end, right_state});
      auto result = scalar(Type::boolean, ir_.merge(native_branch, Type::boolean, left.native));
      env = join_environments(environments, expr.at);
      return result;
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
                      left.region_id && right.region_id &&
                      region_identity(*left.region_id, expr.at) != region_identity(*right.region_id, expr.at)));
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
  struct CaptureAlternative { const Value *value; llvm::BasicBlock *from; };
  Value join_captures(const std::vector<CaptureAlternative> &alternatives, const Location &at) {
    auto result = *alternatives.front().value;
    // The body stays statically known. Only its borrowed regional places vary
    // at runtime; selecting their addresses preserves subsequent slot writes.
    auto select = [&](auto get) -> llvm::Value * {
      auto *first = get(*alternatives.front().value);
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> edges;
      bool different = false;
      for (const auto &alternative : alternatives) {
        auto *value = get(*alternative.value);
        if (bool(value) != bool(first) || (value && value->getType() != first->getType()))
          fail(at, "callable capture alternatives require compatible storage representations");
        different |= value != first;
        edges.emplace_back(value, alternative.from);
      }
      return different ? ir_.phi(first->getType(), edges) : first;
    };
    for (const auto &alternative : alternatives) {
      const auto &value = *alternative.value;
      if (value.type.capture_regions != result.type.capture_regions) {
        auto message = std::string("callable alternatives require the same actual region owners");
        std::set<size_t> owners(result.type.capture_regions.begin(), result.type.capture_regions.end());
        owners.insert(value.type.capture_regions.begin(), value.type.capture_regions.end());
        for (const auto id : owners) message += region_note(id, "a callable alternative retains this region");
        fail(at, message);
      }
      require(value.type, result.type, at);
      if (value.type.capture_owner_regions != result.type.capture_owner_regions)
        fail(at, "runtime callable selection requires the same actual with-owner associations");
      if (value.region_id != result.region_id || value.holding_id != result.holding_id ||
          bool(value.storage) != bool(result.storage) || bool(value.reference_field) != bool(result.reference_field) ||
          value.required_reference != result.required_reference || value.pointer_view != result.pointer_view ||
          value.pointer || value.arguments || value.pending_owner ||
          (value.storage && value.storage->scope != result.storage->scope))
        fail(at, "callable capture alternatives require the same owner and storage kind");
      result.read_only |= value.read_only;
      result.binding_read_only |= value.binding_read_only;
      result.reference_read_only &= value.reference_read_only;
      result.nullable |= value.nullable;
      std::erase_if(result.conformances, [&](const Conformance &fact) {
        return std::find(value.conformances.begin(), value.conformances.end(), fact) == value.conformances.end();
      });
      if (result.storage) {
        result.storage->cells.insert(value.storage->cells.begin(), value.storage->cells.end());
        result.storage->initialized_alternatives |= value.storage->initialized_alternatives;
      }
    }
    std::vector<Value> fact_alternatives;
    for (const auto &alternative : alternatives) fact_alternatives.push_back(*alternative.value);
    join_result_facts(result, std::move(fact_alternatives));
    result.nonnull_when.reset();
    result.referent = nullptr;
    result.native = select([](const Value &value) { return value.native; });
    result.exception_category = select([](const Value &value) { return value.exception_category; });
    if (result.type.kind == Type::exception) {
      auto facts = std::make_shared<ExceptionPossibilities>();
      for (const auto &alternative : alternatives) merge_exception_possibilities(*facts, *alternative.value);
      result.exception_facts = std::move(facts);
    }
    if (result.storage)
      result.storage->address = select([](const Value &value) { return value.storage->address; });
    if (result.reference_field) {
      for (const auto &alternative : alternatives) require(alternative.value->reference_field->type, result.reference_field->type, at);
      result.reference_field->owner = select([](const Value &value) { return value.reference_field->owner; });
      result.reference_field->address = select([](const Value &value) { return value.reference_field->address; });
    }
    if (native_callable(result)) {
      std::vector<size_t> inputs, addresses;
      for (const auto &alternative : alternatives) {
        if (!native_callable(*alternative.value)) fail(at, "callable alternatives require compatible native representations");
        inputs.push_back(alternative.value->callable_flow);
        addresses.push_back(alternative.value->callable_address);
      }
      result.callable_flow = join_callable_flow(std::move(inputs));
      result.callable_address = join_callable_flow(std::move(addresses));
      return result;
    }
    if (std::all_of(alternatives.begin(), alternatives.end(), [&](const CaptureAlternative &alternative) {
          return alternative.value->procedure == result.procedure;
        })) return result;
    if (!result.procedure) fail(at, "control-flow alternatives must all be callable");
    auto procedure = std::make_shared<Procedure>(*result.procedure);
    for (const auto &alternative : alternatives) {
      const auto &candidate = alternative.value->procedure;
      if (!candidate || candidate->form != procedure->form || candidate->lexical != procedure->lexical ||
          candidate->builtin != procedure->builtin || candidate->implementation != procedure->implementation ||
          candidate->declarations != procedure->declarations)
        fail(at, "runtime selection between different callable implementations is not yet supported");
      if (candidate->captures.bindings != procedure->captures.bindings ||
          candidate->captures.captured_bindings != procedure->captures.captured_bindings ||
          candidate->captures.captured_context != procedure->captures.captured_context ||
          candidate->captures.replay != procedure->captures.replay)
        fail(at, "runtime selection between different captured dynamic bindings is not yet supported");
      if (candidate->lexical_captures.regional_slots != procedure->lexical_captures.regional_slots)
        fail(at, "callable capture alternatives require matching regional slots");
    }
    auto join_map = [&](auto &destination, auto get) {
      for (const auto &alternative : alternatives)
        if (get(*alternative.value->procedure).size() != destination.size())
          fail(at, "callable capture alternatives require matching names");
      for (auto &[name, value] : destination) {
        std::vector<CaptureAlternative> captures;
        for (const auto &alternative : alternatives) {
          const auto &source = get(*alternative.value->procedure);
          const auto found = source.find(name);
          if (found == source.end()) fail(at, "callable capture alternatives require matching names");
          captures.push_back({&found->second, alternative.from});
        }
        value = join_captures(captures, at);
      }
    };
    join_map(procedure->lexical_captures.visible, [](const Procedure &value) -> const auto & { return value.lexical_captures.visible; });
    join_map(procedure->lexical_captures.owner_anchors, [](const Procedure &value) -> const auto & { return value.lexical_captures.owner_anchors; });
    result.procedure = std::move(procedure);
    return result;
  }
  Environment join_environments(const std::vector<EnvironmentEdge> &edges, const Location &at) {
    if (edges.empty()) return {};
    auto result = edges.front().environment;
    if (edges.size() == 1) return result;
    std::map<std::vector<size_t>, size_t> owners;
    for (const auto &[id, first] : result.bindings) {
      std::vector<CaptureAlternative> alternatives;
      for (const auto &edge : edges) {
        const auto *value = edge.environment.lookup(id);
        if (!value) break;
        alternatives.push_back({value, edge.from});
      }
      if (alternatives.size() != edges.size()) {
        result.bindings = binding_factory_.remove(result.bindings, id);
        continue;
      }
      if (std::all_of(alternatives.begin(), alternatives.end(), [&](const auto &item) { return item.value == first; })) continue;
      auto value = *first;
      std::vector<size_t> region_ids;
      for (const auto &alternative : alternatives) {
        require(alternative.value->type, value.type, at);
        if (alternative.value->region_id) region_ids.push_back(*alternative.value->region_id);
        value.read_only |= alternative.value->read_only;
        std::erase_if(value.conformances, [&](const Conformance &fact) {
          const auto &facts = alternative.value->conformances;
          return std::find(facts.begin(), facts.end(), fact) == facts.end();
        });
      }
      if (native_callable(value)) value = join_captures(alternatives, at);
      else {
        const auto select = [&](auto get) -> llvm::Value * {
          auto *first_value = get(*first);
          if (!first_value) return nullptr;
          std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
          for (const auto &item : alternatives) incoming.emplace_back(get(*item.value), item.from);
          if (std::all_of(incoming.begin(), incoming.end(), [&](const auto &item) { return item.first == first_value; })) return first_value;
          return ir_.phi(first_value->getType(), incoming);
        };
        value.native = select([](const Value &item) { return item.native; });
        if (value.type.kind == Type::exception) {
          value.exception_category = select([&](const Value &item) { return exception_category(item); });
          auto facts = std::make_shared<ExceptionPossibilities>();
          for (const auto &alternative : alternatives) merge_exception_possibilities(*facts, *alternative.value);
          value.exception_facts = std::move(facts);
        }
      }
      if (!region_ids.empty() && std::any_of(region_ids.begin(), region_ids.end(), [&](size_t owner) { return owner != region_ids.front(); })) {
        if (region_ids.size() != edges.size()) fail(at, "symbol alternatives require regional provenance on every path");
        auto found = owners.find(region_ids);
        if (found == owners.end()) {
          std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> incoming;
          std::set<size_t> origins;
          for (size_t i = 0; i < region_ids.size(); ++i) {
            const auto &region = regions_.at(region_ids[i]);
            incoming.emplace_back(region.owner, edges[i].from);
            origins.insert(region.origins.begin(), region.origins.end());
          }
          const auto joined = new_region(ir_.phi(incoming.front().first->getType(), incoming), at, std::move(origins));
          auto &region = regions_[joined];
          region.live = true;
          for (size_t i = 0; i < region_ids.size(); ++i)
            region.live = FlowCondition::both(region.live, region_ids[i] < edges[i].regions.live.size()
                ? edges[i].regions.live[region_ids[i]] : FlowCondition{false});
          region.symbolic = true;
          region.root = regions_[region_ids.front()].root;
          for (const auto owner : region_ids) if (regions_[owner].root != region.root) region.root.reset();
          found = owners.emplace(region_ids, joined).first;
        }
        value.region_id = found->second;
        value.holding_id.reset();
      }
      value.declaration_cell.reset();
      binding_values_.push_back(std::move(value));
      result.bindings = binding_factory_.add(result.bindings, id, &binding_values_.back());
    }
    return result;
  }
  void route_return(Value value, const Location &at) {
    auto &target = *return_targets_.back();
    valid_view(value, at);
    if (native_callable(value) && !return_context_->owner && return_context_->read)
      value = detach_callable(std::move(value), at);
    if (value.type.owned) value = take_owner(std::move(value), at);
    auto state = flow_state();
    close_state(state, target.holding_begin, value.type.owned ? value.region_id : std::nullopt, target.owner_scope);
    auto environment = *target.environment;
    for (auto frame = active_environments_.rbegin(); frame != active_environments_.rend(); ++frame)
      if (frame->first == target.owner_scope) { environment = *frame->second; break; }
    ir_.unwind(target.cleanup, target.owner_scope);
    target.environments.push_back({std::move(environment), ir_.here(), state});
    target.incoming.push_back({std::move(value), ir_.here(), std::move(state)});
    ir_.jump(target.block);
  }
  Value return_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    if (!return_context_) fail(expr.at, "return requires an enclosing func or proc");
    const auto &context = *return_context_;
    Value result;
    if (!expr.children.empty())
      result = expression(*expr.children.front(), module, env, locals, function_context,
                          context.read, context.copy, context.read,
                          context.type && context.type->kind == Type::procedure ? context.type : std::nullopt, context.owner);
    if (result.type == Type::null_ref && context.owner && context.type)
      result = null_target(*context.type, *context.owner, expr.at);
    if (context.type) result = admit(std::move(result), *context.type, module, expr.at);
    else return_context_->type = result.type;
    if (context.destination && result.pointer != context.destination)
      fail(expr.at, "a (=) overload must return its destination binding's resulting view; return the lhs parameter");
    route_return(std::move(result), expr.at);
    return scalar(Type::never, nullptr);
  }
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
    if (std::any_of(completions.begin(), completions.end(), [](const Completion &item) {
          return !item.value.result_fact_origins.empty();
        })) result.conformances.clear();
    if (result.type.kind == Type::procedure) {
      auto values = completions;
      std::vector<CaptureAlternative> alternatives;
      for (auto &completion : values) {
        // A selected view retains its holder address, not the lexical binding
        // of the expression that supplied it.
        completion.value.pointer.reset();
        alternatives.push_back({&completion.value, completion.from});
      }
      result = join_captures(alternatives, at);
    }
    if (result.storage && std::any_of(completions.begin(), completions.end(),
        [](const Completion &completion) { return !completion.value.native; })) result.native = nullptr;
    result.pointer.reset();
    auto state = completions.front().regions;
    std::set<size_t> origins;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> values, categories;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> addresses;
    auto exception_facts = result.type.kind == Type::exception ? std::make_shared<ExceptionPossibilities>() : nullptr;
    for (const auto &completion : completions) {
      if (result.type.kind == Type::exception) merge_exception_possibilities(*exception_facts, completion.value);
      if (completion.value.type != result.type && completion.value.type.kind == Type::region &&
          result.type.kind == Type::region && completion.value.type.owned && result.type.owned &&
          (completion.value.result_type_origin || result.result_type_origin)) {
        deferred_checks_.push_back({[this, first = result, second = completion.value, at]() mutable {
          resolve_result_type(first, at);
          resolve_result_type(second, at);
          return first.type != second.type;
        }, at, "owning control-flow results require one concrete root layout"});
      } else require(completion.value.type, result.type, at);
      result.read_only |= completion.value.read_only;
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
    std::vector<Value> fact_alternatives;
    for (const auto &completion : completions) fact_alternatives.push_back(completion.value);
    join_result_facts(result, std::move(fact_alternatives));
    restore_flow(state);
    if (!values.empty()) result.native = ir_.phi(ir_.type_of(result.type), values);
    if (!addresses.empty()) result.storage->address = ir_.phi(addresses.front().first->getType(), addresses);
    if (!categories.empty()) result.exception_category = ir_.phi(ir_.type_of(Type::uint64), categories);
    if (result.type.kind == Type::exception) result.exception_facts = std::move(exception_facts);
    if (native_callable(result) && !result.storage) {
      result.storage = StorageView{ir_.hold_callable(result.native, module_lifetime), local_scope_, {}};
      register_callable_holder(result);
      result.callable_temporary = true;
    }
    if (result.type.owned) {
      if (std::any_of(completions.begin(), completions.end(), [&](const Completion &completion) {
            return completion.value.region_id != result.region_id;
          })) result.region_id = new_region(result.native, at, std::move(origins));
      regions_.at(*result.region_id).live = true;
      if (result.type.target) regions_.at(*result.region_id).root = *result.type.target;
      regions_.at(*result.region_id).root_origin = result.result_type_origin;
      if (result.result_fact_origins.size() == 1 &&
          std::any_of(completions.begin(), completions.end(), [&](const Completion &completion) {
            return completion.value.region_id != result.region_id;
          })) {
        auto &region = regions_.at(*result.region_id);
        region.owner_origin = result.result_fact_origins.front();
        for (const auto &completion : completions) region.owner_inputs.insert(*completion.value.region_id);
      }
      result = hold_region(std::move(result), module_lifetime, true);
    }
    return result;
  }
  // A shared finally is checked against every entering completion. After it
  // finishes, unchanged facts can be refined to the selected continuation.
  FlowState continue_flow(const FlowState &path, const FlowState &entry, FlowState exit) {
    for (auto &[scope, queue] : exit.deferred) {
      const auto before = entry.deferred.find(scope);
      const auto original = path.deferred.find(scope);
      if (before != entry.deferred.end() && before->second == queue)
        queue = original == path.deferred.end() ? 0 : original->second;
    }
    for (size_t i = 0; i < std::min(path.live.size(), entry.live.size()); ++i)
      if (exit.live[i].same(entry.live[i])) exit.live[i] = path.live[i];
    for (size_t i = 0; i < std::min(path.holdings.size(), entry.holdings.size()); ++i)
      if (exit.holdings[i] == entry.holdings[i]) exit.holdings[i] = path.holdings[i];
    for (size_t i = 0; i < std::min(path.initialized.size(), entry.initialized.size()); ++i)
      if (exit.initialized[i] == entry.initialized[i]) exit.initialized[i] = path.initialized[i];
    for (size_t i = 0; i < std::min(path.pointers.size(), entry.pointers.size()); ++i)
      if (exit.pointers[i] == entry.pointers[i]) exit.pointers[i] = path.pointers[i];
    auto callables = path.callables;
    for (const auto &[holder, value] : entry.callables)
      if (!exit.callables.lookup(holder)) callables = callable_memory_factory_.remove(callables, holder);
    for (const auto &[holder, value] : exit.callables) {
      const auto *before = entry.callables.lookup(holder);
      if (!before || *before != value) callables = callable_memory_factory_.add(callables, holder, value);
    }
    exit.callables = callables;
    return exit;
  }
  Environment continue_environment(Environment path, const Environment &entry, const Environment &exit) {
    for (const auto &[id, value] : entry.bindings)
      if (!exit.lookup(id)) path.bindings = binding_factory_.remove(path.bindings, id);
    for (const auto &[id, value] : exit.bindings)
      if (entry.lookup(id) != value) path.bindings = binding_factory_.add(path.bindings, id, value);
    return path;
  }
  Value try_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                       bool function_context, bool copy_bits = false, bool read_result = true,
                       std::optional<Type> callable_result = {}, const Value *result_owner = nullptr,
                       const TryEmission *emission = nullptr) {
    const bool parent_module = module_top_level_;
    bool recovered = false;
    const auto [catches, finally] = exception_handlers(expr, module);
    auto *normal_join = ir_.label("try.result");
    auto *failure_join = finally ? ir_.label("try.failure") : nullptr;
    std::vector<Completion> completions;
    std::vector<EnvironmentEdge> environments;
    std::vector<ExceptionalEdge> failures;
    const auto complete = [&](Value result) {
      if (result.type == Type::never) return;
      if (result.region_id) region_pointer(result, expr.at);
      if (result.type.owned) result = take_owner(result, expr.at);
      if (native_callable(result) && !result_owner) result = detach_callable(std::move(result), expr.at);
      completions.push_back({result, ir_.here(), flow_state()});
      environments.push_back({env, ir_.here(), flow_state()});
      ir_.jump(normal_join);
    };
    const auto propagate = [&](Value error) {
      if (!finally) { raise_exception(error); return; }
      failures.push_back({error, ir_.here(), flow_state(), env});
      ir_.jump(failure_join);
    };
    const auto enter_errors = [&](const std::vector<ExceptionalEdge> &incoming) {
      auto state = incoming.front().regions;
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> payloads, categories;
      std::vector<EnvironmentEdge> bindings;
      auto facts = std::make_shared<ExceptionPossibilities>();
      for (const auto &edge : incoming) {
        merge_exception_possibilities(*facts, edge.error);
        intersect_flow(state, edge.regions);
        payloads.emplace_back(edge.error.native, edge.from);
        categories.emplace_back(exception_category(edge.error), edge.from);
        bindings.push_back({edge.environment, edge.from, edge.regions});
      }
      restore_flow(state);
      auto error = scalar(exception_type(ExceptionCategory::system), ir_.phi(ir_.type_of(exception_type(ExceptionCategory::system)), payloads));
      error.exception_category = ir_.phi(ir_.type_of(Type::uint64), categories);
      error.exception_facts = std::move(facts);
      env = join_environments(bindings, expr.at);
      return error;
    };
    HandlerScope protected_scope{ir_.scope(), holdings_.size(), local_scope_, &env, {}};
    ReturnTarget returns{protected_scope.cleanup, protected_scope.holding_begin, local_scope_, &env,
                         finally ? ir_.label("try.return") : nullptr};
    if (finally) return_targets_.push_back(&returns);
    handlers_.push_back(&protected_scope);
    Value normal;
    {
      const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, npe_allowed_ || catches[exception_id(ExceptionCategory::null_pointer) - 1]);
      if (emission && emission->incoming) {
        auto *body_entry = ir_.label("try.body");
        auto *error_entry = ir_.label("try.incoming");
        ir_.choose(ir_.is_null(exception_category(*emission->incoming)), body_entry, error_entry);
        ir_.enter(error_entry);
        raise_exception(*emission->incoming);
        ir_.enter(body_entry);
      }
      normal = emission && emission->body ? emission->body() : block(*expr.children.front(), module, env, locals,
          function_context, emission && emission->discard, callable_result, copy_bits, read_result, result_owner);
    }
    handlers_.pop_back();
    complete(normal);

    struct CatchEntry { llvm::BasicBlock *block; std::vector<ExceptionalEdge> incoming; };
    std::map<const Expression *, CatchEntry> entries;
    for (size_t i = 0; i < protected_scope.landing.size(); ++i) {
      auto &landing = protected_scope.landing[i];
      if (!landing.block) continue;
      ir_.enter(landing.block);
      auto error = enter_errors(landing.incoming);
      const auto *clause = catches[i];
      if (!clause) { propagate(error); continue; }
      auto [entry, inserted] = entries.try_emplace(clause, CatchEntry{nullptr, {}});
      if (inserted) entry->second.block = ir_.label("catch.body");
      entry->second.incoming.push_back({error, ir_.here(), flow_state(), env});
      ir_.jump(entry->second.block);
    }
    // Source order determines lexical visibility even when runtime categories
    // reach the same handler. Each handler body is elaborated only once.
    for (size_t i = 1; i < expr.children.size(); ++i) {
      const auto *clause = expr.children[i].get();
      const auto found = entries.find(clause);
      if (found == entries.end()) continue;
      ir_.enter(found->second.block);
      auto error = enter_errors(found->second.incoming);
      if (clause->annotation) error.type = resolve(*clause->annotation, module, clause->at);
      auto inner_locals = locals;
      inner_locals.declared.erase(clause->text);
      declare_local(inner_locals, clause->text, error, module, clause->at);
      HandlerScope catch_scope{ir_.scope(), holdings_.size(), local_scope_, &env, {}};
      if (finally) handlers_.push_back(&catch_scope);
      Value result;
      {
        const auto active = llvm::SaveAndRestore(caught_exception_, std::optional(error));
        result = block(*clause->children.front(), module, env, inner_locals, function_context,
                       emission && emission->discard, callable_result, copy_bits, read_result, result_owner);
      }
      if (finally) handlers_.pop_back();
      const auto restore_binding = [&](auto &destination, const auto &source) {
        destination.erase(clause->text);
        if (const auto previous = source.find(clause->text); previous != source.end()) destination.insert(*previous);
      };
      restore_binding(inner_locals.visible, locals.visible);
      restore_binding(inner_locals.locations, locals.locations);
      restore_binding(inner_locals.declared, locals.declared);
      restore_binding(inner_locals.regional_slots, locals.regional_slots);
      locals = std::move(inner_locals);
      recovered |= result.type != Type::never;
      complete(result);
      for (auto &landing : catch_scope.landing) if (landing.block) {
        ir_.enter(landing.block);
        propagate(enter_errors(landing.incoming));
      }
    }
    if (finally) return_targets_.pop_back();
    ir_.enter(normal_join);
    auto result = join_completions(completions, expr.at, parent_module);
    if (result.type != Type::never) env = join_environments(environments, expr.at);
    if (!finally) {
      if (emission && emission->recovered) *emission->recovered = recovered;
      return result;
    }

    enum class CompletionKind { returned, failed, normal };
    struct Pending {
      CompletionKind kind;
      Value value;
      llvm::BasicBlock *from;
      FlowState flow;
      Environment environment;
    };
    std::vector<Pending> pending;
    auto *finalizer = ir_.label("finally.body");
    const auto retain = [&](CompletionKind kind, Value value) {
      pending.push_back({kind, std::move(value), ir_.here(), flow_state(), env});
      ir_.jump(finalizer);
    };
    // Keep normal completion last so every preceding dispatch terminates.
    std::optional<Pending> normal_path;
    if (result.type != Type::never) {
      normal_path = Pending{CompletionKind::normal, result, ir_.here(), flow_state(), env};
      ir_.jump(finalizer);
    }
    ir_.enter(returns.block);
    auto returned = join_completions(returns.incoming, expr.at, parent_module);
    if (returned.type != Type::never) {
      env = join_environments(returns.environments, expr.at);
      retain(CompletionKind::returned, returned);
    }
    ir_.enter(failure_join);
    if (failures.empty()) ir_.unreachable();
    else retain(CompletionKind::failed, enter_errors(failures));
    if (normal_path) pending.push_back(std::move(*normal_path));
    ir_.enter(finalizer);
    if (pending.empty()) { ir_.unreachable(); return scalar(Type::never, nullptr); }
    auto state = pending.front().flow;
    std::vector<EnvironmentEdge> bindings;
    std::vector<llvm::BasicBlock *> inputs;
    std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> tags;
    for (size_t i = 0; i < pending.size(); ++i) {
      const auto &path = pending[i];
      intersect_flow(state, path.flow);
      bindings.push_back({path.environment, path.from, path.flow});
      inputs.push_back(path.from);
      tags.emplace_back(ir_.bits(Type::uint64, i), path.from);
    }
    restore_flow(state);
    env = join_environments(bindings, finally->at);
    auto *tag = ir_.phi(ir_.type_of(Type::uint64), tags);
    const auto finalizer_entry = flow_state();
    const auto finalizer_env = env;
    Value finished;
    if (emission && emission->cleanup) {
      std::vector<std::pair<llvm::Value *, llvm::BasicBlock *>> payloads, categories;
      auto facts = std::make_shared<ExceptionPossibilities>();
      for (const auto &path : pending) {
        if (path.kind == CompletionKind::failed) merge_exception_possibilities(*facts, path.value);
        payloads.emplace_back(path.kind == CompletionKind::failed ? path.value.native :
            llvm::Constant::getNullValue(ir_.type_of(exception_type(ExceptionCategory::system))), path.from);
        categories.emplace_back(path.kind == CompletionKind::failed ? exception_category(path.value) :
            ir_.bits(Type::uint64, 0), path.from);
      }
      auto incoming = scalar(exception_type(ExceptionCategory::system),
          ir_.phi(ir_.type_of(exception_type(ExceptionCategory::system)), payloads));
      incoming.exception_category = ir_.phi(ir_.type_of(Type::uint64), categories);
      incoming.exception_facts = std::move(facts);
      finished = emission->cleanup(std::move(incoming));
    } else finished = block(*finally, module, env, locals, function_context);
    require(finished.type, Type::void_type, finally->at);
    if (finished.type == Type::never) return finished;
    if (emission && emission->recovered) *emission->recovered = recovered;
    const auto finalizer_exit = flow_state();
    const auto finalizer_exit_env = env;
    const auto can_recover = [&](const Pending &path) {
      return path.kind == CompletionKind::failed && emission && emission->recovers && emission->recovers(path.flow);
    };
    auto *resumed = std::any_of(pending.begin(), pending.end(), can_recover) ? ir_.label("cleanup.result") : nullptr;
    std::vector<Completion> resumed_values;
    std::vector<EnvironmentEdge> resumed_environments;
    for (size_t i = 0; i < pending.size(); ++i) {
      const auto &path = pending[i];
      std::optional<LLVMEmission::Branch> branch;
      if (i + 1 != pending.size())
        branch = ir_.branch(ir_.binary(Type::uint64, "==", tag, ir_.bits(Type::uint64, i)));
      ir_.continuation(path.from, inputs, finalizer, ir_.here());
      restore_flow(continue_flow(path.flow, finalizer_entry, finalizer_exit));
      env = continue_environment(path.environment, finalizer_env, finalizer_exit_env);
      if (path.kind == CompletionKind::returned) route_return(path.value, expr.at);
      else if (path.kind == CompletionKind::failed && !can_recover(path)) raise_exception(path.value);
      else if (resumed) {
        resumed_values.push_back({path.kind == CompletionKind::failed ? Value{} : path.value, ir_.here(), flow_state()});
        resumed_environments.push_back({env, ir_.here(), flow_state()});
        ir_.jump(resumed);
      } else return path.value;
      if (branch) { ir_.otherwise(*branch); ir_.merge(*branch, Type::void_type); }
    }
    if (resumed) {
      ir_.enter(resumed);
      auto value = join_completions(resumed_values, expr.at, parent_module);
      if (value.type != Type::never) env = join_environments(resumed_environments, expr.at);
      return value;
    }
    return scalar(Type::never, nullptr);
  }

  Value conditional_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context, bool copy_bits = false, bool read_result = true,
                          std::optional<Type> callable_result = {}, const Value *result_owner = nullptr) {
    const auto condition = expression(*expr.children[0], module, env, locals, function_context);
    require(condition.type, Type::boolean, expr.at);

    const bool parent_module = module_top_level_;
    const auto before = flow_state();
    const auto entry_env = env;
    auto native_branch = ir_.branch(condition.native);
    if (condition.nonnull_when && condition.nonnull_when->second) {
      auto &pointer = pointers_.at(condition.nonnull_when->first);
      auto view = *pointer.view; view.nullable = false;
      pointer.view = snapshot_pointer(std::move(view));
    }
    auto first = block(*expr.children[1], module, env, locals, function_context, false, callable_result, copy_bits, read_result, result_owner);
    if (first.type.owned) first = take_owner(first, expr.at);
    if (native_callable(first) && !result_owner) first = detach_callable(std::move(first), expr.at);
    auto first_state = flow_state();
    const auto first_env = env;
    ir_.otherwise(native_branch);
    restore_flow(before);
    env = entry_env;
    if (condition.nonnull_when && !condition.nonnull_when->second) {
      auto &pointer = pointers_.at(condition.nonnull_when->first);
      auto view = *pointer.view; view.nullable = false;
      pointer.view = snapshot_pointer(std::move(view));
    }
    auto second = block(*expr.children[2], module, env, locals, function_context, false, callable_result, copy_bits, read_result, result_owner);
    if (second.type.owned) second = take_owner(second, expr.at);
    if (native_callable(second) && !result_owner) second = detach_callable(std::move(second), expr.at);
    if (first.type == Type::null_ref && storage_owner(second))
      first = null_target(second.type, second, expr.at);
    else if (second.type == Type::null_ref && storage_owner(first))
      second = null_target(first.type, first, expr.at);
    std::vector<EnvironmentEdge> environments;
    std::vector<Completion> completions;
    if (first.type != Type::never) {
      completions.push_back({first, native_branch.first_end, std::move(first_state)});
      environments.push_back({first_env, native_branch.first_end, completions.back().regions});
    }
    if (second.type != Type::never) {
      completions.push_back({second, ir_.here(), flow_state()});
      environments.push_back({env, ir_.here(), flow_state()});
      ir_.jump(native_branch.merge);
    }
    ir_.enter(native_branch.merge);
    auto result = join_completions(completions, expr.at, parent_module);
    if (result.type != Type::never) env = join_environments(environments, expr.at);
    return result;
  }
  Value expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                   bool function_context, bool read = true, bool copy_bits = false, bool read_nested = true,
                   std::optional<Type> callable_result = {}, const Value *result_owner = nullptr) {
    if (clang::isStackNearlyExhausted()) {
#if LLVM_ENABLE_THREADS
      Value result;
      std::exception_ptr error;
      // WPO can have deep source call paths. Use Clang's existing stack
      // support, and resume C++ diagnostics on the originating stack.
      clang::runWithSufficientStackSpace([] {}, [&] {
        try { result = expression(expr, module, env, locals, function_context, read, copy_bits, read_nested, callable_result, result_owner); }
        catch (...) { error = std::current_exception(); }
      });
      if (error) std::rethrow_exception(error);
      return result;
#else
      fail(expr.at, "compiler stack capacity exhausted while elaborating this expression");
#endif
    }
    active_environments_.emplace_back(local_scope_, &env);
    const auto environment_frame = llvm::scope_exit([&] { active_environments_.pop_back(); });
    auto result = evaluate(expr, module, env, locals, function_context, copy_bits, read_nested, callable_result, result_owner);
    if (result.type == Type::never) throw EndPath{};
    if (copy_bits && local_bits(result.type)) return bits_value(read_value(std::move(result), expr.at));
    return read ? read_value(std::move(result), expr.at) : result;
  }
  Value declaration_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context) {
    using K = Expression::Kind;
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) {
          block(*declaration, module, env, locals, function_context, true);
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
        if (expr.declaration_kind == "func" || expr.declaration_kind == "proc") {
          if (value.type.kind != Type::procedure || (!value.procedure && !native_callable(value)))
            fail(declaration->at, "a func/proc initializer must be callable");
          if (value.type.pure_callable != (expr.declaration_kind == "func"))
            fail(declaration->at, "a callable declaration must match its function/procedure kind");
          if (value.nullable) fail(declaration->at, "a func/proc value requires a proven non-null callable");
          if (declaration->annotation)
            value = admit(std::move(value), bind_capture_type(resolve(*declaration->annotation, module, declaration->at),
                          locals.owners(), declaration->at), module, declaration->at);
          if (native_callable(value)) {
            value = read_value(std::move(value), declaration->at);
            if (!value.callable_temporary) {
              value.native = ir_.copy_callable(value.native);
              value.storage = StorageView{ir_.hold_callable(value.native, module_top_level_), local_scope_, {}};
              register_callable_holder(value);
            }
            value.callable_temporary = false;
            value.pointer.reset();
            value.pointer_view = false;
            value.read_only = expr.fixed_binding || declaration->read_only;
          }
          declare_local(locals, declaration->text, std::move(value), module, declaration->at);
          continue;
        }
        if (value.type.kind == Type::procedure && expr.declaration_kind == "ref")
          fail(declaration->at, "non-null callable values use func or proc, not ref");
        std::optional<Value> owner;
        if (!declaration->owner_name.empty()) {
          const auto found = locals.visible.find(declaration->owner_name);
          if (found == locals.visible.end()) fail(declaration->at, "unavailable pointer owner '" + declaration->owner_name + "'");
          owner = found->second;
          require_declaration(*owner, declaration->at);
          if (owner->type.kind != Type::region) fail(declaration->at, "an explicit pointer owner must name a region");
        }
        if (value.type == Type::null_ref) {
          if (expr.declaration_kind != "ptr" || declaration->binding != "=" || !declaration->annotation)
            fail(declaration->at, "null initialization requires ptr, an explicit target type, and '='");
          value = null_target(bind_capture_type(resolve(*declaration->annotation, module, declaration->at),
                              locals.owners(), declaration->at), owner, declaration->at);
        } else {
          if (declaration->annotation) value = admit(std::move(value), bind_capture_type(resolve(*declaration->annotation, module, declaration->at),
                                        locals.owners(), declaration->at), module, declaration->at);
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
          value.read_only = expr.fixed_binding || declaration->read_only;
          value.pointer.reset();
          declare_local(locals, declaration->text, std::move(value), module, declaration->at);
        } else {
          if (expr.declaration_kind == "ref" && value.nullable)
            fail(declaration->at, "ref requires a proven non-null target");
          if (declaration->annotation && !declaration->read_only)
            require_writable(value.read_only, declaration->at, "a writable target annotation cannot bind a read-only view");
          declare_pointer(locals, declaration->text, std::move(value), module, declaration->at, declaration->read_only, expr.fixed_binding,
                          expr.declaration_kind == "ref");
        }
      }
      return {};
  }
  Value alias_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                         bool function_context) {
    using K = Expression::Kind;
    const auto &source = *expr.children.front();
    if (source.kind != K::name && source.kind != K::field && source.kind != K::index)
      fail(expr.at, "var requires an existing variable, field or array element lvalue");
    auto value = expression(source, module, env, locals, function_context, false);
    if (value.type.kind == Type::region || value.procedure || source.kind == K::symbol ||
        (!value.pointer && !value.reference_field && !value.storage))
      fail(expr.at, "var requires existing val/ref/ptr storage; whole-region and symbolic bindings use their own declarations");
    std::string kind = "val";
    if (value.pointer) kind = pointers_.at(*value.pointer).required ? "ref" : "ptr";
    else if (value.reference_field) kind = value.required_reference ? "ref" : "ptr";
    if (!expr.declaration_kind.empty() && kind != expr.declaration_kind)
      fail(expr.at, "var storage kind mismatch: declared " + expr.declaration_kind + ", selected " + kind);
    if (expr.annotation) {
      const auto type = resolve(*expr.annotation, module, expr.at);
      if ((local_bits(value.type) && type != value.type) || !accepts(value, type, module, expr.at))
        fail(expr.at, "var type annotation cannot convert storage: expected " + type_name(type) + ", got " + type_name(value.type));
    }
    if (!expr.owner_name.empty()) {
      const auto owner = locals.visible.find(expr.owner_name);
      if (owner == locals.visible.end() || owner->second.type.kind != Type::region)
        fail(expr.at, "unavailable region owner '" + expr.owner_name + "' for var alias");
      require_declaration(owner->second, expr.at);
      valid_view(owner->second, expr.at);
      if (value.pointer || !value.region_id)
        fail(expr.at, "var storage does not belong to declared region owner '" + expr.owner_name + "'");
      require_same_region(*value.region_id, *owner->second.region_id, expr.at,
          "var storage does not belong to declared region owner '" + expr.owner_name + "'");
    }
    const bool regional = !value.pointer && value.region_id.has_value();
    if (expr.annotation && !expr.read_only)
      require_writable(value.read_only, expr.at, "a writable alias annotation cannot expose a read-only view");
    value.binding_read_only |= expr.fixed_binding;
    value.read_only |= expr.read_only || (kind == "val" && expr.fixed_binding);
    if (regional) {
      value.native = nullptr;
      if (value.reference_field) value.storage.reset();
    }
    declare_local(locals, expr.text, std::move(value), module, expr.at);
    if (regional) locals.regional_slots.insert(expr.text);
    return {};
  }
  std::pair<Parameter, LLVMEmission::Place> record_field(Value object, const std::string &name,
      const Module &module, const Location &at, bool materialize = true) {
    bool pending_layout = false;
    try {
      resolve_region_type(object, at);
      if (object.result_type_origin && region_definition(object.type)) resolve_result_type(object, at);
    } catch (const UnresolvedAccess &) { pending_layout = true; }
    if (object.type.kind == Type::region && object.type.target) {
      if (materialize) object.native = ir_.region_root(region_pointer(object, at));
      object.type = *object.type.target;
      if (object.region_id) object.result_type_origin = regions_.at(*object.region_id).root_origin;
    } else if (object.type.kind == Type::reference) {
      if (materialize) check_nullable(object, at);
      object.type = *object.type.target;
    }
    if (pending_layout && object.result_type_origin)
      if (const auto known = result_layout(object.result_type_origin, at, false)) object.type = *known;
    if (object.type.kind != Type::regional)
      fail(at, "field access requires a product value; dereference regional pointers first");
    resolve(object.type, module, at);
    const auto &fields = products_.at(object.type.identity).fields;
    for (size_t i = 0; i < fields.size(); ++i) {
      if (fields[i].name != name) continue;
      if (!materialize) {
        if (pending_layout) pending_fields_.push_back({object.type, object.result_type_origin, fields[i], nullptr, at});
        return {fields[i], {fields[i].type, nullptr, nullptr}};
      }
      if (!pending_layout)
        return {fields[i], ir_.field(region_pointer(object, at), object.native, object.type, i, fields[i].type)};
      auto [place, offset] = ir_.pending_field(region_pointer(object, at), object.native, fields[i].type);
      pending_fields_.push_back({object.type, object.result_type_origin, fields[i], offset, at});
      return {fields[i], place};
    }
    fail(at, "no field '" + name + "' in " + type_name(object.type));
  }
  Value field_expression(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                          bool function_context) {
    auto value = expression(*expr.children[0], module, env, locals, function_context);
    if (value.type.kind == Type::exception) {
      if (expr.text == "category") return scalar(Type::uint64, exception_category(value));
      if (expr.text == "cause_count") return scalar(Type::uint64, ir_.cause_count(value.native));
      const std::array<std::string_view, 3> fields{"site", "requested", "available"};
      const auto field = std::find(fields.begin(), fields.end(), expr.text);
      if (field == fields.end()) fail(expr.at, "unknown intrinsic exception field '" + expr.text + "'");
      return scalar(Type::uint64, ir_.exception_field(value.native, unsigned(field - fields.begin())));
    }
    const auto [field, place] = record_field(value, expr.text, module, expr.at);
    if (field.type.kind == Type::reference) {
      const auto slot = place;
      const auto offset = ir_.read(slot);
      auto result = scalar(field.type, offset, value.region_id);
      result.nullable = true;
      result.reference_field = slot;
      result.pointer_view = true;
      result.required_reference = field.declaration_kind == "ref";
      result.binding_read_only = ReadOnly::either(value.read_only, field.fixed_binding);
      result.reference_read_only = field.read_only;
      if (field.declaration_kind == "ref" || field.declaration_kind == "ptr") {
        result.type = *field.type.target;
        result.nullable = field.declaration_kind == "ptr";
        if (local_bits(result.type)) {
          result.storage = StorageView{ir_.nullable_address(slot.owner, offset), {}, {}};
          result.native = nullptr;
        }
      }
      result.conformances = value.conformances; result.result_fact_origins = value.result_fact_origins;
      result.read_only = ReadOnly::either(value.read_only, field.read_only);
      return contextual(std::move(result), module);
    }
    const auto target = place;
    if (local_bits(field.type)) {
      auto result = scalar(field.type, nullptr, value.region_id);
      result.storage = StorageView{target.address, {}, {}};
      result.read_only = ReadOnly::either(value.read_only, field.read_only || field.fixed_binding);
      return result;
    }
    return loaded_value(field.type, ir_.read(target));
  }
  Value evaluate(const Expression &expr, const Module &module, Environment &env, Locals &locals,
                 bool function_context, bool copy_bits = false, bool read_result = true,
                 std::optional<Type> callable_result = {}, const Value *result_owner = nullptr) {
    const auto location = ir_.source_location(expr.at);
    using K = Expression::Kind;
    switch (expr.kind) {
    case K::deferred: return deferred_expression(expr, module, env, locals, function_context);
    case K::try_scope: return try_expression(expr, module, env, locals, function_context, copy_bits, read_result, callable_result, result_owner);
    case K::handler: fail(expr.at, "catch requires an enclosing try");
    case K::return_value: return return_expression(expr, module, env, locals, function_context);
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
      require_npe_handler(expr.at, exception_possibilities(error).at(exception_id(ExceptionCategory::null_pointer) - 1));
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
    case K::callable: {
      auto value = define_procedure(*expr.definition, module, env, locals, implementation_context_);
      if (expr.definition->name.empty()) return value;
      value.callable_temporary = false;
      declare_local(locals, expr.definition->name, std::move(value), module, expr.at);
      return {};
    }
    case K::pointer_group: return declaration_expression(expr, module, env, locals, function_context);
    case K::alias: return alias_expression(expr, module, env, locals, function_context);
    case K::pointer: fail(expr.at, "ptr declarator requires a declaration group");
    case K::make_local: {
      const auto type = expr.annotation ? resolve(*expr.annotation, module, expr.at) :
          query_type(*expr.children.front(), module, locals, expr.at);
      auto value = local_storage(type, expr.at);
      const auto initial = expr.annotation ? 0u : 1u;
      if (expr.children.size() > initial) {
        auto source = block(*expr.children[initial], module, env, locals, function_context, false, type, true);
        write_bits(value, source, expr.at);
      }
      return value;
    }
    case K::region_binding: {
      const auto type = resolve(*expr.annotation, module, expr.at);
      const auto initializer = llvm::SaveAndRestore(region_initializer_, type.owned);
      auto value = admit(expression(*expr.children[0], module, env, locals, function_context), type, module, expr.at);
      if (type.target && !expr.read_only)
        require_writable(value.read_only, expr.at, "a writable region annotation cannot expose a read-only owner view");
      value.read_only |= expr.read_only;
      if (type.owned) value = hold_region(take_owner(value, expr.at), module_top_level_, true);
      else value = borrowed(std::move(value));
      value.pointer.reset();
      declare_local(locals, expr.text, value, module, expr.at);
      return {};
    }

    case K::capture_owner: {
      const auto callable = expression(*expr.children.front(), module, env, locals, function_context);
      valid_view(callable, expr.at);
      if (callable.type.kind != Type::procedure) fail(expr.at, "owner projection requires a callable value");
      const auto &names = callable.type.capture_owners;
      const auto found = std::find(names.begin(), names.end(), expr.text);
      if (found == names.end() || callable.type.capture_owner_regions.size() != names.size())
        fail(expr.at, "callable type has no associated owner '" + expr.text + "'; use a name from its visible with clause");
      const auto association = callable.type.capture_owner_regions[size_t(found - names.begin())];
      if (!association) fail(expr.at, "callable owner projection requires an instantiated region association");
      const auto id = *association;
      auto result = callable_owner(callable, id, expr.at);
      result.native = region_pointer(result, expr.at);
      return result;
    }
    case K::field: return field_expression(expr, module, env, locals, function_context);
    case K::index: {
      auto array = expression(*expr.children[0], module, env, locals, function_context);
      auto index = expression(*expr.children[1], module, env, locals, function_context);
      return array_element(array, index, expr.at);
    }
    case K::name: {
      if (const auto local = locals.visible.find(expr.text); local != locals.visible.end()) {
        if (deferred_loop_controls_.contains(local->second.loop_control))
          fail(expr.at, "a delayed defer block cannot capture a loop-control binding; prepare a deferred call argument instead");
        require_declaration(local->second, expr.at);
        if (locals.regional_slots.contains(expr.text)) return captured_view(local->second, expr.at);
        if (local->second.pointer) {
          auto value = pointer_value(*local->second.pointer, expr.at);
          value.binding_read_only = local->second.binding_read_only;
          value.read_only |= local->second.read_only;
          return value;
        }
        if (local->second.region_id) region_pointer(local->second, expr.at);
        return local->second;
      }
      if (const auto global = module.world.global_aliases.find(expr.text); global != module.world.global_aliases.end())
        return module.execution.global_values.at(global->second);
      if (const auto information = lookup_callable(module, expr.text, expr.at); !information.methods.empty()) {
        const auto &methods = information.methods;
        if (methods.size() != 1) fail(expr.at, "a procedure value requires an unambiguous lexical declaration");
        const auto &method = methods.front();
        if (method.form->kind == Form::Kind::procedure && !method.form->operation) {
          const auto value = method.lexical->execution.procedures.find(method.form);
          if (value == method.lexical->execution.procedures.end()) fail(expr.at, "lexical procedure used before its definition");
          auto selected = value->second;
          auto procedure = std::make_shared<Procedure>(*selected.procedure);
          const auto original_context = procedure->implementation.empty()
              ? ImplementationContext{procedure->lexical} : procedure->implementation;
          const auto selected_context = method.implementation.empty()
              ? ImplementationContext{method.lexical} : method.implementation;
          const bool adapted = original_context != selected_context ||
              procedure->declarations != information.declarations;
          procedure->implementation = method.implementation;
          procedure->declarations = information.declarations;
          if (procedure->code && adapted) {
            selected = detach_callable(std::move(selected), expr.at);
            procedure->code = std::make_shared<CallableCode>(*procedure->code);
            procedure->code->dispatch_key = ir_.callable_key();
            ir_.set_callable_entry(selected.native, procedure->code->dispatch_key);
            selected.storage = StorageView{ir_.hold_callable(selected.native, module_top_level_), local_scope_, {}};
          }
          selected.procedure = std::move(procedure);
          if (selected.procedure->code && adapted) {
            callable_flows_.push_back({selected.procedure, {}});
            selected.callable_flow = callable_flows_.size() - 1;
            register_callable_holder(selected);
          }
          return selected;
        }
        if (method.form->kind == Form::Kind::function && method.form->body) {
          return define_procedure(*method.form, *method.lexical, {}, {}, method.implementation, information.declarations);
        }
      }
      fail(expr.at, "unavailable lexical value '" + expr.text + "'");
    }
    case K::symbol:
      if (function_context) fail(expr.at, "a function cannot read symbolic effects");
      return read_symbol(expression_symbol(expr, module, locals), env, expr.at);
    case K::touch:
      if (function_context) fail(expr.at, "a function cannot touch symbolic effects");
      read_symbol(expression_symbol(expr, module, locals), env, expr.at, false);
      return {};
    case K::procedure_effect: {
      if (function_context) fail(expr.at, "a function cannot establish procedure effects");
      const auto id = expression_symbol(expr, module, locals);
      auto value = define_procedure(*expr.definition, module, env, locals, implementation_context_);
      bind(id, std::move(value), module, env, expr.at);
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
        const auto id = expression_symbol(*binding, module, locals);
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
      return {};
    }
    case K::make_region: return make_expression(expr, module, env, locals, function_context);
    case K::transfer: return transfer_expression(expr, module, env, locals, function_context);
    case K::rebind: return rebind_expression(expr, module, env, locals, function_context);
    case K::allocate: return allocate_expression(expr, module, env, locals, function_context);
    case K::range_for: case K::fold: return loop_expression(expr, module, env, locals, function_context);
    case K::block: return block(expr, module, env, locals, function_context, false, callable_result, copy_bits, read_result, result_owner);
    case K::call: return call_expression(expr, module, env, locals, function_context);
    case K::value_call: {
      auto callee = expression(*expr.children.front(), module, env, locals, function_context, false);
      return invoke_procedure(std::move(callee), expr, module, env, locals, function_context, false);
    }
    case K::perform: case K::behave: return invoke(expr, module, env, locals, function_context);
    case K::unary: return unary_expression(expr, module, env, locals, function_context);
    case K::binary: return binary_expression(expr, module, env, locals, function_context);
    case K::conditional: return conditional_expression(expr, module, env, locals, function_context, copy_bits, read_result, callable_result, result_owner);
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
    if (expr.definition) {
      auto nested = locals;
      if (!expr.definition->name.empty()) nested.insert(expr.definition->name);
      for (const auto &parameter : expr.definition->parameters) nested.insert(parameter.name);
      for (const auto &parameter : expr.definition->type_parameters) nested.insert(parameter.name);
      for (const auto &owner : expr.definition->owner_parameters) nested.insert(owner.name);
      if (expr.definition->body && context_dependent(*expr.definition->body, lexical, nested)) return true;
      if (expr.kind == Expression::Kind::callable && !expr.definition->name.empty())
        locals.insert(expr.definition->name);
    }
    if (expr.kind == Expression::Kind::block) {
      auto nested = locals;
      for (const auto &child : expr.children)
        if (context_dependent(*child, lexical, expr.scoped ? nested : locals)) return true;
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
    if (module.execution.state == ModuleExecution::State::complete) return;
    if (module.execution.state == ModuleExecution::State::compiling) fail(at, "cyclic module execution is outside the first prototype");
    const auto checked_scope = llvm::SaveAndRestore(npe_allowed_, false);
    const auto module_scope = llvm::SaveAndRestore(module_top_level_, true);
    const auto scope_id = scopes_.size();
    scopes_.push_back(true); // Module storage remains live through process exit.
    const auto lexical_scope = llvm::SaveAndRestore(local_scope_, scope_id);
    const auto cleanup_scope = llvm::SaveAndRestore(ir_.cleanup_owner(), scope_id);
    const auto holding_begin = holdings_.size();
    module.execution.state = ModuleExecution::State::compiling;
    execution_order_.push_back(module.path);
    if (module.builtin) {
      for (const auto &[name, id] : module.world.symbols) {
        auto proc = std::make_shared<Procedure>();
        proc->builtin = name;
        proc->lexical = &module;
        module.execution.symbolic_resources.emplace(id, Value{Type::procedure, {}, std::move(proc), &module});
      }
      module.execution.state = ModuleExecution::State::complete;
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
      case Form::Kind::region_definition: case Form::Kind::export_resource: break;
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
        auto value = define_procedure(form, module, env, locals);
        if (form.symbolic) {
          const auto id = symbol(module, form.name, form.at);
          module.world.available_symbols.insert(form.name);
          bind(id, std::move(value), module, env, form.at);
        } else {
          value.callable_temporary = false;
          module.execution.procedures.emplace(&form, std::move(value));
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
        value.global_slot = ir_.global_bits(value.type, stored_value(value, value.type, form.at));
        module.execution.global_values.emplace(identity, std::move(value));
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
          const auto resource = source.execution.symbolic_resources.find(source_id);
          if (resource == source.execution.symbolic_resources.end() || !resource->second.procedure)
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
            value = define_procedure(mixin_body(*value.procedure, form, module), module, env, locals);
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
              const auto original = candidate.lexical->execution.procedures.find(candidate.form);
              if (original == candidate.lexical->execution.procedures.end())
                fail(form.at, "procedure body requires its definition encounter before mixin");
              auto &body = mixin_body(*original->second.procedure, form, module);
              auto value = define_procedure(body, module, env, locals);
              declare_callable(module, body);
              value.callable_temporary = false;
              module.execution.procedures.emplace(&body, std::move(value));
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
          module.execution.global_values.emplace(identity->origin, source.execution.global_values.at(identity->origin));
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
      module.execution.state = ModuleExecution::State::complete;
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
      module.execution.symbolic_resources.emplace(id, std::move(value));
    }

    close_holdings(holding_begin, {}, at);
    ir_.restore(native_scope);
    module.execution.state = ModuleExecution::State::complete;
  }

public:
  explicit Compiler(WarningOptions warnings = {}) : warning_options_(std::move(warnings)) {
    ir_.on_exception([this](size_t category, llvm::Value *payload) {
      raise_exception(scalar(exception_type(category), payload));
    });
  }
  Compilation compile(const std::filesystem::path &entry, bool emit_text, bool emit_bitcode) {
    clang::noteBottomOfStack(true);
    auto &module = load(entry.string(), {entry.string()});
    // All source files have been parsed before any execution semantics are lowered.

    try {
      try { execute_module(module, {}, {module.path}); }
      catch (const EndPath &) { /* No later runtime forms execute. */ }
      // Loop headers now include their back edges. Check exactly the reachable
      // implementations; compiling a body can demand further callable nodes.
      for (;;) {
        bool changed = resolve_native_calls(0);
        changed |= resolve_callable_results();
        for (size_t i = 0; i < callable_demands_.size(); ++i) {
          const auto demand = callable_demands_[i];
          const auto choices = callable_choices(demand.flow);
          for (const auto &candidate : choices.definitions)
            changed |= compile_callable(*candidate, *demand.caller, demand.at);
        }
        if (changed) continue;
        for (auto &demand : native_call_demands_) close_callable_effects(demand);
        if (resolve_native_calls(0) || resolve_callable_results() || refine_native_reuse()) continue;
        break;
      }
      for (const auto &demand : callable_demands_)
        if (!callable_choices(demand.flow).closed) throw std::logic_error("unclosed callable result flow");
      for (auto &demand : native_call_demands_) {
        if (demand.selection)
          fail(demand.at, "access-dependent method selection still depends on an unfinished recursive flow");
        for (auto &argument : demand.arguments) {
          auto resolved = argument;
          resolve_result_facts(resolved, demand.at);
          if (demand.pending_argument_facts && resolved.conformances != argument.conformances)
            fail(demand.at, "recursive argument interface facts require a distinct checking context");
          argument = std::move(resolved);
        }
        const auto choices = callable_choices(demand.flow);
        if (!choices.closed) throw std::logic_error("unresolved procedure effects");
        if (demand.context && !demand.reused) {
          if (!choices.definitions.empty()) throw std::logic_error("unchecked reaching procedure body");
          auto previous = ir_.begin_native(demand.definition);
          ir_.unreachable();
          ir_.end_native(demand.definition, std::move(previous));
          ir_.resolve_context(demand.context, demand.definition);
          demand.context = nullptr;
        }
        if (demand.reused) {
          const auto reaching = callable_choices(demand.reused->flow);
          if (!std::is_permutation(choices.definitions.begin(), choices.definitions.end(),
                                  reaching.definitions.begin(), reaching.definitions.end()) ||
              !same_native_context(*demand.reused, demand, true))
            fail(demand.at, "recursive callable arguments or effects require a distinct context");
        }
      }
      for (size_t i = 0; i < pending_fields_.size(); ++i) {
        const auto pending = pending_fields_[i];
        auto record = scalar(pending.record, nullptr);
        record.result_type_origin = pending.result_origin;
        resolve_result_type(record, pending.at);
        const auto root = record.type;
        resolve(root, *products_.at(root.identity).module, pending.at);
        const auto &fields = products_.at(root.identity).fields;
        const auto field = std::find_if(fields.begin(), fields.end(), [&](const Parameter &field) {
          return field.name == pending.field.name;
        });
        const auto kind = [](const Parameter &field) {
          return field.declaration_kind.empty() ? std::string_view("val") : field.declaration_kind;
        };
        if (field == fields.end() || field->type != pending.field.type || kind(*field) != kind(pending.field) ||
            field->read_only != pending.field.read_only || field->fixed_binding != pending.field.fixed_binding)
          fail(pending.at, "selected record does not provide the declared field view");
        if (pending.offset) ir_.resolve_field(pending.offset, root, size_t(field - fields.begin()));
      }
      for (const auto &read : initialization_reads_) {
        if (initialization_known(read.proof) != true)
          fail(read.at, "read of storage before definite initialization\n" + location_text(cells_.at(read.cell).at) +
                       ": note: local storage allocated here");
      }
      for (size_t i = 0; i < projection_permissions_.size(); ++i) {
        const auto permission = projection_permissions_[i];
        (void)bool(permission);
      }
      for (size_t i = 0; i < deferred_checks_.size(); ++i) {
        const auto check = deferred_checks_[i];
        if (check.rejected()) fail(check.at, check.message);
      }
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
    // A reused caller may receive newly constructed contextual images with
    // equivalent schemas. Owner substitution applies only in that caller's
    // descendants. An escaping image with proven unchanged owners also needs
    // its equivalent keys accepted by downstream consumers.
    using ImagePair = std::pair<const Procedure *, const Procedure *>;
    std::vector<std::pair<const NativeCallDemand *, ImagePair>> adaptations;
    for (const auto &demand : native_call_demands_) if (demand.reused) {
      std::vector<ImagePair> pairs;
      std::set<const CallableCode *> escaping;
      if (!same_native_context(*demand.reused, demand, true, nullptr, false, &pairs, &escaping))
        fail(demand.at, "recursive callable context changed after checking");
      for (const auto &pair : pairs) {
        const bool escapes = escaping.contains(pair.first->code.get()) || escaping.contains(pair.second->code.get());
        adaptations.emplace_back(escapes ? nullptr : demand.reused, pair);
      }
    }
    bool matches_changed;
    do {
      matches_changed = false;
      for (const auto &[caller, pair] : adaptations)
        for (auto &child : native_call_demands_) {
          bool inside = !caller;
          for (auto *parent = child.parent; parent; parent = parent->parent) inside |= parent == caller;
          if (!inside || child.matches.contains(pair.second)) continue;
          if (const auto match = child.matches.find(pair.first); match != child.matches.end()) {
            ir_.extend_callable_match(match->second, pair.second->code->dispatch_key);
            child.matches.emplace(pair.second, match->second);
            matches_changed = true;
          }
        }
    } while (matches_changed);
    // Erasing an adapter is irreversible. Commit sharing only after return
    // evidence and every checked context have reached their final meaning.
    for (auto &demand : native_call_demands_) {
      if (!demand.reused) continue;
      bool recursive = false;
      for (auto *parent = demand.parent; parent; parent = parent->parent) recursive |= parent == demand.reused;
      ir_.reuse_native_context(demand.definition, demand.reused->definition, demand.context, recursive);
      demand.context = nullptr;
    }
    ir_.resolve_uncaught();
    for (const auto &[capture, writes] : capture_effects_)
      if (!capture->used)
        for (auto *store : *writes) store->eraseFromParent();
    for (const auto &flow : callable_flows_)
      if (flow.definition && flow.definition->code && flow.definition->code->construction) {
        const auto &code = *flow.definition->code;
        auto required = code.nested;
        for (const auto &[field, capture] : code.capture_fields)
          if (!capture->used) required.erase(field);
        ir_.finish_callable(*code.construction, required);
      }
    // Owner/type errors take precedence over warnings for unused listed captures.
    for (const auto &[at, message] : capture_warnings_) warn(Warning::unused_capture, at, message);
    auto [llvm_ir, bitcode] = ir_.finish(emit_text, emit_bitcode);
    return {scanned_, execution_order_, warnings_, std::move(llvm_ir), std::move(bitcode), regional_runtime_};
  }
};

} // namespace cod::language
