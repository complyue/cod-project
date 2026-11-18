//===--- lang_checked_program.hh - Checked program and evidence ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_diagnostics.hh"
#include "lang_exceptions.hh"
#include "lang_completion.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cod::language {

[[noreturn]] inline void implementation_gap(const Location &at, const std::string &detail) {
  fail(at, "micro-dispatch implementation gap: " + detail + "; this is not a language restriction");
}

inline bool scalar_type(const Type &type) { return numeric(type) || type == Type::boolean; }
inline bool bits_type(const Type &type) { return scalar_type(type) || type.kind == Type::exception; }
inline bool lexical_value_type(const Type &type) { return bits_type(type) || type.kind == Type::procedure; }
inline constexpr size_t null_target = std::numeric_limits<size_t>::max();
enum class BuiltinOperation { none, unary, binary, identity, cause_at, reference_swap, bits_swap, print };
inline bool eager_operation(std::string_view name) {
  return name == "+" || name == "-" || name == "*" || name == "/" || name == "%" || name == "!" ||
      name == "==" || name == "!=" || name == "<" || name == ">" || name == "<=" || name == ">=";
}
struct CheckedArgument {
  ArgumentMode mode;
  Type type;
  bool binding_slot = false;
  bool operator==(const CheckedArgument &) const = default;
};
inline ArgumentMode argument_mode(const Parameter &parameter) {
  if (parameter.owner.empty()) return ArgumentMode::value;
  if (parameter.declaration_kind == "ptr") return ArgumentMode::borrowed_ptr;
  if (parameter.declaration_kind == "ref") return ArgumentMode::borrowed_ref;
  return ArgumentMode::borrowed_read;
}
inline Type exception_type(size_t category) { return {Type::exception, exception_categories.at(category).name, category}; }
inline bool accepts_exception(const Type &source, const Type &target) {
  return source.kind == Type::exception && target.kind == Type::exception && (!target.identity || source == target);
}
inline bool transfers_bits(const Type &source, const Type &target) { return widens(source, target) || accepts_exception(source, target); }
inline bool transfers_value(const Type &from, const Type &to) {
  return from.kind == Type::procedure || to.kind == Type::procedure ? from == to : transfers_bits(from, to);
}
inline bool accepts_argument(const Type &source, const Type &target, ArgumentMode mode) {
  return mode == ArgumentMode::value ? transfers_value(source, target) : source == target;
}
inline std::set<size_t> exception_possibilities(const Type &type) {
  if (type.kind != Type::exception) return {};
  if (type.identity) return {type.identity};
  std::set<size_t> result;
  for (size_t i = 1; i < exception_categories.size(); ++i) result.insert(i);
  return result;
}

