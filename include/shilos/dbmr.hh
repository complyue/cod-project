#pragma once

#include "./region.hh"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace shilos {

namespace detail {

class dbmr_file {
#ifdef _WIN32
  HANDLE handle_ = INVALID_HANDLE_VALUE;

  explicit dbmr_file(HANDLE handle) : handle_(handle) {}

  static std::wstring native_path(const std::string &file_name) {
    return std::filesystem::path(file_name).wstring();
  }

  [[noreturn]] static void throw_last_error(const std::string &message, DWORD error = GetLastError()) {
    throw std::system_error(static_cast<int>(error), std::system_category(), message);
  }

#else
  int fd_ = -1;

  explicit dbmr_file(int fd) : fd_(fd) {}

  [[noreturn]] static void throw_last_error(const std::string &message) {
    throw std::system_error(errno, std::system_category(), message);
  }
#endif

public:
  dbmr_file() = default;

  ~dbmr_file() { close(); }

  dbmr_file(const dbmr_file &) = delete;
  dbmr_file &operator=(const dbmr_file &) = delete;

  dbmr_file(dbmr_file &&other) noexcept {
#ifdef _WIN32
    handle_ = std::exchange(other.handle_, INVALID_HANDLE_VALUE);
#else
    fd_ = std::exchange(other.fd_, -1);
#endif
  }

  dbmr_file &operator=(dbmr_file &&other) noexcept {
    if (this != &other) {
      close();
#ifdef _WIN32
      handle_ = std::exchange(other.handle_, INVALID_HANDLE_VALUE);
#else
      fd_ = std::exchange(other.fd_, -1);
#endif
    }
    return *this;
  }

  static dbmr_file open_existing(const std::string &file_name, bool writable) {
#ifdef _WIN32
    const DWORD access = writable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
    HANDLE handle = CreateFileW(native_path(file_name).c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
      throw_last_error("Failed to open file: " + file_name);
    }
    return dbmr_file(handle);
#else
    const int flags = writable ? O_RDWR : O_RDONLY;
    int fd = ::open(file_name.c_str(), flags);
    if (fd == -1) {
      throw_last_error("Failed to open file: " + file_name);
    }
    return dbmr_file(fd);
#endif
  }

  static dbmr_file create_or_open(const std::string &file_name) {
#ifdef _WIN32
    HANDLE handle = CreateFileW(native_path(file_name).c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
      throw_last_error("Failed to create file: " + file_name);
    }
    return dbmr_file(handle);
#else
    int fd = ::open(file_name.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
      throw_last_error("Failed to create file: " + file_name);
    }
    return dbmr_file(fd);
#endif
  }

  bool valid() const noexcept {
#ifdef _WIN32
    return handle_ != INVALID_HANDLE_VALUE;
#else
    return fd_ != -1;
#endif
  }

  size_t size(const std::string &file_name) const {
#ifdef _WIN32
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(handle_, &file_size)) {
      throw_last_error("Failed to stat file: " + file_name);
    }
    if (file_size.QuadPart < 0 ||
        static_cast<unsigned long long>(file_size.QuadPart) > std::numeric_limits<size_t>::max()) {
      throw std::runtime_error("File is too large to map: " + file_name);
    }
    return static_cast<size_t>(file_size.QuadPart);
#else
    struct stat statbuf;
    if (::fstat(fd_, &statbuf) == -1) {
      throw_last_error("Failed to stat file: " + file_name);
    }
    if (statbuf.st_size < 0 || static_cast<unsigned long long>(statbuf.st_size) > std::numeric_limits<size_t>::max()) {
      throw std::runtime_error("File is too large to map: " + file_name);
    }
    return static_cast<size_t>(statbuf.st_size);
#endif
  }

  void resize(size_t new_size, const std::string &file_name) {
#ifdef _WIN32
    LARGE_INTEGER distance;
    distance.QuadPart = static_cast<LONGLONG>(new_size);
    if (!SetFilePointerEx(handle_, distance, nullptr, FILE_BEGIN) || !SetEndOfFile(handle_)) {
      throw_last_error("Failed to resize file: " + file_name);
    }
#else
    if (::ftruncate(fd_, new_size) == -1) {
      throw_last_error("Failed to resize file: " + file_name);
    }
#endif
  }

