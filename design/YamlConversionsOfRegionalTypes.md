# YAML Conversions of Regional Types

This document provides comprehensive documentation for YAML serialization and deserialization support in the shilos memory region system. It covers the modular YAML integration system, type requirements, and implementation patterns for regional types.

## Overview

YAML integration in shilos is **optional and modular** - regional types do not require built-in YAML methods. Instead, YAML support is provided through standalone template functions in separate headers, allowing users to selectively enable YAML functionality for specific types.

## Design Principles

- **Separation of Concerns**: Core regional types contain no YAML-specific code
- **Modular Headers**: YAML functionality provided in separate `*_yaml.hh` headers
- **Standalone Functions**: YAML operations implemented as free functions, not member methods
- **Concept-Based**: Type safety enforced through C++20 concepts

## YamlConvertible Concept

The `YamlConvertible` concept defines the minimal API a type **T** must expose to participate in the YAML serialization subsystem. A type that satisfies the concept can be **both** converted **to** a `yaml::Node` _and_ reconstructed **from** a `yaml::Node` inside any `memory_region<RT>`:

```cpp
template <typename T, typename RT>
concept YamlConvertible = requires(T t,
                                   const yaml::Node &node,
                                   memory_region<RT> &mr,
                                   T *raw_ptr) {
  // Serialization – may throw AuthorError as YamlAuthor API can throw them
  { to_yaml(t) } -> std::same_as<yaml::Node>;

  // Deserialization – in-place construction at *raw_ptr
  { from_yaml<T>(mr, node, raw_ptr) } -> std::same_as<void>;
};
```

Key points:
- `to_yaml` is a free function that returns a fully-formed `yaml::Node`. It may throw `yaml::AuthorError` as YamlAuthor object API can throw them, and implementations are not mandated to catch them.
- `from_yaml` is a free function template that performs **in-place** deserialization at the uninitialised memory pointed to by `raw_ptr`. This design avoids copy / move operations that regional types forbid.

## yaml::Document API

The `yaml::Document` class provides a **dual API pattern** with two consistent approaches for different error handling preferences:

### Constructor-based APIs (Exception-throwing)
- Throw exceptions directly (`ParseError`, `AuthorError`)
- Use when you prefer exception-based error handling
- More concise for simple error propagation

### Static method-based APIs (noexcept)
- Return `Result` variants (`ParseResult`, `AuthorResult`)
- Use when you prefer explicit error handling via Result types
- Better for functional programming patterns and error composition

### Parsing API

**1. Constructor-based (throws exceptions)**

```cpp
// Parse from string - throws ParseError on failure
yaml::Document doc("config.yaml", yaml_content);

// Parse from file path - throws ParseError on failure
yaml::Document doc("config.yaml", std::ifstream("config.yaml"));
```

**2. Static method-based (noexcept)**

```cpp
// Parse from string - returns ParseResult
ParseResult result = yaml::Document::Parse("config.yaml", yaml_content);

// Parse from file path - returns ParseResult
ParseResult result = yaml::Document::Read("config.yaml");
```

### Authoring API

**1. Constructor-based (throws exceptions)**

