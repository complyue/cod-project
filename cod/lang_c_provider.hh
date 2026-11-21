//===--- lang_c_provider.hh - Checked native C boundary ----------------===//
#pragma once

#include "lang_checked_program.hh"
#include "native/provider_call.h"

#include <sstream>
#include <string>

namespace cod::language {

struct NativeProviderFiles {
  static constexpr auto header_name = COD_PROVIDER_HEADER_NAME;
  static constexpr auto stubs_name = COD_PROVIDER_STUBS_NAME;
  std::string header;
  std::string stubs;
};

struct NativeRowLayout {
  uint64_t size, alignment;
  std::vector<uint64_t> offsets;
};
using NativeRowLayouts = std::map<size_t, NativeRowLayout>;

inline const char *provider_scalar(const Type &type) {
  switch (type.kind) {
  case Type::boolean: case Type::uint8: return cod_provider_scalar_spelling(cod_provider_u8);
  case Type::uint32: return cod_provider_scalar_spelling(cod_provider_u32);
  case Type::uint64: return cod_provider_scalar_spelling(cod_provider_u64);
  case Type::integer: return cod_provider_scalar_spelling(cod_provider_i64);
  case Type::float32: return cod_provider_scalar_spelling(cod_provider_f32);
  case Type::float64: return cod_provider_scalar_spelling(cod_provider_f64);
  default: throw std::logic_error("unchecked scalar type in a generated C provider");
  }
}

enum class NativeStatus : unsigned {
  ok = cod_provider_native_ok, failure = cod_provider_native_failure, boundary_error = cod_provider_native_boundary_error,
};
enum class NativeHereStep : unsigned {
  value = cod_provider_here_value, protected_completion = cod_provider_here_protected, invalid = cod_provider_here_invalid,
};
inline std::string native_constructor_prefix(const Type &handle) { return "cod_native_" + handle.name; }
inline std::string native_construct_name(const Type &handle) { return native_constructor_prefix(handle) + "_construct"; }
inline std::string native_destroy_name(const Type &handle) { return native_constructor_prefix(handle) + "_destroy"; }
inline std::string native_entry_name(const Form &form) { return "cod_native_" + form.name + "_entry"; }
inline std::string native_entry_body_name(const Form &form) { return native_entry_name(form) + "_body"; }
inline std::string native_operation_name(const CheckedNativeOperation &operation) {
  return "cod_native_" + operation.contract.source->name;
}

inline bool provider_stream_write(void *context, const char *text, size_t length) {
  return static_cast<bool>(static_cast<std::ostringstream *>(context)->write(text, length));
}

inline cod_provider_view emit_c_view(const CheckedProgram &program, const Type &type, bool read_only,
                                     const std::string &name, size_t index, const NativeRowLayouts &layouts,
                                     cod_provider_text &header, cod_provider_text &stubs) {
  const auto &element = program.arrays.at(type.identity).element;
  const cod_provider_view view{element.kind == Type::product ? nullptr : provider_scalar(element), read_only};
  std::vector<cod_provider_field> fields;
  cod_provider_row row{};
  if (!view.scalar) {
    for (const auto &field : program.schemas.at(element.identity).fields)
      fields.push_back({provider_scalar(field.type), field.source->name.c_str(), false});
    const auto &layout = layouts.at(element.identity);
    row = {fields.data(), layout.offsets.data(), fields.size(), layout.size, layout.alignment};
  }
  cod_provider_write_view(&header, &stubs, name.c_str(), index, view, &row);
  return view;
}

inline void emit_c_here(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        cod_provider_text &header, cod_provider_text &stubs) {
  const auto &name = operation.contract.source->name;
  std::vector<cod_provider_view> views;
  for (size_t i = 0; i < operation.arrays().size(); ++i) {
    const auto &input = operation.arrays()[i];
    views.push_back(emit_c_view(program, input.type, input.read_only, name, i, layouts, header, stubs));
  }
  const auto &handle = operation.here_handle();
  cod_provider_write_here(&header, &stubs, name.c_str(), handle.name.c_str(), views.data(), views.size());
}

inline cod_provider_argument describe_c_argument(const CheckedNativeInput &input) {
  if (input.type.kind == Type::array) return {nullptr, cod_provider_argument_view, false, input.owner.value_or(0)};
  return {input.owner ? input.type.name.c_str() : provider_scalar(input.type),
          input.owner ? cod_provider_argument_handle : cod_provider_argument_scalar,
          input.type == Type::boolean, input.owner.value_or(0)};
}

inline void emit_c_call(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        cod_provider_text &header, cod_provider_text &stubs) {
  const auto &name = operation.contract.source->name;
  std::vector<cod_provider_argument> inputs;
  std::vector<cod_provider_view> views(operation.inputs.size());
  for (size_t i = 0; i < operation.inputs.size(); ++i) {
    const auto &input = operation.inputs[i];
    inputs.push_back(describe_c_argument(input));
    if (input.type.kind == Type::array)
      views[i] = emit_c_view(program, input.type, true, name, i, layouts, header, stubs);
  }
  const auto &result = *operation.contract.result.type;
  const cod_provider_callable shape{
      name.c_str(), inputs.data(), inputs.size(), result == Type::void_type ? nullptr : provider_scalar(result), 0};
  cod_provider_write_call(&header, &stubs, &shape, views.data());
}

inline void emit_c_entry(const CheckedNativeEntry &entry, const Form &form,
                         cod_provider_text &header, cod_provider_text &stubs) {
  std::vector<cod_provider_argument> inputs;
  for (const auto &input : entry.inputs) inputs.push_back(describe_c_argument(input));
  const cod_provider_callable shape{form.name.c_str(), inputs.data(), inputs.size(),
      entry.result == Type::void_type ? nullptr : provider_scalar(entry.result), form.owner_parameters.size()};
  cod_provider_write_entry(&header, &stubs, &shape);
}

inline NativeProviderFiles emit_native_providers(const CheckedProgram &program, const NativeRowLayouts &layouts) {
  std::ostringstream header, stubs;
  cod_provider_text header_text{provider_stream_write, &header, false}, stubs_text{provider_stream_write, &stubs, false};
  cod_provider_begin(&header_text, &stubs_text);
  if (std::ranges::any_of(program.native_operations, &CheckedNativeOperation::has_here))
    cod_provider_here_preamble(&header_text);

  if (std::ranges::any_of(program.native_entries, [&](const auto &entry) {
        return !program.methods.at(entry.method).source->owner_parameters.empty();
      })) cod_provider_loan_preamble(&header_text);

  std::set<size_t> emitted_borrows;
  const auto emit_borrows = [&](const std::vector<CheckedNativeInput> &inputs) {
    for (const auto &input : inputs) {
      if (input.type.kind != Type::c_type_marker || !emitted_borrows.insert(input.type.identity).second) continue;
      cod_provider_borrow(&header_text, &stubs_text, input.type.name.c_str());
    }
  };
  for (const auto &constructor : program.native_constructors) {
    if (constructor.parent) emit_borrows({{*constructor.parent, {}}});
    emit_borrows(constructor.inputs);
  }
  for (const auto &entry : program.native_entries) emit_borrows(entry.inputs);
  for (const auto &native : program.native_operations)
    emit_borrows(native.inputs);
  if (!emitted_borrows.empty()) stubs << '\n';

  for (const auto &constructor : program.native_constructors) {
    std::vector<cod_provider_argument> inputs;
    for (const auto &input : constructor.inputs) inputs.push_back(describe_c_argument(input));
    std::vector<cod_provider_field> fields;
    if (constructor.failure) {
      const auto &schema = program.schemas.at(*program.exception_types.at(constructor.failure->identity).schema);
      for (const auto &field : schema.fields)
        fields.push_back({provider_scalar(field.type), field.source->name.c_str(), field.type == Type::boolean});
    }
    const cod_provider_constructor shape{
        constructor.handle.name.c_str(), constructor.parent ? constructor.parent->name.c_str() : nullptr,
        inputs.data(), inputs.size(), constructor.failure ? constructor.failure->name.c_str() : nullptr,
        fields.data(), fields.size()};
    cod_provider_write_constructor(&header_text, &stubs_text, &shape);
  }
  for (const auto &entry : program.native_entries) emit_c_entry(entry, *program.methods.at(entry.method).source, header_text, stubs_text);
  for (const auto &native : program.native_operations) {
    if (native.has_here()) emit_c_here(program, native, layouts, header_text, stubs_text);
    else emit_c_call(program, native, layouts, header_text, stubs_text);
  }
  cod_provider_end(&header_text);
  if (header_text.failed || stubs_text.failed || !header || !stubs)
    throw std::runtime_error("could not format generated C providers");
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
