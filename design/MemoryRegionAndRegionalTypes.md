# Memory Region and Regional Types Specification

## Core Requirements

### Memory Region Interface

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

All regional types must satisfy these constraints. Specialized types (regional_str, regional_list) implement additional container functionality while maintaining compliance.

1. Field Type Constraints:

   - Bits types (member types with neither dtor nor internal pointers):
     - Follow standard C++ type rules
     - No additional constraints
   - Members cannot contain external pointers of any kind
   - Members containing internal pointers must:
     - Use only `regional_ptr` (raw pointers including `global_ptr` are prohibited)
     - Point only to bits types or compliant regional types

2. Construction Rules:

   - Must implement at least one constructor taking `memory_region&`
   - When arguments exist, `memory_region&` must be first parameter
   - Default constructors are allowed only when it:
     - Initialize all `regional_ptr` members to null
     - Does no allocation
   - Non-default construction:
     - Must use region-aware constructor with `memory_region&` passed as first parameter
     - All allocations must go through provided `memory_region&`

3. Lifetime Rules:

   - Copy and move construction and assignment are prohibited for regional types (though allowed for bits types)
   - Individual destruction is prohibited for both bits types and regional types
     - Individual objects cannot be destroyed
   - Destruction occurs atomically at region level:
     - Entire object graph released with region
     - The root type (`RT`) of `memory_region<RT>` is responsible for resource acquisition and release
   - Non-root bits types and regional types should avoid owning external resources

4. YAML Serialization:

   - Required:
     - `to_yaml()` serialization method
     - Two `from_yaml` forms:
       1. Returns `global_ptr` (required)
       2. Assigns to `regional_ptr` (optional, default from `YamlConvertible`)
   - Type safety strictly enforced

5. Pointer Rules:
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

## Implementation Details

### Specialized Types

The system implements two specialized types providing common data structures while satisfying all constraints:

1. **regional_str** - String type that:

   - Stores string data in region
   - Satisfies all constraints:
     - Constructed via memory_region
     - No copying/moving
     - Correct YAML serialization
   - Efficient operations:
     - Length/data access
     - Comparison (<=>, ==)
     - std::string_view conversion

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

```cpp
template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node,
                                 memory_region<RT> &mr,
                                 regional_ptr<T> &to_ptr) {
  // Serialization requirement
  { t.to_yaml() } noexcept -> std::same_as<yaml::Node>;

  // Deserialization variants
  { T::from_yaml(mr, node) } -> std::same_as<global_ptr<T, RT>>;
  { T::from_yaml(mr, node, to_ptr) } -> std::same_as<void>;

  // Exception safety contract
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
