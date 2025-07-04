# Memory Region and Regional Types Specification

This document defines the requirements and implementation details for memory regions and regional types in the system.

## Current Stage Strategy and Future Plans

### Current C++ Implementation Phase

This specification represents the current stage of shilos development, implemented in C++20 to establish the foundational design and prove the zero-cost-relocation concept. During this phase:

- **Correctness Over Ergonomics**: The current implementation prioritizes correctness and zero-cost-relocation guarantees over developer ergonomics. Certain conveniences (e.g., stack allocation of regional types, simple pointer arithmetic) are intentionally restricted to maintain the core memory safety properties.

- **C++20 Constraints**: The implementation leverages modern C++20 features while working within the language's constraints. Regional types must follow strict construction rules, lifetime management, and pointer semantics that may feel restrictive compared to standard C++ patterns.

- **Design Validation**: This C++ implementation serves as a proof-of-concept and design validation tool, ensuring the memory region and regional type concepts are sound before developing a dedicated programming language.

### Future Programming Language Development

The long-term vision includes developing a new compiled programming language specifically designed for shilos programs:

- **Native Regional Types**: The new language will provide native support for regional types with improved ergonomics, eliminating many of the current C++ constraints while maintaining zero-cost-relocation guarantees.

- **Simplified Syntax**: Regional type allocation, construction, and pointer usage will be significantly simplified compared to the current C++ implementation.

- **Compiler Optimizations**: The dedicated compiler will be able to optimize regional type usage patterns that are currently difficult to express or optimize in C++.

- **Enhanced Safety**: Language-level guarantees will provide stronger compile-time safety for regional type constraints.

### Transition Strategy

The current C++ implementation serves as:

- A reference implementation for the memory region concept
- A testing ground for regional type design decisions
- A foundation for the future language specification
- A practical tool for early adopters and experimental projects

Developers working with the current C++ implementation should understand that certain ergonomic limitations are intentional design choices that prioritize the core zero-cost-relocation feature. These limitations will be addressed in the future dedicated language while maintaining the fundamental memory safety and performance guarantees.

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
- **Regional types** must satisfy additional structural constraints:
  - **Fixed storage size**: Compile-time deterministic memory layout with no dynamic allocation
  - **RTTI-free**: No virtual functions, virtual destructors, or dynamic dispatch mechanisms
  - **Plain data semantics**: Memory layout must be predictable for safe uninitialized allocation
- Members cannot contain external pointers of any kind
- Members containing internal pointers must:
  - Use only `regional_ptr` (raw pointers including `global_ptr` are prohibited)
  - Point only to bits types or compliant regional types

### 2. Construction Rules

- **Required**: Every regional type must provide at least one constructor that accepts `memory_region&` as its first parameter
- **Optional**: Regional types may provide default constructors (constructors with no parameters)
  - When default constructors are provided, they must initialize all (direct and indirect) `regional_ptr` members to null without performing any allocation
  - Default constructors are not required - many regional types will only provide `memory_region&`-accepting constructors
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

**Current Stage Design Rationale**: The prohibition of copy/move operations is a fundamental design choice that enables zero-cost-relocation. While this restriction may seem limiting compared to standard C++ containers, it ensures that object graphs can be relocated in memory without any pointer updates. The future dedicated programming language will provide more ergonomic patterns for object construction and manipulation while maintaining these core relocation guarantees.

### 4. Container Element Rules

Regional container types (like `regional_vector`, `regional_fifo`, `regional_lifo`, `regional_dict`) have additional constraints:

- **No copy/move insertion**: Container methods must never accept elements via copy or move construction
- **In-place construction only**: Elements must be constructed in-place using `emplace_*` methods that forward construction arguments
- **Template forwarding**: Use perfect forwarding to pass construction arguments directly to element constructors
- **Memory region threading**: Always pass the `memory_region&` to element constructors when required

**Current Stage Implementation Note**: The requirement for in-place construction using `emplace_*` methods is more verbose than standard C++ container insertion patterns. This design choice ensures that all regional type constraints are satisfied while maintaining zero-cost-relocation. The future dedicated programming language will provide more intuitive syntax for container operations while preserving these fundamental safety properties.

### 5. YAML Serialization (Optional)

YAML serialization support is optional and modular for regional types:

