//===--- lang_seed_bodies.hh - Extract executable seed bodies ---------===//
#pragma once

#include "lang_checked.hh"

#include <numeric>
#include <variant>

namespace cod::language::seed {

class BodyExtraction;

// The backend receives the executable rows while their parsed revision is
// still owned by the result. Only the adapter can construct this result.
class ExecutableProgram {
  friend class BodyExtraction;
  SourceProgram source_;
  CheckedProgram program_;

  ExecutableProgram(SourceProgram source, CheckedProgram program)
      : source_(std::move(source)), program_(std::move(program)) {}

public:
  const CheckedProgram &program() const { return program_; }
  const SourceProgram &source() const { return source_; }
};

// Completion reads only these extracted facts. It has no body or frame visitor.
struct BodyDependencies {
  using Role = std::pair<size_t, size_t>;
  struct Requirements {
    std::set<size_t> allocations;
    bool open_replay;
  };
  std::vector<Requirements> direct;
  std::map<Role, std::vector<Role>> allocating_callers;
  std::vector<std::vector<size_t>> replay_callers;

  std::vector<Requirements> complete() const {
    auto result = direct;
    std::vector<Role> roles;
    for (size_t body = 0; body < direct.size(); ++body)
      for (const auto role : direct[body].allocations) roles.emplace_back(body, role);
    for (size_t next = 0; next < roles.size(); ++next) {
      const auto callers = allocating_callers.find(roles[next]);
      if (callers == allocating_callers.end()) continue;
      for (const auto &[body, role] : callers->second)
        if (result[body].allocations.insert(role).second) roles.emplace_back(body, role);
    }

    // Replay follows even the edges that supply an allocator locally.
    std::vector<size_t> open;
    for (size_t body = 0; body < replay_callers.size(); ++body)
      if (direct[body].open_replay) open.push_back(body);
    for (size_t next = 0; next < open.size(); ++next)
      for (const auto caller : replay_callers[open[next]])
        if (!std::exchange(result[caller].open_replay, true)) open.push_back(caller);
    return result;
  }
};

class BodyExtraction {
  using Allocator = CheckedOperation::ArrayAllocator;
  using Declarations = ScalarChecker::ModuleDeclarations;

  struct BoundCall {
    const Expression *site;
    size_t callee;
    std::map<size_t, Allocator> owners;
    bool installs_handler = false;
  };
  struct Local {
    size_t binding;
    std::optional<size_t> place;
    size_t owner;
  };
  struct LocalProcedure { size_t definition; };
  using Binding = std::variant<Local, LocalProcedure>;
  struct HereFormal {
    const CheckedHereContract *contract;
    size_t owner, slot;
  };
  struct Capture {
    const Expression *use;
    Local source;
    Parameter parameter;
  };
  struct HereSupply {
    const Expression *call;
    Type receiving_result;
    size_t owner;
    std::vector<CheckedLexicalHere> lexical_here;
    std::vector<size_t> here_sources;
  };
  struct Outline {
    const Form *source;
    size_t supplier, module;
    std::map<std::string, Binding> visible;
    std::map<std::string, HereFormal> visible_here;
    std::vector<size_t> owners;
    std::vector<OwnerParameter> owner_parameters;
    std::vector<Capture> captures;
    std::vector<Parameter> parameters;
    std::optional<HereSupply> here;
    std::optional<size_t> handler;
  };
  struct LocalDefinition {
    Outline outline;
    Type result;
    std::optional<size_t> body;
  };
  struct BoundBody {
    CheckedInstance instance;
    std::set<size_t> allocations;
    std::vector<BoundCall> calls;
    std::optional<Outline> outline;
    bool open_replay = false;
  };

  const Declarations &declarations_;
  std::map<size_t, size_t> selected_;
  std::deque<BoundBody> bodies_;
  std::vector<LocalDefinition> local_definitions_;
  std::set<size_t> handler_layouts_;
  size_t outlined_methods_ = 0;

  explicit BodyExtraction(const Declarations &declarations) : declarations_(declarations) {}

  static void expect(bool condition, const Location &at, const char *detail) {
    if (!condition) implementation_gap(at, detail);
  }

  static size_t input_owner(size_t role) { return null_target - 1 - role; }

  static size_t here_input_owner(const CheckedHereContract &contract, const Location &at) {
    expect(!contract.labelled && !contract.inputs.empty() && contract.here.empty() &&
           !contract.feedback && !contract.throws_root && contract.throws_categories.empty() &&
           !contract.result.owner && contract.result.mode == ArgumentMode::value && scalar_type(contract.result.type),
           at, "replacement seed needs ordered here offers and independent scalar feedback");
    std::optional<size_t> owner;
    for (const auto &offer : contract.inputs) {
      if (offer.mode == ArgumentMode::value && !offer.owner && scalar_type(offer.type)) continue;
      expect(offer.type.kind == Type::array && offer.mode == ArgumentMode::borrowed_ref &&
             offer.read_only && offer.owner && *offer.owner && (!owner || owner == offer.owner),
             at, "replacement seed needs independent scalar offers or readonly arrays from one established here owner");
      owner = offer.owner;
    }
    expect(owner.has_value(), at, "replacement seed needs an established here input owner");
    return *owner;
  }

  static Type invocation_type(const CheckedHereContract &contract) {
    Type type{Type::procedure};
    type.pure_callable = type.value_result = true;
    for (const auto &offer : contract.inputs) {
      type.parameters.push_back(offer.type);
      type.argument_modes.push_back(offer.mode);
      type.const_views.push_back(offer.read_only);
      type.owners.push_back(offer.owner ? 1 : 0);
    }
    type.owners.push_back(0);
    type.target = std::make_shared<Type>(contract.result.type);
    return type;
  }

  static CheckedOperation place_operand(const CheckedInstance &instance, const Local &local) {
    const auto &binding = instance.bindings.at(local.binding);
    CheckedOperation op;
    op.type = binding.type;
    op.binding = local.binding;
    op.place = local.place;
    op.owners.insert(local.owner);
    op.view = true;
    op.read_only = binding.read_only;
    op.fixed_binding = binding.fixed;
    return op;
  }

  size_t select_body(size_t method_id, const Location &at) {
    const auto &method = declarations_.method(method_id);
    const auto &source = *method.source;
    expect(source.body && !source.implicit_scope && !source.supplied_scope && !source.supplied_group &&
           !source.case_scan_scope && !source.symbolic && !method.interpretation && !method.local_effect_symbol &&
           !method.parent && !method.regional_initializer && method.operation.empty() &&
           method.here.size() <= 1 && method.type_parameters.empty() && source.captures.empty() &&
           method.result.type &&
           (method.result.owns_region() || (method.result.mode == ResultMode::value &&
            (scalar_type(*method.result.type) || method.result.type == Type::void_type))),
           at, "replacement seed needs a fixed ordinary callable body");
    const auto [row, first] = selected_.try_emplace(method_id, bodies_.size());
    if (first) {
      auto &instance = bodies_.emplace_back().instance;
      instance.method = method_id;
      instance.arguments = method.parameters;
      instance.context = method.parameters;
      instance.result = method.result;
    }
    return row->second;
  }

  void add_outline(Outline outline, const Type &result) {
    auto &body = bodies_.emplace_back();
    body.instance.method = declarations_.declarations_.methods.size() + outlined_methods_++;
    body.instance.result.type = result;
    body.outline = std::move(outline);
  }

  void add_literal(Outline outline, const Expression &site, const CheckedHereContract &contract) {
    const auto &offers = contract.inputs;
    expect(outline.source->supplied_group && !site.scoped && site.receiving &&
           site.receiving->shape == ReceivingHeader::Shape::ordered && !site.receiving->from_for &&
           site.definition->parameters.size() == offers.size(),
           site.at, "replacement seed needs a single-brace here body receiving every offer in order");
    outline.parameters = site.definition->parameters;
    outline.owners.push_back(outline.here->owner);
    const std::string handed_owner = "$here.owner.0";
    outline.owner_parameters.push_back({handed_owner, {}, site.at, false, true});
    for (size_t i = 0; i < offers.size(); ++i) {
      auto &parameter = outline.parameters[i];
      expect(parameter.type == Type::void_type && parameter.declaration_kind.empty() && parameter.owner.empty() &&
             !parameter.read_only && !parameter.fixed_binding, parameter.at,
             "replacement seed needs an inferred here reception");
      const auto &offer = offers[i];
      parameter.type = offer.type;
      parameter.read_only = offer.read_only;
      if (offer.owner) {
        parameter.declaration_kind = "ref";
        parameter.fixed_binding = true;
        parameter.owner = handed_owner;
      } else parameter.declaration_kind = "val";
    }
    add_outline(std::move(outline), contract.result.type);
  }

  class Frame {
    BodyExtraction &extraction_;
    BoundBody &body_;
    size_t id_;
    const CheckedMethod *method_;
    const Form &form_;
    size_t module_;
    struct LexicalScope {
      std::map<std::string, Binding> names;
      std::set<size_t> handlers;
    };
    std::vector<LexicalScope> scopes_{1};
    std::map<std::string, HereFormal> here_;
    struct ActiveCatch { size_t place, protected_depth; };
    std::vector<ActiveCatch> catches_;
    size_t protected_depth_ = 0;
    std::map<size_t, size_t> writable_regions_;
    bool in_iteration_ = false;
    using Kind = Expression::Kind;

    const Binding *find(const std::string &name) const {
      for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope)
        if (const auto found = scope->names.find(name); found != scope->names.end()) return &found->second;
      return nullptr;
    }

    bool supplied_body() const { return body_.outline && body_.outline->here; }

    Outline outline(const Form &source) const {
      Outline result{&source, id_, module_, {}, here_, {}, {}, {}, {}, {}, {}};
      for (const auto &scope : scopes_)
        for (const auto &[name, binding] : scope.names) result.visible.insert_or_assign(name, binding);
      return result;
    }

    size_t storage(std::string name, const Type &type, size_t owner, bool read_only, bool borrowed,
                   std::optional<size_t> binding = {}) {
      auto &rows = body_.instance.storage;
      const auto place = rows.size();
      // Construction scratch has a frame role even when it has no source name.
      if (!binding) {
        binding = body_.instance.bindings.size();
        body_.instance.bindings.push_back({type, read_only, true, true});
      }
      rows.push_back({std::move(name), type, owner, read_only, borrowed, *binding});
      return place;
    }

    Local declare(const Location &at, const std::string &name, const Type &type, bool value,
                  bool read_only, bool fixed, size_t owner, bool input = false, bool capture = false) {
      expect(std::ranges::find(owners(), name, &OwnerParameter::name) == owners().end(),
             at, "replacement seed has no lexical shadow of an owner name");
      const auto id = body_.instance.bindings.size();
      auto &binding = body_.instance.bindings.emplace_back(type, read_only, value, fixed);
      binding.requires_initialized = !value;
      const auto place = value || input ? std::optional{storage(name, type, owner, read_only, input && owner != 0, id)}
                                        : std::nullopt;
      auto &scope = capture ? scopes_.front() : scopes_.back();
      const Local local{id, place, owner};
      const auto inserted = scope.names.emplace(name, local).second;
      expect(inserted, at, "duplicate name in the replacement seed frame");
      return local;
    }

