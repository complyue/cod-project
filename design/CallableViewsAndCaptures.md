# Function values, here callables and declared place aliases

Design revision: 137 · Recorded: 2026-11-25 · Status: scoped callable contract; implementation boundaries recorded separately.

## Design authority

At r137 [D-18](DesignAuthorityRegistry.md#d-18) adopts [D-10’s family replay rule](SymbolicDynamicScope.md#family-wide-effect-requirements-before-20) for symbolic handler activation. The [project user’s decision](../CHANGELOG.md#h-perform-family-capture-20261125) sets the capture and restoration scope. Function and functor values, synchronous here calls and the restriction on ordinary proc storage continue.

At r136 [D-18](DesignAuthorityRegistry.md#d-18) applies the [normal-completion decision](../CHANGELOG.md#h-normal-completion-20261124) to the result destination of a proc or here invocation. [D-24](CallableResultInference.md#results-of-proc-func-and-here-bodies) governs the result and [D-17](ExceptionControl.md#return-state-inspection-and-replacement) governs its observation during cleanup.

At r135 the [functor-value decision](../CHANGELOG.md#h-functor-value-distinction-20261124) applies [the callable terminology](CallableStorageAndHereAccess.md#function-value-and-functor-value) to lexical holders, owner projection and bounded here use. A functor value retains a selected receiver and actual owner throughout every use. P-721/P-722 connect that lifetime to D-18-FUNCTOR-VALUE.

[D-07-STRING-FIELDS](DesignAuthorityRegistry.md#d-07-string-fields) covers [direct text places](Utf8Strings.md#direct-string-fields) under [H-STRING-BUILD-20261121](../CHANGELOG.md#h-string-build-20261121). The r134 string prototype distinguishes `val var` for a direct String place from `ref var` for a declared reference slot. Check exact place kind, actual owner and permitted updates through either name. Callable access and lifetime contracts continue.

[H-FUNCTOR-ARGUMENT-20261119](../CHANGELOG.md#h-functor-argument-20261119) continues D-18-HERE at r133 for receiver selection in the [existing here-callable inventory](#forms-of-here-callable). Apply the actual owner and access checks to the target selected at argument preparation, including during synchronous forwarding.

[H-CALLABLE-TAIL-20261119](../CHANGELOG.md#h-callable-tail-20261119) continues the result and cleanup references at r132. D-18 retains direct lexical access, invocation lifetimes and control destinations.

- **Here callable forms.** H-HERE-FUNCTOR-20261118 includes the receiver-bound functor. [K-GENERATOR-YIELD-FORMAL-20261118](../CHANGELOG.md#k-generator-yield-formal-20261118) groups for-loop bodies and here calls under the same parameter at r130. [Here callable forms](#forms-of-here-callable), D-18-HERE and D-29-GENERALIZED-FOR/P-623–P-624 cover the source inventory. Keep the existing lexical use bound, receiver owner and per-form control rules. [H-GENERATOR-RECEPTION-20261119](../CHANGELOG.md#h-generator-reception-20261119) continues this inventory at r131. D-29 checks that an immediate full scope can invoke the enclosing lexical here parameter with its original consumer.

- **Here body control.** [H-HERE-BREAK-CONTINUE-20261118](../CHANGELOG.md#h-here-break-continue-20261118) covers this r128 summary and D-18-HERE-BREAK/P-621–P-622 at storage r10. Results reach the same source destination through direct and forwarded calls. The [examples](CallableStorageAndHereAccess.md#break-and-continue-in-a-single-brace-here-body) check feedback, early call completion and supplier return. Mainline owns native transport.

- [D-18](DesignAuthorityRegistry.md#d-18), [D-18-HERE](DesignAuthorityRegistry.md#d-18-here) and [D-18-HERE-STORAGE](DesignAuthorityRegistry.md#d-18-here-storage). [H-CALLABLE-MODEL-20261118](../CHANGELOG.md#h-callable-model-20261118) confirms r127 and storage r9 for capture-free functions, synchronous here calls, both brace forms and activation identity. P-609–P-612/P-615–P-622 derive from C-65/C-73, D-12-LIFETIME and charter §§3.2–3.3/4.3/6.1. Reusable code and caller-owned state have independently checkable lifetimes. See [callable storage and direct lexical access](CallableStorageAndHereAccess.md) for acceptance across recursion, repeated invocation, forwarding, owner invalidation and cleanup. Mainline owns lowering and verification.
- H-481 preserves definition identity. H-563 covers lexical storage. H-558 governs the alias glossary. H-567/H-568 distinguish callable preparation from bits or graph transfer. H-459–H-462 preserve type-bound defaults and multi-method coverage. D-10 owns dynamic binding and symbolic replay. D-17 owns full-scope completion. D-21/D-26 own dispatch and native coverage. D-28 owns current owner-plus-locator resolution.
- Independently qualified reference/access, placement and declared-place contracts continue. D-18-VALUE-CAPTURE and D-18-BORROW supply no implementation authority for an ordinary function environment. The changed input model also applies to D-17 full scopes and D-10 trailing callbacks and caller-context defaults. Regional callable storage, lifetime extension and a permanent native ABI are outside the contract.

## Language model: no closure abstraction

CoD has no closure abstraction. A storable function selects code. A here body accesses the lexical slots in its particular defining activation and can run only while those slots and their owners are valid.

Compiler implementations must preserve source-visible distinctions: independent value storage, a reference's target, an explicit var alias's selected place, and a symbolic identity's dynamically installed value. Natural lexical nesting supplies ordinary lifetimes. Access that exceeds that nesting must be expressed through the formally specified, statically checked dynamic-scope/owner contracts; capture, heap allocation and code sharing are not escape hatches. Existing explicit region ownership transfer remains its own contract. See the [vision principle](GlobalVariablesDoneRight.md#explicit-lifetimes-without-closures).

## Callable views have lexical storage

Function-value and functor-value holders have lexical storage (H-563). A functor value’s receiver may live in a region. Its callable holder follows the lexical storage rule. The compiler can use registers, stack slots or lexical backing. Even an entry address that fits in one word cannot be placed in a regional field or array.

A regional aggregate or array cannot evade this boundary by embedding a callable-view value directly, through another compound value or through a generic instantiation. Same-owner field references cannot make a lexical callable target regional. Storage eligibility is a type/owner contract, not a special permission inferred because one runtime value is null or has no captures. H-565/H-566 reviews whether callable-bearing ADTs should be admitted specifically in lexical storage; this does not reopen the regional prohibition or automatically qualify such aggregate fields.

A definition-installed symbolic procedure has an internal scoped replay record. Its lifetime is checked through restoration. Module scope is a full lexical scope. A regional semantic graph may describe a definition without storing a callable value.

Acceptance rejects regional storage through nested and generic carriers as well as direct fields. Lexical function holders can receive independent selections. A here value can only be invoked or forwarded synchronously.

## Copying, moving and graph transfer

H-567/H-568 fixes the source-language mental model: **CoD has no C++-style object copying/moving semantics. Only pure bits values admit whole-payload transfer; other objects do not acquire that operation because they are lexical, relocatable or initialized.** CoD does not infer copy/move constructors, source invalidation, hidden graph duplication or lifetime transfer from assignment, capture, grouping or return syntax.

Keep the actual operations distinct:

- Bits transfer reads an initialized valid payload and initializes/writes a destination under its declared rules; it does not transfer object identity or consume the source.
- Reference binding selects an existing target and preserves its actual owner/access. It does not duplicate the pointee or make a declared place alias of the source reference slot.
- Ownership transfer changes responsibility for an existing region resource under the consuming contract; it does not move or copy regional objects.
- Preparing a function value preserves its selected implementation and typed interface. Here calls borrow direct lexical access. Scoped symbolic replay has its own preparation and lifetime rules.
- Graph transfer is explicit construction/mapping into a destination graph according to the applicable library contract. It preserves or transforms sharing/cycles as that contract specifies; CoD supplies no automatic default graph copier or mover.

Compiler-level loads/stores, byte copying, register movement and native C++ utility operations may implement admitted behavior. They cannot establish source-level operation eligibility. In particular, relocating backing while preserving the same logical owner is distinct from graph transfer to another owner. [D-22](ConstructionAndAcquisition.md#storage-eligibility-and-payload-transfer) records aggregate consequences; ordinary capture/cleanup/owner obligations remain unchanged.

## Structural regions

`region` declares a product of named fields. A consumer can require a compatible field subset; WPO retains the concrete region layout, including additional fields and their actual offsets. Field names, val/ref/ptr kinds and data/UoM identities remain significant. Structural compatibility neither creates an owner nor converts one actual region into another. A requirement is a lower bound, not an access view that hides extra concrete fields.

Unmodified/val fields store bits directly; ref/ptr fields store same-owner references. An explicit var alias captures the selected field place: reads observe its later writes or rebinding under ordinary access/initialization and checked-NPE obligations. An ordinary value binding copied from that field instead captures its selected value, as specified below. Region-level allocation/release remains intrinsic; free lists, pools and item reuse remain library policies. No physical allocator or relative-pointer encoding change follows.

## Callable completion

The [callable result rules](CallableResultInference.md#results-of-proc-func-and-here-bodies) allow a void procedure to finish naturally. A procedure returning a value still requires a value return. A func or full-scope here may supply a normal value tail. Return from deferred catch/finally targets the enclosing callable, replaces a pending completion and continues remaining cleanup. A deferred entry may inspect its owning callable’s prepared result under [D-17](ExceptionControl.md#return-state-inspection-and-replacement). H-363 records pure declarations with implicit body results while deferring purity implementation. Implementation availability for explicit returns, missing-return delivery and deferred execution is maintained in the source capability reference. These source changes preserve complete values, caller result storage and owner/access checks.

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

Declared place aliases can name ordinary variables, regional fields and addressable array elements. Whole-region holders, TLS bindings, temporaries and non-addressable logical container elements do not supply this alias form. Selecting a field through a dynamic region resource does not alias its TLS binding. A here body using a declared place alias accesses that once-selected place under its actual owner. A lexical-place alias is admitted only when every use is statically within that place's owner lifetime.

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

If x and y happen to select the same target, the writes naturally alias that target; they still do not merge p and q's slots. `a = y` does not retarget the alias a to y's slot: a remains a name for p's slot. `var b -> a` denotes that same place, without adding a proxy chain. For a direct bits place, `<-` writes its payload and builtin `=` has no reference slot to rebind. For the prototype’s `val var tgt:String`, `tgt = ready` performs the direct String field’s same-owner installation. The declared place alias continues to name the same field. Alias const restricts access through the alias, not other names. An alias type assertion cannot convert the selected storage, alter ptr/ref mode or upgrade permissions.

Selecting `var element -> items[index]` evaluates the selection once. Later index changes or rebinding items do not select a different element. Backing movement follows the selected owner's current resolution information; logical target removal/reuse still needs the container's validity contract. An alias neither pins native addresses nor extends storage lifetime. It need not read a payload merely to name an admitted uninitialized place; later reads/writes retain ordinary definite-initialization rules.

Ordinary parameter passing and result reception obey their declared value/borrow contracts. Passing a var alias does not automatically pass its binding slot as an out parameter; assignment-family destination parameters expose slots only under their explicit D-14 contract. Builtin assignment results expose target views, never the lhs slot. Callable value copies retain independent complete values; sharing immutable code or optimized backing must preserve that independence. A here body accesses the live slot designated by the declared place alias. An explicit ref/ptr argument follows its own parameter contract. These distinctions must survive inference, generic specialization, inlining and optimization. No runtime alias registry or ban on legitimate target sharing follows.

**Two owner roles matter for reference-slot aliases.** In `ref p:T in O`, O owns the target; p's binding slot ordinarily belongs to the current lexical invocation. `var a -> p` names that local slot, so retaining a requires the slot owner as well as the target dependencies of its uses. An alias annotation `var a in O -> p` cannot relabel that local slot as region O storage. A here body accessing either local spelling is bounded by the helper activation. An ordinary reference result may outlive the helper if the caller independently proves O live. An alias directly selecting a field in O instead depends on that field's actual owner, irrespective of where the alias name was declared. This prevents an explicit alias from laundering a short-lived binding through a long-lived referent.

## Callable owners and captures

An ordinary named or anonymous function can use its parameters, locals and static definitions. Referencing an enclosing runtime variable in its body rejects, including through an explicit capture list, a declared place alias or another lexical function holder. Pass runtime data explicitly or invoke a here body while its supplier is live. A function type can independently describe owners for explicit borrowed parameters or results and for origin-bound defaults.

## Here callables and direct lexical access

The [callable storage contract](CallableStorageAndHereAccess.md) covers argument preparation, defaults, symbolic installation, result delivery and cleanup. A func can invoke a here parameter supplied in a proc context under the lexical permissions of that body.

A storable function value denotes its selected definition or family under the statically checked signature. A factory can return a static predicate. A predicate referring to the factory’s local threshold rejects even if the threshold is readonly and would fit in a register.

A `here` body has **direct lexical access**. An outer name still denotes its original declaration and slot. Shadowing works normally. Every invocation, including its required cleanup, must fit the lifetime of the **particular defining activation** and of each slot used. Recursion can have several live instances of the same declaration. An ordinary callable boundary cannot provide access to an enclosing activation it did not retain. Read, write, rebind, consume and dereference retain their ordinary type, access, initialization and lifetime requirements.

```cod
func twice(here step() -> void) { step(); step(); }

proc count() -> u64 {
    val n <- 0u64;
    twice() {{ n <- n + 1u64; }};
    return n;  # 2.
}
```

Both calls update the same n. Likewise `{{ p = other; }}` rebinds the enclosing mutable ref slot, and a later invocation uses the currently selected target. No second source binding or shared heap cell is constructed. The receiver of a `here` parameter can use a common invocation interface with a link to the supplied body's defining activation. Each body knows the slots it uses and how to resolve their owners. A single receiver can therefore invoke bodies from sites with different fixed owner counts. Such a link must remain valid across backing relocation. Inlining or passing just the needed access information is equivalent if observations agree.

<a id="six-forms-of-here-callable"></a>

### Forms of here callable

The source inventory has five families when a `for` body and a single-brace here call are counted together. A block literal describes how the body is written. A here-parameter call describes how a receiver invokes or forwards it. A single expression can use both. Function values and owner-qualified functor values have distinct assignment rules. Supplying a here body creates a synchronous use.

| Source form | Lexical access and control obligations |
| --- | --- |
| Block literal | Standalone `{{ ... }}` invokes immediately. At a here parameter, `{{ ... }}` has its own return and cleanup. A trailing `{ ... }` has supplier-directed return and exception propagation, `continue` for invocation feedback and `break` for the originating call. |
| Here-parameter call, including a `for` body | The supplied body uses its particular defining activation. A receiver may invoke it repeatedly or forward it synchronously to a compatible here formal. The `for` header receives each handoff under its declared modes. In the equivalent single-brace here call, `continue` sends feedback, `break` ends the original call and `return` targets the lexical supplier. [Generator elaboration](RangedForAndGenerators.md#a-generator-declaration-introduces-a-here-parameter-named-yield) |
| `effect @sym(...) { ... }` body | A reached installation may invoke a method that needs lexical slots only while its particular defining activation is available. Module-world family registration alone supplies no live frame. Scoped replay and restoration must prove those slots live. |
| Module top level | Each incorporated module, including the selected `__main__`, executes its top-level code with direct access to its own initialized lexical slots. Its prefix and `postpon` fragments share that module activation. A callable declared there keeps its own access rules. [Module scope and example](ModuleWorlds.md#here-callable-module-top-levels) |
| Functor used as a here callable | A data receiver with a compatible `()` overload can supply a synchronous here argument. Invocation uses the receiver under its actual owner and access permissions. The bounded callable may be forwarded to another compatible here formal while the receiver remains live. The receiver itself keeps its ordinary data type and storage eligibility. |

In `test(limit, 12u64)`, the functor supplies the predicate and the literal supplies the scalar input. The supplied here callable cannot escape its synchronous use. Preparing a functor value in an explicitly owner-qualified holder checks the selected receiver and its lifetime at that separate binding. Storing the underlying receiver data follows its own type and owner rules. The [functor examples](DomainLanguageDesign.md#functor-receivers-and-callback-arguments) show the difference between selecting the receiver at its written argument position and reading a lexical reference inside a supplied body.

The frame proof leaves two obligations. A region referenced through an outer slot may be released while that slot still exists; using a ref to the old instance must reject. Also, a write to an outer slot can invalidate a caller's flow facts. The checker must account for zero, one or repeated calls, including exceptional exits: it cannot assume that a callback initialized a slot, preserved a non-null refinement, or can consume the same owner twice. Access to an outer slot grants no release authority beyond its ordinary declaration. A conservative checker may reject an unproved use.

### Symbolic snapshots at definition activation

Definition activation snapshots every incoming binding in the installed family’s union after local supply. A `perform` invocation overlays the full set and restores the caller’s bindings on exit. Registering static method information supplies no runtime snapshot. Under the restricted design, activation of an effectful definition supplies an internal record for scoped installation. Only `perform` and `behave` invoke that handler. Standalone extraction or assignment installation cannot request a second snapshot. [Restricted handler operations](SymbolicDynamicScope.md#definition-installed-effectful-procedures)

```cod
symbol @rate:u64;
symbol @quote:proc()->u64;
val fee <- 1u64;
effect @rate <- 10u64;
effect @quote() -> u64 { return fee + @rate; };
fee <- 2u64;
effect @rate <- 20u64;
val replayed <- perform @quote();   # 12
val current <- behave @quote();     # 22
```

The definition activates with @rate equal to 10. Both invocations read the live lexical fee as 2. Unwind restores the previous binding and its existing replay record. A here handler's borrowed slots and any regional targets still need valid owners. [Definition mentions](#definition-mentions-and-runtime-view-construction) describes the separately confirmed baseline for first-class callable values. Ordinary function values have no symbolic replay record.

Use the [compiler migration criteria](CallableStorageAndHereAccess.md#implementation-impact-and-verification) for query, flow and native paths.

<a id="value-snapshots-and-invocation-local-captures"></a>
<a id="ordinary-value-input-implementation-contract"></a>
<a id="explicit-borrowed-captures-and-shared-state"></a>
<a id="capture-acceptance-and-remaining-scope"></a>

## Outer slots and explicit arguments

An explicitly passed value parameter receives its ordinary local storage. An outer name in a here body denotes the original slot on every invocation. The difference is observable even for a scalar. A read of a mutable ref slot sees its current binding. Any earlier reference still designates its own target and actual owner. The checker tracks both owner invalidation and the effects of writes through forwarding and unwind.

```cod
func add_one(x:u64) -> u64 { return x + 1u64; }
func choose() -> func(u64)->u64 { return add_one; }
proc run() -> u64 {
    val n <- 4u64;
    func f = choose();
    {{ n <- f(n); }};
    return n;                           # 5
}
```

A stored function cannot enclose n. A synchronous here callback can update n twice and the supplier sees both writes. Attempting to publish, return, or store that callback for later invocation rejects. A declared place alias remains usable under the lifetime of its selected storage. Shared state across dynamic scopes follows the explicit symbolic binding contract.

## Owner projection

A function value has no hidden body owner to project. A functor value carries the actual owner of its bound receiver. Receiving it under a fresh owner name binds that actual owner. Reception under an established owner requires the same actual owner. The actual owner and allocation permission must both be proved for construction through that name. An owner required by an explicit parameter, result or type-bound default must satisfy its own declared contract. A here receiver cannot extract an arbitrary owner from the supplied body or extend that owner's lifetime by forwarding the body.

## Runtime callable contract

The static view establishes parameter access and passing modes, results, declared failures, and applicable methods in the selected module world. Actual type evidence still participates in dispatch. A `typeof origin` parameter covers all required origin methods and uses the origin's named arguments and defaults. Dependencies of a definition-bound default are checked when applicable. They belong to the type contract, independently of which function entry a holder selects.

A lexical function holder can be passed independently, returned to a caller destination, or replaced by `<-`. Preparing a callee or argument fixes the selected value before later arguments can replace its source holder. An explicitly borrowed holder observes updates under its ordinary access contract. Keep destination selection, readiness, cleanup on failure, exceptional exits and selected method evidence intact. A ptr to a holder retains ordinary nullable-target checks. The `is` operator compares designated storage under D-28. Equal machine entries do not establish equal holder identity.

For a statically selected single method a native entry address can suffice. A compiler-generated entry may dispatch among methods or adapt the known signature. A direct call can eliminate the handle altogether. Here calls require access to the right activation across recursion, backing growth, repeated use and synchronous forwarding. [Implementation latitude and acceptance](CallableStorageAndHereAccess.md#what-an-invocation-needs)

## Definition mentions and runtime view construction

A definition name retains its lexical meaning through recursion and type queries. In a value position a func supplies the selected code under the established contract. A name denoting an existing function holder reads its current selection. A direct named procedure call uses current symbolic bindings. Only definition activation prepares a symbolic handler's replay record. `perform` uses its saved bindings and `behave` the current bindings. Both use the live lexical slots of the correct handler activation. A type query performs none of these runtime operations.

## LLVM compilation scale

Reasonable performance is a development requirement, including during the prototype stage. An MVP may narrow the implemented surface, but each implemented path must preserve the business contract and have defensible runtime and compilation costs. Measurements must distinguish construction and complete-value copying from invocation, expose allocation and cleanup behavior, and include shared/deep composition. Passing a convenient demonstration does not establish generality or scalability. No numeric performance budget or particular optimizer/allocator is selected by this requirement.

WPO determines the demanded program and its semantic contexts; it does not require one indivisible LLVM optimization unit. Native function boundaries must represent recursive cycles and shared dependencies without expanding the dynamic call tree into source copies. Reuse must preserve the selected module world, concrete signature, conformance information and captured context. A shared origin or matching signature alone is insufficient.

Code sharing must preserve selected semantic views, concrete types, actual owners, access, initialization, captures and completion. Reuse is an implementation optimization; it does not memoize runtime values or eliminate required effects. Current emitter organization and measured coverage belong in the source capability reference.

## Implementation status and review boundaries

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

H-417 requires one statically provable owner per reference. A here site may use several distinct references, each with its own proven owner. Concrete result discrimination remains a separate representation question.

### D18-Q-result-discrimination — behavior confirmed; representation pending-human

An abstract result can hide different concrete types: callbacks returning `Entity in r` can return Node or Edge. H-375 requires subsequent calls to retain actual-type method selection; a one-concrete-result implementation limit cannot restrict the language. H-376 requires precompiled coverage of admitted alternatives.

Concrete evidence must survive parameters/results, stored and nullable references, TLS, callable environments and owner-preserving relocation, with valid result destinations and exceptional cleanup. H-396 requires concrete discriminators without selecting physical transport, a reflection catalog or a regional object-header tag. Returning only an address and an abstract declared type is insufficient. D-21/D-26 owns coverage and execution preparation; H-417 independently requires statically unique reference owners. Different concrete types may share that one owner, so the owner decision does not settle concrete-type evidence transport.

## Examples and validation

Acceptance covers static function selection, type-origin defaults, recursion with overlapping activations, invalidation of a borrowed owner, slot rebinding, nullable invocation and cleanup. [Source capability reference](../tests/cod/language/README.md)

Measure construction, copy and invocation costs, compilation scale, region release and failure recovery on the supported paths. Reject incompatible kind, owner, access and lifetime; do not infer whole-document qualification from passing examples.