- **Opt-in Design**: Regional types do not require built-in YAML methods
- **Standalone Functions**: YAML support provided via standalone template functions in separate headers
- **Modular Inclusion**: Users include specific `*_yaml.hh` headers to enable YAML for desired types
- **Concept Compliance**: When YAML support is included, types must satisfy `YamlConvertible` concept

### 6. Pointer Implementation Details

The system implements three pointer types with distinct storage and relocation semantics:

#### **`regional_ptr<T>` (Offset-Based Storage)**

- **Storage**: Relative offset from its own memory address (8 bytes)
- **Relocation**: Automatic - offsets remain valid when region memory is remapped
- **Performance**: Zero-cost relocation, optimal for single-region programs
- **Construction**: Can be constructed from raw pointers to calculate offsets
- **Constraint**: Cannot be used with rvalue semantics due to address-relative storage

#### **`global_ptr<T,RT>` (Region-Safe Storage)**

- **Storage**: Region identifier + offset (16 bytes)
- **Relocation**: Automatic - maintains region safety across memory remapping
- **Performance**: 2x space cost, required for multi-region programs
- **Lifetime**: Bound to the referenced `memory_region<RT>`
- **Purpose**: Safe cross-region references with explicit region tracking

#### **Raw Pointers (Temporary References)**

- **Storage**: Standard C++ pointer (8 bytes)
- **Relocation**: Manual - developer responsibility to handle region remapping
- **Usage**: Temporary parameter passing, C++ interop, offset calculation
- **Restriction**: Must not be stored persistently in regional types

#### **Memory Safety Guarantees**

- **Root Type Responsibility**: The root type (`RT`) of `memory_region<RT>` defines memory and type safety semantics
- **Simple Strategy**: No memory reclaim - regional objects never deallocated or change type
- **Advanced Strategy**: If root type supports memory reclaim (e.g., garbage collection), it must define lifetime rules and type-safety strategies in separate specifications

## Allocation and Pointer Semantics for Shilos Programs

### Memory Placement Rules

Shilos programs must follow strict allocation rules based on type classification:

#### **Bits Types (Stack/Register Allocation)**

- **Allowed**: Stack allocation, register allocation, and standard C++ placement
- **Examples**: `int`, `float`, `bool`, `char`, `UUID`, plain structs with no pointers
- **Usage**: Normal C++ semantics apply - can be passed by value, stored in local variables, etc.

#### **Regional Types (Region-Only Allocation)**

- **Prohibited**: Stack allocation, register allocation, static allocation
- **Required**: Must reside in a `memory_region<RT>`
- **Reason**: `regional_ptr` uses address-relative storage requiring stable heap addresses
- **Current Stage Trade-off**: This restriction is a deliberate design choice in the current C++ implementation phase. While it reduces ergonomics compared to standard C++ patterns, it ensures zero-cost-relocation guarantees. The future dedicated programming language will provide more ergonomic alternatives while maintaining these core safety properties.
- **Examples**: `regional_str`, `regional_fifo<T>`, `regional_dict<K,V>`, user-defined regional types

```cpp
// ✅ CORRECT - bits type on stack
int counter = 42;
UUID doc_id = generate_uuid();

// ❌ INCORRECT - regional type on stack
regional_str title("example");  // COMPILATION ERROR

// ✅ CORRECT - regional type in region
auto mr = memory_region<MyRoot>::alloc_region(1024*1024);
regional_str title(*mr, "example");  // OK
```

### Pointer Type Selection Rules

Shilos programs must choose appropriate pointer types based on usage context:

#### **Raw Pointers (C++ Interop Only)**

- **Purpose**: Interoperability with non-shilos C++ code
- **Restrictions**:
  - Must not be stored in regional types
  - Must not outlive the owning `memory_region`
  - Used only for temporary parameter passing and return values
- **Examples**: FFI boundaries, C library integration, temporary references

```cpp
// ✅ CORRECT - temporary interop usage
void print_to_c_library(const char* text) {
    regional_str rs(*mr, "Hello");
    print_to_c_library(rs.c_str());  // Temporary raw pointer for C interop
}

// ❌ INCORRECT - storing raw pointer in regional type
struct BadRegionalType {
    const char* stored_ptr;  // VIOLATES REGIONAL TYPE CONSTRAINTS
};
```

#### **Regional Pointers (Single-Region Programming)**

