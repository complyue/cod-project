# Native C providers, LLVM bindings and scoped cleanup

Revision 10 · Recorded 2026-11-20 · Status: synchronous interop and native handle rules confirmed for initial delivery. The concrete declaration, lowering and build profile is under review.

## Design authority

- **Generated providers.** [D-20-C-PROVIDERS](../../../design/DesignAuthorityRegistry.md#d-20-c-providers) covers [the generated boundary](#generated-c-providers-for-the-first-booter) under [H-BOOTER-GENERATED-C-20261120](../../../CHANGELOG.md#h-booter-generated-c-20261120) and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120). P-642 continues C-80 under charter §§4.3, 6.3, 8.1 and 12. Checked CoD declarations make resource use reviewable and drive equivalent C headers and stubs from both compilers. Static providers and synchronous calls cover the first indexed source, working region and LLVM workload.
- **Native handles and C records.** [D-20-C-HANDLES](../../../design/DesignAuthorityRegistry.md#d-20-c-handles), P-646/P-647 and [H-BOOTER-C-HANDLES-20261120](../../../CHANGELOG.md#h-booter-c-handles-20261120) cover [the scoped handle and record rules](#native-handles-and-c-records). Scoped ownership ties disposal to a live activation and makes actual C resource dependencies visible. Preserve the native C boundary, [D-28 interop](../../../design/DesignAuthorityRegistry.md#d-28-interop), [D-18 here parameters](../../../design/DesignAuthorityRegistry.md#d-18-here-param) and D-14/D-17 ownership and cleanup. Verify both borrowed handle words, loan confinement, valid record layout, and complete and partial acquisition paths.
- **Remaining profile.** P-644/P-645 cover the candidate [workload profile](#first-declaration-profile-for-review). Concrete provider and entry declarations, callback owner binding and view transport, the C calling convention and physical lowering, destructor failure policy and build configuration remain pending. The initial delivery excludes DBMR, dynamic loading, a general foreign interface and a permanent ABI.
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

### First declaration profile for review

The candidate follows the real indexed source, scratch and LLVM operations. The signatures and the encoding of bounded views remain a single review topic.

| Operation group | Proposed first contract |
| --- | --- |
| C entry and paths | The launcher supplies source and output path handles borrowed under a live launcher owner, the immutable intrinsic catalog and a diagnostic destination. Carrying a path as a borrow lets the source constructor consume a permitted constructor argument while the provider resolves its bounded bytes. The entry calls the checked CoD driver synchronously and returns its ordinary status. How the entry introduces owner evidence and transports the catalog and diagnostic destination remains open. |
| Indexed source revision | CoD acquires a source revision as an owned CType. A synchronous use lends read-only bytes and the completed line index under one actual revision owner. CoD formats a rejected byte position while that owner remains live. Tokens retain revision-relative half-open byte spans. An unsuccessful acquisition produces no lend. |
| In-memory working region | CoD computes checked token, syntax and command capacities. A native scratch owner provides fixed backing and supplies properly initialized arrays to a synchronous work body under one actual working owner. The called operations preserve CoD's access and relocation rules. |
| LLVM session | CoD acquires an owned session inside the work scope. The native provider retains the context, module, builder and dependent output handles it must release. Construct borrows the readonly command array under the caller's actual working owner and checks its initialized prefix. Finish verifies and emits the object. The session is destroyed before the scratch owner and source revision. |

The first profile still needs concrete declaration spellings for generated entries and provider operations, a way to bind a producer's actual owner on entry to a callback, and a checked representation for the bounded readonly and writable views. A callback may use an already known owner where its enclosing operation supplies one. For a view without caller-bound owner evidence, the adapter must establish the provider's actual supplying owner before lending. A hardware address or matching opaque payload alone is insufficient. The C declarations, calling convention and compiler-internal storage remain open. Validate target record layouts, alignment and supported extent conversions before sharing backing.

The proposed build starts with the existing seed, checks the declarations, generates the provider headers and stubs and compiles them with the separate native implementations and launcher. The seed emits the CoD driver and its C-callable boundary, and the build links them into the booter. Generation must not depend on first linking its providers. The concrete build graph requires separate configuration authorization.

### Acceptance

- Compare both generators' contracts for the same declarations and target. Check generated constructor and destructor adaptation, both borrowed handle words and synchronous calls in both directions with live lexical access.
- Build and run the first booter through the generated boundary. Use the indexed source and working region to compile [the deferred-release workload](../workloads/deferred-release.cod). Verify LLVM output, run it and observe deferred release before the caller handles the failure. Report an invalid input using the source index before releasing the revision.
- Check wrong-owner and escaped borrows, writable access to the mapped source, out-of-bounds or invalid views, incompatible record layout and nested handle fields in by-value records. Check that a constructor which creates a disposable child associates it with its owner before exposing the loan.
- Exercise failed source and scratch acquisition and partial LLVM setup. Observe the release of each acquired resource once in the dependency order. Verify that a synchronous callback's checked failure preserves its CoD outcome and cleanup, including when ordinary feedback could use the same scalar bits.

## Select APIs by application demand

Add bindings as the compiler needs them for context, module, type, value and block construction, verification, selected optimization and native output. Tokenization and parsing are independent of LLVM. Keep compiler algorithms and regional semantic data in CoD. [H-352](../../../CHANGELOG.md#h-352) reserves third-party intrinsic extension for a separate SPI plan.

## Ordinary defer and output initialization

[D-22](../../../design/ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing) governs provider operations that initialize a caller place. Plain `init n:T in O` and `init ref n:T in O` retain ordinary selected-target reference semantics and initialization responsibility. Explicit `init ref var` or `init ptr var` supplies caller-slot declared place aliasing and `init val var` supplies a caller bits place. A provider must preserve the caller's actual storage and checked path to normal completion. Writes that already occurred remain observable on failure.

[D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes) governs auxiliary deferred work. `defer call(...)` prepares the selected callable and arguments at registration and invokes only the final call at scope exit. A deferred block executes its contents during cleanup. This applies to ordinary provider calls reached by these forms. The owned handle's scoped destructor supplies that resource's ownership cleanup.
