#pragma once

#include "./stake.hh"

#include <cassert>
#include <fcntl.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace shilos {

template <typename T> struct stake_header {
  std::uint16_t magic;
  struct {
    std::int16_t major;
    std::int16_t minor;
  } version;
  std::int16_t flags;
  relativ_ptr<T> root;
};

template <typename T>
constexpr stake_header<T> stake_header_v1 = {
    0x3721, // magic
    {1, 0}, // version
    0,      // flags
    nullptr // root
};

template <typename T> class file_stake : public memory_stake {
protected:
  std::string file_name_;
  stake_header<T> *header_;

public:
  explicit file_stake(const std::string &file_name) : file_name_(file_name), header_(nullptr) {
    int fd = open(file_name.c_str(), O_RDONLY);
    if (fd == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to open file: " + file_name);
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to stat file: " + file_name);
    }

    size_t file_size = statbuf.st_size;
    void *mapped_addr = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (mapped_addr == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
    }

    header_ = static_cast<stake_header<T> *>(mapped_addr);

    assume_region(reinterpret_cast<intptr_t>(mapped_addr), file_size);
  }

  ~file_stake() {
    for (const memory_region *mr = live_region(); mr; mr = mr->prev) {
      munmap(reinterpret_cast<void *>(mr->baseaddr), mr->capacity);
    }
  }

  const relativ_ptr<T> &root() const { return header_->root; }
};

template <typename T> class file_stake_builder : public memory_stake {
protected:
  std::string file_name_;
  int fd_;
  stake_header<T> *header_;

public:
  explicit file_stake_builder(const std::string &file_name, size_t min_capacity = sizeof(stake_header_v1<T>))
      : file_name_(file_name), fd_(-1), header_(nullptr) {
    assert(min_capacity >= sizeof(stake_header_v1<T>));

    size_t file_size = 0;

    int fd = open(file_name.c_str(), O_CREAT | O_RDWR);
    if (fd == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to open file: " + file_name);
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to stat file: " + file_name);
    }
    file_size = statbuf.st_size;

    if (file_size < min_capacity) {
      if (ftruncate(fd, min_capacity) == -1) {
        close(fd);
        throw std::system_error(errno, std::system_category(), "Failed to resize file: " + file_name);
      }
      file_size = min_capacity;
    }

    void *mapped_addr = mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_addr == MAP_FAILED) {
      close(fd);
      throw std::system_error(errno, std::system_category(), "Failed to mmap file: " + file_name);
    }

    fd_ = fd;
    header_ = static_cast<stake_header<T> *>(mapped_addr);

    if (statbuf.st_size < sizeof(stake_header_v1<T>)) {
      *header_ = stake_header_v1<T>;
      msync(mapped_addr, sizeof(stake_header_v1<T>), MS_SYNC);
    }

    assume_region(reinterpret_cast<intptr_t>(mapped_addr), file_size);
  }

  void expand(size_t new_capacity) {
    if (live_region()->capacity >= new_capacity)
      return;

    if (ftruncate(fd_, new_capacity) == -1) {
      throw std::system_error(errno, std::system_category(), "Failed to expand file: " + file_name_);
    }

    void *mapped_addr = mmap(nullptr, new_capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_addr == MAP_FAILED) {
      throw std::system_error(errno, std::system_category(), "Failed to re-mmap file: " + file_name_);
    }

    header_ = static_cast<stake_header<T> *>(mapped_addr);

    assume_region(reinterpret_cast<intptr_t>(mapped_addr), new_capacity);
  }

  ~file_stake_builder() {
    if (fd_ != -1)
      close(fd_);
    for (const memory_region *mr = live_region(); mr; mr = mr->prev) {
      munmap(reinterpret_cast<void *>(mr->baseaddr), mr->capacity);
    }
  }

  relativ_ptr<T> &root() { return header_->root; }
};

} // namespace shilos
