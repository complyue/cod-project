//===--- lang_seed_bodies.hh - Extract executable seed bodies ---------===//
#pragma once

#include "lang_checked.hh"

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

class BodyExtraction {
  using Allocator = CheckedOperation::ArrayAllocator;
  using Declarations = ScalarChecker::ModuleDeclarations;

  struct BoundCall {
    const Expression *site;
    size_t callee;
    std::map<size_t, Allocator> owners;
  };
  struct BoundBody {
    CheckedInstance instance;
    std::set<size_t> allocations;
    std::vector<BoundCall> calls;
  };
  struct Local {
    size_t binding;
    std::optional<size_t> place;
    size_t owner;
  };

  const Declarations &declarations_;
  std::map<size_t, size_t> selected_;
  std::deque<BoundBody> bodies_;

  explicit BodyExtraction(const Declarations &declarations) : declarations_(declarations) {}

  static void expect(bool condition, const Location &at, const char *detail) {
    if (!condition) implementation_gap(at, detail);
  }

  size_t select_body(size_t method_id, const Location &at) {
    const auto &method = declarations_.method(method_id);
    const auto &source = *method.source;
    expect(source.body && !source.implicit_scope && !source.supplied_scope && !source.supplied_group &&
           !source.case_scan_scope && !source.symbolic && !method.interpretation && !method.local_effect_symbol &&
           !method.parent && !method.regional_initializer && method.operation.empty() &&
           method.here.empty() && method.type_parameters.empty() && source.captures.empty() &&
           method.result.type && method.result.mode == ResultMode::value &&
           (scalar_type(*method.result.type) || method.result.type == Type::void_type),
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

  class Frame {
    BodyExtraction &extraction_;
    BoundBody &body_;
    const CheckedMethod &method_;
    std::vector<std::map<std::string, Local>> scopes_{1};
    std::set<size_t> caught_places_;
    using Kind = Expression::Kind;

    const Local *find(const std::string &name) const {
      for (auto scope = scopes_.rbegin(); scope != scopes_.rend(); ++scope)
        if (const auto found = scope->find(name); found != scope->end()) return &found->second;
      return nullptr;
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
                  bool read_only, bool fixed, size_t owner, bool input = false) {
      expect(std::ranges::find(method_.input_owners(), name, &OwnerParameter::name) == method_.input_owners().end(),
             at, "replacement seed has no lexical shadow of an owner name");
      const auto id = body_.instance.bindings.size();
      auto &binding = body_.instance.bindings.emplace_back(type, read_only, value, fixed);
      binding.requires_initialized = !value;
      const auto place = value || input ? std::optional{storage(name, type, owner, read_only, input && owner != 0, id)}
                                        : std::nullopt;
      const auto [entry, inserted] = scopes_.back().emplace(name, Local{id, place, owner});
      expect(inserted, at, "duplicate name in the replacement seed frame");
      return entry->second;
    }

    size_t owner_role(const std::string &name, const Location &at) const {
      const auto &owners = method_.input_owners();
      const auto found = std::ranges::find(owners, name, &OwnerParameter::name);
      expect(found != owners.end() && !found->root && !found->explicit_parameter,
             at, "replacement seed needs an input-bound owner");
      return static_cast<size_t>(found - owners.begin());
    }

    bool can_allocate(size_t owner) const {
      return !owner || !method_.input_owners().at(null_target - 1 - owner).read_only;
    }

    static Allocator capability(size_t owner) {
      return owner ? Allocator{Allocator::input, null_target - 1 - owner} : Allocator{Allocator::lexical};
    }

    const auto &names() const { return extraction_.declarations_.modules_.at(method_.module); }
    const CheckedProgram &declarations() const { return extraction_.declarations_.declarations_; }

    const CheckedFamily *family(const std::string &name) const {
      return extraction_.declarations_.ordinary_family(method_.module, name);
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

    static bool ordinary_loan(ArgumentMode mode, const Type &type) {
      return (mode == ArgumentMode::borrowed_read || mode == ArgumentMode::borrowed_ref) &&
             (type.kind == Type::array || scalar_type(type));
    }

    static size_t single_owner(const CheckedOperation &view, const Location &at) {
      expect(view.view && view.owners.size() == 1, at, "replacement seed needs a view with one established owner");
      return *view.owners.begin();
    }

    void check_slot(const CheckedOperation &operation, const Location &at) const {
      expect(operation.binding && !body_.instance.bindings.at(*operation.binding).value_binding &&
             !operation.fixed_binding, at, "replacement seed needs a writable lexical reference slot");
    }

    CheckedOperation name(const Expression &expr) const {
      const auto *local = find(expr.text);
      if (!local) {
        const auto selected = names().enum_members.find(expr.text);
        expect(selected != names().enum_members.end(), expr.at, "replacement seed needs a bound operand");
        const auto &member = selected->second;
        CheckedOperation op;
        op.type = member.type;
        op.enum_literal = true;
        op.integer = declarations().enums.at(member.type.identity).members.at(member.member).code;
        return op;
      }
      const auto &binding = body_.instance.bindings.at(local->binding);
      CheckedOperation op;
      op.type = binding.type;
      op.binding = local->binding;
      op.place = local->place;
      op.owners.insert(local->owner);
      op.view = true;
      op.read_only = binding.read_only;
      op.fixed_binding = binding.fixed;
      return op;
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

    CheckedOperation provider_call(const Expression &expr, size_t id) {
      const auto &native = declarations().native_operations.at(id);
      const auto &form = *native.contract.source;
      expect(!native.has_here() && !expr.has_here_body(), expr.at, "replacement seed needs a bound native here body");
      expect(expr.children.size() == native.inputs.size(), expr.at, "arguments do not match the C provider declaration");
      if (method_.source->kind == Form::Kind::function && form.kind != Form::Kind::function)
        fail(expr.at, "a function cannot invoke a procedure");
      CheckedOperation op;
      op.type = *native.contract.result.type;
      op.native_call = id;
      std::map<size_t, size_t> owners;
      for (size_t i = 0; i < native.inputs.size(); ++i) {
        const auto &input = native.inputs[i];
        const auto &at = expr.children[i]->at;
        const auto &actual = operand(expr, i);
        if (!input.owner) {
          takes_value(actual, input.type, at);
          op.arguments.push_back({ArgumentMode::value, input.type});
          continue;
        }
        const bool handle = input.type.kind == Type::c_type_marker;
        expect(handle ? matches_native_handle(actual.type, input.type) && !actual.type.owned :
               input.type.kind == Type::array && actual.type == input.type,
               at, "C provider needs a loan of the declared nominal type");
        const auto owner = single_owner(actual, at);
        const auto &role = form.owner_parameters.at(*input.owner);
        if (!role.read_only && (actual.read_only || !can_allocate(owner)))
          fail(at, "readonly loan cannot supply a writable C provider owner");
        const auto [first, inserted] = owners.emplace(*input.owner, owner);
        if (!inserted && first->second != owner) fail(at, "C provider arguments for one owner need the same actual owner");
        op.arguments.push_back({handle ? ArgumentMode::borrowed_handle : ArgumentMode::borrowed_ref, actual.type});
      }
      return op;
    }

    CheckedOperation call(const Expression &expr) {
      CheckedOperation op;
      if (expr.text == "[]<-" && expr.children.size() == 3) {
        confirm_intrinsic(expr, expr.text);
        op = index(expr);
        expect(!op.read_only, expr.at, "indexed transfer needs a writable array view");
        const auto &source = operand(expr, 2);
        if (inline_aggregate_type(op.type))
          expect(source.view && source.type == op.type, expr.at, "indexed transfer needs the exact nominal element");
        else takes_value(source, op.type, expr.children[2]->at);
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
      if (!expr.intrinsic && !find(expr.text)) {
        const auto native = names().native_operations.find(expr.text);
        if (native != names().native_operations.end()) return provider_call(expr, native->second);
      }
      const auto *selected = family(expr.text);
      expect(!expr.intrinsic && !find(expr.text) && selected && !selected->symbol &&
             selected->methods.size() == 1 && selected->declarations.empty() && !expr.has_here_body(),
             expr.at, "replacement seed needs an unambiguous ordinary source call");
      const auto method_id = selected->methods.front();
      const auto &target = extraction_.declarations_.method(method_id);
      expect(method_.source->kind != Form::Kind::function || target.source->kind == Form::Kind::function,
             expr.at, "a function cannot invoke a procedure");
      const auto callee = extraction_.select_body(method_id, expr.at);
      const auto parameters = target.input_parameters();
      expect(parameters.size() == expr.children.size(), expr.at, "ordinary call argument count does not match");
      std::map<size_t, size_t> actual_owners;
      for (size_t i = 0; i < parameters.size(); ++i) {
        const auto &parameter = parameters[i];
        const auto mode = argument_mode(parameter);
        const auto &arg = operand(expr, i);
        const auto &formal = target.parameters[i];
        if (mode == ArgumentMode::value) takes_value(arg, formal, expr.children[i]->at);
        else {
          const auto &at = expr.children[i]->at;
          const auto &roles = target.input_owners();
          const auto role = std::ranges::find(roles, parameter.owner, &OwnerParameter::name);
          const bool native_handle = mode == ArgumentMode::borrowed_handle && formal.kind == Type::native_handle;
          expect(role != roles.end() && !role->root && !role->explicit_parameter &&
                 !parameter.not_init_ref && (!parameter.selects_caller_place() || parameter.selects_caller_ref_slot()) &&
                 (native_handle || ordinary_loan(mode, formal)) && arg.type == formal &&
                 (parameter.read_only || !arg.read_only),
                 at, "replacement seed needs the declared ordinary borrow and access");
          if (parameter.selects_caller_ref_slot()) check_slot(arg, at);
          const auto owner = single_owner(arg, at);
          const auto [first, inserted] = actual_owners.emplace(static_cast<size_t>(role - roles.begin()), owner);
          expect(inserted || first->second == owner, at, "one formal owner needs the same actual owner");
        }
        op.arguments.push_back({mode, formal, parameter.selects_caller_ref_slot()});
      }
      BoundCall edge{&expr, callee, {}};
      for (const auto &[role, owner] : actual_owners)
        if (!target.input_owners()[role].read_only && can_allocate(owner))
          edge.owners.emplace(role, capability(owner));
      body_.calls.push_back(std::move(edge));
      op.type = *target.result.type;
      op.callee = callee;
      return op;
    }

    CheckedOperation make(const Expression &expr) {
      expect(!expr.construction_type_query && expr.annotation && expr.annotation->kind == Type::named,
             expr.at, "replacement seed needs a declared construction type");
      CheckedOperation op;
      const auto &spelling = expr.annotation->name;
      if (const auto found = names().constructors.find(spelling); found != names().constructors.end()) {
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
          takes_value(extract(*item->children.back()), field->type, item->at);
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
      const auto type = names().types.find(spelling);
      expect(type != names().types.end() && type->second.kind == Type::array && expr.constructor_arguments &&
             expr.children.size() == 2 && !family(spelling),
             expr.at, "replacement seed needs an ordinary array constructor");
      const auto owner = expr.owner_name.empty() ? size_t{0} : null_target - 1 - owner_role(expr.owner_name, expr.at);
      expect(can_allocate(owner) && (owner || scopes_.size() == 2), expr.at,
             "replacement seed needs an admitted array allocation owner");
      const auto &array = declarations().arrays.at(type->second.identity);
      expect(declarations().bits_payload(array.element), expr.at, "array fill requires a finite bits element");
      takes_value(operand(expr, 0), Type::uint64, expr.children.front()->at);
      const auto &fill = operand(expr, 1);
      if (inline_aggregate_type(array.element))
        expect(fill.view && fill.type == array.element, expr.at, "array fill needs the exact nominal bits element");
      else takes_value(fill, array.element, expr.children.back()->at);
      op.type = op.operand_type = type->second;
      op.view = true;
      op.owners.insert(owner);
      op.array_allocator = capability(owner);
      if (owner) body_.allocations.insert(op.array_allocator->place);
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
             !expr.annotation && expr.owner_name.empty() && !expr.read_only,
             expr.at, "replacement seed needs an inferred initialized val or ref");
      const auto &initial = operand(expr, 0);
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
      bool terminates = check_body(*expr.children.front());
      for (const auto &handler : std::span(expr.children).subspan(1)) {
        expect(handler->kind == Kind::handler && handler->annotation &&
               handler->annotation->kind == Type::named && handler->read_only,
               handler->at, "replacement seed needs a named readonly exception catch");
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
        caught_places_.insert(*local.place);
        terminates = check_body(*handler->children.front()) && terminates;
        caught_places_.erase(*local.place);
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

    CheckedOperation exception_field(const Expression &expr) {
      const auto &base = operand(expr, 0);
      expect(expr.children.front()->kind == Kind::name && base.place && caught_places_.contains(*base.place) &&
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

    CheckedOperation operation(const Expression &expr) {
      CheckedOperation op;
      switch (expr.kind) {
      case Kind::integer: case Kind::real: return numeric_literal(expr);
      case Kind::boolean:
        op.type = Type::boolean;
        op.integer = expr.text == "true";
        return op;
      case Kind::name: return name(expr);
      case Kind::unary: case Kind::binary: return eager(expr);
      case Kind::call: return call(expr);
      case Kind::make_object: return make(expr);
      case Kind::index: return index(expr);
      case Kind::field: return exception_field(expr);
      case Kind::raise: return braced_throw(expr);
      case Kind::try_scope: return try_statement(expr);
      case Kind::rebind: case Kind::transfer: return assign(expr);
      case Kind::grouped: return operand(expr, 0);
      case Kind::typed_expression:
        expect(expr.annotation == Type::void_type, expr.at, "replacement seed supports the void result assertion");
        operand(expr, 0);
        return op;
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
        operand(expr, 2);
        scopes_.pop_back();
        return op;
      }
      case Kind::return_value:
        if (expr.children.empty()) expect(method_.result.type == Type::void_type, expr.at, "return needs a value");
        else takes_value(operand(expr, 0), *method_.result.type, expr.at);
        op.type = Type::never;
        op.operand_type = *method_.result.type;
        return op;
      default:
        implementation_gap(expr.at, "replacement seed has no executable extraction for this source form");
      }
    }

    const CheckedOperation &extract(const Expression &expr) {
      expect(!expr.checked_ref_argument && !expr.spread && !expr.lexical_symbol_call &&
             !expr.definition && !expr.receiving,
             expr.at, "replacement seed has no extracted transport for this expression");
      auto op = operation(expr);
      if (op.view && !scalar_type(op.type) && op.type != Type::exception)
        op.discarded_view = true;
      const auto [row, first] = body_.instance.operations.emplace(&expr, std::move(op));
      if (!first) throw std::logic_error("replacement seed visited an operation twice");
      return row->second;
    }

  public:
    Frame(BodyExtraction &extraction, BoundBody &body)
        : extraction_(extraction), body_(body), method_(extraction.declarations_.method(body.instance.method)) {}

    void extract_body() {
      const auto parameters = method_.input_parameters();
      auto &frame = body_.instance.frame;
      for (size_t i = 0; i < parameters.size(); ++i) {
        const auto &parameter = parameters[i];
        const auto &type = method_.parameters[i];
        const auto mode = argument_mode(parameter);
        const bool slot = parameter.selects_caller_ref_slot();
        const bool native_handle = mode == ArgumentMode::borrowed_handle && type.kind == Type::native_handle;
        expect(!parameter.not_init_ref && !parameter.variadic && !parameter.automatic_conversion &&
               (!parameter.selects_caller_place() || slot) &&
               (mode == ArgumentMode::value ? scalar_type(type) : native_handle || ordinary_loan(mode, type)),
               parameter.at, "replacement seed needs a fixed scalar, array or native handle formal");
        const auto owner = parameter.owner.empty() ? size_t{0} : null_target - 1 - owner_role(parameter.owner, parameter.at);
        if (owner) frame.owners.try_emplace(owner, i);
        const bool value = mode != ArgumentMode::borrowed_ref && !native_handle;
        const auto local = declare(parameter.at, parameter.name, type, value,
                                   parameter.read_only || (mode == ArgumentMode::value && parameter.fixed_binding),
                                   parameter.fixed_binding || native_handle, owner, true);
        frame.inputs.push_back({*local.place, mode, parameter.selects_caller_place()});
      }
      expect(frame.owners.size() == method_.input_owners().size(), method_.source->at,
             "replacement seed needs an input for every formal owner");
      const auto &end = extract(*method_.source->body);
      expect(end.type == Type::never || method_.result.type == Type::void_type,
             method_.source->at, "replacement seed needs a reachable return for this result");
    }
  };

  // Dependency completion uses only collected allocations and call edges.
  // It cannot call the body visitor or mutate that visitor's output.
  std::vector<std::set<size_t>> complete_allocators() const {
    std::vector<std::set<size_t>> roles;
    for (const auto &body : bodies_) roles.push_back(body.allocations);
    std::vector<bool> complete(bodies_.size()), active(bodies_.size());
    struct Pending { size_t body; size_t call = 0; };
    std::vector<Pending> stack;
    for (size_t id = 0; id < bodies_.size(); ++id) {
      if (complete[id]) continue;
      active[id] = true;
      stack.push_back({id});
      while (!stack.empty()) {
        auto &current = stack.back();
        const auto &body = bodies_[current.body];
        if (current.call == body.calls.size()) {
          complete[current.body] = true;
          active[current.body] = false;
          stack.pop_back();
          continue;
        }
        const auto &call = body.calls[current.call];
        expect(!active[call.callee], call.site->at, "replacement seed has no completed recursive allocator contract");
        if (!complete[call.callee]) {
          active[call.callee] = true;
          stack.push_back({call.callee});
          continue;
        }
        for (const auto role : roles[call.callee]) {
          const auto actual = call.owners.find(role);
          expect(actual != call.owners.end(), call.site->at, "required allocation has no prepared owner capability");
          if (actual->second.kind == Allocator::input) roles[current.body].insert(actual->second.place);
        }
        ++current.call;
      }
    }
    return roles;
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
    for (size_t i = 0; i < bodies_.size(); ++i) Frame(*this, bodies_[i]).extract_body();
  }

  void seal(CheckedProgram &program) {
    const auto roles = complete_allocators();
    for (const auto &entry : program.native_entries)
      expect(roles[selected_.at(entry.method)].empty(), program.methods[entry.method].source->at,
             "a C entry cannot import an array allocator capability");
    for (size_t i = 0; i < bodies_.size(); ++i) {
      auto &body = bodies_[i];
      for (const auto &call : body.calls) {
        if (roles[call.callee].empty()) continue;
        auto arguments = std::make_shared<std::map<size_t, Allocator>>();
        for (const auto role : roles[call.callee]) arguments->emplace(role, call.owners.at(role));
        body.instance.operations.at(call.site).array_allocator_arguments = std::move(arguments);
      }
      body.instance.array_allocator_roles = roles[i];
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
