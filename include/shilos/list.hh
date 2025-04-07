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

template <typename T> class regional_list {
private:
  T head_;
  regional_ptr<regional_list<T>> next_, tail_;

public:
  template <typename RT, typename... Args>
  friend void append_to(regional_ptr<regional_list<T>> &rp, memory_region<RT> &mr, Args &&...args);

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_list(memory_region<RT> &mr, Args &&...args) : head_(std::forward<Args>(args)...) {
    tail_ = this;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  global_ptr<regional_list<T>, RT> prepend(memory_region<RT> &mr, Args &&...args) {
    auto gp = mr.template create<regional_list<T>>(std::forward(args)...);
    gp->next_ = this;
    gp->tail_ = tail().get();
    return gp;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void append(memory_region<RT> &mr, Args &&...args) {
    mr.create_to(&(tail()->next_), std::forward(args)...);
    assert(tail_->next_); // or construction failed yet not throwing ?!
    tail_ = tail_->next_.get();
  }

  T &head() { return head_; }
  const T &head() const { return head_; }

  regional_ptr<regional_list<T>> &next() { return next_; }
  const regional_ptr<regional_list<T>> &next() const { return next_; }

  regional_ptr<regional_list<T>> &tail() {
    assert(tail_); // tail should at least point to self, never null
    while (tail_->next_)
      tail_ = tail_->next_.get();
    return tail_;
  }
  const regional_ptr<regional_list<T>> &tail() const { return const_cast<regional_list<T>>(this)->tail(); }

  size_t size() const {
    size_t count = 1;
    for (regional_list<T> *l = this; l; l = l->next_.get())
      count++;
    return count;
  }
};

template <typename T>
auto operator<=>(const regional_list<T> &lhs, const regional_list<T> &rhs)
  requires std::three_way_comparable<T>
{
  const std::strong_ordering r = lhs.head() <=> rhs.head();
  if (r != std::strong_ordering::equal)
    return r;
  const regional_list<T> *lhs_next = lhs.next().get();
  const regional_list<T> *rhs_next = rhs.next().get();
  if (!lhs_next) {
    if (!rhs_next)
      return std::strong_ordering::equal;
    else
      return std::strong_ordering::less;
  } else if (!rhs_next) {
    return std::strong_ordering::greater;
  }
  return *lhs_next <=> *rhs_next;
}

template <typename RT, typename T, typename... Args>
  requires std::constructible_from<T, memory_region<RT> &, Args...>
void append_to(regional_ptr<regional_list<T>> &rp, memory_region<RT> &mr, Args &&...args) {
  regional_list<T> *const p = rp.get();
  if (!p) {
    mr.template create_to<regional_list<T>>(rp, std::forward(args)...);
  } else {
    mr.template create_to<regional_list<T>>(&(p->tail()->next_), std::forward(args)...);
    rp->next_ = p;
    rp->tail_ = p->tail().get();
  }
}

template <typename RT, typename T, typename... Args>
  requires std::constructible_from<T, memory_region<RT> &, Args...>
void prepend_to(regional_ptr<regional_list<T>> &rp, memory_region<RT> &mr, Args &&...args) {
  regional_list<T> *const p = rp.get();
  mr.template create_to<regional_list<T>>(rp, std::forward(args)...);
  if (p) {
    rp->next_ = p;
    rp->tail_ = p->tail().get();
  }
}

} // namespace shilos
