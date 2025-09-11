# Compile-on-Demand (CoD) — Build-and-Run REPL without JIT

This document specifies the design of the `cod` driver that provides a REPL-like developer experience by compiling and running ephemeral executables, with session state persisted in a disk-backed memory region (DBMR). There is no JIT and no in-process code injection. Instead, each evaluation step produces a small native executable that is run immediately. Session state survives across runs via a persistent workspace file on disk.

---

## Goals

- Deterministic “compile + link + run” workflow grounded in the project manifest (see CodProjectStructureAndPkgMgmt.md) — no hidden state outside of the workspace DBMR and the project’s VCS-managed sources.
- No JIT, no ORC, no Clang-Interpreter — all evaluation happens by compiling a tiny runner that links against project code and standard libraries, then spawning it as a separate process.
- Persistent REPL session state stored in a DBMR file (default: `./.cod/works.dbmr`) that can be shared across processes and inspected offline.
- Extensible workspace root type — defaults to `cod::WorksRoot` (to be defined in `include/cod.hh`), but projects may opt into a custom root type via CodProject.yaml.
- Zero-copy, zero-relocation memory-region invariants are respected at all times (see MemoryRegionAndRegionalTypes.md).
- No standard REPL prelude; the project defines the visible lexical scope for REPL submissions via CodProject.yaml (see “repl.scope”).

## Non-Goals (Stage 1)

- No interpreter semantics for true top-level statements; all input is ordinary C++20 code compiled into a tiny runner. There are no custom REPL directives and no REPL-side definition storage.
- In-REPL incremental linking of compiled fragments. We recompile a tiny runner per evaluation.
- Multi-language support. Only C++20.

---

## High-Level Architecture

- `cod` front-end (binary in `cod/`) implements the interactive loop and command parsing.
- Workspace persistence:
  - Backed by `shilos::DBMR<Root>` where `Root` defaults to `cod::WorksRoot`.
  - File path selectable via `--works <path>` or `-w <path>`. Default is `./.cod/works.dbmr`.
  - The root object lives inside the DBMR and owns session-scoped data as defined by the project (see WorksRoot Model below).
- Build-and-Run pipeline (per evaluation):
  1. Assemble a minimal translation unit (TU) that includes the project-provided REPL scope header(s) from CodProject.yaml (see repl.scope).
  2. Generate a tiny `main()` that embeds the submitted statements/expressions verbatim inside a block.
  3. Compile and link the TU into an executable in a temp workdir.
  4. Launch the executable as a child process, wiring stdio to the REPL and passing a handle to the workspace DBMR file path and open mode (e.g., via argv and/or `COD_WORKS_PATH` environment variable).
  5. Collect exit status, stdout/stderr, and persist any updates the child writes to the DBMR.
- Project integration:
  - `cod` reads `CodManifest.yaml` to discover dependency snapshots and derive include paths and other toolchain flags deterministically.
  - No network activity is performed by `cod`; package resolution is handled by `codp`.

---

## Command-Line Interface (CLI)

- `cod [-w|--works <PATH>] [--project <PATH>]` — start an interactive session bound to the DBMR at PATH (default `./.cod/works.dbmr`). If the DBMR file does not exist, `cod` creates it with a default capacity.
- `cod [-w|--works <PATH>] [--project <PATH>] -e|--eval <EXPRESSION>` — evaluate a single expression or statement and exit (non-interactive mode).
- `cod --help` — show usage.

### Interactive vs Non-Interactive Modes

