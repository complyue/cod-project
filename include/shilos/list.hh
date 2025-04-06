#pragma once

#include "./region.hh"

#include <cassert>
#include <compare>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace shilos {

template <typename T> class regional_list;

template <typename T>
auto operator<=>(const regional_list<T> &lhs, const regional_list<T> &rhs)
  requires std::three_way_comparable<T>;

template <typename T> class regional_cons {
  friend class regional_list<T>;

private:
  T head_;
  regional_list<T> tail_;

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_cons(memory_region<RT> &mr, regional_list<T> &tail, Args &&...args) : tail_(mr, tail) {
    std::construct_at(&head_, mr, std::forward<Args>(args)...);
  }

public:
  T &head() { return head_; }
  const T &head() const { return head_; }

  regional_list<T> &tail() { return tail_; }
  const regional_list<T> &tail() const { return tail_; }
};

template <typename T>
auto operator<=>(const regional_cons<T> &lhs, const regional_cons<T> &rhs)
  requires std::three_way_comparable<T>
{
  if (auto cmp = lhs.head() <=> rhs.head(); cmp != 0)
    return cmp;
  const regional_list<T> &lhs_tail = lhs.tail();
  const regional_list<T> &rhs_tail = rhs.tail();
  if (!lhs_tail) {
    if (!rhs_tail)
      return std::strong_ordering::equal;
    else
      return std::strong_ordering::less;
  } else if (!rhs_tail) {
    return std::strong_ordering::greater;
  }
  return lhs_tail <=> rhs_tail;
}

template <typename T> class regional_list {
  friend class regional_cons<T>;

private:
  regional_ptr<regional_cons<T>> list_head_;

  template <typename RT>
  regional_list(memory_region<RT> &mr, regional_list<T> &list_head) : list_head_(list_head.list_head_.get()) {}

public:
  template <typename RT> regional_list(memory_region<RT> &mr) : list_head_() {}

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_list(memory_region<RT> &mr, Args &&...args) : list_head_() {
    this->prepend(mr, std::forward<Args>(args)...);
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void prepend(memory_region<RT> &mr, Args &&...args) {
    mr.create_at(list_head_, *this, std::forward<Args>(args)...);
  }

  T *head() { return !list_head_ ? nullptr : &list_head_->head(); }
  const T *head() const { return !list_head_ ? nullptr : &list_head_->head(); }

  regional_list<T> *tail() {
    if (!list_head_)
      return nullptr;
    return list_head_->tail();
  }
  const regional_list<T> *tail() const {
    if (!list_head_)
      return nullptr;
    return list_head_->tail();
  }

  explicit operator bool() const { return (bool)list_head_; }

  bool empty() const { return !list_head_; }

  size_t size() const {
    const regional_cons<T> *curr = list_head_.get();
    if (!curr)
      return 0;
    size_t count = 1;
    for (curr = curr->tail(); curr; curr = curr->tail())
      count++;
    return count;
  }

  class iterator {
  private:
    regional_cons<T> *current_;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using reference = T &;

    iterator() : current_() {}
    explicit iterator(regional_cons<T> *node) : current_(node) {}

    reference operator*() { return current_->head(); }
    pointer operator->() { return &(current_->head()); }

    iterator &operator++() {
      current_ = current_->tail().list_head_.get();
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const { return current_ == other.current_; }
  };

  class const_iterator {
  private:
    const regional_cons<T> *current_;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

    const_iterator() : current_() {}
    explicit const_iterator(regional_cons<T> *node) : current_(node) {}

    reference operator*() const { return current_->head(); }
    pointer operator->() const { return &(current_->head()); }

    const_iterator &operator++() {
      current_ = current_->tail().list_head_.get();
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator &other) const { return current_ == other.current_; }
    bool operator!=(const const_iterator &other) const { return current_ != other.current_; }
  };

  iterator begin() { return iterator(list_head_.get()); }
  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(list_head_.get()); }
  const_iterator end() const { return const_iterator(); }

  const_iterator cbegin() const { return const_iterator(list_head_.get()); }
  const_iterator cend() const { return const_iterator(); }
};

template <typename T>
auto operator<=>(const regional_list<T> &lhs, const regional_list<T> &rhs)
  requires std::three_way_comparable<T>
{
  const regional_cons<T> *lhs_cons = lhs.list_head().get();
  const regional_cons<T> *rhs_cons = rhs.list_head().get();
  if (!lhs_cons) {
    if (!rhs_cons)
      return std::strong_ordering::equal;
    else
      return std::strong_ordering::less;
  } else if (!rhs_cons) {
    return std::strong_ordering::greater;
  }
  return *lhs_cons <=> *rhs_cons;
}

//
// operator >> version of prepend for regional_list
//
template <typename T, typename RT, typename... Args>
global_ptr<regional_list<T>, RT> operator>>(global_ptr<regional_list<T>, RT> list, Args &&...args) {
  list->prepend(list.region(), std::forward<Args>(args)...);
  return list;
}

} // namespace shilos
