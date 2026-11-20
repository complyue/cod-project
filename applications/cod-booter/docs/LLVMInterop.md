# Native C providers, LLVM bindings and scoped cleanup

Revision 12 · 2026-11-20 · Confirmed for the first booter: synchronous calls, scoped native handles, LLVM session ownership, C declaration direction and constructor failure.

## Design authority

- **Generated providers.** [D-20-C-PROVIDERS](../../../design/DesignAuthorityRegistry.md#d-20-c-providers) covers [the generated boundary](#generated-c-providers-for-the-first-booter) under [H-BOOTER-GENERATED-C-20261120](../../../CHANGELOG.md#h-booter-generated-c-20261120) and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120). P-642 continues C-80 under charter §§4.3, 6.3, 8.1 and 12. Checked CoD declarations make resource use reviewable and drive equivalent C headers and stubs from both compilers. Static providers and synchronous calls cover the first indexed source, working region and LLVM workload.
- **Native handles and C records.** [D-20-C-HANDLES](../../../design/DesignAuthorityRegistry.md#d-20-c-handles), P-646/P-647 and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120) cover [the scoped handle and record rules](#native-handles-and-c-records). Scoped ownership ties disposal to a live activation and makes actual C resource dependencies visible. Preserve the native C boundary, [D-28 interop](../../../design/DesignAuthorityRegistry.md#d-28-interop), [D-18 here parameters](../../../design/DesignAuthorityRegistry.md#d-18-here-param) and D-14/D-17 ownership and cleanup. Verify both borrowed handle words, loan confinement, valid record layout, and complete and partial acquisition paths.
- **First LLVM session.** [D-20-LLVM-HANDLES](../../../design/DesignAuthorityRegistry.md#d-20-llvm-handles) applies the same owner and capability path to the [LLVM session lifecycle](#llvm-session-ownership) under [H-BOOTER-LLVM-HANDLES-20261120](../../../CHANGELOG.md#h-booter-llvm-handles-20261120). Check disposal of the acquired prefix on construction failure and exact reverse release after successful acquisition. Verification and emission remain explicit operations.
- **First-host declarations and acquisition.** [D-20-C-DECLARATIONS](../../../design/DesignAuthorityRegistry.md#d-20-c-declarations) and [D-20-C-ACQUISITION](../../../design/DesignAuthorityRegistry.md#d-20-c-acquisition) cover the [checked declarations and acquisition outcome](#declarations-and-the-first-host-call) at r12 under [H-BOOTER-C-DECLARATIONS-20261120](../../../CHANGELOG.md#h-booter-c-declarations-20261120). P-644/P-645 apply C-80 and charter §§4.3, 6.3 and 12. One checked signature determines the provider direction and the failure a caller must handle. Check both call directions and observe partial native cleanup before a typed CoD catch. The worked source, scratch and LLVM calls apply the r11 owner and completion contracts. The permanent ABI, general fallible destruction and build configuration have separate scope.
- **Related language contracts.** [D-20-NATIVE-C](../../../design/DesignAuthorityRegistry.md#d-20-native-c) selects C for native operations. [D-22 init destinations](../../../design/DesignAuthorityRegistry.md#d-22-init-places) and [D-17 defer timing](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes) continue within their recorded scopes. [H-601](../../../CHANGELOG.md#h-601) and [H-602](../../../CHANGELOG.md#h-602) establish the ordinary init-reference and explicit var boundary. These contracts govern ordinary CoD places and deferred actions alongside the scoped native owner.

## Generated C providers for the first booter

Checked CoD declarations define the required C behavior interfaces. The seed and booter derive equivalent C contracts and generate provider headers and stubs. Generated code adapts arguments, results and callbacks and performs enforceable boundary checks. Separate handwritten C implements the operations and upholds the native resource protocols. The first booter statically links its providers.

Calls in both directions use synchronous `here` semantics. A body can use direct lexical access in its own supplying activation while that activation is live. Preserve actual owners, access permissions, bounded backing and ordinary or abrupt completion through each call. A provider keeps all lent backing valid for the entire callback, including across nested calls. A callback and its confined views cannot outlive the activation that supplies them. Ordinary feedback, provider failure and CoD abrupt completion remain distinct. Completion crosses the boundary with its original CoD meaning and observes the ordinary cleanup rules.

## Native handles and C records

A fieldless `data "C"` declaration identifies a nominal CType for native handle construction. The marker is used in signatures and has no by-value record role. Each native handle payload is an opaque 64-bit word. CoD declares each constructor's parameters under the bits-or-handle rule. Generate C stubs for the CType's constructors and destructor, with separate handwritten implementations.

- `make owned handle SomeCType(...)` acquires an `owned handle`. It is an owner kind, subject to the lexical-variable-only ownership rule used for `owned region`. At scope exit, the owning variable releases the resource it still owns through the generated destructor stub. An owning handle cannot be a field.
- `make handle SomeCType(...) in oh` acquires a borrow under the actual live owner `oh`. The borrow carries no independent destruction responsibility. When creating the borrow also creates a native child requiring disposal, the provider attaches the child to `oh` before publishing the loan. The provider preserves the child's native dependencies for the duration of that loan.
- A borrowed `handle in oh` crossing from CoD to C carries both opaque 64-bit words, one for its own payload and one for `oh`. They supply the native context even when a signature has no separate owner argument. The checked reference to the actual owner instance supplies the lifetime relationship. The words convey no proof of liveness or release authority.
- Nonempty by-value `data "C"` records are recursively bits-only, including nested records. Borrowed handles can travel as explicit locals, arguments and results with their owner obligations. Native aggregates holding handles stay in C behind a typed handle. Native resources and their handle payloads are process-local.

The lexical storage lifetime, the actual resource owner and the dependencies of any native child remain distinct. A failed acquisition must unwind partial native resources without leaving a live owner or borrow for the caller. The concrete destructor failure policy remains open.

### LLVM session ownership

The first LLVM binding acquires the session as a lexical owned handle. Owner-bound typed borrows cover its views. The generated destructor stub releases a successfully acquired session. C releases any acquired resources before reporting construction failure, while it still owns the acquired prefix. Release the builder, module and context in that order as applicable. CoD invokes verification and emission explicitly while the session is live. Ordinary `defer` and `init` retain their language contracts for other work.

<a id="first-declaration-profile-for-review"></a>

### First declaration profile

The first generated boundary serves the indexed source, scratch and LLVM operations. The following table identifies the owners each operation has to preserve.

| Operation group | First-booter contract |
| --- | --- |
| C entry and paths | The launcher lends source and output path handles under its live activation. The source constructor reads the path while that loan is valid. The CoD driver returns its ordinary compilation status. The production entry also needs an immutable intrinsic catalog and a diagnostic destination. Their concrete transport remains open. |
| Indexed source revision | CoD acquires a source revision as an owned CType. A synchronous use lends read-only bytes and the completed line index under one actual revision owner. CoD formats a rejected byte position while that owner remains live. Tokens retain revision-relative half-open byte spans. An unsuccessful acquisition produces no lend. |
| In-memory working region | CoD computes checked token, syntax and command capacities. A native scratch owner provides fixed backing and supplies properly initialized arrays to a synchronous work body under one actual working owner. The called operations preserve CoD's access and relocation rules. |
| LLVM session | Apply the [confirmed session lifecycle](#llvm-session-ownership) inside the work scope. The construct operation borrows the readonly command array under the caller's actual working owner and checks its initialized prefix. The session is destroyed before the scratch owner and source revision. |

### Declarations and the first host call

**Direction of a checked declaration.** A bodyless `proc "C"` declares a C-provided operation. A `proc "C"` with a CoD body defines an entry that C can call through its generated adapter. Ordinary module lookup resolves the declaration and signature, and the generator diagnoses conflicting definitions of the same boundary identity. An unmarked CoD declaration or module export alone has no C linkage. A bodyless constructor named for a fieldless CType declares its admitted arguments and failures. The generator also emits one disposal hook per owning CType for the provider to implement.

```cod
data "C" Path;
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
        owned handle src:SourceRevision <- make owned handle SourceRevision(path);
        return source_use(src) (bytes, lines) => {
            continue compile_revision(bytes, lines, output);
        };
    } catch (e:SourceOpenException) {
        return input_unavailable(e.os_error);
    }
}
```

**Constructor outcome.** Successful `make owned handle SourceRevision(path)` publishes one lexical owner. On acquisition failure, the C provider saves any error data needed by the declared exception and releases each resource it has already acquired exactly once, in dependency-safe order. After rollback the generated boundary raises the constructor’s declared CoD exception. The failed construction publishes no handle or loan and schedules no destructor for the unpublished owner. A checked failure must be caught or covered by the caller’s `throws` contract. In the example, the catch chooses the ordinary CLI status.

`SourceOpenException` is an ordinary library-defined checked subtype. For an OS open failure, the adapter can receive the preserved error code and implement the equivalent of `throw SourceOpenException { os_error <- code; };`. The raised value owns its exception storage and can outlive the native acquisition attempt. Both seed modes need nominal subtype checking, owned exception construction, and ordinary typed propagation and cleanup on this path. Native failure transport is private to the generator. See [exception construction and ownership](../../../design/ExceptionControl.md#make-exception-expressions).

The example assumes `input_unavailable` supplies the chosen status and `compile_revision` uses the source and output while their owners are live.

**Owner and loan.** The generated launcher entry associates its two `Path` arguments with the live activation supplying the path strings. Each borrow has its own payload and the same actual A. Keep the bounded bytes valid through the call. The source constructor may read them during acquisition. Retaining native input after return needs independently established backing and lifetime. The new revision owns its own mapping and line index. Passing `src` to `source_use` borrows that owner without consuming it. Bind S once from that actual owner. The callback names the already bound S for both source arrays. It introduces no fresh owner. The adapter verifies that both ranges belong to the supplied revision as well as to S. For this path, the caller supplies a reborrow of the owning revision. Two revisions with identical contents or the same reused native address still lend different backing. A child handle held under S does not itself establish a new owner.

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

### Acceptance

- Compare both generators' contracts for the same declarations and target. Check generated constructor and destructor adaptation, both borrowed handle words and synchronous calls in both directions with live lexical access.
- Build and run the first booter through the generated boundary. Use the indexed source and working region to compile [the deferred-release workload](../workloads/deferred-release.cod). Verify LLVM output, run it and observe deferred release before the caller handles the failure. Report an invalid input using the source index before releasing the revision.
- Check wrong-owner and escaped borrows, writable access to the mapped source, out-of-bounds or invalid views, incompatible record layout and nested handle fields in by-value records. Check that a constructor which creates a disposable child associates it with its owner before exposing the loan.
- Exercise successful acquisition and failure after a partial source, scratch or LLVM acquisition. Observe each acquired resource released once in dependency order before the caller’s typed catch runs. Check the declared library exception and its preserved error field, onward propagation, and rejection of an unhandled checked call. On success, observe one final destruction. Check an escaping callback failure separately from normal feedback, including `255u8`.

## Select APIs by application demand

Add bindings as the compiler needs them for context, module, type, value and block construction, verification, selected optimization and native output. Tokenization and parsing are independent of LLVM. Keep compiler algorithms and regional semantic data in CoD. [H-352](../../../CHANGELOG.md#h-352) reserves third-party intrinsic extension for a separate SPI plan.

## Ordinary defer and output initialization

[D-22](../../../design/ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing) governs provider operations that initialize a caller place. Plain `init n:T in O` and `init ref n:T in O` retain ordinary selected-target reference semantics and initialization responsibility. Explicit `init ref var` or `init ptr var` supplies caller-slot declared place aliasing and `init val var` supplies a caller bits place. A provider must preserve the caller's actual storage and checked path to normal completion. Writes that already occurred remain observable on failure.

[D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes) governs auxiliary deferred work. `defer call(...)` prepares the selected callable and arguments at registration and invokes only the final call at scope exit. A deferred block executes its contents during cleanup. This applies to ordinary provider calls reached by these forms. The owned handle's scoped destructor supplies that resource's ownership cleanup.
