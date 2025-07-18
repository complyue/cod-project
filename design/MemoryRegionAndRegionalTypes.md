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

### Memory Allocation and Management

The system provides two approaches for memory region allocation and management, with RAII being the recommended approach:

#### **1. RAII Management with auto_region (Recommended)**

The `auto_region<RT>` template provides automatic memory management with zero-cost resource cleanup:

```cpp
template <typename RT>
class auto_region final {
  // RAII wrapper around memory_region<RT>
  // - Movable but not copyable
  // - Automatic deallocation using correct allocator
  // - Convenient access via operator* and operator->
};
```

**Usage Examples:**

```cpp
// Basic construction with default allocator
auto_region<DocumentStore> region(1024 * 1024);  // 1MB
auto store = region->root();

// With constructor arguments
auto_region<DocumentStore> region_with_args(1024 * 1024, "Document Title");

// With custom allocator (allocator as first parameter)
std::pmr::monotonic_buffer_resource pool;
std::pmr::polymorphic_allocator<std::byte> allocator(&pool);
auto_region<DocumentStore> custom_region(allocator, 1024 * 1024);
auto_region<DocumentStore> custom_with_args(allocator, 1024 * 1024, "Title");

// Access the memory region
(*region).create<regional_str>("example");  // operator*
region->create<regional_str>("example");    // operator->

// Move semantics (but no copying)
auto moved_region = std::move(region);  // OK
// auto copied_region = region;         // Compilation error
```

**Benefits:**

- **Automatic cleanup**: No manual memory management required
- **Exception safety**: Resources cleaned up even if exceptions occur
- **Type safety**: Allocator captured in deleter, ensures correct deallocation
- **Ergonomic**: Natural C++ constructor syntax instead of factory methods

#### **2. Manual Management with free_region Methods**

For cases where RAII is not suitable, manual allocation and deallocation methods are provided:

```cpp
// Allocation methods (existing)
template <typename... Args>
static memory_region<RT>* alloc_region(size_t payload_capacity, Args&&... args);

template <typename Allocator, typename... Args>
static memory_region<RT>* alloc_region_with(Allocator allocator, size_t payload_capacity, Args&&... args);

// Deallocation methods (new)
static void free_region(memory_region<RT>* region);

template <typename Allocator>
static void free_region_with(Allocator allocator, memory_region<RT>* region);
```

**Usage Examples:**

```cpp
// Manual management with default allocator
auto* region = memory_region<DocumentStore>::alloc_region(1024 * 1024);
auto store = region->root();
// ... use region ...
memory_region<DocumentStore>::free_region(region);

// Manual management with custom allocator
std::pmr::monotonic_buffer_resource pool;
std::pmr::polymorphic_allocator<std::byte> alloc(&pool);
auto* region = memory_region<DocumentStore>::alloc_region_with(alloc, 1024 * 1024);
auto store = region->root();
// ... use region ...
memory_region<DocumentStore>::free_region_with(alloc, region);
```

**Important Notes:**

- **Allocator matching**: The same allocator instance used for allocation must be used for deallocation
- **Null safety**: Both `free_region` methods safely handle null pointers
- **Manual responsibility**: Developer must ensure proper cleanup to avoid memory leaks

#### **⚠️ Important: Raw `delete` is Forbidden**

Using raw `delete` on memory regions is **forbidden** and will cause assertion failures:

```cpp
// ❌ FORBIDDEN - triggers assertion failure
auto* region = memory_region<DocumentStore>::alloc_region_with(custom_allocator, 1024 * 1024);
delete region;  // ASSERTION FAILURE - prevents undefined behavior from delete on allocator memory
```

**Runtime Protection**: The `memory_region` destructor contains an assertion that prevents direct `delete` usage, ensuring undefined behavior is caught during development. Using `delete` on memory allocated with custom allocators is undefined behavior, not just a memory leak.

#### **Memory Management Best Practices**

1. **Prefer auto_region**: Use `auto_region<RT>` for new code unless there are specific requirements for manual management
2. **Match allocators**: When using manual management, ensure the same allocator is used for allocation and deallocation
3. **Exception safety**: `auto_region` provides automatic cleanup even when exceptions occur
4. **Custom allocators**: Both RAII and manual approaches support custom allocators with proper cleanup
5. **Move semantics**: `auto_region` supports move construction/assignment but prohibits copying

