//===--- lang_modules.hh - Source module discovery ------------------===//
#pragma once

#include "lang_diagnostics.hh"
#include "codp_module_inputs.hh"

#include <map>
#include <set>

namespace cod::language {

struct SourceModule {
  std::filesystem::path path;
  std::shared_ptr<const IndexedSource> source;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, size_t> dependencies;
};

struct SourceProgram {
  std::vector<SourceModule> modules;
  std::vector<size_t> construction_order;
};

struct ModuleSearchInputs {
  std::filesystem::path bundle;
  std::filesystem::path project;
};

class ModuleLoader {
  SourceProgram program_;
  ModuleSearchInputs search_;
  std::vector<WarningDiagnostic> &warnings_;
  std::optional<std::vector<cod::project::PackageSourceRoot>> packages_;
  std::map<std::filesystem::path, size_t> paths_;
  std::set<size_t> scanning_;
  struct Pending { size_t module, next_form = 0; };
  std::vector<Pending> pending_;

  static bool namespace_path(std::string_view path) {
    if (path.empty() || path.find_first_of("\\:") != std::string_view::npos ||
        path.find('\0') != std::string_view::npos) return false;
    while (true) {
      const auto slash = path.find('/');
      const auto segment = path.substr(0, slash);
      if (segment.empty() || segment == "." || segment == "..") return false;
      if (slash == std::string_view::npos) return true;
      path.remove_prefix(slash + 1);
    }
  }

  static bool claim(const std::filesystem::path &file, const Location &at) {
    std::error_code error;
    const auto status = std::filesystem::symlink_status(file, error);
    if (status.type() == std::filesystem::file_type::not_found) return false;
    if (error) fail(at, "cannot inspect CoD module '" + file.string() + "': " + error.message());
    return std::filesystem::exists(status);
  }

  const std::vector<cod::project::PackageSourceRoot> &packages(const Location &at) {
    if (!packages_) {
      try {
        const auto root = search_.project.empty()
            ? cod::project::find_source_project(program_.modules.front().path)
            : std::optional{search_.project};
        packages_ = root ? cod::project::checked_package_sources(*root)
                         : std::vector<cod::project::PackageSourceRoot>{};
      } catch (const std::exception &error) {
        fail(at, error.what());
      }
    }
    return *packages_;
  }

  std::filesystem::path resolve_dependency(const SourceModule &module, const Form &form) {
    const std::string_view requested = form.path;
    if (requested.starts_with("./")) {
      if (requested.size() == 2 || requested.find('\0') != std::string_view::npos)
        fail(form.at, "invalid source-relative CoD module path");
      return module.path.parent_path() / form.path;
    }
    if (!namespace_path(requested)) fail(form.at, "invalid package module path '" + form.path + "'");
    std::optional<std::filesystem::path> selected;
    for (const auto &package : packages(form.at)) {
      const auto path = package.directory / form.path;
      if (!claim(path, form.at)) continue;
      if (selected)
        fail(form.at, "ambiguous package module '" + form.path + "' claimed by '" +
            selected->string() + "' and '" + path.string() + "'");
      selected = path;
    }
    const auto bundled = search_.bundle / form.path;
    const bool bundled_exists = !search_.bundle.empty() && claim(bundled, form.at);
    if (selected) {
      if (bundled_exists)
        warnings_.push_back({Warning::package_shadow, form.at, "package module '" + form.path +
            "' selects '" + selected->string() + "' and shadows '" + bundled.string() + "'"});
      return *selected;
    }
    if (bundled_exists) return bundled;
    fail(form.at, "module '" + form.path + "' is absent from the project packages and active bundled library");
  }

  size_t discover(const std::filesystem::path &path, const Location &at) {
    std::error_code error;
    const auto resolved = std::filesystem::weakly_canonical(path, error);
    if (error) fail(at, "cannot resolve CoD source '" + path.string() + "'");
    if (const auto found = paths_.find(resolved); found != paths_.end()) {
      if (scanning_.contains(found->second))
        fail(at, "micro-dispatch implementation gap: cyclic module construction remains unproved; this is not a language restriction");
      return found->second;
    }
    std::shared_ptr<const IndexedSource> source;
    try {
      source = std::make_shared<IndexedSource>(resolved);
    } catch (const std::system_error &error) {
      fail(at, "cannot read CoD source '" + resolved.string() + "': " + error.code().message());
    }
    auto parsed = Parser(source).parse();
    if (!parsed.reports.empty()) {
      std::vector<const SyntaxDiagnostic *> ordered;
      ordered.reserve(parsed.reports.size());
      for (const auto &report : parsed.reports) ordered.push_back(&report);
      std::stable_sort(ordered.begin(), ordered.end(), [](const auto *lhs, const auto *rhs) {
        return lhs->at.span.first < rhs->at.span.first;
      });
      std::string message;
      for (const auto *report : ordered) {
        if (!message.empty()) message += '\n';
        message += location_text(report->at) + ": " + report->message;
      }
      throw std::runtime_error(std::move(message));
    }
    const auto id = program_.modules.size();
    paths_.emplace(resolved, id);
    program_.modules.push_back({resolved, source, std::move(parsed.forms), {}});
    scanning_.insert(id);
    pending_.push_back({id});
    return id;
  }

public:
  ModuleLoader(ModuleSearchInputs search, std::vector<WarningDiagnostic> &warnings)
      : search_(std::move(search)), warnings_(warnings) {}

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
      const auto path = resolve_dependency(program_.modules[id], *form);
      const auto dependency = discover(path, form->at);
      program_.modules[id].dependencies.emplace(form, dependency);
    }
    return std::move(program_);
  }
};

} // namespace cod::language