  void *map(size_t mapping_size, bool writable, const std::string &file_name) {
    if (mapping_size == 0) {
      throw std::runtime_error("Cannot map empty file: " + file_name);
    }

#ifdef _WIN32
    const uint64_t size64 = static_cast<uint64_t>(mapping_size);
    const DWORD size_high = static_cast<DWORD>(size64 >> 32);
    const DWORD size_low = static_cast<DWORD>(size64 & 0xffffffffu);
    const DWORD protect = writable ? PAGE_READWRITE : PAGE_READONLY;
    HANDLE mapping = CreateFileMappingW(handle_, nullptr, protect, size_high, size_low, nullptr);
    if (!mapping) {
      throw_last_error("Failed to create file mapping: " + file_name);
    }

    const DWORD access = writable ? (FILE_MAP_READ | FILE_MAP_WRITE) : FILE_MAP_READ;
    void *mapped_addr = MapViewOfFile(mapping, access, 0, 0, mapping_size);
    const DWORD map_error = GetLastError();
    CloseHandle(mapping);
    if (!mapped_addr) {
      throw_last_error("Failed to mmap file: " + file_name, map_error);
    }
    return mapped_addr;
#else
    const int prot = PROT_READ | (writable ? PROT_WRITE : 0);
    void *mapped_addr = ::mmap(nullptr, mapping_size, prot, MAP_SHARED, fd_, 0);
    if (mapped_addr == MAP_FAILED) {
      throw_last_error("Failed to mmap file: " + file_name);
    }
    return mapped_addr;
#endif
  }

  void flush(void *addr, size_t size) noexcept {
    if (!addr || size == 0) {
      return;
    }
#ifdef _WIN32
    FlushViewOfFile(addr, size);
    FlushFileBuffers(handle_);
#else
    ::msync(addr, size, MS_SYNC);
#endif
  }

  void unmap(void *addr, size_t size) noexcept {
    if (!addr) {
      return;
    }
#ifdef _WIN32
    UnmapViewOfFile(addr);
#else
    ::munmap(addr, size);
#endif
  }

  void close() noexcept {
#ifdef _WIN32
    if (handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(handle_);
      handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (fd_ != -1) {
      ::close(fd_);
      fd_ = -1;
    }
#endif
  }
};

} // namespace detail

// Disk Backed Memory Region
template <typename RT>
  requires ValidMemRegionRootType<RT>
class DBMR {
protected:
  std::string file_name_;

private:
  detail::dbmr_file file_;
  memory_region<RT> *region_;
  size_t mapped_size_;
  bool constrict_on_close_;

  // internal ctor to be used by other (mostly static) ctors
  DBMR(const std::string &file_name, detail::dbmr_file file, memory_region<RT> *region, size_t mapped_size)
      : file_name_(file_name), file_(std::move(file)), region_(region), mapped_size_(mapped_size),
        constrict_on_close_(false) {}

public:
  // writable ctor
  DBMR(const std::string &file_name, size_t reserve_free_capacity)
      : file_name_(file_name), file_(), region_(nullptr), mapped_size_(0), constrict_on_close_(false) {
    size_t file_size = 0;

    file_ = detail::dbmr_file::open_existing(file_name, true);
    file_size = file_.size(file_name);
    assert(file_size >= sizeof(memory_region<RT>));

    void *mapped_addr = file_.map(file_size, true, file_name);
    mapped_size_ = file_size;
    region_ = static_cast<memory_region<RT> *>(mapped_addr);

    if (region_->occupation() > file_size) { // this is insane
      file_.unmap(mapped_addr, file_size);
      file_.close();
      throw std::logic_error("!?DBMR occupied more than the file size?!");
    }

    if (region_->root_type_uuid() != RT::TYPE_UUID) {
      file_.unmap(mapped_addr, file_size);
      file_.close();
      throw std::runtime_error(std::string("Root Type mismatch: ") + region_->root_type_uuid().to_string() +
                               " vs expected " + RT::TYPE_UUID.to_string());
    }

    if (region_->free_capacity() < reserve_free_capacity) {
      const size_t new_file_size = file_size + reserve_free_capacity - region_->free_capacity();
      file_.unmap(mapped_addr, file_size);
      file_.resize(new_file_size, file_name);

      file_size = new_file_size;
      mapped_addr = file_.map(file_size, true, file_name);
      mapped_size_ = file_size;

      region_ = static_cast<memory_region<RT> *>(mapped_addr);
      region_->capacity_ = file_size;
      file_.flush(reinterpret_cast<void *>(region_), sizeof(memory_region<RT>));
    }
  }