**Allocation Method Comparison:**

| Method                | Cleanup          | Exception Safety | Custom Allocator | Ergonomics       | Status             |
| --------------------- | ---------------- | ---------------- | ---------------- | ---------------- | ------------------ |
| `auto_region<RT>`     | Automatic        | ✅ Yes           | ✅ Yes           | ✅ High          | ✅ **Recommended** |
| Manual `free_region*` | Manual           | ❌ No            | ✅ Yes           | ⚠️ Medium        | ⚠️ Special cases   |
| Raw `delete`          | ❌ **Assertion** | ❌ No            | ❌ **UB**        | ❌ **Forbidden** | ❌ **Forbidden**   |

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
- **In-place construction only**: Elements must be constructed in-place using `emplace_*` methods that accept construction arguments
- **Const reference parameters**: Use const references (`const Args&...args`) to prevent moves from external contexts
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

// ✅ CORRECT - regional type in region using auto_region (recommended)
auto_region<MyRoot> region(1024*1024);
auto title = region->create<regional_str>(*region, "example");  // OK

// ✅ CORRECT - regional type in region using manual management
auto mr = memory_region<MyRoot>::alloc_region(1024*1024);
auto title = mr->create<regional_str>(*mr, "example");  // OK
memory_region<MyRoot>::free_region(mr);  // Manual cleanup required
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
    auto rs = mr.create<regional_str>(*mr, "Hello");
    print_to_c_library(rs->c_str());  // Temporary raw pointer for C interop
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

// Single region context - regional_ptr is optimal (auto_region recommended)
auto_region<Document> region(1024*1024);
auto node1 = region->create<DocumentNode>(*region, "First");
auto node2 = region->create<DocumentNode>(*region, "Second");
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

// Multi-region context - global_ptr maintains region safety (auto_region recommended)
auto_region<Document> doc_region(1024*1024);
auto_region<RefRoot> ref_region(1024*1024);

auto node = doc_region->create<DocumentNode>(*doc_region, "content");
auto ref = ref_region->create<CrossRegionReference>(*ref_region, "reference");
ref->external_ref_ = doc_region->cast_ptr(node.get());  // global_ptr assignment
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

    index_root->entries_.enque(index_mr, *index_entry);
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
auto_region<Document> region(1024*1024);

// Factory function approach - creates new regional_str
auto title = intern_str(*region, "Document Title");
auto content = intern_str(*region, std::string("Content text"));
auto note = intern_str(*region, std::string_view("Note view"));

// In-place construction approach - constructs at pre-allocated location
auto name_storage = region->allocate<regional_str>();  // Pre-allocated storage
intern_str(*region, "Document Name", *name_storage);  // Initialize in-place
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

// ✅ CORRECT - use string_view to avoid allocation
dict.emplace(*mr, std::string_view("temp_key"), "value");

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

4. **iopd<K,V>** – Insertion-order preserving dictionary designed for YAML mappings:

   - **Purpose**: Represents a YAML mapping (`yaml::Map`) while retaining the *original key order* exactly as it appears in the source document.  This is important for human-friendly configuration files where the order of entries conveys meaning or readability.
   - **Internal structure**: Implements a compact two-layer design
     1. `std::vector<entry>` stores `(key,value)` pairs sequentially – this naturally preserves order for iteration and `to_yaml` serialisation.
     2. `std::unordered_map<std::string, size_t>` maps each key to its index inside the vector, giving **O(1)** average-time lookup, insertion, and update without disturbing the insertion order.
   - **YAML support**: `to_yaml(const iopd<V>&)` serialises the container back into a `yaml::Map` with the preserved order, and `from_yaml()` reconstructs the dictionary while respecting regional memory rules.
   - **Use cases**: Configuration files, metadata sections, and any YAML documents where key order must remain stable across parse ⇄ serialise cycles.

### YAML Integration (Optional)