struct CheckedOperation {
  Type type;
  Type operand_type;
  std::optional<size_t> symbol;
  std::optional<size_t> callee;
  std::optional<size_t> callable, callable_binding, callable_place;
  std::vector<CheckedOperation> callable_captures;
  bool callable_call = false;
  bool callable_nullable = false;
  std::set<size_t> callable_targets;
  std::optional<std::set<size_t>> callable_choices;
  std::optional<size_t> place;
  std::optional<size_t> binding;
  // Immutable selected reference, independent of the mutable binding version.
  std::optional<size_t> selection_identity;
  std::set<size_t> slots;
  std::set<size_t> targets, owners, origins, selection_versions;
  // Formal nullable inputs, or null_target for an independently absent view.
  std::set<size_t> null_sources;
  std::optional<size_t> accumulator;
  bool read_only = false;
  bool fixed_binding = false;
  bool view = false;
  std::uint64_t integer = 0;
  bool minimum_literal = false;
  bool initializes = false;
  ExceptionFacts exceptions;
  std::set<size_t> captures;
  std::optional<size_t> captured_place;
  // Sufficient incoming-selection requirements; empty means initialized.
  std::optional<CleanupEquations::Initialization> initialization;
  bool initialized_target() const { return initialization && initialization->empty(); }
  void constrain_initialization(const std::optional<CleanupEquations::Initialization> &condition) {
    if (initialization && condition) initialization->insert(condition->begin(), condition->end());
    else initialization.reset();
  }
  bool mixed_view = false;
  bool borrowed_result = false;
  BuiltinOperation builtin = BuiltinOperation::none;
  std::string builtin_name; // Selected eager operation, independent of source spelling.
  // Destination calls select every address before checking entry-read promises.
  bool destination_arguments = false;
  std::vector<CheckedArgument> arguments;
  // Possible borrowed callable targets at invocation, after slot selection.
  std::vector<std::set<size_t>> callable_argument_targets;
  std::set<size_t> nullable_arguments;
  std::map<size_t, CleanupEquations::Effects> invocation;
  struct NullTest { size_t binding; std::set<size_t> versions; bool null_when_true = true; };
  struct Predicate {
    enum class Kind { null_test, negation, conjunction, disjunction, alternatives, abrupt, constant_true, constant_false, presence };
    Kind kind;
    NullTest test{};
    std::shared_ptr<const Predicate> left, right;
    CleanupEquations::BooleanValue results{};
    bool complete_results = false;
  };
  // Immutable evidence describes the produced boolean, independently of its source spelling.
  std::shared_ptr<const Predicate> predicate;
  // Absence means no relation; a present empty value retains provisional result evidence.
  std::optional<CleanupEquations::BooleanValue> nullness{};
};

struct CheckedStorage {
  std::string name;
  Type type;
  size_t scope;
  bool read_only;
  bool borrowed = false;
  size_t binding = 0;
};

struct CheckedBinding {
  Type type;
  bool read_only;
  bool value_binding;
  bool fixed;
  bool requires_initialized = false;
  std::optional<size_t> owner_binding = {};
  bool callable_value = false;
  // A definition has lexical identity but no runtime value slot.
  std::optional<size_t> definition = {};
  size_t scope = 0;
  // A fixed alias carries access restrictions and uses its selection's proof state.
  std::optional<size_t> alias = {};
  // A selected alias keeps one physical slot from this finite set for its whole lifetime.
  std::set<size_t> selection = {};
  // Caller-slot inputs can denote the same binding in an activation.
  std::set<size_t> overlaps = {};
};

