# Composing Enumerative Choice with Producers and Transformers

## Abstract

A fare search chooses an itinerary, then chooses among the seats on that itinerary. Either choice may offer zero, one, or many answers. A list comprehension gives this search a concise meaning. A booking application may also need to stop at the first acceptable fare, return feedback to the seat source, or close an open inventory cursor on failure. We study a CoD design in which any compatible producer can supply the choices and a transformer can run another producer. The question is whether independently written stages preserve the observable behavior of hand-written nested loops. We give an ordered-answer model for finite pure searches, work through an effectful counterexample to answer-only equivalence, and describe how to test the stronger claim. The structural producer parameter and pipeline notation are language proposals. The proof and measurements are open.

## A closed traversal

A producer offers a candidate by calling a consumer synchronously. That consumer can supply typed feedback before the producer decides what to do next. We call the pipeline *closed* when a single traversal activation accounts for all these handoffs. The source can be lazy, consult a live service, or even enumerate forever. What makes the pipeline closed is that a handoff uses a consumer belonging to a still-active traversal. A receiver-bound container is one possible source. A generator or transformed traversal satisfying the same protocol works as well. [Generator and here-call equivalence](../design/RangedForAndGenerators.md#generalized-for-loops) · [Structural producer proposal](../design/ProducersAndTransformers.md)

Here a transformer accepts a whole itinerary producer. The application operation `fares_on(route)` enumerates fares for just that route. Both sources use void feedback in this example. The proposed pipe supplies the source to the next stage.

```cod
proc fares_for(here routes() yields u64) yields u64 {
    for val route <- routes() {
        for val fare <- fares_on(route) { yield(fare); }
    }
}

proc affordable(here fares() yields u64, limit:u64) yields u64 {
    for val fare <- fares() {
        if (fare <= limit) { yield(fare); }
    }
}

proc first_affordable() -> u64 {
    for val fare <- itineraries() |> fares_for() |> affordable(100u64) {
        return fare;
    }
    return 0u64;  # This example uses positive fares.
}
```

Suppose route A offers 120, 80, 80 and route B offers 90. Full enumeration offers 80, 80, 90 in that order. The second 80 matters because distinct seats can have the same price. The first-fare consumer reads 120 and 80 on A, closes A's fare traversal and then the itinerary traversal. It never opens B. The expansion from one itinerary to several fares and the filter from several fares to one answer change the pace of enumeration within a single call.

## What an equivalence must preserve

For the finite, pure, fully consumed fragment, let the meaning of a producer be its ordered list of offers. An empty choice denotes the empty list. A single offer denotes a singleton. A dependent choice concatenates the lists for each successive upstream item, and a guard replaces a rejected item's list with the empty list. Structural induction over these constructors gives a route to equality with the corresponding list comprehension. Infinite branches, fairness between branches, destructive sources and repeated traversals require their own assumptions. [Haskell 2010 Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html)

Now let a seat source require Boolean feedback after every normal handoff. Suppose it explicitly interprets true as permission to examine another seat. On 120, the price filter rejects the seat without calling its own consumer. **The filter still owes its source an answer.** Supplying false there ends the search before either 80. A transformer that intends to skip expensive seats supplies true on this path. An outward return on the first 80 takes a different path altogether. It initiates cleanup and owes no normal feedback. This distinction is observable even if a test checks only the first returned fare. [Yield feedback](../design/RangedForAndGenerators.md#single-value-yield-and-feedback-use)

The stronger conjecture is trace preservation under elaboration to explicit nested handoffs. Observe which source is entered, which candidate is offered, the feedback returned to which handoff, which lexical exit is taken and which cleanup runs. A stage can break out of its own upstream traversal and continue its work. A return in the final single-brace consumer targets its enclosing callable. An escaping consumer failure bypasses generator catches around the handoff while running entered cleanup. A trace that merely records output elements conflates these executions. [Control and cleanup](../design/RangedForAndGenerators.md#checked-declarations-and-exception-execution)

## Prior work and evaluation

Icon already uses generators for goal-directed search. Go function iterators allow adapters to push values to a consumer and observe an early-stop signal. Strymonas already demonstrates fused mapping, filtering and nested flat-map without constructing intermediate collections. The research question for CoD is what static guarantee a structural source protocol can give for application-specific feedback and lexical control across stage boundaries. [Griswold, *Programming with Generators*](https://doi.org/10.1093/comjnl/31.3.220) · [Go range over functions](https://go.dev/blog/range-functions) · [Kiselyov et al., *Stream Fusion, to Completeness*](https://yanniss.github.io/streams-popl17.pdf)

Compare a pipeline with explicit CoD loops over the same instrumented sources. Use empty and duplicate offers, a guard that owes upstream feedback, a prefix stop, an outward return and an exception from the final consumer. Compare full traces before measuring time to first answer, total traversal time and intermediate allocations. Repeat with a table-backed source and a computed one while keeping the transformer unchanged. A fused iterator encoding with the same checks and traces would narrow the contribution to language integration. Avoiding a list alone gives no bound on the memory used by a stage such as distinct-value filtering.

The companion study considers what changes when the next input can arrive after the submitting invocation has returned. [Open event pipelines](OpenEventPipelines.md)
