#include "shilos.hh"
#include "shilos/dict.hh"
#include "shilos/list.hh"

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>

using namespace shilos;

// Test root types with TYPE_UUID
struct TestRoot {
  static const UUID TYPE_UUID;
  regional_str data_;

  TestRoot(memory_region<TestRoot> &mr, std::string_view data = "test") : data_(mr, data) {}
};

struct ComplexRoot {
  static const UUID TYPE_UUID;
  regional_str name_;
  regional_fifo<regional_str> items_;
  regional_dict<regional_str, int> counters_;
  regional_ptr<regional_str> selected_;

  ComplexRoot(memory_region<ComplexRoot> &mr, std::string_view name = "complex")
      : name_(mr, name), items_(mr), counters_(mr), selected_() {}
};

// Define UUIDs
const UUID TestRoot::TYPE_UUID = UUID("aaaaaaaa-bbbb-cccc-dddd-111111111111");
const UUID ComplexRoot::TYPE_UUID = UUID("bbbbbbbb-cccc-dddd-eeee-222222222222");

void test_basic_region_creation() {
  std::cout << "=== Testing basic region creation ===" << std::endl;

  // Test RAII approach with auto_region (recommended)
  {
    auto_region<TestRoot> region1(1024 * 1024);
    auto_region<TestRoot> region2(1024 * 1024, "custom_name");

    auto root1 = region1->root();
    auto root2 = region2->root();

    // Test that roots are properly initialized
    assert(root1->data_ == std::string_view("test"));
    assert(root2->data_ == std::string_view("custom_name"));

    std::cout << "RAII region creation tests passed!" << std::endl;
  } // Automatic cleanup via RAII

  // Test manual memory management (for comparison)
  {
    auto mr1 = memory_region<TestRoot>::alloc_region(1024 * 1024);
    auto mr2 = memory_region<TestRoot>::alloc_region(1024 * 1024, "custom_name");

    auto root1 = mr1->root();
    auto root2 = mr2->root();

    // Test that roots are properly initialized
    assert(root1->data_ == std::string_view("test"));
    assert(root2->data_ == std::string_view("custom_name"));

    std::cout << "Manual region creation tests passed!" << std::endl;

    // Manual cleanup
    memory_region<TestRoot>::free_region(mr1);
    memory_region<TestRoot>::free_region(mr2);
  }

  std::cout << "Basic region creation tests passed!" << std::endl;
}

void test_region_allocation() {
  std::cout << "=== Testing region allocation ===" << std::endl;

  auto_region<TestRoot> region(1024 * 1024);

  // Test allocation and construction
  auto root = region->root();

  // Test that we can access the root data
  assert(root->data_ == std::string_view("test"));

  // Test creating additional objects in the region
  auto str_obj = region->create<regional_str>("allocated string");
  assert(*str_obj == std::string_view("allocated string"));

  std::cout << "Region allocation tests passed!" << std::endl;

  // Automatic cleanup via RAII
}

void test_pointer_conversions() {
  std::cout << "=== Testing pointer conversions ===" << std::endl;

  auto_region<ComplexRoot> region(1024 * 1024);
  auto root = region->root();

  // Add some items to the fifo
  root->items_.enque(*region, "item1");
  root->items_.enque(*region, "item2");
  root->items_.enque(*region, "item3");

  // Test regional_ptr assignment via global_ptr
  auto it = root->items_.begin();
  ++it; // Point to second item

  // Set the selected pointer using raw pointer assignment (allowed but unsafe)
  root->selected_ = &(*it);

  assert(root->selected_);
  assert(*root->selected_ == std::string_view("item2"));

  // Test global_ptr creation from regional_ptr
  auto gptr = region->cast_ptr(root->selected_);
  assert(gptr);
  assert(*gptr == std::string_view("item2"));

  std::cout << "Pointer conversions tests passed!" << std::endl;

  // Automatic cleanup via RAII
}

void test_region_constraints() {
  std::cout << "=== Testing region constraints ===" << std::endl;

  auto_region<TestRoot> region(1024 * 1024);
  auto root = region->root();

  // Test that we can access regional types properly
  assert(root->data_ == std::string_view("test"));

  // Test that containers require memory_region for element construction
  auto str_fifo = region->create<regional_fifo<regional_str>>();
  str_fifo->enque(*region, "element1");
  str_fifo->enque(*region, "element2");

  assert(str_fifo->size() == 2);

  std::cout << "Region constraints tests passed!" << std::endl;

  // Automatic cleanup via RAII
}

