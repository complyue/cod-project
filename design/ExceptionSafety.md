# Exception Safety Guarantees for Shilos Memory Regions

## Overview

This document specifies the exception safety guarantees provided by the shilos memory region system. These guarantees ensure that the system maintains memory safety and prevents resource leaks even when exceptions occur.

## Exception Safety Levels

### 1. Strong Guarantee (RAII Classes)

**`auto_region<RT>`** provides the **strong exception safety guarantee**:
- If any operation throws an exception, the system state remains unchanged
- No memory leaks occur
- The region is properly cleaned up via RAII

**Guarantees:**
- Constructor: If construction fails, no memory is allocated
- Move operations: If move construction/assignment throws, source remains valid
- Destructor: Never throws, always completes cleanup

### 2. Basic Guarantee (Manual Management)

**`memory_region<RT>`** manual management provides the **basic exception safety guarantee**:
- Resources are not leaked
- Invariants are maintained
- System remains in a valid state

**Guarantees:**
- `alloc_region`: If allocation fails, returns nullptr (no partial allocation)
- `free_region`: Never throws, safely handles null pointers
- All allocation methods: Throw `std::bad_alloc` on allocation failure

### 3. No-Throw Guarantee (Core Operations)

**Pointer operations** provide the **no-throw guarantee**:
- `regional_ptr` operations never throw
- `global_ptr` operations never throw
- Pointer arithmetic and dereferencing never throw

**Guarantees:**
- All pointer operations are `noexcept`
- All comparison operators are `noexcept`
- All accessor methods are `noexcept`

## Exception Specifications

### Memory Region Allocation

```cpp
// Strong guarantee: Either succeeds completely or throws
template <typename... Args>
static memory_region<RT>* alloc_region(size_t payload_capacity, Args&&... args)
    noexcept(false);  // May throw std::bad_alloc or construction exceptions

// Strong guarantee: Either succeeds completely or throws
template <typename Allocator, typename... Args>
static memory_region<RT>* alloc_region_with(Allocator allocator, 
                                            size_t payload_capacity, 
                                            Args&&... args)
    noexcept(false);  // May throw std::bad_alloc or construction exceptions
```

### Memory Region Deallocation

```cpp
// No-throw guarantee: Never throws, handles null safely
static void free_region(memory_region<RT>* region) noexcept;

// No-throw guarantee: Never throws, handles null safely
template <typename Allocator>
static void free_region_with(Allocator allocator, 
                             memory_region<RT>* region) noexcept;
```

### Object Construction

```cpp
// Strong guarantee: Either constructs object or throws
template <typename VT, typename... Args>
global_ptr<VT, RT> create(Args&&... args)
    noexcept(false);  // May throw std::bad_alloc or construction exceptions

// Strong guarantee: Either constructs object or throws
template <typename VT, typename... Args>
void create_to(regional_ptr<VT>& rp, Args&&... args)
    noexcept(false);  // May throw std::bad_alloc or construction exceptions
```

### Object Allocation

```cpp
// Strong guarantee: Either allocates memory or throws
template <typename T>
T* allocate(size_t n = 1) noexcept(false);  // May throw std::bad_alloc

// Strong guarantee: Either allocates aligned memory or throws
void* allocate(size_t size, size_t align) noexcept(false);  // May throw std::bad_alloc
```

## Exception Types

### Standard Exceptions

- **`std::bad_alloc`**: Thrown when memory allocation fails
- **`std::invalid_argument`**: Thrown for invalid arguments (e.g., cross-region pointer assignment)
- **`std::out_of_range`**: Thrown for out-of-bounds access in containers

### Custom Exceptions

- **`yaml::ParseError`**: Thrown by YAML parsing operations
- **`yaml::AuthorError`**: Thrown by YAML authoring operations
- **`yaml::TypeError`**: Thrown by YAML type mismatches

## Resource Management Guarantees

### 1. Memory Leak Prevention

**RAII Pattern:**
```cpp
// Guaranteed cleanup even if exceptions occur
auto_region<DocumentStore> region(1024 * 1024);
auto doc = region->create<Document>(...);
// If any operation throws, region is automatically cleaned up
```

**Manual Management:**
```cpp
// Must use try-catch for proper cleanup
auto* region = memory_region<DocumentStore>::alloc_region(1024 * 1024);
try {
    auto doc = region->create<Document>(...);
    // ... use region ...
    memory_region<DocumentStore>::free_region(region);
} catch (...) {
    memory_region<DocumentStore>::free_region(region);
    throw;
}
```

### 2. Partial Construction Safety