- **Purpose**: Intra-region references in single-region shilos code
- **Advantages**: Zero-cost relocation, compact storage (offset-based)
- **Context**: When all referenced objects are in the same memory region
- **Usage**: Default choice for single-region programs

```cpp
// ✅ CORRECT - single-region code using regional_ptr
struct DocumentNode {
    regional_str content_;
    regional_ptr<DocumentNode> next_;

    DocumentNode(memory_region<Document>& mr, std::string_view content)
        : content_(mr, content), next_() {}
};

// Single region context - regional_ptr is optimal
auto mr = memory_region<Document>::alloc_region(1024*1024);
auto node1 = mr->create<DocumentNode>(*mr, "First");
auto node2 = mr->create<DocumentNode>(*mr, "Second");
node1->next_ = node2.get();  // regional_ptr assignment
```

#### **Global Pointers (Multi-Region Programming)**

- **Purpose**: Cross-region references in multi-region shilos code
- **Cost**: 2x space overhead for region disambiguation
- **Context**: When referenced objects may be in different memory regions
- **Usage**: Required for multi-region programs to maintain correctness

```cpp
// ✅ CORRECT - multi-region code using global_ptr
struct CrossRegionReference {
    regional_str name_;
    global_ptr<DocumentNode, Document> external_ref_;  // Cross-region reference

    template<typename RT>
    CrossRegionReference(memory_region<RT>& mr, std::string_view name)
        : name_(mr, name), external_ref_() {}
};

// Multi-region context - global_ptr maintains region safety
auto doc_mr = memory_region<Document>::alloc_region(1024*1024);
auto ref_mr = memory_region<RefRoot>::alloc_region(1024*1024);

auto node = doc_mr->create<DocumentNode>(*doc_mr, "content");
auto ref = ref_mr->create<CrossRegionReference>(*ref_mr, "reference");
ref->external_ref_ = doc_mr->cast_ptr(node.get());  // global_ptr assignment
```

### Programming Model Guidelines

Shilos programs can contain both single-region and multi-region code blocks. Code blocks are classified based on their memory access patterns:

#### **Single-Region Code Blocks (Optimal Performance)**

- **Definition**: Code blocks that access only one memory region and never reference objects outside that region
- **Benefits**: Optimal performance, simpler pointer semantics, zero-cost relocation
- **Pointer Choice**: Use `regional_ptr` for all references within the block
- **Pattern**: Function operates entirely within one `memory_region<RT>` context

```cpp
// ✅ Single-region code block - only accesses doc_mr
void process_document_content(memory_region<Document>& doc_mr) {
    auto root = doc_mr.root();
    auto node = doc_mr.create<DocumentNode>(doc_mr, "content");
    root->first_node_ = node.get();  // regional_ptr assignment

    // All operations within doc_mr - single-region semantics
    traverse_nodes(root->first_node_);  // regional_ptr navigation
}
```

#### **Multi-Region Code Blocks (Cross-Region Operations)**

- **Definition**: Code blocks that access multiple memory regions or reference objects across regions
- **Requirements**: Must use `global_ptr` for cross-region references
- **Use Cases**: Data migration, cross-system integration, composite operations
- **Complexity**: Requires explicit region lifetime management and disambiguation

```cpp
// ✅ Multi-region code block - accesses both doc_mr and index_mr
void index_document(memory_region<Document>& doc_mr,
                   memory_region<Index>& index_mr) {
    auto doc_root = doc_mr.root();
    auto index_root = index_mr.root();

    // Cross-region reference requires global_ptr
    auto doc_ref = doc_mr.cast_ptr(doc_root.get());
    auto index_entry = index_mr.create<IndexEntry>(index_mr, "doc_key");
    index_entry->document_ref_ = doc_ref;  // global_ptr assignment

    index_root->entries_.enque(index_mr, std::move(index_entry));
}
```

#### **Hybrid Program Architecture**

- **Reality**: Most shilos programs contain both single-region and multi-region code blocks
- **Strategy**: Maximize single-region blocks for performance, use multi-region blocks only when necessary
- **Optimization**: Design APIs to accept single memory regions when possible

