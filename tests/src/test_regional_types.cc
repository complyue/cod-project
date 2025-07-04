#include "shilos.hh"
#include "shilos/dict.hh"
#include "shilos/list.hh"
#include "shilos/vector.hh"

#include <cassert>
#include <iostream>
#include <string_view>

using namespace shilos;

// Test root type with TYPE_UUID
struct TestRoot {
  static const UUID TYPE_UUID;
  regional_str name_;
  regional_fifo<int> numbers_;

  TestRoot(memory_region<TestRoot> &mr, std::string_view name = "test") : name_(mr, name), numbers_(mr) {}
};

const UUID TestRoot::TYPE_UUID = UUID("cccccccc-dddd-eeee-ffff-333333333333");

void test_regional_str() {
  std::cout << "=== Testing regional_str ===" << std::endl;

  // Create memory region with capacity - using alloc_region static method
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test construction using intern_str factory functions that return global_ptr
  auto str1 = intern_str(mr, std::string_view("Hello, World!"));
  auto str2 = intern_str(mr, std::string_view("Regional String"));
  auto str3 = intern_str(mr, std::string_view("")); // Empty string

  // Test basic operations
  assert(str1->size() == 13);
  assert(str2->size() == 15);
  assert(str3->size() == 0);
  assert(str3->empty());

  // Test data access
  assert(str1->data() != nullptr);
  // Use string_view conversion
  assert(std::string_view(*str1) == "Hello, World!");

  // Test comparison
  auto str4 = intern_str(mr, std::string_view("Hello, World!"));
  assert(*str1 == *str4);
  assert(*str1 != *str2);

  // Test string_view conversion
  std::string_view sv = *str1;
  assert(sv == "Hello, World!");

  std::cout << "regional_str tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

void test_regional_fifo() {
  std::cout << "=== Testing regional_fifo ===" << std::endl;

  // Create memory region with capacity
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test with int (bits type) - use create to get global_ptr
  auto int_fifo = mr.create<regional_fifo<int>>();
  assert(int_fifo->empty());
  assert(int_fifo->size() == 0);

  // Test enque operations
  int_fifo->enque(mr, 1);
  int_fifo->enque(mr, 2);
  int_fifo->enque(mr, 3);

  assert(int_fifo->size() == 3);
  assert(!int_fifo->empty());

  // Test iteration (FIFO order)
  int expected = 1;
  for (const auto &val : *int_fifo) {
    assert(val == expected++);
  }

  // Test with regional_str
  auto str_fifo = mr.create<regional_fifo<regional_str>>();
  str_fifo->enque(mr, std::string_view("first"));
  str_fifo->enque(mr, std::string_view("second"));
  str_fifo->enque(mr, std::string_view("third"));

  assert(str_fifo->size() == 3);

  // Check order by comparing with string_view
  auto it = str_fifo->begin();
  assert(std::string_view(*it) == "first");
  ++it;
  assert(std::string_view(*it) == "second");
  ++it;
  assert(std::string_view(*it) == "third");

  std::cout << "regional_fifo tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

void test_regional_lifo() {
  std::cout << "=== Testing regional_lifo ===" << std::endl;

  // Create memory region with capacity
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test with int (bits type) - use create to get global_ptr
  auto lifo = mr.create<regional_lifo<int>>();
  assert(lifo->empty());
  assert(lifo->size() == 0);

  // Test push operations
  lifo->push(mr, 1);
  lifo->push(mr, 2);
  lifo->push(mr, 3);

  assert(lifo->size() == 3);
  assert(!lifo->empty());

  // Test iteration (LIFO order)
  int expected = 3;
  for (const auto &val : *lifo) {
    assert(val == expected--);
  }

  // Test with regional_str
  auto str_lifo = mr.create<regional_lifo<regional_str>>();
  str_lifo->push(mr, std::string_view("first"));
  str_lifo->push(mr, std::string_view("second"));
  str_lifo->push(mr, std::string_view("third"));

  assert(str_lifo->size() == 3);

  // Check LIFO order by comparing with string_view
  auto it = str_lifo->begin();
  assert(std::string_view(*it) == "third");
  ++it;
  assert(std::string_view(*it) == "second");
  ++it;
  assert(std::string_view(*it) == "first");

  std::cout << "regional_lifo tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

void test_regional_vector() {
  std::cout << "=== Testing regional_vector ===" << std::endl;

  // Create memory region with capacity
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test with int (bits type) - use create to get global_ptr
  auto vec = mr.create<regional_vector<int>>();
  assert(vec->empty());
  assert(vec->size() == 0);

  // Test emplace_back operations
  vec->emplace_back(mr, 10);
  vec->emplace_back(mr, 20);
  vec->emplace_back(mr, 30);

  assert(vec->size() == 3);
  assert(!vec->empty());

  // Test indexed access
  assert((*vec)[0] == 10);
  assert((*vec)[1] == 20);
  assert((*vec)[2] == 30);

  // Test iteration
  int expected = 10;
  for (const auto &val : *vec) {
    assert(val == expected);
    expected += 10;
  }

  // Test with regional_str
  auto str_vec = mr.create<regional_vector<regional_str>>();
  str_vec->emplace_back(mr, std::string_view("apple"));
  str_vec->emplace_back(mr, std::string_view("banana"));
  str_vec->emplace_back(mr, std::string_view("cherry"));

  assert(str_vec->size() == 3);
  assert(std::string_view((*str_vec)[0]) == "apple");
  assert(std::string_view((*str_vec)[1]) == "banana");
  assert(std::string_view((*str_vec)[2]) == "cherry");

  std::cout << "regional_vector tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

void test_regional_dict() {
  std::cout << "=== Testing regional_dict ===" << std::endl;

  // Create memory region with capacity
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test with string keys and int values - use create to get global_ptr
  auto dict = mr.create<regional_dict<regional_str, int>>();
  assert(dict->empty());
  assert(dict->size() == 0);

  // Test insertion using insert_or_assign
  dict->insert_or_assign(mr, std::string_view("one"), 1);
  dict->insert_or_assign(mr, std::string_view("two"), 2);
  dict->insert_or_assign(mr, std::string_view("three"), 3);

  assert(dict->size() == 3);
  assert(!dict->empty());

  // Test lookup using find_value and contains
  assert(dict->contains(std::string_view("two")));
  int *value_ptr = dict->find_value(std::string_view("two"));
  assert(value_ptr != nullptr);
  assert(*value_ptr == 2);

  assert(!dict->contains(std::string_view("nonexistent")));
  int *nonexistent_ptr = dict->find_value(std::string_view("nonexistent"));
  assert(nonexistent_ptr == nullptr);

  // Test iteration
  int sum = 0;
  for (const auto &[key, value] : *dict) {
    sum += value;
  }
  assert(sum == 6); // 1 + 2 + 3

  std::cout << "regional_dict tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

void test_pointer_semantics() {
  std::cout << "=== Testing pointer semantics ===" << std::endl;

  // Create memory region with capacity
  auto mr_ptr = memory_region<TestRoot>::alloc_region(1024 * 1024); // 1MB capacity
  memory_region<TestRoot> &mr = *mr_ptr;

  // Test regional_ptr with intern_str factory function
  auto str_obj = intern_str(mr, std::string_view("test string"));
  regional_ptr<regional_str> rptr;
  rptr = str_obj.get(); // Assign raw pointer to regional_ptr

  assert(rptr != nullptr);
  assert(rptr->size() == 11);
  assert(std::string_view(*rptr) == "test string");

  // Test global_ptr - use cast_ptr method to create from regional_ptr
  auto gptr = mr.cast_ptr(rptr);
  assert(gptr != nullptr);
  assert(gptr->size() == 11);
  assert(std::string_view(*gptr) == "test string");

  std::cout << "pointer semantics tests passed!" << std::endl;

  // Clean up
  delete mr_ptr;
}

int main() {
  std::cout << "Starting regional types test suite..." << std::endl;

  try {
    test_regional_str();
    test_regional_fifo();
    test_regional_lifo();
    test_regional_vector();
    test_regional_dict();
    test_pointer_semantics();

    std::cout << std::endl << "✅ All regional types tests passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
