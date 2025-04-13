# Memory Region and Regional Types Design

## Core Design Principles

### Memory Region

```cpp
template <typename RT>
class memory_region {
  // Should provide:
  // - Type-aware allocation+construction services
  // - Unsupport single value destruction, facilitate destruction by asking the root type to release all resources held by the entire data graph in a single shot
  // - Pointer conversion utilities
};
```

### Regional Type Guidelines

1. Construction:

   - Must receive memory_region& in constructor
   - Never implement nullary constructor
   - Construction always occurs within a memory_region context

2. Lifetime Management:

   - No copy/move semantics (deleted by design)
   - No destruction semantics - memory_region manages all resource release through its root type
   - Should avoid owning external resources
   - Lifetime strictly controlled by containing memory_region
   - Resource release based on data graph from the root value of a region, upon region destruction as a whole

3. Field Type Considerations

   - Members without internal pointers are considerted "bits types", they should be treated like plain C++ types
   - Members with internal pointers must be of regional types themselves

4. YAML Integration:

   - Support to_yaml()
   - Support static `template <typename RT> from_yaml(memory_region<RT>&, ...)` with 2 version, one return a global_ptr, another assign to a given regional_ptr
   - Maintain type safety through conversion

5. Pointer Semantics:
   - Use regional_ptr for internal references
   - global_ptr for cross-region references
   - Use of raw pointers is allowed but may trigger undefined behavior in certain cases

## Design Architecture

### Type Construction Pattern

```cpp
template <typename RT>
class RegionalType {
public:
  // Only allowed construction form
  explicit RegionalType(memory_region<RT>& mr)
    : members_() {
    mr.initialize(members_);
  }

  // Explicitly deleted special members
  RegionalType(const RegionalType&) = delete;
  RegionalType(RegionalType&&) = delete;
  RegionalType& operator=(const RegionalType&) = delete;
  RegionalType& operator=(RegionalType&&) = delete;

  // YAML conversion - dual interface
  static global_ptr<RegionalType, RT> from_yaml(memory_region<RT>& mr,
                                              const yaml::Node& node) {
    auto obj = mr.template create<RegionalType>(mr);
    obj->load_from_yaml(node, mr);
    return obj;
  }

  static void from_yaml(memory_region<RT>& mr,
                      regional_ptr<RegionalType>& out,
                      const yaml::Node& node) {
    mr.template create_to<RegionalType>(out, mr);
    out->load_from_yaml(node, mr);
  }
};
```

### YAML Integration

```cpp
struct YamlConvertible {
  virtual yaml::Node to_yaml() const = 0;
  virtual void from_yaml(const yaml::Node&) = 0;
};
```