```cpp
// ✅ Hybrid program - mix of single-region and multi-region blocks
class DocumentProcessor {
    // Single-region operations (optimal performance)
    void format_document(memory_region<Document>& doc_mr) {
        // Single-region block - all operations in doc_mr
        auto root = doc_mr.root();
        apply_formatting(root->content_);  // regional_ptr operations
    }

    // Multi-region operations (when cross-region access needed)
    void backup_document(memory_region<Document>& doc_mr,
                        memory_region<Backup>& backup_mr) {
        // Multi-region block - references across regions
        auto doc_ref = doc_mr.cast_ptr(doc_mr.root().get());
        auto backup_entry = backup_mr.create<BackupEntry>(backup_mr);
        backup_entry->source_document_ = doc_ref;  // global_ptr
    }
};
```

#### **Code Block Classification Guidelines**

- **Single-Region Block**: Function/method parameters include only one `memory_region<RT>&`
- **Multi-Region Block**: Function/method parameters include multiple `memory_region<RT>&` parameters
- **Boundary Analysis**: Determine pointer requirements by analyzing memory access patterns
- **Performance Impact**: Single-region blocks have zero-cost relocation, multi-region blocks have 2x pointer overhead

### Pointer Semantics Summary

| Pointer Type       | Storage Cost | Relocation | Use Case      | Restrictions                                 |
| ------------------ | ------------ | ---------- | ------------- | -------------------------------------------- |
| `T*` (raw)         | 8 bytes      | Manual     | C++ interop   | Temporary only, no storage in regional types |
| `regional_ptr<T>`  | 8 bytes      | Automatic  | Single-region | Target must be in same region                |
| `global_ptr<T,RT>` | 16 bytes     | Automatic  | Multi-region  | 2x space cost, cross-region safe             |

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
   - Convenience functions via `intern_str` overloads:
     - In-place construction from external strings
     - Factory functions returning `global_ptr<regional_str, RT>`

#### String Internalization Functions

The `intern_str` template functions provide convenient conversion from standard C++ string types to `regional_str` objects. These functions come in two groups:

**In-Place Construction Overloads** (Using placement new):

```cpp
template <typename RT>
void intern_str(memory_region<RT> &mr, const std::string &external_str, regional_str &str);

template <typename RT>
void intern_str(memory_region<RT> &mr, std::string_view external_str, regional_str &str);

template <typename RT>
void intern_str(memory_region<RT> &mr, const char *external_str, regional_str &str);
```

- **Purpose**: Construct `regional_str` objects in-place at pre-allocated memory locations
- **Usage**: When you have already allocated space for a `regional_str` and want to initialize it
- **Implementation**: Uses placement new to construct the object without additional allocation
- **Benefit**: Avoids extra allocation steps when the target location is already known

**Factory Function Overloads** (Returning global_ptr):

```cpp
template <typename RT>
global_ptr<regional_str, RT> intern_str(memory_region<RT> &mr, const std::string &external_str);

template <typename RT>
global_ptr<regional_str, RT> intern_str(memory_region<RT> &mr, std::string_view external_str);

template <typename RT>
global_ptr<regional_str, RT> intern_str(memory_region<RT> &mr, const char *external_str);
```

- **Purpose**: Allocate and construct new `regional_str` objects, returning a safe pointer
- **Usage**: When you need to create a new `regional_str` from external string data
- **Implementation**: Uses `memory_region<RT>::create<regional_str>()` for safe allocation and construction
- **Benefit**: Single-step string internalization with automatic memory management

**String Type Support**: All `intern_str` overloads accept three common C++ string representations:

- `const std::string&` - Standard string objects
- `std::string_view` - Lightweight string views (optimal for performance)
- `const char*` - C-style null-terminated strings

**Usage Examples**:

```cpp
auto mr = memory_region<Document>::alloc_region(1024*1024);

// Factory function approach - creates new regional_str
auto title = intern_str(*mr, "Document Title");
auto content = intern_str(*mr, std::string("Content text"));
auto note = intern_str(*mr, std::string_view("Note view"));

// In-place construction approach - constructs at pre-allocated location
regional_str name_storage;  // Uninitialized storage
intern_str(*mr, "Document Name", name_storage);  // Initialize in-place
```

**Design Rationale**: The `intern_str` functions bridge the gap between standard C++ string handling and regional type constraints. They provide ergonomic alternatives to direct `regional_str` constructor calls while maintaining compliance with all regional type requirements. The dual interface (in-place vs. factory) accommodates different memory management patterns common in shilos programs.

**Important Usage Guidelines**: 

