//===--- lang_checked.hh - LLVM-independent scalar obligations ------===//
#pragma once

#include "lang_syntax.hh"
#include "lang_diagnostics.hh"

#include <charconv>
#include <bit>
#include <cmath>
#include <cstdint>
#include <deque>
#include <limits>
#include <map>

namespace cod::language {

[[noreturn]] inline void implementation_gap(const Location &at, const std::string &detail) {
  fail(at, "micro-dispatch implementation gap: " + detail + "; this is not a language restriction");
}

inline bool scalar_type(const Type &type) { return numeric(type) || type == Type::boolean; }

struct CheckedOperation {
  Type type;
  Type operand_type;
  std::optional<size_t> callee;
  std::optional<size_t> place;
  std::optional<size_t> accumulator;
  bool read_only = false;
  bool view = false;
  std::uint64_t integer = 0;
  bool minimum_literal = false;
  bool initializes = false;
};

struct CheckedStorage {
  std::string name;
  Type type;
  size_t scope;
  bool read_only;
  bool value_binding;
};

struct LexicalState {
  struct Scope { size_t id; std::map<std::string, size_t> bindings; };
  std::vector<Scope> scopes{{0, {}}};
  std::set<size_t> available, initialized;

  std::optional<size_t> lookup(const std::string &name) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
      if (const auto found = scope->bindings.find(name); found != scope->bindings.end()) return found->second;
    return {};
  }
};

// Substitutions are checking obligations, not semantic methods or activation records.
struct CheckedInstance {
  size_t method;
  std::vector<Type> arguments;
  Type result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
  std::vector<CheckedStorage> storage;
};
struct CheckedMethod {
  const Form *source;
  std::vector<size_t> instances;
};
struct CheckedProgram {
  std::vector<CheckedMethod> methods;
  std::deque<CheckedInstance> instances;
  std::map<const Expression *, CheckedOperation> entry;
  std::vector<const Expression *> initializers;
  std::vector<WarningDiagnostic> warnings;
  std::vector<CheckedStorage> entry_storage;
};

class ScalarChecker {
  CheckedProgram program_;
  std::map<std::string, std::vector<size_t>> families_;
  std::set<std::string> print_symbols_;
  std::set<std::string> module_values_;
  std::set<size_t> warned_missing_return_;
  std::set<const Expression *> warned_shadow_;
  LexicalState lexical_;
  std::vector<CheckedStorage> *storage_ = nullptr;
  size_t next_scope_ = 1;
  static constexpr size_t instance_limit = 4096;

  size_t declare(const std::string &name, const Type &type, bool read_only, bool value_binding, const Location &at) {
    auto &scope = lexical_.scopes.back();
    if (scope.bindings.contains(name)) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
    const auto id = storage_->size();
    storage_->push_back({name, type, scope.id, read_only, value_binding});
    scope.bindings.emplace(name, id);
    lexical_.available.insert(id);
    return id;
  }
  static std::set<size_t> intersect(const std::set<size_t> &a, const std::set<size_t> &b) {
    std::set<size_t> result;
    for (const auto id : a) if (b.contains(id)) result.insert(id);
    return result;
  }
  void join(const LexicalState &a, bool a_exits, const LexicalState &b, bool b_exits, const Location &at) {
    lexical_ = a;
    for (size_t i = 0; i < b.scopes.size(); ++i)
      for (const auto &[name, id] : b.scopes[i].bindings) {
        auto [found, inserted] = lexical_.scopes[i].bindings.emplace(name, id);
        if (!inserted && found->second != id) fail(at, "duplicate declaration of '" + name + "' in one lexical scope");
      }
    lexical_.available = a_exits ? b.available : b_exits ? a.available : intersect(a.available, b.available);
    lexical_.initialized = a_exits ? b.initialized : b_exits ? a.initialized : intersect(a.initialized, b.initialized);
  }
  void leave_scope() {
    for (const auto &[name, id] : lexical_.scopes.back().bindings) {
      lexical_.available.erase(id);
      lexical_.initialized.erase(id);
    }
    lexical_.scopes.pop_back();
  }
  static std::optional<std::uint64_t> constant_integer(const Expression &expr,
      const std::map<const Expression *, CheckedOperation> &operations) {
    const auto &fact = operations.at(&expr);
    if (expr.kind == Expression::Kind::integer) return fact.integer;
    if (fact.minimum_literal) return std::uint64_t{1} << 63;
    if (expr.kind == Expression::Kind::unary && (expr.text == "+" || expr.text == "-"))
      if (const auto value = constant_integer(*expr.children.front(), operations))
        return expr.text == "-" ? std::uint64_t{0} - *value : *value;
    return {};
  }
  Type resolve_local_type(const Type &type, const CheckedInstance *instance, const Location &at) const {
    if (instance) {
      const auto &method = *program_.methods[instance->method].source;
      if (const auto index = binder(method, type)) return (*match(method, instance->arguments))[*index];
    }
    if (!scalar_type(type)) implementation_gap(at, "non-scalar local storage type '" + type_name(type) + "'");
    return type;
  }