YAML integration is provided through a modular system using standalone functions and the `YamlConvertible` concept:

#### YamlDocument API

The `YamlDocument` class provides a **dual API pattern** with two consistent approaches for different error handling preferences:

**Constructor-based APIs (Exception-throwing)**
- Throw exceptions directly (`ParseError`, `AuthorError`)
- Use when you prefer exception-based error handling
- More concise for simple error propagation

**Static method-based APIs (noexcept)**
- Return `Result` variants (`ParseResult`, `AuthorResult`)
- Use when you prefer explicit error handling via Result types
- Better for functional programming patterns and error composition

### Parsing API

**1. Constructor-based (throws exceptions)**

```cpp
// Parse from string - throws ParseError on failure
YamlDocument doc("config.yaml", yaml_content);

// Parse from file path - throws ParseError on failure
YamlDocument doc("config.yaml", std::ifstream("config.yaml"));
```

**2. Static method-based (noexcept)**

```cpp
// Parse from string - returns ParseResult
ParseResult result = YamlDocument::Parse("config.yaml", yaml_content);

// Parse from file path - returns ParseResult
ParseResult result = YamlDocument::Read("config.yaml");
```

### Authoring API

**1. Constructor-based (throws exceptions)**

```cpp
// Create document with callback - throws AuthorError on failure
YamlDocument doc("output.yaml", [](YamlAuthor& author) {
    auto root = author.createMap();
    author.setMapValue(root, "key", author.createString("value"));
    author.addRoot(root);
}, true, true); // write=true, overwrite=true
```

**2. Static method-based (noexcept)**

```cpp
// Create document with callback - returns AuthorResult
template<typename AuthorCallback>
  requires std::invocable<AuthorCallback, YamlAuthor&>
static AuthorResult Write(std::string filename, AuthorCallback&& callback, bool write = true, bool overwrite = true) noexcept;
```

**Callback Interface:**

The callback parameter must be invocable with a `YamlAuthor&` reference. This includes:
- Lambda functions: `[](YamlAuthor& author) { ... }`
- Function pointers: `void callback(YamlAuthor& author)`
- Function objects with `operator()(YamlAuthor&)`
- Member functions (with appropriate binding)

### Usage Examples

**Create single-document YAML using static method (noexcept)**

```cpp
AuthorResult doc = YamlDocument::Write("generated.yaml", [](YamlAuthor& author) {
    // Create root map
    auto root = author.createMap();
    
    // Add string values (automatically tracked)
    author.setMapValue(root, "name", author.createString("MyApplication"));
    author.setMapValue(root, "version", author.createString("1.0.0"));
    
    // Add nested structures
    auto config = author.createMap();
    author.setMapValue(config, "port", author.createScalar(8080));
    author.setMapValue(config, "debug", author.createScalar(true));
    author.setMapValue(root, "config", config);
    
    // Add sequences
    auto features = author.createSequence();
    author.pushToSequence(features, author.createString("authentication"));
    author.pushToSequence(features, author.createString("logging"));
    author.setMapValue(root, "features", features);
    
    // Add the root to the document
    author.addRoot(root);
});

// Handle result
if (std::holds_alternative<yaml::YamlDocument>(doc)) {
    auto& yaml_doc = std::get<yaml::YamlDocument>(doc);
    std::cout << "Generated YAML successfully" << std::endl;
} else {
    auto& error = std::get<yaml::AuthorError>(doc);
    std::cerr << "Error: " << error.what() << std::endl;
}
```

**Create multi-document YAML using constructor (throws)**

```cpp
try {
    YamlDocument doc("multi.yaml", [](YamlAuthor& author) {
        // First document
        auto doc1 = author.createMap();
        author.setMapValue(doc1, "type", author.createString("config"));
        author.setMapValue(doc1, "version", author.createScalar(1));
        author.addRoot(doc1);
        
        // Second document
        auto doc2 = author.createMap();
        author.setMapValue(doc2, "type", author.createString("data"));
        author.setMapValue(doc2, "items", author.createSequence());
        author.addRoot(doc2);
    });
    
    // Access multiple documents
    bool multi = doc.isMultiDocument();  // true
    size_t count = doc.documentCount();   // 2
    
    auto& first_doc = doc.root(0);
    auto& second_doc = doc.root(1);
    
    std::cout << "Created multi-document YAML" << std::endl;
} catch (const yaml::AuthorError& e) {
    std::cerr << "Failed to create YAML: " << e.what() << std::endl;
}
```

