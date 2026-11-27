# CoD booter: a compiler application written in CoD

Roadmap revision: 54 · Recorded: 2026-11-27 · Phase: initial seed delivery through alternating booter and seed development.

**Compiler design target.** On the command line, `cod` (Compile on Demand) compiles the entry for the host and executes it immediately, leaving no user-facing executable. The planned production command `cfe` (Compile for Ever) builds optimized executables for the host or a supported cross target and saves them for later use. The booter provides one practical code-generation mode. Both bootstrap compilers prioritize Go-like compile speed with moderate runtime performance. User-selectable performance options belong to `cfe`. **The seed’s default bootstrap build produces and keeps the `cod-booter` binary.** [Compiler modes and release milestones](docs/CompilerPipeline.md#default-execution-and-optimized-builds)

## Design authority

At r54 the [booter source ergonomics decision](../../CHANGELOG.md#h-booter-source-ergonomics-20261127) applies [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap), P-642/P-643, to the [next seed and library work](#booter-source-ergonomics). The trusted seed translates these author-facing forms for the booter. The r53 compiler roles and native interfaces continue.

At r53 the [native package decision](../../CHANGELOG.md#h-native-package-toolchain-20261127) assigns the [production C, C++ and assembly toolset](docs/CompilerPipeline.md#codup-toolchain-for-native-packages) to the `codup` distribution under D-20-TARGET-TOOLCHAIN. The r52 repository roles, host/cross split and C implementation of the shared runtime continue.

At r52 the [production target decision](../../CHANGELOG.md#h-production-cross-build-20261126) extends the `codup` distribution role under [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories). It preserves r51’s single booter mode and assigns [cross-builds](docs/CompilerPipeline.md#production-cross-builds-and-target-runtimes) to `cfe`.

At r51 [D-20-COMPILER-MODES](../../design/DesignAuthorityRegistry.md#d-20-compiler-modes) records the [user’s decision](../../CHANGELOG.md#h-booter-single-codegen-mode-20261126) on booter performance options. The command outputs and checking responsibilities carry forward.

At r50 the [post-rewrite cleanup decision](../../CHANGELOG.md#h-post-rewrite-cleanup-20261126) applies the existing bootstrap scope to command retirement in the seed and to the booter after its rewrite. [D-20-BOOTSTRAP](../../design/DesignAuthorityRegistry.md#d-20-bootstrap) continues from r49.

At r49 the [booter pipeline rewrite](docs/CompilerPipeline.md#replacing-the-booter-pipeline) applies [D-ARCH-REVIEW](../../design/DesignAuthorityRegistry.md#d-arch-review) to the CoD compiler delivered by the replacement seed. The [clarification](../../CHANGELOG.md#h-compiler-restart-mechanism-20261126) preserves the booter’s checking, native interfaces and observable command behavior.

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

### Booter source ergonomics

**Return the package lookup result by value.** The first seed change supports an explicit independent finite-bits nominal result on an ordinary CoD call. In the [resolver](src/PackageModuleResolution.cod), write `-> val PackageResolution` and return the selected alternative. The [caller](src/PackageGraphPaths.cod) can then match the result of one invocation directly, retiring the one-element output array and its extra owner. Preserve the tag and its active fields, including both root indices on ambiguity. The caller receives the value on normal completion after cleanup. [Logical result destination](../../design/ConstructionAndAcquisition.md#caller-provided-result-destinations)

**Use the generic linked deque in CoD.** [`Deque.cod`](src/Deque.cod) exports `deque[T]` and operations at both ends. Use `make deque[UInt64]()` to construct a ready empty deque at its chosen placement. The [source contract analysis](src/SourceContracts.cod) uses it for pending definition IDs and releases its nodes at each pass exit. The [completion checker](src/CompletionChecking.cod) retains its indexed membership and pending tables. Linked probes cover `UInt64` and the finite-bits sum `PackageResolution`, including a pointer to a detached value after later insertions. [Value-only item and construction contract](../../design/CollectionsAndBuiltinContainers.md#container-pointer-results)

**Reuse typed blocks in the graph region.** The [key and spelling tables](src/ModuleIndex.cod) share a CoD-written `SourceBytePool` allocated in the graph region. The pool can reuse a surrendered `SourceBytes` backing of the exact requested length. Growth copies the prefix and initializes the tail before publishing the replacement. The previous backing enters the pool after the rebind. The initial manager keeps one spare and uses the existing zero-length backing when there is nothing to reuse. The visit table uses its own growth path. Whole-region acquisition, relocation and release retain their existing contracts. The generic growable vector follows this work. [Intra-region library policy](../../design/RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy)

The seed translates the pool on the normal booter build. Both booter source commands accept the unchanged `ModuleIndex.cod` and a direct renamed import of `reserve_keys`. The completed source contract covers the pool loan, its same-owner byte-array field and the separate caller array slot. Both source commands read the fixed reference fields of `ModuleBindingWorkspace` and check the full `ModuleExports.cod` and `NameResolution.cod` modules. With a diagnostic limit of one, checking `ModuleGraph.cod` reaches the read of `facts[dependency]` in `AggregateSchemas.cod`. The element type of `AggregateFacts` is the nominal enum `AggregateStorage`.

**Group related operands that share an owner.** [`bind_graph_module`](src/ModuleGraph.cod) receives its syntax, name, publication and checking tables through one `ModuleBindingWorkspace` under `W`. Its fixed reference fields select the existing tables. The nodes and object prefixes retain readonly target access. The synchronous source loan captures one fixed workspace reference, while its source views, diagnostic destination, catalog and graph storage keep their own owner contracts. [Reference binding and bits transfer](../../design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer)

The three [compiler commands](src/ObjectCompiler.cod) lend one `GraphProgress` product from their graph region to the shared traversal. The traversal forwards its selected fields to the graph checker and reads the resulting counts from the same storage.

### Bootstrap executable and entry names

The selected name for the C++ build output is `bin/cod-seed` (`bin/cod-seed.exe` on Windows). That compiler translates `applications/cod-booter/src/__main__.cod` to build the single `cod-booter` executable. Its launcher dispatches compilation, declaration checking and provider generation to three procedures in that entry module. The bootstrap commands continue to accept explicit source paths. [Implementation and cutover](../../design/ImplementationArchitecture.md#naming-the-bootstrap-executables-and-source-entry)

### Trusted source and the seed

Run `cod-seed --parse FILE` for syntax feedback across the selected entry and its reachable source modules. The command stops after source discovery and parsing. Use `cod-seed --check-declarations FILE` to also resolve module selections, nominal layouts and callable signatures. Declarations inside bodies and type checking of executable expressions belong to later work. Both commands report diagnostics without emitting artifacts. The standalone booter build uses `--translate`. Run `cod-seed --translate FILE` with the requested LLVM IR, bitcode or C provider outputs for a trusted entry with a C entry point. Link those products with the matching native providers and runtime.

`cod-seed --translate FILE` selects the replacement pipeline for supported C-entry closures. It extracts executable operations and frames, completes allocator and replay requirements from the recorded calls, then emits the requested LLVM and C provider artifacts. The current scope includes fixed scalar signatures, ordinary array growth, direct C provider calls with scalar operands and supported loans, and construction and catch-field reads of nominal exceptions with scalar payloads.

Ordinary CoD calls can return a borrowed scalar or an admitted nominal payload under an input owner. A `ptr` result carries the selected target or null through the existing completion path. The caller can forward it through a `ptr` argument or binding, or match it once with `(null)` and an exact typed `(ref item:T in O)` arm. The present arm keeps the target's access and actual owner. This supplies the seed's result and case operations for deque pop. The C-facing result ABI is unchanged.

The seed can construct a braced product with same-owner `ref` and `ptr` fields in a named region or in fixed lexical storage. Regional fields store relative locators and decode them with the live controller. A writable field slot can be rebound independently of writes through its selected target. An ordinary direct helper can borrow a recursive product and return a selected node or payload. Each owner role for such a loan has one established placement per emitted helper. A regional role receives the actual controller in the existing private owner argument. Const owner access permits resolving links and does not grant allocation. A helper can serve distinct live regions with the same layout. A mixture of lexical and regional placements for the same role in one helper has a located translation limit.

A direct helper receiving a product under a known writable lexical owner can construct a product or a finite-bits sum in that owner. Its allocation capability follows the same completed call dependencies used for arrays. The supplying full scope releases the extents on exit. Complete bits products can initialize inline fields from an existing view or an independent call result. The source contract work stack gives each pass its own lexical owner. A pass allocates at most one node per queued function and releases all its nodes on exit.

A product can hold references to named arrays under its actual owner. Regional fields store the array header's relative offset and preserve the selected array's nominal type, backing and access. The graph's scoped dependency handler captures its fixed incoming pool reference with the established regional placement. An attached synchronous body can capture a fixed lexical product reference and forward it to a direct helper. This is how the booter lends the binding workspace.

The seed can also construct a finite-bits sum in a known writable region and bind a product's reference field to that sum under the same actual owner. A bits transfer into a sum selects and checks its destination before the right operand, then copies the active payload and tag on success.

Linked deque probes exercise both ends and preserve detached values. Repeated lexical make by ctor and finite-bits make by init produce distinct targets when a reference can retain them. Earlier targets remain live in the full scope, including if a later allocation fails. A directly copied bits initializer can use a scratch row because only its payload reaches the receiving storage. Separate regional and lexical reference probes verify ordinary target selection across alias writes and field rebinding.

The seed translates the synchronous path-loan chain used by `ModuleIndex.insert_visit`, the CLI catalog loan and the source revision loan forwarded by `NativeSourceUse.with_source`. A single-brace `here` body receives ordered readonly arrays under one actual owner, with independent scalar feedback. An ordinary receiver can offer independent scalar values alongside the arrays, as in the graph traversal callback. Each argument keeps its original position. The arrays keep their nominal type and extent, and scalars are received by value. The body can also use stable scalar, array and finite bits product captures, as well as existing borrowed handle and readonly `String` formals. Forwarding uses the resolved incoming place and its actual owner. The linked source-loan probe lends the original `ObjectPrefixes` place, checks a field update in the supplier after the callback and preserves readonly target access. The C entry validates and owns the text snapshot across the synchronous call. Normal feedback, protected callback failure and invalid native status keep their separate paths.

A named lexical procedure with an independent scalar signature can be called directly from its defining activation. The replacement resolves captured names at the declaration and passes their live places on each call. The linked reporting probe forwards a borrowed native diagnostic handle through ordinary returns and a catch.

The same path-loan code translates `ModuleIndex.lookup_path` and its visit search. The replacement supports direct readonly and writable borrows of finite bits products, complete inline product initialization, initialized lexical scalar cells and unguarded nominal sum cases with fresh bits field reception. The lookup writes its result fields only after the callback completes. A local `ref` can name its initializer's exact target type. A finite bits sum can be borrowed through an ordinary call, as in the `VisitObject` loan to `ModuleIndex.mark_checked`.

Direct lexical acquisition of an owning C handle registers cleanup on success. An attached borrow shares the selected live parent’s lifetime and adds no release. Replay accounts for the constructor’s callable kind and any further callback from the owning release. The linked root-opening probe covers source revision acquisition, path preparation, attachment and lookup with the existing native providers.

The replacement installs a freshly acquired LLVM session under its selected owning symbol. A procedure can use the active session as a direct C handle loan. Successful installations restore the preceding binding and release their own session on exit. An explicit `CString` assertion on a readonly `String` loan preserves its owner and checks for an interior NUL before entering the C provider. The linked object-writing probe constructs and verifies LLVM units, writes an object to a UTF-8 filename and runs the resulting program.

Ordinary protected blocks allocate lexical bits arrays in their enclosing invocation. The linked primitive layout probe writes all five size and alignment rows through the real native layout provider.

The seed translates unguarded Boolean and nominal enum cases and typed enum expressions. A typed name follows ordinary lexical lookup before consulting the enum member table. Integer literals are matched before narrowing, and a runtime backing value is evaluated once and raises `TypeAssertionFailedError` when it is outside the declared domain.

The replacement also supports Boolean guards on these arms and on nominal bits sum arms. The subject is evaluated once, and matching field receptions are available to the guard and its body. A false guard continues to the next arm. A failure propagates through the enclosing cleanup. For a borrowed subject, coverage after a guard uses the subsequent tests or an unguarded fallback. The linked parser probe exercises the real `ParserInput.closing_token`. A bare `break` can complete a void case and continue its enclosing computation. The linked case-schema probe verifies that the surrounding range still visits later candidates.

A lexical text builder can receive text at construction, be borrowed by a synchronous helper, and grow through the selected text storage append operation. `make String(builder)` materializes an immutable snapshot in the enclosing invocation. The linked provider-file probe calls both bundled `append` methods through a renamed selection and checks allocation and publication failures against previously published files.

The replacement also translates a writable, non-null abstract reference symbol when the compiled declarations establish one concrete builder layout. An ordinary call prepares the selected target and actual owner together at that argument. Later operands can change the current binding, and the prepared pair still reaches the selected callee. Linked renderer probes cover normal and exceptional restoration, including scopes that borrow provider-prefix records and lexical text snapshots.

The bundled UTF-8 storage intrinsics can read a text size or byte and create a slice owned by the enclosing full scope. Linked `ProviderNames` probes read the reservation rules and render an identifier. They also exercise ordinary scalar conversion at a typed local declaration, out-of-bounds access, and an invalid UTF-8 slice boundary.

The replacement resolves ordinary fixed positional overloads. It extracts the operand types in source order, excludes methods that cannot accept those types, and requires common parameter modes, access and owner roles among the remaining candidates. Selection prefers exact types to lossless scalar preparation and requires a unique most-specific method. Linked controls cover the bundled text and integer `emitStr` overloads, nominal enum preference over its backing, lossless widening and once-only operand evaluation.

An ordinary direct call with one generic definition can infer type parameters written directly as formal types. Repeated occurrences require one canonical type. The seed completes a signature before extracting its body and uses one body for each defining method and ordered type-argument tuple. The same tuple shares an instance across recursion and source-preserving aliases. Concrete operands still follow ordinary value, borrow and owner rules. An independent aggregate result requires `val` and a finite-bits layout. The linked generic probe exercises both scalar and nominal sum arguments. A generic product pattern can also infer its arguments from the actual product’s canonical template and argument tuple. Selection among generic overloads remains outside this profile.

The seed resolves bracketed generic product applications in template fields, ordinary direct signatures, local annotations and construction heads. The layout builder completes concrete rows on demand and shares them across using aliases. It supports exact recursive ref and ptr links and rejects recursive inline layouts and recursive changes to the type arguments. A niladic generic product initializer can explicitly initialize all mutable pointer fields to null. The seed selects the initializer from the concrete construction head, supplies the actual destination owner, and publishes the initialized object after the call. Fields embedding such applications in ordinary product declarations, other generic initializer shapes and a mixed lexical and regional owner role in one ordinary method retain their translation limits.

A conditional can supply a readonly text argument when its completing arms share one established owner. The linked provider-suffix probe checks a single condition evaluation and skips the provider call when the selected arm fails.

An intermediate ordinary call can use an omitted void result once the selected body establishes a no-value exit. The replacement prepares the statement's operands during extraction and gives the call its result and ABI after the callee's evidence is complete. The linked probe reaches `DeclarationTypes.prepare_native_operand_types`. A separate range control observes preserved writes after a callee failure and unchanged storage after a failure preparing arguments.

A plain declared place alias of a lexical scalar `val` shares that variable’s storage and access. The parser’s augmented transfer expansion uses this path for the index updates in `ModuleLowering.receive_formals`. The linked operand probe retains a write made during argument evaluation and skips the final transfer if that operand fails.

A standalone full scope used as an intermediate statement in an ordinary body can establish its own void result. It borrows the enclosing places and runs its cleanup before the enclosing continuation or catch. An ordinary statement guard with an empty alternative can invoke such a scope when its own body establishes a void result. The linked `TokenBuffer.scan_tokens` probe distinguishes nested delimiters from crossed delimiters. Other linked probes cover natural completion, an explicit local return, writes through an incoming scalar borrow and propagation of an abrupt branch. A range iteration can discard incompatible conditional arm values while preserving their effects and failures, as in the linked `ModulePaths.classify_module_path` probe.

An explicitly typed ordinary body can directly return an exhaustive case with a full-scope arm. The scope establishes its result from its local returns. The case then joins exactly matching scalar or enum results before the enclosing return consumes the joined value. Linked indexed cases verify once-only subject evaluation, capture of received fields, and cleanup before normal delivery or propagation of a failure. General case joins and throw-only standalone scopes remain outside this translation profile.

The seed translates a private live handler with one layout per symbol. Each activation saves the live places, required allocator capabilities and outstanding scalar bindings of the installed family. Definite local installations supply uses within their scope. Uncalled members of a reached ordinary family contribute to capture without requiring emitted methods. The collector follows bound synchronous callback bodies and potential further callbacks through the linked C entries and live descriptors. An unbound dependency or a capture that needs non-scalar transport still reports a located translation limit.

The linked native cleanup probe reads 7 during `perform`, sees the caller restored to 29 after normal or exceptional exit, and reads 29 during `behave`. A failing argument does not enter the handler. The linked visit-table probe exercises the actual `ModuleIndex.reserve_visits`, including failed prefix copying before rebinding. The recursive and swapped-owner probes verify allocator forwarding through recorded owner substitutions. [Handler capture and replay](../../design/ImplementationArchitecture.md#emission-boundary-for-live-handler-installations).

Bare `throw;` forwards the original failure from the nearest active catch with its original location and any owned payload. Translation covers direct forwarding out of that catch. Interception inside the same catch remains a separate ownership boundary.

The seed translates lexical owners of regions with explicitly initialized direct bits roots. A fresh region can be constructed in the declaration or returned directly from a named factory. Array allocation and growth use the selected live region, including when an ordinary helper receives its allocator. The verified graph-root path covers projection, capacity failure, initializer rollback and scope release. Unsupported forms report a located translation diagnostic. The unchanged booter entry links from `--translate` output and runs declaration checking, provider generation and object emission. The linked relative-import program prints `11` and `5`, and the deferred-release program prints `1` three times.

The C++ seed translates trusted, valid CoD source into LLVM IR and bitcode to build the CoD-written booter. The current native build also consumes a C provider header and stubs derived from the same interfaces. The seed must resolve names, types, methods, layouts and other facts as far as their consumers need them to produce correct code. In particular, emitted calls must pass the actual owner or allocator when required. Generated code must implement the specified evaluation order, dynamic checks, exceptions, ownership transfers and cleanup. For a nullable access the seed can emit the prescribed runtime null check directly when eliminating it would require a static proof.

Before 2.0 the booter and `e-wrks/cod` check the [family’s incoming dynamic symbols](../../design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20). Each branch contributes its uses after accounting for successful local installations. The seed uses binding records to generate code and leaves proof of symbol availability to the semantic checkers. Where the seed emits a symbolic handler, it uses the installed family’s outstanding-symbol union for capture at activation, replay under `perform` and restoration of the caller on exit.

A `proc` can supply a full-scope literal to one ordinary `here` formal. The seed emits reads of independent numeric, `Bool` and enum symbols using the bindings active each time the receiver invokes the body. Native, forwarded, composed and protected-group handoffs retain their own translation limits. An enclosing handler still needs a replay plan for bindings used through the supplied body. [Direct here support](../../design/ImplementationArchitecture.md#scalar-symbol-reads-in-a-direct-here-body)

For supported live `here` captures, the seed prepares the selected storage address and actual owner. Reads and writes occur in the callback. Semantic checking applies the initialization rule to each reached read.

Remove a static check and the computation used exclusively to support it when translation has no use for its result. This includes whole-program type or ownership proofs whose sole purpose is to reject invalid input. Seed translation makes no claim of source validity. The CoD booter owns type, ownership and other semantic validation, including checking the full source world of its own compiler entry. Record supported self-check coverage separately from generating or running the first booter and from a successful self-build.

The annotated tag [archive/checked-cpp-seed](https://github.com/complyue/cod-project/tree/archive/checked-cpp-seed) preserves the checked seed at `a0922741`. Its 52 focused comparisons across both seed modes and its Lexer, TokenBuffer and ModuleIndex outputs establish the checked-stage reference. The [C++ seed pipeline](../../design/ImplementationArchitecture.md#replacement-seed-pipeline) uses one code-generation mode. Reuse established translation logic where its dependencies are explicit. The CoD booter needs its own [pipeline rewrite](docs/CompilerPipeline.md#replacing-the-booter-pipeline) to keep producer traversal out of reach of consumers. The [selectable operations](docs/CompilerPipeline.md#independently-selectable-compiler-operations) are a separate client-facing goal. Add an individual opt-in seed check when a concrete engineering use warrants it. Document that use, how to enable the check and the behavior it diagnoses. The default path pays none of the analysis cost unique to that check. The [cutover and acceptance](../../design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance) cover the generated booter and its supported commands.

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

### Fixed references

An initialized `const ref` local selects a supported scalar target once. The fixed slot cannot be rebound. A writable target can still receive a bits transfer or an ordinary mutable loan through that reference. An omitted target annotation inherits access from the initializer. In `const ref saved:const UInt64 = source`, both the slot and the access through it are restricted.

An ordinary scalar reference local can rebind to an existing initialized target with the same type and owner and compatible access. A failed right operand leaves the old target selected. The assignment result exposes the destination target with its declared access. A fixed reference taken before the rebind keeps the earlier target. The source commands check the body containing a fixed reference, including an unused binding.

The declaration and C provider commands also check fixed locals initialized from a named byte-array loan with a `UInt64` index. The loan can be an ordinary reference formal or a previously checked local. The checker requires the exact nominal array and owner. A writable target supports indexed byte stores and ordinary mutable loans. Rebinding an ordinary slot under that owner leaves earlier selections intact. Byte-array object emission, grouped fixed references and bare fixed declarations retain their profile limits. [Binding and target qualifiers](../../design/RegionalOwnership.md#independent-binding-and-target-qualifiers)

Both source commands also check lexical references initialized by an unplaced braced constructor of a selected imported finite-bits sum. A mutable slot can select a fresh target of that nominal sum under the same lexical owner, including a slot declared with readonly target access. A failed constructor leaves the previous target selected. At a conditional continuation, the checker retains the common type, owner, access and readiness. Copying the selected bits into an inline field is checked in the original `append_block_item` and `append_block` bodies. A fixed slot cannot rebind.

### Array growth and reference-slot calls

The declaration and C provider commands check `reserve_keys` with its original `SourceBytes` type and `growth_capacity` helper. Within the same module, a `ref var` formal can forward the caller's initialized writable slot. An ordinary reference formal lends its own local slot. Rebinding requires the same nominal array and actual owner, and construction separately requires a writable allocator role. Fixed slots, readonly targets and temporary arrays cannot supply a writable reference slot.

Both source commands check direct imports of completed reference-slot procedures, including a rename of `load_package_inputs`. The published contract keeps each slot initialized under the same nominal type, access and actual owner on normal and escaping exits. A slot may select a different backing on either exit. This admits a caller that reads the current slots after the loader and keeps a separately prepared loan of the original bytes.

The seed can match a nullable named-array reference with `null` and an exact typed `ref` reception. The reception selects the array header and keeps its actual owner and access. Both lexical and regional backings use the ordinary array operations on the selected target. A readonly view cannot supply a writable reception.

The importing visit checks the actual writable slots, owner roles and allocation requirements. It applies each declared or recorded unchecked failure after operand preparation, so a failed later argument prevents entry and preserves earlier effects. Deferred and callback slot forwarding and object lowering remain outside this profile. [Key-array checking contract](docs/ModuleGraph.md#checking-growth-of-the-module-key-array)

The source checker can handle the private `load_for_edge` body in its defining revision and publish the mixed signature of `prepare_logical_dependency` for direct imports. Its two `ref var` arguments keep their separate lexical slots. The ordinary arrays, readonly `PackageEdge` and path handles each use their prepared target and actual owner. The importing visit can lend a locally acquired writable destination handle for the duration of the synchronous call. Both source commands verified this path with the array-backed resolver at `eadfbd47`.

The same mixed caller can import `report_package_issue`. Its body constructs a lexical array even when the length is zero. The checked interface carries possible allocation failures separately from bounds, numeric faults and the `UInt8` result. Local lexical allocation grants no allocator capability through the interface. Both these imports are covered by a source-command witness with a renamed selection. The full `ModuleGraph` and `__main__` inputs still stop in dependency checking.

The source commands check the two distinct slots in `insert_visit` in a same-module reduction with the original bodies. Each slot keeps its own saved place and exact nominal array. Both use the actual allocator for the shared owner role. The staged callable keeps the separate path-handle role and the nominal `CompilerStatus` result. In the linked seed witness, a bounds failure during the second growth leaves the first growth and its copied visit visible. The original keys backing and publication counts survive that failure.

Both source commands also check `reserve_visits`. The visit-array profile requires a locally declared `UInt64`-indexed array whose exact sum element has a completed finite bits schema. The fieldless fill must belong to that sum. The checker prepares the indexed destination before the source, including both bounds checks, and admits the whole-value transfer in discarded position. The final rebind follows the copy loop. Booter object emission uses a separate profile. [Visit-array checking contract](docs/ModuleGraph.md#checking-growth-of-the-module-visit-array)

The source commands also check a local braced constructor written directly into a writable element of a finite-bits sum array. They verify each initializer and the exact nominal element type, including nested inline bits fields. The isolated `mark_failed_visit` body passes both commands with its original schema. Object lowering and uses that consume the transfer's result remain outside this profile. [Indexed sum transfers](docs/ModuleGraph.md#checking-a-local-sum-constructor-in-an-indexed-transfer)

The same source commands can check a selected imported constructor in a discarded indexed write. They use the defining array’s published index and element types and the constructor’s published fields. Nested product constructors and copies from live readonly loans use each field’s exact nominal type. Both commands check the unchanged `ParserInput.append_syntax_node` in the module prefix, including its ordinary `len(nodes)` capacity guard.

Both source commands can check a lexical array of an imported finite-bits sum, including its nominal fill and an indexed case subject. They also check the direct `val PackageResolution` result in `PackageModuleResolution` and its imported case in `PackageGraphPaths`. The defining visit checks the nominal return and publishes the result for the supported readonly array procedure after completion checking. The importing visit checks the actual arguments and binds the case arms to that exact nominal sum. The seed prepares the tag and active fields before local cleanup and publishes the independent result only after normal completion. The linked booter uses this route for package lookup.

### Inline sum cases on borrowed records

The declaration and C provider commands check `case row.payload of { … }` on a readonly, nonnull loan of a finite-bits record. A direct selection from a local or imported `UInt64`-indexed record array can serve as the receiver. An already-demanded caller can also pass the whole element of an imported array to the exact readonly formal of an ordinary same-revision function. The published array and record schemas supply nominal identity. The call adds the selected helper as a body dependency of that caller. Each use keeps the defining module and nominal identity of the record and sum. The projected field retains the receiver’s owner. An indexed selection evaluates its index once and must succeed before the case reads its tag or the call enters. Constructor selection, field reception and case coverage follow the ordinary sum rules.

The readonly indexed element loan is scoped to the field use or immediate call. The source checks require compatible readonly formals and owner substitutions and reject deferred use of the loan. These checks cover `ModuleExportStorage` on both source routes. Both commands also check the unchanged guarded `tokens[opening].delimiter` case in `ParserInput.closing_token`, including `len(tokens)`. Object emission for these sum cases still reaches its existing profile boundary.

An ordinary same-revision function can also receive a writable element of a local `UInt64`-indexed finite-bits record array in the declaration and C provider checks. The indexed argument must match the checked formal and supply a live writable target under its actual owner. This is the call used by `InitializationFlow` to update an operation in place.

A same-revision call can prepare an indexed readonly record loan alongside ordinary writable array and record loans. Both source commands check the unchanged `append_case_pattern` call with its selected token, writable syntax array and initialized parse result. Each writable loan has its own nominal type, access check and actual owner. The targets can share an actual owner. A failed operand prevents entry, preserving earlier preparation. This profile uses ordinary array references.

### Fresh sum arguments

The declaration and C provider commands check a selected, unplaced braced sum constructor supplied directly to an ordinary same-revision function with an independent scalar or void result. The immediate formal must be a readonly reference under a readonly owner role. The construction uses the caller's lexical owner. Each actual has its own target, so two constructions in that scope can share one formal owner role. A failed later operand prevents entry. Deferred use and borrowed results are outside this profile.

Both commands check the original `OwnerQualifiedType` argument in `ParserInput.append_qualified_type`, including the two offset calls.

A lexical reference can also select a newly constructed finite-bits sum in the same revision. The written constructor determines the parent sum and the checked field order, including an imported inline product such as the `SourceSpan` in `ModuleExports.GraphSpelling`. An annotation must agree with the exact nominal sum. The reference selects storage under its actual lexical owner after successful initialization.

### Readonly lexer predicates

The source commands check `Lexer.word_is`, its direct wrappers and the composed predicates used by `ParserInput.parse_binding`. Ordinary operand checking prepares the readonly byte-array and token loans and the optional immutable String literal. The defining visit records direct calls to the selected same-revision functions and the short-circuit continuations. It completes the recorded dependencies leaf first, including forward references and shared leaves. An early return ends that path. A skipped branch contributes no failure. Dependencies outside the represented call shapes and cyclic dependencies leave the profile unavailable.

The published interface supplies the Boolean result, the possible catchable bounds outcome and the separate possibility of a fatal numeric fault. A direct imported call applies the signature to the live loans. The indexed token operand keeps its selected array, index and actual owner. Its bounds check precedes callee entry. An immediate imported call can propagate fatal arithmetic through lexical cleanup and `finally` while bypassing language catches. Supplied bodies, delayed calls and scopes with an active defer retain their separate completion limits.

Both source commands accept the complete, unchanged `ParserInput.cod`. These predicate interfaces are for declaration checking and provider generation. Executable object support has its own admission checks.

### Imported records in indexed transfers

The source commands check `TokenBuffer.mark_delimiter` in the unchanged module prefix. They select the imported `Token` constructor and the writable `Tokens` element using their defining schemas, including through renamed selections. In a discarded builtin transfer, the destination index and bounds check precede the written initializers. The token's scalar fields come from checked reads, and its delimiter field copies the bits of the readable sum under its actual owner. Only a completed fresh value replaces the destination row.

The indexed construction check carries no object lowering evidence.

### Imported indexed readers

Both source commands check the unchanged `ParserInput.punct_at` body and its use in `TokenBuffer.close_delimiter`. The defining visit completes the readonly byte-array reader with a second readonly loan of a finite-bits record array, two independent `UInt64` values and an independent `UInt8` result. A checked literal import can supply the token-kind code. At a source-preserving import, the two nominal arrays and their actual owners remain independent. A writable actual can supply a readonly loan. An independent `UInt8` or `UInt32` can widen to either `UInt64` formal. A failed later operand prevents entry.

The same checked signature can publish an independent `UInt64` result. In `ParserInput.exception_binder_width`, the defining visit uses the completed imported Lexer predicates and the same-module byte reader. It combines their recorded completions with its own operations after operand checking. A short-circuited operand contributes no execution effects.

The contract carries possible bounds failure separately from possible fatal arithmetic. Bounds use ordinary exception routing. The immediate call uses the same fatal exit in a protected body, a handler or a finalizer, subject to the active-defer limit.

### Initialized writable token loans

The source commands check `Lexer.scan_token` and its local helpers. The checker verifies scalar field transfers on the selected finite-bits product, including the field's declared access. The ordinary initialized loan provides the current value of every field. The call can update `kind`, `first`, and `past`. It supplies no initialization promise. The caller reads live fields after a successful call, and a completed store remains visible if a later operation fails.

The defining visit completes bounds and fatal arithmetic evidence over the recorded ordinary call dependencies. An unsupported entered dependency leaves the source call proof unavailable. On a source-preserving import, the caller prepares the byte-array loan, offset, and writable record loan in written order. For `tokens[index]` the bounds check and target selection precede entry. The owner role follows the selected record rather than the intervening value operand.

Object emission for the writable loan has its own admission checks.

### Imported byte predicates

In a body already required to be checked, a direct source-preserving `using` selection can call an ordinary fixed function with one independent `UInt8` value and a `bool` result. The defining visit checks the single returned predicate and completes its exact local call dependencies before publishing the result and signature. Only a proved normal contract with no possible bounds or fatal arithmetic outcome after entry is published in this profile. The caller evaluates and checks the actual once. An indexed read can still fail before entry. Cyclic or unrepresented dependencies leave the contract unavailable.

Both source commands accept the full unchanged `TokenBuffer`, including its imported `Lexer.lexical_error` call.

### Imported syntax-row writers

The source checker can use the completed `ParserInput.append_syntax_node` contract across a direct `using` selection, including a rename. The defining visit proves the ordinary function's body and publishes the five ordered formals together with possible bounds and fatal arithmetic outcomes. The caller prepares a writable array, a writable record, two independent `UInt64` values and a readonly loan of an exact finite-bits sum. The two writable formals can share an actual owner. A direct braced construction uses the selected constructor's defining schema, evaluates its initializers in order and lends the completed value under the caller's lexical owner. An abrupt operand prevents entry and preserves the effects of earlier operands.

Both source commands accept the reduced call to unchanged `append_syntax_node`. They also bind an imported finite scalar-layout record to its writable ordinary reference formal before reading any operands. In an already demanded caller, a direct same-module function can receive that initialized record alongside ordinary array loans. The source checker follows the recorded call edge to check the receiving body and preserves the caller's actual target and owner. A readonly reference cannot supply the writable formal, and separate declared owner roles do not imply separate actual owners.

### Imported binding-name parser

Both source commands now check the imported call to `ParserInput.parse_binding_name` in the unchanged `CallableSyntax`. The defining visit completes the bound parser, recovery, and syntax-row helper dependencies from their checked operations. The result is an independent Boolean. The contract carries possible bounds failure and fatal arithmetic separately. An unsupported entered dependency leaves the contract unavailable.

A direct source-preserving selection prepares the two readonly array loans, the writable syntax array, the initialized writable `ParseResult`, and the scalar operands in written order. The borrowed targets keep their exact nominal types, access and actual owners. A failed operand prevents entry. Successful stores remain visible if a later operation fails. The linked seed witness checks a qualified binding, a missing type, a full syntax array and local binding recovery.

### Imported lexical record construction

In a body that needs operand checking, both source commands check an unplaced braced construction of an imported record with a completed finite-bits C layout. The selected type supplies the initializer fields, including through a rename. The checker verifies each field in written order. A writable field can receive another initializer. A readonly field is initialized exactly once. The local reference selects the completed target under the lexical owner and can restrict its access. A scalar member read or write uses the field of that exact target.

### Local indexed product outputs

Both source commands check an exact local array element lent to a same-revision function’s writable finite-bits product formal. For the selection projection in `ModuleExports.cod`, the bound indexed argument makes the receiving body a dependency. Its independent result and the actual record owner follow the ordinary call contract. The index must succeed before entry. The profile rejects readonly and different nominal destinations.

An initialized lexical array can supply the selected row directly. This covers `edges[declaration]` in `NameResolution.cod` when preparing the writable loan for `bind_synonym_target`. The selected array determines the row's nominal type and actual owner. A failed bounds check prevents the call and preserves effects from earlier arguments.

At the indexed `EnumSelection` transfer, both source commands accept copying an already initialized local finite-bits product into the matching local array element. The binding keeps the exact nominal types and the selected builtin operation. The destination must be writable and its index must succeed before transfer. The source can be readonly, and the destination keeps its copied payload after a later source write. This profile covers a discarded transfer result.

In an ordinary loop, both source commands check a finite-bits product or sum alternative made by init directly on the right of a builtin indexed value transfer to a writable array formal. This covers the `SchemaDeclaration` and `FieldInput` writes in `AggregateSchemas.cod`. The checker matches the exact nominal element and checks all required fields. The destination index is checked before the initializers, which run in written order. Only a completed value replaces the selected row, and earlier rows remain written if a later iteration fails.

For a fresh lexical array of a local finite-bits sum, both source commands check a fill made by init with a fieldless alternative of the exact element type. Length and fill are prepared in written order. The lexical target can supply an array loan or an indexed constructor case. A case on a Boolean or nominal enum field of an indexed product uses the field's checked type and requires operand checking of the actual array access.

### Imported AST inspection

Both source commands check the call to `ParserStorage.inspect_binding` in unchanged `CallableSyntax`. The defining visit completes the bound case operations and ordinary helper dependencies before publishing the inspector. Its signature keeps the readonly nominal AST array, three independent `UInt64` operands, the initialized writable record and the exact nominal `BindingSyntax` result. The call supplies separate possible bounds and fatal arithmetic outcomes. Cyclic or unsupported dependencies leave the contract unavailable.

Both commands also check `inspect_member_selector` in unchanged `ModuleExports` and through a renamed import. Its last argument is an initialized writable `UInt64` loan with its own owner role. The caller receives the nominal `MemberSelectionSyntax`. An invalid selection can leave the loan untouched. A named selection stores its tail, and an all-members selection stores the sentinel. The declared signature alone cannot publish body completion. The call retains the existing operand preparation and possible failure order.

A direct source-preserving selection can use an initialized product formal or a checked reference to a fresh record in the calling lexical scope. The call prepares its operands in order, respects the selected target's access, and can update the caller's live record. The result carries no initialization promise. Linked seed coverage observes the exact fields for typed, bare and recovered bindings and preserves the record on an invalid index.

The two source commands also check `inspect_local_binding`, `inspect_enum_integer`, `find_signature_clause` and `first_sequence_entry`. Each completed signature gives the readonly AST loan, ordered by-value inputs and the final initialized writable loan. The signature and sequence searches require the selector's exact enum type. The Boolean result is independent of the initialized output. A seed-linked probe checks the output fields on success, unchanged output on a rejected local binding and the sentinel written on a missing signature clause.

### Readonly AST queries

Both source commands check the uses of `ParserStorage.is_here_formal` and `formal_variadic_at` in unchanged `CallableSyntax`. The defining visit completes the case operations and the dependencies that validate the formal's revision, binding, and ellipsis extent. The published signature identifies the readonly `UInt64`-indexed finite-bits sum array, the independent index operands, and the exact result type. That result can be a Boolean, integer, or nominal enum in this source profile. A local scratch record used by a helper keeps its own lexical owner.

A direct source-preserving selection prepares the array loan and operands in order. An independent `UInt8` or `UInt32` can widen to a `UInt64` formal regardless of the query's return type. The completion contract keeps bounds and fatal arithmetic separate. The linked parser witness covers multiple `here` formals, a final variadic formal, a misplaced ellipsis, stale revisions, and indices beyond the permitted extent.

The two source commands also check `ParserStorage.published_syntax_id` calls with the readonly AST array, two independent `UInt64` operands and a readonly `NodeId` under its own owner role. A case reception or its checked inline record field can supply the final argument while the receiving arm is active. Renaming the imported callable preserves its signature and completed failure information.

Both source commands check `ParserStorage.named_operator_at` and `Lexer.operator_past` in unchanged `NameResolution`. The first takes one index into a readonly finite-bits sum array. The second takes two byte offsets into a readonly `UInt64`-indexed byte array. Each call prepares its ordered operands under the actual array owner and uses the completed body contract. Seed-linked witnesses cover offsets from different syntax alternatives, the maximum `UInt64` missing-offset result, and empty or clipped byte spans. These query additions cover the booter's source commands.

The same completed source contract covers `find_selected_name`, `checked_lexical_selection_kind`, `ParserStorage.is_function` and `ModuleExportStorage.checked_aggregate_storage`. Queries can mix independent supported scalar or nominal enum operands with readonly `UInt64`-indexed arrays of bytes or finite-bits products and sums. The result can be a Boolean, `UInt8`, `UInt64` or an exact nominal enum. Each array has a declared owner role in formal order. Multiple roles may receive the same actual owner. The defining visit completes the body and its reached dependencies before publishing the signature. Linked seed witnesses check revision and visibility filtering and distinguish valid record and sum schemas from malformed rows.

The booter also accepts a builtin `UInt8` value transfer into a writable `UInt64` destination. The source commands accept the corresponding transfer of a supported `UInt32` value. Both preserve the input's own type and the target's access. A linked object check stores 255 on normal completion and leaves the destination unchanged when the input raises.

### Imported parser name and separator calls

Both source commands check the unchanged resource-name and identifier calls from `CallableSyntax`. The same completed parser contract covers module-name recognition and block-separator recovery. The four ordered loans keep their exact nominal types and actual owners. An independent `UInt64` follows the loans. The signature may end there or declare further independent scalars, each with its declared type and position, including the `UInt8` diagnostic and the Boolean recovery control. Whole-type unsigned widening applies to by-value operands. Operand failure prevents parser entry.

The linked parser verifies the exact spans of `++` and `<-`, the requested identifier diagnostic, the syntax-storage limit, and separator recovery that preserves an existing report.

### Imported syntax-row assembly

Both source commands check `append_qualified_type`, `append_block_item`, `append_block`, `append_module_item`, and `append_module` through the completed writer contract. The void calls prepare an initialized writable array of finite-bits sum rows, an initialized writable record, and each independent scalar operand in written order. The two loans can share an actual owner. The defining visit checks each constructor and the conditional selection of a lexical sum target before publishing its completion facts. Storage exhaustion leaves the previous syntax prefix intact.

The readonly query contract also checks `punct_pair_at`. It carries the two nominal array loans, the index and extent, and the two by-value bytes, including `ASCII` members with the exact backing type. The original short-circuit comparisons distinguish an absent token, a wrong first byte, and a wrong second byte.

### Imported enum references

Both source commands check ordinary owner-qualified enum reference formals and initialized typed reference locals, including renamed type selections. The reference keeps the enum's defining module and declaration identity, its target access, and its actual owner. A by-value use can read the exact backing scalar. A reference loan or write must match the nominal enum, and a readonly target cannot supply a writable loan.

The linked parser recognizes all five augmented transfers and preserves the operator and operand spans. Both source commands accept unchanged `CallableSyntax`.

### Checked byte arithmetic in source bodies

Both source commands check the unchanged decimal-digit arithmetic in `ExpressionParser`. The builtin arithmetic on two `UInt8` operands keeps the `UInt8` result and its checked failure semantics. Mixed `UInt8` and `UInt64` arithmetic uses `UInt64`. An initialized `UInt64` value local can widen a completed `UInt8` or `UInt32` value. That adaptation leaves the arithmetic width and reference-target types intact.

The checked readonly byte-reader contract carries possible fatal arithmetic separately from bounds failure. The linked decimal parser preserves the written width, magnitude and sign and reports an out-of-range literal before publishing a node. Both source routes check the indexed-token arguments in the unchanged calls to `integer_width` and `parse_integer_magnitude`.

### Imported parser recovery

Both source commands check the unchanged calls to `recover_syntax` and `recover_syntax_until`. The completed parser contract carries the readonly token array, the writable syntax array and initialized result record, and the independent value inputs. `recover_syntax` also receives the readonly source bytes. A by-value recovery role keeps the canonical identity of its enum. The call evaluates operands in written order, and a failed operand prevents entry.

The readonly array-query contract covers `closing_token` with the published finite-bits record array and the two independent indices. The linked parser retains separate located reports for malformed block items and conditions, records their expected roles, and keeps the following return.

Both source commands check calls to `parse_scoped_procedure_signature` and `parse_callable_signature` through their completed void parser-writer contracts. The writable `ParseResult` loan uses its defining module's published C-layout schema. The ordinary parameter-list parser also serves nested `here` signatures, with their existing restrictions on nested `here` and variadics.

The readonly predicate contract covers `c_linkage` and `valid_module_path` with a byte array and an exact indexed token loan. Completion uses the checked operations and call dependencies, including the range and early return in path validation. It records possible bounds failure and fatal numeric failure independently. A failure selecting the token precedes invocation. The linked frontend accepts escaped quote and backslash paths and reports invalid escapes at the original token span.

Both source commands accept the unchanged `ExpressionParser` and check the module path call in `ModuleParser`. They also check construction and synchronous writable loans of finite-bits CoD records, including the `FormalSites` record used by `inspect_formal`. The constructor uses its selected nominal schema and checks every initializer. A call uses the initialized target and its actual owner, and the returned classification and record fields keep their exact enum types. Delayed writable loans remain outside this profile.

Both source commands check the unchanged `same_bytes` call comparing written owner names. Each span keeps its nominal byte-array loan, actual owner and two independent bounds. The two roles can borrow the same backing or different backings. The completed body records bounds failure separately from fatal arithmetic. The linked frontend recognizes a matching owner and reports a mismatch while retaining the following declaration.

Both source commands check the original `parse_enum_integer_literal` call. The completed writer contract includes checked `UInt8` decimal arithmetic and carries its possible fatal outcome through the digit helper. The reader preserves the sign, magnitude and written width for the backing-type check. An overflowing literal reports its original span and leaves the following declaration available to recovery.

Both source commands accept the unchanged `ModuleParser` and `Frontend`. The `parse_callable` contract includes the mutually recursive helpers that parse nested callables. From locally checked body facts and frozen call edges, the completion pass propagates unavailability, escaping bounds and fatal arithmetic independently to callers. A failed operand prevents entry. A cycle with completed dependencies supplies a bound on these outcomes without implying termination. The linked parser retains nested callable syntax and diagnoses a malformed declaration while keeping the valid suffix.

Both source commands check named `UInt64` arrays with `UInt64` indices in the same module. A direct reference initializer without a receiving owner uses its lexical scope. An explicit placement uses the named writable owner role. The checker uses the selected nominal array schema to check length and fill in order. After successful construction the array supports indexed reads and writes and synchronous ordinary reference parameters. Bounds failure, allocation failure and attempted writes through readonly loans follow the existing checks. Executable object emission for these arrays remains outside the supported profile.

Both source commands also check a lexically allocated named array with `UInt64` indices and a finite-bits C-layout record element. The braced fill must initialize that exact record. The checker keeps the written length and fill order, and checks indexed scalar field reads and writes against the selected row, bounds and access. The ordering array, heap helpers and diagnostic row preparation pass in a reduction using the imported parser operations.

For a same-revision native function with readonly array loans and no callback or declared failure, both source commands check the ordered operands against the selected signature. The supported operands include handles, by-value scalars, existing formal views and direct lexical arrays of checked C-layout rows. Each array loan keeps its nominal type, prepared backing and actual owner. Repeated roles require the same actual owner. A failed operand prevents the call. The supported calls borrow synchronously.

In a demanded body, both source commands consume the completed `SourceAccess.locate_source_position` contract through a direct import, including a rename. The readonly byte and line arrays share one actual source owner. The independent offset and initialized writable position follow them in written order. The defining visit checks the recursive line search before publishing possible bounds failure and fatal arithmetic separately. A failed operand prevents entry and leaves the position untouched.

In a demanded body, the ordered native call checks also cover a same-revision function with one handle and by-value scalar operands. Existing declaration checks still validate calls outside that body profile. The checker keeps the actual handle owner and access, and applies the existing `UInt8` to `UInt32` widening at a scalar position. Failure preparing an operand prevents native entry.

For a builtin bits transfer to an indexed record field, the selected array must support a writable loan to the exact row, and the field must be writable. Reads retain their existing scalar projection rules. Both source commands check the production symbol-rejection routine in isolation, including both native diagnostic calls and its update to the retained count.

In a demanded body, both source commands check synchronous calls to a same-revision procedure with readonly finite-bits product loans and an independent result or no result. The actual can be an initialized product formal or a checked lexical product reference. The checker prepares each operand in written order and checks its nominal type, access and actual owner. The procedure can also take checked handle loans, readonly arrays and by-value scalars in this profile. A failed operand prevents entry. Both source commands check the original `write_single_report` and its caller in the stored-rejection reduction.

The two source commands also check direct same-revision calls that mix an initialized lexical record, an inline readonly product field or a case-received finite-bits aggregate with writable array and record loans. The field uses its enclosing object's actual owner. A case reception uses its recorded lexical owner and is available in the receiving arm. The callee may write through its writable arguments. Borrowed results and delayed calls are outside this profile. This covers the `BinaryOperands` field and the case-received binding and link operands in unchanged `NameResolution`.

Both source commands accept the unchanged `NativeDiagnostics`, including the position writer in the protected body of `write_ordered_reports`.

### Imported module-index path loans

Both source commands accept the unchanged `ModuleIndex` and direct imported calls to `lookup_path` and `fill_path`, including renamed selections. The checked interface preserves the ordered handle, nominal array and record loans, by-value scalars, declared `PathNotReadyException`, and the independent void or nominal enum result. This profile covers immediate calls in an ordinary activation. Object emission and allocator transport have separate admission checks.

Each borrower retains its actual owner and target access. The two readonly lookup arrays must share the actual owner for their common role. The result record and path handle have their own roles. A failed argument prevents entry. The linked seed witness distinguishes a normal fill, its `ScratchFailed` feedback, an unready path, and a failure during argument preparation.

### Synchronous source-revision loans

Both source commands check the defining body of `NativeSourceUse.with_source` and publish its ordinary callback signature. The relay forwards readonly source bytes and line rows in their declared positions under the same actual revision owner. A renamed direct import binds the same two offers. The caller checks the supplied body and its nominal `CompilerStatus` feedback. The linked seed witness verifies the extents, distinct contents, ordinary feedback and an abrupt callback with its original payload.

### Mixed native diagnostic arguments

Both source commands check immediate calls to the imported C function `append_package_diagnostic`, including renamed selections. The ordered native argument checker consumes the published signature. It checks the writable diagnostic handle, exact diagnostic enum, independent scalar values, and four readonly array loans under their actual owners. A repeated role must use the same actual owner. The checker rejects deferred use of this loan profile. On successful preparation and native completion the call returns an independent `UInt8`. [Native operand preparation](docs/ModuleGraph.md#checking-handles-and-readonly-array-loans-in-one-native-call)

### Readonly record-array predicates

Both source commands check an immediate ordinary Boolean query over one readonly nominal array of finite-bits records. The defining visit checks the body and publishes its ordered signature with completed bounds and arithmetic evidence. A direct import, including a rename, keeps the same array identity and actual owner. The caller prepares the loan before invoking the query and routes any recorded bounds failure through its own completion context.

This covers the unchanged `diagnostic_budget_full` call in `PackageDiagnostics`. Object emission requires its own admission.

### Ordinary diagnostic calls with mixed loans

Both source commands check the unchanged `PackageDiagnostics` module. It imports `report_rejection`, which writes through the diagnostic handle and borrows the source bytes and line rows under one readonly owner role. The defining visit completes the ordered signature and the body's reachable bounds and numeric outcomes before publication. The caller matches each loan to its nominal type and binds a repeated owner role consistently, including when a role is shared by an array and a handle. An abrupt operand prevents entry.

Both commands also check a direct import of `report_package_path`. The defining visit checks the synchronous source callback and its local helper, including the selected native append and the nominal `CompilerStatus` result. A `const val` parameter remains immutable in the body and publishes an ordinary by-value input with its nominal type. The imported procedure profile carries bounds and numeric outcomes from completed body evidence. A body with an unrepresented effect supplies no such contract. Package graph self-check still reaches the mixed-slot interface at `load_for_edge`.

### Package-root loader source checking

Both source commands check the unchanged `LoadPackageInputs` definition. Lexical byte buffers use the current full lexical owner. The staged product and byte arrays use the loader's writable owner role. The checker keeps the nominal array and element identities separate, checks the fill against the element declaration, and keeps the two caller slots distinct. The native directory calls use the same ordered matcher for readonly arrays, borrowed handles and independent values.

The defining visit publishes checked failure bounds and per-exit slot effects for a direct import of the loader. The source commands check the unchanged `PackageGraphPaths` caller, including its imported `PackageResolution` result. The full `ModuleGraph` and CLI entries still have independent dependency-checking limits.

### Imported array length families

In demanded source bodies, a direct `using` selection can call a fixed ordinary family whose methods each return the builtin length of their sole readonly array formal as an independent `UInt64`. The defining module publishes all methods with their exact nominal array types and completed body evidence together. A rename preserves that selection. The caller prepares the live array and its actual owner once, then selects the unique method for that array. A writable actual can supply the readonly formal. Argument failure precedes entry. Extended views, deferred calls and object emission remain outside this profile.

The offset helpers have a separate source-call contract. The defining visit checks the indexed case and nested field projections in the shared readonly helper. Only a direct return that forwards the same array and the two independent `UInt64` arguments in order, with one Boolean literal, can use that completed evidence. The published signature retains nominal array identity and the readonly owner role. After successful preparation, the call supplies an independent `UInt64` on normal return and carries any established unchecked bounds outcome. The maximum `UInt64` sentinel is a normal result.

### Package probe and literal call contracts

The declaration and C provider commands check the imported native package probe with two readonly byte-array loans and four independent 64-bit unsigned bounds. Each loan preserves the array's nominal identity and actual owner. The probe returns the full-width value, including the maximum code for an absent module. The unchanged resolver reaches its first indexed product field after the probe signature is checked. [Native package probe](docs/ModuleGraph.md#checking-the-native-package-probe)

Demanded source bodies can call a directly selected ordinary function proved to return one scalar literal. The defining module publishes this proof for a complete empty signature and an exact literal body, or for a single direct return of an already established local literal helper with the same result type. The wrapper proof uses the bound call and completed leaf regardless of declaration order. Source selection through a rename preserves that contract. [Imported literal calls](docs/ModuleGraph.md#checking-an-imported-function-that-returns-a-scalar-literal)

### Fixed scalar value bindings

An initialized `const val` local evaluates its initializer once and holds an independent readonly scalar on normal completion. Both source commands check the initializer and enforce readonly access even in a private unused body. A readonly loan is allowed, and copying the value into a writable local gives that local independent storage. The source commands also check a fixed scalar initialized from a field of a supported indexed product formal, including the array owner and bounds at the read. The object command emits fixed locals with supported scalar initializers.

Ordinary definitions can use `const val n:T` for a supported scalar `T`. The parameter receives its own copy at the written argument position. Its storage allows reads and readonly loans. The source commands check that restriction even in a private unused definition, and the object command emits supported module calls. Fixed reference formals and fixed formals in C and `here` signatures remain outside this profile. [Parameter checking](docs/ModuleGraph.md#parsing-parameters-for-graph-table-growth)

For a local nominal `UInt64`-indexed `bool` array, the source commands check lexical allocation and fill, indexed reads, and writes through a live writable target. The destination and its bounds check precede the value operand. This covers tag writes in `InitializationFlow`. Reference-slot formals and object emission for these arrays remain outside this profile.

### Predicate assertions

The booter’s three commands check `assert predicate;` in an ordinary body. The predicate must supply a `bool`. A literal false predicate is rejected, including under parentheses. A literal true predicate needs no assertion branch. Otherwise the compiled program evaluates the predicate once and raises unchecked `AssertionFailedError` on false. Failures during evaluation retain their category and follow ordinary cleanup. The source commands check the body containing the assertion.

Quoted assertion messages and reference-identity predicates such as `p is not null` still receive located profile diagnostics. [Predicate assertion contract](../../design/SymbolicDynamicScope.md#predicate-assertions-and-flow-refinement)

### Fixed-width scalars and transparent type synonyms

The seed and booter recognize `Int64`, `UInt8`, `UInt32`, `UInt64`, `Float32` and `Float64` as builtin type names. Compact literal suffixes still work, as in `255u8`, `1u64` and `1.5f32` in the profiles that support those literals. Numbered names such as `i64` are ordinary identifiers. Select `Int` and `UInt` with `using "julia.cod" { Int, UInt };` for the bundled 64-bit bootstrap targets. Short type names require a declared synonym in scope.

A first-order synonym such as `export type Sample = Float64` resolves its target in the defining module before signatures or layouts use it. Chains can refer forward to local declarations and through selected exports. A chain containing only synonyms and no defining type is rejected.

A synonym carries the target’s identity into overload selection, enum backing declarations and C provider signatures. Two synonyms of the same target give identical signatures. Signed and unsigned targets remain distinct even when they have the same storage width. The booter object command checks duplicate signatures for the supported ordinary fixed value families. Each command retains its existing profiles for the underlying type and module graph. [Scalar naming and synonym contract](../../design/JuliaPackagePortability.md#scalar-names-and-transparent-type-synonyms)

### Contextual names in both frontends

Apply the [name rules](../../design/SymbolicDynamicScope.md#clause-words-and-lexical-variable-names) during dependency scanning, parsing and lookup. Cover lexical binders and their uses, `from` and `throws` in clauses, module declarations and ordinary user calls with names such as `fold` and `store`. A lexer accepting the spelling is insufficient if a later phase still diverts it to an intrinsic or rejects the binding. References to unsupported features retain their stage-specific diagnostics.

## Build and run on Linux

The standalone build uses GNU Make 4.3 or newer, a prepared CoD seed and its language library, and a compatible LLVM SDK with shared libraries. A prepared prefix has `bin/cod-seed` and the sources `string.cod` and `ASCII.cod` from [the language library](../../cod/library) in `lib/cod`. Stage the same library beside a distributed `bin/cod-booter` so it can resolve logical imports from any working directory. Include `julia.cod` for explicit selection of `Int` and `UInt`. Use the following commands from the repository root with your ready seed prefix and the existing LLVM SDK in `build`.

```sh
make -C cod/runtime
make -C cod/native/llvm
make -C applications/cod-booter COD_SEED=/path/to/ready-seed/bin/cod-seed
./build-cod-booter/cod-booter applications/cod-booter/workloads/deferred-release.cod \
    build-cod-booter/deferred-release.o
./build/bin/clang build-cod-booter/deferred-release.o -o build-cod-booter/deferred-release
./build-cod-booter/deferred-release
```

The program prints `1` on three lines. Use `TOOLCHAIN` for a ready toolchain prefix. The runtime and LLVM service builds accept `CLANG` and `LLVM_AR`, and the LLVM service also accepts `LLVM_CONFIG`. The booter accepts `COD_SEED`, `CLANG`, `LLVM_CONFIG`, `COD_RUNTIME_ARCHIVE` and `COD_LLVM_ARCHIVE`. Use a separate `OUT` for each build you run concurrently or want to retain. Make interprets relative paths from the selected build directory. Use paths without whitespace.

The scanner and parser use [the bundled ASCII vocabulary](../../docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner) for character codes. The booter's declaration and C provider commands resolve nested enum default selections, including the five free ASCII members. The generated C provider header exposes the complete 128-member table. All three commands resolve written enum members through the selected type and check literal codes before narrowing. For example, `10:ASCII` resolves to LF and `266:ASCII` reports a missing member at the assertion. Linked object coverage also checks a runtime byte against the declared members, with `255u8` taking `TypeAssertionFailedError`. The object checker supports exact enum backing values in scalar returns, assignments, equality and ordinary value arguments. An ordinary local callable can receive a by-value formal whose enum is defined in an imported module. For same-revision ordinary fixed overloads with positional scalar values, an exact enum match takes priority over conversion to its backing type at that argument position. The three commands also support importing a single ordinary method with by-value enum parameters or an independent enum result. The interface preserves the enum’s defining module and identity through renames. A by-value local with an imported enum annotation uses that same identity, including when the annotation names a synonym. The two source commands check such locals in bodies they already demand. Explicitly typed imported enum borrows, borrowed enum parameters and imported overload selection remain outside this profile.

In a demanded body, the declaration and C provider commands also check braced scalar construction of a directly selected imported enum, including a renamed type. The initializer must supply a value of the exact nominal enum. A bare integer or a distinct enum with the same backing type is rejected.

All three commands also decode signed integer literals such as `(-17):Delivery`. The selected enum can be defined locally or imported. A signed backing admits its full 64-bit range, including the minimum. The checker compares the mathematical code before encoding it, so an out-of-range positive value cannot select the minimum and a negative value cannot wrap to an unsigned member. General signed arithmetic remains outside this profile.

The seed processes the booter sources and emits the provider header, C stubs and LLVM IR into `OUT`. The grouped build tracks the library selected by the resolved seed executable. A seed in its CoD CMake development tree can use that checkout's `cod/library` when the installed library directory is absent. When an active project can supply a package override, the standalone build rechecks the source graph before reuse. Clang compiles the generated files and handwritten C adapters, then links the CoD archives and the shared libraries reported by `llvm-config`. The SDK library directory is recorded for the loader. The prepared inputs are read-only. The bootstrap input contract uses the [hand-prepared project directory](../../design/CodProjectStructureAndPkgMgmt.md#directory-layout).

The current bootstrap CLIs use explicit operations. The current booter takes `cod-booter SOURCE OUTPUT` to emit an object, or an explicit declarations or provider request. An installed toolchain can provide `lib/libcod-runtime.a` under its prefix. Set the `COD_RUNTIME_ARCHIVE` environment variable to select a different compatible archive. Link seed-emitted objects or bitcode with the same archive when building an executable outside the seed.

## Next-stage repositories and native runtime

The compiler and toolchain repositories are planned in the [e-wrks GitHub organization](https://github.com/e-wrks).

| Planned repository | Responsibility |
| --- | --- |
| `cod-lang` | CoD language design and the authoritative language specification shared by both compilers. |
| `cod-seed` | The C++ translator for trusted booter source, the CoD-written checking compiler and their C-native components, including the region core and LLVM bindings. The booter stays in this repository after the split. |
| `cod` | The future production compiler written in CoD. The first release targets fast compile-and-run and introduces the experimental `cfe` optimizing CLI. Version 2.0 formally supports `cfe`. Its [output-directory contract](docs/CompilerPipeline.md#default-execution-and-optimized-builds) governs the generated executable. The compiler also provides full diagnostics, [independent check and emission controls for editors and builds](docs/CompilerPipeline.md#independently-selectable-compiler-operations), and a future CoD REPL. [REPL decision](../../CHANGELOG.md#h-retire-cpp-repl-20261125) |
| `codup` | Installs matching CoD and LLVM tools, including `clang`, `clang++`, target assembly support and the link tools and target components used by host `cod` and cross-target `cfe`. [First native package toolset](docs/CompilerPipeline.md#codup-toolchain-for-native-packages) |
| `codmate` | The Language Server Protocol server and IDE plugins, including [reference-site doclet tooltips and navigation](../../design/DomainLanguageDesign.md#definition-doclets). |

The `e-wrks/cod` toolchain will bundle `pcl.cod`, the parser combinator library that implements the parsing HLVM. The production CoD parser is one of its applications. Its DBMR grammar program ships as `cod.grammar` and runs on `pcl.cod` on the compiler’s normal source path. [Installed-toolchain workload](../../design/ParserCombinatorLibSupport.md#packaged-production-parser) · [CoD parser application case study](../../papers/AlgebraicEffectsIsAllYouNeed.md#the-cod-parser-as-an-application-of-the-parsing-hlvm)

Future package management belongs in `e-wrks/cod`. The bootstrap consumes [manually staged source roots](../../design/CodProjectStructureAndPkgMgmt.md#directory-layout).

After the split, a change to what a CoD program means belongs in `cod-lang`. Each compiler repository documents how it implements the rule, its current coverage and any known gaps.

New shared native runtime components are written in C. Package authors may implement the [generated C interface](docs/LLVMInterop.md#native-package-providers-for-nominal-regions) in C or C++. The C++ seed, the booter, the future production compiler and foreign adapters consume the same interface. Specify each operation from the runtime behavior its callers need. Package providers keep C++ classes, templates and exception handling inside their C++ components. This choice covers CoD’s LLVM bindings as well as region support.

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

At the checked-seed checkpoint, in both modes the C++ seed loader continues to independent siblings after a recoverable source or import error. It follows intact imports in a damaged file. If two parents reach the same syntax fault, the seed reports it once with both import sites. The active seed accepts `--max-diagnostics N` on its explicit parse, declaration and translation routes. A package-shadow warning can consume the last slot before another dependency opens. In that case the command reports incomplete coverage and preserves existing outputs. [Seed loader](../../cod/lang_modules.hh) · [Command driver](../../cod/main.cc)

The checked seed also validates independent direct product declarations in a fully parsed module with valid imports even after another reachable file has syntax errors. A failed field leaves its product layout unavailable to inline dependents. The seed still diagnoses an independent layout cycle. [Product schema recovery scope](../../design/ImplementationArchitecture.md#recovering-between-product-declarations-in-the-seed)

Final validation in the checked seed collects concrete uncovered checked exceptions escaping independent named procedure bodies after catches and cleanup. This recovery uses an established, acyclic graph of direct body prerequisites. A rejected body blocks the checks that consume its proof, including module entry when applicable. Earlier type errors in effect solving and unproved dependency profiles retain their existing checks. [Final body validation](../../design/ImplementationArchitecture.md#reporting-uncovered-checked-exceptions-from-independent-bodies)

The released CoD booter continues through independent dependencies and applies `--max-diagnostics N` across the invocation. A child error can block every member of a grouped selector. One such written directive contributes one import note. Reaching the cap exactly returns ordinary rejection when all remaining members depend on that failed child. A separate pending check makes coverage incomplete. [Booter visit](src/ModuleGraph.cod) · [Grouped path review](../../design/ImplementationArchitecture.md#a-failed-path-in-a-grouped-import)

For acceptance, use a valid entry importing two broken siblings, a transitive dependency, and an unrelated broken file in the same directory. Give each broken sibling a syntax error and reach a third syntax error through an intact import in the first sibling. In another reachable module, give two independent products invalid field types. The linked CoD booter reports all five findings. Both build modes of the checked seed also demonstrated this verifier workload before the trusted-source cutover. With `--max-diagnostics 4`, the booter reports the three syntax faults and the first bad field, then reports incomplete coverage. Use a diamond to check repeated discovery. Through the normal compiler commands compare uncapped reporting with a cap of one and a cap above the number of findings. Include a real warning followed by an independent error to exercise the shared budget, its reset on the next invocation and artifact preservation on early exit.

#### Whole-file parser recovery

Every project parser processes the complete input it is asked to parse by default. An explicitly capped compiler invocation can cancel remaining work and records the resulting coverage. For a CoD file this includes lexical scanning, delimiter matching and parsing through EOF, in the C++ seed, the CoD booter and future compiler and editor frontends. A syntax error yields a located diagnostic and a recovered tree. Keep parsing later items in the same list, including independently malformed siblings inside a procedure. Entry points that parse a bounded subproduction give control back to their enclosing parser at the corresponding boundary.

The AST admits an explicit error-report node wherever syntax was expected, including expressions, types, names, patterns, statements and declarations. Record the expected role, the diagnostic or its stable reference, and a half-open span in the original revision. A missing operand may have a zero-width span immediately before its separator. If one fault leaves several slots incomplete, those placeholders may refer to the same report. A missing optional child retains its ordinary representation. Keep correctly parsed children and siblings accessible. Ordinary consumers must distinguish the error alternative before projecting a concrete role. Tools can traverse the valid parts. A file with any syntax error fails compilation, and its recovered forms cannot publish definitions, generate artifacts or replace previous output.

Recover at a boundary justified by the current grammar and delimiter nesting. Each recovery consumes bad input, returns a synchronizing token to an enclosing production that will consume it, or ends at EOF. This also applies to lexical errors and delimiter prepasses. A new line between tokens is ordinary whitespace. A malformed token gets one primary report for the same fault; an independent later error gets its own report. An unterminated triple-quoted token may legitimately cover the remaining bytes through EOF. Preserve that span and any earlier reports. An exhausted physical resource can end the parse with an explicit incomplete outcome.

In a normal block, a complete ordinary `if` with a braced consequent can be followed directly by the next item. This boundary works with spaces or line breaks. An unbraced `return` arm and an enclosing expression such as `:void` follow the ordinary separator rules.

```cod
proc sample() -> UInt64 {
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
