#pragma once

#include "./stake.hh"

#include <cassert>
#include <fcntl.h>
#include <new>
#include <stdexcept>
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
  size_t occupation;
  relativ_ptr<T> root;
};

template <typename T>
constexpr stake_header<T> stake_header_v1 = {
    0x3721,                  // magic
    {1, 0},                  // version
    0,                       // flags
    sizeof(stake_header<T>), // occupation
    nullptr                  // root
};

// a readonly memory_stake viewing a backing file
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

// a writable live_stake backed by the specified file
template <typename T> class file_stake_builder : public live_stake {
protected:
  std::string file_name_;

private:
  bool constrict_on_close_;
  int fd_;
  stake_header<T> *header_;

public:
  explicit file_stake_builder(const std::string &file_name, size_t min_capacity, bool constrict_on_close = true)
      : file_name_(file_name), constrict_on_close_(constrict_on_close), fd_(-1), header_(nullptr) {
    assert(min_capacity >= sizeof(stake_header<T>));

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

    if (statbuf.st_size <= 0) { // a fresh new file
      *header_ = stake_header_v1<T>;
      msync(mapped_addr, sizeof(stake_header_v1<T>), MS_SYNC);
    } else { // an existing file
      assert(header_->magic == stake_header_v1<T>.magic);
      // more versions in the future
      assert(header_->version == stake_header_v1<T>.version);
      //
      assert(header_->occupation >= sizeof(stake_header<T>));
      if (header_->occupation > file_size) { // this is insane
        close(fd);
        throw std::logic_error("!?file_stake occupied more than the file size?!");
      }
    }

    assume_region(reinterpret_cast<intptr_t>(mapped_addr), file_size);
  }

  virtual void *allocate(const size_t size, const size_t align) {
    const size_t spc_demanded = align + size; // assume conservative capacity that demanded
    // calculate current free space from latest mmap-ed region
    const memory_region *region = live_region();
    size_t free_spc = region->capacity - header_->occupation;

    // ensure sufficient free capacity is reserved on the backing file
    if (free_spc < spc_demanded) {
      constexpr size_t SZ1GB = 1024 * 1024 * 1024;
      size_t new_capacity;
      if (region->capacity < SZ1GB) {
        // for small (within 1GB) stake files, double its capacity on each expansion
        for (new_capacity = region->capacity * 2; new_capacity - header_->occupation < spc_demanded;
             new_capacity *= 2) {
          if (new_capacity > SZ1GB) { // crossed GB boundary, switch to large file scenario
            new_capacity = 2 * SZ1GB;
            break; // later it'll be expaned GB after GB until large enough
          }
        }
      } else {
        // for large (beyond 1GB) stake files, expand 1GB a time
        new_capacity = region->capacity + SZ1GB;
        if (new_capacity % SZ1GB != 0) { // align to GB boundary
          new_capacity = SZ1GB * (new_capacity / SZ1GB);
        }
      }
      // grow GB-wise till the allocation demand fulfilled
      while (new_capacity - header_->occupation < spc_demanded) {
        new_capacity += SZ1GB;
      }
      // do file size expansion
      reserve_capacity(new_capacity);
      // update the latest region info, as well as new free space after expansion
      region = live_region();
      free_spc = region->capacity - header_->occupation;
    }
    assert(free_spc >= align + size);

    // use current occupation mark as the allocated ptr, do proper alignment
    void *ptr = static_cast<void *>(region->baseaddr + header_->occupation);
    if (!std::align(align, size, ptr, free_spc)) {
      throw std::bad_alloc();
    }

    // move the occupation mark
    header_->occupation = reinterpret_cast<intptr_t>(ptr) + size - region->baseaddr;

    return ptr;
  }

  size_t free_capacity() { return live_region()->capacity - header_->occupation; }

  void reserve_capacity(size_t new_capacity) {
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

  virtual ~file_stake_builder() {
    for (const memory_region *mr = live_region(); mr; mr = mr->prev) {
      // TODO: should check errors from munmap?
      munmap(reinterpret_cast<void *>(mr->baseaddr), mr->capacity);
    }
    if (fd_ != -1) {
      if (constrict_on_close_) {
        assert(header_->occupation <= live_region()->capacity);
        if (ftruncate(fd_, header_->occupation) == -1) {
          close(fd_);
          throw std::system_error(errno, std::system_category(), "Failed to truncate file: " + file_name_);
        }
      }
      close(fd_);
    }
  }

  relativ_ptr<T> &root() { return header_->root; }
};

} // namespace shilos