**String Lifetime Management**

The authoring API provides automatic string tracking to ensure all string content remains valid:

- **String Creation**: Use `author.createString()` to create strings that will be owned by the document
- **String Views**: Use `author.createStringView()` to create tracked string views
- **Automatic Tracking**: All strings created through the author are automatically tracked
- **Lifetime Guarantee**: Strings remain valid for the lifetime of the returned `YamlDocument`
- **No Manual Management**: Users don't need to manage string lifetimes manually

```cpp
AuthorResult doc = YamlDocument::Write("config.yaml", [](YamlAuthor& author) {
    auto root = author.createMap();
    
    // These strings are automatically tracked and owned by the document
    author.setMapValue(root, "database_url", author.createString("postgresql://localhost:5432/mydb"));
    author.setMapValue(root, "api_key", author.createString("secret-key-12345"));
    
    // String views are also tracked
    std::string_view key_view = author.createStringView("connection_pool");
    author.setMapValue(root, key_view, author.createScalar(10));
    
    author.addRoot(root);
});

// All strings remain valid as long as 'doc' exists
auto root = doc.value().root();
std::string_view db_url = root["database_url"].asString();  // Safe to use
```

**Node Creation Methods**

The `YamlAuthor` class provides methods for creating different node types:

```cpp
// String creation methods
auto str_node = author.createString("text value");
auto str_from_view = author.createString(std::string_view("view"));
auto str_from_cstr = author.createString("c-string");

// String view creation methods (tracked by author)
std::string_view tracked_view = author.createStringView("tracked text");
std::string_view view_from_str = author.createStringView(std::string("from string"));

// Scalar creation methods
auto bool_node = author.createScalar(true);
auto int_node = author.createScalar(42);
auto int64_node = author.createScalar(int64_t(1000000));
auto float_node = author.createScalar(3.14f);
auto double_node = author.createScalar(3.14159);

// Container creation methods
auto map_node = author.createMap();
auto sequence_node = author.createSequence();
```

**Node Manipulation Methods**

The `YamlAuthor` provides methods for modifying nodes during authoring:

```cpp
// Map manipulation
auto map = author.createMap();
author.setMapValue(map, "key", author.createString("value"));

// Sequence manipulation
auto sequence = author.createSequence();
author.pushToSequence(sequence, author.createString("item1"));
author.pushToSequence(sequence, author.createString("item2"));

// Node assignment
auto target = author.createMap();
auto source = author.createString("copied value");
author.assign_node(target, source);  // target now contains the string value
```

**Document Access Methods**

The `YamlDocument` class provides methods for accessing document content:

```cpp
// Single document access
const Node& root = doc.root();               // Access first/only document
Node& mutable_root = doc.root();             // Mutable access

// Multi-document access
const Node& first = doc.root(0);             // Access by index
const Node& second = doc.root(1);            // Access second document
bool is_multi = doc.isMultiDocument();       // Check if multi-document
size_t count = doc.documentCount();          // Get document count

// Access all documents
const std::vector<Node>& all_docs = doc.documents();
std::vector<Node>& mutable_docs = doc.documents();
```

**Error Handling**

The API provides comprehensive error handling through both exception and Result patterns:

