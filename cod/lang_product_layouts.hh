//===--- lang_product_layouts.hh - Concrete generic product layouts ------===//
#pragma once

#include "lang_checked_program.hh"

namespace cod::language::seed {

// The completed schema table is append-only. A demand either publishes all of
// its new layouts or leaves the table untouched.
class ProductLayouts {
  using Key = std::pair<size_t, std::vector<std::pair<Type::Kind, size_t>>>;
  CheckedProgram &program_;
  std::map<Key, size_t> completed_;
  static constexpr size_t expansion_limit = 128;

  struct Demand {
    std::map<Key, size_t> identities;
    std::deque<CheckedSchema> schemas;
    std::set<size_t> active_templates;
  };

  const CheckedSchema &schema(Demand &demand, size_t id) const {
    if (id < program_.schemas.size()) return program_.schemas.at(id);
    return demand.schemas.at(id - program_.schemas.size());
  }

  bool inline_bits(const Type &type, Demand &demand, const Location &at) const {
    if (type.kind != Type::product) return program_.bits_payload(type);
    const auto &layout = schema(demand, type.identity);
    if (!layout.layout_checked) fail(at, "recursive inline generic product layout");
    return layout.bits;
  }

  Type finish_type(Type type, std::span<const Type> arguments, Demand &demand, const Location &at, size_t depth) {
    if (type.kind == Type::parameter) return arguments[type.identity];
    if (type.kind != Type::product_application) return type;
    if (depth == expansion_limit) implementation_gap(at, "growing generic product layout");
    const auto &declaration = program_.product_templates.at(type.identity);
    if (type.parameters.size() != declaration.source->type_parameters.size())
      fail(at, "wrong number of type arguments for '" + type.name + "'");

    Key key{type.identity, {}};
    key.second.reserve(type.parameters.size());
    for (auto &argument : type.parameters) {
      argument = finish_type(std::move(argument), arguments, demand, at, depth + 1);
      if (!scalar_type(argument) && argument.kind != Type::product && argument.kind != Type::sum &&
          argument.kind != Type::array)
        implementation_gap(at, "generic product needs a concrete scalar or nominal data type argument");
      key.second.emplace_back(argument.kind, argument.identity);
    }
    const auto name = type_name(type);
    if (const auto found = completed_.find(key); found != completed_.end())
      return {Type::product, name, found->second};
    if (const auto found = demand.identities.find(key); found != demand.identities.end())
      return {Type::product, name, found->second};

    if (!demand.active_templates.insert(type.identity).second)
      implementation_gap(at, "recursive generic product changes its type arguments");
    const auto id = program_.schemas.size() + demand.schemas.size();
    demand.identities.emplace(std::move(key), id);
    auto &layout = demand.schemas.emplace_back(CheckedSchema{declaration.source, {}, true});
    layout.application = type;
    layout.fields.reserve(declaration.fields.size());
    for (const auto &field : declaration.fields) {
      auto member = finish_type(field.type, type.parameters, demand, field.source->at, depth + 1);
      if (field.storage == PlaceKind::value) {
        if (!inline_bits(member, demand, field.source->at))
          implementation_gap(field.source->at, "generic inline field requires a finite bits payload");
      } else {
        const bool product = member.kind == Type::product && !schema(demand, member.identity).lexical_builder;
        const bool bits_sum = member.kind == Type::sum && program_.bits_payload(member);
        if (!product && !bits_sum && member.kind != Type::array && !scalar_type(member))
          implementation_gap(field.source->at, "generic reference field requires an admitted concrete target");
        layout.bits = false;
      }
      layout.fields.push_back({field.source, std::move(member), field.storage});
    }
    layout.layout_checked = true;
    demand.active_templates.erase(type.identity);
    return {Type::product, name, id};
  }

public:
  explicit ProductLayouts(CheckedProgram &program) : program_(program) {}

  Type resolve(const Type &type, const Location &at) {
    if (type.kind != Type::product_application) return type;
    Demand demand;
    auto result = finish_type(type, {}, demand, at, 0);
    for (auto &schema : demand.schemas) program_.schemas.push_back(std::move(schema));
    completed_.merge(demand.identities);
    return result;
  }
};

} // namespace cod::language::seed
