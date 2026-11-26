//===--- lang_diagnostics.hh - Typed source warning diagnostics ------===//
#pragma once

#include "lang_syntax.hh"

#include <algorithm>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace cod::language {

enum class Warning { multi_alias_extension, package_shadow };
struct WarningOptions {
  bool as_errors = false;
  std::set<Warning> errors;
  bool promotes(Warning kind) const { return as_errors || errors.contains(kind); }
};
struct WarningDiagnostic {
  Warning kind;
  Location at;
  std::string message;

  std::string text() const {
    const char *name = nullptr;
    switch (kind) {
    case Warning::multi_alias_extension: name = "multi-alias-extension"; break;
    case Warning::package_shadow: name = "package-shadow"; break;
    }
    return location_text(at) + ": warning: " + message + " [-W" + name + "]";
  }
};

struct DiagnosticFinding {
  std::variant<SyntaxDiagnostic, WarningDiagnostic> primary;
  std::vector<Location> import_sites;
};

class CompilationDiagnostics {
public:
  enum class Coverage { complete, capped, storage_exhausted };

private:
  std::optional<size_t> limit_;
  std::vector<DiagnosticFinding> findings_;
  Coverage coverage_ = Coverage::complete;
  bool has_error_ = false;

public:
  explicit CompilationDiagnostics(std::optional<size_t> limit = {}) : limit_(limit) {}

  // Call only when there is a pending unit of work or unretained finding.
  bool continue_work() {
    if (coverage_ != Coverage::complete) return false;
    if (!limit_ || findings_.size() < *limit_) return true;
    coverage_ = Coverage::capped;
    return false;
  }

  bool append(SyntaxDiagnostic diagnostic) {
    if (!continue_work()) return false;
    findings_.push_back({std::move(diagnostic), {}});
    has_error_ = true;
    return true;
  }

  bool append(WarningDiagnostic diagnostic) {
    if (!continue_work()) return false;
    findings_.push_back({std::move(diagnostic), {}});
    return true;
  }

  void relate_import(size_t first, size_t past, const Location &at) {
    const auto same_site = [&](const auto &site) {
      return site.source == at.source && site.filename() == at.filename() &&
          site.span.first == at.span.first && site.span.past == at.span.past;
    };
    for (size_t i = first; i < past; ++i) {
      auto &finding = findings_[i];
      if (std::visit([&](const auto &primary) { return same_site(primary.at); }, finding.primary)) continue;
      if (std::ranges::none_of(finding.import_sites, same_site)) finding.import_sites.push_back(at);
    }
  }

  void exhaust_storage() { coverage_ = Coverage::storage_exhausted; }
  Coverage coverage() const { return coverage_; }
  bool complete() const { return coverage_ == Coverage::complete; }
  bool has_error() const { return has_error_; }
  size_t size() const { return findings_.size(); }
  const std::vector<DiagnosticFinding> &findings() const { return findings_; }
};

} // namespace cod::language
