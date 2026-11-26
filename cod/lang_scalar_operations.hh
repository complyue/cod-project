//===--- lang_scalar_operations.hh - Scalar operation binding ---------===//
#pragma once

#include "lang_checked_program.hh"

#include <charconv>
#include <cmath>
#include <limits>

namespace cod::language {

struct EagerTypes {
  Type result, operand;
  bool unary = false, failing = false;
  std::string error;
};
inline std::optional<Type> common_value_type(const std::vector<Type> &values) {
  if (values.empty()) return {};
  auto candidates = values;
  for (const auto kind : {Type::uint8, Type::uint32, Type::uint64, Type::integer, Type::float32, Type::float64})
    candidates.push_back(Type{kind});
  candidates.emplace_back(Type::exception, exception_categories.front().name);
  std::vector<Type> common;
  for (const auto &candidate : candidates)
    if (std::find(common.begin(), common.end(), candidate) == common.end() &&
        std::all_of(values.begin(), values.end(), [&](const auto &type) { return transfers_value(type, candidate); }))
      common.push_back(candidate);
  std::optional<Type> least;
  for (const auto &candidate : common)
    if (std::all_of(common.begin(), common.end(), [&](const auto &other) { return transfers_value(candidate, other); })) {
      if (least && *least != candidate) return {};
      least = candidate;
    }
  return least;
}
inline EagerTypes eager_types(std::string_view name, const std::vector<Type> &arguments) {
  EagerTypes types;
  types.unary = arguments.size() == 1 && (name == "+" || name == "-" || name == "!");
  const auto error = [&](const std::string &message) { types.error = message; return types; };
  if (!types.unary && (arguments.size() != 2 || name == "!")) return error("builtin operator argument count does not match");
  const bool completes = std::none_of(arguments.begin(), arguments.end(), [](const Type &type) { return type == Type::never; });
  if (types.unary) {
    const auto &argument = arguments.front();
    if (!completes) { types.result = types.operand = Type::never; return types; }
    if (name == "!") {
      if (argument != Type::boolean) return error("logical negation requires bool");
    } else if (argument != Type::integer && !(name == "-" && floating(argument)))
      return error("numeric unary operation requires i64, or floating point for negation");
    types.result = types.operand = argument;
    types.failing = name == "-" && argument == Type::integer;
  } else {
    const bool equality = name == "==" || name == "!=";
    const bool comparison = equality || name == "<" || name == ">" || name == "<=" || name == ">=";
    std::vector<Type> values;
    for (const auto &type : arguments) if (type != Type::never) values.push_back(type);
    if (values.empty()) { types.result = types.operand = Type::never; return types; }
    if (equality && (values.front() == Type::boolean || values.front().kind == Type::enumeration) &&
        std::ranges::all_of(values, [&](const auto &type) { return type == values.front(); })) types.operand = values.front();
    else if (comparison && values.size() == 2 && supplies_enum_code(values[0], values[1])) types.operand = values[1];
    else if (comparison && values.size() == 2 && supplies_enum_code(values[1], values[0])) types.operand = values[0];
    else {
      if (!std::all_of(values.begin(), values.end(), numeric)) return error("no applicable numeric operation '" + std::string(name) + "'");
      if (const auto common = common_value_type(values)) types.operand = *common;
      else return error("numeric operands have no lossless common type");
      if (name == "%" && floating(types.operand)) return error("remainder requires integer operands");
    }
    types.result = !completes ? Type{Type::never} : comparison ? Type{Type::boolean} : types.operand;
    types.failing = completes && !comparison && integral(types.operand);
  }
  return types;
}

inline CheckedOperation numeric_literal(const Expression &expr) {
  using K = Expression::Kind;
  CheckedOperation operation;
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
  default: throw std::logic_error("expected scalar literal");
  }
  return operation;
}

} // namespace cod::language
