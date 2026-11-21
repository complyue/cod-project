//===--- lang_c_provider.hh - Checked native C boundary ----------------===//
#pragma once

#include "lang_checked_program.hh"
#include "native/provider_here.h"

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

inline std::string provider_borrow(const Type &handle) { return "cod_native_" + handle.name + "_borrow"; }
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

inline std::string invalid_c_view(const std::string &base, const std::string &count, const std::string &element) {
  return count + " > (uint64_t)PTRDIFF_MAX / sizeof(*" + base + ") || (" + count +
         " && (!" + base + " || (uintptr_t)" + base + " % _Alignof(" + element + ")))";
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

inline void emit_c_call(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        std::ostringstream &header, std::ostringstream &stubs) {
  const auto name = native_operation_name(operation);
  const bool returns_value = operation.contract.result.type != Type::void_type;
  std::ostringstream boundary, implementation, arguments, checks;
  boundary << "cod_native_status " << name << "_call(";
  implementation << "cod_native_status " << name << "_impl(";
  std::map<size_t, std::string> first_handle;
  for (size_t i = 0; i < operation.inputs.size(); ++i) {
    const auto &input = operation.inputs[i];
    const auto suffix = std::to_string(i);
    const auto arg = "arg" + suffix;
    if (i) {
      boundary << ", ";
      implementation << ", ";
      arguments << ", ";
    }
    std::string type;
    if (input.type.kind == Type::array) {
      cod_provider_text header_text{provider_stream_write, &header, false}, stubs_text{provider_stream_write, &stubs, false};
      const auto view = emit_c_view(program, input.type, true, operation.contract.source->name, i, layouts, header_text, stubs_text);
      if (header_text.failed || stubs_text.failed) throw std::runtime_error("could not format a generated C view");
      const auto element = view.scalar ? std::string(view.scalar) : name + "_row" + suffix;
      type = name + "_view" + suffix;
      checks << "    if (!" << arg << " || " << invalid_c_view(arg + "->base", arg + "->count", element)
             << ") return COD_NATIVE_BOUNDARY_ERROR;\n";
    } else if (input.owner) {
      type = provider_borrow(input.type);
      checks << "    if (!" << arg;
      const auto [first, inserted] = first_handle.try_emplace(*input.owner, arg);
      if (!inserted) checks << " || " << arg << "->owner != " << first->second << "->owner";
      checks << ") return COD_NATIVE_BOUNDARY_ERROR;\n";
    } else {
      type = provider_scalar(input.type);
      if (input.type == Type::boolean)
        checks << "    if (" << arg << " > 1) return COD_NATIVE_BOUNDARY_ERROR;\n";
    }
    if (input.owner) boundary << "const " << type << " *" << arg;
    else boundary << type << ' ' << arg;
    implementation << type << ' ' << arg;
    if (input.owner) arguments << '*';
    arguments << arg;
  }
  if (returns_value) {
    const auto separator = operation.inputs.empty() ? "" : ", ";
    boundary << separator << "uint8_t *value";
    implementation << separator << "uint8_t *value";
    arguments << separator << "&result";
  } else if (operation.inputs.empty()) {
    boundary << "void";
    implementation << "void";
  }
  boundary << ')';
  implementation << ')';
  header << boundary.str() << ";\n" << implementation.str() << ";\n\n";
  stubs << boundary.str() << " {\n";
  if (returns_value) stubs << "    if (!value) return COD_NATIVE_BOUNDARY_ERROR;\n";
  stubs << checks.str();
  if (returns_value) stubs << "    uint8_t result = 0;\n";
  stubs << "    const cod_native_status status = " << name << "_impl(" << arguments.str() << ");\n"
        << "    if (status != COD_NATIVE_OK) return COD_NATIVE_BOUNDARY_ERROR;\n";
  if (returns_value) stubs << "    *value = result;\n";
  stubs << "    return COD_NATIVE_OK;\n}\n\n";
}

inline void emit_c_entry(const CheckedNativeEntry &entry, const Form &form,
                         std::ostringstream &header, std::ostringstream &stubs) {
  const auto signature = [&](bool body) {
    std::ostringstream text;
    text << (body ? "void " + native_entry_body_name(form) : "cod_native_status " + native_entry_name(form)) << '(';
    bool comma = false;
    const auto argument = [&](const std::string &type, const std::string &name) {
      if (comma) text << ", ";
      text << type << ' ' << name;
      comma = true;
    };
    for (size_t i = 0; i < entry.inputs.size(); ++i) {
      const auto &input = entry.inputs[i];
      argument(input.owner ? "const " + provider_borrow(input.type) + " *" : provider_scalar(input.type),
               "arg" + std::to_string(i));
    }
    for (size_t i = 0; i < form.owner_parameters.size(); ++i)
      argument(body ? "const void *" : "const cod_native_loan *", "loan" + std::to_string(i));
    if (entry.result != Type::void_type) argument(std::string(provider_scalar(entry.result)) + " *", "value");
    if (!comma) text << "void";
    text << ')';
    return std::move(text).str();
  };
  const auto boundary = signature(false);
  header << boundary << ";\n\n";
  stubs << "extern " << signature(true) << ";\n\n" << boundary << " {\n";
  if (entry.result != Type::void_type) stubs << "    if (!value) return COD_NATIVE_BOUNDARY_ERROR;\n";
  for (size_t i = 0; i < form.owner_parameters.size(); ++i)
    stubs << "    if (!loan" << i << " || !loan" << i << "->activation) return COD_NATIVE_BOUNDARY_ERROR;\n";
  for (size_t i = 0; i < entry.inputs.size(); ++i) {
    const auto &input = entry.inputs[i];
    if (input.owner)
      stubs << "    if (!arg" << i << " || arg" << i << "->owner != loan" << *input.owner << "->owner) return COD_NATIVE_BOUNDARY_ERROR;\n";
    else if (input.type == Type::boolean)
      stubs << "    if (arg" << i << " > 1) return COD_NATIVE_BOUNDARY_ERROR;\n";
  }
  stubs << "    " << native_entry_body_name(form) << '(';
  bool comma = false;
  const auto pass = [&](const std::string &value) {
    if (comma) stubs << ", ";
    stubs << value;
    comma = true;
  };
  for (size_t i = 0; i < entry.inputs.size(); ++i) pass("arg" + std::to_string(i));
  for (size_t i = 0; i < form.owner_parameters.size(); ++i) pass("loan" + std::to_string(i) + "->activation");
  if (entry.result != Type::void_type) pass("value");
  stubs << ");\n    return COD_NATIVE_OK;\n}\n\n";
}

inline NativeProviderFiles emit_native_providers(const CheckedProgram &program, const NativeRowLayouts &layouts) {
  std::ostringstream header, stubs;
  cod_provider_text header_text{provider_stream_write, &header, false}, stubs_text{provider_stream_write, &stubs, false};
  cod_provider_begin(&header_text, &stubs_text);
  if (std::ranges::any_of(program.native_operations, &CheckedNativeOperation::has_here))
    cod_provider_here_preamble(&header_text);

  if (std::ranges::any_of(program.native_entries, [&](const auto &entry) {
        return !program.methods.at(entry.method).source->owner_parameters.empty();
      })) header << R"C(/* The supplying native activation keeps the lent backing live through the call. */
typedef struct cod_native_loan {
    const void *activation;
    uint64_t owner;
} cod_native_loan;

)C";

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
    for (const auto &input : constructor.inputs) {
      inputs.push_back({input.owner ? input.type.name.c_str() : provider_scalar(input.type),
                        input.owner.has_value(), input.type == Type::boolean, input.owner.value_or(0)});
    }
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
  for (const auto &entry : program.native_entries) emit_c_entry(entry, *program.methods.at(entry.method).source, header, stubs);
  for (const auto &native : program.native_operations) {
    if (native.has_here()) emit_c_here(program, native, layouts, header_text, stubs_text);
    else emit_c_call(program, native, layouts, header, stubs);
  }
  cod_provider_end(&header_text);
  if (header_text.failed || stubs_text.failed || !header || !stubs)
    throw std::runtime_error("could not format generated C providers");
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
