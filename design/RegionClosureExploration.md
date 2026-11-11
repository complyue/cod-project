# Structural regions and regional closures: experimental comparison

Design revision: 6 · Recorded: 2026-11-10 · Status: authorized experimental scope; final language selection pending.

## Design authority

- Registry: [D-18](DesignAuthorityRegistry.md#d-18), revision 6, all sections; [H-324](DesignAuthorityRegistry.md#h-324)/[H-325](DesignAuthorityRegistry.md#h-325)/[K-452](DesignAuthorityRegistry.md#k-452).
- Value path: D-18/P-293/P-294 to C-65/C-73/C-74 explores practical, explicit storage and lifetime contracts serving charter §§1–3 through C-65/C-39. H-324 explicitly authorizes autonomous alternative design, branch implementation and empirical comparison within repository principles; this does not approve every experimental mechanism as the final language.
- Dependencies: D-14 owners/field kinds, D-13 storage policy, D-10 callables/effects, D-17 failures/scopes and D-16 fixed bits/future fractions. D-01 relocation/allocator invariants and D-15 module-world/TLS separation remain constraints.
- Non-goals: new RTTI, bootstrap/toolchain work, SNN learning/model selection, per-object ownership, hidden heap closures, cross-region stored pointers, copying/moving regional objects or merging a final design without review.
- Acceptance: native examples and meaningful rejection cases for at least two alternatives, full applicable language regression checks, targeted lifetime/relocation evidence, compact reproducible artifacts and a comparison of practical source use, complexity and limitations. Stop for human review at or before 48 active work hours; exclude waiting, with a conservative allowance for unmeasured baseline preparation.

## Baseline and branch roles

Baseline commit: `96468d9`. The compiler and 338 source hashes match the [colon/nullable acceptance](../tests/cod/language/colon-nullable-validation.json), including 254/254 language cases and native regional, exception, ownership and composition checks.

| Branch | Experimental question |
| --- | --- |
| `codex/region-closure-common` | Can field-subset region requirements preserve actual layouts, owners and existing nominal field identities with a small compiler model? |
| `codex/region-closure-owners` | How far can closures go by retaining explicit region access and using field paths, without capturing ordinary lexical binding cells? |
| `codex/region-closure-slots` | Does explicit selection of regional variable slots materially improve source ergonomics enough to justify its additional place/capture semantics? |

Branches are developed sequentially in the existing SSD checkout using its active build tree. Each compiler is built with `cmake --build ./build`; no configure, clean, build symlink, new bootstrap or NFS copying is part of the experiment. Shared foundations may be committed before alternatives diverge. Alternative branches are evidence, not simultaneous backends or permanent compatibility modes.

## Common contract to exercise

A region definition describes a product of named val/ref/ptr fields. A consumer's requirement is a compatible field subset, not a byte prefix or a different owner. Concrete field layouts remain available to WPO. Nominal data/UoM identities inside fields remain exact. A region instance remains distinct from every other instance satisfying the same shape.

A regional val slot contains bits. Regional ref/ptr slots hold same-region references; capturing such a slot observes later rebinding. Capturing a target value is different. Null does not erase the regional slot's known owner. Readability, access, initialization and checked-NPE obligations remain in force. Captures borrow access without extending lifetime; automatic lexical-cell boxing/promotion is excluded.

Owner-based closures may retain an actual region and project its fields at each use. Slot-based closures explicitly identify stable regional storage, carrying the same owner proof. Captured environment representation must follow ordinary storage/lifetime rules; no implicit host-heap allocation is permitted. The first useful WPO subset may keep callable identity and captures in compiler metadata, but any restrictions on returning, selecting, storing or reusing callbacks must be tested and reported, not hidden by passing a single inline example.

Functions retain their explicit-memory/effect boundary. Procedure lexical state is separate from perform's captured dynamic values and behave/direct calls' current bindings. New syntax must respect single-colon annotations, reserved literals/keywords, assignment versus transfer, and planned fractions. The confirmed double-brace scope boundary belongs to any scope/callback work that actually requires it; unrelated syntax changes are not a reason to expand this experiment.

## Evidence and comparison

Use a small shared matrix: two concrete layouts satisfying one subset, mutation through a helper, val and ref/ptr slot updates observed by a callback, null before/after rebinding, nested/cross-module calls, valid external-owner return, invalid ended/sent owner, lexical-cell rejection and read-only rejection. Add a repeated A/B state-selection or typed-pool example to expose an actual usability difference; do not tune SNN transition equations.

Measure changed compiler logic, semantic cases/special paths, LLVM verification and representative native behavior. Retain exact compiler/source identities with results. Compare emitted code or timing only where it answers a practical design question; no micro-optimization target is implied. Negative findings and unimplemented cases are part of the result.

Progress and active-time checkpoints are recorded in the exploration evidence directory. Final review must distinguish measured active work from waiting and estimates, name each branch/commit, and identify which decisions the user would need to adopt. A promising result does not authorize merging its language contract into main.

## Shared structural foundation evidence

The [structural validation](../tests/cod/language/exploration/structural-validation.json) records 259/259 passing language checks, including different concrete layouts used through one cross-module field requirement, owner inference, readonly rejection, field-kind/type rejection and unique-most-specific versus ambiguous shape dispatch. Compilation uses the existing native LLVM path and unchanged regional runtime.

Requirements act as lower bounds; WPO retains the concrete layout and type for nested dispatch. They do not currently hide extra fields from a demanded helper body. Treating a requirement as a restricted access view is a separate comparison dimension, not a runtime layout cast. Existing data-root syntax remains for baseline comparison, and this checkpoint provides no closure implementation evidence.

## A-01: Owner-based callable captures

Named nested and anonymous func/proc definitions may retain actual regional access and compile-time-known callable values. Ordinary lexical val/ref/ptr capture rejects. A body uses explicit paths such as state.current, reloading each field when called. A returned callback retains its actual region dependency without retaining its defining call scope or acquiring a second owner. Func callbacks use explicit memory and cannot invoke procedures or access dynamic effects. Proc callbacks preserve the existing lexical/current and perform/captured dynamic distinction.

```text
function reader(region state:Counter) -> func[region state]()->const u64 {
    func () -> const u64 { state.ticks }
}
```

The initial [owner validation](../tests/cod/language/exploration/owners-validation.json) records 266/266 language checks, native regional/exception/ownership checks, a 64-module nested-return composition and 10,000 two-frame updates with bounded occupation. Captures see later field writes and pointer rebinding, survive permitted local ownership transfer and reject expired regions or readonly mutation. Mixin preserves nested callable kind and anonymous identity.

This branch keeps callable identity and captured access in WPO metadata. It does not provide a stored runtime closure descriptor. A control-flow join of different callable/capture values rejects explicitly; a single callable may still follow runtime-selected regional fields. The explicit-block/defer/contextual-callback increment remains separate. A field requirement is a minimum, not a restricted view hiding concrete fields.

An owner capture follows an entire field path on every call. Thus a callback using state.link.current observes rebinding of state.link as well as current. Whether a library instead needs to keep the original current slot is the practical question for B-01.


## B-01: Explicit regional-slot and value captures

An explicit list distinguishes a retained regional variable slot from a copied bits value. It can also retain a borrowed region or a statically known callable. The list evaluates left-to-right in the defining context; names in the list belong to the body, not to subsequent capture initializers. Capture names must be distinct from parameters and one another.

```text
region Counter { val ticks:u64; }
function reader(region state:Counter) -> func[region state]()->const u64 {
    func [ticks = state.ticks] () -> const u64 { ticks }
}
function shifted(region state:Counter, delta:u64) -> func[region state]()->const u64 {
    func [ticks = state.ticks, delta <- delta] () -> const u64 {
        ticks + delta
    }
}
```

`=` selects an existing regional field/array-element slot, an already captured regional slot, a borrowed region or a statically known callable. Selecting an ordinary lexical ref/ptr binding rejects even if its current target lies inside a region. `<-` copies bits into an immutable capture; it neither borrows the source storage nor transfers a region owner. Thus `shifted` may return after its local parameter storage ends. These are experimental closure-list rules, not a change to ordinary assignment/transfer.

A captured val slot observes later bits writes. A captured ref/ptr slot observes later rebinding and retains its access/nullability contract. Its name reloads the slot when used. An ordinary local alias initialized from that name borrows the current target and has its own binding: rebinding the alias does not rebind the captured slot. A nested closure can retain the slot by name without converting it into a lexical-cell capture. Reference swaps support regional reference slots as destinations, including overloaded swaps that delegate to the intrinsic; both directions are validated before either write.

```text
func [selected = state.link.selected] () -> const i64
    throws NullPointerException { selected.value }
func [state = state] () -> const i64
    throws NullPointerException { state.link.selected.value }
```

The first callback retains the selected slot inside the original link. The second follows the current link each time. Rebinding `state.link` therefore affects only the second; changing the original link's selected slot affects the first. Both depend on the actual owning region remaining accessible. A slot capture does not retain a logical pool item after a library recycles that slot; item identity/reuse remains a library contract.

Named lexical callable expressions and anonymous callables share this capture contract. Module function method declarations cannot carry lexical capture lists; an explicit callable value supplies that role.

Explicit capture removes the free-name/scope walker used by A-01. Regional-slot identity belongs to the compiler's local binding model, not to every value loaded from that slot. Captures use existing typed storage views and owner checks; the native runtime, LLVM emitter and relative-pointer encoding require no changes. Procedure dynamic captures remain separate: `perform` restores captured symbolic values while observing current regional contents, and `behave` uses current symbolic bindings.

The initial [slot validation](../tests/cod/language/exploration/slots-validation.json) records 273/273 passing language checks, the branch's exact compiler/source identities and native evidence. B-01 retains A-01's WPO-only callable limitation: this is not a runtime closure record that can be stored in a region, serialized or selected from heterogeneous runtime callbacks. Explicit double-brace scopes, defer and contextual block-to-callable syntax are not implemented by this experiment.

## Review criteria

Both alternatives preserve concrete field layouts through structural requirements and can return callbacks over a live external region. Neither extends a region's lifetime, promotes lexical cells or requires RTTI. A-01 offers shorter bodies for code that naturally names its owner; B-01 makes captures visible and adds stable-slot and copied-bits choices while also supporting explicit owner paths.

The source of a dependency is an architectural distinction, not merely syntax: owning a region, borrowing a region, retaining a slot and loading that slot's current target are different operations. The experiments keep those distinctions in compiler metadata. A final design should retain this separation even if the capture spelling changes.

Explicit owner capture is the clearest ordinary idiom in the current comparison. Stable-slot capture remains useful, but its capture-list assignment semantics need to be reconciled with ordinary variable aliasing before final adoption. This recommendation is not language adoption. Region field requirements remain minimum structural requirements over concrete WPO types; strict field-visibility erasure and stored runtime closures need separate design. No additional per-object lifecycle system or generic closure heap is justified by these results.


### Measured comparison

| Criterion | A-01: owner-based | B-01: explicit capture |
| --- | --- | --- |
| Returned callback with a live external owner | Native pass | Native pass |
| Later pointer-slot rebinding | Follows owner path | Captured slot reloads; explicit owner paths also available |
| Replacement of an intermediate path link | Follows replacement | User chooses fixed leaf slot or full path |
| Independent copied local bits | Rejected as lexical capture | Explicit immutable `<-` capture |
| Hidden lexical storage promotion | None | None |
| Capture discovery | Free-name/scope traversal | Explicit list plus regional-slot binding metadata |
| Compiler diff from baseline, frontend | +137 / −18 lines | +161 / −34 lines |
| Compiler diff from baseline, syntax | +35 / −15 lines | +48 / −15 lines |
| LLVM emitter / native runtime changes | None | None |
| 64-module returned callback LLVM artifact | 87,929 bytes | 87,929 bytes |
| 10,000 frame iterations | Bounded region occupation, one acquisition | Bounded region occupation, one acquisition |

Line counts describe review surface, not a complexity score. Artifact sizes do not establish throughput or optimization quality. The repeated-frame probe verifies reuse and native behavior without implementing SNN model equations. Exact [comparison evidence](../tests/cod/language/exploration/comparison.json) and each branch's validation record identify the sources used.


## C-01: Named owner contracts on callable types

H-325 identifies a missing source-level anchor: an unqualified `func()->const u64` does not explain which regional owner must remain available. This experimental increment gives both capture alternatives the same formal boundary, without selecting either capture mechanism or merging it into main.

```text
function reader(region state:Counter)
    -> func[region state]()->const u64 {
    func [ticks = state.ticks] () -> const u64 { ticks }
}
function relay(action:func[region source]()->const u64)
    -> func[region source]()->const u64
    where {region source:Counter} {
    action
}
```

The first body is B-01; A-01 uses `func () -> const u64 { state.ticks }`. The contract is identical. `state` names a concrete region supplied to the factory. `source` is a formal regional owner inferred from the supplied callable, with its structural requirement declared by `where`. The declaration exposes the dependency to callers and provides a diagnostic anchor; it does not allocate a closure in that region or extend ownership. The callable result's scalar return remains independent bits.

In a callable **type**, `[region r]` declares the environment's owner dependency. In a callable **expression**, the existing B-01 `[name = expression]` supplies captured access. Parameter parentheses remain invocation-time arguments. Owner names are compile-time roles; distinct names may refer to one actual region, while equal root shape never makes two region instances identical.

This bounded prototype checks an exact set of retained lexical region owners at typed boundaries. A bare callable type requires no lexical region dependency. Returning or supplying a region-dependent callable under that type rejects with an instruction to declare the owners. A named but different actual region also rejects, even if its fields and type match. Region constraints and readonly access survive forwarding through callbacks and generic helpers; local callable annotations are checked as well as parameters/results.

A callable retaining multiple regions can declare `[region left, region right]`. Explicit region arguments establish those roles. A single unknown owner can be inferred from one remaining dependency; several unknown roles require explicit region arguments rather than guessing an ordering. Relaxing exact dependency sets to an upper-bound contract, and richer multi-owner inference, remain possible alternatives for review.

The compiler carries actual region identities and source owner names in compile-time type/provenance metadata. Expired-owner errors name the callable contract and identify the region acquisition. No runtime type tags, closure heap or regional pointer layout changes are introduced. A signature's dependency set and a callable's own storage owner remain separate.

TLS procedure declarations currently have no syntax anchoring a lexical closure environment's region owners. Binding such a closure to a bare symbolic procedure declaration rejects explicitly instead of erasing its dependencies. Procedures using existing dynamic region symbols retain their established perform/behave rules. This is a visible experimental boundary, not a redesign of TLS symbol ownership or dynamic capture.

Acceptance checks both branches with same-root/different-instance rejection, undeclared owners, omitted owner contracts, expired owners, readonly inference, generic forwarding, overload selection and explicit multi-region composition. The shared compiler logic will be carried to both branches; tested source examples and diagnostics, rather than a presumed final spelling, determine the next review.

C-01 validation records are separate from the initial A-01/B-01 checkpoints. Its positive examples include a typed callback factory returning another owner-anchored callback; its rejection cases also check local annotations, so `const ref` cannot bypass the owner contract. Branch-join diagnostics identify the acquisition sites of retained regions even when both alternatives display the same source-level owner alias.

The [explicit-slot owner-contract checkpoint](../tests/cod/language/exploration/slots-owner-contract-validation.json) passes 280/280 language cases plus native ownership/exception/relocation and composition checks.


### Same contract across both capture alternatives

```text
# A-01: owner-based body
function reader(region state:Counter) -> func[region state]()->const u64 {
    func () -> const u64 { state.ticks }
}
# B-01: explicitly selected regional slot
function reader(region state:Counter) -> func[region state]()->const u64 {
    func [ticks = state.ticks] () -> const u64 { ticks }
}
```

A caller reads the same declared dependency in either branch. The actual region supplied as `state` must remain live, including through wrappers and typed local bindings. The region is named at the factory boundary; it need not be passed again on every callback invocation. The different capture expressions determine how the body accesses regional storage, not whether the callable's owner dependency is visible.

The current prototypes intentionally keep both branches separate. Their common contract is useful evidence for later consolidation, but exact dependency sets, the annotation spelling and symbolic callable environments remain review choices. `main` keeps the baseline. This avoids folding the still-unsettled slot-alias semantics into an apparently settled foundation.

Single-owner callable overloads use the declared region field requirement in specificity comparisons, just as direct region parameters do. Candidate diagnostics print the `where` owner constraints. The positive contract probe places the more specific overload before the general one and requires the structurally stronger match.

The [implicit-owner contract checkpoint](../tests/cod/language/exploration/owners-owner-contract-validation.json) passes 275/275 language cases and the same native/composition checks, including root-requirement specificity.

Final C-01 comparison: A-01 passes 275/275 language checks and B-01 passes 280/280, with native ownership/exception/relocation and composition checks passing for both. The final B-01 record includes matching hashes for the shared owner validation, binding and specificity logic against owner branch d3227f2. Both preserve their original capture mechanism; no final language choice or main merge accompanies the checkpoints.