```cpp
// Create document with callback - throws AuthorError on failure
yaml::Document doc("output.yaml", [](YamlAuthor& author) {
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

### Usage Examples

**Create single-document YAML using static method (noexcept)**

```cpp
AuthorResult doc = yaml::Document::Write("generated.yaml", [](YamlAuthor& author) {
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
if (std::holds_alternative<yaml::Document>(doc)) {
    auto& yaml_doc = std::get<yaml::Document>(doc);
    std::cout << "Generated YAML successfully" << std::endl;
} else {
    auto& error = std::get<yaml::AuthorError>(doc);
    std::cerr << "Error: " << error.what() << std::endl;
}
```

**Create multi-document YAML using constructor (throws)**

```cpp
try {
    yaml::Document doc("multi.yaml", [](YamlAuthor& author) {
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

## Region Instance Methods: create_from_yaml & create_from_yaml_at

`memory_region<RT>` provides two **instance methods** that make it trivial to allocate and initialise regional objects directly from YAML data:

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

## Implementation Structure

YAML support headers typically contain:

1. **Inline Functions**: All YAML logic implemented inline in headers
2. **Template Functions**: Generic `from_yaml<T>()` functions for any memory region type
3. **Complete Functionality**: Full serialization/deserialization with error handling

This modular approach allows users to:
- Use regional types without YAML overhead when not needed
- Selectively enable YAML for specific types
- Maintain clean separation between core functionality and serialization
- Benefit from compile-time optimization of inline implementations

## YAML Container Support

YAML deserialization supports containers holding both bits types and regional types. **Mapping nodes** are deserialised into `iopd<V>` instances, guaranteeing that the *user-defined order of keys* is preserved on round-trip serialisation. Internally the container combines a vector (for order) with a hash index (for O(1) look-ups), so there is no performance trade-off when accessing elements programmatically.

The implementation leverages the fixed-size and RTTI-free constraints of regional types to:
- Allocate uninitialized memory at final container locations
- Use the raw pointer version `from_yaml(mr, node, T* raw_ptr)` for direct in-place construction
- Avoid copy/move operations that would violate regional type constraints

## Example Implementation – Step-by-Step Guide

The following walk-through demonstrates how to add YAML support for a **custom regional struct** as well as a **regional_vector** container that stores both *bits* types and other regional types. The pattern is universally applicable – simply swap field definitions or container choices to match your own data model.

```cpp
// Example domain object – a simple "Book"
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

### 1. Implement to_yaml(const Book&) noexcept

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
- Prefer **composition** – delegate to `to_yaml` of sub-objects / containers.
- When a field is a *bits* scalar (e.g. `int`, `bool`) no special code is needed – the `yaml::Node` constructor handles it.

### 2. Implement from_yaml<Book>()

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
1. **Always construct the outer object before touching its fields** – this guarantees valid lifetimes when sub-objects reference `*this`.
2. Reuse the **generic container deserialisers** (`vector_yaml.hh`, etc.). They automatically handle:
   - bits elements (`int`, `double`, `bool`, …)
   - nested regional elements (inc. other containers)
   - recursive graphs (vector<vector<…>>)
3. For bits fields you may directly assign the result of `node.as<T>()` or rely on the container helper as shown.

### 3. Register convenience wrappers (optional)

Nothing extra is required: because `Book` now satisfies `YamlConvertible` the following helpers work out-of-the-box:

```cpp
auto my_book = region->create_from_yaml<Book>(yaml::Load(file_text));

regional_ptr<Book> slot;
region->create_from_yaml_at<Book>(node, slot);
```

### 4. Container Authoring Checklist

When building **your own regional container** (queue, map, graph, …) that should participate in YAML:

1. **Expose a callback-based insertion API** similar to `regional_vector::emplace_init(memory_region<RT>&, Fn&&)`. This enables deserialisers to construct elements *in-place* without temporary objects.
2. **For bits element types** provide a fast-path that accepts a scalar `yaml::Node` and calls the plain `elem_node.as<T>()` conversion.
3. **For regional element types** delegate to their own `from_yaml` inside the `emplace_init` callback.
4. **Avoid private-member access in deserialisers** – use the public insertion API you exposed.

Following these guidelines guarantees that the container remains free of copy/move operations and can happily nest within itself (`vector<vector<…>>`) or other regional structures.

---

With these patterns in place you can equip any regional type – from simple value objects to arbitrarily deep container graphs – with safe, zero-copy YAML serialisation while preserving the fundamental invariants of the shilos memory model.

## Cross-References

For complete information about memory regions and regional types, see [Memory Region and Regional Types Specification](MemoryRegionAndRegionalTypes.md).
