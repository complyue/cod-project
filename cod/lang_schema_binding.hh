//===--- lang_schema_binding.hh - Selected nominal field layouts -------===//
#pragma once

#include "lang_checked_program.hh"

namespace cod::language {

inline bool matches_native_handle(const Type &type, const Type &marker) {
  return type.kind == Type::native_handle && type.identity == marker.identity;
}

inline const CheckedSchema &scalar_exception_schema(const CheckedProgram &program, const Type &type, const Location &at) {
  if (!nominal_exception(type)) fail(at, "braced throw requires a concrete library-defined exception type");
  const auto &schema = program.schemas.at(*program.exception_types.at(type.identity).schema);
  if (!std::ranges::all_of(schema.fields, [](const auto &field) {
        return field.storage == PlaceKind::value && scalar_type(field.type);
      })) implementation_gap(at, "owned exception fields other than direct scalar bits");
  return schema;
}
inline size_t product_field(const CheckedSchema &schema, const Expression &member) {
  for (size_t i = 0; i < schema.fields.size(); ++i)
    if (schema.fields[i].source->name == member.text) return i;
  fail(member.at, "unknown field '" + member.text + "' of '" + schema.source->name + "'");
}
inline bool readonly_field(const CheckedSchema::Field &field) {
  return field.source->read_only || field.source->fixed_binding;
}

inline std::vector<size_t> product_initializers(const CheckedSchema &schema, const Expression &expr) {
  std::set<size_t> single_initialization;
  std::vector<size_t> order;
  for (const auto &item : expr.initializer_items()) {
    const auto *label = item->initializer_field_label();
    if (!label) fail(item->at, "product make requires named field initializers");
    const auto index = product_field(schema, *label);
    const auto &field = schema.fields[index];
    const bool direct = field.storage == PlaceKind::value;
    const bool rebind = item->kind == Expression::Kind::rebind || item->binding == "=";
    if (field.direct_text_place() && rebind)
      fail(label->at, "a direct String field initializer uses a selected '<-' method");
    if (direct && rebind) fail(label->at, "a direct product field requires bits transfer '<-'");
    if (!direct && !rebind) fail(label->at, "a reference field initializer requires binding '='");
    const bool readonly = direct ? readonly_field(field) : field.source->fixed_binding;
    const bool inline_product = direct && field.type.kind == Type::product;
    if ((readonly || inline_product) && !single_initialization.insert(index).second) {
      if (readonly)
        fail(label->at, "readonly field '" + label->text + "' is initialized more than once");
      implementation_gap(label->at, "repeated inline product initialization");
    }
    order.push_back(index);
  }
  return order;
}

} // namespace cod::language
