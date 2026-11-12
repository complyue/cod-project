# LLVM bindings, handles and scoped cleanup

Revision: 4 · Recorded: 2026-11-12 · Status: planning draft; examples are proposed source notation, not executable fixtures.

## Design authority

- **Registry / coverage:** [D-20](../../../design/DesignAuthorityRegistry.md#d-20), revision 4, all sections. Native handle, provider and output-parameter contracts remain `pending-human`. The call/block distinction and registration-time evaluation are confirmed language requirements under [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes), not implementation evidence for this application.
- **Baseline / assessment:** [H-353](../../../design/DesignAuthorityRegistry.md#h-353) requests selective LLVM bindings, scoped release and explicit generic-owner ref access. [H-354](../../../design/DesignAuthorityRegistry.md#h-354) proposes a write-only `out` modifier and uninitialized local handle storage. [H-355](../../../design/DesignAuthorityRegistry.md#h-355) supplies registration-time evaluation terminology; [H-356](../../../design/DesignAuthorityRegistry.md#h-356) selects `defer init(...)()` and authorizes immediate language work. [K-515](../../../design/DesignAuthorityRegistry.md#k-515) records this plan update. C-80/P-314/P-316–P-318 connects native integration to C-02/C-11/C-79 and charter §6; C-81/P-319/P-320 relates output initialization to C-65's explicit storage/owner contract.
- **Dependencies / invariants:** [D-14](../../../design/RegionalOwnership.md) owner/access and initialization, [D-17](../../../design/ExceptionControl.md) exceptions/scopes, and [D-18](../../../design/RegionalClosures.md) callable values/captures remain authoritative within their recorded scopes. Preserve relocation and distinguish native-resource lifetime from handle storage. No general lexical-reference capture or implicit lifetime extension follows.
- **Acceptance / exclusions:** a future binding must demonstrate valid argument access, normal/exceptional release and no use after native invalidation. This document neither selects a C versus C++ shim ABI nor changes parsing, closure representation, provider registration or runtime code loading.

## Select APIs by application demand

Introduce bindings when a concrete compiler operation needs them: context/module construction, type/value/block construction, verification, selected optimization and output. Tokenization and parsing have no inherent LLVM dependency. Avoid generating an entire API binding surface before the application uses it.

A small provider boundary can expose native services while CoD owns compiler algorithms and regional semantic data. [H-352](../../../design/DesignAuthorityRegistry.md#h-352) places third-party intrinsic extension on an SPI planning path; it does not yet define registration, loading, provider selection or a native ABI. Prefer source-visible module selection over an implicitly edited global registry. The native shim should not become a second implementation of CoD's parser or analyzer.

## Opaque handles and separate lifetimes

Investigate mapping native pointers to distinct CoD handle types. A pointer-backed representation does not itself require a global handle table, RTTI or reference counting. Nor does being pointer-sized make a handle an unrestricted persistent bits payload. Keep three responsibilities explicit:

1. The owner of the CoD slot containing the handle: a lexical scope or, if permitted by a separately selected storage contract, a region.
2. The lifetime and release responsibility of the native resource denoted by that handle.
3. Dependencies among native resources and borrowed handles whose validity ends with their parent resource.

A live slot is not proof that its native target is live. Copying a handle must not silently duplicate release responsibility. Dynamic bindings can expose resources within a scope without transferring their destruction responsibility on every shadow. Acquisition/release and dynamic access are related operations, not the same mechanism.

Native pointer values are process-local. They must not be persisted as usable LLVM resources in DBMR caches. Whether temporary regional storage may hold a specially classified handle, and how save/relocation checks would exclude it, is unresolved; the generic `owner` annotation alone grants no such permission. The compiler application's persistent representations remain independent of live LLVM objects.

## Initialization through an output reference

The proposed initializer combines an ordinary reference, its explicit external owner and a write-only output contract. The `out ref` ordering below is illustrative; exact modifier grammar is not selected:

```cod
proc llvm_context_init(out ref ctx: LLVMContextHandle in o)
    where { owner o }
    -> proc()->void {
    ...
}

ref ctx: LLVMContextHandle;
defer llvm_context_init(ctx)();
```

`owner o` permits caller storage in a lexical scope or a region when the handle's own storage rules permit it. A `region` binder would unnecessarily require the latter. `out` describes access and initialization, while `in o` identifies the actual caller destination; neither substitutes for the other. The call site still passes `ctx` normally, without a separate output argument expression or a read of its payload.

For one-line cleanup, the initializer returns a zero-argument cleanup callable as well as writing `ctx`. Its owner/capture contract needs design together with the native handle category; writing an output and returning cleanup are separate responsibilities.

The proposed `ref ctx: LLVMContextHandle;` allocates target storage but leaves its payload uninitialized. The reference is usable as an output destination immediately; reading the handle before successful initialization rejects. This addresses the local handle case, without authorizing lexical allocation of ordinary non-bits regional graphs. Its extension to other declaration types needs explicit coverage.

An `out` method must establish its output on every normal return; the compiler can then carry that postcondition to the caller. Merely having an `out` annotation is not implementation proof. The CoD body must be checked, and a native provider must uphold its trusted declaration. A write-only output view cannot be used to read the previous payload. Aliasing between arguments and the rules for replacing an already initialized resource remain review items.

Exceptional exit does not grant the normal-return initialization postcondition. Subsequent reads on a catch path still need a valid proof. Partial native acquisition, writes made before failure and release responsibility must agree with the selected initializer contract; this proposal does not imply transactional rollback. These checks are compile-time obligations rather than a new runtime uninitialized-value exception mechanism.

## Registration-time evaluation within defer

The selected language forms are `defer call(...)` and `defer { ... }`. Outside the explicit block form, the operand must be a function/procedure call. Registration evaluates the selected callable and its arguments once; scope exit executes the final invocation using the retained values. In a call chain, only the outermost invocation is deferred. Parentheses retain ordinary grouping semantics and introduce no timing marker.

This supplies the scope-cleanup role commonly implemented by C++ destructors: initialization returns the particular cleanup action, and `defer` attaches it to the acquiring scope. The native binding must still establish who may release the resource; merely copying its handle or cleanup callable does not establish another release obligation.

| Source | At registration | At scope exit |
| --- | --- | --- |
| `defer llvm_context_init(ctx)();` | Run the initializer and retain its returned cleanup callable. | Invoke that cleanup callable. |
| `defer release(select_handle());` | Select `release` and evaluate its argument. | Call it with the prepared argument. |
| `defer { release(select_handle()); }` | Register the block. | Evaluate the whole block, including handle selection. |

This follows [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes). Registration-time evaluation is runtime work when control reaches `defer`, not compile-time evaluation. Normal argument conversion and passing rules still apply: saving a borrowed reference preserves the reference and its owner obligations; it does not snapshot the referenced memory. Bits and callable values retain their ordinary value semantics. Selecting a procedure now does not turn its eventual direct/current-binding call into `perform`.

The implementation should prepare the call through the existing typed call path and retain its operands in the owning scope. It must not evaluate the outer call during registration, reevaluate its arguments during cleanup, or reread a subsequently replaced callable holder. The language scope/defer work can proceed independently of the proposed LLVM handle and `out` features.

For native release, retaining the acquired opaque handle value rather than the lexical `ctx` reference avoids retargeting cleanup if that slot changes. This is a proposed binding-provided cleanup representation, not permission for ordinary closures to capture lexical ref cells. Native-resource ownership, captures, copying and callable allocation failure must be resolved together.

If acquisition throws, no cleanup for that failed acquisition is registered; partial native acquisition remains the initializer's responsibility. Failure while constructing or registering the returned callable also needs a leak-free protocol. Defer runs a registered action once; that fact alone does not establish a linear, single-release contract for every copy of a callable or handle. Cleanup order must respect native dependencies, and behavior if cleanup throws remains part of the exception design.

## Review before implementation

The language refactor is authorized as the next implementation item: implement the call/block timing contract together with the explicit scope rules and focused lifetime/exception checks in D-17. Current source has neither a defer AST/lowering path nor the accepted distinction between grouping braces and explicit block scopes; successful callable tests alone do not establish this feature.

Handle storage/invalidation, `out` syntax and initializer/cleanup resource contracts still need selection before the LLVM lifecycle example becomes executable. Validate that binding on one actual native-resource lifecycle, including failure paths and nesting, before expanding the API surface. No independent lifetime framework, native pointer registry or general early-evaluation operator is required by the selected defer syntax.