**Object Construction:**
- If object construction fails after allocation, memory is properly reclaimed
- No partially constructed objects remain in the region
- All allocated memory is accounted for

**Container Operations:**
- If container operations fail, the container remains in a valid state
- No partially inserted elements remain
- Size and capacity remain consistent

## Container Exception Safety

### regional_vector

**Strong guarantee for all modifying operations:**
- `push_back`: Either succeeds or vector remains unchanged
- `emplace_back`: Either succeeds or vector remains unchanged
- `erase`: Either succeeds or vector remains unchanged
- `clear`: No-throw guarantee

**Basic guarantee for operations that may reallocate:**
- Reserve operations: May throw but vector remains valid
- Insert operations: May throw but vector remains valid

### regional_str

**Strong guarantee for all operations:**
- Construction: Either succeeds or no allocation occurs
- Assignment: Either succeeds or string remains unchanged
- Concatenation: Either succeeds or string remains unchanged

## YAML Integration Exception Safety

### Parsing Operations

**Strong guarantee:**
- `YamlDocument` construction: Either succeeds completely or throws
- `YamlDocument::Parse`: Returns `ParseResult` variant (no-throw)
- `YamlDocument::Read`: Returns `ParseResult` variant (no-throw)

### Authoring Operations

**Strong guarantee:**
- `YamlDocument` authoring construction: Either succeeds or throws
- `YamlDocument::Write`: Returns `AuthorResult` variant (no-throw)

### Serialization Operations

**May throw AuthorError:**
- `to_yaml` functions: May throw `yaml::AuthorError` as YamlAuthor object API can throw them
- Implementations are not mandated to catch these exceptions

## Best Practices

### 1. Prefer RAII

```cpp
// ✅ Recommended: Use auto_region for automatic cleanup
auto_region<MyRoot> region(1024 * 1024);
// All operations are exception-safe

// ⚠️ Manual management requires careful exception handling
auto* region = memory_region<MyRoot>::alloc_region(1024 * 1024);
// Must handle exceptions manually
```

### 2. Use No-Throw Operations When Possible

```cpp
// ✅ Use noexcept operations for critical paths
regional_ptr<MyType> ptr;
// ptr operations are noexcept

// ✅ Use Result types for expected failures
auto result = YamlDocument::Parse("config.yaml", content);
if (result.has_value()) {
    // Success path
} else {
    // Error path
}
```

### 3. Exception-Safe Resource Acquisition

```cpp
// ✅ Exception-safe: Resources acquired in constructor
class MyResource {
    auto_region<MyRoot> region_;
public:
    MyResource() : region_(1024 * 1024) {
        // If any operation throws, region_ is automatically cleaned up
    }
};
```

### 4. Error Handling Strategy

**For configuration loading:**
```cpp
// Use exception-throwing for trusted input
try {
    YamlDocument config("app.yaml", std::ifstream("app.yaml"));
    // Process configuration
} catch (const yaml::ParseError& e) {
    // Handle parse error
}
```

**For user input:**
```cpp
// Use Result types for untrusted input
auto result = YamlDocument::Read(user_file);
if (!result) {
    // Handle error gracefully
    return;
}
```

## Testing Exception Safety

### Unit Tests

All exception safety guarantees must be tested:

1. **RAII cleanup**: Verify automatic cleanup on exception
2. **Memory leaks**: Verify no leaks when exceptions occur
3. **State consistency**: Verify invariants are maintained
4. **Resource limits**: Verify behavior under allocation failures

### Integration Tests

Test exception safety in realistic scenarios:

1. **YAML parsing**: Test with malformed input
2. **Memory exhaustion**: Test with limited memory
3. **Cross-region operations**: Test with invalid region references
4. **Container operations**: Test with edge cases

## Summary

| Component | Exception Safety | Primary Exception | Cleanup Guarantee |
|-----------|------------------|-------------------|-------------------|
| `auto_region` | Strong | `std::bad_alloc` | Automatic |
| `memory_region` | Basic | `std::bad_alloc` | Manual required |
| `regional_ptr` | No-throw | None | N/A |
| `global_ptr` | No-throw | None | N/A |
| `regional_vector` | Strong | `std::bad_alloc` | Automatic |
| `regional_str` | Strong | `std::bad_alloc` | Automatic |
| YAML parsing | Strong/Result | `yaml::ParseError` | Automatic |
| YAML authoring | Strong/Result | `yaml::AuthorError` | Automatic |

All components are designed to prevent resource leaks and maintain memory safety even in the presence of exceptions.
