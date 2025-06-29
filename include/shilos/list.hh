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
  regional_ptr<regional_cons<T>> next_;

public:
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_cons(memory_region<RT> &mr, Args &&...args) : value_(mr, std::forward<Args>(args)...) {}

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  regional_cons(memory_region<RT> &mr, Args &&...args) : value_(std::forward<Args>(args)...) {}

  // Deleted special members
  regional_cons(const regional_cons &) = delete;
  regional_cons(regional_cons &&) = delete;
  regional_cons &operator=(const regional_cons &) = delete;
  regional_cons &operator=(regional_cons &&) = delete;

  T &value() { return value_; }
  const T &value() const { return value_; }

  regional_ptr<regional_cons<T>> &next() { return next_; }
  const regional_ptr<regional_cons<T>> &next() const { return next_; }

  size_t size() const {
    size_t count = 1;
    for (const regional_cons<T> *l = next_.get(); l; l = l->next_.get())
      count++;
    return count;
  }

  // Friend declarations for efficient transfer functions
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_fifo<U> &to);
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

// FIFO (First In, First Out) - Queue semantics
template <typename T> class regional_fifo {
private:
  regional_ptr<regional_cons<T>> head_;
  regional_ptr<regional_cons<T>> tail_;

  // Friend declarations for efficient transfer functions
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_fifo<U> &to);

  // Friend declarations for YAML functions
  template <typename U, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_fifo<U> *raw_ptr);
  template <typename U, typename RT>
  friend global_ptr<regional_fifo<U>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node);

