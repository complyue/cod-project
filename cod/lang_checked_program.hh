//===--- lang_checked_program.hh - Checked program and evidence ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_diagnostics.hh"
#include "lang_exceptions.hh"

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
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cod::language {

inline constexpr size_t null_target = std::numeric_limits<size_t>::max();

[[noreturn]] inline void implementation_gap(const Location &at, const std::string &detail) {
  fail(at, "micro-dispatch implementation gap: " + detail + "; this is not a language restriction");
}

inline bool scalar_type(const Type &type) { return numeric(type) || type == Type::boolean || type.kind == Type::enumeration; }
inline bool nominal_exception(const Type &type) {
  return type.kind == Type::exception && nominal_failure(type.identity);
}
inline bool owning_exception(const Type &type) { return type.kind == Type::exception && owning_failure(type.identity); }
inline bool bits_type(const Type &type) { return scalar_type(type) || (type.kind == Type::exception && !owning_exception(type)); }
inline bool lexical_value_type(const Type &type) { return bits_type(type) || type.kind == Type::procedure; }
inline bool inline_aggregate_type(const Type &type) { return type.kind == Type::product || type.kind == Type::sum; }
enum class BuiltinOperation {
  none, unary, binary, identity, array_length, array_transfer, reference_swap, bits_swap, print,
  builder_append, lexical_result, text_size, text_byte, text_slice
};
inline std::optional<BuiltinOperation> text_access(std::optional<cod_lang_export_kind> primitive) {
  if (primitive == COD_LANG_TEXT_SIZE_EXPORT) return BuiltinOperation::text_size;
  if (primitive == COD_LANG_TEXT_BYTE_EXPORT) return BuiltinOperation::text_byte;
  if (primitive == COD_LANG_TEXT_SLICE_EXPORT) return BuiltinOperation::text_slice;
  return {};
}
inline Type text_access_result(BuiltinOperation operation, const Expression &expr) {
  const size_t arity = operation == BuiltinOperation::text_size ? 1 : operation == BuiltinOperation::text_byte ? 2 : 3;
  if (expr.children.size() != arity || expr.has_here_body())
    fail(expr.at, "text access requires a String and its byte offsets, if any");
  if (operation == BuiltinOperation::text_size) return Type::uint64;
  if (operation == BuiltinOperation::text_byte) return Type::uint8;
  return Type::string;
}
// Nominal schema identities for a selected abstract target. An empty set carries no narrowing.
inline bool eager_operation(std::string_view name) {
  return name == "+" || name == "-" || name == "*" || name == "/" || name == "%" || name == "!" ||
      name == "==" || name == "!=" || name == "<" || name == ">" || name == "<=" || name == ">=";
}
struct CheckedArgument {
  ArgumentMode mode;
  Type type;
  bool binding_slot = false;
  struct ReferenceStorage {
    size_t place, owner;
    auto operator<=>(const ReferenceStorage &) const = default;
  };
  std::optional<ReferenceStorage> reference_storage = {};
  std::optional<size_t> current_reference_symbol = {};
  bool operator==(const CheckedArgument &) const = default;
};
inline ArgumentMode argument_mode(const Parameter &parameter) {
  if (parameter.type.kind == Type::exception_holder) return ArgumentMode::owning_exception;
  if (parameter.selects_caller_ref_slot()) return ArgumentMode::borrowed_ref;
  if (parameter.selects_caller_place()) return ArgumentMode::borrowed_ptr;
  if (parameter.owner.empty()) return ArgumentMode::value;
  if (parameter.declaration_kind == "handle") return ArgumentMode::borrowed_handle;
  if (parameter.declaration_kind == "ptr") return ArgumentMode::borrowed_ptr;
  if (parameter.declaration_kind == "ref") return ArgumentMode::borrowed_ref;
  return ArgumentMode::borrowed_read;
}
inline ArgumentMode argument_mode(const Type &view, size_t position) {
  return view.argument_modes.empty() ? ArgumentMode::value : view.argument_modes.at(position);
}
// A here invocation transports one owner identity per role, in first-use order.
inline std::vector<size_t> here_owner_inputs(const Type &invocation) {
  std::vector<size_t> inputs;
  for (size_t i = 0; i + 1 < invocation.owners.size(); ++i)
    if (invocation.owners[i] == inputs.size() + 1) inputs.push_back(i);
  return inputs;
}
inline bool accepts_exception(const Type &source, const Type &target) {
  if (source.kind != Type::exception || target.kind != Type::exception) return false;
  return !target.identity || source == target ||
      (target.identity == error_root_id && source.identity && source.identity < exception_categories.size() &&
       !exception_categories[source.identity].checked);
}
inline bool transfers_bits(const Type &source, const Type &target) {
  return !owning_exception(source) && !owning_exception(target) &&
      (widens(source, target) || supplies_enum_code(source, target) || accepts_exception(source, target));
}
inline bool transfers_value(const Type &from, const Type &to) {
  return from.kind == Type::procedure || to.kind == Type::procedure ? from == to : transfers_bits(from, to);
}
inline bool accepts_argument(const Type &source, const Type &target, ArgumentMode mode) {
  return mode == ArgumentMode::value ? transfers_value(source, target) :
      source == target || (mode == ArgumentMode::borrowed_ref && widens_text(source, target));
}

enum class HereExit : std::uint8_t { break_call = 1, return_supplier, failure };

struct SumConstructor {
  size_t sum;
  size_t alternative;
  bool operator==(const SumConstructor &) const = default;
};

struct CasePattern {
  enum class Kind { any, null, code };
  Kind kind = Kind::any;
  std::uint64_t code = 0;
  std::optional<SumConstructor> constructor = {};
};

struct CheckedOperation {
  Type type;
  Type operand_type;
  std::optional<size_t> symbol;
  std::optional<size_t> scoped_effect;
  bool publishes_effect = true;
  bool reads_direct_body = false;
  std::optional<size_t> callee;
  std::optional<size_t> place;
  std::optional<size_t> product_field;
  std::optional<size_t> exception_owner;
  bool owns_exception_result = false;
  std::optional<size_t> region_holder;
  // Regional product rows contain relative field locators. The lexical
  // controller identifies the actual live region used for projection.
  std::optional<size_t> regional_product_holder;
  std::optional<size_t> regional_sum_holder;
  bool allocates_scalar_field = false;
  std::vector<size_t> region_initializers;
  struct NativeAcquisition {
    size_t constructor;
    const Expression *initializer;
    std::optional<size_t> parent_place = {};
    std::optional<size_t> parent_borrow = {};
    auto operator<=>(const NativeAcquisition &) const = default;
  };
  std::optional<NativeAcquisition> native_acquisition;
  std::optional<size_t> native_call;
  bool rebind_native_handle = false;
  bool reborrows_native_owner = false;
  std::optional<size_t> binding;
  std::set<size_t> targets, owners;
  bool read_only = false;
  bool fixed_binding = false;
  bool view = false;
  bool discarded_view = false;
  // A checked element address or one of its inline fields. No fixed storage ID proves its identity.
  bool indexed_target = false;
  // The result may retain a ptr carrier even if the current target is present.
  bool nullable_view = false;
  std::uint64_t integer = 0;
  bool enum_literal = false;
  bool enum_assertion = false;
  bool enum_decode = false;
  bool check_c_string = false;
  bool check_predicate_assertion = false;
  struct LexicalAllocation {
    size_t owner;
    enum Kind { text, builder_snapshot, builder, text_slice } kind;
    unsigned builder_inputs = 0;
    auto operator<=>(const LexicalAllocation &) const = default;
  };
  std::optional<LexicalAllocation> lexical_allocation;
  struct ArrayAllocator {
    enum Kind { lexical, region, input } kind;
    size_t place = 0;
    auto operator<=>(const ArrayAllocator &) const = default;
  };
  std::optional<ArrayAllocator> array_allocator;
  std::shared_ptr<const std::map<size_t, ArrayAllocator>> array_allocator_arguments;
  std::optional<size_t> lexical_result_owner;
  std::optional<SumConstructor> sum_constructor;
  bool invokes_regional_initializer = false;
  std::vector<size_t> sum_initializers;
  CasePattern case_pattern;
  std::vector<size_t> case_payload_fields;
  bool case_subject_may_be_null = false;
  bool case_reaches_end = false;
  bool case_scope_resumes = false;
  bool case_arm_completes = false;
  bool minimum_literal = false;
  std::set<size_t> captures;
  bool mixed_view = false;
  bool scalar_value_join = false;
  bool borrowed_result = false;
  enum class CheckedRefPresence { proven, possible_absence, definite_absence };
  CheckedRefPresence checked_ref_presence = CheckedRefPresence::proven;
  struct ReturnFilter {
    bool can_match = true;
    std::optional<std::pair<size_t, size_t>> owner_comparison;
    auto operator<=>(const ReturnFilter &) const = default;
  } return_filter;
  BuiltinOperation builtin = BuiltinOperation::none;
  std::string builtin_name; // Selected eager operation, independent of source spelling.
  // Destination calls select every address before checking entry-read promises.
  bool destination_arguments = false;
  std::vector<CheckedArgument> arguments;
  struct ValuePack {
    enum class Operand { fixed, value, spread, empty };
    size_t fixed_count;
    std::vector<Operand> operands;
    auto operator<=>(const ValuePack &) const = default;
  };
  std::optional<ValuePack> value_pack;
  // Checked operand sources can differ from the children of the source call.
  std::optional<std::vector<const Expression *>> argument_sources;
  struct OperandPreparation {
    enum class Kind { ordinary, here } kind;
    size_t slot;
    auto operator<=>(const OperandPreparation &) const = default;
  };
  // An empty schedule uses ordinary parameter order. A supplied body is bound at entry.
  std::vector<OperandPreparation> operand_preparation;
  std::vector<size_t> scope_owners;
  struct PreparedProducer {
    size_t instance;
    std::vector<size_t> owners; // Actual owners in the producer's declared owner order.
    auto operator<=>(const PreparedProducer &) const = default;
  };
  std::optional<PreparedProducer> prepared_producer;
  struct PipelinePreparation {
    std::vector<const Expression *> written;
    // Leaves from a lexical binding can be absent from this use's written operands.
    std::vector<const Expression *> traversal;
    std::optional<size_t> lexical_source;
    auto operator<=>(const PipelinePreparation &) const = default;
  };
  std::optional<PipelinePreparation> pipeline_preparation;
  size_t argument_count(const Expression &expr) const {
    return argument_sources ? argument_sources->size() : expr.children.size();
  }
  const Expression &argument(const Expression &expr, size_t i) const {
    return argument_sources ? *argument_sources->at(i) : *expr.children.at(i);
  }
  struct SuppliedHere {
    size_t instance;
    std::vector<const Expression *> places;
    std::vector<size_t> owners;
    std::vector<size_t> lexical_here;
    struct ReturningProfile {
      size_t body, receiver;
      auto operator<=>(const ReturningProfile &) const = default;
    };
    std::optional<ReturningProfile> returning;
    auto operator<=>(const SuppliedHere &) const = default;
  };
  std::optional<SuppliedHere> supplied_here;
  struct SavedPipeline {
    std::vector<const Expression *> stages;
    const Expression *source = nullptr;
    std::optional<SuppliedHere> terminal;
  };
  std::optional<SavedPipeline> saved_pipeline;
  std::optional<std::vector<const Expression *>> open_pipeline_operands;
  struct HereArgument {
    std::optional<size_t> lexical;
    const Expression *source = nullptr;
    std::optional<PipelinePreparation> traversal = {};
    auto operator<=>(const HereArgument &) const = default;
  };
  std::vector<HereArgument> here_arguments;
  std::optional<size_t> invokes_here;
  std::optional<size_t> opened_source;
  std::optional<size_t> opened_pipeline_stages;
  std::vector<size_t> scope_here;
  std::optional<HereExit> here_exit;
  std::set<size_t> nullable_arguments;
};

struct CheckedStorage {
  std::string name;
  Type type;
  size_t scope;
  bool read_only;
  bool borrowed = false;
  size_t binding = 0;
  bool subobject = false;
  std::vector<size_t> fields = {};
  std::optional<SumConstructor> reserved_alternative = {};
  bool identity_only = false;
};

enum class PlaceKind { value, reference, pointer, mixed };
inline PlaceKind place_kind(bool value, bool exact_reference) {
  return value ? PlaceKind::value : exact_reference ? PlaceKind::reference : PlaceKind::pointer;
}
inline std::string place_kind_name(PlaceKind kind) {
  switch (kind) {
  case PlaceKind::value: return "val";
  case PlaceKind::reference: return "ref";
  case PlaceKind::pointer: return "ptr";
  case PlaceKind::mixed: return "mixed";
  }
  throw std::logic_error("unknown storage place kind");
}

struct CheckedSchema {
  struct Field {
    const Parameter *source;
    Type type;
    PlaceKind storage;
    bool direct_text_place() const { return storage == PlaceKind::value && type == Type::string; }
    bool stores_target() const { return storage != PlaceKind::value || direct_text_place(); }
    bool readonly_target() const { return source->read_only || direct_text_place(); }
  };
  const Form *source;
  std::vector<Field> fields;
  bool bits;
  std::optional<size_t> native_owner = {};
  bool has_enum_storage = false;
  std::optional<size_t> abstract_parent = {};
  bool lexical_builder = false;
  bool has_sum_storage = false;
  bool layout_checked = false;
};

struct CheckedSum {
  struct Alternative {
    const SumAlternative *source;
    std::vector<CheckedSchema::Field> fields;
    std::vector<size_t> initializers = {};
  };
  const Form *source;
  std::vector<Alternative> alternatives;
  bool regional_only = false;
  size_t module = 0;
};

struct CheckedAbstractType {
  const Form *source;
  std::optional<size_t> parent = {};
};

struct CheckedArray {
  const Form *source;
  Type index;
  Type element;
};

struct CheckedNativeInput {
  Type type;
  std::optional<size_t> owner;
};

struct CheckedNativeConstructor {
  const Form *source;
  Type handle;
  std::vector<CheckedNativeInput> inputs;
  std::optional<Type> failure;
  std::optional<Type> parent = {};
  std::optional<size_t> retained_parent = {};
};

struct CheckedNativeEntry {
  size_t method;
  std::vector<CheckedNativeInput> inputs;
  Type result;
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
  // A reference slot has its own lifetime, separate from its selected target.
  std::set<size_t> place_owners = {};
  std::optional<size_t> embedded_slot = {};
  // A fixed alias carries access restrictions and uses its selection's proof state.
  std::optional<size_t> alias = {};
  // A selected alias keeps one physical slot from this finite set for its whole lifetime.
  std::set<size_t> selection = {};
  // A scalar element selected from stable lexical array backing.
  std::optional<size_t> indexed_element_owner = {};
  // Caller-slot inputs can denote the same binding in an activation.
  std::set<size_t> overlaps = {};
  bool nullable = false;
  const Expression *pipeline = nullptr;
  bool direct_text_place = false;
  bool native_borrow_slot = false;
  bool reservation_slot = false;
  PlaceKind kind() const { return place_kind(value_binding, requires_initialized); }
};

enum class ResultMode { value, borrowed_ref, borrowed_ptr, borrowed_handle, owned_region, owned_exception };
struct ResultContract {
  std::optional<Type> type;
  ResultMode mode = ResultMode::value;
  std::string owner = {};
  bool read_only = false;
  bool borrowed() const { return mode == ResultMode::borrowed_ref || mode == ResultMode::borrowed_ptr || native_borrow(); }
  bool nullable() const { return mode == ResultMode::borrowed_ptr; }
  bool native_borrow() const { return mode == ResultMode::borrowed_handle; }
  bool owns_region() const { return mode == ResultMode::owned_region; }
  bool owns_exception() const { return mode == ResultMode::owned_exception; }
  bool operator==(const ResultContract &) const = default;
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
  // One incoming identity per input-bound owner, independent of target carriers.
  std::map<size_t, size_t> owners;
  bool operator==(const CheckedFrame &) const = default;
};
struct CheckedInstance {
  size_t method;
  ResultContract result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
  std::vector<CheckedStorage> storage;
  std::vector<CheckedBinding> bindings;
  CheckedFrame frame;
  // Completed capability arguments for this executable frame. An empty
  // layout is valid. A missing layout cannot supply the LLVM call ABI.
  std::optional<std::set<size_t>> array_allocator_roles = {};
  // The completed snapshot suffix for a publishing scoped handler frame.
  std::set<size_t> scoped_replay_bindings = {};
};
struct CheckedHereContract {
  struct Component {
    std::string label;
    Type type;
    ArgumentMode mode = ArgumentMode::value;
    // Zero denotes the component's invocation owner. Other roles index the receiver's owners from one.
    std::optional<size_t> owner;
    bool read_only = false;
    bool operator==(const Component &) const = default;
  };
  std::vector<CheckedHereContract> here;
  std::vector<Component> inputs;
  bool labelled = false;
  Component result;
  std::optional<std::vector<Component>> feedback;
  // Concrete categories only. A root contract covers the final program catalog.
  std::set<size_t> throws_categories;
  bool throws_root = false;
  bool operator==(const CheckedHereContract &) const = default;
};
struct CheckedHereTarget {
  std::optional<size_t> body, source;
  std::optional<size_t> pipeline = {};
  auto operator<=>(const CheckedHereTarget &) const = default;
};
struct CheckedLexicalHere {
  std::string name;
  Type invocation;
  CheckedHereContract signature;
  CheckedHereTarget target;
  bool operator==(const CheckedLexicalHere &) const = default;
};
struct CheckedMethod {
  const Form *source = nullptr;
  std::string operation; // Semantic operation identity, independent of the local alias.
  size_t module = 0, interpretation = 0, definition = null_target;
  std::optional<SumConstructor> regional_initializer;
  std::optional<size_t> regional_initializer_fields;
  std::optional<size_t> family, local_effect_symbol;
  std::optional<size_t> instance;
  std::vector<Type> parameters;
  ResultContract result;
  struct LexicalResult {
    const Expression *marker;
    size_t owner;
    const Expression *display_operands = nullptr;
    bool marks(const Expression &expr) const { return marker == &expr || display_operands == &expr; }
  };
  std::optional<LexicalResult> lexical_result;
  bool prepares_display_operands() const { return lexical_result && lexical_result->display_operands; }
  std::set<size_t> throws_categories;
  std::vector<Type> throws_types;
  std::vector<std::string> type_parameters;
  std::optional<size_t> parent;
  std::vector<CheckedHereContract> here;
  // Concrete body or prepared producer bound to each formal.
  std::vector<CheckedHereTarget> here_targets;
  struct HereGroupResults {
    std::optional<Type> supplier;
    Type receiving_call;
    std::optional<bool> supplier_return;
    bool binds_return_payload = false;
    bool operator==(const HereGroupResults &) const = default;
  };
  std::optional<HereGroupResults> here_group;
  struct ScopeInputs {
    struct NamedEntry {
      size_t explicit_parameters;
      bool operator==(const NamedEntry &) const = default;
    };
    std::optional<NamedEntry> named;
    std::vector<Parameter> parameters;
    std::vector<OwnerParameter> owners;
    std::vector<CheckedLexicalHere> lexical_here;
    struct HereReception {
      size_t offered;
      std::vector<size_t> selected;
      auto operator<=>(const HereReception &) const = default;
    };
    std::optional<HereReception> here_reception;
    // The entry receives these owners at each handoff. Remaining owners and
    // live places come from the supplier's activation.
    size_t here_input_owners = 0;
    // A reference input carries the supplier's binding slot.
    std::set<size_t> live_bindings;

  };
  std::optional<ScopeInputs> scope_inputs;
  size_t here_inputs() const {
    return here.size() + (scope_inputs ? scope_inputs->lexical_here.size() : 0);
  }
  bool has_higher_order_here() const {
    return std::ranges::any_of(here, [](const auto &formal) { return !formal.here.empty(); });
  }
  std::span<const Parameter> input_parameters() const {
    if (scope_inputs) return scope_inputs->parameters;
    return std::span<const Parameter>(source->parameters).subspan(here.size());
  }
  const std::vector<OwnerParameter> &input_owners() const {
    return scope_inputs ? scope_inputs->owners : source->owner_parameters;
  }
};
struct CheckedSymbol {
  std::optional<Type> type;
  bool intrinsic_print = false;
  std::string name;
  std::optional<SymbolReference> reference = {};
  bool live_scope = false;
  std::optional<size_t> scoped_instance = {};
  std::optional<Location> declaration = {};
};
// Borrowed symbol inputs occupy a separate range from scope and formal owner roles.
inline constexpr size_t first_symbol_owner = null_target / 2;
inline size_t symbol_owner_key(size_t symbol) { return first_symbol_owner + symbol; }
inline bool is_symbol_owner(size_t owner, size_t symbols) {
  return owner >= first_symbol_owner && owner - first_symbol_owner < symbols;
}
struct CheckedFamily {
  std::optional<size_t> symbol;
  size_t module, origin;
  Location at;
  std::optional<size_t> imported_from;
  std::vector<size_t> methods;
  std::vector<CheckedMethod> declarations;
  std::string operation;
};

struct CheckedNativeOperation {
  CheckedMethod contract;
  std::vector<CheckedNativeInput> inputs;
  bool has_here() const { return !contract.here.empty(); }
  const Type &here_handle() const { return inputs.front().type; }
  const auto &arrays() const { return contract.here.front().inputs; }
};

struct EnumValue {
  Type type;
  size_t member;
  bool operator==(const EnumValue &) const = default;
};

struct CheckedEnum {
  struct Member {
    std::uint64_t code;
    size_t canonical;
  };
  const Form *source;
  Type backing;
  std::vector<Member> members;
  std::map<std::string, size_t> spellings;
};

struct CheckedProgram {
  bool deferred_aggregates = false;
  std::vector<CheckedExceptionCategory> exception_types = intrinsic_exception_types();
  Type exception_type(size_t category) const {
    return {Type::exception, std::string(exception_types.at(category).name), category};
  }
  std::string exception_display_name(size_t category, std::string_view spelling = {}) const {
    const auto &type = exception_types.at(category);
    if (!type.schema) return std::string(type.name);
    const auto &definition = *schemas.at(*type.schema).source;
    const auto canonical = definition.at.filename() + "::" + std::string(type.name);
    if (spelling.empty()) return canonical;
    return std::string(spelling) + " (canonical " + canonical + ")";
  }
  Type append_exception_type(const Form &source, bool checked) {
    const auto category = exception_types.size();
    exception_types.push_back({source.name, checked, schemas.size()});
    schemas.push_back({&source, {}, false});
    return exception_type(category);
  }
  std::vector<CheckedSchema> schemas;
  std::vector<CheckedSum> sums;
  std::vector<CheckedAbstractType> abstract_types;
  std::vector<CheckedArray> arrays;
  std::vector<CheckedEnum> enums;
  std::vector<CheckedNativeConstructor> native_constructors;
  std::vector<CheckedNativeEntry> native_entries;
  std::vector<CheckedNativeOperation> native_operations;
  std::vector<CheckedSymbol> symbols;
  std::deque<CheckedFamily> families;
  std::vector<std::unique_ptr<Expression>> scope_operands;
  std::vector<Expr> generated_here_bodies;
  std::deque<CheckedMethod> methods;
  std::deque<CheckedInstance> instances;
  bool exception_values = false;
  bool builder_type(const Type &type) const {
    return type.kind == Type::product && schemas.at(type.identity).lexical_builder;
  }
  bool bits_payload(const Type &type) const {
    return scalar_type(type) ||
        (type.kind == Type::sum && !sums.at(type.identity).regional_only) ||
        (type.kind == Type::product && schemas.at(type.identity).bits);
  }
  template <typename Visit>
  void visit_inline_fields(const Type &type, const Visit &visit) const {
    if (type.kind == Type::product) {
      for (const auto &field : schemas.at(type.identity).fields) if (!field.stores_target()) visit(field);
    } else {
      for (const auto &alternative : sums.at(type.identity).alternatives)
        for (const auto &field : alternative.fields) if (!field.stores_target()) visit(field);
    }
  }
  bool conforms_to(const Type &actual, const Type &bound) const {
    if (actual == bound) return true;
    if (bound.kind != Type::abstract_type) return false;
    std::optional<size_t> parent;
    if (actual.kind == Type::abstract_type) parent = abstract_types.at(actual.identity).parent;
    else if (actual.kind == Type::product) parent = schemas.at(actual.identity).abstract_parent;
    while (parent) {
      if (*parent == bound.identity) return true;
      parent = abstract_types.at(*parent).parent;
    }
    return false;
  }

};

// Primitive bits and enum bits are stored without the transport envelope.
// Enum formats retain the domain needed to recover their width and tag.
struct StorageFormat {
  Type::Kind kind = Type::void_type;
  size_t enum_identity = 0;
  StorageFormat() = default;
  explicit StorageFormat(const Type &type)
      : kind(scalar_type(type) ? type.kind : Type::void_type),
        enum_identity(type.kind == Type::enumeration ? type.identity : 0) {}
  auto operator<=>(const StorageFormat &) const = default;
  Type type() const { return {kind, {}, enum_identity}; }
};

} // namespace cod::language
