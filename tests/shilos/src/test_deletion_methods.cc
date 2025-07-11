#include "shilos.hh"
#include "shilos/dict.hh"
#include "shilos/vector.hh"

#include <cassert>
#include <iostream>

using namespace shilos;

// Test root type with TYPE_UUID
struct TestRoot {
  static const UUID TYPE_UUID;
  regional_str name_;

  TestRoot(memory_region<TestRoot> &mr, std::string_view name = "test") : name_(mr, name) {}
};

const UUID TestRoot::TYPE_UUID = UUID("aaaaaaaa-bbbb-cccc-dddd-123456789012");

void test_vector_deletion() {
  std::cout << "=== Testing regional_vector deletion ===" << std::endl;

  auto_region<TestRoot> region(1024 * 1024);
  auto vec = region->create<regional_vector<int>>();

  // Add some elements
  for (int i = 0; i < 10; ++i) {
    vec->emplace_back(*region, i);
  }
  std::cout << "Added 10 elements, size: " << vec->size() << std::endl;
  assert(vec->size() == 10);

  // Test pop_back
  vec->pop_back();
  std::cout << "After pop_back, size: " << vec->size() << std::endl;
  assert(vec->size() == 9);
  assert(vec->back() == 8); // Last element should now be 8

  // Test erase_at (remove element at index 2, which has value 2)
  std::cout << "Before erase_at(2): vec[2] = " << (*vec)[2] << std::endl;
  vec->erase_at(2);
  std::cout << "After erase_at(2), size: " << vec->size() << std::endl;
  assert(vec->size() == 8);
  // Due to swap-with-last, vec[2] should now contain the last element (8)
  std::cout << "After erase_at(2): vec[2] = " << (*vec)[2] << std::endl;
  assert((*vec)[2] == 8);

  // Test iterator-based erase
  auto it = vec->begin();
  std::advance(it, 1); // Point to index 1
  int value_at_1 = *it;
  std::cout << "Erasing element at index 1 with value: " << value_at_1 << std::endl;
  auto result_it = vec->erase(it);
  std::cout << "After iterator erase, size: " << vec->size() << std::endl;
  assert(vec->size() == 7);

  // Test clear
  vec->clear();
  std::cout << "After clear, size: " << vec->size() << std::endl;
  assert(vec->size() == 0);
  assert(vec->empty());

  std::cout << "regional_vector deletion tests passed!" << std::endl;
}

void test_dict_deletion() {
  std::cout << "\n=== Testing regional_dict deletion ===" << std::endl;

  auto_region<TestRoot> region(1024 * 1024);
  auto dict = region->create<regional_dict<regional_str, int>>();

  // Add some key-value pairs
  for (int i = 0; i < 5; ++i) {
    std::string key = "key" + std::to_string(i);
    auto [value_ptr, inserted] = dict->insert(*region, key, i * 10);
    assert(inserted);
  }
  std::cout << "Added 5 key-value pairs, size: " << dict->size() << std::endl;
  assert(dict->size() == 5);

  // Test erase by key
  size_t erased_count = dict->erase("key2");
  std::cout << "Erased 'key2', count: " << erased_count << ", new size: " << dict->size() << std::endl;
  assert(erased_count == 1);
  assert(dict->size() == 4);
  assert(!dict->contains("key2"));

  // Test erase non-existent key
  size_t not_erased = dict->erase("nonexistent");
  std::cout << "Tried to erase non-existent key, count: " << not_erased << std::endl;
  assert(not_erased == 0);
  assert(dict->size() == 4);

  // Test erase by iterator
  auto it = dict->find("key1");
  assert(it != dict->end());
  std::cout << "Erasing 'key1' by iterator..." << std::endl;
  auto result_it = dict->erase(it);
  std::cout << "After iterator erase, size: " << dict->size() << std::endl;
  assert(dict->size() == 3);
  assert(!dict->contains("key1"));

  // Test that remaining keys are still accessible
  assert(dict->contains("key0"));
  assert(dict->contains("key3"));
  assert(dict->contains("key4"));
  assert(dict->at("key0") == 0);
  assert(dict->at("key3") == 30);
  assert(dict->at("key4") == 40);

  // Test clear
  dict->clear();
  std::cout << "After clear, size: " << dict->size() << std::endl;
  assert(dict->size() == 0);
  assert(dict->empty());

  std::cout << "regional_dict deletion tests passed!" << std::endl;
}

int main() {
  std::cout << "Starting deletion methods test suite..." << std::endl;

  test_vector_deletion();
  test_dict_deletion();

  std::cout << "\n✅ All deletion methods tests passed!" << std::endl;
  return 0;
}
