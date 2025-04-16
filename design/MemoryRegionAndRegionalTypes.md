# Memory Region and Regional Types Design

## Core Design Principles

### Memory Region Abstraction

```cpp
template <typename RT>
class memory_region {
  // Core responsibilities:
  // - Type-aware allocation and construction services
  // - Atomic resource management (no individual object destruction)
  // - Pointer conversion utilities between regional and global references
  // - Ensures all contained objects follow regional type constraints
};
```

### Regional Type Requirements

1. Construction Semantics:
   - Mandatory constructor parameter: `memory_region&` reference
   - Nullary constructors are explicitly prohibited
   - All construction must occur within a memory_region context

2. Lifetime Management:
   - Copy and move operations are strictly forbidden
   - Destruction follows region-wide atomic pattern:
     * No individual object destruction allowed
     * Complete object graph released when region is destroyed
     * Root value initiates cascading resource release
   - External resource ownership should be minimized
   - Object lifetime is exactly scoped to containing memory_region

3. Field Type Constraints:
   - "Bits types" (members without internal pointers):
     * Treated as plain C++ types
     * No special regional constraints apply
   - Members with internal pointers must:
     * Be regional types themselves
     * Violation makes containing type ill-formed
     * Propagates to any outer regional types

4. YAML Serialization Protocol:
   - Required implementations:
     * `to_yaml()` for serialization
     * Two `from_yaml` variants:
       1. (Mandatory) Returns `global_ptr`
       2. (Optional) Assigns to given `regional_ptr` (default provided by `YamlConvertible`)
   - Strict type safety enforced during conversions

5. Pointer Semantics and Safety:
   - `regional_ptr` (intra-region references):
     * Optimized for temporary usage (rvalue semantics)
     * Persistent storage prohibited (lvalue usage invalid)
     * Violations trigger immediate memory access faults
   - `global_ptr` (cross-region references):
     * Safe but with 2x space overhead
     * Preferred for persistent references
   - Raw pointers:
     * Permitted but bypass safety mechanisms
     * Risks include:
       - Dangling references
       - Type safety violations
       - Memory access faults

## Design Architecture

### YAML Integration Specification

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
