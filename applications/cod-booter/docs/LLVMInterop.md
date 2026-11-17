# LLVM bindings, handles and scoped cleanup

Revision: 5 · Recorded: 2026-11-17 · Status: planning draft; examples are proposed source notation, not executable fixtures.

## Design authority

- **Output-contract continuity:** [H-571](../../../CHANGELOG.md#h-571)/[K-OUTPUT-ACCEPT-20261117](../../../CHANGELOG.md#k-output-accept-20261117) covers document-set r4→5. D-22’s selected init declared-place roles replace a separate out spelling; native handle storage, target-initializing helper syntax, ownership and provider proof remain pending. Preserve C-81/C-65, charter §§4.3/6.3/8/12, D-16 and existing D-17 call/block timing. No application implementation, physical ABI or broader qualification follows; pipeline semantics are unchanged.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../../../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Registry / coverage:** [D-20](../../../design/DesignAuthorityRegistry.md#d-20), revision 5, all sections. Native handle, provider and output-parameter contracts remain `pending-human`. The call/block distinction and registration-time evaluation are confirmed language requirements under [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes), not implementation evidence for this application.
- **Baseline / assessment:** [H-353](../../../CHANGELOG.md#h-353) requests selective LLVM bindings, scoped release and explicit generic-owner ref access. [H-354](../../../CHANGELOG.md#h-354) identifies output-initialization and uninitialized-handle storage requirements; H-571 supplies current init-place roles without selecting a native handle representation. [H-355](../../../CHANGELOG.md#h-355) supplies registration-time evaluation terminology; [H-356](../../../CHANGELOG.md#h-356) selects `defer init(...)()` and authorizes immediate language work. [K-515](../../../CHANGELOG.md#k-515) records this plan update. C-80/P-314/P-316–P-318 connects native integration to C-02/C-11/C-79 and charter §6; C-81/P-319/P-320 relates output initialization to C-65's explicit storage/owner contract.
- **Dependencies / invariants:** [D-14](../../../design/RegionalOwnership.md) owner/access and initialization, [D-17](../../../design/ExceptionControl.md) exceptions/scopes, and [D-18](../../../design/CallableViewsAndCaptures.md) callable values/captures remain authoritative within their recorded scopes. Preserve relocation and distinguish native-resource lifetime from handle storage. No general lexical-reference capture or implicit lifetime extension follows.
- **Acceptance / exclusions:** a future binding must demonstrate valid argument access, normal/exceptional release and no use after native invalidation. This document neither selects a C versus C++ shim ABI nor changes parsing, callable-environment representation, provider registration or runtime code loading.

## Select APIs by application demand

Introduce bindings when a concrete compiler operation needs them: context/module construction, type/value/block construction, verification, selected optimization and output. Tokenization and parsing have no inherent LLVM dependency. Avoid generating an entire API binding surface before the application uses it.

A small provider boundary can expose native services while CoD owns compiler algorithms and regional semantic data. [H-352](../../../CHANGELOG.md#h-352) places third-party intrinsic extension on an SPI planning path; it does not yet define registration, loading, provider selection or a native ABI. Prefer source-visible module selection over an implicitly edited global registry. The native shim should not become a second implementation of CoD's parser or analyzer.

## Opaque handles and separate lifetimes

Investigate mapping native pointers to distinct CoD handle types. A pointer-backed representation does not itself require a global handle table, RTTI or reference counting. Nor does being pointer-sized make a handle an unrestricted persistent bits payload. Keep three responsibilities explicit:

1. The owner of the CoD slot containing the handle: a lexical scope or, if permitted by a separately selected storage contract, a region.
2. The lifetime and release responsibility of the native resource denoted by that handle.
3. Dependencies among native resources and borrowed handles whose validity ends with their parent resource.

A live slot is not proof that its native target is live. Copying a handle must not silently duplicate release responsibility. Dynamic bindings can expose resources within a scope without transferring their destruction responsibility on every shadow. Acquisition/release and dynamic access are related operations, not the same mechanism.

Native pointer values are process-local. They must not be persisted as usable LLVM resources in DBMR caches. Whether temporary regional storage may hold a specially classified handle, and how save/relocation checks would exclude it, is unresolved; the generic `owner` annotation alone grants no such permission. The compiler application's persistent representations remain independent of live LLVM objects.

## Initialization through an output reference

[D-22’s init contract](../../../design/ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing) exposes a caller-selected place and a checked initialization obligation. An init val parameter names bits payload storage; an init ref/ptr parameter names the reference slot being bound. Binding such a slot does not initialize its referent. Selecting the native handle category must precede choosing the applicable parameter and local declaration form; a pointer-sized native handle is not automatically a bits value, regional object or CoD reference.

The binding proposal writes the acquired handle into an eligible caller place and returns a zero-argument cleanup callable. The storage place’s actual owner, the handle’s native-resource lifetime and the cleanup responsibility are distinct. The call must not read an uninitialized input payload. Normal completion establishes the declared initialization obligation; partial native acquisition and exceptional completion require their own release proof. Provider code must uphold the checked contract rather than using the annotation as evidence.

Assuming ctx has been allocated under a separately qualified handle-storage contract, the selected defer timing permits:

```cod
# Application sketch; handle storage and native binding support remain pending.
defer llvm_context_init(ctx)();
```

Public init-place writes can be observed during the call and cleanup. They are not the compiler-private pending return slot: the returned cleanup callable is delivered only when the invocation completes successfully. A failing call supplies no blanket output-initialization postcondition or rollback. Subsequent reads need independently valid evidence, and escaped native resources need explicit cleanup. Runtime target sharing between arguments, replacement of an already initialized native resource and target-initializing helper spelling remain application review items.

## Registration-time evaluation within defer

The selected language forms are `defer call(...)` and `defer { ... }`. Outside the explicit block form, the operand must be a function/procedure call. Registration evaluates the selected callable and its arguments once; scope exit executes the final invocation using the retained values. In a call chain, only the outermost invocation is deferred. Parentheses retain ordinary grouping semantics and introduce no timing marker.

This supplies the scope-cleanup role commonly implemented by C++ destructors: initialization returns the particular cleanup action, and `defer` attaches it to the acquiring scope. The native binding must still establish who may release the resource; merely copying its handle or cleanup callable does not establish another release obligation.

| Source | At registration | At scope exit |
| --- | --- | --- |
| `defer llvm_context_init(ctx)();` | Run the initializer and retain its returned cleanup callable. | Invoke that cleanup callable. |
| `defer release(select_handle());` | Select `release` and evaluate its argument. | Call it with the prepared argument. |
| `defer { release(select_handle()); }` | Register the block. | Evaluate the whole block, including handle selection. |

This follows [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes). Registration-time evaluation is runtime work when control reaches `defer`, not compile-time evaluation. Normal argument conversion and passing rules still apply: saving a borrowed reference preserves the reference and its owner obligations; it does not snapshot the referenced memory. Bits and callable values retain their ordinary value semantics. Selecting a procedure now does not turn its eventual direct/current-binding call into `perform`.

The implementation should prepare the call through the existing typed call path and retain its operands in the owning scope. It must not evaluate the outer call during registration, reevaluate its arguments during cleanup, or reread a subsequently replaced callable holder. The language scope/defer work can proceed independently of the pending LLVM handle and init-place integration.

For native release, retaining the acquired opaque handle value rather than the lexical `ctx` reference avoids retargeting cleanup if that slot changes. This is a proposed binding-provided cleanup representation, not permission for implicit lifetime extension or hidden shared callable binding cells. Native-resource ownership, captures, copying and callable allocation failure must be resolved together.

If acquisition throws, no cleanup for that failed acquisition is registered; partial native acquisition remains the initializer's responsibility. Failure while constructing or registering the returned callable also needs a leak-free protocol. Defer runs a registered action once; that fact alone does not establish a linear, single-release contract for every copy of a callable or handle. Cleanup order must respect native dependencies, and behavior if cleanup throws remains part of the exception design.

## Review before implementation

D-17 owns the selected call/block defer timing and scope rules; current supported paths and limitations are recorded in the [source capability reference](../../../tests/cod/language/README.md). Language defer support does not establish this application’s native-handle/provider contract.

Handle storage/invalidation, applicable init-place forms and initializer/cleanup resource contracts still need selection before the LLVM lifecycle example becomes executable. Validate that binding on one actual native-resource lifecycle, including failure paths and nesting, before expanding the API surface. No independent lifetime framework, native pointer registry or general early-evaluation operator is required by the selected defer syntax.