// Substitutions are checking obligations, not semantic methods or activation records.
struct CallEffects {
  // Operand summaries use incoming ordinals, independent of callee storage.
  struct Read {
    size_t parameter;
    std::set<size_t> initialized;
    CleanupEquations::MissingInitialization uninitialized{};
    CleanupEquations::ConditionClauses when{CleanupEquations::ConditionFacts{}};
    // Sufficient initialization of the original borrowed input targets.
    std::optional<CleanupEquations::Initialization> initialized_inputs = {};
    auto operator<=>(const Read &) const = default;
  };
  std::set<Read> reads;
  std::map<size_t, std::set<size_t>> initialized;
  std::map<size_t, CleanupEquations::MissingInitialization> uninitialized;
  std::map<size_t, std::set<size_t>> present;
  struct BindingResult {
    // Initial input targets and nullable input provenance; null_target denotes
    // an absent target introduced by the method itself.
    std::set<size_t> targets, null_sources;
    bool initialized = false;
    bool operator==(const BindingResult &) const = default;
  };
  std::map<size_t, std::map<size_t, BindingResult>> bindings;
  // Every input target that can be written into a caller binding, including
  // intermediate selections not retained by any completed outcome. null_target
  // records an independently absent target introduced by the method.
  std::map<size_t, std::set<size_t>> binding_sources;
  // Postconditions describe the original borrowed targets, not local parameter
  // bindings after rebinding. Values refer to argument snapshots at call entry.
  // Empty provisional facts do not remove declared completion paths.
  std::map<size_t, std::map<size_t, ExceptionFacts>> payloads;
  std::map<size_t, CleanupEquations::BooleanValues> booleans;
  CleanupEquations::Returned returned;
  std::set<size_t> symbols; // Required current symbol identities, not operand positions.
  // Checked obligations of an immediate scope after local completion. Zero
  // represents the declared Exception root independently of payload evidence.
  std::set<size_t> scope_checked_failures;
};
enum class OwnerMatch { satisfied, unknown, mismatch };
// Proof identities are separate from executable owner handles. Scoped identities
// describe fresh storage within one evidence pass and are normalized when retained.
struct EvidenceOwner {
  enum class Kind { checked, retained, parameter, scoped };
  Kind kind = Kind::checked;
  size_t id;
  const Expression *scope = nullptr;
  EvidenceOwner(size_t id = 0) : id(id) {}
  static EvidenceOwner retained(size_t id) { EvidenceOwner owner{id}; owner.kind = Kind::retained; return owner; }
  static EvidenceOwner parameter(size_t id) { EvidenceOwner owner{id}; owner.kind = Kind::parameter; return owner; }
  static EvidenceOwner for_scope(const Expression &expr) {
    EvidenceOwner owner; owner.kind = Kind::scoped; owner.scope = &expr; return owner;
  }
  bool operator==(const EvidenceOwner &) const = default;
  bool operator<(const EvidenceOwner &other) const {
    if (kind != other.kind) return kind < other.kind;
    if (kind == Kind::scoped) return std::less<const Expression *>{}(scope, other.scope);
    return id < other.id;
  }
};
struct ViewEvidence {
  std::optional<bool> read_only, view, value_binding;
  std::optional<EvidenceOwner> owner = {};
  bool mixed_view = false;
  std::optional<bool> fixed_binding = {}, slot_read_only = {};
  // Independent capture eligibility is distinct from the binding-slot category.
  std::optional<bool> value_capture = {};
  bool operator==(const ViewEvidence &) const = default;
};
struct QueryBinding {
  std::string name;
  Type type;
  ViewEvidence view;
  bool operator==(const QueryBinding &) const = default;
};
enum class ResultMode { value, borrowed_ref, borrowed_ptr };
struct ResultContract {
  std::optional<Type> type;
  ResultMode mode = ResultMode::value;
  std::string owner = {};
  bool read_only = false;
  bool borrowed() const { return mode != ResultMode::value; }
  bool nullable() const { return mode == ResultMode::borrowed_ptr; }
  bool operator==(const ResultContract &) const = default;
};
// Declaration evidence has no runtime payload or authority to access retained storage.
struct QueryEnvironment {
  std::vector<QueryBinding> bindings;
  std::set<std::string> symbol_shadows;
  std::vector<std::vector<OwnerMatch>> owners;
  std::optional<ResultContract> result;
  bool operator==(const QueryEnvironment &) const = default;
};
// Incoming values name storage roles; lexical bindings also include owner names.
struct CheckedInput {
  size_t place;
  ArgumentMode mode;
  bool binding_slot = false;
  bool operator==(const CheckedInput &) const = default;
};
struct CheckedFrame {
  std::vector<CheckedInput> inputs;
  std::vector<size_t> captures;
  // One incoming identity per input-bound owner, independent of target carriers.
  std::map<size_t, size_t> owners;
  bool operator==(const CheckedFrame &) const = default;
};
// Ordinary captured values retain their source access contract independently
// of their saved payload and the storage created for each invocation.
struct CheckedCaptureInput {
  Type type;
  bool read_only = false;
  bool operator==(const CheckedCaptureInput &) const = default;
};
struct CheckedInstance {
  size_t method;
  std::vector<Type> arguments;
  ResultContract result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
  std::vector<CheckedStorage> storage;
  std::vector<CheckedBinding> bindings;
  CallEffects effects;
  std::vector<CheckedCaptureInput> environment;
  std::vector<Type> context, retained_types;
  std::vector<std::optional<Type>> substitutions;
  size_t query_environment = 0;
  CheckedFrame frame;
};
struct CheckedMethod {
  const Form *source = nullptr;
  std::string operation; // Semantic operation identity, independent of the local alias.
  size_t module = 0, interpretation = 0, definition = null_target;
  std::optional<size_t> family;
  std::vector<size_t> nested;
  std::vector<size_t> instances;
  std::vector<Type> parameters;
  ResultContract result;
  std::set<size_t> throws_categories;
  std::vector<Type> throws_types;
  std::vector<Capture> captures;
  std::vector<Capture> queries;
  bool captures_resolved = false;
  bool contextual_result = false;
  bool declaration_context() const { return contextual_result || !queries.empty(); }
  std::vector<std::string> type_parameters;
  std::vector<size_t> retained_types;
  std::optional<size_t> parent;
};
struct CheckedCallable {
  Type type;
  std::optional<size_t> family = {};
  // Actual input identities select semantic methods before parameter preparation.
  std::vector<std::pair<std::vector<Type>, size_t>> dispatch = {};
  // A whole-domain proof can establish one prepared implementation.
  std::optional<size_t> uniform_target = {};
  // Original input types at these positions distinguish fixed scalar rows.
  std::optional<std::vector<size_t>> selection_inputs = {};
  std::set<size_t> implementations() const {
    std::set<size_t> result;
    if (uniform_target) result.insert(*uniform_target);
    for (const auto &[inputs, target] : dispatch) result.insert(target);
    return result;
  }
  size_t target(const std::vector<Type> &arguments) const {
    if (uniform_target || selection_inputs) {
      if (arguments.size() != type.parameters.size()) throw std::logic_error("unchecked callable family invocation");
      for (size_t i = 0; i < arguments.size(); ++i)
        if (!scalar_type(arguments[i]) || !transfers_value(arguments[i], type.parameters[i]))
          throw std::logic_error("unchecked callable family invocation");
    }
    if (uniform_target) return *uniform_target;
    const auto found = std::find_if(dispatch.begin(), dispatch.end(), [&](const auto &entry) {
      if (!selection_inputs) return entry.first == arguments;
      return entry.first.size() == arguments.size() && std::all_of(selection_inputs->begin(), selection_inputs->end(),
          [&](size_t input) { return entry.first.at(input) == arguments.at(input); });
    });
    if (found == dispatch.end()) throw std::logic_error("unchecked callable family invocation");
    return found->second;
  }
  std::vector<std::pair<std::vector<Type>, size_t>> alternatives(const std::vector<Type> &arguments) const {
    if (uniform_target || selection_inputs) return {{arguments, target(arguments)}};
    std::vector<std::pair<std::vector<Type>, size_t>> result;
    for (const auto &entry : dispatch) {
      bool admitted = entry.first.size() == arguments.size();
      for (size_t i = 0; admitted && i < arguments.size(); ++i)
        admitted = entry.first[i] == arguments[i] ||
            ((type.argument_modes.empty() || type.argument_modes[i] == ArgumentMode::value) &&
             accepts_exception(entry.first[i], arguments[i]));
      if (admitted) result.push_back(entry);
    }
    if (result.empty()) throw std::logic_error("unchecked callable family invocation");
    return result;
  }
};
struct CheckedSymbol {
  std::optional<Type> type;
  bool intrinsic_print = false;
  std::string name;
};
struct CheckedFamily {
  std::optional<size_t> symbol;
  size_t module, origin;
  Location at;
  std::optional<size_t> imported_from;
  std::vector<size_t> methods;
  std::vector<CheckedMethod> declarations;
  std::string operation;
};

