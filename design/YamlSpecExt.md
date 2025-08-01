# YAML Specification Extensions

This document describes shilos's extensions and constraints to the YAML specification, focusing on comment support and formatting conventions.

## Design Principles

1. **Robustness and simplicity in implementation** as the primary goal
2. **Human-friendly formatting** as the secondary goal
3. **Idempotent parsing and formatting** - parse(format(parse(x))) === parse(x)
4. **Comments belong to structural elements**, not arbitrary nodes

## Comment Support Model

### Core Type System

```cpp
// Basic node - NO comment fields
struct Node {
    Value value;  // Pure data: string, number, bool, SimpleSequence, Map, etc.
};

// Only structural elements can have comments:
struct MapEntry {
    std::string_view key;
    Node value;
    std::vector<std::string_view> leading_comments;
    std::string_view trailing_comment;
};

struct SeqItem {  // For dash sequences only
    Node value; 
    std::vector<std::string_view> leading_comments;
    std::string_view trailing_comment;
};

struct Document {
    Node root;
    std::vector<std::string_view> leading_comments;  // Document header only (no footer comments)
};
```

### Sequence Type Distinction

**Simple Sequences** (flow style) - No per-item comments:
```yaml
tags: [web, api, service]
ports: [80, 443, 8080]
```

**Dash Sequences** (block style) - Comments allowed per item:
```yaml
branches:
  # Comment about main branch
  - main     # Trailing comment
  # Comment about develop
  - develop
```

**Maps** - Comments allowed per entry:
```yaml
# Comment about this key-value pair
repo_url: "https://example.com/repo.git"  # Trailing comment
```

## Comment Types and Placement

### Document Header Comments
Comments at the beginning of a document, separated from content by blank lines:

```yaml
# This is a document header comment
# It describes the entire YAML document
# Blank line separation is required

uuid: "123-456"
name: project
```

**Important**: YAML documents can only have header comments. Footer comments at the end of documents are not supported.

### Leading Comments
Comments that appear before structural elements, separated by the element they describe:

```yaml
# This comment belongs to the map entry below
key: value

dependencies:
  # This comment belongs to the sequence item below
  - name: example
```

**Important**: Leading comments can only be associated with map entries or dash-sequence items. They cannot be associated with simple values or other structural elements.

### Trailing Comments
Comments that appear on the same line as the value:

```yaml
key: value # This is a trailing comment
- item # This is a trailing comment on sequence item
```

**Important**: Trailing comments can only be associated with simply-valued entries (scalars) or dash-sequence items. They cannot be associated with complex structures like nested maps.

## Formatting Rules

### Comment Output Order
1. **Document header comments** (if any) followed by blank line
2. **Leading comments** for structural elements
3. **Key and colon** for map entries
4. **Value**
5. **Trailing comment** (if any) on same line

### Indentation
- Comments inherit indentation from their structural element
- Leading comments align with the element they precede
- Trailing comments separated by single space from value (aligns with VSCode YAML formatter)

### Blank Line Rules
- Document header comments separated from content by blank line
- No automatic blank lines between regular comments and elements
- Preserve intentional blank lines in leading comment blocks
- **Every YAML document must end with a blank line** (final newline character)

### Mapping-as-Sequence-Item Style
When a mapping appears as a sequence item (e.g., dependencies in a project manifest), the idiomatic format is:

```yaml
sequence_key:
  -
    key1: value1
    key2: value2
  -
    key1: value1
    key2: value2
```

This style provides:
1. **Clear visual separation** between sequence items
2. **Consistent indentation** for nested mappings
3. **Comment placement compatibility** - allows leading and trailing comments for each mapping
4. **Readability** - makes it easy to distinguish between different items in the sequence

The empty line after the dash and before the mapping is the idiomatic pattern that ensures proper comment attachment and formatting consistency.

## Constraints and Limitations

### Not Supported
- Comments within flow sequences: `[a, # comment, b]`
- Comments within flow mappings: `{key: # comment, value}`
- Comments after colons but before values on next line:
  ```yaml
  key: # This pattern is not supported
    value
  ```
- Document footer comments (comments at the end of a YAML document)
- Leading comments on simple scalar values (only map entries and dash-sequence items)
- Trailing comments on complex structures like nested maps (only simple values and dash-sequence items)

### Supported Patterns
- Block-style mappings with comments
- Block-style sequences (dash format) with comments
- Document-level header comments only (no footer comments)
- Trailing comments on same line as simple values and dash-sequence items
- Leading comments on map entries and dash-sequence items only

## Implementation Benefits

This design eliminates complex formatting logic by:
1. **Natural comment ordering** - no need to reorder comments during output
2. **Clear ownership** - comments belong to structural elements, not arbitrary nodes
3. **Type safety** - only comment-capable structures have comment fields
4. **Simplified parsing** - comments associated during structure creation

## Examples

### Complete Document Structure
```yaml
# Document header describing the project
# Multi-line header comments supported

uuid: "example-123"
name: "my-project"

# Comment about repository configuration
repo_url: "https://github.com/example/repo.git" # Official repository

branches:
  # Production branch
  - main # Stable releases
  # Development branch
  - develop # Active development
  
dependencies:
  # Core framework dependency
  - name: "framework"
    version: "1.0.0" # Latest stable
    # Optional development path
    path: "dev/framework"
```

This structure ensures predictable, idempotent formatting while maintaining human readability.
