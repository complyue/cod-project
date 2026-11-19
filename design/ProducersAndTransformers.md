# Producers and Transformers for Nondeterministic Comprehensions

Design revision: 2 · 2026-11-19 · Language-design draft

## Design authority

[D-31](DesignAuthorityRegistry.md#d-31) records the requested design scope. The project user proposed passing a source to a transformer, admitting every source that meets the producer interface, and treating a functor as one implementation. [Decision provenance](../CHANGELOG.md#k-producer-transformers-20261119)

The confirmed baselines are [generator and here-call equivalence](RangedForAndGenerators.md#generalized-for-loops), [functor receiver preparation](DomainLanguageDesign.md#functor-receivers-and-callback-arguments), and [owner-checked synchronous access](CallableStorageAndHereAccess.md#what-an-invocation-needs). C-97/P-633–P-635 propose a derivation from C-96/C-65 and charter §§3.2–3.3/4.3/6.1. Reusing a traversal description lets callers choose the source and the consumer independently while keeping runtime control and borrows with the execution that owns them. The structural parameter spelling, pipe, composition of stored transformers and wider implementation contract are pending human review. The examples and acceptance cases below delimit the proposal.

The paper outline concerns the semantics of enumerating answers. Its proof and experiments are proposed research. [Research scope](../CHANGELOG.md#k-comprehension-research-scope-20261119)

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

Working title: *Composing Enumerative Choice with Producers and Transformers*. Can a transformer accept any source that satisfies its producer interface and compose dependent choices while preserving the feedback, exits and cleanup of explicit nested loops? A fare search illustrates the question. For each itinerary, a second source offers seats available on that itinerary. A filter may skip a seat, several seats may yield the same fare, and a caller asking for the first acceptable fare can finish while both sources are open.

| Comparison | What the paper should establish |
| --- | --- |
| Haskell list comprehensions translate nested generators through `concatMap`. Icon uses generators and goal-directed evaluation to explore alternative results. [Haskell Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html) · [Griswold, *Programming with Generators*](https://doi.org/10.1093/comjnl/31.3.220) | For pure, finite, fully consumed sources, prove equality with the list of answers from nested enumeration, including order, duplicates and empty branches. |
| Go composes push iterators and signals early termination through the result of `yield`. Strymonas fuses pipelines that include filtering and nested flat-map. [Go range over functions](https://go.dev/blog/range-functions) · [Kiselyov et al., *Stream Fusion, to Completeness*](https://yanniss.github.io/streams-popl17.pdf) | Compare the control required in each encoding when a source requires application-specific feedback, a stage stops a single upstream traversal, or the final consumer returns from its enclosing callable. |

Use hand-written nested loops as the reference for effectful searches. Instrument both itinerary and seat producers. On a skipped seat, check which feedback reaches the seat producer even though the consumer never ran. On a return from the first accepted fare, check that the seat source closes before the itinerary source, each closes once, and neither source offers another candidate. Repeat with a consumer exception and with an empty inner source. An executable model can compare those event sequences. A prototype can separately measure time to first answer and intermediate allocations for prefix and full enumeration.

The case for a research paper rests on a general rule for typed composition and a demonstration of what it guarantees across independently written stages. If existing iterator encodings provide the same guarantees with the same static checks, a comparative design note would fit the evidence. Coarse-grained ownership and relocatable graphs form a separate research topic grounded in the [regional storage design](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations).