struct CheckedProgram {
  struct Initializer { size_t module; const Expression *expression; };
  std::vector<CheckedSymbol> symbols;
  std::deque<CheckedFamily> families;
  std::vector<QueryEnvironment> query_environments{{}};
  std::vector<CheckedCallable> callables;
  std::deque<CheckedMethod> methods;
  std::deque<CheckedInstance> instances;
  std::map<const Expression *, CheckedOperation> entry;
  std::vector<Initializer> initializers;
  std::vector<size_t> execution_order;
  std::vector<WarningDiagnostic> warnings;
  std::vector<CheckedStorage> entry_storage;
  std::vector<CheckedBinding> entry_bindings;
  bool exception_values = false;
  std::set<size_t> captured_symbols(const CheckedCallable &value) const {
    std::set<size_t> result;
    for (const auto target : value.implementations()) {
      const auto &symbols = instances[target].effects.symbols;
      result.insert(symbols.begin(), symbols.end());
    }
    return result;
  }
};

struct CheckedBody {
  struct Context {
    const CheckedInstance *instance;
    std::optional<std::map<const Expression *, CheckedOperation>> remapped_operations;
    const std::map<const Expression *, CheckedOperation> &operations() const {
      return remapped_operations ? *remapped_operations : instance->operations;
    }
  };
  std::vector<CheckedStorage> storage;
  CheckedFrame frame;
  // Conservative writable exposure of an invocation input. Unexposed saved
  // payloads can be observed without acquiring another complete image.
  std::set<size_t> writable_captures;
  std::vector<Context> contexts;
};