  static std::optional<size_t> binder(const Form &method, const Type &type) {
    if (type.kind != Type::named) return {};
    for (size_t i = 0; i < method.type_parameters.size(); ++i)
      if (method.type_parameters[i].name == type.name) return i;
    return {};
  }
  static void signature_type(const Form &method, const Type &type, const Location &at, bool result = false) {
    if (scalar_type(type) || binder(method, type) || (result && type == Type::void_type)) return;
    implementation_gap(at, "non-scalar signature type '" + type_name(type) + "'");
  }
  static std::optional<std::vector<Type>> match(const Form &method, const std::vector<Type> &arguments) {
    if (method.parameters.size() != arguments.size()) return {};
    std::vector<Type> substitutions(method.type_parameters.size());
    for (size_t i = 0; i < arguments.size(); ++i) {
      const auto &parameter = method.parameters[i].type;
      if (const auto index = binder(method, parameter)) {
        auto &bound = substitutions[*index];
        if (bound != Type::void_type && bound != arguments[i]) return {};
        bound = arguments[i];
      } else if (parameter != arguments[i]) return {};
    }
    return substitutions;
  }
  // Signature containment includes repeated binders; layout equality is irrelevant.
  static bool implies(const Form &narrow, const Form &broad) {
    if (narrow.parameters.size() != broad.parameters.size()) return false;
    const auto same_narrow = [&](size_t a, size_t b) {
      const auto x = binder(narrow, narrow.parameters[a].type);
      const auto y = binder(narrow, narrow.parameters[b].type);
      return x && y ? x == y : !x && !y && narrow.parameters[a].type == narrow.parameters[b].type;
    };
    for (size_t i = 0; i < broad.parameters.size(); ++i) {
      const auto broad_binder = binder(broad, broad.parameters[i].type);
      if (!broad_binder) {
        if (binder(narrow, narrow.parameters[i].type) || narrow.parameters[i].type != broad.parameters[i].type)
          return false;
      } else {
        for (size_t j = 0; j < i; ++j)
          if (binder(broad, broad.parameters[j].type) == broad_binder && !same_narrow(i, j)) return false;
      }
    }
    return true;
  }
  size_t demand(size_t method, const std::vector<Type> &arguments, const Location &at) {
    auto &definition = program_.methods[method];
    for (const auto id : definition.instances)
      if (program_.instances[id].arguments == arguments) return id;
    if (program_.instances.size() == instance_limit)
      fail(at, "compiler resource limit: 4096 scalar checking obligations while preparing '" + definition.source->name + "'");
    const auto substitutions = match(*definition.source, arguments);
    if (!substitutions) throw std::logic_error("unchecked method demand");
    auto result = *definition.source->result_type;
    if (const auto index = binder(*definition.source, result)) {
      result = (*substitutions)[*index];
      if (result == Type::void_type)
        implementation_gap(at, "result type parameter is not determined by the scalar arguments");
    }
    const auto id = program_.instances.size();
    program_.instances.push_back({method, arguments, result, {}, false, {}});
    definition.instances.push_back(id);
    return id;
  }
  size_t select(const Expression &expr, const std::vector<Type> &arguments) {
    const auto family = families_.find(expr.text);
    if (family == families_.end()) fail(expr.at, "unknown callable '" + expr.text + "'");
    std::vector<size_t> applicable;
    for (const auto method : family->second)
      if (match(*program_.methods[method].source, arguments)) applicable.push_back(method);
    std::vector<size_t> best;
    for (const auto candidate : applicable) {
      const auto &definition = *program_.methods[candidate].source;
      const bool dominated = std::any_of(applicable.begin(), applicable.end(), [&](size_t other) {
        const auto &alternative = *program_.methods[other].source;
        return implies(alternative, definition) && !implies(definition, alternative);
      });
      if (!dominated) best.push_back(candidate);
    }
    if (best.empty()) fail(expr.at, "no applicable method for '" + expr.text + "' (argument types or same-T relation do not match)");
    if (best.size() != 1) fail(expr.at, "ambiguous methods for '" + expr.text + "'");
    return demand(best.front(), arguments, expr.at);
  }
  Type check(const Expression &expr, CheckedInstance *instance,
             std::map<const Expression *, CheckedOperation> &operations, bool read = true) {
    using K = Expression::Kind;
    CheckedOperation operation;
    const auto child = [&](size_t i, bool read_value = true) { return check(*expr.children.at(i), instance, operations, read_value); };
    const auto view = [&](const CheckedOperation &source) {
      operation.place = source.place;
      operation.view = source.view;
      operation.read_only = source.read_only;
    };
    const auto require_value = [&](const Type &type) {
      if (!scalar_type(type)) implementation_gap(expr.at, "non-scalar or abruptly completing operand");
    };
    const auto &method = instance ? program_.methods[instance->method].source : nullptr;
    if (instance && !lexical_.lookup(expr.text) && module_values_.contains(expr.text) &&
        (expr.kind == K::name || expr.kind == K::call || expr.kind == K::perform || expr.kind == K::behave))
      implementation_gap(expr.at, "module-local binding lookup and capture in callable bodies");
    if (expr.spread) implementation_gap(expr.at, "spread arguments");
    switch (expr.kind) {
    case K::integer: {
      auto digits = std::string_view(expr.text);
      operation.type = Type::integer;
      if (digits.ends_with("u8")) { operation.type = Type::uint8; digits.remove_suffix(2); }
      else if (digits.ends_with("u32")) { operation.type = Type::uint32; digits.remove_suffix(3); }
      else if (digits.ends_with("u64")) { operation.type = Type::uint64; digits.remove_suffix(3); }
      const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), operation.integer);
      const auto limit = operation.type == Type::uint8 ? std::uint64_t{255} :
          operation.type == Type::uint32 ? std::uint64_t{0xffffffff} :
          operation.type == Type::integer ? std::uint64_t{0x7fffffffffffffff} : std::numeric_limits<std::uint64_t>::max();
      if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() || operation.integer > limit)
        fail(expr.at, "integer literal is outside " + type_name(operation.type));
      break;
    }
    case K::real: {
      operation.type = expr.text.ends_with("f32") ? Type::float32 : Type::float64;
      auto digits = std::string_view(expr.text);
      if (digits.ends_with("f32") || digits.ends_with("f64")) digits.remove_suffix(3);
      const auto in_range = [&](auto value) {
        const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), value);
        return parsed.ec == std::errc{} && parsed.ptr == digits.data() + digits.size() && std::isfinite(value);
      };
      if (!(operation.type == Type::float32 ? in_range(float{}) : in_range(double{})))
        fail(expr.at, "floating literal is outside " + type_name(operation.type) + " range");
      break;
    }
    case K::boolean: operation.type = Type::boolean; operation.integer = expr.text == "true"; break;
    case K::name: {
      const auto binding = lexical_.lookup(expr.text);
      if (!binding) fail(expr.at, "unknown scalar value '" + expr.text + "'");
      if (!lexical_.available.contains(*binding)) fail(expr.at, "binding '" + expr.text + "' is not available on every reaching path");
      if (read && !lexical_.initialized.contains(*binding)) fail(expr.at, "read of '" + expr.text + "' before definite initialization");
      operation.place = binding;
      operation.type = (*storage_)[*binding].type;
      operation.read_only = (*storage_)[*binding].read_only;
      operation.view = true;
      break;
    }
    case K::unary: {
      if (expr.text == "-" && expr.children.front()->kind == K::integer) {
        auto magnitude = std::string_view(expr.children.front()->text);
        while (magnitude.starts_with('0')) magnitude.remove_prefix(1);
        if (magnitude == "9223372036854775808") {
          operation.type = operation.operand_type = Type::integer;
          operation.minimum_literal = true;
          break;
        }
      }
      const auto argument = child(0);
      require_value(argument);
      if (expr.text == "!") {
        if (argument != Type::boolean) fail(expr.at, "logical negation requires bool");
      } else if (argument != Type::integer && !(expr.text == "-" && floating(argument)))
        fail(expr.at, "numeric unary operation requires i64, or floating point for negation");
      operation.type = operation.operand_type = argument;
      break;
    }
    case K::binary: {
      const bool logical = expr.text == "&&" || expr.text == "||";
      const auto left = child(0);
      const auto before_right = lexical_;
      const auto right = child(1);
      const auto after_right = lexical_;
      if (logical) join(before_right, false, after_right, right == Type::never, expr.at);
      require_value(left); require_value(right);
      const bool equality = expr.text == "==" || expr.text == "!=";
      const bool comparison = equality || expr.text == "<" || expr.text == ">" || expr.text == "<=" || expr.text == ">=";
      if (logical || (equality && left == Type::boolean && right == Type::boolean)) {
        if (left != Type::boolean || right != Type::boolean) fail(expr.at, "logical operation requires bool operands");
        operation.operand_type = Type::boolean;
      } else {
        if (!numeric(left) || !numeric(right)) fail(expr.at, "no applicable numeric operation '" + expr.text + "'");
        if (widens(left, right)) operation.operand_type = right;
        else if (widens(right, left)) operation.operand_type = left;
        else fail(expr.at, "numeric operands have no lossless common type");
        if (expr.text == "%" && floating(operation.operand_type)) fail(expr.at, "remainder requires integer operands");
      }
      operation.type = comparison || logical ? Type{Type::boolean} : operation.operand_type;
      break;
    }
    case K::call: {
      if (lexical_.lookup(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not callable");
      std::vector<Type> arguments;
      for (size_t i = 0; i < expr.children.size(); ++i) { arguments.push_back(child(i)); require_value(arguments.back()); }
      operation.callee = select(expr, arguments);
      const auto &target = program_.instances[*operation.callee];
      if (method && method->kind == Form::Kind::function && program_.methods[target.method].source->kind == Form::Kind::procedure)
        fail(expr.at, "a function cannot invoke a procedure");
      operation.type = target.result;
      break;
    }
    case K::behave: case K::perform: {
      if (lexical_.lookup(expr.text))
        fail(expr.at, "nearest binding '" + expr.text + "' is not a symbol");
      if (!print_symbols_.contains(expr.text)) implementation_gap(expr.at, "dynamic procedure call other than selected std:io print");
      if (method && method->kind == Form::Kind::function) fail(expr.at, "a function cannot invoke a dynamic procedure");
      if (expr.children.size() != 1) fail(expr.at, "print requires one scalar argument");
      require_value(child(0));
      operation.type = Type::void_type;
      break;
    }
    case K::conditional: {
      if (child(0) != Type::boolean) fail(expr.at, "if condition requires bool");
      const auto before = lexical_;
      const auto yes = child(1, read);
      const auto yes_state = lexical_;
      lexical_ = before;
      const auto no = child(2, read);
      const auto no_state = lexical_;
      join(yes_state, yes == Type::never, no_state, no == Type::never, expr.at);
      if (yes == Type::never) operation.type = no;
      else if (no == Type::never || yes == no) operation.type = yes;
      else implementation_gap(expr.at, "conditional join with different scalar result types");
      const auto &yes_fact = operations.at(expr.children[1].get()), &no_fact = operations.at(expr.children[2].get());
      if (yes == Type::never) view(no_fact);
      else if (no == Type::never) view(yes_fact);
      else {
        operation.view = yes_fact.view || no_fact.view;
        operation.read_only = yes_fact.read_only || no_fact.read_only;
        if (yes_fact.place == no_fact.place) operation.place = yes_fact.place;
      }
      break;
    }
    case K::block: {
      if (expr.scoped) lexical_.scopes.push_back({next_scope_++, {}});
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        operation.type = child(i, i + 1 == expr.children.size() ? read : true);
        exits = exits || operation.type == Type::never;
      }
      if (!expr.children.empty()) view(operations.at(expr.children.back().get()));
      if (expr.scoped) leave_scope();
      if (exits) operation.type = Type::never;
      break;
    }
    case K::range_for: case K::fold: {
      const bool fold = expr.kind == K::fold;
      const auto first = child(0), last = child(1);
      if (!integral(first) || !integral(last)) fail(expr.at, "a range requires integer endpoints");
      if (first != last) implementation_gap(expr.at, "range endpoints with different integer types");
      Type accumulator;
      if (fold) { accumulator = child(2); require_value(accumulator); }
      const auto before = lexical_;
      lexical_.scopes.push_back({next_scope_++, {}});
      operation.place = declare(expr.text, first, true, true, expr.at);
      lexical_.initialized.insert(*operation.place);
      if (fold) {
        operation.accumulator = declare(expr.binding, accumulator, true, true, expr.at);
        lexical_.initialized.insert(*operation.accumulator);
      }
      const auto body = child(fold ? 3 : 2);
      if (fold && body != Type::never && body != accumulator) {
        if (widens(body, accumulator)) implementation_gap(expr.at, "fold accumulator conversion");
        fail(expr.at, "fold body does not satisfy the accumulator type");
      }
      leave_scope();
      const auto body_state = lexical_;
      lexical_ = before;
      const auto otherwise = !fold && expr.children.size() == 4 ? child(3) : Type{Type::void_type};
      const auto empty_state = lexical_;
      bool body_possible = true, empty_possible = true;
      const auto start = constant_integer(*expr.children[0], operations), end = constant_integer(*expr.children[1], operations);
      if (start && end) {
        body_possible = first == Type::integer ? std::bit_cast<std::int64_t>(*start) < std::bit_cast<std::int64_t>(*end) : *start < *end;
        empty_possible = !body_possible;
      }
      const bool body_exits = !body_possible || body == Type::never;
      const bool empty_exits = !empty_possible || otherwise == Type::never;
      join(body_state, body_exits, empty_state, empty_exits, expr.at);
      operation.type = body_exits && empty_exits ? Type{Type::never} : fold ? accumulator : Type{Type::void_type};
      operation.operand_type = first;
      break;
    }
    case K::pointer_group: {
      if (expr.declaration_kind != "val") implementation_gap(expr.at, "reference bindings and alias lifetime tracking");
      for (const auto &declaration : expr.children) {
        if (declaration->kind == K::block) { check(*declaration, instance, operations); continue; }
        if (!declaration->owner_name.empty()) fail(declaration->at, "val storage belongs to its lexical scope, not an external owner");
        const auto &initializer = *declaration->children.front();
        Type initial_type;
        bool initialized = true;
        if (initializer.kind == K::make_local) {
          if (initializer.annotation) initial_type = resolve_local_type(*initializer.annotation, instance, initializer.at);
          else {
            const auto &query = *initializer.children.front();
            if (query.kind != K::name) implementation_gap(query.at, "typeof on a non-name local initializer");
            const auto binding = lexical_.lookup(query.text);
            if (!binding) fail(query.at, "unknown value in typeof: '" + query.text + "'");
            initial_type = (*storage_)[*binding].type;
          }
          const size_t value_index = initializer.annotation ? 0 : 1;
          initialized = initializer.children.size() > value_index;
          if (initialized) {
            const auto value_type = check(*initializer.children[value_index], instance, operations);
            if (!widens(value_type, initial_type)) fail(initializer.at, "initializer does not satisfy local storage type");
          }
          CheckedOperation allocation;
          allocation.type = initial_type;
          allocation.initializes = initialized;
          operations[&initializer] = allocation;
        } else initial_type = check(initializer, instance, operations);
        require_value(initial_type);
        const auto type = declaration->annotation ? resolve_local_type(*declaration->annotation, instance, declaration->at) : initial_type;
        if (!widens(initial_type, type)) fail(declaration->at, "initializer does not satisfy declared val type");
        if (lexical_.lookup(declaration->text) && !lexical_.scopes.back().bindings.contains(declaration->text) &&
            warned_shadow_.insert(declaration.get()).second)
          program_.warnings.push_back({Warning::shadow, declaration->at, "declaration shadows outer binding '" + declaration->text + "'"});
        const auto id = declare(declaration->text, type, declaration->read_only || expr.fixed_binding, true, declaration->at);
        if (initialized) lexical_.initialized.insert(id);
        CheckedOperation binding;
        binding.place = id;
        binding.operand_type = type;
        binding.initializes = initialized;
        operations[declaration.get()] = binding;
      }
      break;
    }
    case K::transfer: case K::rebind: {
      const auto destination = child(0, false);
      const auto &target = operations.at(expr.children[0].get());
      if (!target.view) fail(expr.at, "assignment requires a storage destination");
      if (!target.place) implementation_gap(expr.at, "assignment through a runtime-selected storage view");
      const auto id = *target.place;
      const auto &storage = (*storage_)[id];
      if (std::none_of(lexical_.scopes.begin(), lexical_.scopes.end(),
          [&](const LexicalState::Scope &scope) { return scope.id == storage.scope; }))
        fail(expr.at, "assignment destination's lexical owner has ended");
      if (expr.kind == K::rebind) {
        if (storage.value_binding) fail(expr.at, "val storage has no reference binding; use '<-' for bits transfer");
        implementation_gap(expr.at, "reference rebinding and alias lifetime tracking");
      }
      if (target.read_only) fail(expr.at, "cannot write through a readonly destination");
      const auto source = child(1);
      if (!widens(source, destination)) fail(expr.at, "bits transfer source does not satisfy destination type");
      lexical_.initialized.insert(id);
      operation.type = operation.operand_type = destination;
      operation.place = id;
      operation.read_only = operation.view = true;
      break;
    }
    case K::return_value: {
      if (!instance) fail(expr.at, "return requires an enclosing callable");
      const auto value = expr.children.empty() ? Type{Type::void_type} : child(0);
      if (value != Type::never && !widens(value, instance->result))
        fail(expr.at, "return type '" + type_name(value) + "' does not satisfy '" + type_name(instance->result) + "'");
      operation.type = Type::never;
      operation.operand_type = instance->result;
      break;
    }
    case K::deferred: {
      if (expr.children.size() != 1 || expr.children.front()->kind == K::block)
        implementation_gap(expr.at, "deferred blocks and exception handlers");
      child(0);
      operation.type = Type::void_type;
      break;
    }
    default: implementation_gap(expr.at, "source expression '" + expr.text + "' is outside the scalar-method foundation");
    }
    operations[&expr] = operation;
    return operation.type;
  }