```cpp
// Static method approach (noexcept)
ParseResult parse_result = YamlDocument::Parse("config.yaml", yaml_content);
if (std::holds_alternative<yaml::ParseError>(parse_result)) {
    auto& error = std::get<yaml::ParseError>(parse_result);
    std::cerr << "Parse error: " << error.what() << std::endl;
    return;
}

// Constructor approach (throws)
try {
    YamlDocument doc("config.yaml", yaml_content);
    // Use document...
} catch (const yaml::ParseError& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}

// Authoring with static method (noexcept)
AuthorResult author_result = YamlDocument::Write("output.yaml", [](YamlAuthor& author) {
    auto root = author.createMap();
    author.addRoot(root);
});

if (std::holds_alternative<yaml::AuthorError>(author_result)) {
    auto& error = std::get<yaml::AuthorError>(author_result);
    std::cerr << "Authoring error: " << error.what() << std::endl;
    return;
}

// Authoring with constructor (throws)
try {
    YamlDocument doc("output.yaml", [](YamlAuthor& author) {
        auto root = author.createMap();
        author.addRoot(root);
    });
    // Use document...
} catch (const yaml::AuthorError& e) {
    std::cerr << "Authoring error: " << e.what() << std::endl;
}
```

**File I/O Integration**

The authoring API supports automatic file writing:

```cpp
// Write to file automatically (default behavior)
AuthorResult doc = YamlDocument::Write("output.yaml", [](YamlAuthor& author) {
    auto root = author.createMap();
    author.setMapValue(root, "generated", author.createScalar(true));
    author.addRoot(root);
});

// Control file writing behavior
AuthorResult doc_no_write = YamlDocument::Write(
    "output.yaml",
    [](YamlAuthor& author) {
        auto root = author.createMap();
        author.addRoot(root);
    },
    false  // write = false, don't write to file
);

// Control overwrite behavior
AuthorResult doc_no_overwrite = YamlDocument::Write(
    "output.yaml",
    [](YamlAuthor& author) {
        auto root = author.createMap();
        author.addRoot(root);
    },
    true,   // write = true
    false   // overwrite = false, fail if file exists
);
```

**API Comparison**

| Feature | Exception-throwing flavor | noexcept flavor |
|---------|--------------------------|-----------------|
| **Error Handling** | Throws exceptions | Returns Result types |
| **Error Expectation** | Errors are unusual/unexpected | Errors are expected/common |
| **Use Cases** | Trusted input, config files | User input, validation, data processing |
| **Performance** | Slightly faster | Slightly slower |
| **Safety** | Exception-safe | noexcept safe |
| **Parsing** | `YamlDocument(filename, source)` | `YamlDocument::Parse(filename, source)` |
| **File Reading** | Manual file I/O + constructor | `YamlDocument::Read(filepath)` |
| **Authoring** | `YamlDocument(filename, callback, ...)` | `YamlDocument::Write(filename, callback, ...)` |
| **Multi-Document** | ✅ Supported | ✅ Supported |
| **String Lifetime** | Managed by document | Managed by document |

**Exception-throwing flavor scenarios:**
- Loading application configuration files (malformed config is a bug)
- Processing internal data structures (errors indicate programming issues)
- Trusted input where validation has already occurred
- Performance-critical paths where exceptions are truly exceptional

**noexcept flavor scenarios:**
- Processing user-provided YAML files (malformed input is expected)
- Data validation and sanitization workflows
- Batch processing where some files may be invalid
- Interactive tools where graceful error reporting is important
- Integration with Result-based error handling patterns

#### Design Principles

- **Separation of Concerns**: Core regional types contain no YAML-specific code
- **Modular Headers**: YAML functionality provided in separate `*_yaml.hh` headers
- **Standalone Functions**: YAML operations implemented as free functions, not member methods
- **Concept-Based**: Type safety enforced through C++20 concepts

#### YamlConvertible Concept

The `YamlConvertible` concept defines the minimal API a type **T** must expose to participate in the YAML serialization subsystem.  
A type that satisfies the concept can be **both** converted **to** a `yaml::Node` _and_ reconstructed **from** a `yaml::Node` inside any `memory_region<RT>`:

```cpp
template <typename T, typename RT>
concept YamlConvertible = requires(T t,
                                   const yaml::Node &node,
                                   memory_region<RT> &mr,
                                   T *raw_ptr) {
  // Serialization – pure function, must not throw
  { to_yaml(t) } noexcept -> std::same_as<yaml::Node>;

  // Deserialization – in-place construction at *raw_ptr
  { from_yaml<T>(mr, node, raw_ptr) } -> std::same_as<void>;
};
```