inline CheckedBody checked_body(const CheckedProgram &program, const CheckedMethod &method, std::optional<size_t> instance) {
  const auto instances = instance ? std::vector<size_t>{*instance} : method.instances;
  if (instances.empty()) throw std::logic_error("checked body has no implementation context");
  CheckedBody result;
  std::map<size_t, size_t> places;
  // A lexical declaration has one frame role even when a checked context
  // borrows an existing target and another constructs local payload storage.
  for (const auto id : instances) {
    const auto &checked = program.instances[id];
    const auto retain = [&](const CheckedStorage &place) {
      if (places.emplace(place.binding, result.storage.size()).second) result.storage.push_back(place);
    };
    for (const auto &input : checked.frame.inputs) retain(checked.storage.at(input.place));
    for (const auto capture : checked.frame.captures) retain(checked.storage.at(capture));
    for (const auto &place : checked.storage) retain(place);
  }
  bool frame_ready = false;
  for (const auto id : instances) {
    const auto &checked = program.instances[id];
    const auto remap = [&](size_t place) { return place == null_target ? place : places.at(checked.storage.at(place).binding); };
    auto inputs = checked.frame;
    for (auto &input : inputs.inputs) input.place = remap(input.place);
    for (auto &capture : inputs.captures) capture = remap(capture);
    if (frame_ready && result.frame != inputs) throw std::logic_error("incompatible checked native input roles");
    result.frame = std::move(inputs); frame_ready = true;
    bool unchanged = true;
    for (size_t i = 0; i < checked.storage.size(); ++i) unchanged = unchanged && remap(i) == i;
    auto &context = result.contexts.emplace_back(&checked, std::nullopt);
    for (const auto &[expr, operation] : checked.operations) {
      for (size_t a = 0; a < operation.arguments.size(); ++a) {
        if (operation.arguments[a].mode == ArgumentMode::value) continue;
        const auto &argument = checked.operations.at(expr->children.at(a).get());
        for (size_t c = 0; c < checked.frame.captures.size(); ++c)
          if (!checked.storage.at(checked.frame.captures[c]).read_only &&
              argument.targets.contains(checked.frame.captures[c])) result.writable_captures.insert(c);
      }
    }
    if (unchanged) continue;
    const auto optional = [&](std::optional<size_t> &place) { if (place) place = remap(*place); };
    const auto set = [&](std::set<size_t> &source) {
      std::set<size_t> targets;
      for (const auto place : source) targets.insert(remap(place));
      source = std::move(targets);
    };
    const auto lower = [&](auto &&self, CheckedOperation &operation) -> void {
      optional(operation.place); optional(operation.callable_place); optional(operation.accumulator); optional(operation.captured_place);
      set(operation.targets); set(operation.callable_targets); set(operation.captures);
      for (auto &capture : operation.callable_captures) self(self, capture);
    };
    context.remapped_operations = checked.operations;
    for (auto &[expr, operation] : *context.remapped_operations) lower(lower, operation);
  }
  return result;
}

} // namespace cod::language
