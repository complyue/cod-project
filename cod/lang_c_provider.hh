//===--- lang_c_provider.hh - Checked native C constructor boundary ----===//
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
    COD_NATIVE_OK = 0,
    COD_NATIVE_FAILURE = 1,
    COD_NATIVE_BOUNDARY_ERROR = 2
};

)C";
  stubs << "#include \"" << NativeProviderFiles::header_name << "\"\n#include <stddef.h>\n\n";

  std::set<size_t> emitted_borrows;
  for (const auto &constructor : program.native_constructors) for (const auto &input : constructor.inputs) {
    if (!input.owner || !emitted_borrows.insert(input.type.identity).second) continue;
    const auto borrow = provider_borrow(input.type);
    header << "typedef struct " << borrow << " {\n    uint64_t payload;\n    uint64_t owner;\n} " << borrow << ";\n\n";
    stubs << "_Static_assert(sizeof(" << borrow << ") == 2 * sizeof(uint64_t) &&\n"
          << "               offsetof(" << borrow << ", owner) == sizeof(uint64_t), \"invalid " << borrow << " layout\");\n";
  }
  if (!emitted_borrows.empty()) stubs << '\n';

  for (const auto &constructor : program.native_constructors) {
    const auto prefix = "cod_native_" + constructor.handle.name;
    const auto failure = prefix + "_failure";
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
      declaration << "cod_native_status " << prefix << "_construct" << (implementation ? "_impl" : "") << '(';
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
    const auto destroy = "void " + prefix + "_destroy(uint64_t resource)";
    const auto destroy_impl = "void " + prefix + "_destroy_impl(uint64_t resource)";
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
    stubs << "    const cod_native_status status = " << prefix << "_construct_impl(";
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
          << destroy << " {\n    " << prefix << "_destroy_impl(resource);\n}\n\n";
  }
  header << "#ifdef __cplusplus\n}\n#endif\n\n#endif\n";
  return {std::move(header).str(), std::move(stubs).str()};
}

} // namespace cod::language
