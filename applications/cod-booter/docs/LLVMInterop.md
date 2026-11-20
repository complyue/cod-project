# Native C providers, LLVM bindings and scoped cleanup

Revision: 9 · Recorded: 2026-11-20 · Status: provider-generation direction confirmed. The first declaration profile and the separate native-handle proposals remain under review.

## Design authority

- **CoD native interface.** [D-20-NATIVE-C](../../../design/DesignAuthorityRegistry.md#d-20-native-c) and [H-NATIVE-C-INTERFACE-20261118](../../../CHANGELOG.md#h-native-c-interface-20261118) cover r7’s interface language. C-80 and charter §6.3 place LLVM behind explicit compiler operations. Expose each selected operation through C and preserve the handle lifetime and release criteria below. Specific entry points and error conventions remain open.

- **Generated provider boundary.** [D-20-C-PROVIDERS](../../../design/DesignAuthorityRegistry.md#d-20-c-providers) and [H-BOOTER-GENERATED-C-20261120](../../../CHANGELOG.md#h-booter-generated-c-20261120) qualify the generation direction and initial workload boundary. P-642 continues the C-80 value path through charter §§4.3/6.3/12. A checked CoD contract supplies one reviewable source of native operations, and the first build tests that adaptation preserves owners, effects and observable cleanup. Preserve [D-28-INTEROP](../../../design/DesignAuthorityRegistry.md#d-28-interop) and [D-18-HERE-PARAM](../../../design/DesignAuthorityRegistry.md#d-18-here-param). Scope is static providers and synchronous callbacks for the booter’s actual source, region and LLVM uses. The concrete r9 proposal under P-644/P-645 remains `pending-human`. [K-BOOTER-C-PROFILE-20261120](../../../CHANGELOG.md#k-booter-c-profile-20261120) records continuity of the confirmed direction.

- **Output-contract alignment:** [H-601](../../../CHANGELOG.md#h-601)/[H-602](../../../CHANGELOG.md#h-602)/[K-INIT-PLACE-CORRECTION-20261117](../../../CHANGELOG.md#k-init-place-correction-20261117) covers r5→6. D-22 keeps ordinary init reference semantics and explicit var for caller-slot declared place aliasing; no target modifier is introduced. Native handle storage, ownership and provider proof remain pending. Preserve C-81/C-65, charter §§4.3/6.3/8/12, D-16 and D-17 timing/cleanup. Only the dependency/initialization interpretation changes; pipeline behavior and full application/native/value-path qualification remain unchanged.

- **Callable-reference continuity:** [K-ALIAS-GLOSSARY-20261117](../../../CHANGELOG.md#k-alias-glossary-20261117) preserves this revision's existing D-18 dependency under the canonical CallableViewsAndCaptures.md path. Only dependency links and this continuity summary change; existing contracts, qualification, acceptance and D-16 remain unchanged.

- **Registry / coverage:** [D-20](../../../design/DesignAuthorityRegistry.md#d-20), revision 9. The generated provider direction covers the next section's confirmed paragraph. The candidate declaration profile, native-handle and output-parameter mechanisms remain `pending-human`. The call/block distinction and registration-time evaluation are confirmed language requirements under [D-17](../../../design/ExceptionControl.md#deferred-work-and-explicit-scopes).
- **Baseline / assessment:** [H-353](../../../CHANGELOG.md#h-353) requests selective LLVM bindings, scoped release and explicit generic-owner ref access. [H-354](../../../CHANGELOG.md#h-354) identifies output-initialization and uninitialized-handle storage requirements; H-571 supplies current init-place roles without selecting a native handle representation. [H-355](../../../CHANGELOG.md#h-355) supplies registration-time evaluation terminology; [H-356](../../../CHANGELOG.md#h-356) selects `defer init(...)()` and authorizes immediate language work. [K-515](../../../CHANGELOG.md#k-515) records this plan update. C-80/P-314/P-316–P-318 connects native integration to C-02/C-11/C-79 and charter §6; C-81/P-319/P-320 relates output initialization to C-65's explicit storage/owner contract.
- **Dependencies / invariants:** [D-14](../../../design/RegionalOwnership.md) owner/access and initialization, [D-17](../../../design/ExceptionControl.md) exceptions/scopes, and [D-18](../../../design/CallableViewsAndCaptures.md) callable values/captures remain authoritative within their recorded scopes. Preserve relocation and distinguish native-resource lifetime from handle storage. No general lexical-reference capture or implicit lifetime extension follows.
- **Acceptance / exclusions:** a future binding must demonstrate valid argument access, normal/exceptional release and no use after native invalidation. CoD’s LLVM bindings expose a C interface. Concrete signatures, handle representations and provider registration remain open.

## Generated C providers for the first booter

Checked CoD declarations define the required C behavior interfaces. The seed and booter derive the same contract and generate C headers and provider-side stubs as part of the C native components. Generated code adapts arguments, callbacks and results and enforces the checks available at that boundary. Separate handwritten C implements the operations and upholds the provider's native resource protocols. The first revision uses statically linked providers and synchronous callbacks. CoD controls source opening and release under the same cleanup discipline as LLVM.

### First declaration profile for review

The proposal uses one first-profile module with ordinary explicit `using` and publication. A bodyless `native "C" proc` declares a statically linked provider operation. A `native "C" type` introduces a nominal C-owned control record that CoD can receive by `ref` and pass to selected operations. Only C constructs and stores the opaque record in this profile. A `native "C" entry proc` has a normal checked CoD body and requests a C-callable entry. Each declaration has an explicit result and is fully checked before generation. Generation uses the resolved module and declaration identities to produce C symbols and hook prototypes. This proposal excludes overloads at the boundary, generic type parameters, default arguments and reusable native callable values.

The following proposed notation shows the two owner origins. `H` is the actual owner of the C control record lent to CoD by the entry. The callback-local `where` on `work` binds `S`, the actual owner of the mapped revision and its line table. The owner of the control record, the callback's lexical owner, and `S` can all differ. A callback parameter can also name an owner from the outer procedure's `where`, as the LLVM input does.

```cod
native "C" type SourceState;

native "C" proc source_open(ref state:SourceState in H, path:const PathBytes in P) -> u8
    where {owner H, const owner P};

native "C" proc source_use(
    here work(ref bytes:const SourceBytes in S, ref lines:const SourceLines in S) -> u8
        where {const owner S},
    ref state:SourceState in H
) -> u8 where {owner H};

native "C" proc source_release(ref state:SourceState in H) -> void where {owner H};
```

The `where` attached to a `here` formal is proposed only for this native profile. On each synchronous application it binds the supplied resource owner for that application. Two `in S` views must carry the same actual owner. The bound owner and its views are unavailable outside the callback's lifetime. A later loan from the same still-live resource uses the same actual owner. The generated boundary checks the view against the active provider state before creating the CoD borrow. The provider keeps the corresponding backing live and stable for the whole use. An attempt to release or replace that resource during its use fails the native boundary check before any disposal. Other resources can be used in nested calls without adding an exclusivity rule to ordinary CoD references.

| Operation group | Proposed first contract |
| --- | --- |
| C entry | The launcher passes bounded source and output path bytes, initialized opaque control records for the three services, the immutable intrinsic catalog and an initialized diagnostic result place. The generated entry calls the checked CoD driver and returns its ordinary status. CoD can convert a rejected source byte to line and byte column before releasing its index. |
| Indexed source revision | CoD schedules `source_open`, `source_use` and `source_release`. `source_use` lends readonly bytes and a completed line index under one actual revision owner. A failed open lends no view and follows the same registered release path. Tokens retain their revision-relative half-open byte spans. |
| In-memory working region | CoD supplies checked token, syntax and command capacities to a reserve operation. The provider acquires fixed backing, initializes the array storage required by ordinary writable `ref` inputs, and lends the typed arrays under one actual working owner. Zero initialization is sufficient for the first workload's numeric records. CoD schedules release after their last use. |
| LLVM session | CoD chooses when to begin, construct, finish and release. Construct receives the readonly command array under its caller-supplied working owner, the initialized prefix length and the label and slot counts. Finish verifies and emits the object using the output path. Release handles both complete and partial session state before scratch and the mapping are released. |

Each use operation can apply its `here` body once, on the same thread and before the provider call returns. Acquisition failure skips the body. The generated callback reports ordinary feedback and abrupt CoD completion through distinct paths. The adapter retains the original CoD completion until transfer back to the compiled caller. C receives an opaque callback context and a status that tells it to end the current use. After abrupt completion it finishes provider-local work needed to return, and CoD runs its registered cleanup. A provider-originating failure uses its declared result or behavior. The bridge keeps callback results, provider failures and CoD exceptions distinct, including when a normal `u8` happens to equal an error code.

For the first host target, generate C calling signatures with fixed-width scalar values and typed bounded array descriptors. Use declarations to specify element identity, access, extents and owner roles. Verify target layout against the selected CoD record fields and C size and alignment before using the same backing. Check conversion from a view count to a C extent and check the LLVM initialized prefix against the declared extent. The static source index read path requires C `size_t` fields compatible with the CoD byte offsets. Reject unsupported layouts. Keep compiler-private result and `here` carriers inside compiler-generated target code. Provider-facing stubs use the generated C header and call distinct handwritten hooks. The seed and booter compare equivalent generated declarations and adaptation contracts for the same target.

Build the existing seed first. Check the interface declarations and emit the provider header and C stub source as one generation step. Compile that output with the handwritten C provider and launcher. Emit the booter and its C-callable entry and callback thunks through the seed's native path, then link the final executable. The generation step depends on the already built seed and resolves imports and signatures without invoking providers. A checked-in generated baseline could also break a build cycle, with an additional stale-output check on every signature change. An explicit provider-origin owner makes the source and region lifetimes reviewable. A profile restricted to caller-borrowed owners would require a separate source acquisition convention for the initial C backing.

Acceptance compares both generators' contracts and builds the mapped-source, region and LLVM workflow through the generated boundary. Run the resulting deferred-release executable, report an invalid source directly, and exercise failed source/scratch acquisition and partial LLVM setup with exact reverse release. Verify that mapped source remains live through line lookup and that the working views have the same actual owner through LLVM consumption. Reject a writable source view, wrong owner, callback escape, invalid extent and incompatible native layout before provider use. Trace a checked callback failure through the original CoD throw and cleanup path. The handwritten providers are trusted C and retain responsibility for their own resource protocols. Dynamic loading, a general foreign interface, and a permanent ABI remain outside this profile. The concrete build graph requires separate build-configuration authorization.

## Select APIs by application demand

Introduce bindings when a concrete compiler operation needs them: context/module construction, type/value/block construction, verification, selected optimization and output. Tokenization and parsing have no inherent LLVM dependency. Avoid generating an entire API binding surface before the application uses it.

A small provider boundary can expose native services while CoD owns compiler algorithms and regional semantic data. [H-352](../../../CHANGELOG.md#h-352) places third-party intrinsic extension on an SPI planning path; it does not yet define registration, loading, provider selection or the detailed ABI. Prefer source-visible module selection over an implicitly edited global registry. The native shim should not become a second implementation of CoD's parser or analyzer.

## Opaque handles and separate lifetimes

Investigate mapping native pointers to distinct CoD handle types. A pointer-backed representation does not itself require a global handle table, RTTI or reference counting. Nor does being pointer-sized make a handle an unrestricted persistent bits payload. Keep three responsibilities explicit:

1. The owner of the CoD slot containing the handle: a lexical scope or, if permitted by a separately selected storage contract, a region.
2. The lifetime and release responsibility of the native resource denoted by that handle.
3. Dependencies among native resources and borrowed handles whose validity ends with their parent resource.

A live slot is not proof that its native target is live. Copying a handle must not silently duplicate release responsibility. Dynamic bindings can expose resources within a scope without transferring their destruction responsibility on every shadow. Acquisition/release and dynamic access are related operations, not the same mechanism.

Native pointer values are process-local. They must not be persisted as usable LLVM resources in DBMR caches. Whether temporary regional storage may hold a specially classified handle, and how save/relocation checks would exclude it, is unresolved; the generic `owner` annotation alone grants no such permission. The compiler application's persistent representations remain independent of live LLVM objects.

## Initialization through an output reference

[D-22’s init contract](../../../design/ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing) exposes a caller-selected place and a checked initialization obligation. Plain init n:T in O and init ref n:T in O retain ordinary selected-target reference semantics with initialization responsibility. Explicit init ref var/ptr var supplies caller reference-slot declared place aliasing; init val var supplies a caller bits place. Binding a reference slot does not itself initialize its referent, and init alone adds no reference indirection. Selecting the native handle category must precede choosing the applicable parameter and local declaration form; a pointer-sized native handle is not automatically a bits value, regional object or CoD reference.

The binding proposal writes the acquired handle into an eligible caller place and returns a zero-argument cleanup callable. The storage place’s actual owner, the handle’s native-resource lifetime and the cleanup responsibility are distinct. The call must not read an uninitialized input payload. Normal completion establishes the declared initialization obligation; partial native acquisition and exceptional completion require their own release proof. Provider code must uphold the checked contract rather than using the annotation as evidence.

Assuming ctx has been allocated under a separately qualified handle-storage contract, the selected defer timing permits:

```cod
# Application sketch; handle storage and native binding support remain pending.
defer llvm_context_init(ctx)();
```

Public init-place writes can be observed during the call and cleanup. They are not the compiler-private pending return slot: the returned cleanup callable is delivered only when the invocation completes successfully. A failing call supplies no blanket output-initialization postcondition or rollback. Subsequent reads need independently valid evidence, and escaped native resources need explicit cleanup. Runtime target sharing between arguments, replacement of an already initialized native resource and complete provider/modifier proof remain application review items.

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
