# CoD native source prototype

Prototype reference revision: 4 · Recorded: 2026-11-05.

## Design authority

- **Registry / scope:** implementation evidence associated with [D-10 revision 13](../../../design/DesignAuthorityRegistry.md#d-10), under the direct [H-108](../../../design/DesignAuthorityRegistry.md#h-108) prototype request and [H-118](../../../design/DesignAuthorityRegistry.md#h-118) symbol-syntax implementation request, refined by [H-121](../../../design/DesignAuthorityRegistry.md#h-121) for combined using and identity checks. This reference describes the runnable subset; it is not a qualified specification of the full language.
- **Value lineage:** C-35/C-36/C-37 and P-84/P-85/P-86 connect identity, scoped interpretation and pure computation to inspectable programs; C-38/P-87 connects the fixture to application-driven validation. These candidate paths retain their registry status.
- **Human baseline / invariants:** H-96–H-121 supply the local symbol, callable, module, capture and storage rules. Preserve definition-value `perform`, current-binding `behave`, scope restoration, complete scanning before execution composition, and compile-time missing-binding errors. User values have no host-heap lifetime.
- **Acceptance / non-goals:** actual source-to-native compilation and execution, scope/module examples, diagnostics before execution, and an exact-integer incoming gather. General regional allocation, sum/GADT syntax, persistent closures, concurrency and neural learning are outside this checkpoint. Named bits products, FP32/FP64 values and parameter-type methods are implemented.
- **Assessment:** [K-225](../../../design/DesignAuthorityRegistry.md#k-225) records this typed-value/method/composition checkpoint and actual validation. H-122/H-123 authorize this language/runtime increment; H-139–H-141 specify complex composition, once-only incorporation and initial inlining. K-224 records the current module contract. [K-219](../../../design/DesignAuthorityRegistry.md#k-219) records implementation and measured evidence. The symbol-selection/map forms implement H-118; combined using and identity checks implement H-120/H-121; other experimental syntax, numerical conventions and unsupported edge cases do not settle the full language.

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
using symbol "std:io" @print;
function twice(value: i64) -> i64 { value * 2 }
global seed: i64 = twice(3);
effect @x = seed;
procedure @p -> i64 { @x }
effect @x = 9;
behave @print(perform @p);  // 6: definition-time value
behave @print(behave @p);   // 9: current dynamic binding
```

- Scalars `i64`, `f32`, `f64`, `bool` and `void`; decimal integers, floating literals with optional `f32`/`f64` suffix, `true`/`false`, arithmetic, comparisons, `!`, `&&` and `||`. Unsuffixed floating literals use `f64`; arithmetic does not implicitly mix widths. Floating operators use native IEEE arithmetic; signed integer arithmetic checks overflow.
- Lexically named functions with typed parameters and an optional result annotation; symbol-named procedures with the same scalar parameters/results. A no-argument procedure can omit its parameter list. Calls to functions use `name(...)`; procedure calls use `perform @name(...)` or `behave @name(...)`.
- `effect @name = value` binds for the enclosing scope and yields that value. `@name` reads. Nested braces restore bindings on exit. Ordinary assignment is rejected.
- `if (condition) { ... } else { ... }`, with matching branch result types. Blocks yield their last expression; an empty block yields `void`. Semicolons separate expressions and a trailing semicolon does not discard the last value.
- Module-level `symbol`, `function`, `procedure`, immutable bits `global`, resource declarations and expressions. Global initializers must be bits literals or function calls. Local declarations, loops and arrays are not implemented.
- `symbol @local;` declares a fresh identity. `symbol from "vocabulary.cod" @source as @local;` selects an existing source identity; omitting `as` retains its spelling. Neither provides an initial runtime value.
- `using "module.cod" @resource as @local;` installs a source procedure with its definition/captures under an existing destination symbol. `import "module.cod" @resource as @local;` redefines its body in the receiving lexical/dynamic context and installs the result there. The destination must be declared or selected before the encounter, even when `as` is omitted. For ordinary using, omitting `as` requires the destination to share the source identity; a different identity requires explicit `as`, even `using "module.cod" @resource as @resource;`. The outer import rule remains unchanged.
- `using symbol "module.cod" @resource as @local;` selects both the source identity and its procedure in one encounter; `as` is optional. An absent local name becomes an alias of the source identity. An existing name must resolve to that same identity; otherwise use ordinary `using` with an explicit destination. Equal procedure values or spellings do not establish identity equality.
- `import "module.cod" @resource as @local with { @target as @tgt_neuron; };` supplies the receiving module’s `@target` identity for the source-body role `@tgt_neuron`. Unlisted body aliases use the same name in the receiving module. Translation precedes capture/checking, covers reads, writes and procedure calls, and creates no runtime alias map or extra local alias. `using` does not accept a body map.
- Lexical functions and immutable globals can be selected with `using "module.cod" name;`. Function-body `import` is unsupported.
- Synthetic system module `std:io` provides `@print` for one scalar and `@assert` for one boolean. These are procedures; functions cannot invoke them or other effects. Use `using symbol "std:io" @print;`, or explicitly select an existing destination. Built-ins have no source body to `import`.

Prototype expression evaluation is left-to-right; boolean operators short-circuit. Both conditional branches are checked. A symbolic write in a conditional logical operand requires an explicit nested scope. Arithmetic overflow and division by zero fail the native program with exit 70 rather than invoking C++ undefined behavior. These sequencing, block-result and numerical choices remain reviewable prototype conventions.

## Typed values and methods

```text
type Position { index::i64; weight::f32; }
function (+)(p::Position, step::i64)::Position {
    Position(p.index + step, p.weight)
}
function score(p::Position)::f32 { p.weight }
function score(x::f32)::f32 { x * 2.0f32 }
function inspect(p::Position)::i64 {
    let next = p + 1;
    next.index
}
```

Products are nominal bits types identified by the defining module/declaration; equal field shapes do not make them interchangeable. Positional construction checks field count/types. Nested products, field reads, function inputs/results, bits globals and bits effect captures are supported. `using "module" Position as P` preserves identity. Recursive by-value layouts are rejected. These products contain bits fields; they are not implicitly regional objects.

`let` binds an immutable lexical value inside an expression scope; module values use typed `global` declarations. Parameter annotations accept `:` or `::`; results accept `->` or `::`. Methods dispatch on the complete argument-type tuple. Selected function sets from multiple modules can share a local method name; duplicate identical matches are diagnosed when demanded. Operator methods use names such as `(+)`; `p + 1` and `(+)(p, 1)` select the same user-defined method. Unary/binary arities are distinct. Short-circuit operators remain control expressions.

Pure inline scopes use fresh native names and do not add needless C++ nesting. Procedure scopes retain restoration semantics. Machine-code reuse is deferred; [stress_tests.py](stress_tests.py) measures generated-source size and compile/link/run time on deep and shared dependency graphs. Native code is optimized by the existing compiler, so source expansion is not a claim about runtime stack depth.

## Module and storage boundary

All dependency files are parsed before any execution is emitted. A syntax or required-binding error rejects the whole entry before any top-level print can run. Procedure registration during scanning does not capture values; capture occurs at its definition event in the resulting execution.

After the complete scan, the first cross-module encounter incorporates that module’s top-level runtime semantics into the resulting program. Later `symbol from`, `using` and `import` encounters reuse the incorporated module and its resources. The source’s scope then restores its effects. Identity-only selection does not retain an incidental runtime value; selected procedures retain copied bits captures. Imported aliases must have reached their `symbol from` or introducing `using symbol` encounter before use, including use in capture analysis. Combined using is one source encounter; matching an existing identity still installs the selected procedure. A body map does not rewrite source initialization or other existing procedures.

Unknown local suppliers/source-body roles and duplicate mapping roles are diagnosed. Splitting aliases of one source identity across multiple receiving identities is outside the prototype; mapping them to one identity is supported. Broader mapping policy remains open.

The subset accepts acyclic module composition including shared and diamond dependencies. Each module is incorporated once, in first-encounter order; its effects are not rerun or recaptured for another caller. A demanded initialization cycle is diagnosed; general cyclic-initialization semantics remain open. Compiler encounters do not execute user code.

Local symbol identity uses the canonical module path and declaration, preserving aliases and separating distinct source files. Versioned package identity and first-class runtime symbol selection are not implemented. Calls are specialized along known paths; recursion, procedure-valued scope results and general runtime higher-order dispatch are unsupported.

The generated program uses fixed thread-local typed binding slots and stack-based saved values. Known procedures are specialized by the compiler. Compiler ASTs and system machinery can use the host heap; generated user data and scalar captures do not. Regional pointer syntax, lifetime checking, region-backed closures and cross-region representation are not implemented. The presence of TLS does not establish tested multi-thread or reentrant execution support.

## Evidence

[validation.json](validation.json) records the latest checkpoint’s commands, outputs, exit codes, source/executable hashes and component-suite evidence. [stress-validation.json](stress-validation.json) records generated dependency/call-graph checks. Measurements are local observations, not portability or full-language acceptance claims.

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
| [using-symbol-basic.cod](using-symbol-basic.cod), [using-symbol-source.cod](using-symbol-source.cod) | Combined selection, renamed aliases sharing dynamic values, source initialization once, captured/current calls and restoration |
| [using-shared-identity.cod](using-shared-identity.cod), [using-symbol-shared-identity.cod](using-symbol-shared-identity.cod), [using-symbol-reexport.cod](using-symbol-reexport.cod) | Shared vocabulary/re-export identity permits abbreviation and still installs the selected procedure |
| [using-explicit-self.cod](using-explicit-self.cod), [using-identity-mismatch.cod](using-identity-mismatch.cod), [using-symbol-same-value.cod](using-symbol-same-value.cod) | Explicit self-`as` keeps identities distinct; omitted `as` and combined mismatches fail even when procedure values agree |
| [frame.cod](frame.cod) | A = `[2,5]` produces B = `[15,14]`; reverse target traversal agrees, source A remains unchanged until both B values are ready, then publication exposes `[15,14]` |
| Invalid fixtures | Missing binding/capture, effect access in a function, incompatible symbol types, plain assignment, broken dependency and premature alias use, undeclared destinations, invalid mapping names/direction and unsupported source resources fail with source-location diagnostics before execution |

The frame fixture uses scalar values and two explicit calls, not regional arrays or a general frame loop. It validates the H-94 data-flow meaning; it does not enforce an immutable-view type or establish SNN dynamics, training, scalability or performance. The broader [semantic case catalog](../../../applications/steady-snn/research/cod-language-scope-cases.json) includes unimplemented cases; these checks do not mark that entire catalog as passed.

The implementation is in [lang_syntax.hh](../../../cod/lang_syntax.hh), [lang_frontend.hh](../../../cod/lang_frontend.hh) and [main.cc](../../../cod/main.cc). Source edits build with exactly `cmake --build ./build`. The existing default build graph also refreshed runtime sub-build/configuration outputs during validation; no build-configuration source or toolchain was changed.