- **Never use `intern_str` for temporary purposes**: The `intern_str` functions allocate memory in the region and should only be used when you need to store the string persistently. For temporary string operations, use standard C++ string types or `std::string_view`.

- **Use for persistent storage only**: `intern_str` is designed for creating `regional_str` objects that will be stored in regional data structures, not for temporary string manipulation.

- **Prefer direct construction for simple cases**: When you have a simple string literal or known string value, prefer direct `regional_str` construction over `intern_str` for better performance and clarity.

- **Use `std::string_view` for C string conversion**: Avoid direct `const char*` usage in regional type APIs. Convert C strings to `std::string_view` once and reuse the view for multiple operations. This provides better performance and clearer intent.

```cpp
// ❌ INCORRECT - using intern_str for temporary purposes
auto key = intern_str(*mr, "temp_key");  // Unnecessary allocation
dict.emplace(*mr, key, "value");

// ❌ INCORRECT - direct const char* usage
dict.emplace(*mr, "temp_key", "value");  // May cause multiple conversions

// ✅ CORRECT - convert once and reuse std::string_view
std::string_view key_view = "temp_key";
dict.emplace(*mr, key_view, "value");
dict.contains(key_view);  // Reuse the same view

// ✅ CORRECT - direct construction for simple cases
dict.emplace(*mr, regional_str(*mr, "temp_key"), "value");

// ✅ CORRECT - intern_str for persistent storage
auto title = intern_str(*mr, "Document Title");  // Will be stored long-term
document->title_ = title;
```

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
  { from_yaml<T>(mr, node, raw_ptr) } -> std::same_as<void>;

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

**YAML Container Support**: YAML deserialization supports containers holding both bits types and regional types. The implementation leverages the fixed-size and RTTI-free constraints of regional types to:

- Allocate uninitialized memory at final container locations
- Use the raw pointer version `from_yaml(mr, node, T* raw_ptr)` for direct in-place construction
- Avoid copy/move operations that would violate regional type constraints

The raw pointer approach works for simple container scenarios. Complex nested cases (like containers of regional types within other regional types) may still require refinement to achieve full compliance with regional type constraints.

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

## Lvalue-Only Constraint for Regional Types

- **regional_str and all regional types must only be used as lvalues.**
- Passing, returning, or using regional types as rvalues (temporaries) is strictly forbidden.
- The reason is that the lifetime and memory management of regional types are entirely controlled by their owning memory_region. Rvalue semantics would break the unique ownership and lifetime safety guarantees.
- Typical usage:

```cpp
regional_str key(mr, "key1");
dict.insert(mr, key, value); // Correct: key is an lvalue

dict.insert(mr, regional_str(mr, "key1"), value); // Incorrect: regional_str is an rvalue
```

**Always use regional type variables as lvalues in all APIs and tests. Avoid any rvalue usage.**

## Summary and Future Direction

### Current Implementation Philosophy

The shilos system, as implemented in C++20, represents a deliberate trade-off between ergonomics and correctness. The design choices made in this specification prioritize:

1. **Zero-Cost Relocation**: The ability to relocate entire object graphs in memory without pointer updates
2. **Memory Safety**: Strong compile-time guarantees about memory layout and lifetime management
3. **Correctness**: Ensuring that the core concepts are sound and well-tested

These priorities come at the cost of reduced ergonomics compared to standard C++ patterns. Developers working with the current implementation should expect:

- More verbose construction patterns for regional types
- Restrictions on stack allocation and copy/move operations
- Explicit memory region management
- Template-heavy interfaces for type safety

### Long-Term Vision

The current C++ implementation serves as a foundation for a future dedicated programming language that will:

- Provide native syntax for regional types and memory regions
- Eliminate many of the current ergonomic limitations
- Offer stronger compile-time safety guarantees
- Enable more sophisticated compiler optimizations
- Maintain the core zero-cost-relocation property

### Development Strategy

The transition from C++ implementation to dedicated language will be guided by:

- **Experience Gained**: Lessons learned from real-world usage of the C++ implementation
- **Design Validation**: Confirmation that the memory region concept scales to complex applications
- **Performance Verification**: Proof that zero-cost-relocation delivers the expected benefits
- **Community Feedback**: Input from developers using the current implementation

Until the dedicated language is available, the C++ implementation provides a practical tool for exploring the shilos programming model and building applications that benefit from zero-cost-relocation capabilities.
