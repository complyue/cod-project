# Callable views, value captures and declared place aliases

Design revision: 114 · Recorded: 2026-11-17 · Status: scoped callable contract; implementation boundaries recorded separately.

## Design authority

- **Registry and confirmed baseline:** [D-18](DesignAuthorityRegistry.md#d-18) is authoritative for the recorded capture/value/native-boundary scope under H-327–H-335/H-337–H-344/H-349/H-350 and C-65/C-73/C-75/P-293–P-303. H-342 qualifies dedicated complete lexical callable values, nested captures and caller destinations; H-344 permits runtime-sized scope-owned backing. H-417 requires one statically provable owner for each ref/ptr. These qualifications continue; they do not approve every descendant or physical mechanism.
- **Current review:** revision 114's [ordinary value-input implementation contract](#ordinary-value-input-implementation-contract) is **pending human review** under [D-18-VALUE-CAPTURE](DesignAuthorityRegistry.md#d-18-value-capture)/P-485–P-487. H-553 already confirms independent snapshots and invocation locals. The proposed delegation and scoped value path cover ordinary scalar/intrinsic-exception and complete callable values through their existing value contracts; the decision is whether to qualify that implementation contract, not whether to reopen the selected source semantics. [K-783](../CHANGELOG.md#k-783) records the review boundary and preserves all other current qualifications. No new compiler capability is claimed. [K-REPLACEMENT-EFFECTS-20261117](../CHANGELOG.md#k-replacement-effects-20261117) clarifies the existing no-rollback boundary in r114 and its acceptance cases; the human's risk-assessment question is not adoption. The scoped C-65/C-73/C-75 value rationale, invariants, exclusions and all other qualifications remain unchanged.
- **Value rationale:** C-65 keeps selected storage, access and lifetimes explicit; C-73 applies one capture model to named and anonymous callables; C-75 distinguishes a declared place from an independent value. Through P-294/P-295/P-297/P-298 and the candidate bounded P-485 path, these serve charter §§2/3.2–3.3/4.3/6.3/8/12: shared program descriptions must not create shared mutable invocation state, source identities must retain their meaning, and native execution must preserve checked ownership and observable behavior. P-299–P-303 continues the independently qualified callable/native boundary. Dependency edges alone grant no qualification.
- **Governing invariants and acceptance:** preserve exact types and declared access, initialized value reads, definition-bound identities and materialization timing, actual borrowed owners, independent complete nested values, recursion, caller destinations and normal/exceptional cleanup. Exercise both selection paths, snapshots and local writes, owner/access/lifetime rejection, nullable invocation, construction/invocation cost and compilation scale. The bounded review below specifies additional acceptance. No hidden shared cells, lexical-frame promotion, source-lifetime extension, regional callable payload, general object copying/moving, implicit graph transfer or independently managed user heap lifetime. D-01 relocation, D-15 module/TLS separation and D-16 remain unchanged.
- **Separate source and mechanism scopes:** H-553–H-557 records the value/explicit-lifetime model; [D-18-BORROW](DesignAuthorityRegistry.md#d-18-borrow)/P-488–P-490 retains separately pending new lexical-reference and declared-place capture proofs/transport. H-327/H-329's declared-place meaning remains binding. [D-18-ALIAS-TERMS](DesignAuthorityRegistry.md#d-18-alias-terms)/H-558 requires the glossary below, without exclusivity or lifetime privileges. [D-18-LEXICAL-STORAGE](DesignAuthorityRegistry.md#d-18-lexical-storage)/H-563 excludes callable views from regions; H-567/H-568 preserves dedicated callable construction separately from bits transfer and explicit graph transfer. Callable-bearing ADTs/anonymous data remain in D-22's review.
- **Dependency contracts:** D-14 owns storage, permissions and initialization; D-10 owns current/captured symbolic behavior and functions' effect exclusion; D-17 owns completion and declared checked obligations; D-23/D-24 owns established results and independent inference; D-21/D-26 owns actual-type selection and finite native coverage. H-366–H-369/P-327 preserves callable parameter access alongside actual input identity. H-481 fixes ordinary definition lookup and fresh materialization; stored values retain their selected environments. H-482/H-561 supplies selected reference-symbol source/owner rules, while full proof/physical/value paths and runtime support remain separately pending. H-457's named-argument boundary, H-459–H-462's type-bound defaults/complete origin coverage and D18-Q-result-discrimination retain their own parser, evidence and representation boundaries. None is qualified by this review.
- **Current implementation and continuity:** [K-718](../CHANGELOG.md#k-718) and [K-746](../CHANGELOG.md#k-746) continue the recorded receiving, evidence-retention and capture/value scope; [K-782](../CHANGELOG.md#k-782) rejects declared-place captures where lifetime/transport proof is missing, preserving explicit independent callable construction. This is a compiler coverage limit, not a language prohibition. The [source capability reference](../tests/cod/language/README.md) owns current supported paths, diagnostics and validation commands. A supported-source reference experiment informs feasibility only; neither experiments nor passing fixtures supply human confirmation or broader qualification.

## Language model: no closure abstraction

CoD does not define a closure abstraction. It defines callable definitions, materialized callable views, value captures and explicit owner-bounded borrows. A broad computer-science usage may call any callable with an environment a closure; that terminology must not introduce an additional CoD mechanism. In particular there is no closure-owned mutable variable cell, shared retained invocation frame or automatic extension of a lexical variable's lifetime.

Compiler implementations must preserve source-visible distinctions: independent value storage, a reference's target, an explicit var alias's selected place, and a symbolic identity's dynamically installed value. Natural lexical nesting supplies ordinary lifetimes. Access that exceeds that nesting must be expressed through the formally specified, statically checked dynamic-scope/owner contracts; capture, heap allocation and code sharing are not escape hatches. Existing explicit region ownership transfer remains its own contract. See the [vision principle](GlobalVariablesDoneRight.md#explicit-lifetimes-without-closures).

## Callable views have lexical storage

H-563 makes the placement boundary explicit: a CoD callable-view value, including its complete captured environment, belongs to lexical-scope storage and must not be stored in a region. Lexical storage may be optimized into registers, stack storage or scope-owned backing; those implementation choices do not change its logical lifetime. The prohibition applies even to an empty-capture view or a representation that happens to fit in a machine word.

A regional aggregate or array cannot evade this boundary by embedding a callable-view value directly, through another compound value or through a generic instantiation. Same-owner field references cannot make a lexical callable target regional. Storage eligibility is a type/owner contract, not a special permission inferred because one runtime value is null or has no captures. H-565/H-566 reviews whether callable-bearing ADTs should be admitted specifically in lexical storage; this does not reopen the regional prohibition or automatically qualify such aggregate fields.

Procedure-valued symbolic bindings keep their established selection, capture and scoped replay behavior. Their runtime binding records are not a regional home for the complete view/environment and cannot extend its lexical or borrowed-owner lifetimes. A module lifetime is a full lexical scope. Likewise, program/type/definition descriptions in regional semantic graphs are not executable callable-view values; a definition mention materializes a view only where the callable contract requires one.

Acceptance must reject regional construction/storage of a callable view, including empty captures, nested carriers and generic instances. Existing lexical callable parameters/results/captures retain their independent complete environments and checked external borrows. No new regional carrier, graph serialization permission or lifetime extension follows.

## Copying, moving and graph transfer

H-567/H-568 fixes the source-language mental model: **CoD has no C++-style object copying/moving semantics. Only pure bits values admit whole-payload transfer; other objects do not acquire that operation because they are lexical, relocatable or initialized.** CoD does not infer copy/move constructors, source invalidation, hidden graph duplication or lifetime transfer from assignment, capture, grouping or return syntax.

Keep the actual operations distinct:

- Bits transfer reads an initialized valid payload and initializes/writes a destination under its declared rules; it does not transfer object identity or consume the source.
- Reference binding selects an existing target and preserves its actual owner/access. It does not duplicate the pointee or make a declared place alias of the source reference slot.
- Ownership transfer changes responsibility for an existing region resource under the consuming contract; it does not move or copy regional objects.
- Callable-view construction, capture, parameter preparation and result construction follow their dedicated complete-environment contracts. Their existing independent-value behavior remains; it is not a general whole-object copying permission for callable-bearing ADTs. References to callable “copies” elsewhere in this contract describe that dedicated semantic construction, not C++ copy construction or an opaque byte-copy license.
- Graph transfer is explicit construction/mapping into a destination graph according to the applicable library contract. It preserves or transforms sharing/cycles as that contract specifies; CoD supplies no automatic default graph copier or mover.

Compiler-level loads/stores, byte copying, register movement and native C++ utility operations may implement admitted behavior. They cannot establish source-level operation eligibility. In particular, relocating backing while preserving the same logical owner is distinct from graph transfer to another owner. [D-22](ConstructionAndAcquisition.md#storage-eligibility-and-payload-transfer) records aggregate consequences; ordinary capture/cleanup/owner obligations remain unchanged.

## Structural regions

`region` declares a product of named fields. A consumer can require a compatible field subset; WPO retains the concrete region layout, including additional fields and their actual offsets. Field names, val/ref/ptr kinds and data/UoM identities remain significant. Structural compatibility neither creates an owner nor converts one actual region into another. A requirement is a lower bound, not an access view that hides extra concrete fields.

Unmodified/val fields store bits directly; ref/ptr fields store same-owner references. An explicit var alias captures the selected field place: reads observe its later writes or rebinding under ordinary access/initialization and checked-NPE obligations. An ordinary value binding copied from that field instead captures its selected value, as specified below. Region-level allocation/release remains intrinsic; free lists, pools and item reuse remain library policies. No physical allocator or relative-pointer encoding change follows.

## Callable completion

H-361/H-362 and [D-17](ExceptionControl.md) require explicit `return` for values from proc/func brace bodies. Return from deferred catch/finally targets the enclosing callable, replaces a pending completion and continues remaining cleanup. Missing a required return is unchecked, with static diagnostics where possible; void callables may fall through. H-363 records pure declarations with implicit body results while deferring purity implementation. Implementation availability for explicit returns, missing-return delivery and deferred execution is maintained in the source capability reference. These source changes preserve complete values, caller result storage and owner/access checks.

## Alias terminology

**Declared place aliasing is explicit; runtime target aliasing is permitted without a pairwise alias declaration. CoD introduces no implicit variable-slot aliasing.** These are different semantic layers:

| Term | Meaning | Static/runtime boundary |
| --- | --- | --- |
| **Declared place aliasing** | `var a -> slot` declares another name for the selected existing place, without a new independently rebindable holder. | The alias operation and its access/owner contract are checked statically. The particular place can be selected at runtime. |
| **Runtime target aliasing** (object-identity aliasing) | Independent reference bindings happen to select the same live object/target. | No pairwise alias declaration is required. The compiler may prove equality/disjointness in particular cases, or retain possible overlap. |
| **Name/identity aliasing** | A module/type/symbol/callable-definition spelling resolves to an existing declaration identity. | Compile-time name resolution; creates neither a runtime binding slot nor a shared object target by itself. |

**Mandatory terminology for compiler development.** Every use of the alias concept must identify the applicable kind. Use the qualified terms above in design discussions, documentation, source comments, diagnostics and new/refactored interface or data names. Do not use unqualified alias/aliasing as an umbrella category, and do not use “compile-time aliasing” or “runtime aliasing” without identifying the relevant place, target or declaration-identity relation. A discussion involving more than one relation must name each separately. For example, write “capture of a declared place alias” or “runtime target-alias analysis”; “capture alias” and an unspecified “alias analysis” do not identify the contract. Broader storage-overlap analyses must state which overlap relation they establish. Existing implementation names are not evidence that these categories have the same semantics.

“Compile-time aliasing” is shorthand for declared alias semantics, not a promise of compile-time-known addresses or a purely compile-time operation. For `var a -> items[i]`, runtime i may select the place once when the declaration executes. Two explicit aliases `var a -> items[i]` and `var b -> items[j]` can themselves denote the same place when i equals j; their declarations do not prove disjointness. Conversely, target sharing from `ref q = p` can be statically obvious without turning q into a place alias of p. Prefer the qualified terms above when either ambiguity matters. Compile-time module/type/symbol name aliases are a separate identity-resolution operation, not storage aliases.

Runtime target aliasing need not be explicitly declared or individually known during compilation. It remains governed by the language's formal reference, owner, access and lifetime semantics: “not a declared alias relation” does not mean unspecified behavior. Two parameters may receive the same reference, and two checked graph paths may reach the same object. Unique-owner evidence is not unique-target or exclusive-access evidence. Compiler optimizations must preserve permitted overlap, including through readonly paths that coexist with writable ones; this principle supplies no disjoint-memory assumption. Static inference of permitted annotations and capture lists remains available.

```cod
proc observe(ref a:u64 in O, ref b:u64 in O) -> u64 where {owner O} {
    a <- 1u64;
    b <- 2u64;
    return a;
}
# For a valid writable p, observe(p, p) returns 2u64.
# a and b have independent bindings and share p's target.
```

<a id="ordinary-aliases"></a>
## Declared place aliases

```text
[const] [val | ref | ptr] var name [: [const] Type] [in owner] -> lvalue
```

A declared place alias selects existing storage once. It creates no holder, allocation, payload copy or ownership transfer. Omitted kind is inferred; an explicit prefix must match exactly. A ptr field remains ptr even when currently non-null. An optional type annotation checks the selected view without numeric conversion. `:const` is also valid without an explicit type.

`in owner` checks the storage's actual owner. For `val var`, this annotates the aliased place, not an owner-qualified independent val type; the ban on `val T in O` remains. Declaration-leading `const` restricts the selected binding/storage through the alias; type-leading `const` restricts its target and every derived access path. These dimensions are independent under D-14; other writable aliases can still change the storage. Existing `=`, `<-`, `<=>` and `<->` rules apply through either name. Selecting an array element or field path fixes that storage location, rather than re-evaluating the selection later.

Declared place aliases can name ordinary variables, regional fields and addressable array elements. Whole-region holders, TLS bindings, temporaries and non-addressable logical container elements do not supply this alias form. Selecting a field through a dynamic region resource does not alias its TLS binding. Capturing an explicit alias preserves its once-selected place and the place's actual owner dependency; it creates neither a new shared slot nor a longer lifetime. Copying its value into an ordinary binding before capture explicitly requests value semantics instead. A lexical-place alias is admitted only when every use is statically within that place's owner lifetime.

### Compiler rule: target sharing is not binding-slot aliasing

Use **place alias** for `var a -> slot`: a second name for the same existing storage, with no independently rebindable alias holder. Use **borrowed target** for an ordinary ref/ptr binding. Two references may select the same target while their binding slots remain independent. Module/type/symbol name aliases are compile-time identity mappings, not this storage operation. Do not infer place aliasing merely from equal targets, identical types, a borrowed parameter, a captured reference, or an implementation that temporarily shares a native address.

```cod
proc distinguish(ref x:u64 in O, ref y:u64 in O) where {owner O} {
    ref p:u64 in O = x;
    ref q:u64 in O = p;  # Independent binding, same initial target.
    ref var a -> p;       # Explicit alias of p's binding slot.
    a = y;               # Rebinds p; q still selects x.
    q <- 7u64;           # Writes x's payload, not p's binding.
    a <- 9u64;           # Writes p's current target, y.
}
```

If x and y happen to select the same target, the writes naturally alias that target; they still do not merge p and q's slots. `a = y` does not retarget the alias a to y's slot: a remains a name for p's slot. `var b -> a` denotes that same place, without adding a proxy chain. For a val place, `<-` writes that place and builtin `=` has no reference slot to rebind. Alias const restricts access through the alias, not other names. An alias type assertion cannot convert the selected storage, alter ptr/ref mode or upgrade permissions.

Selecting `var element -> items[index]` evaluates the selection once. Later index changes or rebinding items do not select a different element. Backing movement follows the selected owner's current resolution information; logical target removal/reuse still needs the container's validity contract. An alias neither pins native addresses nor extends storage lifetime. It need not read a payload merely to name an admitted uninitialized place; later reads/writes retain ordinary definite-initialization rules.

Ordinary parameter passing and result reception obey their declared value/borrow contracts. Passing a var alias does not automatically pass its binding slot as an out parameter; assignment-family destination parameters expose slots only under their explicit D-14 contract. Builtin assignment results expose target views, never the lhs slot. Callable value copies retain independent complete values; sharing immutable code or optimized backing must preserve that independence. Capture of an explicit alias retains the place, while ordinary ref/ptr capture retains the selected target. These distinctions must survive inference, generic specialization, inlining and optimization. No runtime alias registry or ban on legitimate target sharing follows.

**Two owner roles matter for reference-slot aliases.** In `ref p:T in O`, O owns the target; p's binding slot ordinarily belongs to the current lexical invocation. `var a -> p` names that local slot, so retaining a requires the slot owner as well as the target dependencies of its uses. An alias annotation `var a in O -> p` cannot relabel that local slot as region O storage. A callable capturing ordinary p may outlive the helper when O belongs to a still-live caller; a callable capturing a may not outlive the helper's slot. An alias directly selecting a field in O instead depends on that field's actual owner, irrespective of where the alias name was declared. This prevents an explicit alias from laundering a short-lived binding through a long-lived referent.

## Callable owners and captures

| Syntax | Meaning |
| --- | --- |
| `where {region state:Counter}` | Declare a formal regional owner and its required field shape. |
| `func()->const u64 with {region state}` | Associate a callable with that actual owner. |
| `func [ticks] () -> const u64 { return ticks; }` | Declare the complete body capture list. |
| `func () -> const u64 { return ticks; }` | Infer required body captures. |

```text
data Stream { id:u64; }
region Counter { ticks:u64; }
function reader(ref stream:Stream in state)
    where {region state:Counter}
    -> func()->const u64 with {region state} {
    val var ticks:u64 in state -> state.ticks;
    return func [ticks] () -> const u64 { return ticks; };
}
```

The stream argument supplies its actual region owner; a separate region parameter is unnecessary. The declared place alias names the regional scalar place, so the callback can outlive the factory call while that region remains live and observes later writes to ticks. Its local spelling creates no factory-local holder to retain. Named and anonymous functions/procedures share these capture rules.

Omitted brackets infer required variables. An explicit list must include every required capture: missing entries error, unused entries warn. Every listed capture remains retained, even if unused. Every borrowed-owner dependency must appear in with before any unused warning is considered; an independent scalar value has no dependency on its source storage owner. Every with-listed owner must actually be captured; the list is not a conservative lifetime upper bound.

Captures distinguish independent values from explicit borrows. H-553 gives scalar val and complete callable values one snapshot/per-invocation-value rule. H-554 prohibits creating traditional closure storage with shared updatable lexical binding cells; dynamically shared variable state uses symbolic variables and an explicit statically tracked owner. H-555 also permits explicit lexical-owner references whose lifetime is statically proved. H-327/H-329's explicit place aliases retain their existing borrowed-place meaning: no implicit conversion into an ordinary value variable, and no hidden slot promotion. No capture promotes storage, freezes external targets, copies a regional graph or extends an owner lifetime. Functions remain independent of dynamic effects; procedure lexical captures remain separate from perform's captured symbolic values and behave/direct calls' current dynamic context.

Parentheses group callable types. `-> func()->T with {...} throws E` declares the returned callback's exception, while `-> (func()->T with {...}) throws E` declares the factory's exception. Unbound with names reject; with does not declare a new owner.

## Value snapshots and invocation-local captures

H-553 confirms the implicit-value-parameter model. Materializing a callable view evaluates and retains each admitted value capture as an independent snapshot. Each invocation initializes an independent local binding from that retained snapshot, with the captured name's declared type, mode and access qualifiers. Explicit var aliases instead retain their selected place under the separate alias contract; their spelling is not an ordinary value binding. Captures are hidden inputs to invocation, not additional public parameters or overload-selection evidence. There is no source-level currently-executing-callable value or writable alias to the retained capture record.

| Captured source | Retained information | Meaning inside an invocation |
| --- | --- | --- |
| Ordinary scalar val | Independent initialized bits | A fresh invocation-local val; permitted writes affect only this invocation. |
| Complete func/proc value | Complete selected callable and nested environment | An independent local callable value; replacement or writable borrowing acts on that local value. |
| Explicit ref/ptr in O | Selected target, actual owner, type/access and null state | A local reference binding to the selected target; rebinding is local, target writes obey the borrowed view. |
| Explicit var alias | Selected place and its actual owner/access contract | An alias to that same live place; writes/rebinding affect it, without a new retained binding cell. |

This adds no general by-value regional/ADT graph category. Scalar types retain their exact numeric/UoM/validity rules, with no capture conversion or canonicalization. Complete callables remain their distinct intrinsic lexical value category. Copies of borrowed references do not copy their targets. Implicit and explicit capture lists choose the same semantics; list completeness and unused-entry rules remain.

```cod
proc make_value() -> proc()->u64 {
    val v <- 1u64;
    return proc [v]()->u64 {
        val before <- v;
        v <- 2u64;
        return before;
    };
}
proc f = make_value();
f();  # 1
f();  # 1: a new invocation-local v, initialized from the saved 1.
```

The original v may end with make_value: no reference to its cell was captured. Replacing the source variable after view materialization does not change the snapshot. Changing an invocation-local v does not change that snapshot, another invocation, or another copy of the callable. Complete callable replacement follows exactly the same rule:

```cod
proc make_switch() -> proc()->u64 {
    proc current = proc()->u64 { return 1u64; };
    proc replacement = proc()->u64 { return 2u64; };
    return proc [current, replacement]()->u64 {
        val before <- current();
        current <- replacement;
        return before;
    };
}
proc s = make_switch();
s();  # 1
s();  # 1: replacement changed the first invocation's local current only.
```

**Timing and initialization.** The snapshot is taken when a runtime view is materialized, not when a named definition is registered and not when the stored view is invoked. Copying/adapting an existing view retains its snapshots; mentioning a definition follows H-481's fresh-view rule in its definition-bound context. Type-only queries neither read capture payloads nor allocate a view. Every snapshotted value must be initialized at materialization, even if the body will overwrite it before its first read or an explicit list entry is unused. Copying a ptr's owner/locator is not a pointee read: the ptr may be null or select uninitialized storage under D-14; ref retains its ordinary non-null/initialized-target requirements. Capturing a place alias also does not itself read that place's payload. Subsequent target/alias reads require ordinary definite initialization; there is no implicit init/out responsibility or exemption. A nested view materialized after local mutation snapshots that current local value; returning the new view by value can preserve that new snapshot without retaining the invocation's cell.

**Const, declared place aliases and local identity.** The captured name retains its declared readonly restrictions in the implicit parameter; capture does not silently make const val mutable. A separately declared writable local copy remains permitted under ordinary value-copy rules. A non-const val/callable capture remains writable inside the invocation. A readonly holder of the outer callable may still invoke it: mutation of invocation-local copies does not replace that holder or its retained snapshot, nor make regional target access pure. Local declared place aliases/writable borrowed parameters can refer to a mutable invocation-local capture with its actual invocation owner. A reference to that local cannot outlive that owner. This is separate from retaining an explicitly borrowed external target. Replacing a local captured callable follows the retained-destination and completed-effect boundary below: prepare the new complete value before releasing the destination's then-current value. It never updates the saved environment.

**Nested calls and recursion.** Repeated, recursive or reentrant invocations of the same stored view receive independent locals from its saved values. Ordinary definition mentions can materialize a fresh view using the current permitted lexical dependencies; no recursion-specific lookup selects a mutable saved environment. Copying a view during an invocation copies that selected view's saved values, while a new inner view can capture the invocation's current local values. Existing definition/value identity, effect permissions and scope rules remain.

**Allocation and soundness.** Use ordinary value-parameter/materialization semantics for ownership of the required storage. Retained snapshots belong to their callable value's owning storage; invocation locals belong to that invocation. Register, stack, lexical backing and permitted copy elimination are implementation choices. Internal references must remain valid until their specified last use, and normal/exceptional cleanup must release owned backing without leaking lexical storage or regions. Borrowed owners are not kept alive by copying a capture. H-553 treats OOM as an exceptional allocation outcome, not a reason to introduce readonly callable captures, persistent mutation or a weaker header-only copy. No allocation on every invocation, permanent ABI, shared mutable capture cell, regional copy/move or allocator protocol is mandated.

### Ordinary value-input implementation contract

**Review status: pending human approval of this file with the boundaries stated here.** This is the bounded implementation/value-path proposal for the selected ordinary value semantics. Approval would delegate implementation of this profile and preserve the separate statuses of the file's other scopes; it would not approve all borrow proofs, aggregates or physical protocols discussed elsewhere.

The admitted inputs are initialized ordinary scalar and intrinsic-exception values, and complete func/proc values, whose independent declaration/parameter/result behavior is already admitted by their qualified contracts. A generic instance must establish such a concrete value contract. Numeric/UoM identity and validity, exception payload/cause lifetime and complete nested callable dependencies remain exact. No new scalar type, fraction support, arbitrary compound payload transfer or outside-catalog value interpretation is introduced.

A capture's declaration identity, type, value mode and access must reach checking and lowering intact. Ordinary value capture retains a value snapshot; it does not retain the source variable's cell. Each invocation receives independent local value semantics. Inferred and explicit capture lists, named and anonymous definitions, direct calls and stored views use this same distinction. Explicit declared-place sources remain places, and ordinary ref/ptr sources remain selected references; neither can enter this profile by being flattened to a callable payload or machine word. Their wider capture mechanisms remain separately scoped. An explicit ordinary func/proc value constructed from a selected payload is a value input under its existing contract.

Materialization snapshots the current values through definition-bound identities. Ordinary view construction, stored-view passage and fresh definition mentions retain their existing timing. Direct definition calls prepare hidden capture inputs through the established ordered preparation boundary; later operand effects cannot replace an already selected input. Type-only inspection reads no capture payload and constructs no view. Saved scalar values must be initialized even when overwritten before use or explicitly retained unused. Complete callable inputs retain all independently established owner dependencies inside their environments; value independence does not extend those owners.

Inside an invocation, permitted local writes, dedicated callable replacement, local declared-place aliases and writable helper borrows affect that invocation's locals. They do not alter saved snapshots, the factory's cells or another invocation. Const/read-only access remains as declared. A reference to a local capture cannot escape the invocation owner, while a new complete callable value may retain its own admitted value snapshots. Recursive/reentrant activations remain independent and reuse suitable prepared native bodies. Current/perform symbolic contexts retain their established distinction; this profile adds no symbolic declaration, owner source or purity rule.

Storage ownership must match those lifetimes. A replaceable complete callable local must own the complete value it may release; a borrowed header into a saved nested image is insufficient. Reuse ordinary value construction, replacement and release responsibilities wherever applicable. Successfully prepared input values and locals stay valid for all admitted body, return and deferred uses. Failure during input/local preparation prevents unsafe body access and releases the discarded owned acquisitions exactly once, without rolling back completed source effects. Replacement retains the destination selected before RHS evaluation. Preparation of the incoming complete value must succeed before this operation releases or replaces the value then held in that destination. On failure, this operation performs no partial commit of its attempted replacement; it does not restore a payload saved before RHS evaluation or undo an inner replacement. On success, release the destination's current post-RHS value exactly once before installing the prepared replacement. Ordinary cleanup and effect restoration retain their own contracts. Normal return, failed return preparation, recovery and exception cleanup release every abandoned complete value without releasing a retained snapshot, leaking backing or extending a borrowed owner. This profile neither qualifies new D-17 FIFO/aggregate behavior nor changes already scoped completion rules.

For example, assume current, replacement and another are independently owned compatible callable values returning 1, 2 and 3 respectively, and might_fail may throw WorkError:

```cod
try {
    current <- {
        current <- replacement;
        might_fail();
        another
    };
} catch (e:const WorkError) {
    assert current() == 2u64;
}
```

The single braces preserve ordinary grouping and the selected destination. If might_fail throws, the successful inner replacement remains in current, subject to any subsequent ordinary cleanup/write; the outer replacement contributes no final write. If RHS evaluation succeeds but preparation of the outer complete value fails, the same no-partial-commit rule applies. If the outer operation succeeds, it releases the value then held in current (2) and installs another (3), without releasing the already replaced original (1) again. Double braces would introduce a different ordinary callable/capture boundary and are not equivalent here. This reuses [D-23's destination/effect boundary](ScalarCallResults.md#evaluation-cleanup-and-assignment-results) and [D-14's assignment evaluation rules](RegionalOwnership.md#transfer-results-and-chaining); it introduces no transactional rollback or new compound-assignment policy.

Implementation choices must satisfy the behavior rather than define it. Internal frame descriptors, checked metadata, storage layouts, copy elision and reuse of already owned private temporary backing may reuse existing facilities without a permanent ABI promise. Sharing code or immutable backing cannot create shared writable cells. Prove elision for the admitted path instead of removing mutation guards on borrowed image headers. Measure construction, dedicated value duplication and invocation separately; retain a demonstrated read-only invocation path without image/registration allocation introduced solely to observe unchanged captures. There is no prescribed allocation on every invocation, allocation count for mutable cases, payload-size maximum or new allocator protocol. OOM is an exceptional outcome with valid cleanup, not an alternative source value model.

| Acceptance | Required observation |
| --- | --- |
| Ordinary scalar and complete callable inputs | The examples above return 1 on each call; writes/replacement affect only the current invocation. All already admitted concrete input types retain their exact value contracts. |
| Materialization, source replacement and view passage | Independent views retain the values selected at their own materialization; passing/copying an existing view preserves its snapshots. Later namesakes cannot substitute another declaration. |
| Qualifiers and explicit places | Readonly captures reject forbidden writes; writable local captures admit existing operations. Declared-place captures keep their selected-place requirement and report an implementation gap until proved; explicit ordinary value construction remains distinct. |
| Local helpers, recursion and reentrancy | Writable borrowing reaches only the current local. Repeated/recursive/reentrant calls do not change saved values or each other's locals; increasing runtime depth adds no native methods. |
| Nested result and borrowed escape | An admitted newly constructed inner callable retains the current local value after the invocation ends. A reference to the invocation's local storage cannot escape that owner. Existing nested borrowed dependencies remain live by their own proofs. |
| Initial reads, ordered preparation and queries | Uninitialized snapshots reject, including unused entries and overwrite-first bodies. Prepared inputs survive later operand effects; queries cause no capture reads, construction or runtime effects. |
| Partial preparation and replacement failures | Inject failure at successive nonempty value acquisitions and replacement. Discarded owned acquisitions release once; saved snapshots stay callable, caller contexts restore and remaining cleanup follows its existing contract. Completed RHS effects are not rolled back. Cover inner replacement followed by RHS throw, inner replacement followed by outer preparation failure, and successful outer commit releasing the post-RHS destination value exactly once. |
| Native representation and cost | A stored-view path and a direct-definition path both execute in shared/general and specialized modes, with stable method inventories. Track live image/record counts and bytes through success and failure; demonstrate a read-only invocation path without additional capture-input allocations. |

The decision is whether to adopt this bounded implementation contract and its stated C-65/C-73/C-75 → charter value path. Deferring it leaves new ordinary value-input implementation unqualified; it does not select readonly captures, persistent mutation or another source model. New lexical-reference and declared-place capture proofs, owner-bearing symbolic capture, callable-bearing compounds, generic aggregate result classification, pure/TCO policy, growable-owner transport and a permanent native/persistent ABI remain outside this decision. Existing independent qualifications and explicit implementation gaps continue.

## Explicit borrowed captures and shared state

H-554 requires shared mutable captured-variable state to be explicit through dynamic-scoped symbolic variables backed by a tracked owner. D-10 supplies `ref symbol @value:u64 in @region_owner` and H-561's `ref symbol @value:u64 in O where {owner O}`. The latter may borrow a proved lexical or ordinary regional owner without first installing a region symbol; every active/saved/captured use must fit that owner's lifetime. Capturing a symbol retains its value or selected reference according to its declaration; it does not retain the mutable TLS binding cell. perform replay, behave/current invocation and scoped restoration keep their existing meanings. Writing an admitted reference target is an explicit shared-state operation. Rebinding an invocation-local captured reference or a scoped TLS effect does not rewrite another view's retained capture.

H-555 admits `in O` references when O is an explicitly established lexical owner as well as a region, provided all uses are statically proved not to outlive O. This includes a callable value retaining such a reference. No owner or ordinary lexical binding is promoted. Each ref/ptr still has one provable actual owner; several captures may have different separately identified owners. The resulting view's lifetime must satisfy every borrowed dependency. The `with {owner O}` association uses an existing generic/lexical owner; `with {region r}` retains its regional shape contract. Neither form declares a new owner.

```cod
proc reader(ref x:u64 in O) where {owner O}
    -> proc()->u64 with {owner O} {
    return proc [x]()->u64 with {owner O} { return x; };
}

proc within_owner() {
    val n <- 1u64;
    proc read = reader(n);
    n <- 2u64;
    read();  # 2: a borrowed target, not a copied scalar payload.
}
```

Returning read from reader is valid: x's owner belongs to its caller and remains live. Returning a view that borrows a local n from within_owner would reject when that n's owner ends. Non-escape therefore means not escaping the actual owner, not a blanket ban on returning callbacks from helper calls. Type adaptation, storing/copying a view, nested captures, recursion, deferred use and symbolic replay/restoration cannot erase that dependency. An unknown lifetime proof rejects conservatively; no runtime extension or hidden owning handle repairs it.

An ordinary ref/ptr snapshot fixes its selected target and initializes a local reference binding on each invocation. An explicit var alias instead retains its selected place. For an alias of a ref/ptr field, later field rebinding is observed; for an alias of a scalar field, later bits writes are observed. To freeze the scalar bits or selected reference, first initialize an ordinary val/ref/ptr binding and capture that binding. Neither reading a ref nor naming an alias authorizes graph copying, a new shared cell or lifetime extension. D-10's owner-qualified symbolic references express dynamically scoped shared variable state; ordinary object/field borrowing remains valid. Const, nullability, definite initialization, target identity and owner-plus-locator resolution remain ordinary constraints.

### Capture acceptance and remaining scope

Accept independent scalar/complete-callable snapshots, repeated calls with fresh locals, const-preserving access, mutable local replacement and writable borrowing, nested post-write value snapshots, ordinary definition-versus-view timing, and explicit regional/lexical borrows whose owner outlives every use. Reject uninitialized snapshots, readonly escalation, omitted required owner dependencies, unknown/multiple owners for one reference, view/alias/effect restoration escaping a lexical owner, and any implicit promotion to a shared mutable closure cell. Cover source-holder replacement, copies, recursion/reentrancy, active invocation protection, normal/exceptional release and no backing/region leaks. D-10 owns symbolic declaration/installation constraints; D-14 owns ordinary storage and D-28 owner/locator transport.

Current compiler coverage is recorded separately. Scalar/reference captures and mutation of invocation-local captured callable values require implementation work; existing complete-value copying alone does not establish those paths. The ordinary value-input implementation contract above is pending review; its approval and implementation are distinct. New borrowed snapshot/place transport and broader value-path qualification remain separately scoped. Future pure-callable rules, arbitrary regional graph copying and general runtime lifetime tracking are not selected here.

## Owner projection

`cb -> state` borrows the region named state in cb's visible with-type, retaining the captured permissions. A helper accepting `func()->const u64 with {region r}` projects `cb -> r`; the factory's former name state is not exposed through that parameter. Multiple names map explicitly to actual regions, independently of declaration or allocation order.

In the H-457 named-argument design, write `use((cb -> state))` for a positional projected owner, and `use((cb -> state) -> arena)` to pass that owner to named parameter arena. A bare `use(cb -> state)` instead names parameter state and supplies cb; a missing matching name rejects rather than reverting to projection. The parentheses preserve this borrowed region's category, identity and permissions; scalar grouping-to-val rules do not apply to a region. This is the selected argument-syntax direction, with compiler migration still outstanding.

Projection permits ordinary region helpers and root-field access. It cannot consume ownership, obtain writable access from a readonly association or outlive the region. Each owner association is a logical environment slot; WPO may eliminate its physical storage when the actual region is statically fixed. No separate lookup service or RTTI is required.

## Runtime callable contract

A callable view preserves its selected family dispatcher, module interpretation, view contract and required runtime environment. Several methods are permitted; actual argument types choose the applicable method. Existing native body-selection adapters are implementation infrastructure, not proof of family dispatch across abstract argument/result types. Direct method selection is valid when equivalent for all admitted alternatives. The family architecture and WPO coverage are tracked in [D-21](AheadOfTimeExecutionPreparation.md).

H-459 restricts default-expression authorship to named callable declarations. A callable parameter annotation may use `typeof named_callable` to retain the declaration's named/default contract and its original definition environment. Inline callable types and anonymous callable expressions cannot introduce fresh defaults. An ordinary structural callable type with no default contract requires its arguments; target knowledge cannot implicitly recover erased defaults. Caller-context defaults still run at the actual invocation site with definition-resolved symbolic identities. The inherited default's runtime dependencies must remain available without extending owners. H-460 permits other compatible callable identities: the named origin supplies the public names/defaults, and the target supplies its body with parameters mapped by slot. H-461 makes the default expression and its definition-context association part of the callable type contract; adapting a target does not add default-provider captures to that value. Evaluate the retained expression only when omitted, in the prescribed context and phase, with valid owner/locator and access evidence. This type/evaluation-context dependency remains distinct from the target's ordinary complete body environment; neither may outlive its permitted owners. H-462 retains all signatures of a multi-method named origin; detailed overload/default selection, exact callable-type equality/erasure and physical context evidence remain under review in D-10. No arbitrary adapter, extra owner retention, eager default snapshot or new type/provider ABI is selected.

For `f:typeof origin`, H-462 requires complete implementation of the origin's selected method-signature contract, even if the receiving body currently uses only one overload. A caller-local named family may adapt available implementations. Extra target methods are permitted only when required calls retain valid, unambiguous selection and compatible results/effects/owners; compatible generic methods may provide coverage without one source/native body per origin signature. Origin names/default expressions remain type-contract information. [D-10](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding) supplies the acceptance and local-family grammar boundary; this adds no general same-scope variable redefinition or native ABI.

Family selection uses the callable view's declared parameter target-access qualifiers together with actual argument type evidence. A readonly borrowed parameter remains readonly when supplied from writable storage; a writable borrowed parameter requires writable caller access. Value inputs receive independent local storage with the view's declared access. Actual type identity remains available for method selection before value conversion. The selected method must satisfy the view's passing, access, result, owner and exception contracts. Owner equalities retain their ordinary proof requirements.

Module method registration constructs semantic information and does not capture runtime TLS. Runtime callable materialization creates the complete environment; reinstallation retains the existing value/captures. The definition-mention rule below applies equally inside recursion. Lexical slots, named retained owners, nullable views, copies/replacement and captured/current invocation retain their qualified constraints. Per-family capture construction and abstract evidence must satisfy those constraints rather than introducing implicit owner extension or regional callable storage.

Method calls and callable-value calls separate operand preparation from invocation. Preparation retains the selected code, declared passing modes and ordered conversions. Invocation revalidates the prepared views and supplies the current procedure environment, or the explicitly requested captured environment. Symbolic builtins use the same boundary; their observable action occurs only at invocation. Native callable target protection and prepared-value cleanup retain their existing failure behavior. Scope-owned deferred registration and dispatch remain implementation work.

Target access remains separate from holder binding permissions and pure behavior. Acceptance must cover independent place-alias access permissions, readonly captures, const-holder use, copy-failure preservation and bounded invocation cost.

The ABI belongs to one statically linked CoD binary image. Compiler-generated typed adapters connect a callable view to its implementation and captured environment while preserving that implementation's module world. Native dependencies retain the system CRT contract. Cross-process code references, persisted callable views, dynamic code loading and general RTTI are outside this increment.

Callable view payloads and their environments have lexical lifetime owners. Their physical backing may be stack or heap; runtime-sized environments are supported. The owning scope must release its allocations on normal and exceptional exit. Allocation failure follows the existing unchecked OutOfMemoryError contract. They cannot appear in regional fields or arrays and are not bits payloads eligible for arbitrary transfer. Regions remain relocatable; a regional ADT and its methods provide the business-level equivalent of a functor without storing a native code address in regional data.

Non-null variables use func/proc and have val semantics; nullable references to callable payloads use ptr. Definitions establish callable identities/methods; callable variable declarations construct lexical values:

```text
func read = reader(state);
func copied = read;
copied <- another;
proc update = updater(state);
ptr optional:func()->const u64 with {region state} = null;
```

The initializer is evaluated once. A fresh callable result constructs its complete payload and environment into caller-provided lexical storage. At the internal ABI boundary, a callable-returning function/procedure receives a hidden destination pointer; a forwarding call may pass that destination onward. Merely returning a header pointing into an expired callee environment is invalid. Captured regions remain borrowed and retain their original lifetimes and access permissions.

Declaration and nested value capture copy the complete selected callable value, including its environment. Each invocation initializes its local captured callable values from the retained snapshots under H-553; local replacement does not update those snapshots. A wrapper retains an independent callable snapshot; replacing the original later does not change that snapshot. Explicitly aliased places, referenced targets and regions inside either environment remain borrowed, so permitted changes to those places/targets stay observable. Rebinding an ordinary source reference does not retarget its saved reference value. No regional object is copied and no lifetime is extended.

A default callable parameter has an independent callee-local target, following D-14's default parameter ownership. Its value is prepared at that argument's evaluation boundary, before later arguments can replace the source holder. An explicit `in O` parameter borrows external callable storage instead. A callable value result constructs an independent complete value; an explicitly borrowed result retains its declared external owner. Fresh images with no outstanding borrowed access may transfer into their destination without copying. Allocation failure and later-argument failure must release prepared values without altering the source holder or extending captured-region lifetimes.

`destination <- source` replaces a compatible callable value in existing lexical storage, preserving type/owner checks. Ordinary `destination = source` cannot rebind a func/proc variable. A ptr instead refers to callable storage and uses ordinary nullable-reference rebinding/lifetime rules. Its invocation reads the currently stored callable value. typeof is compile-time only; H-440 uses is null for absence testing, while actual nullable-target access retains checked NPE. H-439 is compares selected storage identity, not code or captured-payload equality; complete independent value copies remain distinct storage.

[D-28](RegionalAndLexicalStorageRefactor.md#captures-tls-native-interfaces-and-persistence) reviews complete environments and caller destinations under growable lexical backing. Environment-relative coordinates, lexical slot locators and borrowed target references remain distinct. The hidden destination denotes the selected caller storage; a future lowering must preserve it if that owner's backing can grow during construction. The existing native destination-pointer path alone supplies no such proof. H-437/[D-28-OWNERS](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations) requires parameter, result and admitted capture transport to reuse shared actual-owner information while retaining each selected target. Recursive activations keep distinct owner instances where required; shared metadata does not promote lexical cells or extend borrowed owners; H-555 admits explicitly owned lexical references only with a non-escape proof. These integration obligations preserve complete-value copies, prepared snapshots, borrowed-region lifetimes and the ban on implicit lexical-cell promotion and shared captured binding slots; physical anchor/destination protocols remain pending.

At the implementation level, the complete callable representation must remain valid under its admitted storage relocation: it contains no pointers into its own prior storage or an expired factory frame. Nested callable environments are included by value. This is an intrinsic lexical value category, not a general bits type that may enter regional fields/arrays. The adapter computes environment access from the current payload address; a copied header pointing to a previous lexical environment does not satisfy this contract.

Direct/behave procedure calls use current dynamic bindings; perform installs the selected procedure's captured symbolic values for its execution scope. Native representation must not merge bodies, module contexts or captured snapshots merely because signatures match.

Recursive calls are required for both functions and procedures. A blanket recursion rejection is not an acceptable endpoint. Call checking must account for cycles in the demanded program; successful finite source expansion is not a substitute for native recursion. The deferred const-function/TCO plan in [D-19](ConstFunctionsAndTailCalls.md) does not restrict procedure recursion. Material conflicts with that plan require human review.

## Definition mentions and runtime view construction

A name bound to a callable definition denotes that definition under ordinary lexical lookup. When its use requires a runtime callable view, construct a fresh view using the definition's established lexical context and the current required symbolic bindings. This rule applies to named functions and procedures, including mentions within their own bodies and explicit/inferred capture operands. CoD defines no special source-level "currently executing callable value" selected by a recursive name.

Names bound to stored callable values or parameters retain ordinary value semantics. Reading/copying such a value preserves its complete selected environment; adapting its view type or reinstalling it does not recapture from its definition. Borrowed access still selects the existing target. Nearest-binding shadowing remains effective: a parameter named f can denote a stored value even inside a callable definition also named f. Bare registered symbol identities and @symbol TLS reads retain D-10's existing distinction; this rule does not turn symbol objects into ordinary callable-definition name aliases.

```cod
symbol @bias:u64;
proc read() -> u64 { return @bias; }

effect @bias <- 10u64;
proc a = read;      # New view: captures 10.
effect @bias <- 20u64;
proc b = read;      # New view: captures 20.
proc c = a;         # Complete value copy: preserves 10.
```

The same timing applies to a recursive definition mention:

```cod
symbol @bias:u64;
symbol @call:proc(u64)->u64;

proc demo() -> u64 {
    effect @bias <- 10u64;
    proc recur(n:u64) -> u64 {
        if (n == 0u64) { return @bias; } else {};
        effect @bias <- 20u64;
        proc again = recur;          # New view captures the current 20.
        effect @call = again;
        return perform @call(0u64);
    };
    effect @call = recur;            # Initial view captures 10.
    return perform @call(1u64);      # Required result: 20.
}
```

These examples specify the required semantics; current implementation coverage is recorded in the [source capability reference](../tests/cod/language/README.md). The inner binding does not mutate the initial view's stored snapshot. perform installs the selected view's captures, while ordinary/behave invocation uses current symbolic bindings under D-10.

Direct definition calls do not require an intermediate view merely because they mention the definition. Type-only queries retain their nonexecuting behavior: they establish type/context evidence without evaluating captures or allocating a view. A recursive definition reference is not itself a recursively embedded runtime environment; the enclosing definition body is not executed to construct its view. Required dependencies still follow ordinary capture analysis and complete-value rules.

Lexical captures keep their definition-bound identities and admitted environment. In an escaped callable, the retained lexical environment supplies those dependencies; a fresh view cannot search the caller's local names, retain an expired factory cell, promote ordinary lexical cells or extend a borrowed owner's lifetime. Stored nested callable dependencies are copied as values; regional dependencies retain their established borrowed meaning. The current-symbolic-binding rule does not grant functions dynamic effects, relax explicit capture-list completeness, or bypass owner/access/initialization checks. Inherited default expressions retain their separate type-bound origin and evaluation rules.

Fresh construction specifies an independent complete value when one is required. It does not prescribe a heap allocation for every mention, a physical image layout or a new native body per view. Storage/copy elimination must preserve capture timing, required effects, selected targets, lifetime and cleanup under the existing optimization contract.

Acceptance requires the following composed cases:

- Repeated definition materialization captures 10 then 20; copying/adapting/reinstalling the first value preserves 10, and perform/current invocation retains its established distinction.
- The recursive example returns 20; forwarding a definition mention as a callable argument and capturing it in a nested callable use the same construction rule.
- Direct recursion and type-only queries preserve lexical definition identity without manufacturing a runtime self value or capture effects.
- An escaped callable retains valid lexical dependencies, with no caller-name substitution; a value parameter or local shadow follows ordinary lookup and copy semantics.
- Missing required captures, invalid owner/access/lifetime, uninitialized payload reads and forbidden function effects reject; construction/copy failure follows ordinary cleanup without extending owners.

## LLVM compilation scale

Reasonable performance is a development requirement, including during the prototype stage. An MVP may narrow the implemented surface, but each implemented path must preserve the business contract and have defensible runtime and compilation costs. Measurements must distinguish construction and complete-value copying from invocation, expose allocation and cleanup behavior, and include shared/deep composition. Passing a convenient demonstration does not establish generality or scalability. No numeric performance budget or particular optimizer/allocator is selected by this requirement.

WPO determines the demanded program and its semantic contexts; it does not require one indivisible LLVM optimization unit. Native function boundaries must represent recursive cycles and shared dependencies without expanding the dynamic call tree into source copies. Reuse must preserve the selected module world, concrete signature, conformance information and captured context. A shared origin or matching signature alone is insufficient.

Code sharing must preserve selected semantic views, concrete types, actual owners, access, initialization, captures and completion. Reuse is an implementation optimization; it does not memoize runtime values or eliminate required effects. Current emitter organization and measured coverage belong in the source capability reference.

## Implementation status and review boundaries

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

H-417 closes the reference-owner alternative question: each ref/ptr needs one statically provable owner, inferred from initialization if omitted. A selected callable or returned-view join with unresolved distinct owners rejects; runtime selection of targets within one established owner remains permitted. This is per-reference uniqueness, not a requirement that every captured reference share one owner. D18-Q-result-discrimination remains a separate representation question. Complete values, checked captures, caller destinations, recursion, owner/access/lifetime proofs and cleanup are required within each qualified scope. Broader family, stage and coverage obligations belong to D-21/D-26; they do not approve a permanent callable ABI.

### D18-Q-result-discrimination — behavior confirmed; representation pending-human

An abstract result can hide different concrete types: callbacks returning `Entity in r` can return Node or Edge. H-375 requires subsequent calls to retain actual-type method selection; a one-concrete-result implementation limit cannot restrict the language. H-376 requires precompiled coverage of admitted alternatives.

Concrete evidence must survive parameters/results, stored and nullable references, TLS, callable environments and owner-preserving relocation, with valid result destinations and exceptional cleanup. H-396 requires concrete discriminators without selecting physical transport, a reflection catalog or a regional object-header tag. Returning only an address and an abstract declared type is insufficient. D-21/D-26 owns coverage and execution preparation; H-417 independently requires statically unique reference owners. Different concrete types may share that one owner, so the owner decision does not settle concrete-type evidence transport.

## Examples and validation

The [callable fixtures](../tests/cod/language/closures/) specify composed acceptance scenarios: independent nested values, captures and current contexts, recursion, selected bodies, caller destinations, nullable views and cleanup. Current runnable coverage is maintained in the [source reference](../tests/cod/language/README.md); fixture presence alone establishes no implementation support.

Measure construction, copy and invocation costs, compilation scale, region release and failure recovery on the supported paths. Reject incompatible kind, owner, access and lifetime; do not infer whole-document qualification from passing examples.
