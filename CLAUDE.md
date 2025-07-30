# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the CoD (Compile-on-Demand) project - a C++20-based ecosystem implementing zero-cost-relocation memory regions and regional types. The project consists of:

- **shilos**: Core memory region and regional type system with zero-cost relocation guarantees
- **cod**: Main compiler/REPL interface (future development)
- **codp**: CoD Package Manager for dependency management
- **llvm-project**: Complete LLVM toolchain as external project dependency

## Build System

### Primary Build Command
```bash
cmake --build ./build
```

### Full Toolchain Build (for development setup)
```bash
./do-build-cod.sh
```

**Important**: The AI agent must exclusively use `cmake --build ./build` for compilation after code modifications. Bash scripts in the project directory are reserved for human developers and must never be executed by the AI agent.

### Build Architecture
- Uses CMake with Ninja generator
- Built as external project of LLVM (`llvm-project/` directory)
- 3-stage build process: stage1 (system compiler) → stage2 (intermediate) → stage3 (final)
- Platform-specific build directories: `build-$(uname -m)-$(uname -s)/` (e.g., `build-x86_64-Darwin/`)
- Root symlinks maintained platform-portably:
  - `build/` → `build-$(uname -m)-$(uname -s)/stage3/` (main src build tree)
  - `built/` → `build-$(uname -m)-$(uname -s)/cod-release/` (final toolchain)
- Symlinks are created by human at initial project setup by running `do-build-cod.sh`

## Testing

### Test Structure
Each component under `tests/` contains a `run-all.sh` script that builds test artifacts and executes all test cases:

```bash
# Run shilos C++ tests
./tests/shilos/run-all.sh

# Run CoD Package Manager tests  
./tests/codp/run-all.sh

# Run YAML comparison tool tests
./tests/yaml-cmp/run-all.sh
```

### Toolchain Selection
Tests support two toolchain modes via `COD_TEST_TOOLCHAIN` environment variable:
- **`build`** (default): Uses `<project>/build/` for development workflow
- **`built`**: Uses `<project>/built/` for pre-release testing

### Test Utilities
- `tests/test-utils.sh`: Common functions for test setup and toolchain configuration
- `yaml-cmp`: YAML comparison utility for testing YAML serialization/deserialization

## Core Architecture

### Memory Regions and Regional Types
The shilos system implements zero-cost-relocation memory management:

- **memory_region<RT>**: Template class for type-aware memory allocation
- **auto_region<RT>**: RAII wrapper for automatic memory management (recommended)
- **regional_ptr<T>**: Offset-based pointer for single-region programs (8 bytes)
- **global_ptr<T,RT>**: Region-safe pointer for multi-region programs (16 bytes)

**Critical**: When handling `global_ptr` or `regional_ptr`, immediately reference `design/MemoryRegionAndRegionalTypes.md` and adhere precisely to all specified protocols.

### Regional Type Constraints
- Must be allocated in memory regions (no stack allocation)
- No copy/move semantics (zero-cost-relocation requirement)
- Construction requires `memory_region&` parameter
- Use `regional_ptr` for internal pointers only
- YAML serialization support is modular and optional

### Container Types
- **regional_str**: Region-allocated string with O(1) operations
- **regional_fifo**: Queue container (FIFO semantics)
- **regional_lifo**: Stack container (LIFO semantics)  
- **iopd<K,V>**: Insertion-order preserving dictionary for YAML mappings

## Development Guidelines

### Language and Standards
- **C++20 exclusively**: Prioritize modern C++20 features (concepts, ranges, modules, coroutines)
- **No deprecated patterns**: Avoid legacy C++ approaches unless explicitly justified
- **LLVM integration**: This workspace is an external project of the LLVM ecosystem

### Memory Management
- **Prefer auto_region**: Use RAII management over manual `free_region` methods
- **Never use raw delete**: Forbidden on memory regions (triggers assertion failure)
- **Allocator matching**: Same allocator must be used for allocation and deallocation

### Documentation Standards
- Write dedicated documents and source-code comments for final state
- Avoid describing what changed or previous efforts
- Two exceptions: change-log files and source code where both old and new designs remain operational

## Package Management (codp)

### CoD Package Manager Concepts
- **VCS-native workflow**: Git is single source of truth, no central registry
- **Deterministic builds**: Lock-file (`CodManifest.yaml`) ensures reproducibility
- **Local dependencies**: Support for developer-local checkouts via `path:` overrides
- **Zero-cost sharing**: Memory regions shared via mmap across processes

### Project Structure
- `CodProject.yaml`: Author-maintained project metadata
- `CodManifest.yaml`: Machine-generated dependency lock file
- `~/.cod/pkgs/`: Global package cache with mirrors and snapshots

## Important Files and Directories

### Core Source
- `include/shilos/`: Public headers for memory region system
- `shilos/`: Core shilos implementation
- `codp/`: Package manager implementation
- `include/codp.hh`: Package manager public interface

### Documentation
- `design/`: Architecture specifications and design documents
- `design/MemoryRegionAndRegionalTypes.md`: Critical memory management specification
- `design/CodProjectStructureAndPkgMgmt.md`: Package management design

### Configuration
- `.cursor/rules/`: Development environment rules and preferences
- `CMakeLists.txt`: Main build configuration
- `do-build-cod.sh`: Full toolchain build script

## Development Workflow

1. **Build changes**: `cmake --build ./build`
2. **Run tests**: `./tests/shilos/run-all.sh && ./tests/codp/run-all.sh`
3. **Memory region work**: Always consult `design/MemoryRegionAndRegionalTypes.md`
4. **Package management**: Reference `design/CodProjectStructureAndPkgMgmt.md`
5. **LLVM integration**: Leverage `llvm-project/` codebase for internal functionality

## Current Implementation Stage

This C++20 implementation prioritizes correctness and zero-cost-relocation guarantees over ergonomics. The long-term vision includes developing a dedicated programming language that will provide more ergonomic regional type syntax while maintaining the core memory safety properties.
