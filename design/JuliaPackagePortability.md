# Porting Julia packages to CoD with `julia.cod`

Research WIP · revision 1 · 2026-11-26

An agent porting a one-sample t-test should be able to recognize the sample, compute the same statistic and ask for the same tail probability in CoD. A second agent porting a distance function should be able to reuse those array operations without learning which C library supplies a probability. These two ports test whether a small CoD package can make important Julia algorithms economical to bring over and maintain.

## Design authority

The project user commissioned research on native C access to Julia's Rmath library and a Julia-like array interface for agent-assisted ports. Direct compilation of Julia source is outside the goal. The user also invited alignment in CoD itself wherever Julia's semantics fit and specifically reopened `x : f64` for that purpose. This preference applies when choosing ordinary CoD operations as well as package APIs. The concrete rounding policy, package layout, provider extension and array profile below are proposals. No port has been executed for this revision. [User decision](../CHANGELOG.md#h-julia-package-portability-20261126) · [Registry and scope](DesignAuthorityRegistry.md#d-julia-pkg-research)

The charter calls for local, reviewable edits by people and agents. Reusing a checked operation across packages lets a reviewer inspect the domain algorithm and check its assumptions about values, storage and lifetimes. The candidate value path is P-756. P-757 identifies the conversion, owner, collection and native-call contracts that constrain the two workloads. [Charter §§1, 4.3 and 8](RegionalSemanticPlatformCharter.md) · [Regional arrays](CollectionsAndBuiltinContainers.md#fold-and-array-access) · [Native C providers](../applications/cod-booter/docs/LLVMInterop.md#generated-c-providers-for-the-first-booter)

## Where to align with Julia

For each recurring Julia operation, write down an input that distinguishes its behavior and identify who else would benefit from that behavior in CoD. A good general meaning belongs on the ordinary language or standard-library path when it fits CoD's other callers. A Julia array coordinate convention can live on a particular array type. A distribution's tail computation belongs with the numerical package. When the proposed meaning touches an existing guarantee, record the program whose result, access or diagnostic would change. This lets the next package author reuse the decision.

### Explicit conversion to binary64

Consider an `i64` sample count of 9,007,199,254,740,993. Julia's `Float64(x)` in its default rounding mode gives 9,007,199,254,740,992. CoD's published rule requires an exactly representable result for a defined execution of `x : f64`. It also requires a warning unless losslessness is proved. That means a spelling an agent would naturally choose for this port has a different meaning precisely when the rounding matters. [Julia numerical conversions](https://docs.julialang.org/en/v1/manual/mathematical-operations/#Numerical-Conversions) · [Julia rounding modes](https://docs.julialang.org/en/v1/manual/integers-and-floating-point-numbers/#Rounding-modes) · [CoD explicit conversion contract](ScalarUnitsAndIndexTypes.md#explicit-numeric-conversion-and-loss-warnings)

**Candidate language rule.** For an explicitly converted, dimensionless `i64` or `u64` operand, let `x : f64` produce the nearest binary64 value, resolving a tie toward the even significand. Give any CoD caller the same operation. These inputs distinguish an exact conversion, both directions of a tie, the signed case and the upper unsigned boundary.

| Integer input | Proposed `f64` result, written as its exact mathematical value |
| --- | --- |
| `2^53 − 1` | `2^53 − 1` |
| `2^53 + 1` | `2^53` |
| `2^53 + 3` | `2^53 + 4` |
| `−(2^53 + 1)` | `−2^53` |
| `2^64 − 1` as `u64` | `2^64` |

For a language decision, compare the binary64 bits from a folded constant and a runtime parameter against Julia's `reinterpret(UInt64, Float64(x))`. Decide separately whether a possible loss of precision should produce an advisory. Under the candidate rule, a diagnostic claiming that this rounding causes undefined behavior would be wrong. Implicit widening affects overload selection even when no explicit conversion is written. Float-to-integer conversion, nominal units, nonnumeric type assertions and planned fractions each have different consumers. A later extension to those operations needs its own distinguishing example. [Whole-type lossless compatibility](NamedQuantitiesAndNumericCompatibility.md#whole-type-lossless-compatibility) · [Planned fractions](FixedWidthFractions.md)

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
