# Porting Julia packages to CoD with `julia.cod`

Research WIP · revision 4 · 2026-11-26

An agent porting a one-sample t-test should be able to recognize the sample, compute the same statistic and ask for the same tail probability in CoD. A second agent porting a distance function should be able to reuse those array operations without learning which C library supplies a probability. These two ports test whether a small CoD package can make important Julia algorithms economical to bring over and maintain.

## Design authority

At r4 [D-11-NUMERIC-CONVERSION](DesignAuthorityRegistry.md#d-11-numeric-conversion) applies the project user’s Julia-alignment direction to explicit scalar conversions, P-774/P-775. The r3 array and operator selections and both port workloads continue. Provider and package implementation remain research. [Decision and scope](ScalarUnitsAndIndexTypes.md#explicit-scalar-numeric-conversions)

At r3 [D-29-STEPPED-RANGES](DesignAuthorityRegistry.md#d-29-stepped-ranges) and [D-30-TYPED-SUBSCRIPTS](DesignAuthorityRegistry.md#d-30-typed-subscripts) settle the porting notation and per-axis call boundary. The r2 decisions on library array policy, CoD colon syntax and type names carry forward. [User confirmation](../CHANGELOG.md#h-ranges-and-typed-subscripts-20261126)

At r2 the project user delegated the concrete library boundaries and confirmed distinct signed and unsigned integer identities and manual porting of Julia ranges. [D-30-ARRAY-POLICY](DesignAuthorityRegistry.md#d-30-array-policy), [D-10-LIBRARY-FIXITY](DesignAuthorityRegistry.md#d-10-library-fixity) and [D-07-TYPE-SYNONYMS](DesignAuthorityRegistry.md#d-07-type-synonyms) cover the decisions below. P-758–P-763 connect them to reviewable domain operations and their prerequisites. The two port workloads carry forward from r1. [Decision and scope](../CHANGELOG.md#h-library-arrays-fixity-and-type-names-20261126)

The project user commissioned research on native C access to Julia's Rmath library and a Julia-like array interface for agent-assisted ports. Direct compilation of Julia source is outside the goal. The user also invited alignment in CoD itself wherever Julia's semantics fit and specifically reopened `x : f64` for that purpose. This preference applies when choosing ordinary CoD operations as well as package APIs. The numerical conversion rule is specified. The provider extension and numerical package APIs below remain proposals. The confirmed r2 boundaries govern the array and source-vocabulary examples. No port has been executed for this revision. [User decision](../CHANGELOG.md#h-julia-package-portability-20261126) · [Registry and scope](DesignAuthorityRegistry.md#d-julia-pkg-research)

The charter calls for local, reviewable edits by people and agents. Reusing a checked operation across packages lets a reviewer inspect the domain algorithm and check its assumptions about values, storage and lifetimes. The candidate value path is P-756. P-757 identifies the conversion, owner, collection and native-call contracts that constrain the two workloads. [Charter §§1, 4.3 and 8](RegionalSemanticPlatformCharter.md) · [Regional arrays](CollectionsAndBuiltinContainers.md#fold-and-array-access) · [Native C providers](../applications/cod-booter/docs/LLVMInterop.md#generated-c-providers-for-the-first-booter)

## Where to align with Julia

For each recurring Julia operation, write down an input that distinguishes its behavior and identify who else would benefit from that behavior in CoD. A good general meaning belongs on the ordinary language or standard-library path when it fits CoD's other callers. A Julia array coordinate convention can live on a particular array type. A distribution's tail computation belongs with the numerical package. When the proposed meaning touches an existing guarantee, record the program whose result, access or diagnostic would change. This lets the next package author reuse the decision.

### Explicit conversion to binary64

For an `i64` sample count of 9,007,199,254,740,993, Julia's `Float64(x)` and CoD's explicit `x : double` specify 9,007,199,254,740,992. Both round directly to binary64 with ties to even. The converse conversion to a builtin integer requires an exact result and can raise `InexactError`. These are ordinary CoD operations available to every library. [Julia numerical conversions](https://docs.julialang.org/en/v1/manual/mathematical-operations/#Numerical-Conversions) · [Julia rounding modes](https://docs.julialang.org/en/v1/manual/integers-and-floating-point-numbers/#Rounding-modes) · [CoD destination rules](ScalarUnitsAndIndexTypes.md#explicit-scalar-numeric-conversions)

| Integer input | Specified `f64` result, written as its exact mathematical value |
| --- | --- |
| `2^53 − 1` | `2^53 − 1` |
| `2^53 + 1` | `2^53` |
| `2^53 + 3` | `2^53 + 4` |
| `−(2^53 + 1)` | `−2^53` |
| `2^64 − 1` as `u64` | `2^64` |

For delivery, compare folded and runtime operands against the bits from Julia’s `reinterpret(UInt64, Float64(x))`. Check exact and inexact integer destinations, including failure before a later operand. Precision advice for explicit floating rounding is opt-in. [Scalar acceptance](ScalarUnitsAndIndexTypes.md#explicit-scalar-numeric-conversions) · [Implicit compatibility](NamedQuantitiesAndNumericCompatibility.md#whole-type-lossless-compatibility)

## Library-defined arrays and the native C boundary

An array type's library specifies its index domains and origins, rank, logical traversal, physical layout, and what slicing, indexed assignment, copying and taking a view do. CoD supplies operation selection, type and owner identity, access, lifetime and failure propagation. A view must retain the actual backing owner and its allowed access. A detached result needs suitable storage. A selected indexing operation can compute a value without exposing an address. [Logical indexed transfer](RegionalOwnership.md#logical-indexed-transfer)

Publish the bundled native-interoperability array vocabulary in `CArray.cod`, under the name `CArray`, with explicit module selection. A project with no such selection gets no implicit `CArray`. The core and bundled defaults leave `Array` available for `julia.cod` to export under ordinary module scope. The initial C-facing workload needs a zero-based contiguous span with its extent, element representation and existing borrow/owner contract. A two-dimensional C array laid out as arrays of rows and a Julia dense matrix laid out by columns can describe the same logical shape with different strides. Crossing a C boundary requires the layout, stride, mutability and lifetime promised by that particular declaration. A strided column can go straight to a stride-aware provider. A contiguous-only provider needs a compatible view or an explicit copy with defined mutation behavior.

The booter's named regional arrays and native byte loans use the existing bootstrap representation. Migrate these as actual C-facing or internal consumers require. Preserve distinct nominal arrays, half-open source spans, generated C interfaces, bounds failures and cleanup across the cutover. A universal replacement of `Visits` and `SourceBytes` by one type would lose overload and access information. Acceptance must distinguish a program importing neither library, either one, and both. With both selected, write through a Julia view, mutate a detached numeric slice and hand a compatible C span to a real provider. [Booter’s current array use](https://github.com/complyue/cod-project/blob/5b0c4c5fb88d8857c16d82b12fd79a83f51cf238/applications/cod-booter/src/ModuleIndex.cod#L54-L65)

### Stepped subscripts

Port Julia’s `1:2:5` as `1 ..= 5 by 2`. A Python half-open slice with start 1, stop 5 and step 2 uses `1 .. 5 by 2`, with the selected array's own origin and bounds rules. Multidimensional subscripts take a comma-separated list of typed axis arguments. For the distance workload, `X[.., 1 ..= 3 by 2]` supplies the full first axis and columns 1 and 3. To preserve Julia's explicit `view`, use the library’s borrowing operation, for example `view_columns(X, 1 ..= 3 by 2)`. Ordinary slice access may produce a detached copy. [Range contract](RangedForAndGenerators.md#stepped-range-values) · [Subscript overloads and units](CollectionsAndBuiltinContainers.md#multidimensional-subscript-overloads)

The bootstrap readers at the decision baseline accept one expression between brackets. Completing the range expression and multi-axis call is frontend work. [Seed bracket reader](https://github.com/complyue/cod-project/blob/3f7e048ce40a12069e8ad225899dcd53640b37ea/cod/lang_syntax.hh#L2411-L2416) · [Booter bracket reader](https://github.com/complyue/cod-project/blob/3f7e048ce40a12069e8ad225899dcd53640b37ea/applications/cod-booter/src/ExpressionParser.cod#L2281-L2300)

On the integer axes of this matrix, the selector step counts logical positions. The backing stride measures the storage distance between adjacent axis positions. For a dense two-row column-major matrix, selecting columns 1 and 3 gives a view with four elements between column starts. For an affine view the resulting stride is the parent stride times the selector step. The integer range contract determines its progression and rejects zero step. The library specifies axis bounds, result axes and copy versus view. A writable view preserves access to its backing and must respect its actual lifetime. Verify reversed selection on a subview, empty selection without a dereference, and a write observed through the parent. Native handoff also checks the provider’s layout and extent requirements.

## Operator grouping belongs to the selected vocabulary

A library can specify precedence and associativity for its expression operators. Use Julia’s grouping as the reference in `julia.cod`. The compiler owns stable token spans, structural delimiters, selection of one visible grouping rule, and application of the resulting syntax to typed operations. A method overload cannot change a grouping after operand types become known. Fix the interpretation of a definition in its own selected vocabulary so an importer cannot silently regroup its body. Conflicting rules active for the same use require an explicit selection or a located diagnostic. Preserve each operation’s evaluation and cleanup rules. [CoD operator tokens](SymbolicDynamicScope.md#symbolic-operator-tokens) · [Julia precedence and associativity](https://docs.julialang.org/en/v1/manual/mathematical-operations/#Operator-Precedence-and-Associativity)

For example, `a + b * c` must select multiplication inside addition in the Julia vocabulary, `a - b - c` must group left and `a ^ b ^ c` must group right. Supporting the power spelling also needs the symbol in the scanner’s operator alphabet. A chained comparison that evaluates its middle operand once or a fused broadcast needs an operation describing that behavior. A binary precedence number alone cannot express it. The public declaration spelling and implementation of general user-declared fixity remain production compiler work. The bootstrap frontends can support the selected subset that the booter uses. [Production parser planning](../docs/DesignTopics.md#production-parser-combinators)

Keep CoD’s colon grammar, including `value:Type`. When porting Julia, rewrite `lo:hi` and `lo:step:hi` explicitly, including ranges inside subscripts. Select a library range or index selector with the intended endpoints and stride. CoD’s half-open `..` is useful when the algorithm already has a past-the-end bound. For Julia’s `1:2:5`, preserve the sequence 1, 3, 5. Check empty and descending ranges and a last value at the maximum integer. Blindly adding one to the last value can overflow. For the Julia full-axis selector in `A[:,2]`, write `A[.., 2]` on an array with the intended coordinates and slice contract. Selecting `julia.cod` leaves the parse of `byte:ASCII` intact.

## Scalar names and transparent type synonyms

Prefer LLVM IR names where the source meaning matches. Use `float` and `double` for supported binary32 and binary64. Preserve the values and type identities of existing `f32` and `f64` spellings during migration. CoD `i64` selects signed arithmetic and `u64` unsigned arithmetic. Both lower to LLVM `i64`; comparison, division and conversion select the appropriate instructions. CoD Boolean conditions and ownership-carrying references keep their source rules. LLVM's `i1` and `ptr` representations alone cannot confer those semantics. This decision does not expose every LLVM internal type or change literal suffixes. [LLVM integer types](https://llvm.org/docs/LangRef.html#integer-type) · [LLVM floating-point types](https://llvm.org/docs/LangRef.html#floating-point-types)

Prioritize transparent synonyms of already established types. A package can publish:

```cod
export type Float64 = double
export type Int64 = i64
export type UInt64 = u64
```

Resolve the right-hand type in the defining module. A synonym carries its canonical identity, including any nominal or unit distinctions, through imports, re-exports, signatures and generated C layout. A synonym introduces no conversion, storage or overload. Two overload heads differing only by synonymous type names are duplicates, while overloads for `Int64` and `UInt64` remain distinct. Diagnose unresolved targets and synonym cycles at their declarations. Preserve the spelling at a use and the canonical identity in diagnostics. Acceptance includes one imported synonym chain, overload selection, duplicate rejection and identical native representation. A module can define both `Array` and its element type names without a global Julia prelude. Simple synonyms are an early frontend deliverable. Parameterized aliases and higher-kinded synonyms for channel pipelines have separate workloads. [Module identities](ModuleWorlds.md) · [Higher-kinded type synonyms](../docs/DesignTopics.md#higher-kinded-type-synonyms)

## Reuse the C numerical library

`Rmath.jl` supplies Julia names, defaults, C argument conversions and random-number hooks over `libRmath-julia`, an independently buildable variant of standalone Rmath. A CoD package can use the same C source without embedding a Julia or R runtime. Keep the array package usable on its own. Pin the native source and the bindings used for comparison when building a prototype. Upstream identifies its Julia bindings as MIT and the underlying C library as GPLv2. Binary packaging must account for the chosen native distribution. [Rmath-julia source and build](https://github.com/JuliaStats/Rmath-julia/blob/982eb93bca6b655d8b433af23ee847561694e2b2/README.md) · [Julia bindings](https://github.com/JuliaStats/Rmath.jl/blob/50280d872a9626c6c99c7f15a658d7b152f6c9f3/src/Rmath.jl) · [Upstream license notice](https://github.com/JuliaStats/Rmath.jl/blob/50280d872a9626c6c99c7f15a658d7b152f6c9f3/LICENSE.md)

The first provider workload is one scalar operation. Have a CoD caller request `logccdf(Normal(), 40)`. A checked CoD declaration supplies the generated provider boundary. Handwritten C converts the tail and log choices to the flags expected by `pnorm5(40, 0, 1, 0, 1)`. The returned value should be about `−804.6084420137538`. Computing the lower CDF first gives `1.0` in binary64 here and loses the right tail. Also exercise the ordinary lower CDF at zero (`0.5`) and the log upper tail at positive infinity (negative infinity). Native transport failure is reported through the provider status. A successful floating-point result can itself be infinite.

At the inspected CoD revision the booter's provider generator can spell `f64` as C `double`. Its declaration checker admits ordinary CoD-to-C provider results of `bool`, `u8`, `u64`, `void` and enum types. The first experiment therefore needs to carry an ordinary `f64` result through declaration checking, C generation, linking and a real CoD caller. C spelling alone establishes only the representation. [Checker at the inspected revision](https://github.com/complyue/cod-project/blob/d3a2381f9ae1709447659633234a28530ae27e52/applications/cod-booter/src/NativeDeclarations.cod#L715-L726) · [Scalar C spellings](https://github.com/complyue/cod-project/blob/d3a2381f9ae1709447659633234a28530ae27e52/applications/cod-booter/src/ProviderViews.cod#L51-L81)

Add the Student t upper tail and quantile for the first port. Expose distribution-level operations such as `ccdf`, `logccdf` and `quantile` so their argument order and stable numerical meaning are available to the agent. Use the target Julia package as the behavioral reference. For example, StatsFuns has its own normal-tail implementation. Floating-point tolerances, degenerate parameters and error behavior belong to the particular port. [StatsFuns normal functions](https://github.com/JuliaStats/StatsFuns.jl/blob/master/src/distrs/norm.jl)

Sampling has a separate ownership question. Rmath.jl installs library-wide, context-free C pointers for uniform, normal and exponential draws. CoD's synchronous `here` borrow has a bounded lifetime. A future `rand(rng, d)` needs to specify whose state advances when two callers interleave and how a long-lived C hook reaches that state. Compare two explicitly seeded streams under interleaving before promising reproducibility. An opaque `double` kernel also discards derivative information carried by a generic number. A differentiable port needs a generic implementation or a derivative rule for that operation. [Upstream RNG installation](https://github.com/JuliaStats/Rmath.jl/blob/50280d872a9626c6c99c7f15a658d7b152f6c9f3/src/Rmath.jl) · [ForwardDiff limits](https://juliadiff.org/ForwardDiff.jl/stable/user/limitations/)

## Array operations an agent can translate directly

Start with dense numeric vectors, matrices and regular strided views. Keep element type, axes, scalar access, iteration and construction of an output with a requested shape visible to generic algorithms. CoD can express elementwise work with a loop or a synchronous callback. The observable obligations below apply to either implementation. [Julia array interfaces](https://docs.julialang.org/en/v1/manual/interfaces/#man-interface-array) · [Julia multidimensional arrays](https://docs.julialang.org/en/v1/manual/arrays/) · [CoD storage access](CollectionsAndBuiltinContainers.md#fold-and-array-access)

| Operation on a writable dense `2 × 3` matrix `A` | Observation in the proposed profile |
| --- | --- |
| `size`, `axes`, `eachindex` | Shape is `(2, 3)`. Ordinary Julia axes start at 1. Julia's range `1:3` visits three positions. Iteration can choose a suitable index for a view. |
| Scalar access | Linear traversal follows columns. `A[3]` selects `A[1,2]`. Keep origin conversion in the array implementation so CoD algorithms can retain nominal index domains elsewhere. |
| `copy`, ordinary slicing and `view` | Mutating the detached result of `A[:,2]` leaves `A` untouched. Writing through `view(A, :, 2)` changes `A`. Julia's indexed assignment `A[:,2] .= 0` writes into the parent as well. |
| `broadcast` and in-place output | `A .+ [10,20]` adds 10 to each first-row element and 20 to each second-row element. An explicit destination supplies the storage for an in-place port. |
| Reduction and `similar` | `sum(A; dims=1)` has shape `(1,3)`; a subsequent broadcast sees the singleton dimension. A fresh result gets a destination owner with the required lifetime. |

A strided view needs an origin, shape and strides measured in elements, plus access to the actual backing under its live owner. An empty extent permits iteration to finish without dereferencing its base. Return a column view from a helper, mutate it while the parent lives and observe the parent. If a Julia caller retains that column beyond the proposed CoD owner's lifetime, let the owner cover the use or change the caller's explicit output contract. Silently materializing a copy loses the write-through behavior. [CoD regional references and owners](RegionalOwnership.md) · [Julia views](https://docs.julialang.org/en/v1/base/arrays/#Base.view)

## Two ports that can disprove the proposal

**One-sample t-test.** Port `OneSampleTTest(x, 5)` for the ten binary64 observations `[5.1, 4.9, 6.2, 5.8, 5.3, 6.1, 5.5, 5.9, 4.7, 6.0]`. Use corrected sample variance with divisor `n−1`. Check mean `5.55`, degrees of freedom `9`, two-sided p-value `0.00947430516135569` and 95% confidence interval `(5.171093364640838, 5.928906635359161)`. Compare the discrete result exactly and the finite floating results with a declared numerical tolerance. This single public operation exercises array traversal, arithmetic, the t upper tail and its quantile. Include a negative test statistic to detect an accidental one-sided implementation. [Upstream implementation](https://github.com/JuliaStats/HypothesisTests.jl/blob/1fbe9dbdc1b831092a213a29b01ce59fae0cf5ea/src/t.jl) · [Input and worked reference](https://juliastats.org/HypothesisTests.jl/stable/specs/t_test/)

**Column distances as an independent port.** Give a second agent the array interface and the source for `Distances.pairwise`. Each column is one observation. The Julia fixture below selects two nonadjacent columns, computes squared distances to three columns and produces a nonsquare output. Swapping the sample axis or assuming packed storage changes the observation. It exercises the array package without Rmath. [Distances.jl column convention](https://github.com/JuliaStats/Distances.jl/blob/c93b809e15080d7e6df1dde6cfe8f0a7f2a1582a/README.md)

```julia
using Distances
X = reshape(Float64[1, 2, 3, 4, 5, 6], 2, 3)
V = view(X, :, 1:2:3)
Y = Float64[1 1 5; 2 4 6]
@assert pairwise(SqEuclidean(), V, Y; dims=2) == [0 4 32; 32 20 0]
```

Use the t-test to refine the initial API, then freeze that revision for the distance port. Give both agents the target source and upstream tests. For the independent port, record which operations translated directly, which new adapters it required, which errors the comparisons caught and which choices still needed a person. To estimate the improvement, compare the same held-out workload under ordinary CoD and under CoD with the frozen package, with the same source, tests and agent budget. Byte-for-byte syntax similarity and the count of bound C symbols say little about that result. The next design revision can use a specific failed port to justify another general language rule, a reusable array operation or an additional numerical backend.