  ~DBMR() {
    if (region_) {
      assert(file_.valid());
      // region_ will be non-readable after munmap
      const size_t occupation = region_->occupation();
      const size_t capacity = region_->capacity();
      assert(occupation <= capacity);
      if (constrict_on_close_ && occupation < capacity) {
        region_->capacity_ = occupation; // to be truncated right below
      }
      file_.flush(reinterpret_cast<void *>(region_), occupation);
      file_.unmap(reinterpret_cast<void *>(region_), mapped_size_ != 0 ? mapped_size_ : capacity);
      region_ = nullptr;
      if (constrict_on_close_ && occupation < capacity) {
        try {
          file_.resize(occupation, file_name_);
        } catch (const std::exception &) {
          std::cerr << "*** Failed to truncate file: " << file_name_ << std::endl;
        }
      }
    }
    file_.close();
  }

  // neither copy/move construction, nor assignment
  DBMR(const DBMR &) = delete;
  DBMR(DBMR &&) = delete;
  DBMR &operator=(const DBMR &) = delete;
  DBMR &operator=(DBMR &&) = delete;

  // readonly ctor
  // WARNING: Memory region opened with readonly() is mapped with PROT_READ only.
  // Any subsequent write operations to this memory region will trigger a bus error
  // and crash the entire process.
  //
  // TODO: Future DSL work should add "readonly" flags on returned objects to
  // semantically prevent writing on readonly DBMR objects at the language level.
  static DBMR<RT> readonly(const std::string &file_name) {
    auto file = detail::dbmr_file::open_existing(file_name, false);

    const size_t file_size = file.size(file_name);
    if (file_size < sizeof(memory_region<RT>)) {
      throw std::runtime_error("Too small for a memory region: " + file_name);
    }

    void *mapped_addr = file.map(file_size, false, file_name);
    auto *region = static_cast<memory_region<RT> *>(mapped_addr);

    if (region->occupation() > file_size) { // this is insane
      file.unmap(mapped_addr, file_size);
      file.close();
      throw std::logic_error("!?DBMR occupied more than the file size?!");
    }

    if (region->root_type_uuid() != RT::TYPE_UUID) {
      file.unmap(mapped_addr, file_size);
      file.close();
      throw std::runtime_error(std::string("Root Type mismatch: ") + region->root_type_uuid().to_string() +
                               " vs expected " + RT::TYPE_UUID.to_string());
    }

    assert(region->capacity() <= file_size);

    return DBMR<RT>(file_name, std::move(file), region, file_size);
  }

  // creation ctor
  template <typename... Args>
    requires std::constructible_from<RT, memory_region<RT> &, Args...>
  static DBMR<RT> create(const std::string &file_name, size_t free_capacity, Args &&...args) {
    auto file = detail::dbmr_file::create_or_open(file_name);

    const size_t file_size = sizeof(memory_region<RT>) + sizeof(RT) + free_capacity;
    file.resize(file_size, file_name);

    void *mapped_addr = file.map(file_size, true, file_name);

    auto *region = static_cast<memory_region<RT> *>(mapped_addr);
    new (region) memory_region<RT>(file_size, std::forward<Args>(args)...);

    return DBMR<RT>(file_name, std::move(file), region, file_size);
  }

  void constrict_on_close(bool constrict_on_close = true) & { constrict_on_close_ = constrict_on_close; }

  memory_region<RT> &region() { return *region_; }
  const memory_region<RT> &region() const { return *region_; }
};

} // namespace shilos
