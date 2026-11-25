//===--- clang_driver.hh - Invoking the seed's Clang driver ----------===//
#pragma once

#include "llvm/Support/Program.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace cod::compiler_driver {

inline bool link_bitcode(const std::vector<std::filesystem::path> &inputs,
                         const std::filesystem::path &output_executable,
                         const std::vector<std::string> &linker_args, const std::string &driver) {
  auto executable = llvm::sys::findProgramByName(driver);
  if (!executable) {
    std::cerr << "Failed to find compiler '" << driver << "': " << executable.getError().message() << '\n';
    return false;
  }

  std::vector<std::string> arguments{*executable};
  for (const auto &input : inputs) {
    arguments.push_back(input.string());
  }
  arguments.insert(arguments.end(), linker_args.begin(), linker_args.end());
  arguments.push_back("-o");
  arguments.push_back(output_executable.string());

  std::vector<llvm::StringRef> argument_refs;
  argument_refs.reserve(arguments.size());
  for (const auto &argument : arguments) {
    argument_refs.emplace_back(argument);
  }

  std::string error_message;
  const int result = llvm::sys::ExecuteAndWait(*executable, argument_refs, std::nullopt, {}, 0, 0, &error_message);
  if (!error_message.empty()) {
    std::cerr << "Failed to execute compiler '" << *executable << "': " << error_message << '\n';
  }
  return result == 0;
}

} // namespace cod::compiler_driver
