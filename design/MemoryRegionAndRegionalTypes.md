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

All regional types must satisfy the following constraints. Specialized types (regional_str, regional_fifo, regional_lifo) implement additional container functionality while maintaining compliance.

### 1. Field Type Constraints

   - **Bits types** (primitive types with neither destructor nor internal pointers):
     - Follow standard C++ type rules
     - No additional constraints
   - Members cannot contain external pointers of any kind
   - Members containing internal pointers must:
     - Use only `regional_ptr` (raw pointers including `global_ptr` are prohibited)
     - Point only to bits types or compliant regional types

### 2. Construction Rules

   - Every regional type must provide at least one constructor that accepts `memory_region&` as its first parameter
   - Default constructors (when provided) initialize all (direct and indirect) `regional_ptr` members to null without performing any allocation
   - Constructors that accept `memory_region&` are permitted to allocate from the provided region
   - Any allocation performed must use the provided `memory_region&` parameter

### 3. Lifetime Rules

Regional types have strict lifetime management requirements:

   - Copy and move construction/assignment are prohibited for regional types (allowed for bits types)
   - Individual destruction is prohibited for both bits types and regional types
     - Individual objects cannot be destroyed
   - Destruction occurs atomically at region level:
     - Entire object graph released with region
     - The root type (`RT`) of `memory_region<RT>` is responsible for resource acquisition and release
   - Non-root bits types and regional types should avoid owning external resources

### 4. YAML Serialization (Optional)

YAML serialization support is optional and modular for regional types:

   - **Opt-in Design**: Regional types do not require built-in YAML methods
   - **Standalone Functions**: YAML support provided via standalone template functions in separate headers
   - **Modular Inclusion**: Users include specific `*_yaml.hh` headers to enable YAML for desired types
   - **Concept Compliance**: When YAML support is included, types must satisfy `YamlConvertible` concept

### 5. Pointer Rules

The system supports several pointer types with specific semantics:

   - `regional_ptr` (intra-region):
     - Stores references as relative offsets from its own memory address
     - Provides region-local storage with automatic relocation support
     - Supports construction from raw pointers for offset calculation
     - Cannot be used with rvalue semantics due to address-relative storage
   - `global_ptr` (cross-region):
     - Safe cross-region references with 2x space cost
     - Lifetime bound to the referenced `memory_region`
   - Raw pointers:
     - May be passed around temporarily but not stored persistently in region memory
     - Can be used for `regional_ptr` construction/assignment to calculate offsets
   - Memory safety:
     - The root type (`RT`) of `memory_region<RT>` defines memory and type safety semantics
     - The simplest strategy is to not support reclaim of region memory, thus regional objects will never be deallocated nor change type
     - If the root type supports memory reclaim (e.g., garbage collection), it must clearly define lifetime rules of the object graph and type-safety strategies in separate specifications.

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

2. **regional_fifo** - Queue (FIFO) container that:
   - Implements linked list with queue semantics (first-in-first-out)
   - Satisfies all constraints:
     - Constructed via memory_region
     - No copying/moving
     - Correct YAML serialization
   - Complete container interface:
     - Iteration (begin()/end())
     - Size tracking
     - Comparison (<=>)
   - Efficient operations:
     - enque/enque_front

3. **regional_lifo** - Stack (LIFO) container that:
   - Implements linked list with stack semantics (last-in-first-out)
   - Satisfies all constraints:
     - Constructed via memory_region
     - No copying/moving
     - Correct YAML serialization
   - Complete container interface:
     - Iteration (begin()/end())
     - Size tracking
     - Comparison (<=>)
   - Efficient operations:
     - push/push_back

### YAML Integration (Optional)

YAML integration is provided through a modular system using standalone functions and the `YamlConvertible` concept:

#### Design Principles

- **Separation of Concerns**: Core regional types contain no YAML-specific code
- **Modular Headers**: YAML functionality provided in separate `*_yaml.hh` headers
- **Standalone Functions**: YAML operations implemented as free functions, not member methods
- **Concept-Based**: Type safety enforced through C++20 concepts

#### YamlConvertible Concept

The `YamlConvertible` concept works with standalone functions:

```cpp
template <typename T, typename RT>
concept YamlConvertible = requires(T t, const yaml::Node &node,
                                 memory_region<RT> &mr,
                                 regional_ptr<T> &to_ptr) {
  // Standalone serialization function
  { to_yaml(t) } noexcept -> std::same_as<yaml::Node>;

  // Standalone deserialization forms
  { from_yaml<T>(mr, node) } -> std::same_as<global_ptr<T, RT>>;
  { from_yaml<T>(mr, node, to_ptr) } -> std::same_as<void>;

  // Exception safety guarantees
  requires requires {
    []() {
      try {
        memory_region<RT> mr;
        yaml::Node node;
        regional_ptr<T> to_ptr;
        auto ptr = from_yaml<T>(mr, node);
        from_yaml<T>(mr, node, to_ptr);
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
  to_ptr = from_yaml<T>(mr, node);
}
```

#### Usage Pattern

```cpp
// Core type definition (no YAML dependencies)
#include "my_regional_type.hh"

// Optional YAML support
#include "my_regional_type_yaml.hh"  // Enables YAML for MyRegionalType

// Usage
MyRegionalType obj = ...;
yaml::Node node = to_yaml(obj);                    // Standalone function
auto restored = from_yaml<MyRegionalType>(mr, node); // Template function
```

#### Implementation Structure

YAML support headers typically contain:

1. **Inline Functions**: All YAML logic implemented inline in headers
2. **Template Functions**: Generic `from_yaml<T>()` functions for any memory region type
3. **Concept Verification**: Static assertions to ensure `YamlConvertible` compliance
4. **Complete Functionality**: Full serialization/deserialization with error handling

This modular approach allows users to:
- Use regional types without YAML overhead when not needed
- Selectively enable YAML for specific types
- Maintain clean separation between core functionality and serialization
- Benefit from compile-time optimization of inline implementations

#### Example Implementation

The `CodDep` and `CodProject` types demonstrate this pattern:

**Core Types** (`codp.hh`):
```cpp
class CodDep {
  // Core functionality only - no YAML methods
  UUID uuid_;
  regional_str name_;
  regional_str repo_url_;
  regional_fifo<regional_str> branches_;
  // ... constructors and accessors only
};

class CodProject {
  // Core functionality only - no YAML methods  
  UUID uuid_;
  regional_str name_;
  regional_fifo<CodDep> deps_;
  // ... constructors and accessors only
};
```

**Optional YAML Support** (`codp_yaml.hh`):
```cpp
// Standalone serialization functions
inline yaml::Node to_yaml(const CodDep& dep) noexcept { /* ... */ }
inline yaml::Node to_yaml(const CodProject& project) noexcept { /* ... */ }

// Template deserialization functions
template <typename RT>
global_ptr<CodDep, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) { /* ... */ }

template <typename RT>
global_ptr<CodProject, RT> from_yaml(memory_region<RT> &mr, const yaml::Node &node) { /* ... */ }

// Regional pointer overloads
template <typename RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<CodDep> &to_ptr) { /* ... */ }

template <typename RT>
void from_yaml(memory_region<RT> &mr, const yaml::Node &node, regional_ptr<CodProject> &to_ptr) { /* ... */ }

// Concept verification
static_assert(yaml::YamlConvertible<CodDep, void>);
static_assert(yaml::YamlConvertible<CodProject, void>);
```

This design ensures that:
- Core types remain focused on their primary responsibilities
- YAML functionality is completely optional and modular
- All YAML logic is implemented inline for optimal performance
- The `YamlConvertible` concept provides compile-time type safety