Key points:
- `to_yaml` must be a **noexcept** free function that returns a fully-formed `yaml::Node`.
- `from_yaml` is a free function template that performs **in-place** deserialization at the uninitialised memory pointed to by `raw_ptr`.  
  This design avoids copy / move operations that regional types forbid.

#### Region Instance Methods: `create_from_yaml` & `create_from_yaml_at`

`memory_region<RT>` now provides two **instance methods** that make it trivial to allocate and initialise regional objects directly from YAML data:

```cpp
template <typename T>
  requires YamlConvertible<T, RT>
global_ptr<T, RT> memory_region<RT>::create_from_yaml(const yaml::Node &node);

template <typename T>
  requires YamlConvertible<T, RT>
void memory_region<RT>::create_from_yaml_at(const yaml::Node &node,
                                            regional_ptr<T> &target);
```

Usage guidelines:

1. **create_from_yaml**  
   - Allocates a **new** object of type `T` inside the region (`*this`).  
   - Deserialises the object from `node` using the `from_yaml` free-function.  
   - Returns a `global_ptr<T,RT>` so the caller can store or pass the reference immediately.

   ```cpp
   yaml::Node doc = yaml::LoadFile("invoice.yaml");
   auto invoice = region->create_from_yaml<Invoice>(doc);
   ledger->entries_.push_back(invoice);
   ```

2. **create_from_yaml_at**  
   - Intended for **pre-allocated storage** (e.g. container slot or struct field).  
   - Performs allocation and in-place deserialization, then assigns the resulting pointer to the supplied `regional_ptr`.

   ```cpp
   regional_ptr<Invoice> slot;
   region->create_from_yaml_at<Invoice>(doc, slot);  // slot now references the deserialised Invoice
   ```

Both methods rely on the lower-level `from_yaml` free function, guaranteeing that all allocations happen inside the correct `memory_region` and that no forbidden copy / move operations occur.

These instance helpers, combined with the `YamlConvertible` concept, provide an ergonomic yet fully region-safe bridge between YAML documents and regional object graphs.

#### Implementation Structure

YAML support headers typically contain:

1. **Inline Functions**: All YAML logic implemented inline in headers
2. **Template Functions**: Generic `from_yaml<T>()` functions for any memory region type
3. **Complete Functionality**: Full serialization/deserialization with error handling

This modular approach allows users to:

- Use regional types without YAML overhead when not needed
- Selectively enable YAML for specific types
- Maintain clean separation between core functionality and serialization
- Benefit from compile-time optimization of inline implementations

**YAML Container Support**: YAML deserialization supports containers holding both bits types and regional types.  **Mapping nodes** are deserialised into `iopd<V>` instances, guaranteeing that the *user-defined order of keys* is preserved on round-trip serialisation.  Internally the container combines a vector (for order) with a hash index (for O(1) look-ups), so there is no performance trade-off when accessing elements programmatically.

The implementation leverages the fixed-size and RTTI-free constraints of regional types to:

- Allocate uninitialized memory at final container locations
- Use the raw pointer version `from_yaml(mr, node, T* raw_ptr)` for direct in-place construction
- Avoid copy/move operations that would violate regional type constraints

#### Example Implementation – Step-by-Step Guide

The following walk-through demonstrates how to add YAML support for a **custom regional
struct** as well as a **regional_vector** container that stores both *bits* types and
other regional types.  The pattern is universally applicable – simply swap field
definitions or container choices to match your own data model.

```cpp
// Example domain object – a simple “Book”
struct Book {
  static const UUID TYPE_UUID;              // ← mandatory root identifier

  regional_str               title_;
  regional_vector<int>       ratings_;      // bits-type elements (1-5)
  regional_vector<regional_str> authors_;   // nested regional types

  // NOTE: construction *must* take a memory_region reference.
  template <typename RT>
  Book(memory_region<RT> &mr,
       std::string_view t = {})
      : title_(mr, t), ratings_(mr), authors_(mr) {}
};

const UUID Book::TYPE_UUID = UUID("12345678-9abc-def0-1234-56789abcdef0");
```

### 1.  Implement `to_yaml(const Book&) noexcept`

