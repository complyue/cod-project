//===--- lang_schema_binding.hh - Selected nominal field layouts -------===//
#pragma once

#include "lang_checked_program.hh"

namespace cod::language {

inline const Expression &ungrouped_expression(const Expression &expr) {
  const auto *operand = &expr;
  while (operand->kind == Expression::Kind::grouped) operand = operand->children.front().get();
  return *operand;
}

inline bool matches_native_handle(const Type &type, const Type &marker) {
  return type.kind == Type::native_handle && type.identity == marker.identity;
}

inline size_t select_native_constructor(const CheckedProgram &program, const Type &declared,
                                        const Type &constructed, const Expression &expr) {
  if (declared.kind != Type::c_type_marker || constructed.kind != Type::c_type_marker)
    fail(expr.at, "native handle construction requires a 'handle type' declaration");
  if (declared != constructed) fail(expr.at, "C handle constructor must match the declared owner type");
  const bool attached = expr.kind == Expression::Kind::make_borrowed_handle;
  const bool borrow_only = program.schemas.at(constructed.identity).native_owner.has_value();
  if (borrow_only && !attached)
    fail(expr.at, "borrow-only CType '" + constructed.name + "' cannot be acquired with 'make own handle'");
  if (attached && !borrow_only)
    fail(expr.at, "make handle requires a borrow-only CType with a declared owning CType");
  const auto constructor = std::ranges::find(program.native_constructors, constructed, &CheckedNativeConstructor::handle);
  if (constructor == program.native_constructors.end()) fail(expr.at, "no C constructor declared for '" + constructed.name + "'");
  if (constructor->inputs.size() != expr.children.size()) fail(expr.at, "wrong number of C constructor arguments");
  return std::distance(program.native_constructors.begin(), constructor);
}

inline Type native_handle_type(Type marker, bool owned) {
  marker.kind = Type::native_handle;
  marker.owned = owned;
  return marker;
}

inline void check_native_invocation_kind(const Form *caller, const Form &callee, const Location &at,
                                         bool prepared = false) {
  if (callee.kind == Form::Kind::function) {
    if (prepared) fail(at, "defer call requires a procedure");
  } else if (caller && caller->kind == Form::Kind::function) {
    fail(at, "a function cannot invoke a procedure");
  }
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

inline CasePattern bind_case_pattern(const CheckedProgram &program, const Type &subject, const Expression &arm,
                                     const std::map<std::string, SumConstructor> &constructors) {
  using K = CasePattern::Kind;
  if (arm.receiving && (subject.kind != Type::sum || arm.text == "_" || arm.text == "null"))
    implementation_gap(arm.at, "receiving fields in this case pattern");
  if (arm.text == "_") return {};
  if (arm.text == "null") return {K::null};
  if (subject.kind == Type::sum) {
    const auto constructor = constructors.find(arm.text);
    if (constructor == constructors.end() || constructor->second.sum != subject.identity)
      fail(arm.at, "'" + arm.text + "' is not a selected data constructor of '" + subject.name + "'");
    return {K::code, constructor->second.alternative, constructor->second};
  }
  if (subject == Type::boolean) {
    if (arm.text == "true" || arm.text == "false") return {K::code, arm.text == "true"};
    fail(arm.at, "a bool case pattern requires true, false, null or a catch-all");
  }
  if (subject == Type::null_ref) fail(arm.at, "a null case subject requires a null pattern or a catch-all");
  const auto &domain = program.enums.at(subject.identity);
  const auto spelling = domain.spellings.find(arm.text);
  if (spelling != domain.spellings.end()) {
    const auto canonical = domain.members.at(spelling->second).canonical;
    return {K::code, domain.members.at(canonical).code};
  }
  fail(arm.at, "unknown member '" + arm.text + "' of enum '" + subject.name + "'");
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

inline void bits_payload_type(const CheckedProgram &program, const Type &source, const Type &destination,
                              const Location &at) {
  if (!inline_aggregate_type(destination)) {
    if (!transfers_value(source, destination)) fail(at, "bits transfer source does not satisfy destination type");
    return;
  }
  if (!program.bits_payload(destination))
    implementation_gap(at, "whole-payload transfer requires recursively bits fields");
  if (source != destination) fail(at, "aggregate bits transfer requires the exact destination type");
}

struct CaseField {
  size_t field;
  Type type;
  bool read_only;
};

template <typename Resolve>
std::vector<CaseField> bind_case_fields(const CheckedProgram &program, const Expression &arm,
                                      const CheckedSum::Alternative &alternative, Resolve resolve) {
  std::vector<CaseField> result;
  for (const auto &item : arm.receiving->items) {
    if (item.declaration_kind != "val" || item.operation != "<-" || !item.owner.empty())
      implementation_gap(item.at, "sum case reception outside a fresh direct bits value");
    const auto found = std::ranges::find_if(alternative.fields, [&](const auto &field) {
      return field.source->name == item.selector;
    });
    if (found == alternative.fields.end())
      fail(item.at, "unknown field '" + item.selector + "' of '" + alternative.source->name.text + "'");
    const auto type = item.annotation ? resolve(*item.annotation) : found->type;
    bits_payload_type(program, found->type, type, item.at);
    result.push_back({static_cast<size_t>(found - alternative.fields.begin()), type, item.read_only || item.fixed_binding});
  }
  return result;
}

inline const CheckedSchema &direct_region_roots(const CheckedProgram &program, const Type &schema, const Location &at) {
  if (schema.kind != Type::region_schema) fail(at, "make region requires a named region root schema");
  const auto &roots = program.schemas.at(schema.identity);
  for (const auto &root : roots.fields)
    if (root.stores_target() || !program.bits_payload(root.type))
      implementation_gap(root.source->at, "region roots outside direct bits storage");
  return roots;
}

inline std::vector<size_t> region_initializers(const CheckedSchema &roots, const Expression &expr) {
  for (const auto &item : expr.initializer_items())
    if (!item->initializer_field_label())
      implementation_gap(item->at, "region initialization outside the direct root initializer form");
  return product_initializers(roots, expr);
}
inline Type owned_region_type(const Type &schema) {
  Type type{Type::region};
  type.owned = true;
  type.target = std::make_shared<Type>(schema);
  return type;
}

} // namespace cod::language