    size_t owner_role(const std::string &name, const Location &at) const {
      const auto &parameters = owners();
      const auto found = std::ranges::find(parameters, name, &OwnerParameter::name);
      expect(found != parameters.end() && !found->root && !found->explicit_parameter,
             at, "replacement seed needs an input-bound owner");
      return static_cast<size_t>(found - parameters.begin());
    }

    const std::vector<OwnerParameter> &owners() const {
      return method_ ? method_->input_owners() : body_.outline->owner_parameters;
    }

    bool const_input_owner(size_t owner) const {
      return owner < null_target && owner >= null_target - owners().size() &&
             owners().at(null_target - 1 - owner).read_only;
    }

    std::optional<Allocator> allocator_for(size_t owner) const {
      if (!owner) return Allocator{Allocator::lexical};
      if (const auto region = writable_regions_.find(owner); region != writable_regions_.end())
        return Allocator{Allocator::region, region->second};
      if (owner > first_symbol_owner && !const_input_owner(owner))
        return Allocator{Allocator::input, null_target - 1 - owner};
      return {};
    }

    const auto &names() const { return extraction_.declarations_.modules_.at(module_); }
    const CheckedProgram &declarations() const { return extraction_.declarations_.declarations_; }

    const CheckedFamily *family(const std::string &name) const {
      return extraction_.declarations_.ordinary_family(module_, name);
    }

    void confirm_intrinsic(const Expression &expr, const std::string &name) const {
      expect(expr.intrinsic || (!family(name) && !find(name) && !names().native_operations.contains(name)),
             expr.at, "replacement seed needs a builtin operation without an unresolved override");
    }

    const CheckedOperation &operand(const Expression &expr, size_t index) {
      return extract(*expr.children.at(index));
    }

    static void takes_value(const CheckedOperation &value, const Type &to, const Location &at) {
      expect(scalar_type(value.type) && transfers_value(value.type, to),
             at, "replacement seed needs an admitted value transfer");
    }

    bool ordinary_loan(const Parameter &parameter, ArgumentMode mode, const Type &type) const {
      if (type == Type::string)
        return mode == ArgumentMode::borrowed_ref && parameter.read_only &&
               !parameter.owner.empty() && !parameter.selects_caller_place();
      if (declarations().builder_type(type))
        return (mode == ArgumentMode::borrowed_ref || mode == ArgumentMode::borrowed_read) &&
               !parameter.selects_caller_place();
      return (mode == ArgumentMode::borrowed_read || mode == ArgumentMode::borrowed_ref) &&
             (type.kind == Type::array || scalar_type(type) ||
              (inline_aggregate_type(type) && declarations().bits_payload(type)));
    }

    void takes_payload(const CheckedOperation &value, const Type &to, const Location &at) const {
      bits_payload_type(declarations(), value.type, to, at);
      expect(!inline_aggregate_type(to) || value.view, at, "aggregate bits transfer needs a source view");
    }

    static size_t single_owner(const CheckedOperation &view, const Location &at) {
      expect(view.view && view.owners.size() == 1, at, "replacement seed needs a view with one established owner");
      return *view.owners.begin();
    }

    void check_slot(const CheckedOperation &operation, const Location &at) const {
      expect(operation.binding && !body_.instance.bindings.at(*operation.binding).value_binding &&
             !operation.fixed_binding, at, "replacement seed needs a writable lexical reference slot");
    }

    void bind_input(const Parameter &parameter, const Type &type, bool capture = false) {
      const auto mode = argument_mode(parameter);
      const bool slot = parameter.selects_caller_ref_slot();
      const bool native_handle = mode == ArgumentMode::borrowed_handle && type.kind == Type::native_handle;
      expect(!parameter.not_init_ref && !parameter.variadic && !parameter.automatic_conversion &&
             (!parameter.selects_caller_place() || slot) &&
             (mode == ArgumentMode::value ? scalar_type(type) : native_handle || ordinary_loan(parameter, mode, type)),
             parameter.at, "replacement seed needs a fixed scalar, array, bits aggregate, text or native handle formal");
      auto &frame = body_.instance.frame;
      const auto owner = parameter.owner.empty() ? size_t{0} : input_owner(owner_role(parameter.owner, parameter.at));
      if (owner) frame.owners.try_emplace(owner, frame.inputs.size());
      const bool value = mode != ArgumentMode::borrowed_ref && !native_handle;
      const auto local = declare(parameter.at, parameter.name, type, value,
                                 parameter.read_only || (mode == ArgumentMode::value && parameter.fixed_binding),
                                 parameter.fixed_binding || native_handle || type == Type::string, owner, true, capture);
      frame.inputs.push_back({*local.place, mode, parameter.selects_caller_place()});
    }

    void bind_capture(const Expression &expr, Local original) {
      auto &outlined = *body_.outline;
      const auto &supplier = extraction_.bodies_.at(outlined.supplier).instance;
      const auto &source = supplier.bindings.at(original.binding);
      const bool live_view = source.type.kind == Type::array ||
          (supplied_body() && source.type.kind == Type::product && declarations().bits_payload(source.type));
      const bool handle = source.type.kind == Type::native_handle && !source.type.owned;
      const bool text = source.type == Type::string && source.read_only;
      const bool incoming_loan = (handle || text) && original.place && original.owner > first_symbol_owner &&
          std::ranges::any_of(supplier.frame.inputs, [&](const auto &input) {
            return input.place == *original.place && !input.binding_slot &&
                   input.mode == (handle ? ArgumentMode::borrowed_handle : ArgumentMode::borrowed_ref);
          });
      expect(!source.nullable &&
             (live_view || incoming_loan || (scalar_type(source.type) && (source.value_binding || source.fixed))),
             expr.at, "replacement seed needs an admitted scalar place, bits view or incoming text or handle loan in a lexical capture");
      if (outlined.handler && scalar_type(source.type))
        expect(source.read_only || (source.fixed && source.requires_initialized), expr.at,
               "replacement seed needs a readonly scalar or fixed scalar reference capture in a live handler");
      expect(!original.owner || original.owner > first_symbol_owner, expr.at,
             "replacement seed has no capture of a locally owned region view");
      const auto known = std::ranges::find(outlined.owners, original.owner);
      const auto role = static_cast<size_t>(known - outlined.owners.begin());
      if (known == outlined.owners.end()) {
        const auto &method = extraction_.declarations_.method(supplier.method);
        const bool read_only = original.owner && method.input_owners().at(null_target - 1 - original.owner).read_only;
        outlined.owners.push_back(original.owner);
        outlined.owner_parameters.push_back({"$scope.owner." + std::to_string(role), {}, expr.at, false, read_only});
      }
      Parameter parameter{expr.text, source.type, expr.at, false, {}};
      parameter.declaration_kind = handle ? "handle" : "ref";
      parameter.owner = outlined.owner_parameters.at(role).name;
      parameter.read_only = source.read_only;
      parameter.fixed_binding = true;
      // A borrowed array or product can live in a mutable supplier slot. Read that slot
      // at execution, while keeping the outlined body's own access target-only.
      if (live_view && !source.value_binding) {
        parameter.caller_place = Parameter::CallerPlace::reference_slot;
        if (outlined.handler) parameter.fixed_binding = source.fixed;
      }
      bind_input(parameter, parameter.type, true);
      body_.instance.arguments.push_back(parameter.type);
      outlined.captures.push_back({&expr, original, std::move(parameter)});
    }

    CheckedOperation name(const Expression &expr) {
      const auto *binding = find(expr.text);
      if (!binding && body_.outline) {
        const auto found = body_.outline->visible.find(expr.text);
        if (found != body_.outline->visible.end()) {
          const auto *source = std::get_if<Local>(&found->second);
          expect(source, expr.at, "replacement seed needs a direct lexical procedure call");
          bind_capture(expr, *source);
          binding = find(expr.text);
        }
      }
      if (!binding) {
        expect(!here_.contains(expr.text) && (!body_.outline || !body_.outline->visible_here.contains(expr.text)),
               expr.at, "replacement seed needs a direct here invocation");
        const auto selected = names().enum_members.find(expr.text);
        expect(selected != names().enum_members.end(), expr.at, "replacement seed needs a bound operand");
        return checked_enum_literal(declarations(), selected->second);
      }
      const auto *local = std::get_if<Local>(binding);
      expect(local, expr.at, "replacement seed needs a direct lexical procedure call");
      auto op = place_operand(body_.instance, *local);
      if (op.type.kind == Type::native_handle && op.type.owned) {
        op.type.owned = false;
        op.reborrows_native_owner = true;
      }
      return op;
    }

    bool visible_local(const std::string &name) const {
      return find(name) || (body_.outline && body_.outline->visible.contains(name));
    }

    const HereFormal *find_here(const Expression &expr) {
      if (const auto current = here_.find(expr.text); current != here_.end()) return &current->second;
      if (!body_.outline) return nullptr;
      const auto found = body_.outline->visible_here.find(expr.text);
      if (found == body_.outline->visible_here.end()) return nullptr;
      expect(supplied_body(), expr.at, "replacement seed has no here formal capture in a lexical procedure");
      auto &here = *body_.outline->here;
      const auto &source = found->second;
      expect(source.owner == here.owner, expr.at, "borrowed here input has a different actual owner from its contract");
      auto rebased = *source.contract;
      for (auto &input : rebased.inputs) if (input.owner) input.owner = 1;
      const auto slot = here.lexical_here.size();
      here.lexical_here.push_back({expr.text, invocation_type(rebased), std::move(rebased), {}});
      here.here_sources.push_back(source.slot);
      return &here_.emplace(expr.text, HereFormal{source.contract, input_owner(0), slot}).first->second;
    }

    CheckedOperation invoke_here(const Expression &expr, const HereFormal &formal) {
      const auto &offers = formal.contract->inputs;
      expect(!expr.has_here_body() && expr.children.size() == offers.size(),
             expr.at, "here call requires an argument for every ordered offer");
      CheckedOperation operation;
      operation.type = formal.contract->result.type;
      operation.operand_type = invocation_type(*formal.contract);
      operation.invokes_here = formal.slot;
      for (size_t i = 0; i < offers.size(); ++i) {
        const auto &input = operand(expr, i);
        const auto &offer = offers[i];
        const auto &at = expr.children[i]->at;
        if (offer.mode == ArgumentMode::value) takes_value(input, offer.type, at);
        else {
          expect(input.type == offer.type && input.view, at, "here call needs the declared nominal array view");
          if (single_owner(input, at) != formal.owner)
            fail(at, "borrowed here input has a different actual owner from its contract");
        }
        operation.arguments.push_back({offer.mode, offer.type});
      }
      return operation;
    }

