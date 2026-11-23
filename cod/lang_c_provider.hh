//===--- lang_c_provider.hh - Checked native C boundary ----------------===//
#pragma once

#include "lang_checked_program.hh"
#include "native/provider_call.h"
#include "native/provider_enum.h"

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

using NativeEnumNames = std::map<size_t, std::unique_ptr<char, decltype(&std::free)>>;

inline cod_provider_scalar enum_backing(const Type &type) {
  switch (type.kind) {
  case Type::uint8: return cod_provider_u8;
  case Type::uint32: return cod_provider_u32;
  case Type::uint64: return cod_provider_u64;
  case Type::integer: return cod_provider_i64;
  default: throw std::logic_error("unchecked enum backing in a generated C provider");
  }
}

inline NativeEnumNames emit_c_enums(const CheckedProgram &program, const std::string &entry_path,
                                   cod_provider_text &header, cod_provider_text &stubs) {
  NativeEnumNames names;
  const auto emit = [&](const Type &type) {
    if (type.kind != Type::enumeration || names.contains(type.identity)) return;
    const auto &domain = program.enums.at(type.identity);
    const auto path = std::filesystem::path(domain.source->at.filename()).generic_string();
    std::unique_ptr<char, decltype(&std::free)> name{cod_provider_enum_name(entry_path.c_str(), path.c_str(),
        domain.source->name.c_str()), std::free};
    if (!name) throw std::bad_alloc{};
    std::vector<cod_provider_enum_member> members;
    for (size_t i = 0; i < domain.members.size(); ++i) {
      const auto &member = domain.members[i];
      members.push_back({domain.source->enum_members[i].name.text.c_str(), member.code, member.canonical == i});
    }
    cod_provider_write_enum(&header, &stubs, name.get(), enum_backing(domain.backing), members.data(), members.size());
    names.emplace(type.identity, std::move(name));
  };
  for (const auto &constructor : program.native_constructors)
    for (const auto &input : constructor.inputs) emit(input.type);
  for (const auto &entry : program.native_entries) {
    for (const auto &input : entry.inputs) emit(input.type);
    emit(entry.result);
  }
  for (const auto &operation : program.native_operations) {
    for (const auto &input : operation.inputs) emit(input.type);
    emit(*operation.contract.result.type);
  }
  return names;
}

inline const char *provider_value_type(const Type &type, const NativeEnumNames &enums) {
  if (type == Type::void_type) return nullptr;
  return type.kind == Type::enumeration ? enums.at(type.identity).get() : provider_scalar(type);
}

inline const char *provider_result_type(const Type &type, const NativeEnumNames &enums) {
  return type == Type::boolean ? "COD_BOOL" : provider_value_type(type, enums);
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
                        const NativeEnumNames &enums, cod_provider_text &header, cod_provider_text &stubs) {
  const auto &name = operation.contract.source->name;
  std::vector<cod_provider_view> views;
  for (size_t i = 0; i < operation.arrays().size(); ++i) {
    const auto &input = operation.arrays()[i];
    views.push_back(emit_c_view(program, input.type, input.read_only, name, i, layouts, header, stubs));
  }
  const auto &handle = operation.here_handle();
  const auto &result = *operation.contract.result.type;
  cod_provider_write_here(&header, &stubs, name.c_str(), handle.name.c_str(), views.data(), views.size(),
                          result.kind == Type::enumeration ? provider_value_type(result, enums) : nullptr);
}

inline cod_provider_argument describe_c_argument(const CheckedNativeInput &input, const NativeEnumNames &enums) {
  if (input.type.kind == Type::array) return {nullptr, cod_provider_argument_view, false, input.owner.value_or(0), false};
  if (text_type(input.type)) return {nullptr, input.type == Type::c_string ? cod_provider_argument_c_string : cod_provider_argument_text,
                                    false, *input.owner, false};
  if (input.type.kind == Type::c_type_marker)
    return {input.type.name.c_str(), cod_provider_argument_handle, false, input.owner.value_or(SIZE_MAX), false};
  return {provider_value_type(input.type, enums), cod_provider_argument_scalar,
          input.type == Type::boolean, 0, input.type.kind == Type::enumeration};
}

