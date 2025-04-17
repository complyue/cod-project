# Memory Region and Regional Types Specification

This document defines the requirements and implementation details for memory regions and regional types in the system.

## Core Requirements

### Memory Region Interface

The `memory_region` template class provides the foundation for regional memory management:

```cpp
template <typename RT>
class memory_region {
  // Core functionality:
  // - Type-aware allocation and construction
  // - Atomic resource management (objects cannot be individually destroyed)
  // - Conversion between regional_ptr and global_ptr
  // - Enforcement of regional type constraints
};
```

### Regional Type Constraints

All regional types must satisfy the following constraints. Specialized types (regional_str, regional_list) implement additional container functionality while maintaining compliance.

### 1. Field Type Constraints

   - **Bits types** (primitive types with neither destructor nor internal pointers):
     - Follow standard C++ type rules
     - No additional constraints
   - Members cannot contain external pointers of any kind
   - Members containing internal pointers must:
     - Use only `regional_ptr` (raw pointers including `global_ptr` are prohibited)
     - Point only to bits types or compliant regional types

### 2. Construction Rules

Regional types must adhere to strict construction requirements:

   - Must implement at least one constructor taking `memory_region&`
   - When arguments exist, `memory_region&` must be first parameter
   - Default constructors are allowed only when it:
     - Initialize all `regional_ptr` members to null
     - Does no allocation
   - Non-default construction:
     - Must use region-aware constructor with `memory_region&` passed as first parameter
     - All allocations must go through provided `memory_region&`

### 3. Lifetime Rules

Regional types have strict lifetime management requirements:

   - Copy and move construction/assignment are prohibited for regional types (allowed for bits types)
   - Individual destruction is prohibited for both bits types and regional types
     - Individual objects cannot be destroyed
   - Destruction occurs atomically at region level:
     - Entire object graph released with region
     - The root type (`RT`) of `memory_region<RT>` is responsible for resource acquisition and release
   - Non-root bits types and regional types should avoid owning external resources

### 4. YAML Serialization

All regional types must support YAML serialization with these requirements:

   - Required implementations:
     - `to_yaml()` serialization method
     - Two `from_yaml` forms:
       1. Returns `global_ptr` (required)
       2. Assigns to `regional_ptr` (optional, default from `YamlConvertible`)
   - Type safety strictly enforced

### 5. Pointer Rules

The system supports several pointer types with specific semantics:

   - `regional_ptr` (intra-region):
     - Designed for region-local storage (lvalue)
     - Relative to its own memory address - rvalue semantics is illegal
   - `global_ptr` (cross-region):
     - Safe but with 2x space cost
     - Lifetime bound to the referenced `memory_region`
   - Raw pointers:
     - Raw pointers to regional memory can be passed around in the program, but not allowed to be stored in region memory
   - Soundness:
     - The root type (`RT`) of `memory_region<RT>` should decide and define memory and type safety semantics
     - The simplest strategy is to not support reuse of region memory, thus regional objects will never be deallocated nor change type
     - If the root type does support memory reuse, e.g. support garbage collection, it should clearly define lifetime rules of the object graph it would manage, and type-safety strategies to follow by the memory_region user, in separate specifications.

## Usage Guidelines

### Memory Placement

- Objects of regional types must reside in a `memory_region<RT>`
- Stack or register allocation is prohibited due to `regional_ptr` address relativity

### Pointer Semantics

- Raw pointers/references may be used temporarily but:
  - Must not outlive the owning `memory_region`
  - Must account for potential garbage collection by the root type
- `regional_ptr` provides automatic relocation when region memory is remapped

## Implementation Details

### Specialized Container Types

The system provides optimized implementations of common data structures that satisfy all regional type constraints:

1. **regional_str** - String type that:

   - Region-allocated string storage
   - Satisfies all regional type constraints:
     - Construction exclusively via `memory_region`
     - No copy/move operations
     - Compliant YAML serialization
   - Optimized operations:
     - O(1) length/data access
     - Efficient comparison operators (<=>, ==)
     - Zero-cost `std::string_view` conversion

2. **regional_list** - Linked list that:
   - Implements linked list with both ends tracked
   - Satisfies all constraints:
     - Constructed via memory_region
     - No copying/moving
     - Correct YAML serialization
   - Complete container interface:
     - Iteration (begin()/end())
     - Size tracking
     - Comparison (<=>)
   - Efficient operations:
     - prepend_to/append_to

### YAML Integration

The YAML integration is enforced via the `YamlConvertible` concept:

```cpp
template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node,
                                 memory_region<RT> &mr,
                                 regional_ptr<T> &to_ptr) {
  // Required serialization method
  { t.to_yaml() } noexcept -> std::same_as<yaml::Node>;

  // Required deserialization forms
  { T::from_yaml(mr, node) } -> std::same_as<global_ptr<T, RT>>;
  { T::from_yaml(mr, node, to_ptr) } -> std::same_as<void>;

  // Exception safety guarantees
  requires requires {
    []() {
      try {
        memory_region<RT> mr;
        yaml::Node node;
        regional_ptr<T> to_ptr;
        auto ptr = T::from_yaml(mr, node);
        T::from_yaml(mr, node, to_ptr);
      } catch (const yaml::Exception &) {
        // Expected behavior
      } catch (...) {
        static_assert(false,
          "from_yaml() must only throw yaml::Exception or derived types");
      }
    };
  };
};

// Default implementation of the second from_yaml in terms of the first
template <typename T, typename RT>
  requires YamlConvertible<T, RT>
void from_yaml(memory_region<RT>& mr, const yaml::Node& node, regional_ptr<T>& to_ptr) {
  to_ptr = T::from_yaml(mr, node);
}
```
