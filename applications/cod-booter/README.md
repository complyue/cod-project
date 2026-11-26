# CoD booter: a compiler application written in CoD

Roadmap revision: 48 · Recorded: 2026-11-26 · Phase: initial seed delivery through alternating booter and seed development.

**Compiler design target.** On the command line, `cod` (Compile on Demand) compiles the entry for this run and executes it immediately, leaving no user-facing executable. The planned production command `cfe` (Compile for Ever) spends more time optimizing the program and saves the executable for later runs. Seed and booter development prioritize Go-like compile speed with moderate runtime performance. **The seed’s default bootstrap build produces and keeps the `cod-booter` binary.** [Compiler modes and release milestones](docs/CompilerPipeline.md#default-execution-and-optimized-builds)

## Design authority

At r48 the [production parser bundle](../../design/DesignAuthorityRegistry.md#d-20-parser-bundle) identifies `pcl.cod` as the HLVM and the production CoD parser as an application of it, following the [user’s clarification](../../CHANGELOG.md#h-pcl-hlvm-parser-application-20261126).

At r47 the [production CLI decision](../../CHANGELOG.md#h-cfe-output-directory-20261126) names the optimizing entry point and specifies its executable output directory under [D-20-COMPILER-MODES](../../design/DesignAuthorityRegistry.md#d-20-compiler-modes). The [compiler modes contract](docs/CompilerPipeline.md#default-execution-and-optimized-builds) gives both output-location checks.

At r46 the [production parser bundle](../../design/DesignAuthorityRegistry.md#d-20-parser-bundle) adds an installed-toolchain acceptance case to the production roadmap. The [user’s delivery decision](../../CHANGELOG.md#h-production-parser-bundle-20261126) continues the existing division between the production parser and the bootstrap frontends.

At r45 [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues r44’s trusted translation boundary and identifies the persistent booter executable as the seed’s default bootstrap output. The [compiler modes decision](../../CHANGELOG.md#h-default-run-and-optimized-build-20261125) sets the fast compilation target for both bootstrap compilers and the production roadmap under [D-20-COMPILER-MODES](../../design/DesignAuthorityRegistry.md#d-20-compiler-modes).

At r44 [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues r43's trusted input, runtime and self-check boundaries. The [replacement seed decision](../../CHANGELOG.md#h-compiler-pipeline-replacement-20261125) assigns the new translator to mainline. [D-20-STAGE-CONTROLS](../../design/DesignAuthorityRegistry.md#d-20-stage-controls) sets the bootstrap goals and production requirement, with separate acceptance for each stage.

At r43 [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues r42’s seed/checker boundary for the [family capture decision](../../CHANGELOG.md#h-perform-family-capture-20261125). The seed must emit scoped replay for valid source.

At r42 [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues the r41 local-supply rule. The [project user’s clarification](../../CHANGELOG.md#h-effect-checking-cost-and-compiler-stages-20261125) identifies selected-path binding verification for removal from the seed and assigns the [family check](../../design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) to both pre-2.0 semantic checkers.

At r39 the [project user’s naming decision](../../CHANGELOG.md#h-bootstrap-tool-and-entry-names-20261125) names the seed executable and applies the reserved entry name to this application. [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues the r38 stage responsibilities. [D-15-ENTRY](../../design/DesignAuthorityRegistry.md#d-15-entry) governs the entry identity.

At r38 the [project user’s seed decision](../../CHANGELOG.md#h-trusted-seed-translation-20261125) assigns [translation of trusted booter source](#trusted-source-and-the-seed) to the C++ seed and static soundness checking to the CoD booter. [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) and [D-20-DIAGNOSTICS](../../design/DesignAuthorityRegistry.md#d-20-diagnostics) revise P-642/P-643 and P-627/P-628 for this division of work. The booter’s readable source, its shared C interface and the observable behavior of valid programs carry forward. The stage policy below defines which checks apply throughout the entry’s fused world.

At r37 the [repository-name correction](../../CHANGELOG.md#h-package-repository-name-20261125) continues the r36 roadmap. The [manual package input decision](../../CHANGELOG.md#h-manual-bootstrap-packages-20261125) replaces the bootstrap package manager with the [project-local layout](../../design/CodProjectStructureAndPkgMgmt.md#directory-layout). The seed and booter apply the same module lookup. Package management is future work in `e-wrks/cod`.

At r35 the [shilos retirement](../../CHANGELOG.md#h-retire-shilos-20261125) confirms the [native component boundary](#next-stage-repositories-and-native-runtime) under [D-20-NATIVE-C](../../design/DesignAuthorityRegistry.md#d-20-native-c). Shared support uses C. High-level features belong preferentially in CoD and the seed’s internal compiler logic uses C++.

- Roadmap r31 adds [compiler memory acceptance](#compiler-memory-acceptance) to [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) under P-642/P-643. The [project user’s decision](../../CHANGELOG.md#h-bootstrap-compiler-memory-20261124) permits a higher C++ seed footprint when reducing it would materially harm maintainability. Reasonable memory use by the linked CoD booter is a delivery requirement. The booter’s existing correctness and resource-lifetime requirements continue. Roadmap r32 records the [measured release budgets](../../CHANGELOG.md#k-booter-memory-baseline-20261124). The [r33 release observations](../../CHANGELOG.md#k-booter-entry-world-release-20261124) continue that scope and the r30 entry-world diagnostic contract. Roadmap r34 clarifies which builds share the heavy slot under the [workload-based concurrency decision](../../CHANGELOG.md#h-workload-based-build-concurrency-20261124).

- Roadmap r30 covers [entry-world diagnostic coverage](#diagnostic-coverage-of-the-entry-modules-fused-world) in the seed, booter and production compiler. [D-20-WORLD-DIAGNOSTICS](../../design/DesignAuthorityRegistry.md#d-20-world-diagnostics), P-713/P-714, records the [project user’s decision](../../CHANGELOG.md#h-entry-world-diagnostics-20261124). The entry determines the work and the invocation owns the optional diagnostic limit. Acceptance covers independent syntax and semantic errors across dependencies and safe publication on completion. The [r30 status review](../../CHANGELOG.md#k-seed-world-product-status-20261124) covers seed dependency traversal and independent direct product declarations.

- At r27 [D-20-PARSE-RECOVERY](../../design/DesignAuthorityRegistry.md#d-20-parse-recovery), P-688/P-689, covers [whole-file recovery](#whole-file-parser-recovery). The project user [requires all parsers to continue and allows role-bearing error nodes](../../CHANGELOG.md#h-whole-file-parser-recovery-20261123). Acceptance covers independent faults in one file with useful syntax after them, all reports at the user-facing boundary and preservation of prior artifacts.

- [D-20-DIAGNOSTIC-NAMES](../../design/DesignAuthorityRegistry.md#d-20-diagnostic-names), P-684/P-685, at r26 records the [two-name diagnostic principle](../../CHANGELOG.md#h-diagnostic-names-20261122). A reader can connect the spelling at a use site to its defining identity under charter §§8–9. Apply it at all stages within P-627/P-628. Acceptance covers renamed use sites, defining-module qualification and runtime construction provenance.

- [D-20-MODULE-GRAPH](../../design/DesignAuthorityRegistry.md#d-20-module-graph), roadmap r25, covers [module traversal, frames and scoped reentry](docs/ModuleGraph.md) under the [user-directed layer review](../../CHANGELOG.md#h-compiler-layers-20261122). P-680/P-681 connect compiler-language ownership to inspectable end-to-end behavior. Verify the actual graph, nested restoration, typed AST, provider files and object workloads. The separate pipeline/cache proposal retains its own scope.

- [D-20-HANDLE-EFFECT](../../design/DesignAuthorityRegistry.md#d-20-handle-effect) at r24 covers the first object writer’s [scoped LLVM session](docs/LLVMInterop.md#owning-native-handle-effects) under [H-NATIVE-HANDLE-EFFECT-20261121](../../CHANGELOG.md#h-native-handle-effect-20261121), P-672/P-673. Check the same actual session across CoD helper calls in both frontends, typed construction failure and exact scope cleanup. C operations still take checked borrows.

- [D-20-SHARED-RUNTIME](../../design/DesignAuthorityRegistry.md#d-20-shared-runtime) and [D-20-NATIVE-BUILD](../../design/DesignAuthorityRegistry.md#d-20-native-build), roadmap r23, apply the [runtime](../../CHANGELOG.md#h-shared-c-runtime-20261121) and [native build](../../CHANGELOG.md#h-native-build-boundary-20261121) decisions. P-654–P-657 connect reusable native implementations to consistent execution and verifiable compiler boundaries under charter §§4.3, 6.3 and 12. Check the same archive through seed execution and the seed-built booter, link emitted code without LLVM, and build reusable components without building either compiler. Existing language contracts and application adapter ownership continue. Permanent ABI and distribution retain their own scope.

- [D-17-EXCEPTION-BINDINGS](../../design/DesignAuthorityRegistry.md#d-17-exception-bindings) and [D-10-CONTEXTUAL-NAMES](../../design/DesignAuthorityRegistry.md#d-10-contextual-names) at roadmap r22 apply the [catch](../../CHANGELOG.md#h-catch-holder-only-20261121) and [lexical name](../../CHANGELOG.md#h-lexical-name-release-20261121) decisions to both bootstrap frontends. P-650–P-653 connect readable, checkable source to one identifiable release obligation and ordinary lookup. Check positive and negative grammar boundaries. Runtime delivery remains scoped per operation.

- **Initial seed delivery.** [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap), r16, covers [the seed scope and acceptance](#initial-seed-delivery). [H-BOOTER-DRIVEN-SEED-20261119](../../CHANGELOG.md#h-booter-driven-seed-20261119) confirms the booter-driven delivery priority and source-readability requirement. [H-BOOTER-NATIVE-MINIMUM-20261119](../../CHANGELOG.md#h-booter-native-minimum-20261119) confirms the shared C interface, basic C-native region support and minimal LLVM bindings as required for delivery. P-642 connects C-80 to charter §§4.3/6.3/8.1/12 through explicit native lifetimes, correct execution, readable CoD source and end-to-end evidence. P-643 records governing contracts. The exact provider ABI and operations beyond the booter's needs retain their own review boundaries.

- **Checked DBMR program revisions.** [D-20](../../design/DesignAuthorityRegistry.md#d-20) records this separate research checkpoint. The [CoD source listing](../../CHANGELOG.md#k-warehouse-cod-source-20261119) supplies the research workload. [Authority record](../../CHANGELOG.md#k-typed-dbmr-program-overview-20261119). The checkpoint applies the charter’s source and edit loop and native execution goals (§§6.3/7/8/10/12) through C-01/C-02/C-05/C-06 and the existing C-80/P-314–P-318 plan. Check graph/source correspondence, safe publication, equivalent execution observations and distinct performance measures. Graph schemas, editing APIs, specialization policy and implementation qualification remain open.

- **Diagnostics and booter exception handling.** [H-BOOTER-EXCEPTIONS-DIAGNOSTICS-20261120](../../CHANGELOG.md#h-booter-exceptions-diagnostics-20261120) confirms the bootstrap exception-handling responsibility, diagnostic scope and repository placement at r17. [D-20-DIAGNOSTICS](../../design/DesignAuthorityRegistry.md#d-20-diagnostics) and [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories) apply P-586/P-627/P-628 and the [stage-policy baseline](../../CHANGELOG.md#h-compiler-diagnostics-20261119). Under charter sections 6.3, 8.1 and 12, check observable exception behavior in the booter and assign analysis for detailed explanations to the production compiler.

- **Generated C providers.** [D-20-C-PROVIDERS](../../design/DesignAuthorityRegistry.md#d-20-c-providers) and [H-BOOTER-GENERATED-C-20261120](../../CHANGELOG.md#h-booter-generated-c-20261120) select checked CoD declarations as the source of the initial provider contract. [D-20-C-HANDLES](../../design/DesignAuthorityRegistry.md#d-20-c-handles) and [H-BOOTER-C-HANDLES-20261120](../../CHANGELOG.md#h-booter-c-handles-20261120) select synchronous here calls, lexical native owners and bits-only records for this workflow. P-642/P-646/P-647 connect the first native run to reviewable CoD, actual owner checks and exact cleanup under charter §§4.3, 6.3, 8.1 and 12. [The first-host declarations](docs/LLVMInterop.md#declarations-and-the-first-host-call) specify the two directions of `proc "C"` and require complete native rollback before a failed acquisition raises its declared exception. [R20 decision record](../../CHANGELOG.md#h-booter-c-declarations-20261120).

- **Next-stage roadmap.** [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories) records the [toolchain roles](../../CHANGELOG.md#h-toolchain-repositories-20261118). At r12 the [repository split decision](../../CHANGELOG.md#h-lang-repository-20261119) assigns the shared language specification to `cod-lang` and all five repositories to the same GitHub organization. Both compiler implementations refer language decisions to that specification. C-80/P-317 supplies the planning rationale and P-586 records dependencies. At r8, [D-20-NATIVE-C](../../design/DesignAuthorityRegistry.md#d-20-native-c) records the [human decision](../../CHANGELOG.md#h-native-c-interface-20261118) to implement new CoD native components in C and expose a C interface. C-80 and charter §§4.3/6.3 connect seed and self-hosted clients to the same runtime semantics. Assess each operation against its CoD lifetime, visibility and failure guarantees. The detailed ABI and migration plan remain open. Revision 10 applies [H-DBMR-IMMUTABLE-20261119](../../CHANGELOG.md#h-dbmr-immutable-20261119) to the C provider. Verify private construction, readonly opens and complete publication from RAM or file-backed storage.

- **Output initialization.** [D-22](../../design/DesignAuthorityRegistry.md#d-22-init-places) governs calls that initialize an ordinary caller place under C-81/C-65 and charter §§4.3, 6.3, 8 and 12. The first native workflow uses the scoped handle contract for resource acquisition and disposal.

- **Registry and scope.** [D-20](../../design/DesignAuthorityRegistry.md#d-20), roadmap revision 23. The pipeline and interop documents retain their scoped qualification. [H-353](../../CHANGELOG.md#h-353) supplies the compiler application baseline. [H-356](../../CHANGELOG.md#h-356) supplies the selected defer call/block timing. Detailed application contracts beyond the confirmed scopes remain `pending-human`.
- **Value:** C-80/P-314–P-318 connects inspectable compiler semantics to C-01, reusable structured artifacts to C-05, and native integration to C-02/C-11/C-79. The charter's §§2, 5.3–5.4, 6–7 and 12 provide the context; detailed derivation and trade-offs remain pending.
- **Boundaries / acceptance:** preserve regional relocation, owner/access checks, independent module worlds and explicit TLS effects where used. Keep confirmed direction, proposals and missing evidence distinguishable. Native ABI, toolchain operations and performance claims require their own authorization or evidence.

## Purpose

Use the CoD-written booter as the seed's first implementation workload. Progressively write CoD's tokenizer, parser, analyzer and compilation pipeline in CoD, selecting and binding LLVM APIs as real compiler operations require them. The production compiler planned for `e-wrks/cod` will compile its own CoD sources and provide full diagnostics.

The unit of design is the compiler operation and its data, not a C++ class translated into CoD. Existing C++ code can supply the seed compiler, implementation evidence and a behavioral comparison. Its container choices, frontend/backend coupling and internal helper structure do not prescribe the new application's architecture. The language contracts remain authoritative when an existing implementation disagrees.

Compiler operations give concrete uses for typed source positions and indices, ADT-based syntax and diagnostics, identity-based module information, regional intermediate artifacts, owner-checked operations and scoped native resources. Evaluate each capability through the compiler operation it expresses.

## Initial seed delivery

Deliver a usable seed that supports the basic development workflow for the first CoD-written booter. Its minimum implementation scope comes from that booter's real source and end-to-end build and execution. Business application prototypes and research paper prototypes belong to separate work. Their semantic requirements, implementation and acceptance are outside the current seed goal.

Alternate small booter and seed increments. Start with the CoD lexer and parser on representative source. Write the next booter operation in clear CoD, build and run it with the seed, then add the smallest coherent seed capabilities that operation needs. Repeat through checking and native output as the initial booter takes shape. Reuse existing seed capabilities where they help emit the code that the booter needs.

Conveniences such as inference or shorthand can wait when explicit source is clear. Readability is a delivery requirement. The booter should express its compiler operations using CoD's types, ownership, scoped resources and explicit effects where those features make the program clearer or enforce a relevant invariant. A workaround that obscures the algorithm or discards those benefits is grounds to extend the required seed subset. Review real booter source alongside the native result.

Both stages preserve exception behavior in the code they generate. For example, if a callee throws and its deferred release succeeds, the caller’s matching handler must see the resource already released. Check that the handler runs and the resource is released exactly once. An unsupported operation that the seed cannot translate receives a direct failure. The booter owns validation of exception propagation and handling in source programs.

Initial delivery requires the seed to reproducibly build and run the first CoD-written booter against CoD's shared C interface. The booter must demonstrate all of the following in its workload.

- Call the C-native components it uses through that interface, with the applicable owner, access and resource lifetime checks.
- Create an in-memory region, allocate compiler data in it and release it through the C region service. The region service supplies the basic operations the booter uses under CoD's ownership and relocation rules.
- Use the minimal LLVM bindings exposed through C to verify and emit runnable native output. Run a representative program and check its behavior and the release of native resources on supported completion paths.

Representative source and invalid-input cases must behave as specified, and the booter source must pass the readability review. The booter's first self-build is a downstream milestone. DBMR persistence and broader LLVM binding coverage are separate from initial delivery.

Build the initial booter with statically linked C providers generated from resolved CoD interface declarations. The seed and booter derive equivalent C contracts, including native handle constructor and destructor stubs. Separate handwritten C supplies the operations and their resource protocols. Calls in both directions use synchronous here semantics. CoD scopes the ownership of source, scratch and LLVM resources. The object writer can install the LLVM session as an owning effect for procedures that need it. See the [native handle and C record rules](docs/LLVMInterop.md#native-handles-and-c-records) for borrowed values and by-value records.

### Bootstrap executable and entry names

The selected name for the C++ build output is `bin/cod-seed` (`bin/cod-seed.exe` on Windows). That compiler translates `applications/cod-booter/src/__main__.cod` to build the single `cod-booter` executable. Its launcher dispatches compilation, declaration checking and provider generation to three procedures in that entry module. The bootstrap commands continue to accept explicit source paths. [Implementation and cutover](../../design/ImplementationArchitecture.md#naming-the-bootstrap-executables-and-source-entry)

### Trusted source and the seed

Run `cod-seed --parse FILE` for syntax feedback across the selected entry and its reachable source modules. The command stops after source discovery and parsing. Use `cod-seed --check-declarations FILE` to also resolve module selections, nominal layouts and callable signatures. Declarations inside bodies and type checking of executable expressions belong to later work. Both commands report diagnostics without emitting artifacts. Translation uses `--check` with the requested artifact options, or `--run` to execute the result.

`cod-seed --translate FILE` selects the replacement pipeline for supported C-entry closures. It extracts executable operations and frames, completes allocator arguments from the recorded calls, then emits the requested LLVM and C provider artifacts. The current scope includes fixed scalar signatures, ordinary array growth, direct C provider calls with scalar operands and supported loans, and construction and catch-field reads of nominal exceptions with scalar payloads.

The replacement translates the synchronous path-loan chain used by `ModuleIndex.insert_visit`, the CLI catalog loan and the source revision loan forwarded by `NativeSourceUse.with_source`. A single-brace `here` body receives ordered readonly arrays under one actual owner, with independent scalar feedback. An ordinary receiver can offer independent scalar values alongside the arrays, as in the graph traversal callback. Each argument keeps its original position. The arrays keep their nominal type and extent, and scalars are received by value. The body can also use stable scalar or array captures and capture existing borrowed handle and readonly `String` formals. Forwarding uses the resolved incoming place and its actual owner. The C entry validates and owns the text snapshot across the synchronous call. Normal feedback, protected callback failure and invalid native status keep their separate paths.

A named lexical procedure with an independent scalar signature can be called directly from its defining activation. The replacement resolves captured names at the declaration and passes their live places on each call. The linked reporting probe forwards a borrowed native diagnostic handle through ordinary returns and a catch.

The same path-loan code translates `ModuleIndex.lookup_path` and its visit search. The replacement supports direct readonly and writable borrows of finite bits products, complete inline product initialization, initialized lexical scalar cells and unguarded nominal sum cases with fresh bits field reception. The lookup writes its result fields only after the callback completes.

Direct lexical acquisition of an owning C handle registers cleanup on success. An attached borrow shares the selected live parent’s lifetime and adds no release. The linked root-opening probe covers source revision acquisition, path preparation, attachment and lookup with the existing native providers.

The replacement installs a freshly acquired LLVM session under its selected owning symbol. A procedure can use the active session as a direct C handle loan. Successful installations restore the preceding binding and release their own session on exit. An explicit `CString` assertion on a readonly `String` loan preserves its owner and checks for an interior NUL before entering the C provider. The linked object-writing probe constructs and verifies LLVM units, writes an object to a UTF-8 filename and runs the resulting program.

Ordinary protected blocks allocate lexical bits arrays in their enclosing invocation. The replacement also translates unguarded Boolean and nominal enum cases, including older member spellings resolved from the subject type. The linked primitive layout probe writes all five size and alignment rows through the real native layout provider.

A lexical text builder can receive text at construction, be borrowed by a synchronous helper, and grow through the selected text storage append operation. `make String(builder)` materializes an immutable snapshot in the enclosing invocation. A linked probe uses the snapshots with the real provider-file publisher, including allocation and publication failures that preserve earlier files.

The replacement translates lexical owners of regions with explicitly initialized direct bits roots. A fresh region can be constructed in the declaration or returned directly from a named factory. Array allocation and growth use the selected live region, including when an ordinary helper receives its allocator. The verified graph-root path covers projection, capacity failure, initializer rollback and scope release. Unsupported forms report a located translation diagnostic. The complete booter build uses `--check`.

The C++ seed translates trusted, valid CoD source into LLVM IR and bitcode to build the CoD-written booter. The current native build also consumes a C provider header and stubs derived from the same interfaces. The seed must resolve names, types, methods, layouts and other facts as far as their consumers need them to produce correct code. In particular, emitted calls must pass the actual owner or allocator when required. Generated code must implement the specified evaluation order, dynamic checks, exceptions, ownership transfers and cleanup. For a nullable access the seed can emit the prescribed runtime null check directly when eliminating it would require a static proof.

Before 2.0 the booter and `e-wrks/cod` check the [family’s incoming dynamic symbols](../../design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20). Each branch contributes its uses after accounting for successful local installations. Remove the seed’s selected-path missing-binding diagnostic and the analysis used exclusively to establish availability. Where the seed emits a symbolic handler, it uses the installed family’s outstanding-symbol union for capture at activation, replay under `perform` and restoration of the caller on exit.

A `proc` can supply a full-scope literal to one ordinary `here` formal. The seed emits reads of independent numeric, `Bool` and enum symbols using the bindings active each time the receiver invokes the body. Native, forwarded, composed and protected-group handoffs retain their own translation limits. An enclosing handler still needs a replay plan for bindings used through the supplied body. [Direct here support](../../design/ImplementationArchitecture.md#scalar-symbol-reads-in-a-direct-here-body)

For supported live `here` captures, the seed prepares the selected storage address and actual owner. Reads and writes occur in the callback. Semantic checking applies the initialization rule to each reached read.

Remove a static check and the computation used exclusively to support it when translation has no use for its result. This includes whole-program type or ownership proofs whose sole purpose is to reject invalid input. Seed translation makes no claim of source validity. The CoD booter owns type, ownership and other semantic validation, including checking the full source world of its own compiler entry. Record supported self-check coverage separately from generating or running the first booter and from a successful self-build.

The annotated tag [archive/checked-cpp-seed](https://github.com/complyue/cod-project/tree/archive/checked-cpp-seed) preserves the checked seed at `a0922741`. Its 52 focused comparisons across both seed modes and its Lexer, TokenBuffer and ModuleIndex outputs establish the checked-stage reference. The [replacement C++ pipeline](../../design/ImplementationArchitecture.md#replacement-seed-pipeline) is the active development path. Reuse established translation logic where its dependencies are explicit. The booter should apply the seed’s experience with [selectable operations](docs/CompilerPipeline.md#independently-selectable-compiler-operations) when its development resumes. Add an individual opt-in seed check when a concrete engineering use warrants it. Document that use, how to enable the check and the behavior it diagnoses. The default path pays none of the analysis cost unique to that check. The [cutover and acceptance](../../design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance) cover the generated booter and its supported commands.

### Compiler memory acceptance

Keep the C++ seed readable and maintainable. A smaller representation is worthwhile when its measured benefit justifies its complexity. If replacing a standard representation such as `std::variant` would materially obscure the implementation, a higher seed peak is acceptable. Reserve a single host slot for full booter builds and other memory-heavy work. Routine compiler and tooling invocations may overlap them when memory permits. [Build-host concurrency](../../AGENTS.md#concurrency-on-the-build-host)

Verify the linked `cod-booter` executable’s own peak resident memory through process exit on its normal declaration, provider and object commands. Record the executable, host, input size and reachable module count alongside the outcome. Include a small successful program, a larger supported workload, a graph with a shared dependency and a rejected input with its expected diagnostics and preserved output. Compare peaks as source size and graph size grow. Establish practical budgets from these measurements and investigate disproportionate growth, including temporary per-module storage retained across sibling visits. Record the seed’s cost to build the executable separately. A regional capacity alone does not establish the process peak. [Module graph storage and visits](docs/ModuleGraph.md)

The release comparison on x86-64 Linux uses the largest peak RSS from three serial invocations per command with the normal allocator. The following regression budgets apply to the measured fixtures on a comparable build and host. Source size counts all reachable CoD files once.

| Workload | Check declarations | Emit C provider | Emit object |
| --- | ---: | ---: | ---: |
| Small graph fixtures, including shared, wide and nested imports, up to 49 modules and 9 KiB of source | 64 MiB | 64 MiB | 80 MiB |
| Dense source fixtures, up to 512 scalar functions and 32 KiB of source | 72 MiB | 72 MiB | 96 MiB |

The dense-source budget also covers the successful declaration and provider commands on the real lexer. Apply the corresponding budget to comparable damaged inputs and verify their diagnostics and previous artifacts. Record the number of files actually opened when a compiler source input stops early. On the compiler entry at `2bc0e290`, all three commands opened 63 source files totaling 1,810,241 bytes. The larger peak across the two builds was 167.3 MiB for declarations, 165.6 MiB for the C provider and 297.5 MiB for object emission. All three commands rejected the input and preserved prior outputs. Successful full self-compilation still needs measurement. [Fixture baseline](../../CHANGELOG.md#k-booter-memory-baseline-20261124) · [63-file release measurement](../../CHANGELOG.md#k-booter-entry-world-release-20261124)

### Exception syntax in both frontends

The C++ seed and CoD booter must parse the [owning catch header and ordinary ref declarations](../../design/ExceptionControl.md#exception-declarations-and-catch-bindings) and the [normal construction forms](../../design/ExceptionControl.md#make-exception-expressions). Both recognize `catch (e)`, `catch (e:E)` and `catch (e:const E)` in try and defer. A ref or holder declaration prefix in a catch, or an owner constraint in its header, is a syntax error. Locals, parameters and results distinguish owning exception declarations from ordinary ref borrows. Both parsers preserve explicit target constness and construction placement.

Acceptance checks each frontend at its own parse boundary. A valid form carries its binding mode, owner name when present, and construction operands into checking. Malformed syntax fails at parsing. When an execution path is unsupported, the compiler can reject the parsed form during checking or lowering. Supported paths must execute their required propagation, handling and cleanup. For example, parsing `catch (e:const E)` as a borrowed local loses the handler’s release obligation even when the handler only reads e.

### Contextual names in both frontends

Apply the [name rules](../../design/SymbolicDynamicScope.md#clause-words-and-lexical-variable-names) during dependency scanning, parsing and lookup. Cover lexical binders and their uses, `from` and `throws` in clauses, module declarations and ordinary user calls with names such as `fold` and `store`. A lexer accepting the spelling is insufficient if a later phase still diverts it to an intrinsic or rejects the binding. References to unsupported features retain their stage-specific diagnostics.

## Build and run on Linux

The standalone build uses GNU Make 4.3 or newer, a prepared CoD seed and its language library, and a compatible LLVM SDK with shared libraries. The seed prefix provides `bin/cod-seed` and `lib/cod/string.cod`. Use the following commands from the repository root with your ready seed prefix and the existing LLVM SDK in `build`.

```sh
make -C cod/runtime
make -C cod/native/llvm
make -C applications/cod-booter COD_SEED=/path/to/ready-seed/bin/cod-seed
./build-cod-booter/cod-booter applications/cod-booter/workloads/deferred-release.cod \
    build-cod-booter/deferred-release.o
./build/bin/clang build-cod-booter/deferred-release.o -o build-cod-booter/deferred-release
./build-cod-booter/deferred-release
```

The program prints `1` on three lines. Use `TOOLCHAIN` for a ready toolchain prefix. The runtime and LLVM service builds accept `CLANG` and `LLVM_AR`, and the LLVM service also accepts `LLVM_CONFIG`. The booter accepts `COD_SEED`, `CLANG`, `LLVM_CONFIG`, `COD_RUNTIME_ARCHIVE` and `COD_LLVM_ARCHIVE`. Use a separate `OUT` for each build you run concurrently or want to retain, such as `OUT=../../build-cod-booter-shared COD_SEED_FLAGS=--no-specialize`. Make interprets relative paths from the selected build directory. Use paths without whitespace.

The seed processes the booter sources and emits the provider header, C stubs and LLVM IR into `OUT`. The grouped build tracks the library selected by the resolved seed executable. A seed in its CoD CMake development tree can use that checkout's `cod/library` when the installed library directory is absent. When an active project can supply a package override, the standalone build rechecks the source graph before reuse. Clang compiles the generated files and handwritten C adapters, then links the CoD archives and the shared libraries reported by `llvm-config`. The SDK library directory is recorded for the loader. The prepared inputs are read-only. The bootstrap input contract uses the [hand-prepared project directory](../../design/CodProjectStructureAndPkgMgmt.md#directory-layout).

The current bootstrap CLIs use explicit operations. Run a program with `./build/bin/cod-seed --run FILE` after building the runtime. The current booter takes `cod-booter SOURCE OUTPUT` to emit an object, or an explicit declarations or provider request. An installed toolchain can provide `lib/libcod-runtime.a` under its prefix. Set the `COD_RUNTIME_ARCHIVE` environment variable to select a different compatible archive. Link seed-emitted objects or bitcode with the same archive when building an executable outside the seed.

## Next-stage repositories and native runtime

The compiler and toolchain repositories are planned in the [e-wrks GitHub organization](https://github.com/e-wrks).

| Planned repository | Responsibility |
| --- | --- |
| `cod-lang` | CoD language design and the authoritative language specification shared by both compilers. |
| `cod-seed` | The C++ translator for trusted booter source, the CoD-written checking compiler and their C-native components, including the region core and LLVM bindings. The booter stays in this repository after the split. |
| `cod` | The future production compiler written in CoD. The first release targets fast compile-and-run and introduces the experimental `cfe` optimizing CLI. Version 2.0 formally supports `cfe`. Its [output-directory contract](docs/CompilerPipeline.md#default-execution-and-optimized-builds) governs the generated executable. The compiler also provides full diagnostics, [independent check and emission controls for editors and builds](docs/CompilerPipeline.md#independently-selectable-compiler-operations), and a future CoD REPL. [REPL decision](../../CHANGELOG.md#h-retire-cpp-repl-20261125) |
| `codup` | Toolchain version management in the role of rustup. |
| `codmate` | The Language Server Protocol server and IDE plugins, including [reference-site doclet tooltips and navigation](../../design/DomainLanguageDesign.md#definition-doclets). |

The `e-wrks/cod` toolchain will bundle `pcl.cod`, the parser combinator library that implements the parsing HLVM. The production CoD parser is one of its applications. Its DBMR grammar program ships as `cod.grammar` and runs on `pcl.cod` on the compiler’s normal source path. [Installed-toolchain workload](../../design/ParserCombinatorLibSupport.md#packaged-production-parser) · [CoD parser application case study](../../papers/AlgebraicEffectsIsAllYouNeed.md#the-cod-parser-as-an-application-of-the-parsing-hlvm)

Future package management belongs in `e-wrks/cod`. The bootstrap consumes [manually staged source roots](../../design/CodProjectStructureAndPkgMgmt.md#directory-layout).

After the split, a change to what a CoD program means belongs in `cod-lang`. Each compiler repository documents how it implements the rule, its current coverage and any known gaps.

New CoD native components are written in C. The CoD native interface is C-based. The C++ seed, the booter, the future production compiler and foreign adapters consume the same interface. Specify each operation from the runtime behavior its callers need. C++ classes, templates and exception propagation are internal to C++ clients or dependencies. This choice covers CoD’s LLVM bindings as well as region support.

### Shared runtime and independent component builds

The shared language runtime provides allocation and release, backing for callable values and deferred actions, and common exception operations. Build one static archive per target and reuse it when linking the seed's emitted programs and the seed-built booter. The future `e-wrks/cod` compiler and its emitted programs use the same implementation. The seed executable links the archive into the programs it runs. Source sites and program-defined exception names stay with emitted code. The archive uses the canonical intrinsic exception catalog and has no LLVM dependency. The link interface is private to matching compiler and runtime builds.

Reusable C components own their build entry points and outputs. The indexed source and region storage services are header-only. The standalone LLVM service builds its command-construction and verified object-emission implementation against a prepared SDK. It owns complete object publication. The booter keeps its generated stubs and handwritten adapters for CoD handles, status mapping and callback loans. Application builds consume prepared component headers and archives.

At the separate DBMR checkpoint, the region service must [publish a newly created disk-backed region](../../design/ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr) only after initialization and preserve an existing destination in a race. Create private staging on the target filesystem, reject mismatched destinations, and verify the destination at the instant another process can open it. A killed builder and two publishers racing for the same name must preserve every previously published image. The C service follows the CoD operation contract. The [shilos retirement](../../design/ImplementationArchitecture.md#retiring-shilos) covers the old C++ provider and its callers.

For exception regions, distinguish fixed logical capacity, address-space reservation, backing enabled on demand and resident pages. Choose target VM operations behind the interface. Preserve owner identity and locators while enabling pages, and release acquired resources exactly once on partial failure. The C runtime must release owned exception-array members as well as the aggregate region. The native provider should support the [exception capacity rules](../../design/ExceptionControl.md#image-setting-for-exception-region-capacity), including site budgets if their syntax is adopted. Measure virtual address space and resident use separately on target systems.

Specify and check the initial region and LLVM operations alongside the booter increments that use them. Broader interfaces, toolchain distribution and editor integration have separate planning and acceptance. Repository creation, source migration and toolchain operations retain their own authorization.

### Diagnostics by compiler stage

The C++ seed reports failures of the translation it performs, such as an unreadable dependency, an unresolved call target needed by LLVM or an unsupported native layout. Additional engineering checks are opt-in. The seed preserves prior artifacts when translation fails. Its successful output says that it generated the requested program. Static soundness of the input belongs to the CoD booter.

The booter checks source types, ownership, control flow and exception handling, with direct, located diagnostics for failures and honest stops for unsupported cases. The production compiler in `e-wrks/cod` adds diagnostic-only passes, cross-path explanations, counterexamples and suggested fixes. Both enforce the applicable language contracts and preserve required runtime behavior.

#### Diagnostic coverage of the entry module’s fused world

The CoD booter and production compiler check the full fused world prepared from the selected entry. The C++ seed prepares that same world for translation. The world includes the entry, its direct and transitive dependencies, and their selected module interpretations. Seed reporting covers translation failures and explicitly enabled checks within that world. By default there is no diagnostic count limit. Continue discovery and all independent required checks after a recoverable failure. A malformed declaration can block checks that need its type while leaving other declarations, imports and modules checkable. An intact import in a damaged file can establish a dependency for diagnostic traversal. When an import path cannot be resolved, report that failure and continue along the other established edges. A source checker publishes a checked interface only after all required checks succeed. The seed publishes its translation artifacts after resolving the facts needed for emission.

Present each independent finding in a reproducible order, with the source and relevant import or interpretation context. A diamond that reaches the same malformed source twice has one syntax finding for that fault. A failure particular to a different receiving interpretation retains its own context. Package resolution may consult the configured roots. Files outside the entry’s dependency graph have no part in this diagnostic traversal. The stage and its enabled checks determine which findings it can report.

The caller may set a positive maximum for the combined number of errors and warnings in one invocation. Count each reported finding once across all files, phases and selected interpretations. Related locations and import notes belong to their finding. Report the finding that reaches the limit. If work remains, stop, report that coverage was cut short and run the ordinary cleanup. Any error or incomplete check preserves prior output. Warnings keep their severity. Resource exhaustion that makes further progress impossible also yields an incomplete result.

At the checked-seed checkpoint, in both modes the C++ seed loader continues to independent siblings after a recoverable source or import error. It follows intact imports in a damaged file. If two parents reach the same syntax fault, the seed reports it once with both import sites. The seed accepts `--max-diagnostics N` with `--check` or `--run`. A package-shadow warning can consume the last slot before another dependency opens. In that case the command reports incomplete coverage, preserves existing outputs and skips execution. [Seed loader](../../cod/lang_modules.hh) · [Command driver](../../cod/main.cc)

The checked seed also validates independent direct product declarations in a fully parsed module with valid imports even after another reachable file has syntax errors. A failed field leaves its product layout unavailable to inline dependents. The seed still diagnoses an independent layout cycle. [Product schema recovery scope](../../design/ImplementationArchitecture.md#recovering-between-product-declarations-in-the-seed)

Final validation in the checked seed collects concrete uncovered checked exceptions escaping independent named procedure bodies after catches and cleanup. This recovery uses an established, acyclic graph of direct body prerequisites. A rejected body blocks the checks that consume its proof, including module entry when applicable. Earlier type errors in effect solving and unproved dependency profiles retain their existing checks. [Final body validation](../../design/ImplementationArchitecture.md#reporting-uncovered-checked-exceptions-from-independent-bodies)

The released CoD booter continues through independent dependencies and applies `--max-diagnostics N` across the invocation. A child error can block every member of a grouped selector. One such written directive contributes one import note. Reaching the cap exactly returns ordinary rejection when all remaining members depend on that failed child. A separate pending check makes coverage incomplete. [Booter visit](src/ModuleGraph.cod) · [Grouped path review](../../design/ImplementationArchitecture.md#a-failed-path-in-a-grouped-import)

For acceptance, use a valid entry importing two broken siblings, a transitive dependency, and an unrelated broken file in the same directory. Give each broken sibling a syntax error and reach a third syntax error through an intact import in the first sibling. In another reachable module, give two independent products invalid field types. The linked CoD booter reports all five findings. Both build modes of the checked seed also demonstrated this verifier workload before the trusted-source cutover. With `--max-diagnostics 4`, the booter reports the three syntax faults and the first bad field, then reports incomplete coverage. Use a diamond to check repeated discovery. Through the normal compiler commands compare uncapped reporting with a cap of one and a cap above the number of findings. Include a real warning followed by an independent error to exercise the shared budget, its reset on the next invocation and artifact preservation on early exit.

#### Whole-file parser recovery

Every project parser processes the complete input it is asked to parse by default. An explicitly capped compiler invocation can cancel remaining work and records the resulting coverage. For a CoD file this includes lexical scanning, delimiter matching and parsing through EOF, in the C++ seed, the CoD booter and future compiler and editor frontends. A syntax error yields a located diagnostic and a recovered tree. Keep parsing later items in the same list, including independently malformed siblings inside a procedure. Entry points that parse a bounded subproduction give control back to their enclosing parser at the corresponding boundary.

The AST admits an explicit error-report node wherever syntax was expected, including expressions, types, names, patterns, statements and declarations. Record the expected role, the diagnostic or its stable reference, and a half-open span in the original revision. A missing operand may have a zero-width span immediately before its separator. If one fault leaves several slots incomplete, those placeholders may refer to the same report. A missing optional child retains its ordinary representation. Keep correctly parsed children and siblings accessible. Ordinary consumers must distinguish the error alternative before projecting a concrete role. Tools can traverse the valid parts. A file with any syntax error fails compilation, and its recovered forms cannot publish definitions, generate artifacts or replace previous output.

Recover at a boundary justified by the current grammar and delimiter nesting. Each recovery consumes bad input, returns a synchronizing token to an enclosing production that will consume it, or ends at EOF. This also applies to lexical errors and delimiter prepasses. A new line between tokens is ordinary whitespace. A malformed token gets one primary report for the same fault; an independent later error gets its own report. An unterminated triple-quoted token may legitimately cover the remaining bytes through EOF. Preserve that span and any earlier reports. An exhausted physical resource can end the parse with an explicit incomplete outcome.

```cod
proc sample() -> u64 {
    val first <- ;
    val second <- ;
    return 7u64
}
```

For this file return a recovered procedure with two separate missing-expression reports and an intact final return. Report both to the caller in source order. Also exercise a lexical error before a syntax error, unmatched delimiters, a fault near EOF and valid definitions after damaged input. Error nodes and diagnostic iteration can refer to the same evidence. A first-error compatibility field may expose the primary diagnostic, but every independent recovered diagnostic must be available and reported by the normal command. The bootstrap stages can report concise codes and locations; full explanations remain production compiler work. [Native reporting boundary](docs/LLVMInterop.md#builtin-catalog-and-rejection-reporting)

#### Names in diagnostics

When a diagnostic names something through a **name/identity alias**, show both the spelling used at the reported site and the **canonical fully qualified name** of the resolved declaration. For example, if a module imports `Report` from `storage/errors.cod` as `WriteFailure`, the diagnostic can say **`WriteFailure` (canonical `storage/errors.cod::Report`)**. The qualification is display notation. Use the defining module’s canonical identity, enclosing type for a member, and package or version context as needed to distinguish declarations. A renamed re-export leads back to the original declaration. An enum member’s alternate spelling leads to its canonical member. When lookup is ambiguous, show the relevant candidates with their own canonical names. When lookup fails to find a declaration, report the unresolved spelling.

Apply this to errors, warnings and notes, including runtime diagnostics attributed to a named source occurrence. Use the spelling from that occurrence. Two sites can use different names for the same identity. An enum value alone, for example, carries no history of the source spelling. A diagnostic about the value alone can use the canonical name. Keep a reported source location distinct from the defining identity. For a declared place alias, identify the binding and selected place when known. Runtime target sharing alone supplies no alternate declaration name. [Name and place terminology](../../design/CallableViewsAndCaptures.md#alias-terminology)

When the seed or booter reports such a use, it shows both names using the evidence from name resolution and the source occurrence. The production compiler owns additional diagnostic-only analyses under the stage policy above. For a source-attributed runtime diagnostic that names a resolved exception, preserve the pair when lowering that named use. [Exception identity in the object](docs/ModuleGraph.md#exception-identity-layout-and-names-in-the-object)

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

Use the [warehouse policy](../../papers/AlgebraicEffectsIsAllYouNeed.md#two-entries-share-one-fallback) to exercise the semantic pipeline, persistence and native execution together. Inspect the published DCG from CoD. Redirect the retail decline edge in a private successor and verify that the urgent entry still reaches the shared fallback. For a fixture with a textual source, also check ordinary CoD source into a typed graph, exercise the corresponding source edit and compare the reachable operations, decisions and ordered effect traces. This comparison applies where the fixture supplies both source forms.

Validate and publish the edited application DBMR, then select it and rerun it in the same live session. A stale-base edit or invalid program root must leave the previous publication executable. Check that the program can be reopened at a different mapping address and that changing only the effect handler preserves program identity. Exercise the native execution path against reference results and traces, including both reservation outcomes and a handler failure.

Measure **steady-state throughput** on warmed, equivalent workloads and report artifact I/O separately. Measure **edit-to-first-correct-result latency** in a running session from edit submission through validation, publication, any invalidation and compilation, and execution of the first result that reflects the change. The [CoD graph editor](../../papers/AlgebraicEffectsIsAllYouNeed.md#a-cod-program-that-changes-one-edge) supplies the change, and the [warehouse cases](../../papers/AlgebraicEffectsIsAllYouNeed.md#what-the-planner-can-check) specify the inputs, expected decisions and ordered reservation requests. Native throughput and interactive latency remain to be measured in CoD.

## Decisions to revisit before implementation

| Topic | Planning position | Still needed |
| --- | --- | --- |
| Stage caches | Preserve suitable regional results directly as DBMR. | Stage-specific dependency/compatibility rules, publication behavior and measured payoff. |
| LLVM surface | Bind APIs as compiler operations need them. | Checked declarations and generated provider evidence from the first workload. |
| Native handles | Use [scoped owners and explicit borrowed handles](docs/LLVMInterop.md#native-handles-and-c-records). | Generated evidence for actual owner binding and bounded views, target layout, general fallible disposal and the build graph. |

Current implementation evidence belongs with the booter workloads and the seed capabilities they exercise.

### Guarded scalar results

The native module compiler can retain a compatible non-null scalar target from a `try` used as a conditional arm. A value read within the guard prepares its bits before `finally`. When the enclosing conditional joins the guarded target with independent bits, it reads the selected target after the guard completes. The current subset requires one result category across the completing branches of that guard. It admits a nullable target from a guarded arm under explicit discard.
