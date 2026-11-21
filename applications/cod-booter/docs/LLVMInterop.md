# Native C providers, LLVM bindings and scoped cleanup

Revision 18 · 2026-11-21 · Confirmed for the first booter: synchronous calls, scoped native handles, LLVM session ownership, C declaration direction, typed `SourceRevision` failure generation and declared owner CTypes for borrowed handles.

## Design authority

[D-20-C-ENUM](../../../design/DesignAuthorityRegistry.md#d-20-c-enum) at r18 applies the [delegated enum review](../../../CHANGELOG.md#k-typed-enum-review-20261121) to by-value compiler status exchange. P-660/P-661 connect the checked declaration to matching C stubs. Acceptance validates actual codes on normal delivery, preserves protected completion and exercises the source-open failure path. Shared foreign storage and permanent ABI are separate.

[D-10-OWN-QUALIFIER](../../../design/DesignAuthorityRegistry.md#d-10-own-qualifier) and [H-CONTEXTUAL-OWN-20261121](../../../CHANGELOG.md#h-contextual-own-20261121) cover the `own handle` binding and `make own handle` constructor spelling at r17.

- **Generated providers.** [D-20-C-PROVIDERS](../../../design/DesignAuthorityRegistry.md#d-20-c-providers) covers [the generated boundary](#generated-c-providers-for-the-first-booter) under [H-BOOTER-GENERATED-C-20261120](../../../CHANGELOG.md#h-booter-generated-c-20261120) and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120). P-642 continues C-80 under charter §§4.3, 6.3, 8.1 and 12. Checked CoD declarations make resource use reviewable and drive equivalent C headers and stubs from both compilers. Static providers and synchronous calls cover the first indexed source, working region and LLVM workload.
- **Native handles and C records.** [D-20-C-HANDLES](../../../design/DesignAuthorityRegistry.md#d-20-c-handles), P-646/P-647 and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120) cover [the scoped handle and record rules](#native-handles-and-c-records). [H-BOOTER-HANDLE-OWNER-CTYPE-20261120](../../../CHANGELOG.md#h-booter-handle-owner-ctype-20261120) qualifies [borrow-only declarations and actual owner binding](#declaring-and-borrowing-native-handles) at r15 under the same C-80/C-65 value path. Check two live instances of one owner CType, forwarding and a launcher entry with independent loans. [R16 continuity](../../../CHANGELOG.md#k-booter-handle-bindings-20261120) covers [initialization and ordinary CoD borrow results](#borrowed-handle-locals-and-results) under the same owner and access rules. Scoped ownership ties disposal to a live activation and makes actual C resource dependencies visible. Preserve the native C boundary, [D-28 interop](../../../design/DesignAuthorityRegistry.md#d-28-interop), [D-18 here parameters](../../../design/DesignAuthorityRegistry.md#d-18-here-param) and D-14/D-17 ownership and cleanup. Verify both borrowed handle words, loan confinement, valid record layout, and complete and partial acquisition paths.
- **First LLVM session.** [D-20-LLVM-HANDLES](../../../design/DesignAuthorityRegistry.md#d-20-llvm-handles) applies the same owner and capability path to the [LLVM session lifecycle](#llvm-session-ownership) under [H-BOOTER-LLVM-HANDLES-20261120](../../../CHANGELOG.md#h-booter-llvm-handles-20261120). Check disposal of the acquired prefix on construction failure and exact reverse release after successful acquisition. Verification and emission remain explicit operations.
- **First-host declarations and acquisition.** [D-20-C-DECLARATIONS](../../../design/DesignAuthorityRegistry.md#d-20-c-declarations) and [D-20-C-ACQUISITION](../../../design/DesignAuthorityRegistry.md#d-20-c-acquisition) cover the [checked declarations and acquisition outcome](#declarations-and-the-first-host-call). [H-BOOTER-C-DECLARATIONS-20261120](../../../CHANGELOG.md#h-booter-c-declarations-20261120) confirms the r12 baseline. [H-SOURCE-REVISION-FAILURE-20261120](../../../CHANGELOG.md#h-source-revision-failure-20261120) confirms the r13 `SourceRevision` failure interface. P-644/P-645 apply C-80 and charter §§4.3, 6.3 and 12. The checked signature gives the caller and both generators one failure contract. Check the generated type and payload and observe partial native cleanup before a typed CoD catch. Permanent ABI, general fallible destruction and build configuration have separate scope.
- **Catalog and rejection reporting.** The [first launcher integration](#builtin-catalog-and-rejection-reporting) continues [D-20-C-PROVIDERS](../../../design/DesignAuthorityRegistry.md#d-20-c-providers), D-20-C-HANDLES and [D-20-DIAGNOSTICS](../../../design/DesignAuthorityRegistry.md#d-20-diagnostics) at r14 under P-642/P-643/P-627. One checked native boundary lends the canonical builtin inputs and carries facts already available from a failing check. Verify the same builtin selections and an invalid-input report that survives releasing the indexed source. The private launcher operations carry no public ABI or diagnostic analysis requirement. [Continuity record](../../../CHANGELOG.md#k-booter-catalog-diagnostic-20261120).
- **Related language contracts.** [D-20-NATIVE-C](../../../design/DesignAuthorityRegistry.md#d-20-native-c) selects C for native operations. [D-22 init destinations](../../../design/DesignAuthorityRegistry.md#d-22-init-places) and [D-17 defer timing](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes) continue within their recorded scopes. [H-601](../../../CHANGELOG.md#h-601) and [H-602](../../../CHANGELOG.md#h-602) establish the ordinary init-reference and explicit var boundary. These contracts govern ordinary CoD places and deferred actions alongside the scoped native owner.

## Generated C providers for the first booter

Checked CoD declarations define the required C behavior interfaces. The seed and booter derive equivalent C contracts and generate provider headers and stubs. Generated code adapts arguments, results and callbacks and performs enforceable boundary checks. Separate handwritten C implements the operations and upholds the native resource protocols. The first booter statically links its providers.

Calls in both directions use synchronous `here` semantics. A body can use direct lexical access in its own supplying activation while that activation is live. Preserve actual owners, access permissions, bounded backing and ordinary or abrupt completion through each call. A provider keeps all lent backing valid for the entire callback, including across nested calls. A callback and its confined views cannot outlive the activation that supplies them. Ordinary feedback, provider failure and CoD abrupt completion remain distinct. Completion crosses the boundary with its original CoD meaning and observes the ordinary cleanup rules.

For [enum arguments, results and normal feedback](../../../design/TypedEnums.md#generated-c-exchange), generate an exact-width carrier and namespaced constants. Validate C-originating normal values before exposing them as CoD enum values. For the [compiler status workload](../../../design/TypedEnums.md#first-workload-and-acceptance), the launcher receives codes 0–7 through the ordinary result channel.

## Native handles and C records

A fieldless `data "C"` declaration identifies a nominal CType for native handle construction. The marker is used in signatures and has no by-value record role. Each native handle payload is an opaque 64-bit word. CoD declares each constructor's parameters under the bits-or-handle rule. Generate C stubs for declared constructors and a disposal hook for CoD-owned resources. Handwritten C implements the resource operations.

- `make own handle SomeCType(...)` acquires an `own handle`. It is an owner kind, subject to the lexical-variable-only ownership rule used for `own region`. At scope exit, the owning variable releases the resource it still owns through the generated destructor stub. An owning handle cannot be a field.
- `make handle View(...) in oh` constructs a borrow under the actual live owner `oh` of the CType declared by `View`. The borrow carries no independent destruction responsibility. When creating the borrow also creates a native child requiring disposal, the provider attaches the child to `oh` before publishing the loan. The provider preserves the child's native dependencies for the duration of that loan.
- A borrowed `handle in oh` crossing from CoD to C carries both opaque 64-bit words, one for its own payload and one for `oh`. They supply the native context even when a signature has no separate owner argument. The checked reference to the actual owner instance supplies the lifetime relationship. The words convey no proof of liveness or release authority.
- Nonempty by-value `data "C"` records are recursively bits-only, including nested records. Borrowed handles can travel as explicit locals, arguments and results with their owner obligations. Native aggregates holding handles stay in C behind a typed handle. Native resources and their handle payloads are process-local.

The lexical storage lifetime, the actual resource owner and the dependencies of any native child remain distinct. A failed acquisition must unwind partial native resources without leaving a live owner or borrow for the caller. The concrete destructor failure policy remains open.

### Declaring and borrowing native handles

A borrow-only fieldless CType names its owning CType in its declaration. Here `LLVMSession` is a nominal type. At the use site, `session` is one live instance. Ordinary module lookup resolves the type identities.

```cod
data "C" LLVMSession;
data "C" ModuleView in LLVMSession;
proc "C" ModuleView(val index:u64);
proc "C" inspect_module(handle view:ModuleView in O) where {const owner O};

# Inside a caller with a live binding declared as own handle session:LLVMSession:
inspect_module(make handle ModuleView(0u64) in session);
```

The constructor takes its written arguments and receives the selected owner through `in session`. Check its declared owner CType, actual binding, access and lifetime before lending. The provider attaches any newly acquired disposable child to that instance before publishing the borrow. A borrow-only CType cannot be acquired with `make own handle` or used as an owning CType for another declaration. Its borrow carries no destructor obligation. A fieldless owner CType can itself be borrowed. For example, `source_use(src)` passes the same owned `SourceRevision` to the ordinary `handle revision:SourceRevision in S` parameter and binds S to its actual owner. The caller keeps the release obligation.

A borrow's C representation supplies its own opaque payload and the actual owner's opaque word. Inside CoD, several borrows may share one checked owner binding, and the compiler may materialize the owner word at the crossing. Locals, forwarding calls, results and synchronous `here` bodies preserve the binding selected when the borrow arose, while the selected owner remains live. Replacing a holder cannot retarget an earlier borrow. Selecting between two views of one session is valid. Selecting between views of two independent sessions into one result requires proof of a single actual owner under the [ordinary owner rules](../../../design/RegionalOwnership.md#pointer-variables-and-owner-generic-parameters). A matching CType or equal machine words cannot supply that proof.

The launcher can use ordinary fieldless CTypes for the owners of foreign-supplied borrows.

```cod
data "C" LauncherPaths;
data "C" Path in LauncherPaths;
data "C" LauncherDiagnostics;
data "C" DiagnosticDestination in LauncherDiagnostics;
```

The private marker names describe what C owns during this invocation. The launcher lends one actual path owner A for both paths and a separate writable record owner D. In the [launcher entry](../src/CompilerCLI.cod), the two occurrences of `Path in A` constrain A to `LauncherPaths`. The diagnostic parameter constrains D to `LauncherDiagnostics`. The generated adapter binds the incoming owner evidence for A and D separately and checks agreement for the two uses of A. The native caller supplies truthful typed provenance and valid backing and keeps both loans live through the synchronous call. The boundary words alone cannot authenticate arbitrary C input. Merely naming an owner CType grants no constructor, allocation or disposal permission. The entry borrows both owners. The launcher retains responsibility for their storage. A host payload may differ from its owner word, as it does for a path; equality for the current diagnostic record is a private provider convention.

### Borrowed handle locals and results

```cod
proc forward(handle input:ModuleView in O) -> handle ModuleView in O
    where {const owner O} {
    handle view:ModuleView in O = input;
    return view;
}

# Inside a caller with a live session:
handle first:ModuleView = make handle ModuleView(0u64) in session;
handle observed:ModuleView in session = forward(first);
```

In a lexical declaration, `=` binds the borrow produced by the initializer. Omitting `in O` infers its single proven actual owner. Writing `in O` checks the initializer against an owner already in scope. Either spelling also checks the nominal handle CType, access and lifetime. Initializing from another local gives the new binding the same selected borrow. Dropping either local leaves responsibility for the native child with the session.

For an ordinary CoD callable, `-> handle T in O` ties the result to an actual owner established by an input. Each returning path must establish T and O and respect all access and loan-duration restrictions through cleanup and delivery to the caller. Returning an input through a lexical local is valid. Returning a child of a session acquired by the callee rejects when that session ends at return. A callback-confined borrow cannot escape its loan merely because its resource owner survives. Forwarding a readonly capability cannot authorize a writable native operation. Check both native words when a consumer passes the result on to C.

Post-initialization `view = other;` for a native handle local is a separate question. The two alternatives are a mutable local that can select another compatible borrow under its original owner, and a local fixed to its initial borrow. Initialization and ordinary CoD results can be implemented independently of this choice.

### LLVM session ownership

The first LLVM binding acquires the session as a lexical owned handle. Owner-bound typed borrows cover its views. The generated destructor stub releases a successfully acquired session. C releases any acquired resources before reporting construction failure, while it still owns the acquired prefix. Release the builder, module and context in that order as applicable. CoD invokes verification and emission explicitly while the session is live. Ordinary `defer` and `init` retain their language contracts for other work.

<a id="first-declaration-profile-for-review"></a>

### First declaration profile

The first generated boundary serves the indexed source, scratch and LLVM operations. The following table identifies the owners each operation has to preserve.

| Operation group | First-booter contract |
| --- | --- |
| C entry and paths | The launcher lends source and output path handles under its live activation. The source constructor reads the path while that loan is valid. The CoD driver returns its ordinary compilation status. The booter borrows its builtin table through a scoped catalog operation. The launcher supplies independent diagnostic storage. See [catalog and rejection reporting](#builtin-catalog-and-rejection-reporting). |
| Indexed source revision | CoD acquires a source revision as an owned CType. A synchronous use lends read-only bytes and the completed line index under one actual revision owner. CoD resolves a rejected byte position while that owner remains live. Tokens retain revision-relative half-open byte spans. An unsuccessful acquisition produces no lend. |
| In-memory working region | CoD computes checked token, syntax and command capacities. A native scratch owner provides fixed backing and supplies properly initialized arrays to a synchronous work body under one actual working owner. The called operations preserve CoD's access and relocation rules. |
| LLVM session | Apply the [confirmed session lifecycle](#llvm-session-ownership) inside the work scope. The construct operation borrows the readonly command array under the caller's actual working owner and checks its initialized prefix. The session is destroyed before the scratch owner and source revision. |

### Declarations and the first host call

**Direction of a checked declaration.** A bodyless `proc "C"` declares a C-provided operation. A `proc "C"` with a CoD body defines an entry that C can call through its generated adapter. Ordinary module lookup resolves the declaration and signature, and the generator diagnoses conflicting definitions of the same boundary identity. An unmarked CoD declaration or module export alone has no C linkage. A bodyless constructor named for a fieldless CType declares its admitted arguments and failures. The generator also emits one disposal hook per owning CType for the provider to implement.

```cod
data "C" LauncherPaths;
data "C" Path in LauncherPaths;
data "C" SourceRevision;
data SourceOpenException <: Exception { os_error:u64; }

proc "C" SourceRevision(handle path:Path in A) throws SourceOpenException
    where {const owner A};

proc "C" source_use(
    here work(ref bytes:const SourceBytes in S, ref lines:const SourceLines in S) -> u8,
    handle revision:SourceRevision in S
) -> u8 where {const owner S};

proc "C" compile_cli(handle path:Path in A, handle output:Path in A) -> u8
    where {const owner A} {
    try {
        own handle src:SourceRevision <- make own handle SourceRevision(path);
        return source_use(src) (bytes, lines) => {
            continue compile_revision(bytes, lines, output);
        };
    } catch (e:SourceOpenException) {
        return input_unavailable(e.os_error);
    }
}
```

**Constructor outcome.** Successful `make own handle SourceRevision(path)` publishes one lexical owner. On acquisition failure, the C provider saves any error data needed by the declared exception and releases each resource it has already acquired exactly once, in dependency-safe order. After rollback the generated boundary raises the constructor’s declared CoD exception. The failed construction publishes no handle or loan and schedules no destructor for the unpublished owner. A checked failure must be caught or covered by the caller’s `throws` contract. In the example, the catch chooses the ordinary CLI status.

For `SourceRevision`, derive the typed C failure interface in the generated provider stub from the checked `throws SourceOpenException` and `data SourceOpenException <: Exception { os_error:u64; }`. The handwritten provider preserves `os_error` before cleanup can change the OS error state, releases the acquired prefix in dependency-safe order, and returns the preserved value through that interface. The generated adapter then performs the equivalent of `throw SourceOpenException { os_error <- code; };`. The raised failure owns its CoD storage. The caller receives an ordinary typed catch and reads `e.os_error`.

Acceptance checks equivalent generated contracts from the seed and booter, failure after partial acquisition, exact rollback before the catch, and delivery of the original OS error even when cleanup changes the OS error state. Failed acquisition publishes no handle or loan and schedules no destructor. The generated transport is private to the two compilers. See [exception construction and ownership](../../../design/ExceptionControl.md#make-exception-expressions).

The example assumes `input_unavailable` supplies the chosen status and `compile_revision` uses the source and output while their owners are live.

**Owner and loan.** The generated launcher entry associates its two `Path` arguments with the live activation supplying the path strings. Each borrow has its own payload and the same actual A. Keep the bounded bytes valid through the call. The source constructor may read them during acquisition. Retaining native input after return needs independently established backing and lifetime. The new revision owns its own mapping and line index. Passing `src` to `source_use` borrows that owner without consuming it. Bind S once from that actual owner. The callback names the already bound S for both source arrays. It introduces no fresh owner. The adapter verifies that both ranges belong to the supplied revision as well as to S. Two revisions with identical contents or the same reused native address still lend different backing. A child handle held under S does not itself establish a new owner.

Inside `compile_revision`, acquire scratch in a lexical holder. Its use operation lends initialized writable arrays under actual owner W. Acquire LLVM session L inside that callback. Pass the command array to construct as a readonly borrow in W, check `used <= len(commands)` and read only the initialized prefix. Finish verification and emission explicitly while L lives. On each exit, release L, end the scratch loan and release scratch, then end the source loan and release the revision. Keep the active loan’s backing valid across nested or reentrant calls. A reset, release or replacement that would invalidate it must be prevented before native disposal. The array views are confined to their loan even when the resource owner outlives the callback.

For example, these fragments show the command-array part of the scratch interface. The other workspace arrays have their own element types and the same W. The LLVM operation names L and W independently.

```cod
proc "C" scratch_use(here work(ref rows:NativeCommands in W) -> u8,
                     handle scratch:Scratch in W) -> u8 where {owner W};

proc "C" construct(handle session:LLVMSession in L,
                   ref rows:const NativeCommands in W, used:u64) -> u8
    where {owner L, const owner W};
```

**Generated first-host representation.** A separate C struct per borrowed CType contains `uint64_t payload` and `uint64_t owner`; preserve the two opaque words at the C boundary. The generator can use typed descriptors with a base pointer and an element count for native array loans. For example, a byte view has a `const uint8_t *`, a source-line view a pointer to const layout-checked rows, and a writable scratch view a pointer to writable rows. The owner witness and active loan may be shared in the generated call context. These descriptors are boundary machinery, never persisted CoD by-value records. Validate range and initialized length, count conversion and multiplication, layout and alignment, read/write permissions and the actual backing contract before exposing a CoD view. Allow valid empty ranges without dereferencing a base. C must attest to backing and stability where a generated check cannot infer them from an address.

For example, a generated source callback can receive both readonly arrays and one shared loan context. A separate scratch callback can receive a writable command view in its own context. The opaque call context also identifies the live callback activation. The following are private header shapes.

```c
typedef struct { uint64_t payload, owner; } cod_path_borrow;
typedef struct { const uint8_t *base; size_t count; } cod_bytes_ro;
typedef struct { const cod_source_span *base; size_t count; } cod_lines_ro;
typedef struct { cod_llvm_command *base; size_t count; } cod_commands_rw;

typedef cod_here_u8_result (*cod_source_body)(
    cod_here_call *call, cod_bytes_ro bytes, cod_lines_ro lines);
```

A callback returning `u8` can deliver every value from 0 to 255. Give its C result a separate completion tag. Normal completion carries the typed feedback. An abrupt callback completion carries or refers to the original CoD completion, including its exception or lexical control destination. The provider stops invoking the callback, runs required cleanup and forwards that completion through the generated shim. A provider’s own operation status occupies its own typed path. Check ordinary feedback `255u8`, a provider failure and an escaping checked callback exception independently. Names, padding, calling convention and outcome carrier layout are private to the coordinated first-host generators. Compare their generated declarations and behavior on the same target.

The initial providers use a nonthrowing `void` disposal hook that releases each acquired resource exactly once. Report an actionable flush, verification or publication failure from an explicit operation while its owner is live. This suffices for the three workloads. A provider that can report failure only during implicit disposal would require a separate decision about exceptions during cleanup; this profile grants no silent loss of a promised result.

The proposed build starts with the existing seed, checks the declarations, generates the provider headers and stubs and compiles them with the separate native implementations and launcher. The seed emits the CoD driver and its C-callable boundary, and the build links them into the booter. Generation must not depend on first linking its providers. The concrete build graph requires separate configuration authorization.

### Builtin catalog and rejection reporting

The first shipped booter uses a scoped native `IntrinsicCatalog`. Its checked synchronous use operation lends the spelling bytes and export rows together. Both readonly views belong to the same actual catalog owner B for the duration of the callback. A row’s module and name spans index those exact bytes. Each span must satisfy `first <= past <= byte_count`. Seed and booter consume one canonical definition of spellings, kinds, identities and checkedness. The owner may lend immutable static backing directly. See the [builtin definitions](../../../cod/lang_intrinsic_catalog.h) and the [CoD row layout](../src/ModuleExports.cod).

```cod
data "C" IntrinsicCatalog;
proc "C" IntrinsicCatalog();
proc "C" catalog_use(
    here work(ref spellings:const SourceBytes in B, ref exports:const ModuleExports in B) -> u8,
    handle catalog:IntrinsicCatalog in B
) -> u8 where {const owner B};
```

For each invocation, the launcher supplies storage for one initially empty, fixed-size bits record under its own live owner. A checked synchronous write fills it on rejection. Take offsets from scratch-backed nodes while those nodes are live. Resolve the half-open source span and any available related location through the live revision’s line index. Write the existing stage, diagnostic code and available numeric positions into the record. A failure before a revision is available has no source position. In particular, the OS error of a failed open remains available through the ordinary typed acquisition path.

The launcher reads and formats the record after the compiler returns and the source loan closes. A sink handle can name the caller’s record, or a checked writable view can supply its storage directly. The write only stores bits. The launcher needs neither source bytes nor scratch references to format them. The booter retains its ordinary compilation status and exception handling. The names and record layout are private to the first launcher.

### Acceptance

- Compare both generators' contracts for the same declarations and target. Check generated constructor and destructor adaptation, both borrowed handle words and synchronous calls in both directions with live lexical access.
- Build and run the first booter through the generated boundary. Use the indexed source and working region to compile [the deferred-release workload](../workloads/deferred-release.cod). Verify LLVM output, run it and observe deferred release before the caller handles the failure. Resolve available diagnostic positions before releasing the revision. Check the launcher’s report after release.
- Check wrong-owner and escaped borrows, writable access to the mapped source, out-of-bounds or invalid views, incompatible record layout and nested handle fields in by-value records. Check that a constructor which creates a disposable child associates it with its owner before exposing the loan. Reject construction under the wrong CType or an expired owner. Forward two views of one session through a helper and a here body and inspect both C words. Check that a join across independent session owners rejects and that nested launcher invocations use their own loans.
- Compare catalog rows and spellings for both compiler modes, including an exported symbol and a checked exception. Verify a rejection with a known source span, a diagnostic with an available related position, and a source-open failure without a position. Reuse the launcher's result storage for a success and check that no stale diagnostic is reported.
- Exercise successful acquisition and failure after a partial source, scratch or LLVM acquisition. Observe each acquired resource released once in dependency order before the caller’s typed catch runs. Check the declared library exception and its preserved error field, onward propagation, and rejection of an unhandled checked call. On success, observe one final destruction. Check an escaping callback failure separately from normal feedback, including `255u8`.

## Select APIs by application demand

Add bindings as the compiler needs them for context, module, type, value and block construction, verification, selected optimization and native output. Tokenization and parsing are independent of LLVM. Keep compiler algorithms and regional semantic data in CoD. [H-352](../../../CHANGELOG.md#h-352) reserves third-party intrinsic extension for a separate SPI plan.

## Ordinary defer and output initialization

[D-22](../../../design/ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing) governs provider operations that initialize a caller place. Plain `init n:T in O` and `init ref n:T in O` retain ordinary selected-target reference semantics and initialization responsibility. Explicit `init ref var` or `init ptr var` supplies caller-slot declared place aliasing and `init val var` supplies a caller bits place. A provider must preserve the caller's actual storage and checked path to normal completion. Writes that already occurred remain observable on failure.

[D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes) governs auxiliary deferred work. `defer call(...)` prepares the selected callable and arguments at registration and invokes only the final call at scope exit. A deferred block executes its contents during cleanup. This applies to ordinary provider calls reached by these forms. The owned handle's scoped destructor supplies that resource's ownership cleanup.
