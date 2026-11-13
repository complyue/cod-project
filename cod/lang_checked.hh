//===--- lang_checked.hh - LLVM-independent scalar obligations ------===//
#pragma once

#include "lang_syntax.hh"

#include <charconv>
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
  size_t parameter = 0;
  std::uint64_t integer = 0;
  bool minimum_literal = false;
};

// Substitutions are checking obligations, not semantic methods or activation records.
struct CheckedInstance {
  size_t method;
  std::vector<Type> arguments;
  Type result;
  std::map<const Expression *, CheckedOperation> operations;
  bool complete = false;
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
  std::vector<std::string> warnings;
};

class ScalarChecker {
  CheckedProgram program_;
  std::map<std::string, std::vector<size_t>> families_;
  std::set<std::string> print_symbols_;
  std::set<size_t> warned_missing_return_;
  static constexpr size_t instance_limit = 4096;

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
    program_.instances.push_back({method, arguments, result, {}, false});
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
             std::map<const Expression *, CheckedOperation> &operations) {
    using K = Expression::Kind;
    CheckedOperation operation;
    const auto child = [&](size_t i) { return check(*expr.children.at(i), instance, operations); };
    const auto require_value = [&](const Type &type) {
      if (!scalar_type(type)) implementation_gap(expr.at, "non-scalar or abruptly completing operand");
    };
    const auto &method = instance ? program_.methods[instance->method].source : nullptr;
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
      if (method) {
        const auto parameter = std::find_if(method->parameters.begin(), method->parameters.end(),
            [&](const Parameter &value) { return value.name == expr.text; });
        if (parameter != method->parameters.end()) {
          operation.parameter = static_cast<size_t>(parameter - method->parameters.begin());
          operation.type = instance->arguments[operation.parameter];
          break;
        }
      }
      fail(expr.at, "unknown scalar value '" + expr.text + "'");
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
      const auto left = child(0), right = child(1);
      require_value(left); require_value(right);
      const bool logical = expr.text == "&&" || expr.text == "||";
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
      if (method && std::any_of(method->parameters.begin(), method->parameters.end(),
          [&](const Parameter &parameter) { return parameter.name == expr.text; }))
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
      if (method && std::any_of(method->parameters.begin(), method->parameters.end(),
          [&](const Parameter &parameter) { return parameter.name == expr.text; }))
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
      const auto yes = child(1), no = child(2);
      if (yes == Type::never) operation.type = no;
      else if (no == Type::never || yes == no) operation.type = yes;
      else implementation_gap(expr.at, "conditional join with different scalar result types");
      break;
    }
    case K::block: {
      bool exits = false;
      for (size_t i = 0; i < expr.children.size(); ++i) {
        operation.type = child(i);
        exits = exits || operation.type == Type::never;
      }
      if (exits) operation.type = Type::never;
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
        if (parameter.variadic || parameter.automatic_conversion || !parameter.owner.empty() ||
            (!parameter.declaration_kind.empty() && parameter.declaration_kind != "val"))
          implementation_gap(parameter.at, "variadic, conversion or borrowed parameter contract");
        signature_type(*form, parameter.type, parameter.at);
      }
      for (const auto &parameter : form->type_parameters)
        if (parameter.bound) implementation_gap(parameter.at, "bounded type parameters");
      const auto id = program_.methods.size();
      for (const auto other : families_[form->name]) {
        const auto &definition = *program_.methods[other].source;
        if (implies(*form, definition) && implies(definition, *form)) fail(form->at, "duplicate method signature for '" + form->name + "'");
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
    for (const auto *expr : program_.initializers) check(*expr, nullptr, program_.entry);
    for (size_t i = 0; i < program_.instances.size(); ++i) {
      auto &instance = program_.instances[i];
      const auto &method = *program_.methods[instance.method].source;
      const auto completion = check(*method.body, &instance, instance.operations);
      if (instance.result != Type::void_type && completion != Type::never && warned_missing_return_.insert(instance.method).second)
        program_.warnings.push_back(location_text(method.at) + ": warning: callable may finish without an explicit return [-Wmissing-return]");
      instance.complete = true;
    }
    return std::move(program_);
  }
};

} // namespace cod::language