**Interactive REPL Mode** (default when no `-e`/`--eval` is specified):
- Starts an interactive session with a `cod>` prompt
- Accepts multiple submissions over the session lifetime
- Supports REPL commands like `%quit`, `%help`
- Supports line continuation with backslash (`\`)
- Session persists until explicitly terminated

**Non-Interactive Eval Mode** (when `-e`/`--eval` is specified):
- Evaluates the provided expression/statement once
- Exits immediately after evaluation
- Returns exit code 0 on success, non-zero on failure
- Suitable for scripting and automation
- No REPL commands or interactive features

### Input Model (Both Modes)

- Accepts plain C++20 statements and expressions, not custom directives.
- The visible lexical scope of each submission is determined by the scope header(s) specified by the project in CodProject.yaml (see "repl.scope").
- There is no facility to define new functions or types from submissions; users are expected to edit project sources and have `cod` rebuild affected modules on demand. Submissions should call into functions/classes exposed by the scope header(s).
- Each submission is compiled and linked into a temporary executable; there is no JIT.

### Usage Examples

**Interactive REPL:**
```bash
# Start interactive session with default workspace
cod

# Start with custom workspace and project
cod -w /path/to/workspace --project /path/to/project
```

**Non-Interactive Evaluation:**
```bash
# Evaluate a simple expression
cod -e "std::cout << 'Hello, World!' << std::endl;"

# Evaluate with custom workspace
cod -w /path/to/workspace -e "myFunction(42);"

# Use in scripts (check exit code)
cod -e "return validateData();" && echo "Validation passed"
```

---

## WorksRoot Model (default workspace root)

Header: `include/cod.hh`

The default root type must satisfy `ValidMemRegionRootType`. It provides:

- `static inline constexpr uuid TYPE_UUID` — type identity.
- `WorksRoot(memory_region<WorksRoot>&)` constructor — constructs owned structures in-place. No copy/move.
- Data members (regional types only; zero-cost relocation, allocated in-region):
  - `list<str>` notes — optional user notes or annotations stored by project code.
  - `vector<uint8_t>` scratch — general-purpose scratch area for future features.
  - Optional indices and registries as needed by the project (e.g., `dict<str, uint32_t>`), but the default starts minimal.

Rationale:
- Keep the root lean; there is no REPL-managed definition storage. Project code decides how to use the workspace for state across runs.

---

## DBMR Lifecycle & Capacity

- Default file name: `.cod/works.dbmr` in the current working directory.
- Open/creation:
  - Read-only open is used for inspection commands; read-write for interactive sessions.
  - On creation, `DBMR<WorksRoot>::create(path, free_capacity, …)` is called with a sensible default (e.g., 64–256 MiB configurable via a CLI flag in the future).
- Close:
  - Call `constrict_on_close(true)` when the session ends cleanly to optionally shrink the file to occupied size.
  - The driver ensures `msync` of updates before unmapping.

Concurrency model (Stage 1):
- Single-writer semantics: one REPL mutating a given DBMR at a time. Multiple read-only observers are allowed.

---

## Submission Semantics

- Input: plain C++20 statements or expressions.
- The driver synthesises a runner TU:
  - Injects `#include` lines for the configured scope header(s) from CodProject.yaml (see repl.scope).
  - Provides a `main()` that forwards the workspace path via environment or argv.
  - Embeds the submitted code inside a block in `main()`; the code can reference any symbols made visible by the scope header(s).
- Printing/output:
  - The runner forwards stdout/stderr of the submitted code. There is no automatic printing; printing is up to the project’s APIs or the submission itself.

Compilation unit layout (runner):

```cpp
// scope: project-provided headers
#include "<scope header 1>"
// ... potentially more headers per configuration

int main(int argc, const char** argv) {
  // The driver may set COD_WORKS_PATH and/or pass the works path via argv
  // Project-provided scope can read it and open/access the workspace as needed.

  // Begin submitted code
  {
    /* user statements / expression */
  }
  // End submitted code
  return 0;
}
```

---

## Build Pipeline & Toolchain Flags

- Inputs:
  - Project’s `CodManifest.yaml` → list of dependency snapshots and (for locals) paths.
  - Existing build metadata if available (e.g., compile_commands.json) may be reused in later stages.
- Stage 1 flag synthesis:
  - Include paths derived from project layout and dependencies resolved by the manifest.
  - C++20 standard, warnings matching repo defaults, and feature macros consistent with the main build.
  - Link against the project’s build outputs as needed to resolve symbols referenced by the submission (preferred: static libraries or objects produced by the project build configured for the current manifest).
- Build cache:
  - **Local project and dependencies**: The REPL uses a per-workspace temporary directory (e.g., `./.cod/works/<hash>/`) for build artifacts from the current project and local dependencies (those specified with `path:` in CodProject.yaml).
  - **Repository dependencies**: Build artifacts for remote repository dependencies are cached globally under `~/.cod/cache/` to enable sharing across multiple projects that depend on the same snapshots.
  - **Cache structure**:
    - Local cache: `./.cod/works/<cache_key>/` contains bitcode files (`.bc`) for both `.cc` and `.hh` files from the current project and local deps.
    - Global cache: `~/.cod/cache/<repo_url_key>/<commit_hash>/` contains bitcode files for repository dependencies.
  - **Bitcode generation**: Instead of storing executables (which would be submission-specific), we generate and cache LLVM bitcode:
    - `.cc` files → `.bc` bitcode modules for faster linking in subsequent REPL evaluations.
    - `.hh` files → `.bc` bitcode for precompiled headers when they contain significant template instantiations or constexpr computations.
  - **Cache key** includes: toolchain version, compiler flags, project snapshot identifiers, and content hash of the source file. Any mismatch invalidates the cache entry.
  - **Future stage**: Repository dependency caching under `~/.cod/cache/` will be implemented in a later stage; initially all artifacts go to local `./.cod/works/` directories.
- Execution:
  - After link, run the produced executable; capture stdout/stderr and exit code.
  - Timeouts and signal handling will be plumbed through and reported.

### Cache Schema Design Considerations

**Bitcode vs. Object Files:**
- LLVM bitcode (`.bc`) provides better optimization opportunities during final linking compared to pre-compiled object files.
- Bitcode is more portable across different optimization levels and can be re-optimized based on the specific REPL submission context.
- Smaller storage footprint compared to debug-enabled object files.

**Header File Caching:**
- **Pros**: Headers with heavy template instantiations, constexpr computations, or large inline functions benefit significantly from pre-compilation.
- **Cons**: Most headers are lightweight and caching may not provide meaningful speedup while consuming additional storage.
- **Strategy**: Initially cache all headers as bitcode; add heuristics in later stages to skip caching for headers below a complexity threshold.

**Cache Locality Trade-offs:**
- **Local cache** (`./.cod/works/`): Fast access, project-specific, but no sharing across projects.
- **Global cache** (`~/.cod/cache/`): Enables sharing of repository dependencies across projects, but requires careful cache key design to avoid conflicts.
- **Hybrid approach**: Local cache for current project + local deps ensures fast iteration; global cache for repo deps maximizes reuse.

**Cache Invalidation:**
- **Semantic hashing**: Use Clang's AST parsing capabilities to generate hashes based on semantic structure rather than textual content, enabling cache hits for semantically equivalent code (e.g., whitespace-only changes, comment modifications).
- **Efficiency optimization**: Perform file modification timestamp comparison first; only parse AST and compute semantic hash if the file has been modified since last cache entry.
- **Toolchain changes**: Toolchain version changes require full cache invalidation to prevent ABI mismatches.

---

## Integration with CodProject.yaml (Workspace Root Type and Scope)

Projects can:

1) Opt into a custom works root type.
2) Define the visible lexical scope for REPL submissions.