```cpp
inline yaml::Node to_yaml(const Book &b) noexcept {
  yaml::Node n(yaml::Map{});
  n["title"]   = std::string_view(b.title_);
  n["ratings"] = to_yaml(b.ratings_);   // vector<int> → sequence
  n["authors"] = to_yaml(b.authors_);   // vector<regional_str> → recursive call
  return n;
}
```

Key points:
* Prefer **composition** – delegate to `to_yaml` of sub-objects / containers.
* When a field is a *bits* scalar (e.g. `int`, `bool`) no special code is needed –
  the `yaml::Node` constructor handles it.

### 2.  Implement `from_yaml<Book>()`

```cpp
template <typename RT>
  requires ValidMemRegionRootType<RT>
void from_yaml(memory_region<RT> &mr,
               const yaml::Node &node,
               Book *raw_ptr) {

  // Basic validation (defensive programming encouraged)
  if (!node.IsMap())
    throw yaml::TypeError("Book YAML must be a map");

  // 1️⃣  Construct the outer object *first* (in-place)
  new (raw_ptr) Book(mr);
  Book &b = *raw_ptr;

  // 2️⃣  Load simple scalar field
  b.title_ = intern_str(mr, node["title"].as<std::string_view>());

  // 3️⃣  Use helper for the container with *bits* elements
  b.ratings_.clear();
  from_yaml(mr, node["ratings"], &b.ratings_);    // resolved by ADL to vector_yaml.hh

  // 4️⃣  Use helper for the container with *regional* elements
  b.authors_.clear();
  from_yaml(mr, node["authors"], &b.authors_);    // works recursively
}
```

Implementation hints:
1. **Always construct the outer object before touching its fields** – this guarantees
   valid lifetimes when sub-objects reference `*this`.
2. Reuse the **generic container deserialisers** (`vector_yaml.hh`, etc.).  They
   automatically handle:
   • bits elements (`int`, `double`, `bool`, …)
   • nested regional elements (inc. other containers)
   • recursive graphs (vector<vector<…>>)
3. For bits fields you may directly assign the result of `node.as<T>()` or rely on
   the container helper as shown.

### 3.  Register convenience wrappers (optional)

Nothing extra is required: because `Book` now satisfies `YamlConvertible` the
following helpers work out-of-the-box:

```cpp
auto my_book = region->create_from_yaml<Book>(yaml::Load(file_text));

regional_ptr<Book> slot;
region->create_from_yaml_at<Book>(node, slot);
```

### 4.  Container Authoring Checklist

When building **your own regional container** (queue, map, graph, …) that should
participate in YAML:

1. **Expose a callback-based insertion API** similar to
   `regional_vector::emplace_init(memory_region<RT>&, Fn&&)`.  This enables
   deserialisers to construct elements *in-place* without temporary objects.
2. **For bits element types** provide a fast-path that accepts a scalar `yaml::Node`
   and calls the plain `elem_node.as<T>()` conversion.
3. **For regional element types** delegate to their own `from_yaml` inside the
   `emplace_init` callback.
4. **Avoid private-member access in deserialisers** – use the public insertion
   API you exposed.

Following these guidelines guarantees that the container remains free of
copy/move operations and can happily nest within itself (`vector<vector<…>>`) or
other regional structures.

---

With these patterns in place you can equip any regional type – from simple
value objects to arbitrarily deep container graphs – with safe, zero-copy YAML
serialisation while preserving the fundamental invariants of the shilos memory
model.

## Lvalue-Only Constraint for Regional Types

- **regional_str and all regional types must only be used as lvalues.**
- Creating temporary instances of regional types is strictly forbidden.
- The reason is that the lifetime and memory management of regional types are entirely controlled by their owning memory_region. Temporary objects would exist outside of any region's control, breaking the unique ownership and lifetime safety guarantees.
- Typical usage:

```cpp
auto key = mr.create<regional_str>(mr, "key1");
dict.insert(mr, *key, value); // Correct: *key is an lvalue

dict.insert(mr, regional_str(mr, "key1"), value); // Incorrect: attempting to create regional_str temporary
```

**Always use regional type variables that are properly allocated in memory regions. Never attempt to create temporary regional type objects.**

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
