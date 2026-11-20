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
inline std::string native_constructor_prefix(const Type &handle) { return "cod_native_" + handle.name; }
inline std::string native_construct_name(const Type &handle) { return native_constructor_prefix(handle) + "_construct"; }
inline std::string native_destroy_name(const Type &handle) { return native_constructor_prefix(handle) + "_destroy"; }
inline std::string native_entry_name(const Form &form) { return "cod_native_" + form.name + "_entry"; }
inline std::string native_entry_body_name(const Form &form) { return native_entry_name(form) + "_body"; }

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

inline NativeProviderFiles emit_native_providers(const CheckedProgram &program) {
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
      if (!input.owner || !emitted_borrows.insert(input.type.identity).second) continue;
      const auto borrow = provider_borrow(input.type);
      header << "typedef struct " << borrow << " {\n    uint64_t payload;\n    uint64_t owner;\n} " << borrow << ";\n\n";
      stubs << "_Static_assert(sizeof(" << borrow << ") == 2 * sizeof(uint64_t) &&\n"
            << "               offsetof(" << borrow << ", owner) == sizeof(uint64_t), \"invalid " << borrow << " layout\");\n";
    }
  };
  for (const auto &constructor : program.native_constructors) emit_borrows(constructor.inputs);
  for (const auto &entry : program.native_entries) emit_borrows(entry.inputs);
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
  header << "#ifdef __cplusplus\n}\n#endif\n\n#endif\n";
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
