# CoD native source prototype

Prototype reference revision: 2 · Recorded: 2026-11-05.

## Design authority

- **Registry / scope:** implementation evidence associated with [D-10 revision 8](../../../design/DesignAuthorityRegistry.md#d-10), under the direct [H-108](../../../design/DesignAuthorityRegistry.md#h-108) prototype request and [H-118](../../../design/DesignAuthorityRegistry.md#h-118) symbol-syntax implementation request. This reference describes the runnable subset; it is not a qualified specification of the full language.
- **Value lineage:** C-35/C-36/C-37 and P-84/P-85/P-86 connect identity, scoped interpretation and pure computation to inspectable programs; C-38/P-87 connects the fixture to application-driven validation. These candidate paths retain their registry status.
- **Human baseline / invariants:** H-96–H-118 supply the local symbol, callable, module, capture and storage rules. Preserve definition-value `perform`, current-binding `behave`, scope restoration, complete scanning before execution composition, and compile-time missing-binding errors. User values have no host-heap lifetime.
- **Acceptance / non-goals:** actual source-to-native compilation and execution, scope/module examples, diagnostics before execution, and an exact-integer incoming gather. General regional allocation, ADT/GADT syntax, persistent closures, concurrency and neural learning are outside this milestone.
- **Assessment:** [K-217](../../../design/DesignAuthorityRegistry.md#k-217) records implementation and measured evidence. The symbol-selection/map forms implement H-118; other experimental syntax, numerical conventions and unsupported edge cases do not settle the full language.

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
symbol @print;
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
- `symbol @local;` declares a fresh identity. `symbol from "vocabulary.cod" @source as @local;` selects an existing source identity; omitting `as` retains its spelling. Neither provides an initial runtime value.
- `using "module.cod" @resource as @local;` installs a source procedure with its definition/captures under an existing destination symbol. `import "module.cod" @resource as @local;` redefines its body in the receiving lexical/dynamic context and installs the result there. The destination must be declared or selected before the encounter, even when `as` is omitted. Its identity may differ from the source resource identity.
- `import "module.cod" @resource as @local with { @target as @tgt_neuron; };` supplies the receiving module’s `@target` identity for the source-body role `@tgt_neuron`. Unlisted body aliases use the same name in the receiving module. Translation precedes capture/checking, covers reads, writes and procedure calls, and creates no runtime alias map or extra local alias. `using` does not accept a body map.
- Lexical functions and immutable globals can be selected with `using "module.cod" name;`. Function-body `import` is unsupported.
- Synthetic system module `std:io` provides `@print` for one scalar and `@assert` for one boolean. These are procedures; functions cannot invoke them or other effects. Declare their destination symbols before `using`; built-ins have no source body to `import`.

Prototype expression evaluation is left-to-right; boolean operators short-circuit. Both conditional branches are checked. A symbolic write in a conditional logical operand requires an explicit nested scope. Arithmetic overflow and division by zero fail the native program with exit 70 rather than invoking C++ undefined behavior. These sequencing, block-result and numerical choices remain reviewable prototype conventions.

## Module and storage boundary

All dependency files are parsed before any execution is emitted. A syntax or required-binding error rejects the whole entry before any top-level print can run. Procedure registration during scanning does not capture values; capture occurs at its definition event in the resulting execution.

Every `symbol from`, `using` and `import` encounter incorporates source execution. The source’s scope then restores its effects. Identity-only selection does not retain an incidental runtime value; selected procedures retain copied scalar captures. Imported aliases must have reached their `symbol from` encounter before use, including use in capture analysis. A body map does not rewrite source initialization or other existing procedures.

Unknown local suppliers/source-body roles and duplicate mapping roles are diagnosed. Splitting aliases of one source identity across multiple receiving identities is outside the prototype; mapping them to one identity is supported. Broader mapping policy remains open.

The subset accepts acyclic, unambiguous module composition. A second encounter with an effectful source module is diagnosed; use locally bound resources after the first encounter. Declaration-only vocabulary/function modules and the built-in module may be reused because they have no execution effects. This restriction leaves the general repetition/cycle policy open.

Local symbol identity uses the canonical module path and declaration, preserving aliases and separating distinct source files. Versioned package identity and first-class runtime symbol selection are not implemented. Calls are specialized along known paths; recursion, procedure-valued scope results and general runtime higher-order dispatch are unsupported.

The generated program uses fixed thread-local scalar binding slots and stack-based saved values. Known procedures are specialized by the compiler. Compiler ASTs and system machinery can use the host heap; generated user data and scalar captures do not. Regional pointer syntax, lifetime checking, region-backed closures and cross-region representation are not implemented. The presence of TLS does not establish tested multi-thread or reentrant execution support.

## Evidence

[validation.json](validation.json) records exact commands, outputs, exit codes, source hashes and the validated executable hash for 49 source fixtures (23 successful native programs, one expected native arithmetic failure and 25 expected frontend rejections) plus one frontend-only check. It records the successful normal source build and complete CoD component suite: language, six C++ evaluation checks, eight cache checks and workspace checks.

The suite discovers [test_language.sh](../test_language.sh), which runs [run_tests.py](run_tests.py) against expected results in [cases.json](cases.json). To repeat the component suite with the existing build toolchain:

```sh
COD_TEST_TOOLCHAIN=build tests/cod/run-all.sh
```

For the language group alone, use `COD_TEST_TOOLCHAIN=build tests/cod/test_language.sh`. Optional fixture stems select individual cases; the frontend-only check still runs. Set `COD_LANGUAGE_REPORT` to save a JSON result record. These tests compile and run accepted programs and require rejected programs to fail before any preceding top-level output.

| Fixture | Observation |
| --- | --- |
| [scopes.cod](scopes.cod), [transitive.cod](transitive.cod) | Captured/current values, body-local restoration, repeat calls, transitive `behave` requirements |
| [using.cod](using.cod), [import.cod](import.cod) | Source context gives `401,408`; receiving redefinition gives `907,908`; each source executes first and restores its effects |
| [symbol-trigger.cod](symbol-trigger.cod), [function-resource.cod](function-resource.cod), [global-resource.cod](global-resource.cod) | Symbol, function and global references each incorporate observable source execution |
| [identity.cod](identity.cod), [procedure-rebinding.cod](procedure-rebinding.cod), [effect-result.cod](effect-result.cod) | Alias identity versus distinct files, scoped procedure rebinding, newly bound expression result |
| [identity-restore.cod](identity-restore.cod), [using-destination.cod](using-destination.cod), [using-vocabulary-destination.cod](using-vocabulary-destination.cod) | Identity selection retains no source value; fresh or vocabulary-selected destinations install procedures independently of the source identity |
| [import-mapping.cod](import-mapping.cod), [import-default-identity.cod](import-default-identity.cod), [reimport-mapping.cod](reimport-mapping.cod) | Explicit supplier/role mapping, same-name receiving identities, receiving captures and reimported-body translation |
| [mapping-effects.cod](mapping-effects.cod), [mapping-perform-call.cod](mapping-perform-call.cod), [mapped-behave-later.cod](mapped-behave-later.cod) | Mapped writes/calls, transitive capture requirements, restoration and later caller bindings for `behave` |
| [frame.cod](frame.cod) | A = `[2,5]` produces B = `[15,14]`; reverse target traversal agrees, source A remains unchanged until both B values are ready, then publication exposes `[15,14]` |
| Invalid fixtures | Missing binding/capture, effect access in a function, incompatible symbol types, plain assignment, broken dependency and premature alias use, undeclared destinations, invalid mapping names/direction and unsupported source resources fail with source-location diagnostics before execution |

The frame fixture uses scalar values and two explicit calls, not regional arrays or a general frame loop. It validates the H-94 data-flow meaning; it does not enforce an immutable-view type or establish SNN dynamics, training, scalability or performance. The broader [semantic case catalog](../../../applications/steady-snn/research/cod-language-scope-cases.json) includes unimplemented cases; these checks do not mark that entire catalog as passed.

The implementation is in [lang_syntax.hh](../../../cod/lang_syntax.hh), [lang_frontend.hh](../../../cod/lang_frontend.hh) and [main.cc](../../../cod/main.cc). Source edits build with exactly `cmake --build ./build`. The existing default build graph also refreshed runtime sub-build/configuration outputs during validation; no build-configuration source or toolchain was changed.