public:
  regional_fifo() : head_(), tail_() {}

  template <typename RT> regional_fifo(memory_region<RT> &mr) : head_(), tail_() {}

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_fifo(memory_region<RT> &mr, Args &&...args) : head_(), tail_() {
    mr.create_to(&head_, mr, std::forward<Args>(args)...);
    tail_ = head_;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  regional_fifo(memory_region<RT> &mr, Args &&...args) : head_(), tail_() {
    mr.create_to(&head_, mr, std::forward<Args>(args)...);
    tail_ = head_;
  }

  // Deleted special members
  regional_fifo(const regional_fifo &) = delete;
  regional_fifo(regional_fifo &&) = delete;
  regional_fifo &operator=(const regional_fifo &) = delete;
  regional_fifo &operator=(regional_fifo &&) = delete;

  // Add element to back of queue
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void enque(memory_region<RT> &mr, Args &&...args) {
    auto new_node = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    if (!head_) {
      head_ = tail_ = new_node.get();
    } else {
      tail_->next() = new_node.get();
      tail_ = new_node.get();
    }
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void enque(memory_region<RT> &mr, Args &&...args) {
    auto new_node = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    if (!head_) {
      head_ = tail_ = new_node.get();
    } else {
      tail_->next() = new_node.get();
      tail_ = new_node.get();
    }
  }

  // Add element to front of queue
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void enque_front(memory_region<RT> &mr, Args &&...args) {
    auto new_head = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    new_head->next() = head_.get();
    if (!head_) {
      tail_ = new_head.get();
    }
    head_ = new_head.get();
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void enque_front(memory_region<RT> &mr, Args &&...args) {
    auto new_head = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    new_head->next() = head_.get();
    if (!head_) {
      tail_ = new_head.get();
    }
    head_ = new_head.get();
  }

  // Access front element without removing
  T *front() { return head_ ? &head_->value() : nullptr; }

  const T *front() const { return head_ ? &head_->value() : nullptr; }

  // Access back element without removing
  T *back() { return tail_ ? &tail_->value() : nullptr; }

  const T *back() const { return tail_ ? &tail_->value() : nullptr; }

  bool empty() const { return !head_; }
  size_t size() const { return head_ ? head_->size() : 0; }

  regional_ptr<regional_cons<T>> &head() { return head_; }
  const regional_ptr<regional_cons<T>> &head() const { return head_; }

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

// LIFO (Last In, First Out) - Stack semantics
template <typename T> class regional_lifo {
private:
  regional_ptr<regional_cons<T>> head_;
  regional_ptr<regional_cons<T>> tail_;

  // Friend declarations for efficient transfer functions
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_back(regional_lifo<U> &from, regional_fifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_fifo<U> &from, regional_lifo<U> &to);
  template <typename U> friend bool transfer_front_to_front(regional_lifo<U> &from, regional_fifo<U> &to);

  // Friend declarations for YAML functions
  template <typename U, typename RT>
  friend void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_lifo<U> *raw_ptr);
  template <typename U, typename RT>
  friend global_ptr<regional_lifo<U>, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node);

public:
  regional_lifo() : head_(), tail_() {}

  template <typename RT> regional_lifo(memory_region<RT> &mr) : head_(), tail_() {}

  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  regional_lifo(memory_region<RT> &mr, Args &&...args) : head_(), tail_() {
    mr.create_to(&head_, mr, std::forward<Args>(args)...);
    tail_ = head_;
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  regional_lifo(memory_region<RT> &mr, Args &&...args) : head_(), tail_() {
    mr.create_to(&head_, mr, std::forward<Args>(args)...);
    tail_ = head_;
  }

  // Deleted special members
  regional_lifo(const regional_lifo &) = delete;
  regional_lifo(regional_lifo &&) = delete;
  regional_lifo &operator=(const regional_lifo &) = delete;
  regional_lifo &operator=(regional_lifo &&) = delete;

  // Add element to top of stack
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void push(memory_region<RT> &mr, Args &&...args) {
    auto new_head = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    new_head->next() = head_.get();
    if (!head_) {
      tail_ = new_head.get();
    }
    head_ = new_head.get();
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void push(memory_region<RT> &mr, Args &&...args) {
    auto new_head = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    new_head->next() = head_.get();
    if (!head_) {
      tail_ = new_head.get();
    }
    head_ = new_head.get();
  }

  // Add element to bottom of stack
  template <typename RT, typename... Args>
    requires std::constructible_from<T, memory_region<RT> &, Args...>
  void push_back(memory_region<RT> &mr, Args &&...args) {
    auto new_node = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    if (!head_) {
      head_ = tail_ = new_node.get();
    } else {
      tail_->next() = new_node.get();
      tail_ = new_node.get();
    }
  }

  template <typename RT, typename... Args>
    requires std::constructible_from<T, Args...>
  void push_back(memory_region<RT> &mr, Args &&...args) {
    auto new_node = mr.template create<regional_cons<T>>(mr, std::forward<Args>(args)...);
    if (!head_) {
      head_ = tail_ = new_node.get();
    } else {
      tail_->next() = new_node.get();
      tail_ = new_node.get();
    }
  }

  // Access top element without removing (same as front for stack)
  T *top() { return head_ ? &head_->value() : nullptr; }
  const T *top() const { return head_ ? &head_->value() : nullptr; }

  // Access bottom element without removing
  T *back() { return tail_ ? &tail_->value() : nullptr; }
  const T *back() const { return tail_ ? &tail_->value() : nullptr; }

  // Alias for consistency with FIFO interface
  T *front() { return top(); }
  const T *front() const { return top(); }

  bool empty() const { return !head_; }
  size_t size() const { return head_ ? head_->size() : 0; }

  regional_ptr<regional_cons<T>> &head() { return head_; }
  const regional_ptr<regional_cons<T>> &head() const { return head_; }

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

// Efficient transfer functions between lists - no allocation, just pointer manipulation
template <typename T> bool transfer_front_to_back(regional_fifo<T> &from, regional_fifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();
  if (!from.head_) {
    from.tail_ = nullptr; // List is now empty
  }

  // Add node to back of destination
  if (to.empty()) {
    to.head_ = to.tail_ = node;
  } else {
    to.tail_->next() = node;
    to.tail_ = node;
  }
  node->next() = nullptr;

  return true;
}

template <typename T> bool transfer_front_to_back(regional_lifo<T> &from, regional_lifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();
  if (!from.head_) {
    from.tail_ = nullptr; // List is now empty
  }

  // Add node to back of destination
  if (to.empty()) {
    to.head_ = to.tail_ = node;
  } else {
    to.tail_->next() = node;
    to.tail_ = node;
  }
  node->next() = nullptr;

  return true;
}

template <typename T> bool transfer_front_to_front(regional_fifo<T> &from, regional_fifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();
  if (!from.head_) {
    from.tail_ = nullptr; // List is now empty
  }

  // Add node to front of destination
  if (to.empty()) {
    to.head_ = to.tail_ = node;
  } else {
    node->next() = to.head_.get();
    to.head_ = node;
  }

  return true;
}

template <typename T> bool transfer_front_to_front(regional_lifo<T> &from, regional_lifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();

  // Add node to front of destination (top of stack)
  node->next_ = to.head_.get();
  to.head_ = node;

  return true;
}

template <typename T> bool transfer_front_to_back(regional_fifo<T> &from, regional_lifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();

  // Add node to back of destination
  if (to.empty()) {
    to.head_ = node;
    to.tail_ = node;
  } else {
    to.tail_->next_ = node;
    to.tail_ = node;
  }
  node->next_ = nullptr;

  return true;
}

template <typename T> bool transfer_front_to_back(regional_lifo<T> &from, regional_fifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();

  // Add node to back of destination
  if (to.empty()) {
    to.head_ = node;
    to.tail_ = node;
  } else {
    to.tail_->next_ = node;
    to.tail_ = node;
  }
  node->next_ = nullptr;

  return true;
}

template <typename T> bool transfer_front_to_front(regional_fifo<T> &from, regional_lifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();

  // Add node to front of destination (top of stack)
  node->next_ = to.head_.get();
  to.head_ = node;

  return true;
}

template <typename T> bool transfer_front_to_front(regional_lifo<T> &from, regional_fifo<T> &to) {
  if (from.empty())
    return false;

  // Remove front node from source
  regional_cons<T> *node = from.head_.get();
  from.head_ = node->next_.get();
  if (!from.head_) {
    from.tail_ = nullptr; // List is now empty
  }

  // Add node to front of destination
  if (to.empty()) {
    to.head_ = to.tail_ = node;
    node->next() = nullptr;
  } else {
    node->next() = to.head_.get();
    to.head_ = node;
  }

  return true;
}

// Comparison operators for FIFO
template <typename T>
auto operator<=>(const regional_fifo<T> &lhs, const regional_fifo<T> &rhs)
  requires std::three_way_comparable<T>
{
  return lhs.head() <=> rhs.head();
}

// Comparison operators for LIFO
template <typename T>
auto operator<=>(const regional_lifo<T> &lhs, const regional_lifo<T> &rhs)
  requires std::three_way_comparable<T>
{
  return lhs.head() <=> rhs.head();
}

} // namespace shilos
