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

template <typename T> class regional_cons;

template <typename T> class regional_list {
  friend class regional_cons<T>;

protected:
  regional_ptr<regional_cons<T>> list_head_;

  template <typename RT> regional_list(global_ptr<RT, regional_cons<T>> list_head) : list_head_(list_head) {}

public:
  regional_list() : list_head_() {}

  template <typename RT, typename... Args> void prepand(memory_region<RT> &mr, Args &&...args) {
    list_head_ = mr->create(std::forward<Args>(args)...);
  }

  regional_ptr<regional_cons<T>> &list_head() { return list_head_; }
  const regional_ptr<regional_cons<T>> &list_head() const { return list_head_; }

  explicit operator bool() { return (bool)list_head_; }

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
};

template <typename T, typename RT, typename... Args>
global_ptr<regional_list<T>, RT> operator>>(global_ptr<regional_list<T>, RT> list, Args &&...args) {
  list->prepand(list.region(), std::forward<Args>(args)...);
  return list;
}

template <typename T>
auto operator<=>(const regional_list<T> &lhs, const regional_list<T> &rhs)
  requires std::three_way_comparable<T>;

template <typename T> class regional_cons {
protected:
  T head_;
  regional_list<T> tail_;

public:
  template <typename RT, typename... Args>
  regional_cons(global_ptr<RT, regional_list<T>> tail, Args &&...args)
      : tail_(static_cast<global_ptr<RT, regional_cons<T>>>(tail)) {
    new (&head_) T(std::forward<Args>(args)...);
  }

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

} // namespace shilos