    void supply_here(const Expression &call, const CheckedHereContract &contract, const Type &receiving_result,
                     size_t owner, CheckedOperation &operation) {
      expect(!body_.outline, call.at, "replacement seed does not supply a here body from an outlined body");
      const auto &site = *call.children.back();
      expect(bool(site.definition), site.at, "replacement seed needs a supplied here definition");
      auto outlined = outline(*site.definition);
      outlined.here = HereSupply{&call, receiving_result, owner, {}, {}};
      extraction_.add_literal(std::move(outlined), site, contract);
      operation.argument_sources.emplace();
      for (const auto &operand : std::span(call.children).first(call.children.size() - 1))
        operation.argument_sources->push_back(operand.get());
    }

    CheckedOperation eager(const Expression &expr) {
      CheckedOperation op;
      std::vector<Type> types;
      for (const auto &arg : expr.children) types.push_back(extract(*arg).type);
      if (expr.text == "&&" || expr.text == "||") {
        expect(types.size() == 2 && types[0] == Type::boolean && types[1] == Type::boolean,
               expr.at, "logical operations require Boolean inputs");
        op.type = Type::boolean;
        return op;
      }
      expect(eager_operation(expr.text), expr.at, "replacement seed needs a supported scalar operator");
      confirm_intrinsic(expr, expr.text);
      const auto types_resolved = eager_types(expr.text, types);
      if (!types_resolved.error.empty()) fail(expr.at, types_resolved.error);
      op.type = types_resolved.result;
      op.operand_type = types_resolved.operand;
      op.builtin = types_resolved.unary ? BuiltinOperation::unary : BuiltinOperation::binary;
      op.builtin_name = expr.text;
      return op;
    }

    static size_t native_loan(const CheckedOperation &actual, const Type &marker, const Location &at) {
      if (marker.kind != Type::c_type_marker || !matches_native_handle(actual.type, marker) || actual.type.owned)
        fail(at, "expected a native handle borrow of type '" + marker.name + "'");
      return single_owner(actual, at);
    }

    void bind_native_owner(std::map<size_t, size_t> &owners, size_t role, size_t owner, const Location &at) const {
      const auto [first, inserted] = owners.emplace(role, owner);
      if (inserted) return;
      // Each operand selects its own live descriptor. The symbol ID does not
      // certify the equality of the selected owner words.
      expect(!is_symbol_owner(first->second, declarations().symbols.size()) &&
             !is_symbol_owner(owner, declarations().symbols.size()), at,
             "replacement seed needs separate C owner roles for dynamic handle loans");
      if (first->second != owner) fail(at, "C arguments for one owner need the same actual owner");
    }

    std::string symbol_name(const Expression &expr, const CheckedSymbol &symbol) const {
      auto result = "symbol '@" + expr.text + "'";
      if (symbol.declaration && symbol.name != expr.text)
        result += " (canonical " + symbol.declaration->filename() + "::@" + symbol.name + ")";
      return result;
    }

