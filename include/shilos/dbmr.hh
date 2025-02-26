#pragma once

#include "./region.hh"

#include <cassert>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace shilos {

// Disk Backed Memory Region
template <typename RT>
  requires ValidMemRegionRootType<RT>
class DBMR {
protected:
  std::string file_name_;

private:
  int fd_;
  memory_region<RT> *region_;
  bool constrict_on_close_;

  // internal ctor to be used by other (mostly static) ctors
  DBMR(const std::string &file_name, int fd, memory_region<RT> *region)
      : file_name_(file_name), fd_(fd), region_(region), constrict_on_close_(false) {}

public:
  // writable ctor
  DBMR(const std::string &file_name, size_t reserve_free_capacity)
      : file_name_(file_name), fd_(-1), region_(nullptr), constrict_on_close_(false) {
    size_t file_size = 0;

    fd_ = open(file_name.c_str(), O_RDWR);
    if (fd_ == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to open file: " + file_name);
    }

    struct stat statbuf;
    if (fstat(fd_, &statbuf) == -1) {
      close(fd_);
      throw std::system_error(errno, std::system_category(), "Failed to stat file: " + file_name);
    }
    file_size = statbuf.st_size;
    assert(file_size >= sizeof(memory_region<RT>));

    void *mapped_addr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_addr == MAP_FAILED) {
      close(fd_);
      throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
    }
    region_ = static_cast<memory_region<RT> *>(mapped_addr);

    if (region_->occupation() > file_size) { // this is insane
      munmap(mapped_addr, file_size);
      close(fd_);
      throw std::logic_error("!?DBMR occupied more than the file size?!");
    }

    if (region_->root_type_uuid() != RT::TYPE_UUID) {
      munmap(mapped_addr, file_size);
      close(fd_);
      throw std::runtime_error(std::string("Root Type mismatch: ") + region_->root_type_uuid().to_string() +
                               " vs expected " + RT::TYPE_UUID.to_string());
    }

    if (region_->free_capacity() < reserve_free_capacity) {
      const size_t new_file_size = file_size + reserve_free_capacity - region_->free_capacity();
      munmap(mapped_addr, file_size);
      if (ftruncate(fd_, new_file_size) == -1) {
        close(fd_);
        throw std::system_error(errno, std::system_category(), "Failed to resize file: " + file_name);
      }

      mapped_addr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
      if (mapped_addr == MAP_FAILED) {
        close(fd_);
        throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
      }

      region_ = static_cast<memory_region<RT> *>(mapped_addr);
      region_->capacity_ = file_size = new_file_size;
    }
  }

  ~DBMR() {
    if (region_) {
      assert(fd_ != -1);
      // region_ will be non-readable after munmap
      const size_t occupation = region_->occupation();
      const size_t capacity = region_->capacity();
      assert(occupation <= capacity);
      if (constrict_on_close_ && occupation < capacity) {
        region_->capacity_ = occupation; // to be truncated right below
      }
      msync(reinterpret_cast<void *>(region_), capacity, MS_SYNC);
      munmap(reinterpret_cast<void *>(region_), capacity);
      if (constrict_on_close_ && occupation < capacity) {
        if (ftruncate(fd_, occupation) == -1) {
          std::cerr << "*** Failed to truncate file: " << file_name_ << std::endl;
        }
      }
    }
    if (fd_ != -1) {
      close(fd_);
    }
  }

  // neither copy/move construction, nor assignment
  DBMR(const DBMR &) = delete;
  DBMR(DBMR &&) = delete;
  DBMR &operator=(const DBMR &) = delete;
  DBMR &operator=(DBMR &&) = delete;

  // readonly ctor
  static const DBMR<RT> read(const std::string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    if (fd == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to open file: " + file_name);
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to stat file: " + file_name);
    }
    const size_t file_size = statbuf.st_size;
    if (file_size < sizeof(memory_region<RT>)) {
      throw std::runtime_error("Too small for a memory region: " + file_name);
    }

    void *mapped_addr = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped_addr == MAP_FAILED) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
    }
    auto *region = static_cast<memory_region<RT> *>(mapped_addr);

    if (region->occupation() > file_size) { // this is insane
      munmap(mapped_addr, file_size);
      close(fd);
      throw std::logic_error("!?DBMR occupied more than the file size?!");
    }

    if (region->root_type_uuid() != RT::TYPE_UUID) {
      munmap(mapped_addr, file_size);
      close(fd);
      throw std::runtime_error(std::string("Root Type mismatch: ") + region->root_type_uuid().to_string() +
                               " vs expected " + RT::TYPE_UUID.to_string());
    }

    assert(region->capacity() <= file_size);

    return DBMR<RT>(file_name, fd, region);
  }

  // creation ctor
  template <typename... Args>
  static DBMR<RT> create(const std::string &file_name, size_t free_capacity, Args &&...args) {
    int fd = open(file_name.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to create file: " + file_name);
    }

    const size_t file_size = sizeof(memory_region<RT>) + sizeof(RT) + free_capacity;
    if (ftruncate(fd, file_size) == -1) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to resize file: " + file_name);
    }

    void *mapped_addr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_addr == MAP_FAILED) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
    }

    auto *region = static_cast<memory_region<RT> *>(mapped_addr);
    new (region) memory_region<RT>(file_size, std::forward<Args>(args)...);

    return DBMR<RT>(file_name, fd, region);
  }

  void constrict_on_close(bool constrict_on_close = true) & { constrict_on_close_ = constrict_on_close; }

  memory_region<RT> *region() { return region_; }
  const memory_region<RT> *region() const { return region_; }
};

} // namespace shilos
