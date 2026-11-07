# Scalar units, strict brands and array index domains

Discussion revision: 13 · Recorded: 2026-11-06 · Status: alternatives under discussion.

## Design authority

- **Registry / scope:** [D-11](DesignAuthorityRegistry.md#d-11), revision 13, all sections; the mechanism and complete implementation contract are `pending-human`. [K-292](DesignAuthorityRegistry.md#k-292) records H-163/H-164’s indexed-write call shape without changing unit identity or conversion rules.
- **Value lineage:** C-47/P-112 derives strict semantic scalar identities and typed indexing from C-10's explicit language semantics; D-11/P-113 develops that requirement into usable arithmetic. The candidate path continues through C-10/P-14/C-01 to inspectable typed business intent. Detecting an incorrect coordinate conversion before execution is the concrete benefit; creating a general numeric framework without a demonstrated use is not.
- **Dependencies:** P-118 refers to C-13's static-pool formulas as an example, not authority to select an application configuration. C-49/P-119/P-120 record independent UoM identity and reuse; C-50/P-121–P-123 record pure conversion methods and argument-type dispatch; C-51/P-124–P-128 record parameter-controlled conversion, selected-bundle dependency and purity/identity/sequencing constraints. Existing compile-time origin/alias rules inform alias behavior without using symbols as UoMs or choosing a type-identity encoding.
- **Human baseline:** H-123 requires structured types and asks about scale-law index support while excluding SNN modeling from the coming increment. H-125 requires strict branded types, an array-declared index type and customizable scalar arithmetic, and requests UoM/newtype comparison. H-128 requires independent compile-time UoM entities, first-class type identities and identity-preserving local aliases, with no representation by symbols. H-130 supplies pure conversions and Julia-style argument-type method dispatch. H-134 specifies fresh definitions plus one-shot using selections with associated conversions; UoM import is not supported. H-135 makes automatic conversion a parameter annotation, H-136 specifies compile-time route discovery and configurable ambiguity warnings, and H-137/H-138 specify source-declared indirect routes with direct conversion taking priority. H-148 requires each argument’s conversion to complete before evaluating the next argument. H-131 adds parenthesized operator names as syntax sugar for readable overloaded methods. H-132 keeps compound UoMs in the roadmap for physical-quantity computation and defers their immediate implementation. H-126 requires a substantive comparison before choosing the arithmetic foundation; it does not accept the agent's proposed newtype-first boundary.
- **Invariants / non-goals:** distinct unit identities stay distinct; any automatic conversion must be enabled by the parameter and use selected pure conversion functions; no user heap, runtime unit dictionary, per-slot tag or heap-built topology follows. Preserve SoA and regional relocation. This document chooses no neural law, learning algorithm, pool configuration, general GADT implementation or complete operator grammar. Function access to regional snapshots is a separate subject in D-10 under H-124/H-127/H-129.
- **Acceptance:** compare ordinary formulas, array access, literals, generic numeric functions, local/global positions, conversions and invalid cases; identify both what the checker proves and what remains a value/range condition. Implementation evidence must eventually cover compile-time diagnostics, native results and representation cost.
- **Operation boundary:** H-153–H-155 extend parenthesized naming to intrinsic operations and allow function/procedure overloads under one spelling. H-158 keeps intrinsic representation separate from CoD symbols; H-159 selects current-binding procedure invocation. [D-10](SymbolicDynamicScope.md#operator-and-intrinsic-overloads) records P-136/P-137. Conversion formulas and pure index mappings remain functions under H-130.
- **Assessment:** [K-285](DesignAuthorityRegistry.md#k-285) records revision 12, reconciling the general operation boundary with the unchanged pure-conversion requirement. [K-260](DesignAuthorityRegistry.md#k-260) records revision 11, clarifying snapshot boundaries around argument conversions under H-129/H-133/H-148 without selecting a new lifetime mechanism. [K-247](DesignAuthorityRegistry.md#k-247) records revision 10 and H-148’s per-argument conversion order. [K-240](DesignAuthorityRegistry.md#k-240) records revision 8, aligning operator support and integer-overflow wording with the bounded native implementation. [K-230](DesignAuthorityRegistry.md#k-230) records revision 7, typed regional array integration, conversion-candidate selection and precision/representation evidence. [K-227](DesignAuthorityRegistry.md#k-227) records revision 6 and actual typed pool-index/numeric acceptance. [K-226](DesignAuthorityRegistry.md#k-226) records revision 5 and the bounded native UoM implementation/evidence. [K-223](DesignAuthorityRegistry.md#k-223) records revision 4 and the one-shot using/conversion decisions; K-222 records the compound-UoM deferral; K-221 records the operator-method discussion; K-220 records the preceding identity/arithmetic and snapshot boundaries. No prior qualified D-11 baseline exists; this is not an implementation approval or continuity-based qualification.

## Initial increment and compound-UoM roadmap

H-132 requests compound UoMs in the plan, particularly for future physical-quantity calculations, without requiring their immediate implementation. The initial work uses independently identified named UoMs, typed scalar/index values, pure conversion methods and parenthesized operator names. It does not need automatic products, quotients, powers or algebraic unit normalization.

For example, the first pool program can define named `SetCount`, `SlotsPerSet` and `SlotCount` domains and an explicit multiplication method connecting them. That provides typed arithmetic without yet requiring the compiler to derive a compound result such as `Set * (Slot/Set)`. The compound notation in the research comparison below describes future capability, not an implementation prerequisite.

The roadmap retains unit-expression products/quotients, powers where useful, normalization, unit-polymorphic inference and the distinction between algebraic aliases and fresh nominal identities. It must preserve H-128 identity and H-134 using-selection semantics. No solver, implicit conversion graph, automatic scaling policy or physical-unit catalog is selected merely by retaining that roadmap.

The immediate choices are recorded in [the language increment review](DomainLanguageDesign.md#immediate-increment-decisions): existing regional references use the calling procedure scope, UoMs use one-shot selections rather than definition import, and parameter annotations control automatic conversion and route preference. Compound-unit equivalence and inference are deferred rather than blockers for this run.

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

H-134 specifies two UoM mechanisms: create a fresh definition, or select existing definitions with `using`. There is no UoM `import`. Different conversion behavior calls for a new definition rather than copying and rebinding a source definition. Procedure `using`/`import` semantics remain separate and unchanged.

A `using` selection brings the selected UoMs and their associated conversion functions together. It may select only part of a source module. A conversion involving an unselected UoM is not added to the receiving module's available conversion set. Repeated using is not an incremental way to extend that set: select the intended bundle in one declaration. Local aliases preserve selected identities.

For a source exporting `m`, `cm`, `mm` and `km`, selecting `{m, cm, mm}` exposes conversions among the selected units and excludes conversions involving `km`. Selecting just `{m, cm}` cannot make an indirect route through `mm` available to the receiver's automatic search. A source-bound conversion body can still depend on its own implementation helpers; those dependencies do not independently expose more conversion candidates to the caller.

The bounded interpretation is local to the receiving module: each source selection is made once, rather than progressively reopened, and another receiving module may select its own bundle. A second selection from the same source or duplicate selection of an already included UoM is diagnosed. This is declaration composition, not a global prohibition against multiple modules using the same UoM identity. Exact batch spelling is an implementation choice within H-134's identity and selection rules.

Conversion formulas keep their source lexical bindings and remain pure functions. Layout or array data required by an operation must be explicit inputs or valid fixed lexical data; a conversion cannot fetch a hidden dynamic region, access effects or call a procedure. Declared units remain compile-time entities. Scanning and cross-module encounters follow D-10 rather than an implicit exemption for unit declarations.

## Parameter-controlled automatic conversion

H-135 makes acceptance a declaration-site choice:

| Parameter annotation | Meaning |
| --- | --- |
| `amount (m)` | Require the `m` identity; do not insert a conversion for this parameter. |
| `amount (m*)` | Permit automatic conversion to `m` using the selected pure conversion functions. |
| `amount (m*<-cm<-mm)` | Permit conversion to `m`; a direct conversion wins, otherwise use the declared indirect route `mm → cm → m` when it connects the supplied unit to `m`. |

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

A pure conversion can fail a numeric check even though it cannot mutate regions or invoke effects. For `consume(source(0u64), { behave @print(99); 1 })`, if the first parameter converts through `target(100u64 / scalar(x))`, division by zero stops execution before `99` is printed. Purity does not permit moving a potentially failing conversion past another argument’s effects.

Overload and route selection remain compile-time operations. The compiler can collect the argument types before choosing the method, provided the emitted execution order follows H-148. Exact arguments need no conversion, and the callable body starts only after all arguments and their conversions finish.

Snapshot lifetime follows D-10. A conversion nested inside a running function shares that function tree’s read-only interval. When a procedure evaluates a function-call expression, each conversion returns before later argument effects; the called function’s read-only interval begins after all arguments and conversions finish. Earlier bits arguments retain their evaluated values, while a regional view exposes the storage contents at that interval. No copied snapshot or extra lifetime resource is implied.

## Pure method dispatch and array integration

H-130 chooses Julia-style parameter-type method dispatch. Conversion permission is independently controlled by H-135's parameter annotation; exact-only parameters remain exact even when conversions are available. Pure conversion calls preserve the outer function's snapshot context when nested within it. Operand expressions are evaluated once and conversion insertion must preserve the language's evaluation rules.

Illustrative method shapes, pending the complete numeric/array grammar:

```text
function consume(amount (m)) { ... }
function consume_convertible(amount (m*)) { ... }
function consume_via_cm(amount (m*<-cm<-mm)) { ... }

convert_index(layout::PoolLayout, set::SetIndex, slot::LocalSlot)::GlobalSlot
getindex(a::SlotWeights, i::GlobalSlot)::Weight
getindex(a::NeuronRates, i::NeuronIndex)::Rate
```

The unit annotation supplies the unit identity and acceptance policy; it need not replace independent numeric-representation annotations. A nontrivial layout-dependent mapping can remain an explicit pure function with layout arguments. Automatic search cannot invent unavailable runtime inputs or dynamic resource bindings.

Whole-program compilation can specialize known method choices and conversion paths. The source forms select functions through argument types, not receiver-owned conversion operators. Normal method-definition conflicts remain distinct from H-136's warning about multiple valid conversion routes. No Julia runtime method mutation, automatic promotion policy or module keyword semantics is imported wholesale. [Julia methods](https://docs.julialang.org/en/v1/manual/methods/), [Julia conversion and promotion](https://docs.julialang.org/en/v1/manual/conversion-and-promotion/)

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

This would make the array/index association a pure method over explicit snapshot inputs. `([])` is a proposal to discuss, not an additional user-confirmed spelling. Mutation hooks remain subject to procedure-only effects and do not become pure functions through naming sugar. Short-circuit control and dynamic binding forms require their own evaluation semantics and cannot simply be rewritten as eager operator calls. Operator naming alone does not override these rules.

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

The H-122/H-123 increment implements named units over `i64`, `u8`, `u32`, `u64`, `f32` or `f64`. Unit identity is compile-time metadata; native values retain the scalar representation without a runtime unit tag. The following is runnable prototype syntax, not a complete unit algebra or permanent grammar:

```text
unit m::f64;
unit cm::f64;
conversion function cm_to_m(x (cm))::m { m(scalar(x) / 100.0) }
function consume(x (m*))::f64 { scalar(x) }
```

A constructor such as `m(2.0)` requires the exact scalar representation. `scalar(x)` explicitly extracts it. Unit arithmetic uses declared pure methods such as `(+)`; merely sharing a scalar representation grants no implicit interchangeability. Compound-unit algebra remains deferred.

A receiving module selects `using units "units.cod" {m as metres, cm, mm};`. Each source is selected once in that receiver, and selecting an already selected identity through a re-export is also rejected. Associated conversion methods are selected only when both endpoint units belong to that bundle. Their bodies and private helpers retain source lexical bindings. Re-exports preserve identities and the selected conversion set. Ordinary resource using/import cannot bypass this bundle rule.

Automatic insertion searches the call site's selected conversion graph; the parameter's target and declared route aliases resolve in the defining module. An exact argument-tuple method is preferred before inspecting conversion routes. Otherwise there must be one applicable method; an overload whose required conversion step is unavailable is inapplicable and does not reject a different viable overload. When no alternative declaration applies, a missing declared step retains its diagnostic. Ambiguity between methods is an error distinct from ambiguity between routes for a chosen method. These dispatch details are bounded implementation conventions for review, not a general inference/coherence theory.

Direct conversion takes priority; otherwise a supplied route is followed. Without a declared route, the implementation chooses the shortest available path, with source/bundle declaration order breaking equal lengths. Any other simple route produces a warning identifying the chosen route and one alternative witness. This deterministic warn-and-continue fallback does not assert numerical equivalence. `--warnings-as-errors` or `-Werror=unit-conversion` promotes that warning to a frontend error. A direct choice or explicit route avoids warnings about unrelated indirect alternatives; multiple conversion functions for the same selected step still require a warning.

Search visits a finite graph. After selecting a shortest route, excluding each chosen edge in turn finds an alternative witness without enumerating every possible cyclic walk. A disconnected cycle supplies no route. Conversions are emitted as pure calls and evaluate the actual argument once. Their snapshot boundary follows the enclosing call context described above; it does not extend across later argument effects in a procedure.

[Native fixtures](../tests/cod/language/README.md) and their validation records cover exact/starred acceptance, direct and declared priorities, multiple paths, configurable warnings, omitted units, duplicate selections, nominal aliases, source-private helpers, purity and captured/current unit-valued effects. A generated case re-exports 32 units through 16 modules and applies 31 conversions while a back-edge is present. This is language/runtime evidence, not SNN modeling or a compound-UoM implementation.

### Numeric and indexing checkpoint

The bounded prototype exposes one-, four- and eight-byte unsigned carriers, signed `i64` and FP32/FP64. Identity is independent of representation; a constructor requires its declared scalar type, and numeric casts remain explicit in the pure formula. Integer arithmetic overflow is checked; floating operators use native IEEE arithmetic. Float-to-integer casts truncate before range checking, including negative fractions whose truncated value is zero. Floating casts can round, so a numeric round-trip is not promised merely because unit identities are preserved. Literal range validation parses directly at the declared precision; an intermediate FP64 rounding must not reject an otherwise finite FP32 boundary value.

Native `for` and `fold` expressions traverse half-open integer ranges, including integer-backed unit ranges. A fold's accumulator must preserve its type; named product accumulators express several related intermediate values without a heap object or extra model state. This supports bounded layout formulas without requiring compound-unit algebra.

The separate unit/layout/index modules in [pool-typed.cod](../tests/cod/language/pool-typed.cod) exercise the existing 16-class worked calculation. `ClassId`, `ClassSetId`, `GlobalSetId`, `LocalSlot`, `GlobalSlot`, `SetCount`, `SlotCount` and `SlotsPerSet` have distinct identities. A pure one-input conversion widens `SlotsPerSet` to `SlotCount`; multiplication dispatch consumes that conversion through a starred parameter. Layout-dependent prefix/round-trip mapping remains an explicit pure function, so automatic search invents no hidden layout input. Native checks reproduce the published totals and class boundary/median mappings; a global set ID passed to a class-local parameter is rejected. The arithmetic example does not approve that candidate as the final SNN pool or add auxiliary topology storage.


### Typed regional arrays and measured cost

The native subset declares arrays such as `array Sources[GlobalSlot]::Neuron` and `array Weights[GlobalSlot]::u8`. Indexing requires the declared unit identity; related arrays can share that identity while retaining separate scalar buffers. Unit tags and conversion registries have no per-element runtime representation. This does not prove class membership or array bounds: runtime indexing still checks the selected array's length.

[region-gather.cod](../tests/cod/language/region-gather.cod) connects the pool index formulas to real regional arrays and a pure FP32 gather called by an A/B update procedure. The [full-pool arithmetic measurement](../tests/cod/language/gather-full-pool-validation.json) covers 655,350 sets and 112,281,600 slots for the existing J16,q10 example. Four-byte source IDs plus one-byte weights occupy 561,408,000 bytes. Two four-byte state arrays with one arithmetic row per set occupy 5,242,800 bytes; headers/root/allocation alignment add 128 bytes in this generated layout. The total is 566,650,928 bytes. The benchmark row count is not a selected CC neuron count, and the measurement does not redefine the application memory budget or add a semantic-vector table.

[numeric-validation.json](../tests/cod/language/numeric-validation.json) records independent FP32/exact-integer comparisons over typed regional inputs. Literal tests distinguish representable subnormals and rounding-boundary values from actual underflow/overflow. These checks substantiate the bounded implementation; generic unit-polymorphic inference, compound algebra, richer conversion coherence and the full design contract remain open.