    size_t selected_symbol(const Expression &expr) const {
      if (visible_local(expr.text) || here_.contains(expr.text) ||
          (body_.outline && body_.outline->visible_here.contains(expr.text)) ||
          names().enum_members.contains(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not a symbol");
      const auto selected = names().symbols.find(expr.text);
      if (selected == names().symbols.end()) fail(expr.at, "unknown symbol '@" + expr.text + "'");
      const auto &symbol = declarations().symbols.at(selected->second);
      if (form_.kind == Form::Kind::function)
        fail(expr.at, "a function cannot access dynamic bindings through " + symbol_name(expr, symbol));
      return selected->second;
    }

    size_t owning_symbol(const Expression &expr) const {
      const auto id = selected_symbol(expr);
      const auto &symbol = declarations().symbols.at(id);
      expect(!supplied_body(), expr.at, "replacement seed has no owning dynamic effects in a supplied body");
      if (symbol.reference || !symbol.type || symbol.type->kind != Type::native_handle || !symbol.type->owned)
        implementation_gap(expr.at, "replacement seed needs an owning native handle " + symbol_name(expr, symbol));
      return id;
    }

    const CheckedOperation &native_operand(const Expression &expr, size_t index, const Type &formal) {
      const auto &source = *expr.children.at(index);
      if (formal.kind != Type::c_type_marker || source.kind != Kind::symbol) return extract(source);
      const auto id = owning_symbol(source);
      const auto &symbol = declarations().symbols.at(id);
      if (!matches_native_handle(*symbol.type, formal))
        fail(source.at, symbol_name(source, symbol) + " cannot supply a native handle borrow of type '" + formal.name + "'");
      CheckedOperation op;
      op.symbol = id;
      op.type = native_handle_type(*symbol.type, false);
      op.view = true;
      op.owners.insert(symbol_owner_key(id));
      return remember(source, std::move(op));
    }

    CheckedOperation provider_call(const Expression &expr, size_t id) {
      const auto &native = declarations().native_operations.at(id);
      const auto &form = *native.contract.source;
      expect(native.has_here() == expr.has_here_body(), expr.at, "arguments do not match the C provider's here declaration");
      expect(expr.children.size() == native.inputs.size() + expr.has_here_body(),
             expr.at, "arguments do not match the C provider declaration");
      check_native_invocation_kind(&form_, form, expr.at);
      CheckedOperation op;
      op.type = *native.contract.result.type;
      op.native_call = id;
      std::map<size_t, size_t> owners;
      for (size_t i = 0; i < native.inputs.size(); ++i) {
        const auto &input = native.inputs[i];
        const auto &at = expr.children[i]->at;
        const auto &actual = native_operand(expr, i, input.type);
        if (!input.owner) {
          takes_value(actual, input.type, at);
          op.arguments.push_back({ArgumentMode::value, input.type});
          continue;
        }
        const bool handle = input.type.kind == Type::c_type_marker;
        if (!handle)
          expect(actual.type == input.type &&
                 (input.type.kind == Type::array || (text_type(input.type) && actual.read_only)),
                 at, "C provider needs a loan of the declared type and access");
        const auto owner = handle ? native_loan(actual, input.type, at) : single_owner(actual, at);
        const auto &role = form.owner_parameters.at(*input.owner);
        if (!role.read_only && (actual.read_only || (handle ? const_input_owner(owner) : !allocator_for(owner))))
          fail(at, "readonly loan cannot supply a writable C provider owner");
        bind_native_owner(owners, *input.owner, owner, at);
        op.arguments.push_back({handle ? ArgumentMode::borrowed_handle : ArgumentMode::borrowed_ref, actual.type});
      }
      if (native.has_here()) {
        const auto &contract = native.contract.here.front();
        const auto owner = owners.find(here_input_owner(contract, expr.at) - 1);
        expect(owner != owners.end(), expr.at, "native here loan needs the prepared actual owner");
        supply_here(expr, contract, op.type, owner->second, op);
      }
      return op;
    }

    bool text_piece(const Type &type) const {
      return text_type(type) || declarations().builder_type(type);
    }

    CheckedOperation append_text(const Expression &expr) {
      expect(expr.children.size() == 2 && !expr.has_here_body(), expr.at,
             "text append requires a builder and one text piece");
      const auto &target = operand(expr, 0);
      const auto owner = single_owner(target, expr.children[0]->at);
      if (!declarations().builder_type(target.type) || target.read_only || const_input_owner(owner))
        fail(expr.children[0]->at, "text append requires writable builder and owner access");
      const auto &piece = operand(expr, 1);
      if (!text_piece(piece.type)) fail(expr.children[1]->at, "text append requires a text piece");
      single_owner(piece, expr.children[1]->at);
      CheckedOperation op;
      op.builtin = BuiltinOperation::builder_append;
      return op;
    }

    CheckedOperation call_local(const Expression &expr, const LocalProcedure &local, bool region_receiver) {
      auto &known = extraction_.local_definitions_.at(local.definition);
      if (!known.body) {
        known.body = extraction_.bodies_.size();
        extraction_.add_outline(std::move(known.outline), known.result);
      }
      const auto id = *known.body;
      const auto &callee = extraction_.bodies_.at(id);
      const auto &definition = *callee.outline;
      expect(!region_receiver && !expr.has_here_body() && expr.children.size() == definition.parameters.size(),
             expr.at, "lexical procedure call needs its declared positional arguments");
      if (form_.kind == Form::Kind::function) fail(expr.at, "a function cannot invoke a procedure");
      CheckedOperation op;
      op.type = *callee.instance.result.type;
      op.callee = id;
      for (size_t i = 0; i < definition.parameters.size(); ++i) {
        const auto &type = definition.parameters[i].type;
        takes_value(operand(expr, i), type, expr.children[i]->at);
        op.arguments.push_back({ArgumentMode::value, type});
      }
      body_.calls.push_back({&expr, id, {}});
      return op;
    }

    bool fixed_overload_inputs(const CheckedMethod &method, const CheckedMethod &first) const {
      const auto parameters = method.input_parameters();
      const auto first_parameters = first.input_parameters();
      const auto &roles = method.input_owners();
      const auto &first_roles = first.input_owners();
      if (method.interpretation || method.scope_inputs || !method.type_parameters.empty() ||
          !method.retained_types.empty() || !method.here.empty() || parameters.size() != method.parameters.size() ||
          roles.size() != first_roles.size()) return false;
      for (size_t i = 0; i < roles.size(); ++i)
        if (roles[i].root || roles[i].explicit_parameter || roles[i].read_only != first_roles[i].read_only)
          return false;
      for (size_t i = 0; i < parameters.size(); ++i) {
        const auto &parameter = parameters[i];
        const auto &type = method.parameters[i];
        const auto mode = argument_mode(parameter);
        const auto &prior = first_parameters[i];
        if (parameter.automatic_conversion || parameter.not_init_ref || parameter.selects_caller_place() ||
            mode != argument_mode(prior)) return false;
        if (mode == ArgumentMode::value) {
          if (!scalar_type(type)) return false;
        } else {
          const auto role = std::ranges::find(roles, parameter.owner, &OwnerParameter::name);
          const auto first_role = std::ranges::find(first_roles, prior.owner, &OwnerParameter::name);
          if (role == roles.end() || first_role == first_roles.end() ||
              role - roles.begin() != first_role - first_roles.begin() ||
              parameter.read_only != prior.read_only || !ordinary_loan(parameter, mode, type)) return false;
        }
      }
      return true;
    }

    static bool matches_fixed_inputs(const CheckedMethod &method, std::span<const Type> types, bool widening) {
      for (size_t i = 0; i < types.size(); ++i) {
        const auto mode = argument_mode(method.input_parameters()[i]);
        const auto &formal = method.parameters[i];
        if (types[i] == formal || (mode == ArgumentMode::borrowed_ref && widens_text(types[i], formal))) continue;
        if (!widening || mode != ArgumentMode::value || !accepts_argument(types[i], formal, mode)) return false;
      }
      return true;
    }

    size_t select_overload(const Expression &expr, const CheckedFamily &family,
                           std::vector<const CheckedOperation *> &operands) {
      expect(!expr.has_here_body() && std::ranges::none_of(expr.children, [](const auto &operand) {
        return operand->spread || operand->checked_ref_argument || operand->kind == Kind::named_argument;
      }), expr.at, "replacement seed needs positional operands for an ordinary overload");
      const auto definition = [&](size_t id) -> const CheckedMethod & {
        return extraction_.declarations_.method(id);
      };
      const auto call_name = [&] {
        const auto &source = *definition(family.methods.front()).source;
        auto name = "'" + expr.text + "'";
        if (source.name != expr.text)
          name += " (canonical " + source.at.filename() + "::" + source.name + ")";
        return name;
      };
      std::vector<size_t> candidates;
      for (const auto id : family.methods) {
        const auto &method = definition(id);
        expect(std::ranges::none_of(method.input_parameters(), &Parameter::variadic),
               expr.at, "replacement seed needs fixed ordinary overload inputs");
        if (method.here.empty() && method.parameters.size() == expr.children.size()) candidates.push_back(id);
      }
      if (candidates.empty()) fail(expr.at, "no method with the written arity for " + call_name());
      const auto &first = definition(candidates.front());
      for (const auto id : candidates)
        expect(fixed_overload_inputs(definition(id), first), expr.at,
               "replacement seed needs fixed ordinary overloads with common input modes, access and owner roles");

      // Common preparation makes the same view/access checks apply to every
      // candidate. Select from the types of the ordered, already extracted operands.
      std::vector<Type> types;
      types.reserve(expr.children.size());
      operands.reserve(expr.children.size());
      for (const auto &source : expr.children) {
        const auto &arg = extract(*source);
        operands.push_back(&arg);
        types.push_back(arg.type);
      }
      for (const bool widening : {false, true}) {
        std::vector<size_t> applicable;
        for (const auto id : candidates)
          if (matches_fixed_inputs(definition(id), types, widening)) applicable.push_back(id);
        if (applicable.empty()) continue;
        std::optional<size_t> best;
        for (const auto id : applicable) {
          const auto &method = definition(id);
          const bool dominated = std::ranges::any_of(applicable, [&](size_t other) {
            const auto &alternative = definition(other);
            return matches_fixed_inputs(method, alternative.parameters, widening) &&
                   !matches_fixed_inputs(alternative, method.parameters, widening);
          });
          if (dominated) continue;
          if (best)
            fail(expr.at, "ambiguous methods for " + call_name() + ": " +
                 location_text(definition(*best).source->at) + " " + location_text(method.source->at));
          best = id;
        }
        if (!best) fail(expr.at, "no unique most-specific method for " + call_name());
        return *best;
      }
      fail(expr.at, "no applicable method for " + call_name());
    }

    CheckedOperation call(const Expression &expr, bool region_receiver = false) {
      CheckedOperation op;
      if (expr.text == "[]<-" && expr.children.size() == 3) {
        confirm_intrinsic(expr, expr.text);
        op = index(expr);
        expect(!op.read_only, expr.at, "indexed transfer needs a writable array view");
        const auto &source = operand(expr, 2);
        takes_payload(source, op.type, expr.children[2]->at);
        op.builtin = BuiltinOperation::array_transfer;
        op.read_only = true;
        return op;
      }
      if (expr.intrinsic && expr.text == "len") {
        expect(expr.children.size() == 1, expr.at, "array length requires one operand");
        const auto &array = operand(expr, 0);
        expect(array.view && array.type.kind == Type::array, expr.at, "array length requires an array view");
        op.type = Type::uint64;
        op.builtin = BuiltinOperation::array_length;
        return op;
      }
      if (!expr.intrinsic)
        if (const auto *local = std::get_if<LocalProcedure>(find(expr.text)))
          return call_local(expr, *local, region_receiver);
      if (!expr.intrinsic && !visible_local(expr.text)) {
        if (const auto *formal = find_here(expr)) return invoke_here(expr, *formal);
        const auto primitive = names().text_primitives.find(expr.text);
        if (primitive != names().text_primitives.end() && primitive->second == COD_LANG_BUILDER_APPEND_EXPORT)
          return append_text(expr);
        const auto native = names().native_operations.find(expr.text);
        if (native != names().native_operations.end()) return provider_call(expr, native->second);
      }
      const auto *selected = family(expr.text);
      expect(!expr.intrinsic && !visible_local(expr.text) && selected && !selected->symbol &&
             !selected->methods.empty() && selected->declarations.empty(),
             expr.at, "replacement seed needs an unambiguous ordinary source call");
      std::vector<const CheckedOperation *> operands;
      const auto method_id = selected->methods.size() == 1 ? selected->methods.front() :
          select_overload(expr, *selected, operands);
      const auto &target = extraction_.declarations_.method(method_id);
      expect(target.result.owns_region() == region_receiver, expr.at,
             "an owning region result requires a direct owning declaration");
      if (form_.kind == Form::Kind::function && target.source->kind != Form::Kind::function)
        fail(expr.at, "a function cannot invoke a procedure");
      const auto callee = extraction_.select_body(method_id, expr.at);
      const auto parameters = target.input_parameters();
      expect(target.here.size() == size_t{expr.has_here_body()} &&
             parameters.size() + expr.has_here_body() == expr.children.size(),
             expr.at, "ordinary call arguments do not match the selected declaration");
      std::map<size_t, size_t> actual_owners;
      for (size_t i = 0; i < parameters.size(); ++i) {
        const auto &parameter = parameters[i];
        const auto mode = argument_mode(parameter);
        const auto &arg = operands.empty() ? operand(expr, i) : *operands[i];
        const auto &formal = target.parameters[i];
        if (mode == ArgumentMode::value) takes_value(arg, formal, expr.children[i]->at);
        else {
          const auto &at = expr.children[i]->at;
          const auto &roles = target.input_owners();
          const auto role = std::ranges::find(roles, parameter.owner, &OwnerParameter::name);
          const bool native_handle = mode == ArgumentMode::borrowed_handle && formal.kind == Type::native_handle;
          expect(role != roles.end() && !role->root && !role->explicit_parameter &&
                 !parameter.not_init_ref && (!parameter.selects_caller_place() || parameter.selects_caller_ref_slot()) &&
                 (native_handle || ordinary_loan(parameter, mode, formal)) && arg.type == formal &&
                 (parameter.read_only || !arg.read_only),
                 at, "replacement seed needs the declared ordinary borrow and access");
          if (parameter.selects_caller_ref_slot()) check_slot(arg, at);
          const auto owner = single_owner(arg, at);
          if (native_handle && !role->read_only && const_input_owner(owner))
            fail(at, "const owner cannot supply a writable native handle owner");
          if (declarations().builder_type(formal) && !role->read_only && const_input_owner(owner))
            fail(at, "const owner cannot supply a writable builder owner");
          const auto [first, inserted] = actual_owners.emplace(static_cast<size_t>(role - roles.begin()), owner);
          expect(inserted || first->second == owner, at, "one formal owner needs the same actual owner");
        }
        op.arguments.push_back({mode, formal, parameter.selects_caller_ref_slot()});
      }
      BoundCall edge{&expr, callee, {}};
      for (const auto &[role, owner] : actual_owners) if (!target.input_owners()[role].read_only)
        if (const auto allocator = allocator_for(owner)) edge.owners.emplace(role, *allocator);
      body_.calls.push_back(std::move(edge));
      op.type = *target.result.type;
      op.callee = callee;
      if (expr.has_here_body()) {
        const auto &contract = target.here.front();
        const auto owner = actual_owners.find(here_input_owner(contract, expr.at) - 1);
        expect(owner != actual_owners.end(), expr.at, "here loan needs a prepared actual owner");
        supply_here(expr, contract, op.type, owner->second, op);
        op.here_arguments.resize(1);
      }
      return op;
    }

    Type resolve_annotation(const Type &annotation, const Location &at) const {
      if (annotation.kind != Type::named) return annotation;
      const auto found = names().types.find(annotation.name);
      if (found == names().types.end()) fail(at, "unknown type '" + annotation.name + "'");
      return found->second;
    }

    void bind_independent_parameters(Outline &outlined) {
      outlined.parameters = outlined.source->parameters;
      for (auto &parameter : outlined.parameters) {
        expect(!parameter.here && !parameter.variadic && !parameter.automatic_conversion &&
               !parameter.selects_caller_place() && parameter.owner.empty() && !parameter.not_init_ref &&
               (parameter.declaration_kind.empty() || parameter.declaration_kind == "val"),
               parameter.at, "replacement seed needs independent lexical procedure inputs");
        parameter.type = resolve_annotation(parameter.type, parameter.at);
        expect(scalar_type(parameter.type), parameter.at, "replacement seed needs a scalar lexical procedure input");
      }
    }

    CheckedOperation define_local(const Expression &expr) {
      const auto &source = *expr.definition;
      expect(!body_.outline && source.ordinary_procedure() && !source.name.empty() && source.body &&
             !source.operation && !source.c_linkage && !source.symbolic && !source.explicit_captures &&
             source.owner_parameters.empty() && source.type_parameters.empty() && source.throws_types.empty() &&
             source.result_type && source.result_owner.empty() && source.result_kind.empty(), expr.at,
             "replacement seed needs a named lexical procedure with a fixed independent signature");
      const auto result = resolve_annotation(*source.result_type, source.at);
      expect(result == Type::void_type || scalar_type(result), source.at,
             "replacement seed needs a scalar or void lexical procedure result");
      expect(std::ranges::find(owners(), source.name, &OwnerParameter::name) == owners().end(), expr.at,
             "replacement seed has no lexical shadow of an owner name");
      const auto definition = extraction_.local_definitions_.size();
      const auto inserted = scopes_.back().names.emplace(source.name, LocalProcedure{definition}).second;
      expect(inserted, expr.at, "duplicate name in the replacement seed frame");
      auto outlined = outline(source);
      bind_independent_parameters(outlined);
      extraction_.local_definitions_.push_back({std::move(outlined), result, {}});
      return {};
    }

    const Type &handler_contract(size_t id, const Expression &expr) const {
      const auto &symbol = declarations().symbols.at(id);
      expect(!symbol.reference && symbol.type && symbol.type->kind == Type::procedure &&
             !symbol.type->pure_callable && symbol.type->target && !symbol.type->variadic &&
             symbol.type->owners.empty() && symbol.type->argument_modes.empty() &&
             (scalar_type(*symbol.type->target) || *symbol.type->target == Type::void_type),
             expr.at, "replacement seed needs a fixed procedure symbol with independent inputs and result");
      return *symbol.type;
    }

    CheckedOperation install_handler(const Expression &expr) {
      const auto id = selected_symbol(expr);
      const auto &contract = handler_contract(id, expr);
      const auto &source = *expr.definition;
      expect(!body_.outline && !in_iteration_ && source.kind == Form::Kind::procedure && source.body &&
             source.owner_parameters.empty() && source.type_parameters.empty() && !source.explicit_captures &&
             source.result_type && source.result_owner.empty() && source.result_kind.empty(),
             expr.at, "replacement seed needs a live handler in an ordinary procedure frame");
      auto outlined = outline(source);
      outlined.handler = id;
      bind_independent_parameters(outlined);
      const auto result = resolve_annotation(*source.result_type, source.at);
      expect(result == *contract.target && std::ranges::equal(outlined.parameters, contract.parameters, {},
                 &Parameter::type) && std::ranges::equal(source.throws_types, contract.throws_types, {},
                 [&](const Type &type) { return resolve_annotation(type, source.at); }), expr.at,
             "live handler must implement the selected symbol's exact contract");
      const auto added = extraction_.handler_layouts_.insert(id).second;
      expect(added, expr.at, "replacement seed needs one live handler layout per symbol");

      const auto callee = extraction_.bodies_.size();
      extraction_.add_outline(std::move(outlined), result);
      body_.calls.push_back({&expr, callee, {}, true});
      scopes_.back().handlers.insert(id);
      CheckedOperation op;
      op.symbol = id;
      op.scoped_effect = callee;
      return op;
    }

    CheckedOperation invoke_handler(const Expression &expr) {
      const auto id = selected_symbol(expr);
      const auto &contract = handler_contract(id, expr);
      expect(std::ranges::any_of(scopes_, [&](const auto &scope) { return scope.handlers.contains(id); }),
             expr.at, "replacement seed needs a selected live handler in the current invocation");
      expect(expr.children.size() == contract.parameters.size() && !expr.has_here_body(), expr.at,
             "live handler invocation needs its declared positional arguments");
      CheckedOperation op;
      op.symbol = id;
      op.type = *contract.target;
      for (size_t i = 0; i < contract.parameters.size(); ++i) {
        const auto &type = contract.parameters[i];
        takes_value(operand(expr, i), type, expr.children[i]->at);
        op.arguments.push_back({ArgumentMode::value, type});
      }
      return op;
    }

    CheckedOperation fresh_region(const Expression &expr) {
      expect(!supplied_body() && expr.annotation && expr.initializer_block, expr.at,
             "replacement seed needs a named region with explicit root initializers");
      const auto schema = resolve_annotation(*expr.annotation, expr.at);
      const auto &roots = direct_region_roots(declarations(), schema, expr.at);
      auto order = region_initializers(roots, expr);
      expect(order.size() == roots.fields.size() && std::set(order.begin(), order.end()).size() == order.size(),
             expr.at, "replacement seed needs one initializer for each region root");

      takes_value(operand(expr, 0), Type::uint64, expr.children.front()->at);
      const auto items = expr.initializer_items();
      for (size_t i = 0; i < order.size(); ++i) {
        const auto &input = *items[i]->children.back();
        takes_payload(extract(input), roots.fields[order[i]].type, input.at);
      }
      CheckedOperation op;
      op.type = owned_region_type(schema);
      op.operand_type = schema;
      op.region_initializers = std::move(order);
      return op;
    }

    CheckedOperation bind_region(const Expression &expr) {
      expect(!supplied_body() && !in_iteration_ && expr.annotation && expr.annotation->owned &&
             expr.annotation->target && expr.children.size() == 1, expr.at,
             "replacement seed needs a fixed lexical owning region declaration");
      const auto schema = resolve_annotation(*expr.annotation->target, expr.at);
      const auto &initializer = ungrouped_expression(*expr.children.front());
      expect(initializer.kind == Kind::make_region ||
             (initializer.kind == Kind::call && !initializer.intrinsic && !initializer.has_here_body()),
             initializer.at, "replacement seed needs a fresh region or a direct owning region factory");
      const auto &value = remember(initializer, initializer.kind == Kind::make_region
          ? fresh_region(initializer) : call(initializer, true));
      if (value.type != owned_region_type(schema))
        fail(initializer.at, "constructed region does not match the declared root schema");

      CheckedOperation op;
      const auto owner = body_.instance.bindings.size() + 1;
      op.region_holder = declare(expr.at, expr.text, value.type, true, expr.read_only, true, owner).place;
      if (!expr.read_only) writable_regions_.emplace(owner, *op.region_holder);
      return op;
    }

    size_t array_owner(const Expression &expr) const {
      if (expr.owner_name.empty()) return 0;
      if (const auto *binding = find(expr.owner_name)) {
        const auto *local = std::get_if<Local>(binding);
        expect(local, expr.at, "array allocation needs an owning region or an input owner");
        const auto &type = body_.instance.bindings.at(local->binding).type;
        expect(type.kind == Type::region && type.owned && local->place,
               expr.at, "array allocation needs an owning region or an input owner");
        return local->owner;
      }
      return input_owner(owner_role(expr.owner_name, expr.at));
    }

    CheckedOperation::NativeAcquisition native_acquisition(const Expression &initializer, const Type &declared) {
      const auto constructed = resolve_annotation(*initializer.annotation, initializer.at);
      const auto id = select_native_constructor(declarations(), declared, constructed, initializer);
      const auto &constructor = declarations().native_constructors.at(id);
      expect(!constructor.retained_parent, initializer.at,
             "replacement seed has no C handle acquisition with a retained parent");
      check_native_invocation_kind(&form_, *constructor.source, initializer.at);

      std::map<size_t, size_t> owners;
      for (size_t i = 0; i < constructor.inputs.size(); ++i) {
        const auto &input = constructor.inputs[i];
        const auto &at = initializer.children[i]->at;
        const auto &actual = native_operand(initializer, i, input.type);
        if (input.type.kind != Type::c_type_marker) {
          takes_value(actual, input.type, at);
          continue;
        }
        const auto owner = native_loan(actual, input.type, at);
        if (!constructor.source->parameters.at(i).read_only && actual.read_only)
          fail(at, "readonly handle cannot supply a writable C constructor argument");
        if (!constructor.source->owner_parameters.at(*input.owner).read_only && const_input_owner(owner))
          fail(at, "const owner cannot supply a writable C constructor owner");
        bind_native_owner(owners, *input.owner, owner, at);
      }
      return {id, &initializer};
    }

    CheckedOperation install_native_effect(const Expression &expr) {
      const auto id = owning_symbol(expr);
      const auto &symbol = declarations().symbols.at(id);
      if (expr.binding != "<-" || expr.children.size() != 1)
        fail(expr.at, "an owning effect for " + symbol_name(expr, symbol) + " requires '<-' and fresh 'make own handle' construction");
      const auto &initializer = ungrouped_expression(*expr.children.front());
      if (initializer.kind != Kind::make_owned_handle || !initializer.annotation)
        fail(initializer.at, "an owning effect for " + symbol_name(expr, symbol) + " requires fresh 'make own handle' construction");
      const auto &type = *symbol.type;
      CheckedOperation op;
      op.symbol = id;
      op.operand_type = type;
      op.native_acquisition = native_acquisition(initializer, {Type::c_type_marker, type.name, type.identity});
      return op;
    }

    CheckedOperation attach_native_handle(const Expression &expr) {
      expect(!supplied_body() && expr.annotation, expr.at, "replacement seed needs a direct lexical C handle attachment");
      const auto type = resolve_annotation(*expr.annotation, expr.at);
      auto plan = native_acquisition(expr, type);
      const auto &parent = *declarations().native_constructors.at(plan.constructor).parent;
      const auto *local = std::get_if<Local>(find(expr.owner_name));
      if (!local || !local->place)
        fail(expr.at, "make handle requires a live owned '" + parent.name + "' handle");
      const auto &binding = body_.instance.bindings.at(local->binding);
      if (!binding.type.owned || !matches_native_handle(binding.type, parent))
        fail(expr.at, "make handle requires a live owned '" + parent.name + "' handle");
      if (binding.read_only || const_input_owner(local->owner))
        fail(expr.at, "a readonly native handle cannot attach a child");
      plan.parent_place = local->place;
      CheckedOperation op;
      op.type = native_handle_type(type, false);
      op.view = true;
      op.owners.insert(local->owner);
      op.native_acquisition = std::move(plan);
      return op;
    }

    CheckedOperation bind_native_handle(const Expression &expr) {
      expect(!supplied_body() && expr.annotation && expr.children.size() == 1, expr.at,
             "replacement seed needs a direct lexical C handle declaration");
      const auto marker = resolve_annotation(*expr.annotation, expr.at);
      CheckedOperation op;
      if (expr.binding == "<-") {
        if (in_iteration_) fail(expr.at, "a loop-body variable declaration requires a nested per-iteration {{ ... }} scope");
        const auto &initializer = ungrouped_expression(*expr.children.front());
        expect(initializer.kind == Kind::make_owned_handle && initializer.annotation, initializer.at,
               "replacement seed needs a fresh owning C handle initializer");
        op.native_acquisition = native_acquisition(initializer, marker);
        // The low owner keys name local storage in this activation's frame.
        // They carry no array allocation capability.
        const auto owner = body_.instance.bindings.size() + 1;
        op.place = declare(expr.at, expr.text, native_handle_type(marker, true), true, false, true, owner).place;
      } else {
        expect(expr.binding == "=" && expr.owner_name.empty(), expr.at,
               "replacement seed needs a borrowed handle declaration with an inferred owner");
        const auto &initial = operand(expr, 0);
        const auto owner = native_loan(initial, marker, expr.children.front()->at);
        op.place = declare(expr.at, expr.text, initial.type, true, initial.read_only, true, owner).place;
      }
      return op;
    }

    CheckedOperation make_product(const Expression &expr, const Type &type, bool local_storage) {
      expect(expr.kind == Kind::make_object && expr.annotation &&
             !expr.construction_type_query && !expr.constructor_arguments &&
             expr.initializer_block && expr.owner_name.empty() &&
             resolve_annotation(*expr.annotation, expr.at) == type &&
             declarations().bits_payload(type),
             expr.at, "replacement seed needs a complete inline bits product initializer");
      const auto &schema = declarations().schemas.at(type.identity);
      const auto order = product_initializers(schema, expr);
      expect(std::set(order.begin(), order.end()).size() == schema.fields.size(), expr.at,
             "replacement seed needs an initializer for each product field");
      CheckedOperation op;
      op.type = op.operand_type = type;
      if (local_storage) op.place = storage("$product", type, 0, false, false);
      op.view = true;
      op.owners.insert(0);
      for (size_t i = 0; i < order.size(); ++i) {
        const auto &item = *expr.initializer_items()[i];
        const auto &field = schema.fields[order[i]];
        const auto &input = *item.children.back();
        if (field.type.kind == Type::product) {
          // The backend initializes this nested product in its destination.
          remember(input, make_product(input, field.type, false));
        } else takes_payload(extract(input), field.type, input.at);
        CheckedOperation write;
        write.type = write.operand_type = field.type;
        write.product_field = order[i];
        remember(item, std::move(write));
      }
      return op;
    }

    CheckedOperation make_scalar(const Expression &expr, const Type &type) {
      const auto *initializer = expr.construction_initializer();
      expect(initializer && !initializer->children.empty() && !expr.constructor_arguments &&
             expr.owner_name.empty() && !supplied_body() && !in_iteration_, expr.at,
             "replacement seed needs an initialized lexical scalar construction");
      takes_value(extract(*initializer), type, initializer->at);
      CheckedOperation op;
      op.type = op.operand_type = type;
      op.place = storage("$scalar", type, 0, false, false);
      op.view = true;
      op.owners.insert(0);
      return op;
    }

    CheckedOperation make_lexical_text(const Expression &expr, const Type &type, bool builder) {
      expect(expr.owner_name.empty() && !in_iteration_ && !supplied_body(), expr.at,
             "replacement seed needs lexical text in the enclosing full scope");
      const auto count = expr.children.size();
      if (!expr.constructor_arguments || (builder ? count != 0 && count != 2 : count != 1))
        fail(expr.at, builder ? "a lexical text builder requires zero arguments or two positional text pieces"
                             : "make String requires one positional text operand");
      using Allocation = CheckedOperation::LexicalAllocation;
      Allocation plan{0, builder ? Allocation::builder : Allocation::text};
      for (size_t i = 0; i < count; ++i) {
        const auto &piece = operand(expr, i);
        if (!text_piece(piece.type)) fail(expr.children[i]->at, "text construction requires a text piece");
        single_owner(piece, expr.children[i]->at);
        if (declarations().builder_type(piece.type)) plan.builder_inputs |= 1u << i;
      }
      if (!builder && plan.builder_inputs) plan.kind = Allocation::builder_snapshot;
      CheckedOperation op;
      op.type = op.operand_type = type;
      op.view = true;
      op.read_only = !builder;
      op.owners.insert(0);
      op.lexical_allocation = plan;
      return op;
    }

    CheckedOperation make(const Expression &expr) {
      expect(!expr.construction_type_query && expr.annotation,
             expr.at, "replacement seed needs a declared construction type");
      CheckedOperation op;
      const auto &spelling = expr.annotation->name;
      const auto found = expr.annotation->kind == Type::named ? names().constructors.find(spelling) : names().constructors.end();
      if (found != names().constructors.end()) {
        const auto selected = found->second;
        const auto &sum = declarations().sums.at(selected.sum);
        const auto &arm = sum.alternatives.at(selected.alternative);
        expect(expr.initializer_block && expr.owner_name.empty() && !sum.regional_only &&
               arm.initializers.empty() && !family(spelling),
               expr.at, "replacement seed needs an inline sum initializer");
        op.type = Type{Type::sum, sum.source->name, selected.sum};
        op.place = storage("$sum", op.type, 0, false, false);
        op.sum_constructor = selected;
        std::set<size_t> supplied;
        for (const auto &item : expr.initializer_items()) {
          const auto *label = item->initializer_field_label();
          expect(label && item->kind == Kind::transfer, item->at, "sum initializer needs a named value transfer");
          const auto field = std::ranges::find_if(arm.fields, [&](const auto &field) { return field.source->name == label->text; });
          expect(field != arm.fields.end() && !field->stores_target(), label->at, "sum initializer field is unavailable");
          const auto ordinal = static_cast<size_t>(field - arm.fields.begin());
          expect(supplied.insert(ordinal).second, label->at, "replacement seed needs a single initializer per field");
          takes_payload(extract(*item->children.back()), field->type, item->at);
          CheckedOperation initializer;
          initializer.operand_type = field->type;
          body_.instance.operations.emplace(item.get(), std::move(initializer));
          op.sum_initializers.push_back(ordinal);
        }
        expect(supplied.size() == arm.fields.size(), expr.at, "sum construction has a field without an initializer");
        op.view = true;
        op.owners.insert(0);
        return op;
      }
      const auto type = resolve_annotation(*expr.annotation, expr.at);
      if (scalar_type(type)) return make_scalar(expr, type);
      const bool builder = declarations().builder_type(type);
      if (builder || type == Type::string) return make_lexical_text(expr, type, builder);
      if (type.kind == Type::product) return make_product(expr, type, true);
      expect(type.kind == Type::array && expr.constructor_arguments &&
             expr.children.size() == 2 && !family(spelling),
             expr.at, "replacement seed needs an ordinary array constructor");
      expect(!supplied_body(), expr.at, "replacement seed does not allocate arrays inside a here literal");
      const auto owner = array_owner(expr);
      const auto allocator = allocator_for(owner);
      // Ordinary blocks use the enclosing full invocation's allocation owner.
      expect(allocator && (allocator->kind != Allocator::lexical || !in_iteration_), expr.at,
             "replacement seed needs an admitted array allocation owner");
      const auto &array = declarations().arrays.at(type.identity);
      expect(declarations().bits_payload(array.element), expr.at, "array fill requires a finite bits element");
      takes_value(operand(expr, 0), Type::uint64, expr.children.front()->at);
      const auto &fill = operand(expr, 1);
      takes_payload(fill, array.element, expr.children.back()->at);
      op.type = op.operand_type = type;
      op.view = true;
      op.owners.insert(owner);
      op.array_allocator = allocator;
      if (allocator->kind == Allocator::input) body_.allocations.insert(allocator->place);
      return op;
    }

    CheckedOperation index(const Expression &expr) {
      if (expr.kind == Kind::index) confirm_intrinsic(expr, "[]");
      const auto &array = operand(expr, 0);
      expect(array.view && array.type.kind == Type::array, expr.at, "indexing requires an array view");
      const auto &shape = declarations().arrays.at(array.type.identity);
      takes_value(operand(expr, 1), shape.index, expr.children[1]->at);
      CheckedOperation op;
      op.type = shape.element;
      op.operand_type = array.type;
      op.view = op.indexed_target = true;
      op.read_only = array.read_only;
      op.owners = array.owners;
      return op;
    }

    CheckedOperation assign(const Expression &expr) {
      confirm_intrinsic(expr, expr.kind == Kind::transfer ? "<-" : "=");
      const auto &target = operand(expr, 0);
      const auto &source = operand(expr, 1);
      CheckedOperation op;
      op.type = op.operand_type = target.type;
      op.view = true;
      op.owners = target.owners;
      if (expr.kind == Kind::rebind) {
        check_slot(target, expr.at);
        expect(source.type == target.type &&
               single_owner(target, expr.at) == single_owner(source, expr.at) &&
               (target.read_only || !source.read_only), expr.at,
               "reference rebinding must preserve type, owner and target access");
        op.arguments = {{ArgumentMode::borrowed_ptr, target.type, true}, {ArgumentMode::borrowed_ptr, source.type}};
        return op;
      }
      expect(target.view && !target.read_only && (target.indexed_target || target.binding),
             expr.at, "bits transfer needs a writable place");
      takes_value(source, target.type, expr.at);
      op.arguments = {{ArgumentMode::borrowed_ptr, target.type}, {ArgumentMode::value, target.type}};
      return op;
    }

    CheckedOperation declaration(const Expression &expr, bool fixed) {
      const bool value = expr.declaration_kind == "val";
      expect((value || expr.declaration_kind == "ref") && expr.children.size() == 1 &&
             (!value || !expr.annotation) && expr.owner_name.empty() && !expr.read_only,
             expr.at, "replacement seed needs an initialized ref or an inferred initialized val");
      const auto &initial = operand(expr, 0);
      if (expr.annotation && resolve_annotation(*expr.annotation, expr.at) != initial.type)
        fail(expr.at, "reference initializer does not match the declared target type");
      expect(initial.type.kind != Type::native_handle, expr.at, "a native handle view needs a handle declaration");
      const auto owner = value ? size_t{0} : single_owner(initial, expr.at);
      if (value) takes_value(initial, initial.type, expr.at);
      const auto local = declare(expr.at, expr.text, initial.type, value, value ? fixed : initial.read_only, fixed, owner);
      CheckedOperation op;
      op.binding = local.binding;
      op.type = op.operand_type = initial.type;
      op.place = local.place;
      op.view = !value;
      return op;
    }

    CheckedOperation try_statement(const Expression &expr) {
      const auto check_body = [&](const Expression &body) {
        const auto &result = extract(body);
        expect(result.type == Type::never || result.type == Type::void_type, body.at,
               "replacement seed needs an effect-only protected body");
        return result.type == Type::never;
      };
      ++protected_depth_;
      bool terminates = check_body(*expr.children.front());
      --protected_depth_;
      for (const auto &handler : std::span(expr.children).subspan(1)) {
        expect(handler->kind == Kind::handler && handler->annotation && handler->annotation->kind == Type::named,
               handler->at, "replacement seed needs a named exception catch");
        const auto type = names().types.find(handler->annotation->name);
        expect(type != names().types.end() && type->second.kind == Type::exception, handler->at,
               "catch needs a resolved exception category");
        if (nominal_exception(type->second)) scalar_exception_schema(declarations(), type->second, handler->at);
        scopes_.emplace_back();
        const auto local = declare(handler->at, handler->text, type->second, true, true, true, 0);
        CheckedOperation receipt;
        receipt.type = type->second;
        receipt.place = local.place;
        body_.instance.operations.emplace(handler.get(), std::move(receipt));
        catches_.push_back({*local.place, protected_depth_});
        terminates = check_body(*handler->children.front()) && terminates;
        catches_.pop_back();
        scopes_.pop_back();
      }
      CheckedOperation op;
      if (terminates) op.type = Type::never;
      return op;
    }

    CheckedOperation braced_throw(const Expression &expr) {
      expect(expr.annotation && expr.annotation->kind == Type::named,
             expr.at, "replacement seed needs a braced throw with a named exception");
      const auto type = names().types.find(expr.annotation->name);
      expect(type != names().types.end(), expr.at, "throw needs a resolved exception category");
      const auto &schema = scalar_exception_schema(declarations(), type->second, expr.at);
      const auto fields = product_initializers(schema, expr);
      expect(std::set(fields.begin(), fields.end()).size() == schema.fields.size(), expr.at,
             "replacement seed needs a complete exception payload");
      for (size_t i = 0; i < fields.size(); ++i) {
        const auto &item = *expr.children[i];
        const auto &field = schema.fields[fields[i]];
        const auto &value = operand(item, 0);
        if (value.type != Type::never) takes_value(value, field.type, item.at);
        CheckedOperation write;
        write.type = value.type == Type::never ? value.type : field.type;
        write.operand_type = field.type;
        write.product_field = fields[i];
        body_.instance.operations.emplace(&item, std::move(write));
        if (value.type == Type::never) break;
      }
      CheckedOperation op;
      op.type = Type::never;
      op.operand_type = type->second;
      return op;
    }

    CheckedOperation throw_exception(const Expression &expr) {
      if (expr.annotation || !expr.children.empty()) return braced_throw(expr);
      if (catches_.empty()) fail(expr.at, "bare throw requires an active catch");
      const auto &caught = catches_.back();
      // The existing transfer takes the holder from this catch. A nested
      // interception would need proof that the original holder is still usable.
      expect(caught.protected_depth == protected_depth_, expr.at,
             "replacement seed cannot transfer a caught exception across an intervening protected body");
      CheckedOperation op;
      op.type = Type::never;
      op.place = op.exception_owner = caught.place;
      return op;
    }

    CheckedOperation exception_field(const Expression &expr, const CheckedOperation &base) {
      expect(expr.children.front()->kind == Kind::name && base.place &&
             std::ranges::find(catches_, *base.place, &ActiveCatch::place) != catches_.end() &&
             nominal_exception(base.type), expr.at, "replacement seed needs a field read from the active nominal catch");
      const auto &schema = scalar_exception_schema(declarations(), base.type, expr.at);
      const auto index = product_field(schema, expr);
      CheckedOperation op;
      op.type = schema.fields[index].type;
      op.operand_type = base.type;
      op.product_field = index;
      op.exception_owner = base.place;
      op.read_only = true;
      return op;
    }

    CheckedOperation assert_enum(const Expression &expr, const Type &type) {
      const auto &written = *expr.children.front();
      if (written.kind == Kind::name && !visible_local(written.text) && !here_.contains(written.text) &&
          (!body_.outline || !body_.outline->visible_here.contains(written.text)) &&
          !names().binds_other_than_enum_member(written.text) &&
          !names().enum_members.contains(written.text)) {
        if (const auto member = enum_member(declarations(), type, written.text))
          return checked_enum_literal(declarations(), *member);
        fail(written.at, "unknown member '" + written.text + "' of enum '" + type.name + "'");
      }
      const auto &literal = ungrouped_expression(written);
      const bool negative = literal.kind == Kind::unary && literal.text == "-" &&
                            literal.children.front()->kind == Kind::integer;
      if (negative || literal.kind == Kind::integer) {
        bool minimum = false;
        if (negative) {
          confirm_intrinsic(literal, "-");
          auto magnitude = std::string_view(literal.children.front()->text);
          while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
          minimum = magnitude == "9223372036854775808";
        }
        if (const auto member = enum_literal_from_integer(declarations(), type, written, literal, negative, minimum))
          return checked_enum_literal(declarations(), *member);
      }
      const auto &source = operand(expr, 0);
      if (supplies_enum_code(type, source.type)) {
        CheckedOperation op;
        op.type = type;
        op.enum_decode = true;
        return op;
      }
      if (source.type != type && source.type != Type::never)
        fail(expr.at, "type assertion cannot match enum '" + type.name + "'");
      auto op = source;
      op.enum_assertion = true;
      op.enum_literal = op.enum_decode = false;
      return op;
    }

    CheckedOperation typed_expression(const Expression &expr) {
      const auto type = resolve_annotation(*expr.annotation, expr.at);
      if (type.kind == Type::enumeration) return assert_enum(expr, type);
      if (type == Type::void_type) {
        operand(expr, 0);
        return {};
      }
      expect(type == Type::c_string, expr.at,
             "replacement seed supports enum, void and CString result assertions");
      auto op = operand(expr, 0);
      expect(text_type(op.type) && op.view && op.read_only && op.owners.size() == 1,
             expr.at, "replacement seed needs a readonly String loan for a CString assertion");
      op.check_c_string = op.type == Type::string;
      op.type = Type::c_string;
      return op;
    }

    CheckedOperation field(const Expression &expr) {
      const auto &base = operand(expr, 0);
      if (base.type.kind == Type::exception) return exception_field(expr, base);
      const bool region_root = base.type.kind == Type::region && base.type.owned && base.type.target && base.place;
      expect(region_root || (base.type.kind == Type::product && base.view && declarations().bits_payload(base.type)),
             expr.at, "replacement seed needs an inline field of a bits product or owned region");
      const auto &type = region_root ? *base.type.target : base.type;
      const auto &schema = declarations().schemas.at(type.identity);
      const auto index = product_field(schema, expr);
      const auto &member = schema.fields[index];
      CheckedOperation op;
      op.type = member.type;
      op.operand_type = type;
      op.product_field = index;
      if (region_root) op.region_holder = base.place;
      op.view = op.indexed_target = true;
      op.read_only = base.read_only || readonly_field(member);
      op.owners = base.owners;
      return op;
    }

    CheckedOperation check_case(const Expression &expr) {
      const auto &subject = operand(expr, 0);
      const bool sum = subject.type.kind == Type::sum && subject.view && declarations().bits_payload(subject.type);
      const bool enumeration = subject.type.kind == Type::enumeration;
      expect((sum || enumeration || subject.type == Type::boolean) &&
             !expr.case_breaks && !expr.case_resumes,
             expr.at, "replacement seed needs a closed Boolean, enum or bits sum case without case transfers");
      std::set<std::uint64_t> covered;
      bool catch_all = false;
      CheckedOperation op;
      op.type = Type::never;
      for (const auto &entry : std::span(expr.children).subspan(1)) {
        const auto &arm = *entry;
        expect(arm.kind == Kind::case_arm && !arm.case_resumes && arm.children.size() == 1, arm.at,
               "replacement seed needs an unguarded case arm");
        CheckedOperation matched;
        matched.case_pattern = bind_case_pattern(declarations(), subject.type, arm, names().constructors);
        const auto &pattern = matched.case_pattern;
        expect(pattern.kind != CasePattern::Kind::null, arm.at, "replacement seed needs a non-null case pattern");
        if (pattern.kind == CasePattern::Kind::any) catch_all = true;
        else covered.insert(pattern.code);
        scopes_.emplace_back();
        if (arm.receiving) {
          const auto constructor = *pattern.constructor;
          const auto &alternative = declarations().sums.at(constructor.sum).alternatives.at(constructor.alternative);
          const auto receiving = bind_case_fields(declarations(), arm, alternative, [&](const Type &type) {
            return resolve_annotation(type, arm.at);
          });
          for (size_t i = 0; i < receiving.size(); ++i) {
            const auto &receiver = *arm.case_bindings.at(i);
            const auto &saved = receiving[i];
            const auto local = declare(receiver.at, receiver.text, saved.type, true, saved.read_only,
                                       receiver.fixed_binding, 0);
            CheckedOperation binding;
            binding.place = local.place;
            binding.type = saved.type;
            binding.operand_type = alternative.fields.at(saved.field).type;
            matched.case_payload_fields.push_back(saved.field);
            remember(receiver, std::move(binding));
          }
        }
        const auto &result = operand(arm, 0);
        matched.type = result.type;
        matched.case_arm_completes = result.type != Type::never;
        if (matched.case_arm_completes) {
          expect(result.type == Type::void_type || scalar_type(result.type),
                 arm.at, "replacement seed needs case arms with void or independent scalar results");
          expect(op.type == Type::never || op.type == result.type, arm.at, "case arms require one result type");
          op.type = result.type;
          op.scalar_value_join |= result.view;
        }
        scopes_.pop_back();
        remember(arm, std::move(matched));
      }
      if (!catch_all) {
        if (sum) op.case_reaches_end = covered.size() != declarations().sums.at(subject.type.identity).alternatives.size();
        else if (enumeration) {
          const auto &domain = declarations().enums.at(subject.type.identity);
          op.case_reaches_end = std::ranges::any_of(domain.members, [&](const auto &member) {
            return !covered.contains(member.code);
          });
        } else op.case_reaches_end = covered.size() != 2;
      }
      if (op.case_reaches_end) {
        if (op.type == Type::never) op.type = Type::void_type;
        else if (op.type != Type::void_type)
          fail(expr.at, "value case may finish without a result; provide a completing arm or a catch-all");
      }
      return op;
    }

    CheckedOperation operation(const Expression &expr) {
      CheckedOperation op;
      switch (expr.kind) {
      case Kind::integer: case Kind::real: return numeric_literal(expr);
      case Kind::string_literal:
        op.type = Type::string;
        op.view = op.read_only = true;
        op.owners.insert(0);
        return op;
      case Kind::boolean:
        op.type = Type::boolean;
        op.integer = expr.text == "true";
        return op;
      case Kind::name: return name(expr);
      case Kind::callable: return define_local(expr);
      case Kind::unary: case Kind::binary: return eager(expr);
      case Kind::call: return call(expr);
      case Kind::make_object: return make(expr);
      case Kind::region_binding: return bind_region(expr);
      case Kind::native_handle_binding: return bind_native_handle(expr);
      case Kind::make_borrowed_handle: return attach_native_handle(expr);
      case Kind::effect: return expr.definition ? install_handler(expr) : install_native_effect(expr);
      case Kind::procedure_effect:
        expect(expr.definition != nullptr, expr.at, "replacement seed needs a private live handler body");
        return install_handler(expr);
      case Kind::behave: case Kind::perform: return invoke_handler(expr);
      case Kind::index: return index(expr);
      case Kind::field: return field(expr);
      case Kind::case_expression: return check_case(expr);
      case Kind::case_failure:
        op.type = Type::never;
        return op;
      case Kind::raise: return throw_exception(expr);
      case Kind::try_scope: return try_statement(expr);
      case Kind::rebind: case Kind::transfer: return assign(expr);
      case Kind::grouped: return operand(expr, 0);
      case Kind::typed_expression: return typed_expression(expr);
      case Kind::block:
        expect(!expr.scoped, expr.at, "replacement seed has no extracted full-scope body");
        scopes_.emplace_back();
        for (const auto &item : expr.children) {
          op.type = extract(*item).type;
          if (op.type == Type::never) break;
        }
        scopes_.pop_back();
        return op;
      case Kind::pointer_group:
        for (const auto &item : expr.children) {
          expect(item->kind == Kind::pointer, item->at, "replacement seed needs an initialized local declaration");
          body_.instance.operations.emplace(item.get(), declaration(*item, expr.fixed_binding));
        }
        return op;
      case Kind::conditional: {
        expect(operand(expr, 0).type == Type::boolean, expr.at, "conditional requires a Boolean operand");
        scopes_.emplace_back();
        const auto yes = operand(expr, 1).type;
        scopes_.pop_back();
        scopes_.emplace_back();
        const auto no = operand(expr, 2).type;
        scopes_.pop_back();
        expect(yes == no || yes == Type::never || no == Type::never,
               expr.at, "replacement seed needs compatible conditional results");
        op.type = yes == Type::never ? no : yes;
        return op;
      }
      case Kind::range_for: {
        const auto &first = operand(expr, 0);
        const auto &last = operand(expr, 1);
        expect(integral(first.type) && first.type == last.type && expr.children.size() == 3,
               expr.at, "replacement seed needs an ordinary half-open scalar range");
        op.operand_type = first.type;
        scopes_.emplace_back();
        op.place = declare(expr.at, expr.text, first.type, true, true, true, 0).place;
        const auto enclosing_iteration = std::exchange(in_iteration_, true);
        operand(expr, 2);
        in_iteration_ = enclosing_iteration;
        scopes_.pop_back();
        return op;
      }
      case Kind::return_tail: {
        expect(!supplied_body() && !body_.instance.result.owns_region(), expr.at,
               "replacement seed needs an independent callable result at the tail");
        const auto &tail = operand(expr, 0);
        op.type = tail.type;
        if (tail.type != Type::never && tail.type != Type::void_type) {
          const auto &result = *body_.instance.result.type;
          takes_value(tail, result, expr.at);
          op.type = Type::never;
          op.operand_type = result;
        }
        return op;
      }
      case Kind::return_value: case Kind::continue_value:
        expect(supplied_body() == (expr.kind == Kind::continue_value), expr.at,
               "replacement seed needs an ordinary return or a local here continue");
        if (body_.instance.result.owns_region()) {
          expect(expr.children.size() == 1, expr.at, "return needs a fresh region");
          const auto &value = ungrouped_expression(*expr.children.front());
          expect(value.kind == Kind::make_region, value.at, "replacement seed needs a direct fresh region return");
          if (remember(value, fresh_region(value)).type != *body_.instance.result.type)
            fail(value.at, "constructed region does not match the declared root schema");
        } else if (expr.children.empty()) expect(body_.instance.result.type == Type::void_type, expr.at, "return needs a value");
        else takes_value(operand(expr, 0), *body_.instance.result.type, expr.at);
        op.type = Type::never;
        op.operand_type = *body_.instance.result.type;
        return op;
      default:
        implementation_gap(expr.at, "replacement seed has no executable extraction for this source form");
      }
    }

    const CheckedOperation &remember(const Expression &expr, CheckedOperation op) {
      // Calls with a here operand are open until the receiver-to-body
      // dependency is represented. Native disposal has its own contract.
      if (!body_.open_replay &&
          (op.native_acquisition || op.native_call || op.invokes_here || expr.has_here_body() ||
           (expr.kind == Kind::symbol && op.symbol)))
        body_.open_replay = true;
      if (op.view && !scalar_type(op.type) && op.type != Type::exception)
        op.discarded_view = true;
      const auto [row, first] = body_.instance.operations.emplace(&expr, std::move(op));
      if (!first) throw std::logic_error("replacement seed visited an operation twice");
      return row->second;
    }

    const CheckedOperation &extract(const Expression &expr) {
      expect(!expr.checked_ref_argument && !expr.spread && !expr.lexical_symbol_call &&
             (expr.definition ? expr.kind == Kind::callable || expr.kind == Kind::effect ||
                                 expr.kind == Kind::procedure_effect : expr.kind != Kind::callable) && !expr.receiving,
             expr.at, "replacement seed has no extracted transport for this expression");
      return remember(expr, operation(expr));
    }

  public:
    Frame(BodyExtraction &extraction, size_t id)
        : extraction_(extraction), body_(extraction.bodies_[id]), id_(id),
          method_(body_.outline ? nullptr : &extraction.declarations_.method(body_.instance.method)),
          form_(method_ ? *method_->source : *body_.outline->source),
          module_(method_ ? method_->module : body_.outline->module) {}

    void extract_body() {
      if (method_) {
        const auto parameters = method_->input_parameters();
        for (size_t i = 0; i < parameters.size(); ++i) bind_input(parameters[i], method_->parameters[i]);
        if (!method_->here.empty()) {
          const auto &contract = method_->here.front();
          const auto role = here_input_owner(contract, form_.at);
          expect(role <= owners().size(), form_.at, "here declaration has no established owner");
          const auto owner = input_owner(role - 1);
          const auto &formal = form_.parameters.front();
          expect(!find(formal.name), formal.at, "duplicate here parameter name");
          here_.emplace(formal.name, HereFormal{&contract, owner, 0});
        }
      } else {
        for (const auto &offered : body_.outline->parameters) {
          bind_input(offered, offered.type);
          body_.instance.arguments.push_back(offered.type);
        }
      }
      const auto &end = extract(*form_.body);
      expect(body_.instance.frame.owners.size() == owners().size(), form_.at,
             "replacement seed needs an input for every formal owner");
      expect(end.type == Type::never || body_.instance.result.type == Type::void_type,
             form_.at, "replacement seed needs a reachable return for this result");
      if (body_.outline) body_.instance.context = body_.instance.arguments;
    }
  };

  std::optional<Allocator> call_allocator(const BoundCall &call, size_t role) const {
    if (!call.installs_handler) {
      const auto found = call.owners.find(role);
      return found == call.owners.end() ? std::nullopt : std::optional{found->second};
    }
    // The outlined handler's operands are established at its installation.
    // A required input role has already passed the capture's access check.
    const auto &target = *bodies_[call.callee].outline;
    const auto owner = target.owners.at(role);
    return owner ? Allocator{Allocator::input, null_target - 1 - owner} : Allocator{Allocator::lexical};
  }

  BodyDependencies freeze_dependencies() const {
    BodyDependencies graph;
    if (!handler_layouts_.empty()) graph.replay_callers.resize(bodies_.size());
    for (size_t id = 0; id < bodies_.size(); ++id) {
      const auto &body = bodies_[id];
      graph.direct.push_back({body.allocations, body.open_replay});
      for (const auto &call : body.calls) {
        if (!graph.replay_callers.empty()) graph.replay_callers[call.callee].push_back(id);
        for (const auto &[owner, input] : bodies_[call.callee].instance.frame.owners) {
          const auto role = null_target - 1 - owner;
          const auto actual = call_allocator(call, role);
          if (actual && actual->kind == Allocator::input)
            graph.allocating_callers[{call.callee, role}].emplace_back(id, actual->place);
        }
      }
    }
    return graph;
  }

  void extract() {
    const auto &program = declarations_.declarations_;
    expect(!program.native_entries.empty(), {declarations_.source_.modules.front().path.string()},
           "replacement seed currently needs a C entry");
    for (const auto &module : declarations_.source_.modules)
      for (const auto &form : module.forms())
        expect(form->kind != Form::Kind::expression, form->at, "replacement seed has no executable module initializer");
    for (const auto &entry : program.native_entries) select_body(entry.method, program.methods[entry.method].source->at);
    // Demand a source body once. A discovered dependency only adds its own
    // binding work; it cannot restart a caller or publish a callee summary.
    for (size_t i = 0; i < bodies_.size(); ++i) Frame(*this, i).extract_body();
  }

  void seal_outline(CheckedProgram &program, size_t id) {
    auto &body = bodies_.at(id);
    auto &outline = *body.outline;
    auto &supplier = bodies_.at(outline.supplier).instance;
    if (body.instance.method != program.methods.size())
      throw std::logic_error("outlined method order changed before emission");

    auto &method = program.methods.emplace_back();
    method.source = outline.source;
    method.module = outline.module;
    method.definition = body.instance.method;
    method.parameters = body.instance.arguments;
    method.result = body.instance.result;
    if (outline.handler) {
      auto &symbol = program.symbols.at(*outline.handler);
      symbol.live_scope = true;
      symbol.scoped_instance = id;
      method.throws_types = symbol.type->throws_types;
    }
    CheckedMethod::ScopeInputs scope{};
    scope.lexical_method = body.instance.method;
    scope.parameters = std::move(outline.parameters);
    scope.owners = std::move(outline.owner_parameters);
    CheckedOperation::SuppliedHere *supplied = nullptr;
    if (auto &here = outline.here) {
      auto &handoff = supplier.operations.at(here->call).supplied_here;
      handoff = CheckedOperation::SuppliedHere{id, {}, {}, std::move(here->here_sources), {}};
      supplied = &*handoff;
      supplied->owners.assign(std::next(outline.owners.begin()), outline.owners.end());
      const auto offered = scope.parameters.size();
      std::vector<size_t> positions(offered);
      std::iota(positions.begin(), positions.end(), size_t{0});
      scope.here_reception = CheckedMethod::ScopeInputs::HereReception{offered, std::move(positions)};
      scope.here_input_owners = 1;
      scope.lexical_here = std::move(here->lexical_here);
      method.here_group = CheckedMethod::HereGroupResults{supplier.result.type, here->receiving_result, {}, false};
    } else {
      scope.named = CheckedMethod::ScopeInputs::NamedEntry{scope.parameters.size(), {}};
    }
    for (auto &capture : outline.captures) {
      const auto [row, inserted] = supplier.operations.emplace(capture.use, place_operand(supplier, capture.source));
      if (!inserted) throw std::logic_error("lexical capture already has a supplier operand");
      if (supplied) supplied->places.push_back(capture.use);
      else scope.named->bindings.insert(capture.parameter.name);
      if (capture.parameter.selects_caller_ref_slot()) scope.live_bindings.insert(scope.parameters.size());
      scope.parameters.push_back(std::move(capture.parameter));
    }
    method.scope_inputs = std::move(scope);
  }

  void prepare_local_captures(const CheckedProgram &program, const BoundCall &call, CheckedOperation &operation) const {
    const auto &body = bodies_.at(call.callee);
    const auto &outline = *body.outline;
    const auto parameters = program.methods.at(body.instance.method).input_parameters();
    operation.scope_owners = outline.owners;
    auto &arguments = operation.argument_sources.emplace();
    if (!call.installs_handler)
      for (const auto &argument : call.site->children) arguments.push_back(argument.get());
    const auto first_capture = parameters.size() - outline.captures.size();
    for (size_t i = 0; i < outline.captures.size(); ++i) {
      const auto &capture = outline.captures[i];
      const auto &parameter = parameters[first_capture + i];
      operation.arguments.push_back({argument_mode(parameter), parameter.type, parameter.selects_caller_ref_slot()});
      arguments.push_back(capture.use);
    }
  }

  void seal(CheckedProgram &program) {
    const auto completed = freeze_dependencies().complete();
    for (const auto &entry : program.native_entries)
      expect(completed[selected_.at(entry.method)].allocations.empty(), program.methods[entry.method].source->at,
             "a C entry cannot import an array allocator capability");
    for (size_t i = 0; i < bodies_.size(); ++i) if (const auto &outline = bodies_[i].outline) {
      expect(outline->handler || completed[i].allocations.empty(), outline->source->at,
             "replacement seed needs an ordinary outlined body without imported allocators");
      seal_outline(program, i);
    }
    for (size_t i = 0; i < bodies_.size(); ++i) {
      auto &body = bodies_[i];
      for (const auto &call : body.calls) {
        auto &operation = body.instance.operations.at(call.site);
        const auto &callee = completed[call.callee];
        if (call.installs_handler && callee.open_replay)
          implementation_gap(call.site->at, "replacement seed cannot complete a live handler's empty replay inventory");
        if (const auto &outline = bodies_[call.callee].outline; outline && !outline->here)
          prepare_local_captures(program, call, operation);
        if (callee.allocations.empty()) continue;
        auto arguments = std::make_shared<std::map<size_t, Allocator>>();
        for (const auto role : callee.allocations) {
          const auto actual = call_allocator(call, role);
          expect(actual.has_value(), call.site->at, "required allocation has no prepared owner capability");
          arguments->emplace(role, *actual);
        }
        operation.array_allocator_arguments = std::move(arguments);
      }
      body.instance.array_allocator_roles = completed[i].allocations;
      body.instance.complete = true;
      program.methods[body.instance.method].instances.push_back(i);
      program.instances.push_back(std::move(body.instance));
    }
  }

public:
  static ExecutableProgram bind(Declarations declarations) {
    BodyExtraction extraction(declarations);
    extraction.extract();
    auto program = std::move(declarations.declarations_);
    extraction.seal(program);
    return {std::move(declarations.source_), std::move(program)};
  }
};

} // namespace cod::language::seed
