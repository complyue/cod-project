# LLVM bindings, handles and scoped cleanup

Revision: 3 · Recorded: 2026-11-11 · Status: planning draft; examples are proposed source notation, not executable fixtures.

## Design authority

- **Registry / coverage:** [D-20](../../../design/DesignAuthorityRegistry.md#d-20), revision 3, all sections; native handle, provider, output-parameter and registration-time evaluation contracts are `pending-human`.
- **Baseline / assessment:** [H-353](../../../design/DesignAuthorityRegistry.md#h-353) requests selective LLVM bindings, scoped release and explicit generic-owner ref access. [H-354](../../../design/DesignAuthorityRegistry.md#h-354) proposes a write-only `out` modifier and uninitialized local handle storage. [H-355](../../../design/DesignAuthorityRegistry.md#h-355) selects the term registration-time evaluation within defer and requests syntax alternatives; [K-475](../../../design/DesignAuthorityRegistry.md#k-475) records those unselected candidates. C-80/P-314/P-316–P-318 connects native integration to C-02/C-11/C-79 and charter §6; C-81/P-319/P-320 relates output initialization to C-65's explicit storage/owner contract.
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
defer ((llvm_context_init(ctx)))();
```

`owner o` permits caller storage in a lexical scope or a region when the handle's own storage rules permit it. A `region` binder would unnecessarily require the latter. `out` describes access and initialization, while `in o` identifies the actual caller destination; neither substitutes for the other. The call site still passes `ctx` normally, without a separate output argument expression or a read of its payload.

For one-line cleanup, the initializer returns a zero-argument cleanup callable as well as writing `ctx`. Its owner/capture contract needs design together with the native handle category; writing an output and returning cleanup are separate responsibilities.

The proposed `ref ctx: LLVMContextHandle;` allocates target storage but leaves its payload uninitialized. The reference is usable as an output destination immediately; reading the handle before successful initialization rejects. This addresses the local handle case, without authorizing lexical allocation of ordinary non-bits regional graphs. Its extension to other declaration types needs explicit coverage.

An `out` method must establish its output on every normal return; the compiler can then carry that postcondition to the caller. Merely having an `out` annotation is not implementation proof. The CoD body must be checked, and a native provider must uphold its trusted declaration. A write-only output view cannot be used to read the previous payload. Aliasing between arguments and the rules for replacing an already initialized resource remain review items.

Exceptional exit does not grant the normal-return initialization postcondition. Subsequent reads on a catch path still need a valid proof. Partial native acquisition, writes made before failure and release responsibility must agree with the selected initializer contract; this proposal does not imply transactional rollback. These checks are compile-time obligations rather than a new runtime uninitialized-value exception mechanism.

## Registration-time evaluation within defer

The candidate `defer ((llvm_context_init(ctx)))();` separates two moments:

| When | Proposed operation |
| --- | --- |
| Registration | Evaluate the inner initializer once, initialize caller storage and retain its returned callable value. |
| Scope exit | Invoke that retained value in the scope's deferred-cleanup order. |

Plain `defer f(x)` remains delayed evaluation under [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes). Registration-time evaluation means runtime evaluation when control reaches the defer expression, not compile-time evaluation. The spelling, nesting and applicability beyond this pattern remain open.

### Syntax candidates

All three sketches retain the returned callable value now and invoke it at scope exit. None is selected.

| Form | Example | Practical trade-off |
| --- | --- | --- |
| Double-parenthesis boundary | `defer ((init(ctx)))();` | Compact and shows the invocation outside the registration-time expression. Repeated parentheses gain timing significance, requiring preservation during parsing and clear limits on nested markers. |
| Named registration clause | `defer (proc cleanup = init(ctx)) { cleanup(); }` | Names the retained value and extends the existing explicit-capture direction `defer (x) { f(x) }`. Longer, but supports several preparations and a readable cleanup body without expression-level staging markers. |
| Bracketed boundary | `defer [init(ctx)]();` | Compact and visually distinct from grouping. Adds a special expression-list meaning beside existing callable capture lists; list elements, indexing and capture-versus-invocation grammar need precise separation. |

The named clause is the initial recommendation for further discussion, not an accepted syntax. Its initializer executes at registration; the binding is available only to the deferred body, whose invocation occurs later. A callable declaration retains a callable value rather than capturing a mutable lexical ref cell. An illustrative extension is `defer (val saved = value) { report(saved); }`, using explicit value semantics for a saved bits argument. Any reference form must retain its ordinary owner/lifetime obligations.

Named clauses need declaration/cleanup scope handling; boundary forms need AST markers that survive ordinary grouping/index parsing. All forms share the substantial implementation requirements: correct scope exit, callable storage, definite initialization and exceptional cleanup. No measured implementation-cost claim follows from this grammar comparison. Additional preparation expressions, evaluation order and failure between preparations need a contract before those generalizations are implemented.

The candidate lowering uses scope-owned callable storage plus deferred invocation. It should not reread a later-replaced callable holder. Selecting a procedure early does not silently change direct-call/current-binding semantics into `perform` semantics.

For native release, retaining the acquired opaque handle value rather than the lexical `ctx` reference avoids retargeting cleanup if that slot changes. This is a proposed binding-provided cleanup representation, not permission for ordinary closures to capture lexical ref cells. Native-resource ownership, captures, copying and callable allocation failure must be resolved together.

If acquisition throws, no cleanup for that failed acquisition is registered; partial native acquisition remains the initializer's responsibility. Failure while constructing or registering the returned callable also needs a leak-free protocol. Defer runs a registered action once; that fact alone does not establish a linear, single-release contract for every copy of a callable or handle. Cleanup order must respect native dependencies, and behavior if cleanup throws remains part of the exception design.

## Review before implementation

Select handle storage and invalidation rules, initializer/cleanup contracts, and registration-time syntax before implementing this idiom. Validate it on one actual native-resource lifecycle, including failure paths and nesting, before expanding the binding surface. General scope/defer lowering and runtime callable support are dependencies; neither is supplied by this application plan. No independent lifetime framework, native pointer registry or general mechanism for evaluating arbitrary expressions early is presumed necessary.
