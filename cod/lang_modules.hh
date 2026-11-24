//===--- lang_modules.hh - Source module discovery ------------------===//
#pragma once

#include "lang_diagnostics.hh"
#include "codp_module_inputs.hh"

#include <map>
#include <set>

namespace cod::language {

struct SourceModule {
  enum class ReadState { unavailable, recovered, parsed };
  std::filesystem::path path;
  std::shared_ptr<const IndexedSource> source;
  std::vector<std::unique_ptr<Form>> forms;
  std::map<const Form *, size_t> dependencies;
  std::set<const Form *> failed_edges;
  ReadState read_state = ReadState::unavailable;
  size_t findings_first = 0, findings_past = 0;
};

struct SourceProgram {
  std::vector<SourceModule> modules;
  std::vector<size_t> construction_order;

  bool ready_for_checking() const {
    return !modules.empty() && construction_order.size() == modules.size() &&
        std::ranges::all_of(modules, [](const auto &module) {
          return module.read_state == SourceModule::ReadState::parsed && module.failed_edges.empty();
        });
  }
};

struct ModuleSearchInputs {
  std::filesystem::path bundle;
  std::filesystem::path project;
};

class ModuleLoader {
  SourceProgram program_;
  ModuleSearchInputs search_;
  CompilationDiagnostics &diagnostics_;
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
      } catch (const std::bad_alloc &) {
        throw;
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
        diagnostics_.append(WarningDiagnostic{Warning::package_shadow, form.at, "package module '" + form.path +
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
    const auto id = program_.modules.size();
    paths_.emplace(resolved, id);
    program_.modules.emplace_back();
    auto &module = program_.modules.back();
    module.path = resolved;
    module.findings_first = diagnostics_.size();
    std::shared_ptr<const IndexedSource> source;
    try {
      source = std::make_shared<IndexedSource>(resolved);
    } catch (const std::system_error &error) {
      if (error.code() == std::errc::not_enough_memory) {
        diagnostics_.exhaust_storage();
      } else {
        diagnostics_.append(SyntaxDiagnostic{at, "cannot read CoD source '" + resolved.string() + "': " +
            error.code().message()});
      }
      module.findings_past = diagnostics_.size();
      return id;
    } catch (const std::runtime_error &error) {
      diagnostics_.append(SyntaxDiagnostic{at, "cannot read CoD source '" + resolved.string() + "': " + error.what()});
      module.findings_past = diagnostics_.size();
      return id;
    }
    auto parsed = Parser(source).parse();
    module.source = std::move(source);
    module.forms = std::move(parsed.forms);
    module.read_state = parsed.reports.empty() ? SourceModule::ReadState::parsed : SourceModule::ReadState::recovered;
    std::stable_sort(parsed.reports.begin(), parsed.reports.end(), [](const auto &lhs, const auto &rhs) {
      return lhs.at.span.first < rhs.at.span.first;
    });
    for (auto &report : parsed.reports) if (!diagnostics_.append(std::move(report))) break;
    module.findings_past = diagnostics_.size();
    scanning_.insert(id);
    pending_.push_back({id});
    return id;
  }

public:
  ModuleLoader(ModuleSearchInputs search, CompilationDiagnostics &diagnostics)
      : search_(std::move(search)), diagnostics_(diagnostics) {}

  SourceProgram load(const std::filesystem::path &entry) {
    try {
      discover(entry, {entry.string()});
    } catch (const LocatedFailure &failure) {
      diagnostics_.append(failure.diagnostic);
    }
    while (diagnostics_.complete() && !pending_.empty()) {
      auto &pending = pending_.back();
      const auto id = pending.module;
      if (pending.next_form == program_.modules[id].forms.size()) {
        scanning_.erase(id);
        program_.construction_order.push_back(id);
        pending_.pop_back();
        continue;
      }
      const auto *form = program_.modules[id].forms[pending.next_form++].get();
      if (form->kind == Form::Kind::syntax_error || form->path.empty() || form->path.starts_with("std:")) continue;
      if (!diagnostics_.continue_work()) break;
      // Forms retain their addresses when discovery grows either work list.
      try {
        const auto path = resolve_dependency(program_.modules[id], *form);
        if (!diagnostics_.continue_work()) break;
        const auto dependency = discover(path, form->at);
        program_.modules[id].dependencies.emplace(form, dependency);
        const auto &target = program_.modules[dependency];
        diagnostics_.relate_import(target.findings_first, target.findings_past, form->at);
      } catch (const LocatedFailure &failure) {
        program_.modules[id].failed_edges.insert(form);
        diagnostics_.append(failure.diagnostic);
      }
    }
    return std::move(program_);
  }
};

} // namespace cod::language
