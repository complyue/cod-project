//===--- lang_c_provider.hh - Checked native C boundary ----------------===//
#pragma once

#include "lang_checked_program.hh"

#include <sstream>
#include <string>

namespace cod::language {

struct NativeProviderFiles {
  static constexpr auto header_name = "cod_providers.h";
  static constexpr auto stubs_name = "cod_providers.c";
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
  case Type::boolean: case Type::uint8: return "uint8_t";
  case Type::uint32: return "uint32_t";
  case Type::uint64: return "uint64_t";
  case Type::integer: return "int64_t";
  case Type::float32: return "float";
  case Type::float64: return "double";
  default: throw std::logic_error("unchecked scalar type in a generated C provider");
  }
}

inline std::string provider_borrow(const Type &handle) { return "cod_native_" + handle.name + "_borrow"; }
enum class NativeStatus : unsigned { ok, failure, boundary_error };
enum class NativeHereStep : unsigned { value, protected_completion, invalid };
inline std::string native_constructor_prefix(const Type &handle) { return "cod_native_" + handle.name; }
inline std::string native_construct_name(const Type &handle) { return native_constructor_prefix(handle) + "_construct"; }
inline std::string native_destroy_name(const Type &handle) { return native_constructor_prefix(handle) + "_destroy"; }
inline std::string native_entry_name(const Form &form) { return "cod_native_" + form.name + "_entry"; }
inline std::string native_entry_body_name(const Form &form) { return native_entry_name(form) + "_body"; }
inline std::string native_operation_name(const CheckedNativeOperation &operation) {
  return "cod_native_" + operation.contract.source->name;
}

inline std::string emit_c_view(const CheckedProgram &program, const Type &type, bool read_only, const std::string &name,
                               size_t index, const NativeRowLayouts &layouts, std::ostringstream &header,
                               std::ostringstream &stubs) {
  const auto &array = program.arrays.at(type.identity);
  const auto suffix = std::to_string(index);
  std::string element;
  if (array.element.kind == Type::product) {
    element = name + "_row" + suffix;
    const auto &fields = program.schemas.at(array.element.identity).fields;
    header << "typedef struct " << element << " {\n";
    for (const auto &field : fields) header << "    " << provider_scalar(field.type) << ' ' << field.source->name << ";\n";
    header << "} " << element << ";\n";
    const auto &layout = layouts.at(array.element.identity);
    stubs << "_Static_assert(sizeof(" << element << ") == " << layout.size << " && _Alignof(" << element << ") == "
          << layout.alignment << ", \"unexpected native row layout\");\n";
    for (size_t f = 0; f < fields.size(); ++f)
      stubs << "_Static_assert(offsetof(" << element << ", " << fields[f].source->name << ") == "
            << layout.offsets.at(f) << ", \"unexpected native row offset\");\n";
  } else element = provider_scalar(array.element);
  const auto view = name + "_view" + suffix;
  header << "typedef struct " << view << " {\n    " << (read_only ? "const " : "") << element
         << " *base;\n    uint64_t count;\n} " << view << ";\n";
  stubs << "_Static_assert(sizeof(" << view << ") == 2 * sizeof(uint64_t) && offsetof(" << view
        << ", count) == sizeof(uint64_t), \"unexpected native view layout\");\n";
  return element;
}

inline std::string invalid_c_view(const std::string &base, const std::string &count, const std::string &element) {
  return count + " > (uint64_t)PTRDIFF_MAX / sizeof(*" + base + ") || (" + count +
         " && (!" + base + " || (uintptr_t)" + base + " % _Alignof(" + element + ")))";
}