Proposed extension (YAML schema):

```yaml
works:
  root_type:
    qualified: "cod::WorksRoot"   # fully-qualified C++ type name
    header: "cod.hh"              # header that defines the root type

repl:
  scope: "main.hh"                # header (or relative path) that defines what the REPL sees
  # Future: allow a list
  # scope:
  #   - "api/main.hh"
  #   - "extras/bench.hh"
```

Rules:
- `root_type.qualified` must satisfy `ValidMemRegionRootType`; if omitted, defaults to `cod::WorksRoot`.
- `repl.scope` is a header path relative to the project root (or an include path). The REPL includes it verbatim in the runner TU.
- The scope header should make available the functions/types that submissions may call. It may also provide helpers to access the DBMR path (e.g., reading `COD_WORKS_PATH`).
- The REPL validates `TYPE_UUID` on open; mismatch produces a clear error suggesting opening a matching DBMR or recreating.

YAML conversion:
- As with all project metadata, YAML parsing/serialisation is implemented via the `YamlConvertible` concept and kept separate from allocation logic.

---

## Error Reporting & Diagnostics

- Build failures: show compiler and linker diagnostics verbatim, with the generated TU path and a `--keep-tmp` option to preserve artefacts for debugging.
- Runtime failures: show exit status / signal; forward child stdout/stderr to the REPL.
- Workspace errors: DBMR open/resize/type-mismatch errors include file path, expected vs actual `TYPE_UUID`, and guidance.

---

## Safety, Isolation, and Determinism

- No code injection into the REPL host process — each run is a separate process.
- DBMR-backed state allows cooperative multi-process tooling without copying.
- Deterministic toolchain flags are derived from the manifest; network is never touched by `cod`.

---

## Future Enhancements

- Optional helpers in a tiny header (opt-in) to ease DBMR access from the project’s scope header (kept outside the REPL by default).
- `%bench`-like mode as a flag to compile with `-O3` and time execution.
- Multi-root workspaces for multi-project sessions, mediated by `global_ptr` and explicit root registries.
- Optional CCache-like backend for runner builds keyed by submission hashes.
- Precompiled headers for faster startup.

---

## Open Questions

- What is the minimal contract between the runner and the project scope for DBMR access? Environment variable name and argv position are proposed; the scope header can decide how to consume it.
- Policy for default DBMR capacity and growth strategy. DBMR supports resize on demand, but we need user-friendly defaults and limits.
- Concurrency semantics if multiple writers are unavoidable — likely to require advisory locking at the DBMR file level.

---

## Implementation Notes (Stage 1)

- `include/cod.hh`:
  - Define `cod::WorksRoot` satisfying the regional type rules (no copy/move, in-region construction, `TYPE_UUID`).
- `cod/main.cc`:
  - Parse `--works/-w` and `--project`.
  - Open or create the DBMR; validate root type UUID.
  - Treat each REPL submission as ordinary C++ code; wrap it minimally into a runner TU that includes the configured scope header(s) and embeds the submitted statements.
  - Drive the build pipeline and child process execution.
- Keep YAML conversions in `*_yaml.hh` files separate from core allocation logic.
