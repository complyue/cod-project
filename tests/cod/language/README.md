# CoD LLVM source prototype

Prototype reference revision: 64 · Recorded: 2026-11-06.

## Design authority

- **Registry / scope:** implementation reference for [D-10r34](../../../design/DesignAuthorityRegistry.md#d-10), the existing D-07 language/runtime increment and D-11r13. The complete design contracts remain pending-human; this reference describes the bounded implementation. [K-290](../../../design/DesignAuthorityRegistry.md#k-290) records H-161’s reserved module words and H-162’s distinct assignment family; [K-291](../../../design/DesignAuthorityRegistry.md#k-291) records H-163/H-164’s indexed `([]=)` syntax. [K-292](../../../design/DesignAuthorityRegistry.md#k-292) records the current 140 source and five composition checks; other evidence retains its own run/input scope.
- **Value lineage:** C-02 supplies LLVM native execution; C-35–C-37/P-84–P-86 track nominal identity, scoped effects and pure computation through C-10. Formal candidate derivations retain their registry status.
- **Human baseline:** H-96–H-141 supply the local symbol, callable, module, storage, snapshot and UoM rules; H-122/H-123 authorize this language/runtime increment. [H-143](../../../design/DesignAuthorityRegistry.md#h-143) selects one direct LLVM path. H-144/H-145 require contract-focused tests and ongoing simplification. H-146 accepts the temporary first-field self-reference limitation. H-147 continues the work for ten active hours, excluding waiting. H-148 requires per-argument conversion before the next argument. H-149 permits nested local shadowing and requires unique declarations within each scope. H-150 rejects reserved expression names in lexical value declarations.
- **Operation baseline:** H-153–H-155 require reserved ordinary names, parenthesized intrinsic overload declarations and function/procedure overloads under one operation. H-158 keeps intrinsic representation separate from CoD symbols; H-159 selects current-binding procedure invocation. H-160 defers new decisions to the work-period review. [K-282](../../../design/DesignAuthorityRegistry.md#k-282) records the bounded implementation; [K-283](../../../design/DesignAuthorityRegistry.md#k-283) records definition-time identity resolution and composed native-wrapper/index examples. [K-284](../../../design/DesignAuthorityRegistry.md#k-284) records token-category separation and callable-kind diagnostics. [K-285](../../../design/DesignAuthorityRegistry.md#k-285) aligns the UoM document with the same callable boundary while preserving pure conversions. [K-286](../../../design/DesignAuthorityRegistry.md#k-286) records current cross-document scope and refreshed numeric/regional/gather evidence. [K-287](../../../design/DesignAuthorityRegistry.md#k-287) records shared compiler binding snapshots and their measured metadata cost.
- **Invariants / non-goals:** preserve captured `perform`, caller-bound `behave`, restoration, once-only post-scan module incorporation, compile-time missing-binding errors and region-only user storage. No general GADT, cross-region fields, persistent closures, concurrency or SNN dynamics/training is claimed.
- **Acceptance / evidence:** execute actual native CoD programs; reject invalid types/bindings/purity/lifetimes before execution; preserve region sharing/cycles through relocation; compare typed SoA arithmetic with an independent reference. [K-276](../../../design/DesignAuthorityRegistry.md#k-276) records deep imported-body and regional snapshot composition; each evidence file retains its timestamp and source hashes. [K-289](../../../design/DesignAuthorityRegistry.md#k-289) and the [work-period record](work-period-validation.json) preserve H-147’s active-time accounting and scoped acceptance.

## Run

From the repository root, with its existing `build` toolchain:

```sh
build/bin/cod --run tests/cod/language/scopes.cod
build/bin/cod --run tests/cod/language/using.cod
build/bin/cod --run tests/cod/language/import.cod
build/bin/cod --run tests/cod/language/region-gather.cod
```

`--run` parses the entry and its complete local dependency graph, checks demanded call paths, emits LLVM IR/bitcode directly, invokes the existing compiler/linker with a small native system runtime, then executes the result. It removes its own temporary runtime source, bitcode and executable. The source mode needs no `CodProject.yaml` or DBMR. The existing `--eval` C++ facility remains a separate driver mode.

```sh
build/bin/cod --check tests/cod/language/using.cod -v
build/bin/cod --check tests/cod/language/using.cod --emit-llvm /tmp/cod-example.ll --emit-bc /tmp/cod-example.bc
```

`--check` checks and lowers without invoking native compilation or executing the program. `-v` lists scanned modules and execution-composition encounters. `--emit-llvm` and `--emit-bc` save textual and binary forms of the same execution IR. `--emit-runtime FILE` writes the small C++ system services when separately linking an emitted artifact. CoD business logic is emitted only as LLVM IR; there is no backend selector. These outputs are derived artifacts, not persistent semantic graphs. LLVM/bitcode and native runtime source must be emitted together for external linking; their system-service interface is internal to that compiler revision. `--run` and `--check` each accept one entry and cannot be combined with `--eval`.

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

- Scalars `i64`, `u8`, `u32`, `u64`, `f32`, `f64`, `bool` and `void`; decimal integers with optional leading zeroes, floating literals with optional `f32`/`f64` suffix, `true`/`false`, arithmetic, comparisons, `!`, `&&` and `||`. Unsuffixed floating literals use `f64`; arithmetic does not implicitly mix widths. Floating operators use native IEEE arithmetic; signed integer arithmetic checks overflow.
- Lexically named functions with typed parameters and an optional result annotation; symbol-named procedures support the same typed values and regional references. A no-argument procedure can omit its parameter list. Calls to functions use `name(...)`; symbol-named procedure calls use `perform @name(...)` or `behave @name(...)`. Operator/intrinsic procedure overloads use operation syntax and current bindings.
- `effect @name = value` binds for the enclosing scope and yields that value. `@name` reads. Nested braces restore bindings on exit. Ordinary `@name = value` rebinding is rejected. The prototype assigns one value type to each symbol identity across the program. A conflicting binding reports the symbol and both types, including nominal module origins even when local aliases have the same spelling.
- `if (condition) { ... } else { ... }`, with matching branch result types. Blocks yield their last expression; an empty block yields `void`. Semicolons separate expressions and a trailing semicolon does not discard the last value.
- Module-level `symbol`, `function`, `procedure`, immutable bits `global`, resource declarations and expressions. Global initializers must be bits literals or function calls. Their selected outer callable must be pure, including for overloaded operation spellings; native mutation and region-resource primitives remain procedural. Local immutable `let` bindings, bounded loops and declared regional arrays support numerical kernels.
- `symbol @local;` declares a fresh identity. `symbol from "vocabulary.cod" @source as @local;` selects an existing source identity; omitting `as` retains its spelling. Neither provides an initial runtime value. Symbol tokens such as `@effect` are distinct from lexical keywords; quoted module paths are neither keywords nor operators.
- `using "module.cod" @resource as @local;` installs a source procedure with its definition/captures under an existing destination symbol. `import "module.cod" @resource as @local;` redefines its body in the receiving lexical/dynamic context and installs the result there. The destination must be declared or selected before the encounter, even when `as` is omitted. For ordinary using, omitting `as` requires the destination to share the source identity; a different identity requires explicit `as`, even `using "module.cod" @resource as @resource;`. The outer import rule remains unchanged.
- `using symbol "module.cod" @resource as @local;` selects both the source identity and its procedure in one encounter; `as` is optional. An absent local name becomes an alias of the source identity. An existing name must resolve to that same identity; otherwise use ordinary `using` with an explicit destination. Equal procedure values or spellings do not establish identity equality.
- `import "module.cod" @resource as @local with { @target as @tgt_neuron; };` supplies the receiving module’s `@target` identity for the source-body role `@tgt_neuron`. Unlisted body aliases use the same name in the receiving module. Translation precedes capture/checking, covers reads, writes and procedure calls, and creates no runtime alias map or extra local alias. `using` does not accept a body map.
- Lexical functions and immutable globals can be selected with `using "module.cod" name;`. Function-body `import` is unsupported.
- Synthetic system module `std:io` provides `@print` for one scalar and `@assert` for one boolean. These are procedures; functions cannot invoke them or other effects. Use `using symbol "std:io" @print;`, or explicitly select an existing destination. Built-ins have no source body to `import`.

Prototype expression evaluation is left-to-right; boolean operators short-circuit. Both conditional branches are checked. A symbolic write in a conditional logical operand requires an explicit nested scope, including assignment of the same scalar or region resource again. Integer arithmetic overflow and integer division by zero fail the native program with exit 70 before LLVM can evaluate an invalid operation. LLVM-emitted checks include the offending CoD expression’s file, line and column; nested calls restore the enclosing expression’s location. Native allocation-service errors include the allocating expression’s location before the allocator message. This embeds diagnostic text without requiring source files at runtime or providing source-level debugging. These sequencing, block-result and numerical choices remain reviewable prototype conventions.

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

`let` binds an immutable lexical value inside an expression scope; module values use typed `global` declarations. Parameter annotations accept `:` or `::`; results accept `->` or `::`. Methods dispatch on the complete argument-type tuple. Selected function sets from multiple modules can share a local method name; duplicate identical matches are diagnosed when demanded. Ambiguous calls and unmatched named-function calls show the supplied argument types and up to four declaration names/locations, with an omitted-candidate count for larger sets. Ambiguity lists matching definitions; an unmatched overload call lists available definitions. Original names and function/procedure kinds remain visible through aliases. Single-method argument errors retain their direct type/count diagnostics. Operator methods use names such as `(+)`; `p + 1` and `(+)(p, 1)` select the same user-defined method. Supported eager unary/binary operator calls share the corresponding prefix/infix lowering, including scalar operations such as `(+)(2, 3)` and `(!)(false)`. Explicit calls remain valid function-call global initializers. Unary/binary arities are distinct. Short-circuit operators remain control expressions.

Demanded function/procedure calls are inlined into LLVM control flow. Compile-time call failures include source locations for the failing definition and its callers. Deep paths retain the innermost four and outermost four calls, with an omitted-call count; this is compiler context, not a runtime stack trace. The semantic checker carries LLVM values directly for expressions, captures and region owners. Typed TLS slots back scoped dynamic writes. WPO preserves definition-time binding values as SSA values; captured reads use those values along demanded typed paths. Current native services receive explicit arguments, and no foreign ABI exposes the private CoD TLS slots. Definition environments are compiler metadata, not runtime closures or extended region lifetimes. They share immutable maps of binding records; a definition retains a snapshot root, and a subsequent binding creates a distinct record. The compiler owns those records until lowering finishes. Captured-map membership preserves the distinction between frozen reads and fresh current bindings. Scoped writes retain their ordinary restoration. Machine-code reuse is deferred. [stress_tests.py](stress_tests.py) checks deep and shared dependency graphs and records IR/bitcode, executable and `.text` size plus separate checking/linking/process time, including 512 modules and 1,000 procedure specializations. Each checked artifact is linked with its emitted system services; the ordinary source corpus exercises `--run`. The pure chain takes loop-varying inputs and checks its result against independent modular arithmetic; a failing leaf in the same graph checks bounded diagnostics back to the entry call. Other fixed-input cases permit constant folding, so their sizes are not worst-case bounds. LLVM's standard optimization pipeline handles native optimization; no custom optimizer or backend abstraction is used.

[Capture metadata observations](capture-metadata-validation.json) measure registering many procedures while demanding one call. The shared-snapshot run at 1,024 definitions used 62,716 KiB peak compiler RSS and emitted 1,302 bytes of IR. These are compiler-process observations on the recorded Linux host, separate from native regional memory measurements.

The regional composition case redefines one procedure body through 96 modules, mapping it to a fresh local symbol identity and lexical helper at each step. Its deep pure calls observe intervening writes, while `perform` retains the original regional input and `behave` traverses a second region’s cyclic graph through the current binding.

## Intrinsic and procedural operation overloads

`function (len)(value::Box)::u64 { ... }` explicitly declares an intrinsic overload. `procedure (store)(cell::ref<Cell>, amount::i64)::i64 { ... }` declares a procedural overload; its body can access dynamic resources and mutate regional storage. Parenthesized eager operator names such as `(+)` support both kinds too. These are compiler operation names, independent of any `@len` or `@store` symbol. `(@len)` is a grouped symbolic read; neither `(@len)` nor `("len")` is an intrinsic declaration name.

Calls dispatch on argument types across both kinds. A function cannot call a selected procedure overload; the checker does not substitute a different pure method. Exact matches precede conversion-enabled alternatives, and equal matches remain ambiguous. Arguments execute once with each selected conversion before the next argument. A procedural operation uses current dynamic bindings, including the captured context of an enclosing ordinary `perform`. Pure UoM conversions retain their function-only rule.

`using "intrinsic-library.cod" (len);` selects the source method set with its lexical definitions. Procedural overloads require an operator/intrinsic destination, preserving the separate symbol-named procedure interface. Applicable user methods precede the native primitive fallback. `len(array)` therefore retains native array behavior when the selected methods apply to other types. `(len)(value)` is the explicit-call spelling.

A source-module helper can retain native fallback while the caller overloads the same primitive signature. In the regression example, an ordinary source procedure `@raw_store` calls `store(ref<f32>, f32)` in its own module, and the receiving module's procedural `(store)` overload delegates to it with `behave @raw_store(...)`. The helper retains its source lexical method table. This uses the existing module/procedure rules and requires no primitive-bypass syntax or intrinsic symbol identity.

Ordinary declarations cannot use literal/control keywords, intrinsic call names or built-in type names. The intrinsic group is `region`, `used_bytes`, `capacity_bytes`, `deref`, `len`, `ref_at`, `store`, `setfield`, `scalar`, `i64`, `u8`, `u32`, `u64`, `f32` and `f64`; `bool`, `void` and `ref` are also reserved ordinary names. Contextual names such as `in` and `else`, record field selectors and `@symbol` tokens retain their distinct grammar. For example, bare `function len(...)` is rejected, while a record field named `len` is valid.

The primary module-form words `type`, `regional`, `unit`, `array`, `using`, `import` and `conversion` are reserved against ordinary declarations and aliases under H-161. Field names such as `record.type` and symbol names such as `@using` retain their grammar.

`setfield` remains a native primitive with a compile-time field selector. Its overload declarations/aliases are rejected until that selector contract is designed. H-162 selects a distinct `(=)` family: explicit definitions, source selection and calls use the existing typed dispatcher. Its methods need not match `(store)` behavior. Indexed assignment `a[i] = value` calls the separate `([]=)(a, i, value)` family under H-163/H-164. It preserves both container and index types, evaluates each argument once, completes each selected conversion before the next argument, and yields the selected method result. Explicit calls and `using "module" ([]=);` use the same methods. A matching visible method is required; there is no implicit forwarding to `store` or `(=)`. Record-field assignment, intrinsic procedure-body import and explicit intrinsic `perform` syntax remain deferred. Existing `store`/`setfield` primitives implement writes within procedural overloads; no runtime method registry, user heap or new regional pointer representation is introduced.

The lean regression cases in `intrinsic-overloads.cod`, `intrinsic-pure-error.cod`, `intrinsic-reserved-name.cod` and `intrinsic-symbol-name.cod` cover cross-module mixed methods, lexical helper reuse, regional writes, native fallback, pure global initialization, capture/current context, an unused overload's missing binding, native-signature delegation, token-category separation, keyword rejection and purity failure. Existing conversion-order fixtures also exercise a procedural operator. `indexed-assignment-order.cod` verifies that a failing index conversion prevents RHS effects. `module-reserved-name.cod` rejects a primary module word at declaration time. [assignment-validation.json](assignment-validation.json) records this follow-up’s actual source/composition and focused boundary checks.

## Units and conversion selection

```text
unit metres::f64;
unit cm::f64;
conversion function cm_to_metres(x (cm))::metres {
    metres(scalar(x) / 100.0)
}
function distance(x (metres*))::f64 { scalar(x) }
```

A unit definition creates a distinct compile-time type identity over a numeric scalar. `metres(2.0)` constructs it from that exact representation; `scalar(x)` extracts the representation explicitly. Aliases preserve identity and do not add a runtime tag. Methods can define meaningful arithmetic on unit values.

`using units "units-library.cod" {m as metres, cm, mm};` selects one bundle of units and associated conversion methods. Repeated selections from the same source, duplicate identities through re-exports, and UoM import are rejected. Omitted-unit conversions are unavailable to automatic search, while source-bound private helper calls retain their lexical meaning. Selected conversion functions share the ordinary lexical method namespace: they can join an overload set but cannot collide with a global value or type name. Type aliases obey that same namespace rule, including aliases introduced by a later unit bundle.

`x (metres)` requires that identity; `x (metres*)` permits conversion. `x (metres*<-cm<-mm)` uses a direct conversion when available, otherwise the declared indirect route. Automatic graph search chooses a unique path. Multiple unresolved paths warn and continue using the shortest route with declaration-order tie breaking, reporting an alternative witness. `--warnings-as-errors` and `-Werror=unit-conversion` turn those warnings into errors. Paths use the call site's selected bundle; route names resolve where the parameter is defined. Exact argument-tuple methods are selected before conversion-enabled alternatives.

[units-procedure.cod](units-procedure.cod) composes a source definition and two receiver-context procedure redefinitions. Each module defines distinct same-named units and pure conversion/helper functions. Ordinary `using` retains the source types and captures; procedure `import` checks parameter/result types and body names in the receiver, with explicit symbol maps for its effects. The fixture checks all three definitions under captured `perform` and current-binding `behave`. It imports procedure bodies, not UoM definitions.

A `conversion function` requires one exact UoM parameter and an explicit UoM result; both automatic routing and direct calls enforce that signature. Conversions are pure functions; they cannot read/write effects or invoke procedures. The original argument is evaluated once. Calls evaluate each argument and complete its selected conversion before evaluating the next argument, under [H-148](../../../design/DesignAuthorityRegistry.md#h-148). A failing conversion prevents later argument effects. Functions, procedures and overloaded operator spellings share this order. Identity/route checks happen before native compilation, including when a preceding module initializer would print at runtime. The compiler does not enumerate cyclic walks or claim that distinct numeric formulas are equivalent.

## Bounded iteration and numeric representations

```text
function sum(n::u32)::u64 {
    fold (i in 0u32 .. n, total = 0u64) { total + u64(i) }
}
for (i in 0 .. 10) { behave @print(i); };
```

`for` is a void expression: its body is evaluated for effects, and its final value is discarded before the iteration scope closes. A reference to iteration-local storage may therefore be the final body expression without escaping that scope. `fold` returns its accumulator, initialized once and replaced by the body's value after each iteration. Ranges include the first endpoint and exclude the last; reversed/empty ranges execute zero times. Endpoints must share an integer type or one integer-backed UoM identity. Each iteration has its own dynamic scope, while procedure writes to regional storage persist until the owning region ends. These are native runtime loops, not compile-time iteration expansion. Product and unit accumulators retain their types.

A nested lexical scope may shadow an outer local; the initializer sees the outer binding, and leaving the scope restores its visibility. Declarations within one scope must be unique. Function parameters and loop index/accumulator bindings share their body’s declaration scope. Conditional branches and the conditional right operand of `&&`/`||` keep their local declarations inside that scope. [H-149’s naming rule](../../../design/SymbolicDynamicScope.md#local-value-shadowing) concerns lexical values; symbolic writes still require `effect`.

Parameters, `let` declarations, loop indices and fold accumulators reject [reserved expression names](../../../design/SymbolicDynamicScope.md#lexical-declaration-keywords), such as `true`, `null` or `if`, before execution. Contextual names such as `in` remain usable. Field selectors such as `.true` and symbolic names such as `@true` keep their identifier meaning.

Unsigned literals use `u8`, `u32` or `u64` suffixes. Integer arithmetic checks overflow, including unsigned underflow; it does not silently wrap. Explicit `i64`, `u8`, `u32`, `u64`, `f32` and `f64` calls convert raw numeric values. Integer narrowing checks range. Float-to-integer conversion truncates toward zero, then checks the mathematical integer range before the native cast; NaN/infinity fail. Integer-to-float and float narrowing may round; finite floating overflow fails. Explicit floating casts may underflow to zero, while an out-of-range source literal is diagnosed. Floating literal validation uses its declared precision, including representable subnormals and values near rounding boundaries. Bare scalar type names are reserved ordinary identifiers. Parenthesized numeric operations can have explicit methods for nominal inputs: the pool-index fixture defines `(u64)` for `SlotsPerSet` and `LocalSlot`, then uses `u64(capacity(j))` and `u64(slot)` at its raw arithmetic boundary. This neither enables implicit brand erasure nor adds an automatic UoM conversion route.

[pool-typed.cod](pool-typed.cod) composes separate unit, layout and index modules. It reproduces the documented `J=16, q=10` arithmetic candidate: 655,350 sets, 112,281,600 slots and final slot index 112,281,599. Boundary/median round-trips cover every class. Distinct class-local/global-set and local/global-slot types prevent accidental interchange; conversions requiring layout remain explicit pure functions. This fixture selects no neural law, final pool geometry, allocation policy or bounds proof for arbitrary caller values.

## Regional storage and pure snapshots

```text
unit Neuron::u32;
array Rates[Neuron]::f32;
regional type Node { value::f32; next::ref<Node>; rates::ref<Rates>; }
symbol @arena;
symbol @rates;
effect @arena = region(4096u64);
effect @rates = alloc @arena Rates(4u64, 0.5f32);
store(@rates, Neuron(1u32), 2.0f32);
function total(a::Rates)::f32 {
  fold (i in Neuron(0u32) .. Neuron(u32(len(a))), sum = 0.0f32) { sum + a[i] }
}
```

Regions use the existing shilos region allocation/release operations with a fixed payload capacity. `alloc @arena` creates an array or regional record; allocations persist until region teardown. Regional records cannot be stack-constructed or copied/moved. Arrays store contiguous scalar elements with a declared index type; related arrays may share that type. Separate source-ID and weight arrays occupy four and one bytes per element respectively. Product fields use parallel arrays, rather than a hidden array-of-records representation.

`ref<T>` is a typed regional reference; `deref` produces a bits value or a noncopying regional view. Record fields use `.field`, arrays use `[index]`, and `len` returns `u64`. `ref_at(array, index)` returns an existing scalar-element reference. Array access requires the declared index identity. Bounds and null dereferences have runtime diagnostics; `deref` checks a regional reference even if its resulting view is unused. `null` is inferred from regional reference fields and conditional joins. This syntax is the bounded executable prototype, not a settled storage ABI or complete pointer language.

Accepted known issue ([H-146](../../../design/DesignAuthorityRegistry.md#h-146)): assigning a record to its first pointer field produces field-relative displacement zero, so the field reads back as null. The current encoding and behavior are retained without a new store diagnostic. Future intrinsic object fields are expected to resolve this limitation; their definition, layout and compatibility rules remain open. Other nonzero-distance cycles are covered by relocation evidence.

Procedures may use `store(array, index, value)`, `store(scalar_ref, value)` and `setfield(record_ref, field, value)`. The `field` selector names a declared field, as `.field` does; it is not evaluated as an expression. Field names such as `effect` or `true` retain their identifier meaning in this position, including inside parentheses. Stored results survive dynamic-scope restoration. `effect` only changes a binding and restores that binding on scope exit. `used_bytes` and `capacity_bytes` inspect a region resource from a procedure. Capacity includes the native region header; allocation requests specify payload bytes.

A function and its nested function calls may read every object reachable through their explicit inputs, including reference fields. They cannot allocate, mutate storage, access effect bindings or call procedures. This enforces the single-threaded read-only snapshot without copying a graph or introducing snapshot state. Argument expressions run in the caller before this interval: a copied bits argument keeps its evaluated value, while a regional view exposes storage after all argument expressions and conversions finish. An existing input reference may be returned to the calling procedure; a subsequent outer function call observes intervening procedure writes. Pointer captures preserve the reference value, not the pointed-to contents. Explicit inputs may come from separate live regions: equal relative offsets do not make references equal, and returned references retain their original owner. This does not permit a stored cross-region field.

The current compiler specializes owning-region information along known call paths. Returning a reference to a newly created, ending region is rejected; returning an allocation in a still-live caller region is allowed, including allocations made inside a bounded fold. A nested region scope may return copied bits while releasing its regional storage. Conditional/fold references retain one known owning region. Persisted cross-region fields remain a separate design topic. An imported source procedure may be used with `behave` under live caller bindings, while `perform` rejects a required capture whose source region has expired.

[region-factory-import.cod](region-factory-import.cod) checks source and receiving-context factories with distinct nominal record types. Its dependency also allocates in the caller’s region during initialization. A selected procedure retains that reference after the dependency scope ends; a later selection reuses the capture even after the caller binds a second region. `perform` observes the original object’s updated contents, while `behave` reads the caller’s current object.

[region-gather.cod](region-gather.cod) combines unit/layout/index modules with regional type, pure kernel and procedure modules. It fills separate source and weight arrays with varying source indices and weights, then performs four A/B pull updates with FP32 snapshots. Every target value and the checksum of each frame match an independent calculation that rounds multiplication, division and addition to FP32. The small trace covers the initial frame and four updates across 15 targets. This is a programming-facility check, not a neuron transition or training rule. [gather_benchmark.py](gather_benchmark.py) scales the same kernel to 4,095 rows, 693,504 incoming slots and eight updates, using uniform weights and checksum-only output; [gather-validation.json](gather-validation.json) records comparison with an independent rounded FP32 reference. On the recorded Linux run, regional occupation is 3,500,416 bytes, of which 3,500,280 are array elements. The linked record contains IR/bitcode/native size and local process time, including initialization and checksum output. These observations establish runnable scale for this arithmetic workload, not SNN throughput.

The optional `gather_benchmark.py --full-pool` case exercises all 112,281,600 slots of the J16,q10 arithmetic layout with uniform input and one gather. [gather-full-pool-validation.json](gather-full-pool-validation.json) records 566,650,928 occupied bytes with measured local process time in the linked record. It assigns one arithmetic row per slot set, rather than selecting an SNN neuron count. All 5-byte slot elements remain separate four-byte and one-byte arrays. [numeric_properties.py](numeric_properties.py) compares 22 FP32 and 19 u64 regional-input results against independent arithmetic; [numeric-validation.json](numeric-validation.json) records its trace and source hashes.

## Module and storage boundary

All dependency files are parsed before any execution is emitted. A syntax or required-binding error rejects the whole entry before any top-level print can run. Procedure registration during scanning does not capture values; capture occurs at its definition event in the resulting execution.

After the complete scan, the first cross-module encounter incorporates that module’s top-level runtime semantics into the resulting program. Later `symbol from`, `using` and `import` encounters reuse the incorporated module and its resources. The source’s scope then restores its effects. Identity-only selection does not retain an incidental runtime value; selected procedures retain captured bits and regional-reference values; a reference does not extend its region lifetime or copy its target. Imported aliases must have reached their `symbol from` or introducing `using symbol` encounter before use, including direct symbol references in procedure definitions. Combined using is one source encounter; matching an existing identity still installs the selected procedure. A body map does not rewrite source initialization or other existing procedures.

Unknown local suppliers/source-body roles and duplicate mapping roles are diagnosed. Splitting aliases of one source identity across multiple receiving identities is outside the prototype; mapping them to one identity is supported. Broader mapping policy remains open.

The subset accepts acyclic module composition including shared and diamond dependencies. Each module is incorporated once, in first-encounter order; its effects are not rerun or recaptured for another caller. A demanded initialization cycle is diagnosed; general cyclic-initialization semantics remain open. Compiler encounters do not execute user code.

Local symbol identity uses the canonical module path and declaration, preserving aliases and separating distinct source files. Versioned package identity and first-class runtime symbol selection are not implemented. Calls are specialized along known paths; recursion, procedure-valued scope results and general runtime higher-order dispatch are unsupported.

The generated program uses fixed thread-local typed binding slots and stack-based saved values. Known procedures are specialized by the compiler. Compiler ASTs and system machinery can use the host heap; generated user data and scalar captures do not. Regional references carry offsets with owning-region information specialized along the known WPO path. Persisted intra-region fields use shilos `regional_ptr`; source-level cross-region fields and persistent closures remain unsupported. The presence of TLS does not establish tested multi-thread or reentrant execution support.

## Evidence

[validation.json](validation.json) records the latest checkpoint’s commands, outputs, exit codes, source/executable hashes and the explicitly dated preceding component-suite evidence. [stress-validation.json](stress-validation.json) records generated dependency/call-graph checks. [regional-validation.json](regional-validation.json) records native layout, complete-image relocation and compilation/process measurements from [regional_runtime_tests.py](regional_runtime_tests.py). The harness verifies LLVM artifact round-tripping and inspects the records/arrays created by LLVM execution through an independent layout view. The fixture writes an FP32-backed UoM through an element reference returned by a pure graph traversal. The harness copies the complete region, releases the original, then checks the stored scalar through an independent float layout, sharing, a cycle and mutation through the relocated references. Measurements are local observations, not portability or full-language acceptance claims.

The suite discovers [test_language.sh](../test_language.sh), which runs [run_tests.py](run_tests.py) against expected results in [cases.json](cases.json). To repeat the component suite with the existing build toolchain:

```sh
COD_TEST_TOOLCHAIN=build tests/cod/run-all.sh
```

For the language group alone, use `COD_TEST_TOOLCHAIN=build tests/cod/test_language.sh`. Optional fixture stems select individual cases; the frontend-only check still runs. Set `COD_LANGUAGE_REPORT` to save a JSON result record. These tests compile and run accepted programs and require rejected programs to fail before any preceding top-level output.

Coverage is judged by the contracts below, not by the number of fixtures or a source-line percentage. Each group needs an accepted execution path and the relevant rejection or corruption check. Overlapping spelling variants are not separate acceptance goals.

| Contract and consequence | Key evidence |
| --- | --- |
| Effects select the right context and restore it; otherwise nested calls silently change caller behavior. | `scopes`, `transitive`, `capture-scope-restore` including chained effect results; unavailable captures/bindings rejected. |
| Module loading is separate from once-only execution; identities survive aliases and remain distinct from values. | `module-diamond`, resource-trigger fixtures, `import-mapping`, `using-symbol-reexport`, identity mismatch despite equal procedure values. |
| Products/UoMs preserve identity, pure methods and deliberate conversion choice. | `products`, `methods`, selected unit bundles, exact/direct/declared routes, ambiguity warning/error, argument evaluated once. |
| Control flow preserves order, short circuiting, iteration scopes and fold types. | `arithmetic`, rejected unscoped logical writes even to an equal scalar/resource, `bounded-iteration`, `iteration-scope` with nested local shadowing and a discarded iteration-local reference, rejected same-scope redeclaration and reserved local names, contextual `in` bindings, product/reference folds and invalid accumulator types. |
| Pure calls read input-reachable data while procedures own mutation and region lifetime. | `products` with regional nested-product storage, `region-records` with UoM element references, `region-capture` across independent owners and later-argument rebinding, module `perform`/`behave`, returned parent references and fold construction, nested-region bits results, rejected writes/escapes/cross-region fields. |
| Regional storage actually relocates without pointer repair. | LLVM artifact/relocation harness: sharing and cycle survive freeing the original; array mutation follows the relocated references. |
| Numeric representation and typed SoA indexing produce the intended arithmetic. | Named overflow/cast/precision boundaries, `region-empty` with alternating boolean elements and a reference write at the maximum `u8` index, small independent numeric sample, `region-gather` and one full-pool measurement. |
| Composition remains usable at the requested depth and dependency shape. | One stress set: deep pure/procedure paths, shared DAG, conversion re-exports and regional snapshot chain. |

The broader [semantic case catalog](../../../applications/steady-snn/research/cod-language-scope-cases.json) includes unimplemented application cases. This corpus does not validate neural equations, learning, general GADT behavior, concurrency, source-level LLVM debugging or cross-platform runtime layouts. Tests are added when a concrete uncovered contract or bug warrants one; repetition alone is not evidence of stronger coverage.

The implementation is in [lang_syntax.hh](../../../cod/lang_syntax.hh), [lang_frontend.hh](../../../cod/lang_frontend.hh), [lang_llvm.hh](../../../cod/lang_llvm.hh) and [main.cc](../../../cod/main.cc). [lang_native_runtime.hh](../../../cod/lang_native_runtime.hh) supplies the fixed system services. Source edits build with exactly `cmake --build ./build`; no toolchain or build-configuration source was changed for this lowering path.
