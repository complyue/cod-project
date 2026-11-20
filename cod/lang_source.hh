//===--- lang_source.hh - Indexed source revisions -------------------===//
#pragma once

#include "cod_source.h"

#include <compare>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace cod::language {

struct SourceSpan {
  size_t first = 0, past = 0;
  auto operator<=>(const SourceSpan &) const = default;
};

class IndexedSource {
  std::string name_;
  std::unique_ptr<cod_source_revision, decltype(&cod_source_release)> revision_{nullptr, cod_source_release};

public:
  explicit IndexedSource(const std::filesystem::path &path) : name_(path.string()) {
    const auto path_bytes = path.u8string();
    cod_source_error error;
    revision_.reset(cod_source_open_readonly(reinterpret_cast<const char *>(path_bytes.c_str()), &error));
    switch (error.status) {
    case COD_SOURCE_OK: break;
    case COD_SOURCE_SYSTEM_ERROR:
      throw std::system_error(static_cast<int>(error.system_error), std::system_category());
    case COD_SOURCE_NOT_REGULAR: throw std::system_error(std::make_error_code(std::errc::invalid_argument));
    case COD_SOURCE_TOO_LARGE: throw std::system_error(std::make_error_code(std::errc::file_too_large));
    case COD_SOURCE_NO_MEMORY: throw std::system_error(std::make_error_code(std::errc::not_enough_memory));
    case COD_SOURCE_CHANGED: throw std::runtime_error("source changed while building its line index");
    }
  }

  const std::string &name() const { return name_; }
  std::string_view bytes() const {
    const auto source = cod_source_borrow(revision_.get());
    return source.size ? std::string_view(reinterpret_cast<const char *>(source.bytes), source.size) : std::string_view{};
  }
  std::pair<size_t, size_t> line_column(size_t offset) const {
    size_t line, column;
    if (!cod_source_line_column(cod_source_borrow(revision_.get()), offset, &line, &column))
      throw std::logic_error("source location outside its indexed revision");
    return {line, column};
  }
};

struct Location {
  std::string file;
  std::shared_ptr<const IndexedSource> source = {};
  SourceSpan span = {};
  auto operator<=>(const Location &) const = default;

  const std::string &filename() const { return source ? source->name() : file; }
};

inline std::string location_text(const Location &at) {
  const auto [line, column] = at.source ? at.source->line_column(at.span.first) : std::pair<size_t, size_t>{1, 1};
  return at.filename() + ":" + std::to_string(line) + ":" + std::to_string(column);
}

} // namespace cod::language