public:
  CheckedProgram check(const std::vector<std::unique_ptr<Form>> &forms) {
    for (const auto &form : forms) {
      if (form->kind == Form::Kind::expression) { program_.initializers.push_back(form->body.get()); continue; }
      if (form->kind == Form::Kind::using_resource && form->using_symbol && form->path == "std:io" && form->name == "print") {
        if (!print_symbols_.insert(form->alias).second) fail(form->at, "duplicate print symbol binding");
        continue;
      }
      if (form->kind != Form::Kind::function && form->kind != Form::Kind::procedure)
        implementation_gap(form->at, "module declaration or selection '" + form->name + "'");
      if (form->symbolic || form->operation || form->conversion || form->explicit_captures || !form->owner_parameters.empty() ||
          !form->throws_types.empty() || !form->body || !form->result_owner.empty())
        implementation_gap(form->at, "callable captures, effects, declarations, operators, owners or throws contracts");
      if (!form->result_type) implementation_gap(form->at, "omitted callable result type");
      if (*form->result_type != Type::void_type && (!form->result_const || !form->result_kind.empty()))
        implementation_gap(form->at, "non-const scalar result contract");
      signature_type(*form, *form->result_type, form->at, true);
      std::set<std::string> names;
      for (const auto &parameter : form->parameters) {
        if (!names.insert(parameter.name).second) fail(parameter.at, "duplicate parameter name");
        if (parameter.variadic || parameter.automatic_conversion || !parameter.owner.empty())
          implementation_gap(parameter.at, "variadic, conversion or borrowed parameter contract");
        signature_type(*form, parameter.type, parameter.at);
      }
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      const auto id = program_.methods.size();
      for (const auto other : families_[form->name]) {
        const auto &definition = *program_.methods[other].source;
        if (implies(*form, definition) && implies(definition, *form)) {
          for (size_t p = 0; p < form->parameters.size(); ++p)
            if (form->parameters[p].read_only != definition.parameters[p].read_only)
              implementation_gap(form->at, "target-access-qualified overload selection");
          fail(form->at, "duplicate method signature for '" + form->name + "'");
        }
        if (definition.kind != form->kind) implementation_gap(form->at, "mixed function/procedure family");
      }
      program_.methods.push_back({form.get(), {}});
      families_[form->name].push_back(id);
    }
    for (const auto &symbol : print_symbols_)
      if (families_.contains(symbol)) fail(program_.methods[families_[symbol].front()].source->at, "callable conflicts with symbol '" + symbol + "'");
    for (size_t i = 0; i < program_.methods.size(); ++i) {
      const auto &method = *program_.methods[i].source;
      if (!method.type_parameters.empty()) continue;
      std::vector<Type> arguments;
      for (const auto &parameter : method.parameters) arguments.push_back(parameter.type);
      demand(i, arguments, method.at);
    }
    storage_ = &program_.entry_storage;
    lexical_ = {};
    next_scope_ = 1;
    for (const auto *expr : program_.initializers) check(*expr, nullptr, program_.entry);
    for (const auto &[name, id] : lexical_.scopes.front().bindings) module_values_.insert(name);
    for (size_t i = 0; i < program_.instances.size(); ++i) {
      auto &instance = program_.instances[i];
      const auto &method = *program_.methods[instance.method].source;
      storage_ = &instance.storage;
      lexical_ = {};
      next_scope_ = 1;
      for (size_t p = 0; p < method.parameters.size(); ++p) {
        const auto &parameter = method.parameters[p];
        const bool value = parameter.declaration_kind == "val";
        const auto id = declare(parameter.name, instance.arguments[p], parameter.read_only || (value && parameter.fixed_binding), value, parameter.at);
        lexical_.initialized.insert(id);
      }
      const auto completion = check(*method.body, &instance, instance.operations);
      if (instance.result != Type::void_type && completion != Type::never && warned_missing_return_.insert(instance.method).second)
        program_.warnings.push_back({Warning::missing_return, method.at, "callable may finish without an explicit return"});
      instance.complete = true;
    }
    return std::move(program_);
  }
};

} // namespace cod::language