inline void emit_c_here(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        std::ostringstream &header, std::ostringstream &stubs) {
  const auto name = native_operation_name(operation);
  const auto handle = provider_borrow(operation.here_handle());
  std::vector<std::string> views;
  for (size_t i = 0; i < operation.arrays().size(); ++i) {
    const auto &input = operation.arrays()[i];
    views.push_back(emit_c_view(program, input.type, input.read_only, name, i, layouts, header, stubs));
  }
  const auto arrays = name + "_arrays", callback = name + "_callback", state = name + "_state";
  header << "typedef struct " << arrays << " {\n";
  for (size_t i = 0; i < views.size(); ++i) header << "    " << name << "_view" << i << " array" << i << ";\n";
  header << "} " << arrays << ";\n"
         << "typedef cod_native_here_step (*" << callback << ")(void *context, const " << arrays << " *views, uint8_t *feedback);\n";
  for (size_t i = 0; i < views.size(); ++i)
    stubs << "_Static_assert(offsetof(" << arrays << ", array" << i << ") == " << i
          << " * 2 * sizeof(uint64_t), \"unexpected native array offset\");\n";
  const auto boundary = "cod_native_status " + name + "_call(const " + handle + " *handle, " + callback +
      " body, void *context, cod_native_here_step *completion, uint8_t *value)";
  header << boundary << ";\n"
         << "cod_native_status " << name << "_impl(" << handle << " handle, " << callback << " run, void *loan, uint8_t *value);\n"
         << "int " << name << "_validate_impl(" << handle << " handle, const " << arrays << " *views);\n\n";

  stubs << "typedef struct " << state << " {\n    " << handle << " handle;\n    " << callback
        << " body;\n    void *context;\n    cod_native_here_step completion;\n} " << state << ";\n\n"
        << "static cod_native_here_step " << name << "_run(void *context, const " << arrays << " *views, uint8_t *feedback) {\n"
        << "    " << state << " *state = context;\n"
        << "    if (state->completion != COD_NATIVE_HERE_VALUE) return state->completion;\n"
        << "    if (!views || !feedback) return state->completion = COD_NATIVE_HERE_INVALID;\n";
  for (size_t i = 0; i < views.size(); ++i) {
    const auto view = "views->array" + std::to_string(i);
    stubs << "    if (" << invalid_c_view(view + ".base", view + ".count", views[i])
          << ") return state->completion = COD_NATIVE_HERE_INVALID;\n";
  }
  stubs << "    if (!" << name << "_validate_impl(state->handle, views)) return state->completion = COD_NATIVE_HERE_INVALID;\n"
        << "    return state->completion = state->body(state->context, views, feedback);\n}\n\n"
        << boundary << " {\n"
        << "    if (!handle || !body || !context || !completion || !value || handle->payload != handle->owner)\n"
        << "        return COD_NATIVE_BOUNDARY_ERROR;\n"
        << "    " << state << " state = {*handle, body, context, COD_NATIVE_HERE_VALUE};\n"
        << "    uint8_t result = 0;\n"
        << "    const cod_native_status status = " << name << "_impl(*handle, " << name << "_run, &state, &result);\n"
        << "    *completion = state.completion;\n"
        << "    if (state.completion == COD_NATIVE_HERE_VALUE && status == COD_NATIVE_OK) *value = result;\n"
        << "    return state.completion == COD_NATIVE_HERE_INVALID ? COD_NATIVE_BOUNDARY_ERROR : status;\n}\n\n";
}