inline void emit_c_call(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        const NativeEnumNames &enums, cod_provider_text &header, cod_provider_text &stubs) {
  const auto &name = operation.contract.source->name;
  std::vector<cod_provider_argument> inputs;
  std::vector<cod_provider_view> views(operation.inputs.size());
  for (size_t i = 0; i < operation.inputs.size(); ++i) {
    const auto &input = operation.inputs[i];
    inputs.push_back(describe_c_argument(input, enums));
    if (input.type.kind == Type::array)
      views[i] = emit_c_view(program, input.type, true, name, i, layouts, header, stubs);
  }
  const auto &result = *operation.contract.result.type;
  const cod_provider_callable shape{
      name.c_str(), inputs.data(), inputs.size(), provider_result_type(result, enums), 0,
      result.kind == Type::enumeration, result == Type::boolean};
  cod_provider_write_call(&header, &stubs, &shape, views.data());
}

inline void emit_c_entry(const CheckedNativeEntry &entry, const Form &form, const NativeEnumNames &enums,
                         cod_provider_text &header, cod_provider_text &stubs) {
  std::vector<cod_provider_argument> inputs;
  for (const auto &input : entry.inputs) inputs.push_back(describe_c_argument(input, enums));
  const cod_provider_callable shape{form.name.c_str(), inputs.data(), inputs.size(),
      provider_result_type(entry.result, enums), form.owner_parameters.size(),
      entry.result.kind == Type::enumeration, entry.result == Type::boolean};
  cod_provider_write_entry(&header, &stubs, &shape);
}

inline NativeProviderFiles emit_native_providers(const CheckedProgram &program, const NativeRowLayouts &layouts,
                                                const std::string &entry_path) {
  std::ostringstream header, stubs;
  cod_provider_text header_text{provider_stream_write, &header, false}, stubs_text{provider_stream_write, &stubs, false};
  cod_provider_begin(&header_text, &stubs_text);
  if (std::ranges::any_of(program.native_entries, [](const auto &entry) { return entry.result == Type::boolean; }) ||
      std::ranges::any_of(program.native_operations, [](const auto &operation) {
        return *operation.contract.result.type == Type::boolean;
      }))
    cod_provider_print(&header_text, "typedef uint8_t COD_BOOL;\n\n");
  if (std::ranges::any_of(program.native_operations, &CheckedNativeOperation::has_here))
    cod_provider_here_preamble(&header_text);

  if (std::ranges::any_of(program.native_entries, [&](const auto &entry) {
        return !program.methods.at(entry.method).source->owner_parameters.empty();
      })) cod_provider_loan_preamble(&header_text);
  const auto has_text = [](const auto &boundary) {
    return std::ranges::any_of(boundary.inputs, [](const auto &input) { return text_type(input.type); });
  };
  const auto publishes_text = [&](Type::Kind kind) {
    return std::ranges::any_of(program.native_operations, [&](const auto &operation) {
      return std::ranges::any_of(operation.inputs, [&](const auto &input) { return input.type.kind == kind; });
    });
  };
  cod_provider_text_preamble(&header_text, &stubs_text, std::ranges::any_of(program.native_entries, has_text),
                            publishes_text(Type::string), publishes_text(Type::c_string));
  const auto enums = emit_c_enums(program, entry_path, header_text, stubs_text);

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
    for (const auto &input : constructor.inputs) inputs.push_back(describe_c_argument(input, enums));
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
  for (const auto &entry : program.native_entries)
    emit_c_entry(entry, *program.methods.at(entry.method).source, enums, header_text, stubs_text);
  for (const auto &native : program.native_operations) {
    if (native.has_here()) emit_c_here(program, native, layouts, enums, header_text, stubs_text);
    else emit_c_call(program, native, layouts, enums, header_text, stubs_text);
  }
  cod_provider_end(&header_text);
  if (header_text.failed || stubs_text.failed || !header || !stubs)
    throw std::runtime_error("could not format generated C providers");
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
