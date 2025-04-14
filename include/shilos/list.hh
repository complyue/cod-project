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

template <typename T> class regional_cons {
private:
  T value_;
  regional_ptr<regional_cons<T>> next_, tail_;

public:
  template <typename RT, typename... Args>
  friend void append_to(regional_ptr<regional_cons<T>> &rp, memory_region<RT> &mr, Args &&...args);

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_cons(memory_region<RT> &mr, Args &&...args) : value_(std::forward<Args>(args)...) {
    tail_ = this;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  global_ptr<regional_cons<T>, RT> prepend(memory_region<RT> &mr, Args &&...args) {
    auto gp = mr.template create<regional_cons<T>>(std::forward(args)...);
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

  T &value() { return value_; }
  const T &value() const { return value_; }

  regional_ptr<regional_cons<T>> &next() { return next_; }
  const regional_ptr<regional_cons<T>> &next() const { return next_; }

  regional_ptr<regional_cons<T>> &tail() {
    assert(tail_); // tail should at least point to self, never null
    while (tail_->next_)
      tail_ = tail_->next_.get();
    return tail_;
  }
  const regional_ptr<regional_cons<T>> &tail() const { return const_cast<regional_cons<T>>(this)->tail(); }

  size_t size() const {
    size_t count = 1;
    for (regional_cons<T> *l = this; l; l = l->next_.get())
      count++;
    return count;
  }
};

// Forward declaration of the pointer version of <=> operator
template <typename T>
auto operator<=>(const regional_ptr<regional_cons<T>> &lhs, const regional_ptr<regional_cons<T>> &rhs)
  requires std::three_way_comparable<T>;

template <typename T>
auto operator<=>(const regional_cons<T> &lhs, const regional_cons<T> &rhs)
  requires std::three_way_comparable<T>
{
  const std::strong_ordering r = lhs.value() <=> rhs.value();
  if (r != std::strong_ordering::equal)
    return r;

  const regional_ptr<regional_cons<T>> &lhs_next = lhs.next();
  const regional_ptr<regional_cons<T>> &rhs_next = rhs.next();

  if (!lhs_next) {
    if (!rhs_next)
      return std::strong_ordering::equal;
    else
      return std::strong_ordering::less;
  } else if (!rhs_next) {
    return std::strong_ordering::greater;
  }

  return lhs_next <=> rhs_next;
}

template <typename T>
auto operator<=>(const regional_ptr<regional_cons<T>> &lhs, const regional_ptr<regional_cons<T>> &rhs)
  requires std::three_way_comparable<T>
{
  if (!lhs) {
    if (!rhs)
      return std::strong_ordering::equal;
    else
      return std::strong_ordering::less;
  } else if (!rhs) {
    return std::strong_ordering::greater;
  }

  const std::strong_ordering r = lhs->value() <=> rhs->value();
  if (r != std::strong_ordering::equal)
    return r;

  const regional_ptr<regional_cons<T>> &lhs_next = lhs->next();
  const regional_ptr<regional_cons<T>> &rhs_next = rhs->next();

  return lhs_next <=> rhs_next;
}

template <typename T> class regional_list {
private:
  regional_ptr<regional_cons<T>> head_;

public:
  regional_list() : head_() {}

  template <typename RT> regional_list(memory_region<RT> &mr) : head_() {}

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_list(memory_region<RT> &mr, Args &&...args) : head_() {
    mr.create_to(head_, std::forward<Args>(args)...);
  }

  regional_ptr<regional_cons<T>> &head() { return head_; }

  size_t size() const { return head_ ? head_->size() : 0; }

  class iterator {
    regional_cons<T> *current_;

  public:
    iterator(regional_cons<T> *current) : current_(current) {}

    T &operator*() { return current_->value(); }
    T *operator->() { return &current_->value(); }

    iterator &operator++() {
      current_ = current_->next().get();
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &other) const { return current_ == other.current_; }
    bool operator!=(const iterator &other) const { return current_ != other.current_; }

    yaml::Node to_yaml() const {
      yaml::Sequence seq;
      for (const auto &item : *this) {
        if constexpr (yaml::YamlConvertible<T, void>) {
          seq.emplace_back(item.to_yaml());
        } else {
          throw yaml::TypeError("List element type is not YamlConvertible");
        }
      }
      return yaml::Node(seq);
    }

    template <typename RT>
      requires yaml::YamlConvertible<T, RT>
    static global_ptr<regional_list<T>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) {
      if (!std::holds_alternative<yaml::Sequence>(node.value)) {
        throw yaml::TypeError("Expected Sequence for regional_list");
      }

      auto list = mr.template create<regional_list<T>>();
      const auto &seq = std::get<yaml::Sequence>(node.value);
      for (const auto &item : seq) {
        append_to(*list, mr, T::from_yaml(mr, item));
      }
      return list;
    }

    template <typename RT>
      requires yaml::YamlConvertible<T, RT>
    static void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<regional_list<T>> &to_ptr) {
      if (!std::holds_alternative<yaml::Sequence>(node.value)) {
        throw yaml::TypeError("Expected Sequence for regional_list");
      }

      mr.template create_to<regional_list<T>>(to_ptr);
      const auto &seq = std::get<yaml::Sequence>(node.value);
      for (const auto &item : seq) {
        if constexpr (yaml::YamlConvertible<T, RT>) {
          append_to(*to_ptr, mr, T::from_yaml(mr, item));
        } else {
          throw yaml::TypeError("List element type is not YamlConvertible");
        }
      }
    }
  };

  class const_iterator {
    const regional_cons<T> *current_;

  public:
    const_iterator(const regional_cons<T> *current) : current_(current) {}

    const T &operator*() const { return current_->value(); }
    const T *operator->() const { return &current_->value(); }

    const_iterator &operator++() {
      current_ = current_->next().get();
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

  iterator begin() { return iterator(head_.get()); }
  iterator end() { return iterator(nullptr); }

  const_iterator begin() const { return const_iterator(head_.get()); }
  const_iterator end() const { return const_iterator(nullptr); }
};

template <typename T>
auto operator<=>(const regional_list<T> &lhs, const regional_list<T> &rhs)
  requires std::three_way_comparable<T>
{
  return lhs.head() <=> rhs.head();
}

template <typename RT, typename T, typename... Args>
  requires std::constructible_from<T, memory_region<RT> &, Args...>
void append_to(regional_list<T> &rp, memory_region<RT> &mr, Args &&...args) {
  regional_cons<T> *const p = rp.get();
  if (!p) {
    mr.template create_to<regional_cons<T>>(rp, std::forward(args)...);
  } else {
    mr.template create_to<regional_cons<T>>(&(p->tail()->next_), std::forward(args)...);
    rp->next_ = p;
    rp->tail_ = p->tail().get();
  }
}

template <typename RT, typename T, typename... Args>
  requires std::constructible_from<T, memory_region<RT> &, Args...>
void prepend_to(regional_list<T> &rp, memory_region<RT> &mr, Args &&...args) {
  regional_cons<T> *const p = rp.get();
  mr.template create_to<regional_cons<T>>(rp, std::forward(args)...);
  if (p) {
    rp->next_ = p;
    rp->tail_ = p->tail().get();
  }
}

} // namespace shilos
