# CoD booter: a compiler application written in CoD

Roadmap revision: 22 · Recorded: 2026-11-21 · Phase: initial seed delivery through alternating booter and seed development.

## Design authority

- [D-17-EXCEPTION-BINDINGS](../../design/DesignAuthorityRegistry.md#d-17-exception-bindings) and [D-10-CONTEXTUAL-NAMES](../../design/DesignAuthorityRegistry.md#d-10-contextual-names) at roadmap r22 apply the [catch](../../CHANGELOG.md#h-catch-holder-only-20261121) and [lexical name](../../CHANGELOG.md#h-lexical-name-release-20261121) decisions to both bootstrap frontends. P-650–P-653 connect readable, checkable source to one identifiable release obligation and ordinary lookup. Check positive and negative grammar boundaries. Runtime delivery remains scoped per operation.

- **Initial seed delivery.** [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap), r16, covers [the seed scope and acceptance](#initial-seed-delivery). [H-BOOTER-DRIVEN-SEED-20261119](../../CHANGELOG.md#h-booter-driven-seed-20261119) confirms the booter-driven delivery priority and source-readability requirement. [H-BOOTER-NATIVE-MINIMUM-20261119](../../CHANGELOG.md#h-booter-native-minimum-20261119) confirms the shared C interface, basic C-native region support and minimal LLVM bindings as required for delivery. P-642 connects C-80 to charter §§4.3/6.3/8.1/12 through explicit native lifetimes, correct execution, readable CoD source and end-to-end evidence. P-643 records governing contracts. The exact provider ABI and operations beyond the booter's needs retain their own review boundaries.

- **Checked DBMR program revisions.** [D-20](../../design/DesignAuthorityRegistry.md#d-20) records this separate research checkpoint. The [CoD source listing](../../CHANGELOG.md#k-warehouse-cod-source-20261119) supplies the research workload. [Authority record](../../CHANGELOG.md#k-typed-dbmr-program-overview-20261119). The checkpoint applies the charter’s source and edit loop and native execution goals (§§6.3/7/8/10/12) through C-01/C-02/C-05/C-06 and the existing C-80/P-314–P-318 plan. Check graph/source correspondence, safe publication, equivalent execution observations and distinct performance measures. Graph schemas, editing APIs, specialization policy and implementation qualification remain open.

- **Diagnostics and booter exception handling.** [H-BOOTER-EXCEPTIONS-DIAGNOSTICS-20261120](../../CHANGELOG.md#h-booter-exceptions-diagnostics-20261120) confirms the bootstrap exception-handling responsibility, diagnostic scope and repository placement at r17. [D-20-DIAGNOSTICS](../../design/DesignAuthorityRegistry.md#d-20-diagnostics) and [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories) apply P-586/P-627/P-628 and the [stage-policy baseline](../../CHANGELOG.md#h-compiler-diagnostics-20261119). Under charter sections 6.3, 8.1 and 12, check observable exception behavior in the booter and assign analysis for detailed explanations to the production compiler.

- **Generated C providers.** [D-20-C-PROVIDERS](../../design/DesignAuthorityRegistry.md#d-20-c-providers) and [H-BOOTER-GENERATED-C-20261120](../../CHANGELOG.md#h-booter-generated-c-20261120) select checked CoD declarations as the source of the initial provider contract. [D-20-C-HANDLES](../../design/DesignAuthorityRegistry.md#d-20-c-handles) and [H-BOOTER-C-HANDLES-20261120](../../CHANGELOG.md#h-booter-c-handles-20261120) select synchronous here calls, lexical native owners and bits-only records for this workflow. P-642/P-646/P-647 connect the first native run to reviewable CoD, actual owner checks and exact cleanup under charter §§4.3, 6.3, 8.1 and 12. [The first-host declarations](docs/LLVMInterop.md#declarations-and-the-first-host-call) specify the two directions of `proc "C"` and require complete native rollback before a failed acquisition raises its declared exception. [R20 decision record](../../CHANGELOG.md#h-booter-c-declarations-20261120).

- **Next-stage roadmap.** [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories) records the [toolchain roles](../../CHANGELOG.md#h-toolchain-repositories-20261118). At r12 the [repository split decision](../../CHANGELOG.md#h-lang-repository-20261119) assigns the shared language specification to `cod-lang` and all five repositories to the same GitHub organization. Both compiler implementations refer language decisions to that specification. C-80/P-317 supplies the planning rationale and P-586 records dependencies. At r8, [D-20-NATIVE-C](../../design/DesignAuthorityRegistry.md#d-20-native-c) records the [human decision](../../CHANGELOG.md#h-native-c-interface-20261118) to implement new CoD native components in C and expose a C interface. C-80 and charter §§4.3/6.3 connect seed and self-hosted clients to the same runtime semantics. Assess each operation against its CoD lifetime, visibility and failure guarantees. The detailed ABI and migration plan remain open. Revision 10 applies [H-DBMR-IMMUTABLE-20261119](../../CHANGELOG.md#h-dbmr-immutable-20261119) to the C provider. Verify private construction, readonly opens and complete publication from RAM or file-backed storage.

- **Output initialization.** [D-22](../../design/DesignAuthorityRegistry.md#d-22-init-places) governs calls that initialize an ordinary caller place under C-81/C-65 and charter §§4.3, 6.3, 8 and 12. The first native workflow uses the scoped handle contract for resource acquisition and disposal.

- **Registry and scope.** [D-20](../../design/DesignAuthorityRegistry.md#d-20), roadmap revision 22. The pipeline and interop documents retain their scoped qualification. [H-353](../../CHANGELOG.md#h-353) supplies the compiler application baseline. [H-356](../../CHANGELOG.md#h-356) supplies the selected defer call/block timing. Detailed application contracts beyond the confirmed scopes remain `pending-human`.
- **Value:** C-80/P-314–P-318 connects inspectable compiler semantics to C-01, reusable structured artifacts to C-05, and native integration to C-02/C-11/C-79. The charter's §§2, 5.3–5.4, 6–7 and 12 provide the context; detailed derivation and trade-offs remain pending.
- **Boundaries / acceptance:** preserve regional relocation, owner/access checks, independent module worlds and explicit TLS effects where used. Keep confirmed direction, proposals and missing evidence distinguishable. Native ABI, toolchain operations and performance claims require their own authorization or evidence.

## Purpose

Use the CoD-written booter as the seed's first implementation workload. Progressively write CoD's tokenizer, parser, analyzer and compilation pipeline in CoD, selecting and binding LLVM APIs as real compiler operations require them. The production compiler planned for `e-wrks/cod` will compile its own CoD sources and provide full diagnostics.

The unit of design is the compiler operation and its data, not a C++ class translated into CoD. Existing C++ code can supply the seed compiler, implementation evidence and a behavioral comparison. Its container choices, frontend/backend coupling and internal helper structure do not prescribe the new application's architecture. The language contracts remain authoritative when an existing implementation disagrees.

Compiler operations give concrete uses for typed source positions and indices, ADT-based syntax and diagnostics, identity-based module information, regional intermediate artifacts, owner-checked operations and scoped native resources. Evaluate each capability through the compiler operation it expresses.

## Initial seed delivery

Deliver a usable seed that supports the basic development workflow for the first CoD-written booter. Its minimum implementation scope comes from that booter's real source and end-to-end build and execution. Business application prototypes and research paper prototypes belong to separate work. Their semantic requirements, implementation and acceptance are outside the current seed goal.

Alternate small booter and seed increments. Start with the CoD lexer and parser on representative source. Write the next booter operation in clear CoD, build and run it with the seed, then add the smallest coherent seed capabilities that operation needs. Repeat through checking and native output as the initial booter takes shape. Reuse existing seed capabilities and validate accepted programs against the applicable language contracts.

Conveniences such as inference or shorthand can wait when explicit source is clear. Readability is a delivery requirement. The booter should express its compiler operations using CoD's types, ownership, scoped resources and explicit effects where those features make the program clearer or enforce a relevant invariant. A workaround that obscures the algorithm or discards those benefits is grounds to extend the required seed subset. Review real booter source alongside the native result.

The booter fills gaps in the C++ seed’s exception handling. Accepted programs must propagate exceptions, enter matching handlers and perform the cleanup their CoD contracts require. For example, if a callee throws and its deferred release succeeds, the caller’s matching handler must see the resource already released. Check that the handler runs and the resource is released exactly once. The seed may reject an exception feature it has yet to implement. Its minimum support is determined by the booter source it must build. Invalid compiler input can use a diagnostic result from the failing check. An unrecoverable fault in the compiler itself may report its location and terminate, with a core dump where enabled.

Initial delivery requires the seed to reproducibly build and run the first CoD-written booter against CoD's shared C interface. The booter must demonstrate all of the following in its workload.

- Call the C-native components it uses through that interface, with the applicable owner, access and resource lifetime checks.
- Create an in-memory region, allocate compiler data in it and release it through the C region service. The region service supplies the basic operations the booter uses under CoD's ownership and relocation rules.
- Use the minimal LLVM bindings exposed through C to verify and emit runnable native output. Run a representative program and check its behavior and the release of native resources on supported completion paths.

Representative source and invalid-input cases must behave as specified, and the booter source must pass the readability review. The booter's first self-build is a downstream milestone. DBMR persistence and broader LLVM binding coverage are separate from initial delivery.

Build the initial booter with statically linked C providers generated from checked CoD declarations. The seed and booter derive equivalent C contracts, including native handle constructor and destructor stubs. Separate handwritten C supplies the operations and their resource protocols. Calls in both directions use synchronous here semantics. CoD acquires the source, scratch backing and LLVM resources under lexical ownership with scope-exit destruction. See the [native handle and C record rules](docs/LLVMInterop.md#native-handles-and-c-records) for borrowed values and by-value records.

### Exception syntax in both frontends

The C++ seed and CoD booter must parse the [owning catch header and ordinary ref declarations](../../design/ExceptionControl.md#exception-declarations-and-catch-bindings) and the [normal construction forms](../../design/ExceptionControl.md#make-exception-expressions). Both recognize `catch (e)`, `catch (e:E)` and `catch (e:const E)` in try and defer. A ref or holder declaration prefix in a catch, or an owner constraint in its header, is a syntax error. Locals, parameters and results distinguish owning exception declarations from ordinary ref borrows. Both parsers preserve explicit target constness and construction placement.

Acceptance checks each frontend at its own parse boundary. A valid form carries its binding mode, owner name when present, and construction operands into checking. Malformed syntax fails at parsing. When an execution path is unsupported, the compiler can reject the parsed form during checking or lowering. Supported paths must execute their required propagation, handling and cleanup. For example, parsing `catch (e:const E)` as a borrowed local loses the handler’s release obligation even when the handler only reads e.

### Contextual names in both frontends

Apply the [name rules](../../design/SymbolicDynamicScope.md#clause-words-and-lexical-variable-names) during dependency scanning, parsing and lookup. Cover lexical binders and their uses, `from` and `throws` in clauses, module declarations and ordinary user calls with names such as `fold` and `store`. A lexer accepting the spelling is insufficient if a later phase still diverts it to an intrinsic or rejects the binding. References to unsupported features retain their stage-specific diagnostics.

## Build and run on Linux

The standalone build uses GNU Make 4.3 or newer and an existing CoD seed and compatible LLVM SDK with shared libraries. From the repository root, build the booter and run the deferred-release workload.

```sh
make -C applications/cod-booter
./build-cod-booter/cod-booter applications/cod-booter/workloads/deferred-release.cod \
    build-cod-booter/deferred-release.o
./build/bin/clang build-cod-booter/deferred-release.o -o build-cod-booter/deferred-release
./build-cod-booter/deferred-release
```

The program prints `1` on three lines. Use `TOOLCHAIN` for a different ready toolchain prefix, or set `COD_SEED`, `LLVM_CONFIG`, `CLANG` and `CLANGXX` individually. Use a separate `OUT` for each build you run concurrently or want to retain, such as `OUT=../../build-cod-booter-shared COD_SEED_FLAGS=--no-specialize`. Make interprets relative paths from the booter directory. Use paths without whitespace.

The seed checks the booter sources and emits the provider header and stubs, LLVM IR and C++ runtime into `OUT`. Clang compiles those outputs and the handwritten C providers. The link uses the shared libraries reported by `llvm-config`, with the SDK library directory recorded for the loader. The seed and LLVM build tree is a read-only input to Make.

## Next-stage repositories and native runtime

All five planned repositories will live in the [e-wrks GitHub organization](https://github.com/e-wrks).

| Planned repository | Responsibility |
| --- | --- |
| `cod-lang` | CoD language design and the authoritative language specification shared by both compilers. |
| `cod-seed` | The C++ seed compiler, the CoD-written booter and the C-native components they use, including the region core and LLVM bindings. The booter stays in this repository after the split. |
| `cod` | The future production CoD compiler written in CoD, including full diagnostics and the additional analysis they require. |
| `codup` | Toolchain version management in the role of rustup. CoD package dependency resolution and lockfiles have their own [codp design](../../design/CodProjectStructureAndPkgMgmt.md). |
| `codmate` | The Language Server Protocol server and IDE plugins. |

After the split, a change to what a CoD program means belongs in `cod-lang`. Each compiler repository documents how it implements the rule, its current coverage and any known gaps.

New CoD native components are written in C. The CoD native interface is C-based. The C++ seed, the booter, the future production compiler and foreign adapters consume the same interface. Specify each operation from the runtime behavior its callers need. C++ classes, templates and exception propagation are internal to C++ clients or dependencies. This choice covers CoD’s LLVM bindings as well as region support.

At the separate DBMR checkpoint, the region service must [publish a newly created disk-backed region](../../design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr) only after initialization and preserve an existing destination in a race. Create private staging on the target filesystem, reject mismatched destinations, and verify the destination at the instant another process can open it. A killed builder and two publishers racing for the same name must preserve every previously published image. The C++ region and DBMR code is historical reference. The new components follow the CoD operation contracts.

For exception regions, distinguish fixed logical capacity, address-space reservation, backing enabled on demand and resident pages. Choose target VM operations behind the interface. Preserve owner identity and locators while enabling pages, and release acquired resources exactly once on partial failure. The C runtime must release owned exception-array members as well as the aggregate region. The native provider should support the [exception capacity rules](../../design/ExceptionControl.md#image-setting-for-exception-region-capacity), including site budgets if their syntax is adopted. Measure virtual address space and resident use separately on target systems.

Specify and check the initial region and LLVM operations alongside the booter increments that use them. Broader interfaces, toolchain distribution and editor integration have separate planning and acceptance. Repository creation, source migration and toolchain operations retain their own authorization.

### Diagnostics by compiler stage

The C++ seed and the CoD-written booter in `e-wrks/cod-seed` share one diagnostic policy across parsing, name resolution, type checking, ownership, control flow and code generation. Both enforce the checks needed for correct compilation. Report failed checks directly, using the reason and source location when available. Unsupported programs receive a direct diagnostic. Accepted programs retain required runtime checks. The booter’s added responsibility here is exception handling. Its delivery requires no extra analysis or diagnostic output beyond the seed policy.

The future compiler in `e-wrks/cod` owns full diagnostics, including extra passes, cross-path reasoning, counterexamples, explanatory notes and suggested fixes. For a lifetime error, the seed and booter can report the failed check. Reconstructing other paths to explain the error belongs in `e-wrks/cod`. The same boundary applies to optional coverage warnings, overload explanations, receiving-overlap warnings and implicit-import suggestions.

## Document map

| Document | Responsibility |
| --- | --- |
| [Compiler artifacts and pipeline](docs/CompilerPipeline.md) | Stage boundaries, regional representations, DBMR reuse, module worlds and compilation scale. |
| [LLVM bindings and native resources](docs/LLVMInterop.md) | Generated C providers, synchronous here calls, scoped handles and the first native workload. |

Reusable language decisions belong in the relevant `design/` contracts after discussion. These application documents describe requirements and candidate uses without silently extending those contracts. The application name does not refer to the repository's existing native toolchain bootstrap scripts.

## Proposed progression

Use the initial seed delivery scope to select work at each checkpoint. The feature and runtime contracts needed by each implementation retain their own qualification. For example, when a booter operation needs [D-17 scoped cleanup](../../design/ExceptionControl.md#immediate-implementation-and-acceptance), `defer init(...)()` evaluates acquisition and saves its returned callable now, then invokes it at scope exit. `defer { ... }` delays the whole block.

| Checkpoint | Useful result | Evidence to seek |
| --- | --- | --- |
| Basic frontend | CoD tokenization and parsing produce inspectable source-linked artifacts. | Correct tokens and syntax on representative CoD modules, direct reporting of invalid input, and readable, typed booter source. |
| Basic C-native region support | The booter builds temporary compiler data in an in-memory region through the shared C interface. | Observe allocation, owner and lifetime checks, and region release. |
| Semantic pipeline | CoD builds module worlds and analyzes demanded calls with explicit identities and context. | Module identity, selected methods, owners and effects agree with accepted language contracts. |
| Native execution | The booter uses a small C-facing LLVM binding surface to verify and emit executable output. | Observe generated program behavior and native resource release on the supported completion paths. Measure frontend and LLVM time and memory separately. |
| Self-hosting | The CoD compiler builds its CoD implementation through the selected native boundary. | Compare seed-built and self-built compilers on compilation outcomes, repeatability and large-program costs. Evaluate diagnostics against each compiler’s stage policy. |
| Persistent reuse, when justified by the compiler workload | Selected stage artifacts save as DBMR and load in a fresh process. | Compare observations after relocation. Reject stale or incompatible artifacts. Measure loading, validation and recomputation costs separately. |

Full LLVM coverage, byte-identical compiler binaries, independent module compilation, distributed builds and an immediate replacement of the seed compiler are not checkpoint requirements. Exact acceptance corpora and supported language scope remain to be selected. Large multi-module programs and deep dependency graphs matter alongside small examples; a demonstration alone is not self-hosting or performance acceptance.

### Checked DBMR program revisions

Use the [warehouse policy](../../papers/AlgebraicEffectsIsAllYouNeed.md#7-what-we-can-show-and-what-to-measure) to exercise the semantic pipeline, persistence and native execution together. Check ordinary CoD source into a typed program graph, publish it as a DBMR and inspect it from CoD. Make the same policy change independently in source and through a typed graph edit. Compare reachable operations, decisions and ordered effect traces to verify that both edits express the same checked program behavior.

Validate and publish the edited application DBMR, then select it and rerun it in the same live session. A stale-base edit or invalid program root must leave the previous publication executable. Check that the program can be reopened at a different mapping address and that changing only the effect handler preserves program identity. Exercise the native execution path against reference results and traces, including both reservation outcomes and a handler failure.

Measure **steady-state throughput** on warmed, equivalent workloads and report artifact I/O separately. Measure **edit-to-first-correct-result latency** in a running session from edit submission through validation, publication, any invalidation and compilation, and execution of the first result that reflects the change. The [warehouse example in CoD](../../papers/AlgebraicEffectsIsAllYouNeed.md#appendix-the-warehouse-revision-in-cod) supplies the input, expected decisions and ordered reservation trace. Native throughput and interactive latency remain to be measured in CoD.

## Decisions to revisit before implementation

| Topic | Planning position | Still needed |
| --- | --- | --- |
| Stage caches | Preserve suitable regional results directly as DBMR. | Stage-specific dependency/compatibility rules, publication behavior and measured payoff. |
| LLVM surface | Bind APIs as compiler operations need them. | Checked declarations and generated provider evidence from the first workload. |
| Native handles | Use [lexical owners and explicit borrowed handles](docs/LLVMInterop.md#native-handles-and-c-records). | Generated evidence for actual owner binding and bounded views, target layout, general fallible disposal and the build graph. |

Current implementation evidence belongs with the booter workloads and the seed capabilities they exercise.

### Guarded scalar results

The native module compiler can retain a compatible non-null scalar target from a `try` used as a conditional arm. A value read within the guard prepares its bits before `finally`. When the enclosing conditional joins the guarded target with independent bits, it reads the selected target after the guard completes. The current subset requires one result category across the completing branches of that guard. It admits a nullable target from a guarded arm under explicit discard.
