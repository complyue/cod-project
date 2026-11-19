# Producers and Transformers for Nondeterministic Comprehensions

Design revision: 1 · 2026-11-19 · Language-design draft

## Design authority

[D-31](DesignAuthorityRegistry.md#d-31) records the requested design scope. The project user proposed passing a source to a transformer, admitting every source that meets the producer interface, and treating a functor as one implementation. [Decision provenance](../CHANGELOG.md#k-producer-transformers-20261119)

The confirmed baselines are [generator and here-call equivalence](RangedForAndGenerators.md#generalized-for-loops), [functor receiver preparation](DomainLanguageDesign.md#functor-receivers-and-callback-arguments), and [owner-checked synchronous access](CallableStorageAndHereAccess.md#what-an-invocation-needs). C-97/P-633–P-635 propose a derivation from C-96/C-65 and charter §§3.2–3.3/4.3/6.1. Reusing a traversal description lets callers choose the source and the consumer independently while keeping runtime control and borrows with the execution that owns them. The structural parameter spelling, pipe, composition of stored transformers and wider implementation contract are pending human review. The examples and acceptance cases below delimit the proposal.

## A transformer takes a producer as an input

A producer offers zero or more candidates to a synchronous consumer. The candidates can occur in a deterministic order and can repeat. A transformer takes an upstream producer plus its ordinary configuration, and offers candidates to its own downstream consumer. Its input is the traversal itself. It can skip it, inspect a prefix or drive it to completion, subject to the upstream contract. A filter makes zero or one downstream handoffs per upstream item. A map makes one. A dependent nested traversal (flat-map) can make many.

In the proposed syntax, `here src() yields u64` declares the upstream parameter and the outer `yields u64` declares the downstream handoff. Both use void feedback in this example.

```cod
func half_price(here src() yields u64) yields u64 {
    for val fare <- src() { yield(fare / 2u64); }
}

func at_most(here src() yields u64, ceiling:u64) yields u64 {
    for val fare <- src() {
        if (fare <= ceiling) { yield(fare); }
    }
}

for val fare <- published_fares(day)
                |> half_price()
                |> at_most(100u64) {
    quote(fare);
}
```

The proposed `|>` supplies the first explicit producer parameter of the stage on its right. The implicit downstream `yield` remains separate, and `100u64` binds `ceiling`. The final loop supplies the consumer. If the source offers 80, 120 and 240, the body receives 40 and 60. Reversing the stages produces only 40. A named generator, a compatible callback-first operation, a receiver-bound functor or another composed traversal could supply the same input. Admission depends on a checked protocol, independent of membership in any particular container type. The [feature overview](SymbolicDynamicScope.md#comparison-with-haskell-lists) compares ordered answers with Haskell lists.

## Preparation, control and storage

Prepare the source receiver and written arguments once in source order when binding the immediate traversal. Start the source body when a stage invokes its upstream producer. Thus a stage that skips its input still observes effects from preparing that input's arguments, and never enters the source body. Calling a source twice may observe different state or consume more input. A proposed reusable plan stores its own admissible configuration. A new traversal supplies a live source and consumer and performs its own preparation.

Establish yielded types, modes, labels, owners, feedback and checked failures before inspecting the final consumer body. A `func` stage must satisfy its own effect restrictions. Caller-owned work follows the existing handoff rule. A scalar or borrowed item uses the normal receiving rules. A borrowed output still needs one provable owner for its component throughout the traversal. Feedback matters in both directions. For example, a stage consuming a producer that requires Boolean feedback must provide a Boolean even for an item it filters out, when the downstream consumer never ran. A stage can declare that policy in its code. [Yield and feedback contracts](RangedForAndGenerators.md#generator-and-yield-signatures)

Consider an upstream source with a deferred close. If a prefix stage stops its own traversal of that source, the source runs its close and control resumes in the stage. If the final consumer executes an outward return, it exits the callable that supplied that body, through all entered cleanups. A consumer exception follows the existing generator catch-bypass rule. Completed effects remain observable on every path. A proposed elaboration must preserve these distinct control destinations through every intermediate stage. [Generator control and cleanup](RangedForAndGenerators.md#generalized-for-loops)

For example, a budget functor can store just its ceiling and take a source on each call. A composition of a discount rule and a budget rule could likewise store those configurations and take a fresh source later. Persistent data must satisfy ordinary regional layout and same-owner field rules. Each invocation borrows its own source and consumer. Storing that consumer or extending a lexical owner's lifetime would violate the existing here-callable contract. Some stages, such as distinct-value filtering, need traversal-local state that grows with the input. Avoiding an intermediate list alone gives no constant-space guarantee. [Callable storage and direct lexical access](CallableStorageAndHereAccess.md)

## Open decisions and acceptance

- **Binding and syntax.** Specify how to check a supplied source with ordinary arguments without starting it, and how named/defaulted arguments and overloads interact with the producer parameter. Check pipe tokenization alongside case guards, sum alternatives and existing operators. The ordinary generator call still requires its consumer. A general expression such as `budget(half_price(src))` needs an explicit binding rule before it can mean composition.
- **Independent composition.** An immediately consumed pipeline and a stored transformer that takes a source later have different lifetimes. Decide the source spelling and permitted representation for constructing the latter. Verify that the same stored configuration works with two independently owned sources and that trying to persist a live consumer rejects.
- **Observable equivalence.** Compare the fare pipeline with hand-written nested handoffs on empty, repeated and filtered inputs. Check preparation side effects, normal feedback, skipped input, an upstream-local stop, a final consumer return and a checked consumer failure. Record candidate order, effects, control destination and each release. Include a rejecting borrowed-owner case and a rejecting feedback mismatch.
- **Research boundary.** Enumerative choice preserves the order and multiplicity the producer offers. Scheduling fairly across infinite branches, transactional rollback and resuming a suspended traversal are separate operations. For the pure, finite, fully consumed subset, compare the answer sequence with list-comprehension semantics.

## Research question and prior work

Can a language type a reusable, relocatable description of a comprehension separately from its live execution, while preserving the observable behavior of nested hand-written handoffs? A candidate paper title is *Owner-Checked Comprehensions with Persistent Transformer Plans*. The persistence claim concerns admissible rule data. Live frames, consumers and partially executed searches have separate lifetime requirements.

| Prior result | Question for CoD |
| --- | --- |
| Haskell comprehensions specify ordered enumeration. Fusion can eliminate intermediate lists from compiled comprehensions. [Haskell Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html) · [Gill, Launchbury and Peyton Jones](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/07/deforestation-short-cut.pdf) | Identify the domain in which a CoD traversal returns the same sequence, then account for observable effects and early exits. |
| Go demonstrates a push iterator that filters another iterator. Strymonas gives a staged implementation for composable stream transformations including nested streams. [Go range over functions](https://go.dev/blog/range-functions) · [Kiselyov et al., POPL 2017](https://yanniss.github.io/streams-popl17.pdf) | Compare producer composition, short-circuiting, state and allocation against established streaming baselines. |
| Clojure transducers let one transformation run against different sources. Substrait specifies a serializable representation of relational plans. [Clojure transducers](https://clojure.org/reference/transducers) · [Substrait plan serialization](https://substrait.io/serialization/basics/) | Show what owner checking adds when a persisted rule runs against new sources and a live, effectful consumer. |
| Defunctionalization relates higher-order computations to first-order data. Rust lending iterators express items whose lifetimes depend on a borrow of the producer. [Danvy and Nielsen](https://tidsskrift.dk/brics/article/view/21684) · [Rust GAT lending iterator example](https://blog.rust-lang.org/2022/10/28/gats-stabilization/) | Explain precisely which rule data can survive relocation, which owner covers a yielded view and which callback remains usable only during its invocation. |

A research paper would need a small calculus or executable reference model, a statement of owner safety and preservation of control/effect traces, and a prototype exercising those claims. A discriminating experiment would publish an admissible rule description, reload it at a different address, supply a fresh source and ask for only the first match. Compare its answers and cleanup trace with a hand-written traversal, then measure allocations, peak live storage and time to first answer for both prefix and full enumeration. The experiment and proofs are proposed work.
