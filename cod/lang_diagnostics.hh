//===--- lang_diagnostics.hh - Typed source warning diagnostics ------===//
#pragma once

#include "lang_syntax.hh"

#include <set>
#include <string>

namespace cod::language {

enum class Warning { unit_conversion, multi_alias_extension, shadow, unused_capture, missing_return };
struct WarningOptions {
  bool as_errors = false;
  std::set<Warning> errors;
};
struct WarningDiagnostic {
  Warning kind;
  Location at;
  std::string message;

  std::string text() const {
    const char *name = nullptr;
    switch (kind) {
    case Warning::unit_conversion: name = "unit-conversion"; break;
    case Warning::multi_alias_extension: name = "multi-alias-extension"; break;
    case Warning::shadow: name = "shadow"; break;
    case Warning::unused_capture: name = "unused-capture"; break;
    case Warning::missing_return: name = "missing-return"; break;
    }
    return location_text(at) + ": warning: " + message + " [-W" + name + "]";
  }
};

} // namespace cod::language
