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
  struct Pending { size_t module, next_form = 0; };
  std::vector<Pending> pending_;

  size_t discover(const std::filesystem::path &path, const Location &at) {
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
    pending_.push_back({id});
    return id;
  }

public:
  SourceProgram load(const std::filesystem::path &entry) {
    discover(entry, {entry.string()});
    while (!pending_.empty()) {
      auto &pending = pending_.back();
      const auto id = pending.module;
      if (pending.next_form == program_.modules[id].forms.size()) {
        scanning_.erase(id);
        program_.construction_order.push_back(id);
        pending_.pop_back();
        continue;
      }
      const auto *form = program_.modules[id].forms[pending.next_form++].get();
      if (form->path.empty() || form->path.starts_with("std:")) continue;
      // Forms retain their addresses when discovery grows either work list.
      const auto dependency = discover(program_.modules[id].path.parent_path() / form->path, form->at);
      program_.modules[id].dependencies.emplace(form, dependency);
    }
    return std::move(program_);
  }
};

} // namespace cod::language