void test_multiple_regions() {
  std::cout << "=== Testing multiple regions ===" << std::endl;

  auto_region<TestRoot> region1(1024 * 1024, "region1");
  auto_region<TestRoot> region2(1024 * 1024, "region2");

  auto root1 = region1->root();
  auto root2 = region2->root();

  // Test that regions are independent
  assert(root1->data_ == std::string_view("region1"));
  assert(root2->data_ == std::string_view("region2"));

  // Test that regions are truly independent
  std::cout << "Region 1 data: " << root1->data_ << std::endl;
  std::cout << "Region 2 data: " << root2->data_ << std::endl;

  // Test global_ptr references
  assert(root1->data_ == std::string_view("region1"));
  assert(root2->data_ == std::string_view("region2"));

  std::cout << "Multiple regions tests passed!" << std::endl;

  // Automatic cleanup via RAII - both regions destroyed automatically
}

void test_region_lifetime() {
  std::cout << "=== Testing region lifetime ===" << std::endl;

  std::unique_ptr<auto_region<TestRoot>> region_ptr;
  std::optional<global_ptr<TestRoot, TestRoot>> gptr;

  // Test 1: Create region and verify access within scope
  {
    auto region = std::make_unique<auto_region<TestRoot>>(1024 * 1024, "scoped");
    auto root = (*region)->root();

    // Test that we can access the object
    assert(root->data_ == std::string_view("scoped"));

    // Test that global_ptr is valid within scope
    assert(root);
    assert(root.get() != nullptr);

    std::cout << "Root data in scope: " << root->data_ << std::endl;

    // Transfer ownership to extend region lifetime
    region_ptr = std::move(region);
    gptr.emplace(std::move(root)); // Use emplace to construct in place
  }

  // Test 2: Verify global_ptr still works when region is properly managed
  assert(gptr.has_value());
  assert(gptr->get() != nullptr);
  assert((**gptr).data_ == std::string_view("scoped"));

  std::cout << "Root data after scope: " << (**gptr).data_ << std::endl;

  // Test 3: Test region destruction and cleanup
  gptr.reset();       // Clear the global_ptr first
  region_ptr.reset(); // Then destroy the region

  // Test manual memory management lifetime for comparison
  {
    auto *mr = memory_region<TestRoot>::alloc_region(1024 * 1024, "manual");
    auto root = mr->root();

    assert(root->data_ == std::string_view("manual"));
    std::cout << "Manual region root data: " << root->data_ << std::endl;

    // Manual cleanup
    memory_region<TestRoot>::free_region(mr);
  }

  std::cout << "Region lifetime tests passed!" << std::endl;
}

void test_nested_regional_types() {
  std::cout << "=== Testing nested regional types ===" << std::endl;

  auto_region<ComplexRoot> region(1024 * 1024, "nested");
  auto root = region->root();

  // Create nested structure
  root->items_.enque(*region, "first");
  root->items_.enque(*region, "second");
  root->items_.enque(*region, "third");

  // Test that nested allocations work
  assert(root->items_.size() == 3);

  // Test accessing nested elements
  auto it = root->items_.begin();
  assert(*it == std::string_view("first"));
  ++it;
  assert(*it == std::string_view("second"));
  ++it;
  assert(*it == std::string_view("third"));

  // Test regional_ptr to nested element
  it = root->items_.begin();
  root->selected_ = &(*it);
  assert(*root->selected_ == std::string_view("first"));

  // Test dictionary operations using the current API
  root->counters_.insert_or_assign(*region, std::string_view("first"), 1);
  root->counters_.insert_or_assign(*region, std::string_view("second"), 2);

  assert(root->counters_.size() == 2);
  assert(*root->counters_.find_value("first") == 1);
  assert(*root->counters_.find_value("second") == 2);

  std::cout << "Nested regional types tests passed!" << std::endl;

  // Automatic cleanup via RAII
}

int main() {
  std::cout << "Starting memory regions test suite..." << std::endl;

  try {
    test_basic_region_creation();
    test_region_allocation();
    test_pointer_conversions();
    test_region_constraints();
    test_multiple_regions();
    test_region_lifetime();
    test_nested_regional_types();

    std::cout << std::endl << "✅ All memory regions tests passed!" << std::endl;
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
