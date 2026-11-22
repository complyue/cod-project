# Module graph, workspace and dependency callbacks

## Design authority

[D-20-MODULE-GRAPH](../../../design/DesignAuthorityRegistry.md#d-20-module-graph), revision 6. The project user set the compiler/native boundary and delegated pre-code architecture review to the design discussion. [H-COMPILER-LAYERS-20261122](../../../CHANGELOG.md#h-compiler-layers-20261122) records the instruction and the reviewer's scoped clearance. P-680 derives the design from C-80. Inspectable compiler decisions and lifetimes make a real module compilation a test of the compiler language. P-681 identifies the applicable sum, regional storage, module selection, dynamic installation and native loan contracts. Coverage includes the first booter's declaration, C-provider and object routes. The [acceptance workload](#acceptance) checks their behavior and physical separation. Permanent ABI, persistent caches and changes to language semantics require their own decisions.

[K-BOOTER-AST-TABLE-20261122](../../../CHANGELOG.md#k-booter-ast-table-20261122) continues that architecture for the [first indexed AST path](#first-indexed-ast-path). Its private representation uses the existing named-array constructor, valid bits transfer and constructor-pattern checks. Acceptance includes the parser, checker, lowering and emitted object.

The [frame and node identity clearance](../../../CHANGELOG.md#k-booter-ast-membership-20261122) covers revision issuance, table lookup and nested invocation isolation.

The [typed syntax consumer clearance](../../../CHANGELOG.md#k-booter-syntax-consumers-20261122) covers the existing parser and the passes that consume its nodes, including their shared declaration checks.

The [single-file object route clearance](../../../CHANGELOG.md#k-booter-typed-object-20261122) covers the isolated CompilerCLI activation, its lexical workspace, the existing generated providers and the linked workload.

The [dependency-edge review](../../../CHANGELOG.md#k-booter-graph-edge-prerequisites-20261122) establishes the path loan and regional storage requirements for the next implementation proposal.

## Layer responsibilities

| Responsibility | Implementation |
| --- | --- |
| Compiler state and policy | The seed implements its compiler objects and algorithms in C++. The booter implements its module frame, typed workspace, canonical-key comparison and interning, dependency table, traversal, checking, diagnostic selection and publication in CoD. CoD decodes import spellings and decides which exports, interfaces, native units and generated provider content to produce. |
| Native operations | C performs OS path resolution, lends bounded canonical path bytes and mapped source with its line index, supplies aligned raw region storage, and exposes the LLVM and file operations the compiler requests. The launcher enters CoD. A synchronous native loan or call to LLVM has its own checked owner, bounds and completion contract. |

The migration removes compiler-owned decisions from the current [C graph provider](../native/module_graph_provider.c) and [typed scratch provider](../native/scratch_provider.c). In particular, selecting the root, recursing into a child, marking a module checked, interpreting source escapes and assigning a diagnostic to a requesting edge are compiler operations. The same division applies to declaration checking, provider generation and object construction.

## Owners across a visit

| Lifetime | Contents and use |
| --- | --- |
| Compilation graph G | Canonical keys, stable invocation-local module IDs, visit states, edges, and published spellings, interfaces, command and literal ranges. A checked entry becomes visible only after its required fields and ranges are initialized and the module succeeds. Retain the graph through final output. |
| Module visit F | A fresh CoD frame and typed workspace for each active visit. CoD computes checked counts and placement for tokens, AST nodes, dependency candidates, bindings, facts and commands. Raw storage gains its element type and readiness through checked construction. |
| Indexed source revision S | One successful owning acquisition per opened revision. Reacquire immutable source bytes and the matching line index from that same S for successive local stages. End each native loan before visiting a child. |
| Diagnostic destination | The launcher supplies separately owned storage. CoD resolves a span against its own revision while live. A dependency failure reported to the parent uses its requesting import span. An open failure without a source position retains its OS error. |

A suspended parent may keep its F and S. Across a child call it retains IDs and spans for graph data and reacquires views before use. An ID requires the graph it belongs to. Publish the required bytes or typed rows in G before their source owner ends. Growth must preserve actual-owner evidence. In particular, [consuming region resize](../../../design/RegionalAndLexicalStorageRefactor.md) requires accounting for the owner and all live dependent uses, including handler and cleanup bindings. A stable module ID alone cannot keep a borrow valid across replacement. The design permits storage strategies that avoid replacing a borrowed region.

Use named sum alternatives for states and syntax nodes with differing payloads. For example, a literal has its value, a call has its callee and argument indices, and a return has its applicable return payload. Keep the common half-open source span, the applicable operator anchor, and the association with the owning syntax revision. A consumer matches the constructor before accessing the variant fields. Child indices retain their meaning when the backing relocates. Use enums for simple closed codes. Project initialized native command rows at the LLVM boundary; the CoD AST layout is private to the compiler.

## First indexed AST path

The first workload parses a module containing a named procedure that returns an integer literal and a call that consumes its result. [ExpressionParser](../src/ExpressionParser.cod) constructs literal, call and return alternatives, together with the module, callable, block and argument structure the input needs. [ParserInput](../src/ParserInput.cod) appends complete nodes. [OperandChecking](../src/OperandChecking.cod) matches the call and resolves its callee; the parser records the callee expression's node ID. [ModuleLowering](../src/ModuleLowering.cod) consumes the checked nodes and emits the corresponding constant, call and return. Node IDs identify entries in one table. That table's frame associates its spans with the particular source revision used to build it, including when a source loan ends between stages.

Use a table-private slot type to keep capacity separate from valid syntax. In this source fragment, `ExprNode` has a half-open span and a typed sum payload. F is the writable workspace owner; `capacity` and `completed` are already established.

```cod
data AstSlot = Vacant {} | Occupied { node:ExprNode; };
array AstSlots[u64]:AstSlot;

ref slots = make AstSlots(capacity, make Vacant {}) in F;
# At the next unpublished index, after checking index == used and index < len(slots):
slots[index] <- make Occupied { node <- completed; };
```

The length and the complete fill argument are each evaluated once in order, including for length zero. Each slot initially holds an independent Vacant value. The parser constructs the complete next row before publishing the new root and used count. Readers check membership of the ID in the published prefix and project fields only for the matched constructor. A physically in-bounds vacant slot cannot be consumed as a syntax node. An ordinary typed reservation can stage a chosen constructor at a known target; it supplies neither the array length nor a completed fill. [Named-array placement](../../../design/ConstructionAndAcquisition.md#source-migration) · [Reservation](../../../design/ConstructionAndAcquisition.md#typed-reservation-and-completed-construction)

This application qualifies the concrete finite layout. All node and slot alternatives contain only direct, recursively bits fields, tags and indices. Transfer preserves the selected constructor and its initialized fields. Inactive fields and padding supply no payload. A native implementation may specialize fill and transfer under the same observable evaluation and failure rules. Adding a stored reference or a resource field requires revisiting this representation. [Bits classification](../../../design/ConstructionAndAcquisition.md#bits-classification-and-representation-obligations)

For growth, create a filled replacement array under the live workspace owner, transfer the occupied prefix at the same indices, and change the table binding only after success. Existing references still refer to their original storage. Consumers carry IDs across the switch and obtain current views through the table. In this path published AST rows are append-only, so no consumer retains a projected field across a change of active alternative. On capacity or construction failure, the preceding published range and root remain usable. Allocation and cleanup keep their ordinary effects. Consuming a region with `resize` has a different ownership and failure contract.

Verify an emitted object returns or reports the expected literal through the named call in both seed modes. Also verify zero and multiple slots, a write leaving its neighbor vacant, independent widths and aligned payloads, rejection of a non-bits element on this transfer path, wrong-table or unpublished IDs, a mismatched arm, stable IDs and spans after growth, and failure before replacement. Keep an isolated implementation until the existing production syntax coverage can switch to the typed table. Delivery uses the actual CoD compiler path and retains its provider and object workloads.

### Frame and node identity

When a visit opens its owning source revision S, the CoD compilation graph G issues a fresh invocation-local revision key. Frame F associates that key with S, one logical AST table and its published count. The stored bits type is `data NodeId { revision:u64; index:u64; }`. A constructor with an optional child represents absence separately, so row zero is usable. Growth preserves this table’s key and indices. Independently rebuilding a table requires a fresh key. A new open gets a fresh key even for the same path, bytes or backing address. Keys issued in G stay spent through failed visits and retries. Check for exhaustion before reuse or wrap and follow the existing failure and cleanup path before publishing a frame.

The parser issues IDs for published rows. Given the originating compilation and a live frame, lookup checks the ID's revision against F, its index against the published count, and the slot's `Occupied` constructor before projecting its node. For a source span the visit borrows bytes and the line index together from F's actual S. Resolve the span within that loan. A parent can reopen a loan on its own S after child reentry.

Two compilations may both issue `(revision = 1, index = 0)`. Their IDs are meaningful only inside their originating G. Keep IDs private to that compilation's parser, checker and lowering data flow. A nested compilation starts with its own inputs and returns its ordinary output or status. Its caller resumes with the original frame. Any interface that could mix IDs from independent invocations must establish their origin separately before lookup. A numeric match supplies neither that evidence nor access to storage. Native interfaces receive their existing source loans or completed outputs. A process-wide key allocator is unnecessary.

Exercise two real frames in one compilation whose occupied rows have the same index, and reject each foreign ID before projection. Reopen the same path and check that the old ID fails. In nested independent compilations, exercise colliding numeric pairs and verify that each call resolves its own source span and restores the outer frame. Keep the valid-ID growth and failed-publication checks in the parser-to-object workload.

### Typed syntax consumers

Migrate each existing syntax family from its parser through all passes that interpret it. Begin with module and block lists, named callable headers and bodies, arguments and builtin selection. Continue with the other grammar already handled by those modules. The shared compiler must check its transitive definitions even when the compiled program exercises just a named call and a literal.

| CoD modules | Responsibility in the migration |
| --- | --- |
| ParserStorage, ParserInput, CallableSyntax, ModuleParser, ExpressionParser | Construct complete alternatives with named operand roles, optional children, half-open spans and the applicable operator anchors. Represent an operator choice with a closed enum when the operators share an operand shape. Shared sequence operations preserve the existing source order. |
| NameResolution, DeclarationTypes, NativeDeclarations, ModuleExports | Match the syntax for names, signatures, selections and native declarations. These passes own definition identity, result types, actual-owner relationships and publication checks. Published interfaces keep their established defining-module and declaration identities. |
| ModuleFrontend, OperandChecking, CompletionChecking | Retain profile selection, operand, owner, access and completion checks and their existing diagnostic locations. Shared syntax helpers can return a specific syntactic fact such as a callable header or a selected name. |
| Compiler, Lowering, ModuleLowering, ModuleDriver, ModuleGraph | Read the alternatives and source anchors needed by their existing work. This clearance includes AST reads for graph selection and diagnostics. Graph traversal, native providers and production cutover retain their separate scopes. |

For example, the parser records a trailing `here` call, the receiver specification and its body. [NativeDeclarations](../src/NativeDeclarations.cod) matches the call and body, uses the selected call's checked result identity and records the active callback for a full-scope body. Operand and completion checks still apply the respective scope's return rules. Keep these decisions in the passes that own them. A generic accessor returning the old kind or numbered payload columns would lose the distinction.

A stored child link uses the frame membership check before lookup. After that check a pass may use the derived index in its own same-frame work table. Row zero, an absent optional child, an unpublished or vacant row, and a valid node of another constructor have distinct meanings. A failed lookup cannot supply a default child or a partially published declaration. Constructor-specific forms such as a bare return may represent absence in the constructor itself.

Verify each family on its existing accepted and rejected sources, including ordinary and native `here` calls, owner or receiver mismatches, completion and diagnostic spans. Check the complete isolated CompilerCLI declarations and run the named-call object in both seed modes. Before production cutover, compare generated providers, cross-module output and failure/cleanup behavior with the established workloads. Keep a single typed AST along each migrated path.

### Single-file object route

The isolated build enters through the generated [CompilerCLI](../src/CompilerCLI.cod) entry with the launcher's actual input path, output name and diagnostic destination. The ordinary catalog loan supplies the builtin spellings and exports. The CoD activation owns G. After a successful source open it issues a revision key, binds F to S and invokes the compiler under `source_use`. Use the existing workspace plan and CoD lexical named arrays for tokens, AST slots, analysis tables and commands. Keep the revision, published lengths and phase results in CoD. Fill each array with a valid value and check the published range before consuming any row. A default fill establishes storage readiness only.

Use the existing module parser, selection, name resolution, operand and completion checking, and command lowering on the typed nodes. A standard catalog selection can supply the workload's output operation. A request for a dependent source file requires graph traversal and must stop at the existing unsupported or rejection path if traversal is unavailable. Preserve the production route during this stage.

After successful CoD checking and lowering, [ObjectCompiler](../src/ObjectCompiler.cod) installs its owning LLVM session and calls [`llvm_construct`](../src/NativeLLVM.cod) with the completed command count, function/label/slot counts and the exact source bytes those commands address. The command view belongs to F and the source view to S. Both remain live through the synchronous call. The LLVM session owns what it subsequently verifies and emits. The existing output-name conversion and publication contract apply. For the full graph, publish command and literal ranges in G and close the source revisions before final LLVM construction.

Resolve source diagnostics during the matching byte and line-index loan. The launcher can consume its own diagnostic record after cleanup. Scope translation to `ScratchFailed` to a workspace planning or acquisition failure covered by that status. Preserve source-open failures, language rejection, protected callback completion and LLVM statuses through their existing paths. Release each successful acquisition once. A rejected input never reaches object emission.

Build the isolated executable with the current generated stubs, indexed-source, catalog, diagnostic and LLVM providers, and shared runtime. The launcher calls the generated entry. Use its existing target registration and diagnostic handling. The typed AST and frame stay in CoD. In both seed modes, start from a file with a named procedure returning a literal, call it, link the emitted object and check the exact observable result. Exercise a source error at a known position with a pre-existing output, workspace failure and normal and abrupt cleanup. Exercise two real opens of the same path in one G and the stable-ID growth check. The dependency traversal, post-close graph-owned construction and production cutover keep their separate acceptance.

## First dependency edge

**Native path bound to its actual owner.** Use one nominal `PathOwner` for borrowed `Path` values. The launcher lends a ready input path under its live activation. Establish the root's canonical key through its own OS path, preserving source-open diagnostics. The launcher's output-name loan keeps its original owner. CoD can acquire a separate empty owner for a child. An ordinary native operation accepts a writable destination, the selected parent path and a bounded view of the decoded child spelling. CoD handles module selection and escape decoding. Native code resolves a relative spelling against that parent's canonical directory and handles absolute paths according to the host path rules. Reject embedded NUL before calling a NUL-terminated OS interface. Canonical key bytes keep their exact native representation and length. CoD compares and interns them without passing them through `String` or `CString`.

The provider installs the completed path once in the selected destination. Until then it must prevent publication of a `Path` or key loan. On failure it cleans partial native allocations, preserves the failure information and leaves the empty owner safe to release. A later attempt to fill a ready owner must preserve existing loans. The path and its bounded key view then borrow the same actual owner. Intern the key into G while that view is live. `SourceRevision(path)` borrows the path during open and establishes its own independently owned revision. Keep a parent path owner available while child path resolution needs it. Before visiting a child, end the parent source loan, the canonical-key byte loan and any graph-element views that growth can invalidate. Retain the graph IDs, parent import span and independently live resources. Diagnose the failing import against its requesting source.

**Storage for the first edge.** The delivery workload uses a real in-memory region for G. It stores canonical key bytes, visit states and the published ranges needed after F or S ends. Links and ranges in G must survive relocation. Resolve them against the live G and form bounded native views only for the call that uses them. For table growth within available regional storage, construct a replacement, transfer the published prefix and install the new table after success. Growth of a table confers no permission to resize the underlying region in place. Across child reentry retain stable IDs and reacquire views from G afterwards.

An isolated root-to-child probe may use named arrays in the compilation's lexical scope to verify CoD path selection, interning, traversal and handler restoration with real files. Keep the same graph operations and verify regional placement, relocation, actual-owner bounds and release on the delivery path. An absolute element pointer in a lexical array descriptor cannot be stored as a regional link. Bring the intended regional source and the smallest required seed increment into the next review together.

**Next implementation review.** Show the actual CoD declarations and generated bindings for the borrowed launcher root, writable child staging and ready-only loans. Show the native representation for each actual owner, failure mapping, relative and absolute paths, bounded key identity including native non-UTF-8 bytes where supported, and cleanup on root open, resolution and child open failures. Include two same-type owners, attempted premature lending or replacement, and the requesting import span. Show the regional table layout, placement and transient view formation on the real edge. The concrete provider code, regional lowering and graph handler wiring receive pre-code review as a batch.

## Reentry and completion

The compilation activation installs a scoped symbolic dependency procedure. The driver visits the root, discovers edges in source order and invokes the installed procedure with `behave` for each edge. It reuses checked modules and applies the current compiler profile's cycle and depth checks to active visits. On a new visit it opens the child revision and creates its frame. Once all required children succeed, the parent binds, checks and publishes using their stable IDs.

The first root runs with no preinstalled dependency handler. A lookup while executing `behave` sees the current installation. An actual definition-activation snapshot or `touch` dependency must still be satisfied at activation. Recursive calls can use ordinary named CoD procedures. A nested compilation may execute the same lexical installation site for its own graph and restore its predecessor on exit. The application needs neither a stored handler nor a new rule for recursive capture or method-family composition. [Definition-installed procedures](../../../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)

Normal return, rejection and exceptional exit preserve the parent binding and frame. Release each successfully acquired resource exactly once in reverse acquisition order within its lifetime. A failed native acquisition cleans its acquired prefix before reporting failure. An abrupt CoD completion crosses a native loan only after its protected cleanup. A failed visit publishes no checked entry for that module. Retain completed graph data needed by LLVM after source mappings close. File output follows the existing publication contract.

## Acceptance

- Build real booters in both seed modes. Exercise a root with no previous handler, then nested compilations with distinct graphs. Observe outer → inner → outer dispatch on success and after a caught child exception. Check the actual graph and frame selected at each step, cleanup counts, and restoration before the outer handler resumes.
- Exercise a diamond with a reused leaf and a renamed re-export, the 49-unit graph, cycle and depth failures, failed open/canonicalization, and child rejection at the parent's import span. Check module and definition identity, initialized publication, and preservation of existing object output on rejection.
- Exercise typed construction, legal relocation or storage growth, and subsequent matching and lowering of the same indexed AST. Reject wrong-variant access and incomplete fields. Verify that transient source and frame views cannot enter G or survive owner replacement.
- Compare the real checked cross-module output (`0, 11, 0`) and deferred-release output (`1, 1, 1`). Compare generated CompilerCLI provider bytes with the seed, then relink and run using those providers. Verify that acquired revisions close before final LLVM construction from graph-owned units. Review the C entry points as part of acceptance.