inline void emit_c_call(const CheckedProgram &program, const CheckedNativeOperation &operation, const NativeRowLayouts &layouts,
                        std::ostringstream &header, std::ostringstream &stubs) {
  const auto name = native_operation_name(operation);
  std::ostringstream boundary, implementation, arguments, checks;
  boundary << "cod_native_status " << name << "_call(";
  implementation << "cod_native_status " << name << "_impl(";
  std::map<size_t, std::string> first_handle;
  for (size_t i = 0; i < operation.inputs.size(); ++i) {
    const auto &input = operation.inputs[i];
    const auto suffix = std::to_string(i);
    const auto arg = "arg" + suffix;
    std::string type;
    if (input.type.kind == Type::array) {
      const auto element = emit_c_view(program, input.type, true, name, i, layouts, header, stubs);
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
    if (input.owner) boundary << "const " << type << " *" << arg << ", ";
    else boundary << type << ' ' << arg << ", ";
    implementation << type << ' ' << arg << ", ";
    if (input.owner) arguments << '*';
    arguments << arg << ", ";
  }
  boundary << "uint8_t *value)";
  implementation << "uint8_t *value)";
  header << boundary.str() << ";\n" << implementation.str() << ";\n\n";
  stubs << boundary.str() << " {\n"
        << "    if (!value) return COD_NATIVE_BOUNDARY_ERROR;\n" << checks.str()
        << "    uint8_t result = 0;\n"
        << "    const cod_native_status status = " << name << "_impl(" << arguments.str() << "&result);\n"
        << "    if (status != COD_NATIVE_OK) return COD_NATIVE_BOUNDARY_ERROR;\n"
        << "    *value = result;\n"
        << "    return COD_NATIVE_OK;\n}\n\n";
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
  header << R"C(#ifndef COD_GENERATED_PROVIDERS_H
#define COD_GENERATED_PROVIDERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t cod_native_status;
enum {
)C" << "    COD_NATIVE_OK = " << static_cast<unsigned>(NativeStatus::ok)
     << ",\n    COD_NATIVE_FAILURE = " << static_cast<unsigned>(NativeStatus::failure)
     << ",\n    COD_NATIVE_BOUNDARY_ERROR = " << static_cast<unsigned>(NativeStatus::boundary_error) << "\n};\n\n";
  stubs << "#include \"" << NativeProviderFiles::header_name << "\"\n#include <stddef.h>\n\n";
  if (std::ranges::any_of(program.native_operations, &CheckedNativeOperation::has_here))
    header << "typedef uint32_t cod_native_here_step;\nenum {\n"
           << "    COD_NATIVE_HERE_VALUE = " << static_cast<unsigned>(NativeHereStep::value) << ",\n"
           << "    COD_NATIVE_HERE_PROTECTED = " << static_cast<unsigned>(NativeHereStep::protected_completion) << ",\n"
           << "    COD_NATIVE_HERE_INVALID = " << static_cast<unsigned>(NativeHereStep::invalid) << "\n};\n\n";

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
      const auto borrow = provider_borrow(input.type);
      header << "typedef struct " << borrow << " {\n    uint64_t payload;\n    uint64_t owner;\n} " << borrow << ";\n\n";
      stubs << "_Static_assert(sizeof(" << borrow << ") == 2 * sizeof(uint64_t) &&\n"
            << "               offsetof(" << borrow << ", owner) == sizeof(uint64_t), \"invalid " << borrow << " layout\");\n";
    }
  };
  for (const auto &constructor : program.native_constructors) emit_borrows(constructor.inputs);
  for (const auto &entry : program.native_entries) emit_borrows(entry.inputs);
  for (const auto &native : program.native_operations)
    emit_borrows(native.inputs);
  if (!emitted_borrows.empty()) stubs << '\n';

  for (const auto &constructor : program.native_constructors) {
    const auto failure = native_constructor_prefix(constructor.handle) + "_failure";
    const auto construct_name = native_construct_name(constructor.handle);
    const auto destroy_name = native_destroy_name(constructor.handle);
    const CheckedSchema *schema = nullptr;
    if (constructor.failure) {
      schema = &program.schemas.at(*program.exception_types.at(constructor.failure->identity).schema);
      header << "/* Payload of the checked " << constructor.failure->name << " exception. */\n"
             << "typedef struct " << failure << " {\n";
      for (const auto &field : schema->fields)
        header << "    " << provider_scalar(field.type) << ' ' << field.source->name << ";\n";
      if (schema->fields.empty()) header << "    uint8_t reserved;\n";
      header << "} " << failure << ";\n\n";
    }

    const auto construct = [&](bool implementation) {
      std::ostringstream declaration;
      declaration << "cod_native_status " << construct_name << (implementation ? "_impl" : "") << '(';
      for (size_t i = 0; i < constructor.inputs.size(); ++i) {
        const auto &input = constructor.inputs[i];
        if (input.owner) {
          if (!implementation) declaration << "const ";
          declaration << provider_borrow(input.type) << (implementation ? " " : " *");
        } else declaration << provider_scalar(input.type) << ' ';
        declaration << "arg" << i << ", ";
      }
      declaration << "uint64_t *resource";
      if (schema) declaration << ", " << failure << " *error";
      declaration << ')';
      return std::move(declaration).str();
    };
    const auto destroy = "void " + destroy_name + "(uint64_t resource)";
    const auto destroy_impl = "void " + destroy_name + "_impl(uint64_t resource)";
    const auto boundary = construct(false);
    header << boundary << ";\n" << destroy << ";\n"
           << construct(true) << ";\n" << destroy_impl << ";\n\n";

    stubs << boundary << " {\n    if (!resource";
    if (schema) stubs << " || !error";
    for (size_t i = 0; i < constructor.inputs.size(); ++i) {
      const auto &input = constructor.inputs[i];
      if (input.owner) stubs << " || !arg" << i;
      else if (input.type == Type::boolean) stubs << " || arg" << i << " > 1";
    }
    stubs << ") return COD_NATIVE_BOUNDARY_ERROR;\n";
    std::map<size_t, size_t> first_owner_input;
    for (size_t i = 0; i < constructor.inputs.size(); ++i) if (const auto owner = constructor.inputs[i].owner) {
      const auto [first, inserted] = first_owner_input.try_emplace(*owner, i);
      if (!inserted) stubs << "    if (arg" << first->second << "->owner != arg" << i << "->owner) return COD_NATIVE_BOUNDARY_ERROR;\n";
    }
    stubs << "    uint64_t acquired = 0;\n";
    if (schema) stubs << "    " << failure << " reported = {0};\n";
    stubs << "    const cod_native_status status = " << construct_name << "_impl(";
    for (size_t i = 0; i < constructor.inputs.size(); ++i)
      stubs << (constructor.inputs[i].owner ? "*" : "") << "arg" << i << ", ";
    stubs << "&acquired" << (schema ? ", &reported" : "") << ");\n"
          << "    if (status == COD_NATIVE_OK) {\n        *resource = acquired;\n        return status;\n    }\n";
    if (schema) {
      stubs << "    if (status == COD_NATIVE_FAILURE) {\n";
      for (const auto &field : schema->fields) if (field.type == Type::boolean)
        stubs << "        if (reported." << field.source->name << " > 1) return COD_NATIVE_BOUNDARY_ERROR;\n";
      stubs << "        *error = reported;\n        return status;\n    }\n";
    }
    stubs << "    return COD_NATIVE_BOUNDARY_ERROR;\n}\n\n"
          << destroy << " {\n    " << destroy_name << "_impl(resource);\n}\n\n";
  }
  for (const auto &entry : program.native_entries) emit_c_entry(entry, *program.methods.at(entry.method).source, header, stubs);
  for (const auto &native : program.native_operations) {
    if (native.has_here()) emit_c_here(program, native, layouts, header, stubs);
    else emit_c_call(program, native, layouts, header, stubs);
  }
  header << "#ifdef __cplusplus\n}\n#endif\n\n#endif\n";
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
