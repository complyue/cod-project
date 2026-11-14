//===--- lang_modules.hh - Source module discovery ------------------===//
#pragma once

#include "lang_syntax.hh"

#include <fstream>
#include <iterator>
#include <map>
#include <set>

namespace cod::language {

struct SourceModule {
  std::filesystem::path path;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, size_t> dependencies;
};

struct SourceProgram {
  std::vector<SourceModule> modules;
  std::vector<size_t> construction_order;
};

class ModuleLoader {
  SourceProgram program_;
  std::map<std::filesystem::path, size_t> paths_;
  std::set<size_t> scanning_;

  size_t scan(const std::filesystem::path &path, const Location &at) {
    std::error_code error;
    const auto resolved = std::filesystem::weakly_canonical(path, error);
    if (error) fail(at, "cannot resolve CoD source '" + path.string() + "'");
    if (const auto found = paths_.find(resolved); found != paths_.end()) {
      if (scanning_.contains(found->second))
        fail(at, "micro-dispatch implementation gap: cyclic module construction remains unproved; this is not a language restriction");
      return found->second;
    }
    std::ifstream input(resolved, std::ios::binary);
    if (!input) fail(at, "cannot read CoD source '" + resolved.string() + "'");
    const std::string source{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    if (input.bad()) fail(at, "failed while reading CoD source '" + resolved.string() + "'");
    const auto id = program_.modules.size();
    paths_.emplace(resolved, id);
    program_.modules.push_back({resolved, Parser(source, resolved.string()).parse(), {}});
    scanning_.insert(id);
    // Source forms have stable addresses while recursive scans grow the catalog.
    std::vector<const Form *> selections;
    for (const auto &form : program_.modules[id].forms) {
      if (!form->path.empty() && !form->path.starts_with("std:"))
        selections.push_back(form.get());
    }
    for (const auto *form : selections) {
      const auto dependency = scan(resolved.parent_path() / form->path, form->at);
      program_.modules[id].dependencies.emplace(form, dependency);
    }
    scanning_.erase(id);
    program_.construction_order.push_back(id);
    return id;
  }

public:
  SourceProgram load(const std::filesystem::path &entry) {
    scan(entry, {entry.string()});
    return std::move(program_);
  }
};

} // namespace cod::language
