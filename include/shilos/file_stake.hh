
#pragma once

#include "./stake.hh"

#include <string>

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
    0xCAFE3721, // magic
    {1, 0},     // version
    0,          // flags
    nullptr     // root
};

template <typename T> class file_stake : public memory_stake {
protected:
  std::string file_name_;
  stake_header<T> *header;

public:
  static const file_stake &&readonly(const std::string &file_name) {
    //
    return file_stake(file_name, true, true);
  }

  file_stake(const std::string &file_name,
             bool readonly = false,                           //
             bool must_existing = false,                      //
             size_t min_capacity = sizeof(stake_header_v1<T>) //
             )
      : file_name_(file_name) {
    //
    // mmap the data file according the args, have `header` point to the start address
    //
    // when not readonly, ensure the file size is at least `min_capacity`
    //
    // if the file is newly created, fill in v1 header and flush
    //
    // call base `assume_region()` method
    //
  }

  ~file_stake() {
    // TODO: munmap all `memory_region`s ever assumed
  }

  const relativ_ptr<T> &root() const { return header->root; }
  relativ_ptr<T> &root() { return header->root; }
};

} // namespace shilos
