# Scalar units, strict brands and array index domains

Discussion revision: 31 · Recorded: 2026-11-10 · Status: alternatives under discussion.

## Design authority

- **Region-source continuity:** [H-307](DesignAuthorityRegistry.md#h-307)/[K-435](DesignAuthorityRegistry.md#k-435) updates region examples to the declaration-keyword surface in [D-14](RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language). C-70/P-270–P-271 preserves the intrinsic borrowed/owning distinction and existing lifecycle constraints. This revision changes source notation and evidence navigation only; it does not qualify pending transport, optional-owner, numeric or general graph contracts. Acceptance is coherent examples plus the migrated compiler/lifecycle checks recorded by K-435.

- **Registry / scope:** [D-11](DesignAuthorityRegistry.md#d-11), revision 31, all sections; [K-398](DesignAuthorityRegistry.md#k-398) records H-256 priority implementation planning for representation-independent UoM; [K-397](DesignAuthorityRegistry.md#k-397) adds the H-254/H-255 future fraction/representation dependency and conflict-review requirement; full contract and candidate value derivations remain `pending-human`. [K-396](DesignAuthorityRegistry.md#k-396) coordinates the module-world dependency without changing scalar/unit rules. [K-354](DesignAuthorityRegistry.md#k-354) records implicit priority for names automatically selected by unit bundles. K-353 retains exported conversion-alias evidence with preserved bundle membership, formula context and alias deduplication. K-343 retains its earlier method-selection review. Only the identified human-confirmed propositions authorize their stated implementation scopes. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred. [K-402](DesignAuthorityRegistry.md#k-402) records the implemented quantity/widening subset and56 focused checks.
- **Value lineage:** C-47/P-112 and D-11/P-113 derive strict scalar/index meanings from C-10/P-14/C-01's inspectable typed business intent. C-49/P-119/P-120 preserves independent unit identity/reuse; C-50/P-121–P-123 and C-51/P-124–P-128 govern typed methods, conversions and explicit route control. C-39/P-94 and C-56/P-186 retain region-only data and nominal abstractions. Candidate value paths keep their registry status; P-118's application formula dependency grants no authority to choose an SNN configuration.
- **Human baseline:** H-123/H-125/H-128/H-130–H-138 supplies structured values, branded array indices, first-class UoM identities distinct from symbols, local aliases, one-shot using bundles, pure conversion methods, explicit starred parameters and direct-before-declared-indirect routes. H-148 requires each argument's conversion before the next argument. Compound units remain planned under H-132. H-193/H-197/H-198/H-200 and H-209/H-210 supplies regional user data, mutable explicit-memory functions, type/data and unique-most-specific matching. H-218/H-220 requires declared data conformance, adapter import permission and consumer visibility through using. H-230–H-233 supplies selected callable identities/contexts and explicit publication; these rules do not register new UoM conversion definitions implicitly.
- **Native scope / evidence:** Scalar units, selected conversion routes and typed arrays retain their recorded [numeric evidence](../tests/cod/language/numeric-validation.json). [Regional-data evidence](../tests/cod/language/data-validation.json) covers arithmetic pool layouts with reusable regional result storage. [Module evidence](../tests/cod/language/module-integration-validation.json) covers explicit conformance and inherited matching. [View-fold evidence](../tests/cod/language/view-fold-validation.json) covers borrowed record/array accumulators with fixed concrete type, owner and admitted-interface guarantees. [Explicit-library evidence](../tests/cod/language/explicit-library-validation.json) covers caller-region graph construction and frame updates through functions, with independent FP32 comparisons at two scales. Each report retains its actual source revision; no historical result is relabeled current.
- **Invariants / non-goals:** Preserve nominal unit identity, declared index domains, explicit conversion choices, conversion/body source context and argument order. Conversions cannot access dynamic effects. Regional data stays in regions without copy/move; views do not extend ownership. No implicit unit erasure, copied UoM definitions, compound-unit implementation, allocator change or SNN model decision is selected.
- **Pending boundaries:** H-211/H-212/H-214–H-217 defines source-preserving selections, mixin, local aliases and low-priority associated method names; H-225/H-226 fixes declared-signature association including result-only factories and using-local shadow/import-local extension. H-230/H-232 supplies separate extension identities and using-preserved/import-adapted ordinary helper contexts; [Module worlds](ModuleWorlds.md) supplies receiver-local identity composition and H-253 conflict rejection; explicit conflict arbitration remains undecided. General interaction between ordinary callable adaptation and automatic-conversion registration remains open. Standalone signatures and cross-alias extension grants remain open. H-204–H-210's transfer, borrowed-owner arguments and abstract-root dispatch do not settle the full const/owned/make/assignment grammar. No native acceptance is claimed for those unresolved forms.
- **Acceptance / current authorization:** H-221 authorizes language/runtime work with focused native validation. Keep incorrect index identities and unavailable/ambiguous conversion paths diagnosable, preserve argument order and source identities across modules, and show that structured numerical intermediates and loop-carried views retain their owner and type guarantees. Passing one numerical example does not qualify the full design.

Module-world selection constructs only the receiver's information. Selected UoM identities and existing conversion edges retain their source identities; aliases do not manufacture new edges. [The architecture audit](../tests/cod/language/world-architecture-validation.json) covers the shared module machinery and same-identity merging; K-395 also checks the aliased-unit bundle. Ordinary callable selection is not automatic-conversion registration. The scalar, route, compound-unit and ownership boundaries below retain their own contracts.

## Initial increment and compound-UoM roadmap

H-132 requests compound UoMs in the plan, particularly for future physical-quantity calculations, without requiring their immediate implementation. The initial work uses independently identified named UoMs, typed scalar/index values, pure conversion methods and parenthesized operator names. It does not need automatic products, quotients, powers or algebraic unit normalization.

For example, the first pool program can define named `SetCount`, `SlotsPerSet` and `SlotCount` domains and an explicit multiplication method connecting them. That provides typed arithmetic without yet requiring the compiler to derive a compound result such as `Set * (Slot/Set)`. The compound notation in the research comparison below describes future capability, not an implementation prerequisite.

The roadmap retains unit-expression products/quotients, powers where useful, normalization, unit-polymorphic inference and the distinction between algebraic aliases and fresh nominal identities. It must preserve H-128 identity and H-134 using-selection semantics. No solver, implicit conversion graph, automatic scaling policy or physical-unit catalog is selected merely by retaining that roadmap.

The immediate choices are recorded in [the language increment review](DomainLanguageDesign.md#immediate-increment-decisions): existing regional references use the calling procedure scope, UoMs use one-shot selections rather than definition import, and parameter annotations control automatic conversion and route preference. Compound-unit equivalence and inference are deferred rather than blockers for this run.

## Representation-independent units and planned fractions

[FixedWidthFractions](FixedWidthFractions.md), D-16, records H-254/H-255's planned separation of unit identity, numerical representation and admissibility. Unit definitions do not fix a storage width/layout; conversion input/output types constrain their accepted and produced representations. Intrinsic fractions such as `i64/i32` are numerical bits representations that may carry applicable UoMs. A denominator is not itself a unit. Conversion routes must remain valid across their typed representation endpoints, without silently introducing return-type dispatch or a promotion rule.

The fraction plan requires unreduced ordinary arithmetic, explicit same-type canonicalization and fixed storage without special regional allocation/ownership. Its planned constructor/output separator is `//`; line comments use `#`. K-401/K-402 implement the H-256 comment reservation and quantity refactor; fraction implementation remains deferred. Future changes to numeric representations, conversion routing, dispatch, compound units or related language features must check D-16. Conflicts with its accepted requirements must be discussed before implementing the affected choice; unrelated qualified work may proceed. D-16's open promotion, layout, grammar and signature choices remain open.

### Priority UoM refactor boundary

H-256 requests prompt implementation planning for representation-independent UoM, separately from fraction support. A unit declaration establishes semantic identity without fixing one numerical representation. Value/parameter/result types retain the representation information needed for layout and typed arithmetic. Conversion methods state their accepted inputs and produced outputs; route applicability checks the intermediate representations as well as unit identities. Identity-preserving aliases and receiver-local selected bundles remain unchanged obligations.

Acceptance for this prerequisite should demonstrate one shared unit identity used with multiple supported integer/floating representations, preservation of distinct same-spelled unit identities, typed conversions with differing input/output representations, rejected incompatible routes, source/receiver isolation and preserved per-argument conversion order. No fraction runtime is needed to establish those properties. K-402 records native acceptance of this prerequisite; full fraction execution remains deferred.

H-260 selects independent `unit m;` declarations and quantity annotations such as `f64(m)`, `u64(m)` and `x::f64(m*<-cm<-mm)`. `m(3u64)` associates m's identity with the supplied u64 bits; `scalar` exposes the underlying representation. H-261 selects whole-type lossless widening. [K-402](DesignAuthorityRegistry.md#k-402) and [56 focused checks](../tests/cod/language/quantity-widening-validation.json) establish the bounded native implementation.

### Quantity types and lossless widening

```text
unit m;
unit cm;
conversion function cm_to_m(x::f64(cm))::f64(m) {
  m(scalar(x) / 100.0)
}
function distance(x::f64(m*))::f64 { scalar(x) }
```

Units are compile-time identities; quantities combine one identity with a numeric representation. Aliases do not change either component. The existing concrete representations have these additional implicit destinations:

| Source | Lossless destinations |
| --- | --- |
| u8 | u32, u64, i64, f32, f64 |
| u32 | u64, i64, f64 |
| f32 | f64 |
| i64, u64, f64 | none |

Identity conversions remain valid. Bool is not numeric. The rule concerns every value of a source type, rather than the magnitude of one literal. In particular, i64/u64→f64 and u32→f32 require explicit conversion. Fraction-specific promotion remains deferred. Floating widening preserves numerical values, signed zeros and infinities, not a promise about NaN payload encodings.

Exact applicable methods take precedence. Otherwise there must be a unique most-specific applicable signature under lossless widening; incomparable candidates reject. Builtin mixed scalar arithmetic chooses the unique least common safe representation: u32 with f32 uses f64, whereas i64 with f64 has none. Quantity operations retain their declared overloads; widening can adapt their arguments while preserving the unit identity. Mutable reference targets are not widened.

Quantity parameter `f64(m)` permits safe representation widening within m; `f64(m*)` additionally permits selected unit conversions. Each declared conversion has typed input/output quantities. A route may widen to an input or from an output only under the table above; its intermediate representation must remain compatible. Numeric widening is compatibility, not another semantic conversion edge or an extra source of path ambiguity. Direct unit conversion remains preferred, then an explicit unit itinerary, then the existing searched-route/warning policy. Conversions complete before the next argument's effects. No implicit narrowing, ordinary return-type-only overload selection, runtime unit tag or source-world editing is introduced.

## The requirement

A machine integer is insufficient to describe the role of an array index. An incoming-set ID, a global slot position, a class-local set position and a neuron ID can share the same representation while addressing different domains. The array must state its accepted index domain. Arithmetic must preserve or deliberately transform that meaning, without requiring callers to erase it to an ordinary integer at every operation.

Related arrays can usefully share a domain: parallel source-ID and weight arrays use the same slot positions; A and B state arrays use the same neuron positions. Sharing a declared domain is a proposal consistent with SoA, not an assumption that every array allocation needs a fresh type. Per-instance brands and dynamic pool/class membership are separate choices.

The requirements have three distinct parts:

1. **Semantic identity:** which domain a value belongs to.
2. **Arithmetic relations:** which operand meanings produce which result meaning.
3. **Value validity:** whether the result fits the representation and is inside this array or class.

A solution to one part does not automatically solve the other two.

## Independent UoM identity

H-128 confirms that a UoM is a named compile-time entity with its own first-class type identity. It is not a runtime value installed under a symbol, and a symbol is not its representation. Two fresh definitions called `Length` in different modules may identify different UoMs. Selecting one through `using` under a local alias preserves that UoM's identity; it does not create a new unit or merge it with another same-named definition.

The following describes identity, not a selected syntax:

```text
module A defines Length; module B independently defines Length
A.Length != B.Length
local alias L selects A.Length
L == A.Length
```

This gives unit algebra nominal atoms rather than string labels. Aliases can be resolved before arithmetic normalization, so `L / Time` and `A.Length / Time` use the same unit identities. The identity category remains separate from symbolic effects even if the compiler shares source-resolution infrastructure. No runtime unit object or per-value identity field is implied by a first-class compile-time type identity.

Three operations need distinct semantics: selecting an alias of an existing UoM; defining a fresh UoM; and naming a derived expression such as `Length / Time`. A name for a derived expression might be an alias of its normalized algebraic value, while a fresh named UoM with a declared conversion need not be definitionally identical to that expression. H-128 fixes independent identity and aliases, but does not settle derived-definition equality or conversion rules. The distinction must be explicit rather than inferred from a shared spelling or numerical conversion factor.

## UoM definitions, one-shot using and conversion bundles

H-134 specifies two UoM mechanisms: create a fresh definition, or select existing definitions with `using`. Copying and rebinding a UoM definition is not supported; different conversion behavior requires a new definition. H-212 names body redefinition mixin and uses import for identity-preserving selection with extension permission. H-134 still governs one-shot UoM bundles; an extension-only unit import form and its interaction with that bundle rule remain undesigned, and unit-definition mixin is not authorized.

A `using` selection brings the selected UoMs and their associated conversion functions together. It may select only part of a source module. A conversion involving an unselected UoM is not added to the receiving module's available conversion set. Repeated using is not an incremental way to extend that set: select the intended bundle in one declaration. Local aliases preserve selected identities.

For a source exporting `m`, `cm`, `mm` and `km`, selecting `{m, cm, mm}` exposes conversions among the selected units and excludes conversions involving `km`. Selecting just `{m, cm}` cannot make an indirect route through `mm` available to the receiver's automatic search. A source-bound conversion body can still depend on its own implementation helpers; those dependencies do not independently expose more conversion candidates to the caller.

Published aliases of an existing conversion-bundle entry remain selectable. The bundle uses the exported local name, rather than requiring the original declaration spelling to be public. Its input/output identities and selected source implementation context remain attached to the entry. Several aliases of that same entry name one automatic conversion edge; they do not create spurious path ambiguity. A private original spelling is not added alongside an exported alias. [Native alias evidence](../tests/cod/language/unit-export-alias-validation.json) covers direct and indirect conversion, both published aliases, private-name rejection and the existing sequencing/identity checks.

This bounded mechanism selects entries already present in the source bundle. Ordinary using/import of a callable is not itself registration of a new automatic-conversion formula. In particular, an import-adapted callable context does not replace an existing bundle entry merely because it shares the source body. Such a callable can retain its ordinary named-call behavior; broader conversion-extension and registration syntax remains pending. The alias repair does not copy a UoM definition, add omitted-unit edges or reinterpret conversion formulas under the receiving module.

Conversion names brought automatically by a unit bundle occupy the same lower-priority namespace as other implicitly selected type methods (H-217/H-226). A local function, global or explicitly selected type name may shadow that name. An explicitly selected callable controls its own using/import extension permission. Name lookup completes before argument dispatch; a shadowing function with no matching parameter type does not fall back to the hidden converter. Automatic conversion retains its selected edge independently of ordinary name lookup. Same-spelled converters from distinct unit vocabularies remain separate identities: automatic routes use their typed endpoints, while an ambiguous direct name requires explicit callable selection. [Native name-priority evidence](../tests/cod/language/unit-name-priority-validation.json) covers these boundaries.

The bounded interpretation is local to the receiving module: each source selection is made once, rather than progressively reopened, and another receiving module may select its own bundle. A second selection from the same source or duplicate selection of an already included UoM is diagnosed. This is declaration composition, not a global prohibition against multiple modules using the same UoM identity. Exact batch spelling is an implementation choice within H-134's identity and selection rules.

Conversion formulas keep their source lexical bindings and remain pure functions. Layout or array data required by an operation must be explicit inputs or valid fixed lexical data; a conversion cannot fetch a hidden dynamic region, access effects or call a procedure. Declared units remain compile-time entities. Scanning and cross-module encounters follow D-10 rather than an implicit exemption for unit declarations.

## Parameter-controlled automatic conversion

H-135 makes acceptance a declaration-site choice:

| Parameter annotation | Meaning |
| --- | --- |
| `amount::f64(m)` | Require the `m` identity; permit lossless representation widening but no implicit unit conversion. |
| `amount::f64(m*)` | Permit automatic conversion to `m` using the selected pure conversion functions. |
| `amount::f64(m*<-cm<-mm)` | Permit conversion to `m`; a direct conversion wins, otherwise use the declared indirect route `mm → cm → m` when it connects the supplied unit to `m`. |

The star is a parameter conversion policy, not a new UoM identity, a compound unit or a runtime flag on the numeric value. Matching `m` needs no conversion. An actual conversion changes the value through the selected pure formulas; it does not unify source and destination identities. Scalar representation and rounding remain part of those formulas and the numeric contract.

H-136–H-138 establish the selection order:

1. Use a direct applicable conversion when one exists.
2. Otherwise follow the parameter's declared indirect route, when supplied; its steps must be available and form a source-to-target route.
3. With no declared route, discover conversion paths at compile time. Select a unique path automatically.
4. When multiple routes remain unresolved, issue a warning. A compiler option can promote that warning to an error. Warn-and-continue is supported; it is not silently replaced by unconditional rejection.

Warnings should identify the selected route and alternatives so the source can state an intended route. The prototype may use a deterministic fallback for warn-and-continue; the choice must be reported and must not claim different formulas or rounding paths are equivalent. Search must terminate on conversion cycles; revisiting the same UoM does not create an endlessly growing set of useful alternatives. These are implementation requirements for a finite, reviewable compile-time choice, not a runtime conversion registry.

A declaration that names an unavailable or disconnected route needs a direct diagnostic rather than silently ignoring the user's choice. Selection is restricted to the receiving module's conversion bundle; omitted UoMs do not become reachable merely because the compiler scanned their source definitions. Priority resolves the choice before assessing remaining ambiguity; a direct or explicitly chosen valid route is not an unresolved choice among every route in the source library.

### Argument evaluation and automatic conversions

H-148 requires left-to-right argument evaluation with each selected automatic conversion completed before evaluating the next argument. Each original argument is evaluated exactly once. This applies consistently to functions, procedures and overloaded operators, including their parenthesized call spelling.

A conversion function can fail a numeric check; its function context excludes dynamic-effect access. For `consume(source(0u64), { behave @print(99); 1 })`, if the first parameter converts through `target(100u64 / scalar(x))`, division by zero stops execution before `99` is printed. Function syntax does not permit moving a potentially failing conversion past another argument’s effects.

Overload and route selection remain compile-time operations. The compiler can collect the argument types before choosing the method, provided the emitted execution order follows H-148. Exact arguments need no conversion, and the callable body starts only after all arguments and their conversions finish.

Memory access follows D-10: functions may edit explicit writable inputs, and no automatic read-only snapshot spans nested calls or conversions. Borrowed arguments retain their storage provenance. H-201 const bits arguments retain the value evaluated at their argument position; H-202 const non-bits arguments are read-only views whose targets may change through other writable aliases. The prototype's scalar-by-value/reference subset remains recorded separately from the unimplemented qualifier syntax.

## Pure method dispatch and array integration

H-130 chooses Julia-style parameter-type method dispatch. Conversion permission is independently controlled by H-135's parameter annotation; exact-only parameters remain exact even when conversions are available. Conversion functions retain the function dynamic-context restriction, without implying a read-only snapshot. Operand expressions are evaluated once and conversion insertion must preserve the language's evaluation rules.

Illustrative method shapes, pending the complete numeric/array grammar:

```text
function consume(amount::f64(m)) { ... }
function consume_convertible(amount::f64(m*)) { ... }
function consume_via_cm(amount::f64(m*<-cm<-mm)) { ... }

convert_index(layout::PoolLayout, set::SetIndex, slot::LocalSlot)::GlobalSlot
getindex(a::SlotWeights, i::GlobalSlot)::Weight
getindex(a::NeuronRates, i::NeuronIndex)::Rate
```

The unit annotation supplies the unit identity and acceptance policy; it need not replace independent numeric-representation annotations. A nontrivial layout-dependent mapping can remain an explicit pure function with layout arguments. Automatic search cannot invent unavailable runtime inputs or dynamic resource bindings.

Whole-program compilation can specialize known method choices and conversion paths. The source forms select functions through argument types, not receiver-owned conversion operators. Normal method-definition conflicts remain distinct from H-136's warning about multiple valid conversion routes. No Julia runtime method mutation is implied; H-261 supplies the primitive lossless-widening policy, while the selected using/import extension distinction and mixin redefinition are recorded separately in D-07/D-10. [Julia methods](https://docs.julialang.org/en/v1/manual/methods/), [Julia conversion and promotion](https://docs.julialang.org/en/v1/manual/conversion-and-promotion/)

## Region-root constraints for transfer methods — design scope

H-206/H-208 extends the typed-method requirement to graph transfer: both source and destination region root types participate, abstract RT constraints can match concrete memory-policy implementations, and the selected method receives actual borrowed region references to invoke those implementations. This is a dependency of D-14's transfer contract, not an implicit change to UoM conversion routes or their selection scope.

The candidate method key includes both value/access types and both owner/root types. Matching an abstract root interface preserves the concrete RT and owner instance; it does not convert one region representation into another or imply covariance of writable regional storage. Owner identity must still prove that each supplied region owns its paired object. Same nominal root type alone is not sufficient.

H-209 confirms declaration-order-independent unique-best method selection. Among applicable candidates, narrower accepted argument tuples take precedence over broader ones; a nominal constraint narrower on each argument and strictly narrower on at least one dominates its fallback. Incomparable crossed specializations or equivalent distinct applicable signatures cannot be resolved by declaration order. No applicable method or no unique best method is an error. H-210/H-211 supplies nominal abstract-to-abstract <: relationships. Concrete data applicability requires explicit conformance under H-218, with import required to declare on an external ADT and using/import able to obtain an adapter's relations under H-220. Declaring conformance to two unrelated abstractions does not rank one above the other. No structural implication between independent abstract identities is inferred. The complete abstract/generic constraint language remains pending. [Julia method ambiguities](https://docs.julialang.org/en/v1/manual/methods/#Method-Ambiguities) documents the relevant crossed-argument case and order independence.

Type declares abstractions whose methods may be distributed across modules; no Interface body exists. Abstract <: inherits methods nominally; data <: explicitly declares conformance. No matching method set or data structure implicitly grants that relationship. No closed inventory of complete visible bodies is required; demanded method calls still require a visible valid implementation. Multiple abstract parents are deferred; a concrete type may declare several conformances through head-only declarations in different modules. The proposed generic root method retains R with a bound R <: RootPolicy rather than automatically asserting covariance of mutable region ::R. Declaration/body and bound syntax are reviewed in [D-07](DomainLanguageDesign.md#abstract-types-and-data-declarations). UoM identities and parameter-enabled conversion routes remain their existing mechanism; the unique-best dispatch rule does not select a new priority between subtype-applicable and conversion-enabled candidates or change conversion-route warnings into method tie breakers.

H-217 separates lower-priority implicitly selected method names from explicit bindings. The H-226 resolution order selects the operation by name before argument-type dispatch; an explicit alias with no matching overload reports an error rather than falling back to a hidden implicit name. This leaves UoM conversion paths and their warnings unchanged. Ordinary helpers inside conversions can receive H-227 caller-selected implementations, while compiler operation/intrinsic candidate lookup remains lexical. [Native evidence](../tests/cod/language/associated-method-validation.json) covers that distinction and procedure rejection inside conversion functions. Distributed abstract methods require visibility and inheritance rules independent of declaration order. Explicitly declared conformance in the module's visible adapter context supplies the relationship used for applicability; inherited candidates no longer need to prove it through circular structural inference.

Known concrete roots on demanded WPO paths permit static specialization; runtime root alternatives still need an explicit discrimination contract. Real owner arguments must be obtained from operand provenance with declared permissions, not ambient @region lookup. Lexical bits storage is not disguised as region ::RT. Full source examples and acceptance obligations are in [D-14](RegionalOwnership.md#root-policy-dispatch-and-actual-owner-arguments); the compiler supports explicit-conformance abstract parameter matching and unique-most-specific selection, followed by parameter-enabled UoM conversion where needed. The bounded compiler supports `where {region r::RootPolicy}` inferred from operand owners and explicit region parameters paired with `in r`. These do not choose runtime root discrimination, general mutable-region covariance or new conversion routes.

## Parenthesized operator method names

H-131 requests operators in parentheses as magic-method names, making operator definitions and overloads concise. Pure numeric/index operations use function methods and argument-type dispatch under H-130. H-154/H-155 also permit procedural overloads under the same operation spelling; their bodies may use effects, and a pure caller cannot select them. Parentheses delimit an operation name in a callable declaration, without introducing a CoD symbol, effect binding or separate runtime dispatch mechanism. H-158/H-159 preserve compiler-internal operation representation and current-binding invocation for a selected procedure. Conversion formulas remain pure functions regardless of their spelling.

Illustrative definition syntax, with helper names used only to show the body boundary:

```text
function (+)(i::SlotIndex, step::I64)::SlotIndex {
    shift_index(i, step)
}

function (-)(x::SlotDelta)::SlotDelta {
    negate_delta(x)
}

function (-)(a::SlotIndex, b::SlotIndex)::SlotDelta {
    index_difference(a, b)
}
```

The candidate call relationships are:

```text
i + step  ≡ (+)(i, step)
-x        ≡ (-)(x)
a - b     ≡ (-)(a, b)
```

Unary and binary methods share the parenthesized name and differ by argument count/types. No extra unary-magic spelling is needed. The overload family can express heterogeneous input/result types, including UoM products, rather than requiring every operator to return its operand type. These examples do not independently approve the SlotDelta type or its arithmetic policy.

For an eager operator, infix/prefix notation should resolve to the same method and preserve the same operand evaluation as its explicit call: each operand is evaluated once, and dispatch does not commute, duplicate or discard it. Parsing precedence and associativity belong to the expression grammar; a method overload does not silently change the parse of `a + b * c`. User-defined tokens/fixities and higher-order operator values remain separate scope, rather than implicit requirements of this naming form.

A consistent indexing-read candidate is:

```text
function ([])(a::SlotWeights, i::SlotIndex)::Weight {
    slot_read(a, i)
}

a[i] ≡ ([])(a, i)
```

This would make the array/index association a function method over explicit inputs. `([])` is a proposal to discuss, not an additional user-confirmed spelling. Under H-197/H-198, functions may modify explicit writable regional inputs; dynamic-context access and procedure calls remain excluded. Operator naming does not change that boundary. Short-circuit control and dynamic binding forms require their own evaluation semantics and cannot simply be rewritten as eager operator calls. Operator naming alone does not override these rules.

Acceptance should cover a declared binary method reached by both spellings, unary/binary arity resolution, heterogeneous UoM results, rejected index-domain combinations and preserved operand evaluation/grouping. The [native prototype](../tests/cod/language/README.md#intrinsic-and-procedural-operation-overloads) supports parenthesized eager operator names and their equivalent prefix/infix calls, plus typed intrinsic overloads of both callable kinds. Explicit numeric-operation methods can accept a nominal index value without enabling implicit brand erasure or adding an automatic unit-conversion edge. H-163/H-164 select the write form `a[i] = value` as `([]=)(a, i, value)`. Keeping the original array/container and index types permits the same nominal-type dispatch and conversion checks as explicit method calls. A selected procedural write remains forbidden in a pure function. The proposed indexing-read spelling `([])` and broader operator grammar remain outside that implemented subset.

## What the language precedents establish

Haskell `newtype` introduces a distinct type with the representation of its underlying type; wrapping/unwrapping can have no execution overhead. It is stronger than a type synonym. This supplies an identity mechanism, but does not itself specify useful arithmetic between different meanings. [Haskell 2010, §4.2.3](https://www.haskell.org/onlinereport/haskell2010/haskellch4.html)

The standard Haskell `Num` operations use one type for both operands and the result. That is awkward for a multiplication whose inputs and output have three different semantic types. `GeneralizedNewtypeDeriving` can reuse an existing numeric instance, but reproduces its operation family rather than deriving dimensional relationships. This is a limitation of that interface, not a proof that Haskell or newtypes cannot support units; richer classes or libraries can supply another interface. [Haskell numeric classes](https://www.haskell.org/onlinereport/haskell2010/haskellch6.html), [GHC newtype deriving](https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/newtype_deriving.html)

F# makes unit expressions part of static typing: addition/subtraction preserve matching units, multiplication/division compose them, and unit expressions normalize algebraically. Unit parameters are erased at runtime. Its distinction between numeric representation and measure parameters is directly relevant to CoD's integer indices and floating-point data. [F# specification, chapter 9](https://fsharp.github.io/fslang-spec/units-of-measure/)

Unit-polymorphic inference has a formal basis beyond ad hoc overload generation. Kennedy's work develops dimension-polymorphic types, inference and dimensional invariance. CoD would still need its own bounded rules for finite-width integer operations, effects and regional values; those results do not establish a CoD implementation. [Programming languages and dimensions](https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-391.html), [Relational Parametricity and Units of Measure](https://www.microsoft.com/en-us/research/wp-content/uploads/1997/01/relationalparametricityandunitsofmeasure.pdf)

## Comparing the mechanisms

| Concern | Strict wrappers with individually defined operations | UoM arithmetic over nominal base measures |
| --- | --- | --- |
| Distinguish unrelated scalar meanings | Direct: each declaration is a distinct type. | Distinct base measures can prevent interchange; reducing every count to dimensionless would lose this. |
| Infer a new result meaning after multiplication/division | Requires a declared operator relation or an additional generic algebra. | Follows from unit products, quotients and cancellation. |
| Reuse sum, scale and numerical helpers | Requires suitable generic operation constraints, or repeated definitions. | Unit-polymorphic signatures can express common relationships once. |
| Prevent inappropriate operations on opaque IDs | Only explicitly supplied operations need exist. | Unrestricted quantity algebra permits expressions such as ID squared; an index/opaque-role boundary may still be needed. |
| Distinguish positions with different origins | Can use separate declarations and conversion functions. | Equal units alone do not distinguish local/global positions or point/difference roles. |
| Numeric literals and scalar offsets | Requires a literal/construction/overload policy. | Requires a measure-inference policy; a nonzero scalar cannot universally acquire the other operand's unit. |
| Runtime representation | Scalar wrapping can be erased. | Measure annotations can be erased. Neither implies that a real conversion has zero arithmetic cost. |
| Compiler and library work | Simple identity checking; usable generic arithmetic can transfer substantial work to operators/libraries. | Adds normalization and unit constraints; can remove many per-domain arithmetic declarations. |

The attractive UoM route is therefore more than replacing one tag spelling with another. It makes selected arithmetic relationships inferable. Its cost should be compared against the complete usable wrapper/operator system, not against a wrapper that has no arithmetic support.

## Applying units to the actual pool formulas

The following is semantic notation, not selected CoD grammar. `Set` and `Slot` are independently named count measures; they are not silently erased to dimensionless values. A quantity `Quantity{u64, Slot/Set}` has the integer representation of slots per set. Positions and their origins are discussed separately below.

For the [pool geometry](../applications/steady-snn/docs/ConnectivityScalingResearch.md#formula-based-indexing):

```text
A, q                  : Count{Set}       # A = q * 2^b
j, e, t, b            : dimensionless integer representations
c0 = 100              : Quantity{Slot/Set}
c1 = 144              : Quantity{Slot/Set}
beta = 2*c0 + c1       : Quantity{Slot/Set}  # numeric value 344
capacity(j)           : Quantity{Slot/Set}
set_prefix(j)         : Count{Set}
slot_prefix(j)        : Count{Slot}
k                     : class-local set displacement, measured in Set
l                     : local slot displacement, measured in Slot

slot_displacement = k * capacity(j) + l
```

UoM derives the result as `Slot`: `Set * (Slot/Set) + Slot`. Substituting a slot count for k produces a different unit and is rejected at the expected result. A wrapper-only solution needs a corresponding operation declaration or conversion helper. The computation still needs the correct class and origin; unit agreement is not that proof.

The prefix formula also type-checks without dropping units:

```text
slot_prefix(j) = beta*A - (((beta - c0*t)*A) >> e)
```

Every summed/subtracted term is a slot count. The shift amount is dimensionless; the shifted quantity retains its measure. Numeric rounding, range and shift validity require their own integer rules.

The inverse formulas reveal a useful boundary. After expressing a position as a coordinate from the appropriate origin:

```text
set_ratio  = (2*A - one_set - set_coordinate) / q
slot_ratio = (beta*A - one_slot - slot_coordinate) / ((beta/8)*q)
```

Both ratios are dimensionless, suitable for the positive-integer `floor_log2` used by the existing exact inverse. Treating q as an untyped scalar, or the numeric factor 344 as dimensionless, would obscure these relationships. The resulting class ordinal still needs its semantic domain and range; a logarithm does not manufacture an array-valid index.

The source-ID array's byte displacement is `slot_displacement * 4 * unit(Byte/Slot)`; the weight array uses `1 * unit(Byte/Slot)`. These have the same byte measure but different array origins. Arithmetic derives the displacement; array selection supplies the correct origin. There is no proposal to store unit tags in either array.

## What UoM alone does not establish

**Position versus displacement.** A local slot coordinate and a global slot coordinate share the Slot measure, but require different origins. A position plus a displacement yields a position, and a difference of positions yields a displacement; ordinary quantity algebra does not distinguish these roles. Affine-space libraries model this explicitly. This is a useful design precedent, not a requirement to introduce a general affine-space framework into CoD. [mp-units affine-space model](https://mpusz.github.io/mp-units/2.3/users_guide/framework_basics/the_affine_space/)

For this workload, a named index domain with a checked coordinate-conversion function may suffice. Whether that domain is a separate strict wrapper, a parameter of an index type or part of a unified scalar type remains open. An origin/domain identity must not be canceled merely because the underlying units match. A/B and parallel SoA arrays can intentionally share a coordinate domain while retaining distinct storage bases.

**Bounds and pool membership.** An index with the right type can still be negative, beyond the end, from another class with a different capacity, or invalid after arithmetic. The first increment need not solve arbitrary dependent proofs. Array access/construction must retain necessary checks unless the compiler proves them, and a class-local position needs its class supplied to the mapping. A brand is not a bounds proof.

**Representation and arithmetic.** u32/u64 and FP32/FP64 are choices independent of units. Overflow, truncation, narrowing and rounding remain observable. Integer division in the inverse formulas is deliberate; unit inference must not silently change it to real division. A type-preserving shift is not permission to wrap on overflow. Physical unit conversions, where requested, may need arithmetic and are not automatically exact.

**Information lost by algebraic cancellation.** Ratios with identical numerator/denominator measures become dimensionless in ordinary UoM. If two such ratios have distinct business roles, those roles require another identity constraint. This is a reason to retain strict types for appropriate values, not to disable useful cancellation everywhere.

## Practical choices to evaluate before implementation

| Topic | Useful candidate | Failure to avoid |
| --- | --- | --- |
| Base identity | H-128: independent named compile-time UoMs with first-class type identities; local aliases preserve identity, and symbols do not represent units. | Same spelling or representation merges unrelated domains. |
| Arithmetic | Initial named-unit methods and pure coordinate conversions; compound product/quotient normalization belongs to the H-132 roadmap. | Hand-written overloads for every routinely derivable product, or implicit reinterpretation of an index. |
| Literals | Distinguish scalar factors, measured constants and offsets; evaluate narrowly scoped inference for zero/step literals. | Allowing every bare number or numeric variable to acquire arbitrary units to make an expression type-check. |
| `index + scalar` | A deliberately defined ordinal-shift operation or contextual offset construction. | Quietly weakening the same-unit addition rule for all quantities. |
| Generics | Unit parameters for operations with an actual reusable relation, such as sums and scaling. | Reproducing Haskell's entire numeric hierarchy or adding a general theorem solver without a workload. |
| Non-arithmetic operations | Require explicit semantics for bit masks, ordinal extraction and logarithms; preserve measures where meaningful. | Automatically inheriting all integer operations on every index type. |
| ABI / storage | Erase compile-time units/brands; keep scalar widths and checked conversions explicit. | Runtime wrappers, extra SoA fields or an implicit serialization contract. |

Haskell's distinctions among nominal, representational and phantom parameter roles illustrate why generic containers must not silently allow semantic relabeling merely because layouts match. CoD needs the relevant invariant; it need not reproduce GHC's complete role system or coercion API. [GHC roles](https://ghc.gitlab.haskell.org/ghc/doc/users_guide/exts/roles.html)

## Working assessment and acceptance cases

UoM has a concrete advantage for the pool's count/stride/offset arithmetic and generic numeric helpers. H-128/H-130/H-134–H-138 establish independent UoM identities, one-shot using, pure method dispatch and parameter-controlled conversion paths as local design requirements. A promising candidate is **nominal base identities plus UoM arithmetic, with explicit index-domain conversions where origins or opaque roles matter**. This is an agent assessment for discussion, not a confirmed replacement for strict brands or an instruction to implement three independent type frameworks. The algebraic/equivalence and index-conversion details remain under discussion; the confirmed local UoM requirements do not need reconfirmation.

The full comparison/acceptance catalog below covers both the initial increment and the compound-UoM roadmap. Automatic dimensional inference and unit-polymorphic cases are deferred by H-132; the initial implementation uses explicit named-unit methods. Compare the source and inferred types for:

1. A pure function computing class capacity and returning a structured index conversion result.
2. A set displacement multiplied by slots-per-set, and an intentionally swapped Slot/Set operand.
3. Shared A/B neuron indices and shared source-ID/weight slot indices; a neuron/slot mix must fail.
4. Local-to-global conversion, including a local index from the wrong class or pool.
5. A unit-polymorphic sum, scalar scaling, and a dimension-changing product.
6. Zero, nonzero literals and runtime scalar offsets without implicit loss of domain identity.
7. Correctly typed out-of-range values, checked narrowing, overflow and positive-integer inverse arguments.
8. Representation inspection showing no per-element tag or wrapper allocation; actual conversion instructions remain visible.

The bounded implementation increment uses the confirmed call, using and conversion rules. Compound-UoM algebra remains planned for later and is not a prerequisite.

## Bounded native prototype

The H-260/H-261 increment implements independently named units with quantities represented by `i64`, `u8`, `u32`, `u64`, `f32` or `f64`. Unit identity is compile-time metadata; native values retain the scalar representation without a runtime unit tag. The following is runnable prototype syntax, not a complete unit algebra or permanent grammar:

```text
unit m;
unit cm;
conversion function cm_to_m(x::f64(cm))::f64(m) { m(scalar(x) / 100.0) }
function consume(x::f64(m*))::f64 { scalar(x) }
```

A constructor such as `m(2.0)` preserves its input scalar representation; `m(2u32)` uses the same identity with u32 bits. `scalar(x)` explicitly extracts it. Unit arithmetic uses declared pure methods such as `(+)`; merely sharing a scalar representation grants no implicit interchangeability. Compound-unit algebra remains deferred.

A receiving module selects `using units "units.cod" {m as metres, cm, mm};`. Each source is selected once in that receiver, and selecting an already selected identity through a re-export is also rejected. Associated conversion methods are selected only when both endpoint units belong to that bundle. Their bodies and private helpers retain source lexical bindings. Re-exports preserve identities and the selected conversion set. Ordinary resource using/import cannot bypass this bundle rule.

Automatic insertion searches the call site's selected conversion graph; the parameter's target and declared route aliases resolve in the defining module. An exact argument-tuple method is preferred before inspecting conversion routes. Otherwise there must be one most-specific applicable method under the confirmed H-261 widening rule; an overload whose required conversion step is unavailable is inapplicable and does not reject a different viable overload. When no alternative declaration applies, a missing declared step retains its diagnostic. Ambiguity between methods is an error distinct from ambiguity between routes for a chosen method. These scoped dispatch rules do not establish a complete generic inference/coherence theory.

Direct conversion takes priority; otherwise a supplied route is followed. Without a declared route, the implementation chooses the shortest available path, with source/bundle declaration order breaking equal lengths. Any other simple route produces a warning identifying the chosen route and one alternative witness. This deterministic warn-and-continue fallback does not assert numerical equivalence. `--warnings-as-errors` or `-Werror=unit-conversion` promotes that warning to a frontend error. A direct choice or explicit route avoids warnings about unrelated indirect alternatives; multiple conversion functions for the same selected step still require a warning.

Search visits a finite graph. After selecting a shortest route, excluding each chosen edge in turn finds an alternative witness without enumerating every possible cyclic walk. A disconnected cycle supplies no route. Conversions are emitted as function calls without dynamic-context access and evaluate the actual argument once. Each selected conversion finishes before the next argument is evaluated. Ordinary functions do not impose an implicit read-only snapshot on supplied memory.

[Native fixtures](../tests/cod/language/README.md) and their validation records cover exact/starred acceptance, direct and declared priorities, multiple paths, configurable warnings, omitted units, duplicate selections, nominal aliases, source-private helpers, purity and captured/current unit-valued effects. A generated case re-exports 32 units through 16 modules and applies 31 conversions while a back-edge is present. This is language/runtime evidence, not SNN modeling or a compound-UoM implementation.

### Numeric and indexing checkpoint

The bounded prototype exposes one-, four- and eight-byte unsigned carriers, signed `i64` and FP32/FP64. Identity is independent of representation; a constructor preserves its supplied scalar type. Calls and builtin mixed arithmetic apply H-261 lossless widening; narrowing and potentially lossy numeric casts remain explicit in the formula. Integer arithmetic overflow is checked; floating operators use native IEEE arithmetic. Float-to-integer casts truncate before range checking, including negative fractions whose truncated value is zero. Floating casts can round, so a numeric round-trip is not promised merely because unit identities are preserved. Literal range validation parses directly at the declared precision; an intermediate FP64 rounding must not reject an otherwise finite FP32 boundary value.

Native `for` and `fold` expressions traverse half-open integer ranges, including integer-backed unit ranges. A fold's accumulator must preserve its concrete type and, for a regional view, its owner and any admitted abstract interface. It can carry a record or array view directly; each iteration updates or selects existing storage, and the body result must satisfy the accumulator's initial guarantees. Several related intermediate fields can live in one caller-allocated data object; functions update that explicit storage and return its borrowed view. The typed pool example reuses one regional SetLocation result slot, while scalar/UoM reductions remain scalar values. This supports bounded layout formulas without requiring compound-unit algebra.

The separate unit/layout/index modules in [pool-typed.cod](../tests/cod/language/pool-typed.cod) exercise the existing 16-class worked calculation. `ClassId`, `ClassSetId`, `GlobalSetId`, `LocalSlot`, `GlobalSlot`, `SetCount`, `SlotCount` and `SlotsPerSet` have distinct identities. A pure one-input conversion widens `SlotsPerSet` to `SlotCount`; multiplication dispatch consumes that conversion through a starred parameter. Layout-dependent prefix/round-trip mapping remains an explicit pure function, so automatic search invents no hidden layout input. Native checks reproduce the published totals and class boundary/median mappings; a global set ID passed to a class-local parameter is rejected. The arithmetic example does not approve that candidate as the final SNN pool or add auxiliary topology storage.


### Typed regional arrays and measured cost

The native subset declares arrays such as `array Sources[u64(GlobalSlot)]::u32(Neuron)` and `array Weights[u64(GlobalSlot)]::u8`. Indexing requires the declared unit identity; related arrays can share that identity while retaining separate scalar buffers. Unit tags and conversion registries have no per-element runtime representation. This does not prove class membership or array bounds: runtime indexing still checks the selected array's length.

[region-gather.cod](../tests/cod/language/region-gather.cod) connects the pool index formulas to regional SoA arrays. A library function allocates and initializes the graph in an explicitly supplied region; another function performs each A/B update using the FP32 gather. The entry program holds the region and uses a lexical procedure for tracing. Layout and index-result storage are allocated once; the update loop makes no allocations.

[Native measurements](../tests/cod/language/explicit-library-validation.json) compare both scales against an independent rounded FP32 reference:

| Arithmetic rows | Connection slots | Updates | Array-element bytes | Other occupied bytes | Total occupied bytes |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 4,095 | 693,504 | 8 | 3,500,280 | 168 | 3,500,448 |
| 655,350 | 112,281,600 | 1 | 566,650,800 | 160 | 566,650,960 |

For the J16,q10 arithmetic case, four-byte source IDs and one-byte weights occupy 561,408,000 bytes in separate arrays. Two FP32 state arrays occupy 5,242,800 bytes. The remaining measured bytes cover native region/array metadata, the graph record, layout, index-result storage and allocation alignment; they are not a proposed application bookkeeping budget. The row count is one per slot set, not a selected CC neuron count. These measurements define no neuron transition, training rule or semantic-vector-table cost. The [earlier full-pool report](../tests/cod/language/gather-full-pool-validation.json) retains its own input revision and layout.

[numeric-validation.json](../tests/cod/language/numeric-validation.json) records independent FP32/exact-integer comparisons over typed regional inputs. Literal tests distinguish representable subnormals and rounding-boundary values from actual underflow/overflow. These checks substantiate the bounded implementation; generic unit-polymorphic inference, compound algebra, richer conversion coherence and the full design contract remain open.
