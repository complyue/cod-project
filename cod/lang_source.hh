//===--- lang_source.hh - Indexed source revisions -------------------===//
#pragma once

#include "llvm/ADT/ScopeExit.h"
#include "llvm/Support/FileSystem.h"

#include <algorithm>
#include <compare>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace cod::language {

struct SourceSpan {
  size_t first = 0, past = 0;
  auto operator<=>(const SourceSpan &) const = default;
};

class IndexedSource {
  std::string name_;
  llvm::sys::fs::mapped_file_region mapping_;
  std::vector<SourceSpan> lines_;

public:
  explicit IndexedSource(const std::filesystem::path &path) : name_(path.string()) {
    namespace fs = llvm::sys::fs;
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
      throw std::system_error(error ? error : std::make_error_code(std::errc::invalid_argument));
    const auto path_bytes = path.u8string();
    auto file = fs::openNativeFileForRead(llvm::StringRef(reinterpret_cast<const char *>(path_bytes.data()), path_bytes.size()));
    if (!file) throw std::system_error(llvm::errorToErrorCode(file.takeError()));
    const llvm::scope_exit close([&] { fs::closeFile(*file); });
    fs::file_status status;
    if (auto error = fs::status(*file, status)) throw std::system_error(error);
    if (!fs::is_regular_file(status)) throw std::system_error(std::make_error_code(std::errc::invalid_argument));
    const auto length = status.getSize();
    if (length > std::numeric_limits<size_t>::max()) throw std::system_error(std::make_error_code(std::errc::file_too_large));
    if (length) {
      mapping_ = fs::mapped_file_region(*file, fs::mapped_file_region::readonly, static_cast<size_t>(length), 0, error);
      if (error) throw std::system_error(error);
    }

    const auto data = bytes();
    size_t first = 0;
    for (size_t newline = data.find('\n'); newline != std::string_view::npos; newline = data.find('\n', first)) {
      lines_.push_back({first, newline});
      first = newline + 1;
    }
    lines_.push_back({first, data.size()});
  }

  const std::string &name() const { return name_; }
  std::string_view bytes() const {
    return mapping_ ? std::string_view(mapping_.const_data(), mapping_.size()) : std::string_view{};
  }
  std::pair<size_t, size_t> line_column(size_t offset) const {
    // A line span excludes the LF separator. The next line includes its own start,
    // including the empty last line after a terminal LF.
    const auto next = std::ranges::upper_bound(lines_, offset, {}, &SourceSpan::first);
    return {static_cast<size_t>(next - lines_.begin()), offset - (next - 1)->first + 1};
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
