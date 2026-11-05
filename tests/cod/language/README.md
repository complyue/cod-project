# CoD native source prototype

Prototype reference revision: 1 · Recorded: 2026-11-05.

## Design authority

- **Registry / scope:** implementation evidence associated with [D-10 revision 5](../../../design/DesignAuthorityRegistry.md#d-10), under the direct [H-108](../../../design/DesignAuthorityRegistry.md#h-108) prototype request. This reference describes the runnable subset; it is not a qualified specification of the full language.
- **Value lineage:** C-35/C-36/C-37 and P-84/P-85/P-86 connect identity, scoped interpretation and pure computation to inspectable programs; C-38/P-87 connects the fixture to application-driven validation. These candidate paths retain their registry status.
- **Human baseline / invariants:** H-96–H-111 supply the local symbol, callable, module, capture and storage rules. Preserve definition-value `perform`, current-binding `behave`, scope restoration, complete scanning before execution composition, and compile-time missing-binding errors. User values have no host-heap lifetime.
- **Acceptance / non-goals:** actual source-to-native compilation and execution, scope/module examples, diagnostics before execution, and an exact-integer incoming gather. General regional allocation, ADT/GADT syntax, persistent closures, concurrency and neural learning are outside this milestone.
- **Assessment:** [K-214](../../../design/DesignAuthorityRegistry.md#k-214) records implementation and measured evidence. Syntax and numerical conventions below are experimental prototype choices, not additional human-confirmed language decisions.

## Run

From the repository root, with its existing `build` toolchain:

```sh
build/bin/cod --run tests/cod/language/scopes.cod
build/bin/cod --run tests/cod/language/using.cod
build/bin/cod --run tests/cod/language/import.cod
build/bin/cod --run tests/cod/language/frame.cod
```

`--run` parses the entry and its complete local dependency graph, checks demanded call paths, generates native source, invokes the project's Clang/LLVM compiler and linker, then executes the result. It cleans its own temporary source, bitcode and executable. It does not require a `CodProject.yaml`, modify a DBMR, or pass CoD text through as C++. The existing `--eval` C++ mode remains available separately.

```sh
build/bin/cod --check tests/cod/language/using.cod -v
build/bin/cod --check tests/cod/language/using.cod --emit-cpp /tmp/cod-example.cc
```

`--check` checks and lowers without invoking native compilation or executing the program. `-v` lists scanned modules and execution-composition encounters. `--emit-cpp` saves the generated source for review; that source is a derived artifact. `--run` and `--check` each accept one entry and cannot be combined with `--eval`.

## Supported source forms

```text
symbol @x;
symbol @p;
using "std:io" @print;
function twice(value: i64) -> i64 { value * 2 }
global seed: i64 = twice(3);
effect @x = seed;
procedure @p -> i64 { @x }
effect @x = 9;
behave @print(perform @p);  // 6: definition-time value
behave @print(behave @p);   // 9: current dynamic binding
```

- Scalar `i64`, `bool` and `void`; decimal integer literals, `true`/`false`, arithmetic, comparisons, `!`, `&&` and `||`.
- Lexically named functions with typed parameters and an optional result annotation; symbol-named procedures with the same scalar parameters/results. A no-argument procedure can omit its parameter list. Calls to functions use `name(...)`; procedure calls use `perform @name(...)` or `behave @name(...)`.
- `effect @name = value` binds for the enclosing scope and yields that value. `@name` reads. Nested braces restore bindings on exit. Ordinary assignment is rejected.
- `if (condition) { ... } else { ... }`, with matching branch result types. Blocks yield their last expression; an empty block yields `void`. Semicolons separate expressions and a trailing semicolon does not discard the last value.
- Module-level `symbol`, `function`, `procedure`, immutable bits `global`, resource declarations and expressions. Global initializers must be bits literals or function calls. Local declarations, loops and arrays are not implemented.
- `using "module.cod" @resource as @alias;` retains symbol identity and a source procedure's definition/captures. `import "module.cod" @resource;` redefines a procedure body in the receiving lexical/dynamic context. Its lexical names and symbols must resolve there. Lexical functions and immutable globals can be brought in with `using "module.cod" name;`. Function-body `import` is unsupported.
- Synthetic system module `std:io` provides `@print` for one scalar and `@assert` for one boolean. These are procedures; functions cannot invoke them or other effects.

Prototype expression evaluation is left-to-right; boolean operators short-circuit. Both conditional branches are checked. A symbolic write in a conditional logical operand requires an explicit nested scope. Arithmetic overflow and division by zero fail the native program with exit 70 rather than invoking C++ undefined behavior. These sequencing, block-result and numerical choices remain reviewable prototype conventions.

## Module and storage boundary

All dependency files are parsed before any execution is emitted. A syntax or required-binding error rejects the whole entry before any top-level print can run. Procedure registration during scanning does not capture values; capture occurs at its definition event in the resulting execution.

Every `using`/`import` resource encounter incorporates source execution, including a symbol-only reference. The source's scope then restores its effects. Only selected effective resources transfer; procedure captures survive as copied scalar values. Imported aliases must have reached their `using`/`import` encounter before use, including use in a procedure definition's capture analysis.

The subset accepts acyclic, unambiguous module composition. A second encounter with an effectful source module is diagnosed; use locally bound resources after the first encounter. Declaration-only vocabulary/function modules and the built-in module may be reused because they have no execution effects. This restriction leaves the general repetition/cycle policy open.

Local symbol identity uses the canonical module path and declaration, preserving aliases and separating distinct source files. Versioned package identity and first-class runtime symbol selection are not implemented. Calls are specialized along known paths; recursion, procedure-valued scope results and general runtime higher-order dispatch are unsupported.

The generated program uses fixed thread-local scalar binding slots and stack-based saved values. Known procedures are specialized by the compiler. Compiler ASTs and system machinery can use the host heap; generated user data and scalar captures do not. Regional pointer syntax, lifetime checking, region-backed closures and cross-region representation are not implemented. The presence of TLS does not establish tested multi-thread or reentrant execution support.

## Evidence

[validation.json](validation.json) records exact commands, outputs, exit codes, source hashes and the validated executable hash for 22 direct checks. It also records the successful normal source build, native compiler/linker/runner process trace, frontend-only check and C++ compatibility check. No repository test suite was invoked.

| Fixture | Observation |
| --- | --- |
| [scopes.cod](scopes.cod), [transitive.cod](transitive.cod) | Captured/current values, body-local restoration, repeat calls, transitive `behave` requirements |
| [using.cod](using.cod), [import.cod](import.cod) | Source context gives `401,408`; receiving redefinition gives `907,908`; each source executes first and restores its effects |
| [symbol-trigger.cod](symbol-trigger.cod), [function-resource.cod](function-resource.cod), [global-resource.cod](global-resource.cod) | Symbol, function and global references each incorporate observable source execution |
| [identity.cod](identity.cod), [procedure-rebinding.cod](procedure-rebinding.cod), [effect-result.cod](effect-result.cod) | Alias identity versus distinct files, scoped procedure rebinding, newly bound expression result |
| [frame.cod](frame.cod) | A = `[2,5]` produces B = `[15,14]`; reverse target traversal agrees, source A remains unchanged until both B values are ready, then publication exposes `[15,14]` |
| Invalid fixtures | Missing binding/capture, effect access in a function, incompatible symbol types, plain assignment, broken dependency and premature alias use fail with source-location diagnostics before execution |

The frame fixture uses scalar values and two explicit calls, not regional arrays or a general frame loop. It validates the H-94 data-flow meaning; it does not enforce an immutable-view type or establish SNN dynamics, training, scalability or performance. The broader [semantic case catalog](../../../applications/steady-snn/research/cod-language-scope-cases.json) includes unimplemented cases; these checks do not mark that entire catalog as passed.

The implementation is in [lang_syntax.hh](../../../cod/lang_syntax.hh), [lang_frontend.hh](../../../cod/lang_frontend.hh) and [main.cc](../../../cod/main.cc). Source edits build with exactly `cmake --build ./build`. The existing default build graph also refreshed runtime sub-build/configuration outputs during validation; no build-configuration source or toolchain was changed.
