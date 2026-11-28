# Producers and Transformers for Nondeterministic Comprehensions

Design revision: 14 · 2026-11-25

## Design authority

At r14 the [OO call-operator decision](../CHANGELOG.md#h-oo-call-operator-head-20261125) supplies the receiver heads for the invoice processors. [D-31](DesignAuthorityRegistry.md#d-31) continues the r13 composition and feedback contracts. Check the same upstream demand, ordered deliveries and termination through either pipeline spelling.

At r13, D-32-CHAN-ENDS/P-712 adds a workload for the future formal type language. The sending end of a chan can consume typed input in one Actor's pipeline. The receiving end can produce admitted messages for another Actor's pipeline. A transformer can be composed locally on either side. Express these independent roles once CoD supports higher-kinded type synonyms. Acceptance must check the message type and the direction at each endpoint, with synchronous consumer feedback scoped to its local traversal. [Chan endpoint contract](ActorChannels.md#independent-chan-endpoints) · [Decision](../CHANGELOG.md#h-actor-region-transport-and-chan-ends-20261124)

[D-31-COMPOSITE](DesignAuthorityRegistry.md#d-31-composite) is the confirmed implementation contract for the [operand-role rules](#resolving-the-role-of-a-name) and the [composite invoice-review workload](#composite-implementation-target), including their comparison and rejection cases. The user confirmed the r10 contract and directed mainline to proceed. That scope is recorded at r11. [Human decision](../CHANGELOG.md#h-pipeline-composite-contract-20261119)

C-97 derives from C-96/C-65 under charter §§3.2–3.3, 4.3 and 6.1. A caller can reuse a processor with independent sources and consumers while each traversal retains its own control, effects and actual owners. The covered parts of P-633–P-635 and P-638/P-639 apply [generator and here-call equivalence](RangedForAndGenerators.md#generalized-for-loops), [functor receiver preparation](DomainLanguageDesign.md#functor-receivers-and-callback-arguments) and [owner-checked synchronous access](CallableStorageAndHereAccess.md#what-an-invocation-needs). [Registry](DesignAuthorityRegistry.md#d-31)

Acceptance compares written operand preparation, library construction, handoffs, feedback, terminal results and cleanup with explicit calls in both directions. It includes the three bounded lexical partial shapes, invalidated owners, conflicting protocols and a data-only regional functor. Mainline assesses the complete representation before compiler code and tests, and owns lowering and cost-driven local syntax. Language design owns observable behavior and reviews any proposed change to it. [Composite target](#composite-implementation-target) · [Division of work](../CHANGELOG.md#h-pipeline-design-remit-20261119)

At r12 the user deferred the general producer, consumer and transformer interfaces, including contextual generator binding, until CoD supports named type signatures. The future discussion and its distinguishing example are recorded in [DesignTopics](../docs/DesignTopics.md#named-signatures-and-pipelines). Mutable local here holders and a permanent native representation remain open. [Scheduling decision](../CHANGELOG.md#h-named-signatures-before-pipeline-protocols-20261123) The broader [D-31 proposal](DesignAuthorityRegistry.md#d-31) and [storage proposal](DesignAuthorityRegistry.md#d-31-pipeline-storage) retain their own scope. The paper's proof and measurements are research work. [Research scope](../CHANGELOG.md#k-comprehension-research-scope-20261119)

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
                !> half_price()
                |> at_most(100u64) {
    quote(fare);
}
```

The `|>` operator composes the two processors. In processor position the stage designators bind their written configuration and leave the upstream source and downstream consumer open. Here `100u64` binds `ceiling`. The weaker `!>` supplies the source, and the final loop supplies the consumer. If the source offers 80, 120 and 240, the body receives 40 and 60. Reversing the stages produces only 40. A named generator, a compatible callback-first operation, a receiver-bound functor or another composed traversal could supply the same input. Admission depends on a checked protocol, independent of membership in any particular container type. The [feature overview](SymbolicDynamicScope.md#comparison-with-haskell-lists) compares ordered answers with Haskell lists.

## Processor functors in either direction

A processor can be an ordinary data instance. For an invoice report, a policy gives a base price and the maximum discount in basis points. The constructor derives the lowest qualifying whole-cent amount, rounding up so an accepted invoice cannot exceed the permitted discount. That business calculation runs once. Traversal only needs a comparison.

```cod
data InvoiceFloor { cents:u64 }

func InvoiceFloor(base_cents:u32, discount_bps:u16) {
    assert discount_bps <= 10000u16;
    val payable_bps <- 10000u64 - (discount_bps : u64);
    cents <- ((base_cents : u64) * payable_bps + 9999u64) / 10000u64;
}

func const InvoiceFloor.(())(here src() yields u64) yields u64 {
    for val cents <- src() {
        if (cents >= self.cents) { yield(cents); }
    }
}
```

The constructor receives initialization places for the fields. The call operator receives a completed instance. The two belong to the type's separate [method families](DomainLanguageDesign.md#constructor-and-call-operator-method-families). The bounded first here parameter supplies the downstream consumer after receiver binding, and the explicit producer parameter receives the upstream traversal. The call operator runs once each time the stage is driven. It can consume or yield any number of items.

```cod
ref p1 = make InvoiceFloor(12500u32, 2000u16);
ref p2 = make WholeUnits(2u8);
ref p3 = make FirstN(2u64);

# Alternative spellings for a source that yields invoice totals in cents.
for val item <- p3 <| p2 <| p1 <! src { report(item); }
src !> p1 |> p2 |> p3 (item) => { report(item); };
src !> p1 |> p2 |> p3 $> report;
report <$ p3 <| p2 <| p1 <! src;
```

Here `src` supplies a traversal with no unbound ordinary arguments. The floor is 10000 cents. WholeUnits takes a supported number of decimal places and computes its divisor (100 for two places). FirstN keeps its remaining count local to each invocation, skips its input at limit zero, and stops its upstream traversal as soon as it has delivered its limit. For source totals 8000, 12050, 14099 and 16000, each spelling reports 120 and 140. An on-demand source never reads the fourth total. A second run gets its own count. A method with a writable receiver may deliberately update a field, and later runs on that instance observe the change.

### Processor composition and source attachment

The processor chain has an input port and an output port. Composition connects processors in the middle. Endpoint connectors attach the source producer to the input port or the terminal consumer to the output port.

| Binding level and spelling | Association | Operation |
| --- | --- | --- |
| `p \|> q` | Left | Two processors form a processor that feeds p's output into q. |
| `q <\| p` | Right, same precedence as the forward form | The same connection written downstream first. |
| Outer, `src !> plan` or `plan <! src` | One source connector | Connect the producer. The output can then supply a loop or another consumer. |
| Outer, `plan $> sink` or `sink <$ plan` | One consumer connector | Connect the consumer. The input can still receive a source. |

The four endpoint spellings share the weaker level. Parse one source and one consumer on a chain as a matched pair. Both `src !> p1 |> p2 $> sink` and `sink <$ p2 <| p1 <! src` connect src to p1, p1 to p2 and p2 to sink. There is no arbitrary left or right fold of endpoint operators. Elaborate the middle processor first, feed it the source to obtain a traversal, and give that traversal to the terminal operation. This specifies the method calls for both spellings while retaining written operand preparation order. A second connector for the same end rejects.

Either end can be left open for bounded composition. `src !> plan` can supply a compatible producer parameter. Conversely, `plan $> report` can supply a synchronous consumer-of-a-source parameter: once given a compatible source, it runs that source through plan and calls report for each output. Creating either partial connection runs no traversal. A consumer-only connection retains the lexical permissions and lifetime of its supplying site, including when forwarded. Context must distinguish an open processor from an already complete producer in `name $> report`; an unresolved dual role requires an explicit call or typed helper. With an established generator, `invoices(day) $> report` runs directly with no middle stages.

Forward grouping is `src !> ((p1 |> p2) |> p3)`. Reverse grouping is `(p3 <| (p2 <| p1)) <! src`. These parentheses display the parse tree. In either form the processing order is p1, p2, p3. Each binary operation is a lookup point. The builtin composition can wire adjacent stages without invoking their bodies or materializing an intermediate object. Check the yield, feedback, access, owners and failures at each connection. A selected library method has its ordinary result contract and observable evaluation.

Ordinary operand operations, including member access, calls and logical negation, bind before both pipeline levels. Both bind inside the RHS of assignment. A for body or a trailing `(receivers) => { ... }` supplies one consumer for the entire attachment. For a scalar val output, `(item)` introduces a fresh binding as in `for val item <- ...`. To write an existing destination, pair `for item <- ...` with `(item <-) => { ... }`. Labelled and borrowed outputs keep their normal receiving specifications.

Require one direction within an unparenthesized run of composition operators. Reject `p1 |> p2 <| p3`. The source attachment has its own tier, so `src !> p3 <| p2 <| p1` is unambiguous. Reject a repeated ungrouped attachment, including `src !> p1 !> p2` and `left !> plan <! right`. A two-source merge takes its inputs explicitly. Reject two terminal operators in one ungrouped chain or a second consumer for the same traversal. Scan the six contiguous spellings as whole tokens. In particular, `a < !b` keeps comparison with Boolean negation, while `a<!b` selects attachment. The required parse and type diagnostics fit the seed correctness policy.

A source-free composition can be passed once and run on two independent inputs. Here the processor positions establish the remaining source and consumer protocols:

```cod
proc report_days(here plan(here input() yields u64) yields u64,
                 first:u64, second:u64) {
    for val amount <- invoices(first) !> plan { report(amount); }
    for val amount <- invoices(second) !> plan { report(amount); }
    return;
}

report_days(p1 |> p2 |> p3, monday, tuesday);
```

Preparing this argument binds the three existing receiver identities once. Each invocation gets a new source, consumer and traversal-local state. If the processors share mutable data, ordinary writes to those targets stay observable. Immediate composition needs no lasting allocation. The [lexical binding](#lexical-variables-for-open-pipelines) gives the builtin connection a local name and lets overloads return ordinary domain data.

Preparation evaluates operands once in written order and fixes each functor receiver's target, access and owner at its occurrence. Preparing prebound names gives the same connections in both examples. If two operand expressions log or rebind a later receiver, reversing their positions can change preparation effects or selected targets. Bind effectful operands in separate declarations when changing direction must preserve their evaluation order. During traversal p3 drives p2, p2 drives p1, and p1 drives src. Items travel outwards. Skipped items and consumer feedback follow each selected method's declared protocol. A break at the terminal ends the driven pipeline. An outward return reaches the callable supplying the terminal body through the entered cleanups.

Owners of the instances may differ as long as each remains live through all uses. Regional instances store ordinary relocatable data. A checked invocation carries bounded upstream and downstream here arguments. Saving one of those arguments into a processor field would violate its lifetime contract. Builtin connections in both directions can elaborate to checked nested invocations, preserving written operand preparation order and terminal control destinations.

### A named terminal consumer

Write `traversal $> report` or `report <$ traversal` when the body simply calls report on each handoff. In `report <$ p3 <| p2 <| p1 <! src`, the terminal receives the complete traversal on its right. A generator with no processor stages can use either spelling directly. For a single offered value and void feedback the builtin expansion is `(item) => { report(item); }` with a fresh, compiler-private binding using the offered mode. A named proc is invoked directly in the lexical context supplying that body. A func site must have permission to make the same call. The expansion also works with a named func, here parameter or callable data receiver when the ordinary call is valid there.

```cod
src !> p1 |> p2 |> p3 $> report;
report <$ invoices(day);

# If the final yield expects bool feedback:
offers(day) $> still_needed;
# Equivalent explicit consumer:
offers(day) (item) => { continue still_needed(item); };
```

Resolve the terminal call from the established handoff and ordinary call-site rules. For a labelled handoff, pass every field by its label as a named argument, preserving its mode and actual owner. The source prepares the handoff once under D-29. Use an explicit receiving header for projection, renaming or writes into existing destinations. A void-feedback consumer discards an independently valid call result with its cleanup. A non-void consumer must supply the declared feedback. A normal return from an ordinary named procedure completes that call. An escaping checked failure belongs to the caller's loop body and follows the generator's established cleanup and catch-bypass rules. Zero offers execute zero terminal calls. If the name denotes a rebindable callable or functor variable, each handoff reads it just as the written body would.

The existing `(receivers) => { ... }` arrow introduces a written body. The terminal operators are ordinary two-character tokens. In `src !> make FirstN(2u64) $> report`, the parentheses supply constructor arguments. In `src !> p3 (item) => { report(item); }`, they supply the receiving header. The builtin per-item form uses a lexical name for the callee and yields the generator's normal void completion. Normal feedback belongs to the individual handoff.

### Library-defined pipeline operations

The proposed extension has three binary method families. `p |> q` and `q <| p` select the same composition method with parameters in upstream-to-downstream order. Likewise `src !> plan` and `plan <! src` select one attachment method with source first. `traversal $> sink` and `sink <$ traversal` select one terminal method with traversal first. In a declaration written with a reverse spelling, reverse the two explicit operand positions when registering the signature. Equivalent declarations are duplicates under the ordinary module-world rules. Mirroring parameter roles for lookup preserves evaluation of the actual operands in their written order. Precedence and associativity belong to the grammar.

Use ordinary visible operator methods and specificity. An ambiguous best match diagnoses at its use. A selected method retains its effects and failures. For composition or attachment with no applicable method, try the structural builtin. Composition must deliver a processor usable on a later source. Attachment must supply a traversal callable with the terminal consumer. A generator-shaped attachment method binds its ordinary operands during preparation and runs its body when driven. A library composition method can construct a domain-specific plan with ordinary construction and lifetime rules. Caller-owned control and checked-failure rules hold at each handoff. [Method selection](DomainLanguageDesign.md#method-selection-examples-and-evidence) · [Generator control](RangedForAndGenerators.md#generalized-for-loops)

```cod
proc (!>)(table:const InvoiceIndex in I, floor:const InvoiceFloor in P)
    yields u64 where {const owner I, const owner P} {
    for val cents <- ordered_totals_from(table, floor.cents) { yield(cents); }
    return;
}

table !> floor $> report;
report <$ floor <! table;
```

Here the library defines an indexed query with the floor read on opening the traversal and results in the index's documented order. Either spelling selects that same operation. In `table !> floor |> limit`, attachment receives the result of composing floor and limit. The binary attachment overload shown above applies only to its declared pair. A library could define `(|>)` for those processor types to construct a query plan and `(!>)` for the index and that plan. Grouping matters once a method has a domain result type, observes mutable state, throws or performs I/O. Optimization across a selected overload must preserve its contract and effects. A readonly view alone cannot justify caching a mutable cutoff across handoffs.

A terminal overload can take a bounded traversal and an ordinary typed sink object. The method runs once and returns its declared result, even if the traversal offers nothing. For example, a library can supply an invoice-counting terminal:

```cod
proc ($>)(here rows() yields u64, sink:const InvoiceCount in O)
    -> u64 where {const owner O} {
    val n:u64 <- 0u64;
    for val amount <- rows() { n <- n + 1u64; }
    return n;
}

val count <- invoices(day) !> floor $> counter;
# Equivalent for these prebound operands, with counter:InvoiceCount:
val again <- counter <$ floor <! invoices(day);
```

Each line is a separate invocation. An empty source returns zero from the counting method. A named proc such as report supplies the per-handoff spelling through a direct call in the caller's body. It cannot become a procedure value by overload selection. A library can also overload the item call itself or give a writer object a `(())` method. If a sink name both admits the builtin item call and is an applicable whole-traversal terminal object, the shorthand has two meanings. Reject that use in either direction. A written here body selects the per-item call; explicit `($>)(traversal, sink)` selects the declared operator method. The result's expected type cannot silently choose between per-item execution and a whole-traversal operation.

### Resolving the role of a name

Parse `name $> report` as one consumer connection, retaining the operands, spelling and written order. The checker uses the declared or inferred protocol of name, the methods visible in the current module world and the roles explicitly supplied by the surrounding construct. It can do this before evaluating either operand. A simple name lookup in the parser would miss functor overloads, typed here parameters and prior partial connections.

```cod
proc example(here rows() yields u64,
             here stage(here input() yields u64) yields u64) {
    rows $> report;                 # rows only needs a consumer: execute now.
    here later = stage $> report;   # stage still needs its input: bind now.
    rows !> later;                  # Supply the missing source and execute.
    return;
}
```

Both occurrences of the dollar connector have the same parse. The two declarations already establish different input requirements. In a three-part chain, `rows !> stage $> report` also gives stage the middle position explicitly. A typed helper can similarly specify its operand role. A bare statement, semicolon, inferred destination or expected terminal result supplies no general preference for running over binding. In the example `here later` names the connection that stage's input protocol has already established.

For a genuine collision, suppose a functor called both has a source method with no ordinary arguments and a processor method with one required upstream argument. In a plain procedural context `both $> report;` can satisfy both roles. Reject the ambiguity. Existing syntax offers two local workarounds:

```cod
for val n <- both() { report(n); }   # Select the no-input generator method.
invoices(day) !> both $> report;     # Select its processor role on these invoices.
```

To keep the second form open, use a typed helper taking `here stage(here input() yields u64) yields u64` and form `stage $> report` there as above. Independently, a sink with both an item call and a whole-traversal operator follows the [terminal disambiguation rule](#library-defined-pipeline-operations). An explicit receiving body selects per-item use; an explicit operator call selects the whole-traversal overload. The seed can report the conflicting roles found while checking. Richer suggestions fall under the general diagnostics policy.

## What applications gain from streaming

These examples use library operations as precedents. Their CoD counterparts are design sketches. Source and sink adapters have the domain contracts described below. The proposed `here src() yields ...` accepts an argument-bound source with the declared protocol. Plain generator loops supply the reference behavior. A single upstream traversal can lower to a bounded here call taking its consumer as another here parameter. The shared compiler work is moderate in design scope: establish the protocol at overload selection, evaluate bound arguments once, carry owner evidence, and preserve protected exits across both calls. After that, piping and nested application can share the same checked call.

### Report on incomplete survey records

Frames uses pipes to answer a data-quality question about its occupational survey fixture. It reads rows with individually optional columns, filters for missing prestige, projects income, drops absent incomes and folds sum and count together. The count therefore measures records contributing to the mean. A row lacking both fields contributes to neither accumulator. The shipped 102-row CSV gives four contributing incomes (8206, 918, 611 and 3643) and a mean of 3344.5. [Frames program](https://github.com/acowley/Frames/blob/1700500d4e6048207a9e96cf5773ef25feff67fd/test/UncurryFoldPartialData.hs) · [CSV fixture](https://github.com/acowley/Frames/blob/1700500d4e6048207a9e96cf5773ef25feff67fd/test/data/prestigePartial.csv)

```cod
func unscored_incomes(here rows() yields {
    has_prestige:bool, has_income:bool, income:u64,
}) yields u64 {
    for { val has_prestige <-, val has_income <-, val income <- } := rows() {
        if (not has_prestige && has_income) { yield(income); }
    }
}

{ count <-, mean <- } := count_and_mean(unscored_incomes(survey_rows(path)));
# Equivalent proposed pipe spelling for the producer argument.
{ count <-, mean <- } := count_and_mean(survey_rows(path) !> unscored_incomes());
```

Each line is an alternative call. The adapter above supplies presence bits and an unused scalar payload for a missing income. Any file, database or test producer supplying that schema can drive the stage. The terminal needs one count and one running sum. To reproduce the Haskell example on an empty selection, its floating-point mean is NaN. A reporting API promising a defined mean only for positive counts can expose that distinction explicitly. The loop and fold are low-cost library work once the input contract is available. Schema inference from a sample CSV is a separate Frames feature.

### Verify a package archive during download

Pantry's HTTP helper feeds each response into a SHA-256 sink, a byte-count sink and a caller-supplied sink using conduit's `ZipSink`. Its archive loader chooses a temporary file as the third sink and passes the file onward after all checks and handle closure. It can reject excess bytes mid-response. Exact length and the final digest require EOF. Expected length and digest are independently optional in Pantry. [HTTP helper](https://github.com/commercialhaskell/pantry/blob/7289154db99b24d8042b45e98767176a8cc8895c/src/Pantry/HTTP.hs#L52-L97) · [Archive caller](https://github.com/commercialhaskell/pantry/blob/7289154db99b24d8042b45e98767176a8cc8895c/src/Pantry/Archive.hs#L256-L261)

```cod
# Inside an archive operation with a private temporary file and a fresh hash state.
for ref chunk:const Chunk = chunks() {
    size <- add_with_limit(size, byte_count(chunk), expected_size);
    sha256_update(hash, chunk);
    write_chunk(writer, chunk);
}
verify_digest(sha256_finish(hash), expected_sha);
verify_exact_size(size, expected_size);
close_writer(writer);
inspect_archive(staged_path);
```

Here `chunks` may be bound to HTTP or to a test source with the same readonly chunk contract. The source must report transport failures and distinguish them from EOF. A reusable input buffer can belong to one generator activation. All three operations finish using its current contents before the producer refills it. The temp file has cleanup on failure, the byte count starts at zero and the optional checks run only when an expectation is supplied. For an expected three-byte file, two chunks containing “a” and “bc” succeed with the matching digest. A fourth byte fails immediately. A three-byte substitution reaches EOF and fails the digest check. A consumer that stops after a valid-looking header cannot certify the archive.

One synchronous pass and bounded working memory suffice for this fixed set of consumers. The integration cost is moderate: the HTTP, digest and temporary-file adapters must preserve failure and cleanup before the file reaches the archive reader. A general facility for independently finishing sinks would also need a contract for which sinks keep consuming and how their results are collected. This use case works with an explicit loop.

### Replay two sorted journals in order

The `streaming` library provides `mergeOn` for two sorted inputs. It keeps both records on equal keys, with the left one first. Consider replaying billing and refund journals using the same ordering key. The journal interpretation is a CoD application of the documented operation. It needs an event sequence suitable for audit and can emit the first events without reading two complete files into memory. Here the event ID is the value the audit sink needs. [Streaming.Prelude merge operations](https://hackage-content.haskell.org/package/streaming-0.2.4.0/docs/Streaming-Prelude.html#v:mergeOn)

```cod
proc audit_order(here left() yields { tick:u64, entry:u64 },
                 right:JournalCursor in R) yields u64 where {owner R} {
    for { val tick <-, val entry <- } := left() {
        for val id <- before(right, tick) { yield(id); }
        yield(entry);
    }
    for val id <- remaining(right) { yield(id); }
    return;
}

for val id <- audit_order(billing_entries(day), refunds) { audit(id); }
```

The cursor adapter `before` consumes entries with smaller keys. It retains the first equal or larger entry for the next comparison, and `remaining` drains the tail. With left keys 10, 20 and right keys 5, 20, 30, replay visits right-5, left-10, left-20, right-20, right-30. Keys within each input must already be nondecreasing. The pull-capable side needs one record of lookahead, with cursor-owned storage if that record is borrowed. A file or database cursor with peek/advance makes this a moderate library task. Retaining only scalar keys and IDs suffices for this example.

Two arbitrary one-shot push producers present a different implementation contract. While one producer's body is active, the stage can enter the second producer. Returning from that traversal closes it. The interface provides no operation to resume it at the next item once control has returned to the first. Reopening can repeat I/O or lose progress. Alternating two such inputs with the same bounded memory and consumption guarantees needs an additional cursor or suspension protocol. Materializing an input changes those guarantees. The example establishes a useful boundary for closed synchronous pipelines without selecting a general suspension mechanism.

## One fare search, three spellings

For a given day, itineraries A and B are offered in that order. Seats on A cost 120, 80 and 80. The seat on B costs 90. Each seat cursor takes Boolean feedback on a normal handoff. True asks it to offer another seat on this route, and false finishes that seat cursor. Itinerary traversal takes void feedback. All prices are positive and the application uses zero for no match. Assume successful cursor cleanup in the traces below.

**Reference program.** Single-brace loop bodies put the return in the caller. This gives a reference for order, feedback and cleanup under the established generator contract.

```cod
proc first_affordable(day:u64) -> u64 {
    for val route <- itineraries(day) {
        for val fare <- fares_on(route) {
            if (fare <= 100u64) { return fare; }
            continue true;
        }
    }
    return 0u64;
}
```

**Candidate reusable declarations for the deferred discussion.** The proposed `here src() yields Y -> F` accepts an already argument-bound producer that offers Y and receives F on each normal handoff. A stage supplies the future consumer when it invokes src. The generator's own outer `yields` declares a separate downstream handoff. Thus both the source and the consumer may be used synchronously through the stage. Only the downstream consumer occupies the implicit first `yield` slot supplied by the final loop. The upstream formal still needs its own bounded producer binding.

```cod
proc fares_for(here routes() yields u64) yields u64 -> bool {
    for val route <- routes() {
        for val fare <- fares_on(route) {
            continue yield(fare);
        }
    }
}

proc affordable(here src() yields u64 -> bool, limit:u64) yields u64 -> bool {
    for val fare <- src() {
        if (fare > limit) { continue true; }
        continue yield(fare);
    }
}
```

For comparison, the callback-shaped signature of affordable would be `proc affordable(here yield(fare:u64) -> bool, here src(here offer(fare:u64) -> bool) -> void, limit:u64) -> void`. This is a design expansion of the two protocols. It exposes the void final completions and the two Boolean replies explicitly. The shorter declaration keeps the same distinctions while using the spelling already familiar from generators. A bound named generator, a callback-first operation, a callable data receiver or another stage can satisfy src. Check the offer and reply modes, labels, owner lifetimes and checked failures for the actual source. A source offering the same prices with void feedback cannot supply this formal. A different source implementation with the complete required protocol can.

**Nested application or a pipeline.** These are alternative proposed loop headers for the same procedure above. In both, the loop body returns the first fare and the procedure returns zero after the loop if there was no handoff.

```cod
for val fare <- affordable(fares_for(itineraries(day)), 100u64) { return fare; }

for val fare <- itineraries(day) !> fares_for() |> affordable(100u64) {
    return fare;
}
```

The nested form identifies the upstream argument. In a uniquely established producer-parameter position it binds the ordinary arguments of `itineraries(day)` and leaves the consumer for the stage. In the forward spelling, `!>` identifies this source position explicitly and `|>` connects the processors. Both examples prepare day and the limit once in written order and enter each source body when driven. Stored configurations and replay have separate contracts.

With a consumer that always replies true, the two stages offer 80, 80 and 90. In the first-result program, the rejected 120 receives true from the filter. Its downstream body never runs. The first 80 reaches the caller's `return`; it supplies no Boolean reply. The active seat cursor and then the itinerary cursor close once, and B is never opened. False on a normal handoff has a different, application-defined meaning here: the current seat cursor finishes and the itinerary stage can advance to B.

**Practical precedents.** The conduit tutorial walks a directory, filters Haskell filenames, runs a file source for each surviving path and feeds one sink. Its forward chain makes the dependent traversal easy to see. A deployed use in Pantry takes a caller-supplied byte sink and feeds it alongside hash and size checks. Both separate the user's operation from a particular input implementation. [conduit tutorial](https://github.com/snoyberg/conduit) · [Pantry HTTP download code](https://github.com/commercialhaskell/pantry/blob/master/src/Pantry/HTTP.hs)

The streaming library reads two lines with `S.stdoutLn $ S.take 2 S.stdinLn`, an example of nested application. In the pipes author's logging example, a prefix stage follows `take` with `drain`: only the first log is forwarded, while the remaining producer work still executes. Preserving that distinction is why the CoD comparison specifies when the source stops. For a typed reply at each offer, the closer Haskell analogue is the lower-level `Pipes.Core.respond`, which sends an output and resumes with the downstream reply. [streaming examples](https://github.com/haskell-streaming/streaming) · [pipes logging example](https://haskellforall.com/2014/02/streaming-logging) · [Pipes.Core](https://hackage-content.haskell.org/package/pipes-4.3.16/docs/Pipes-Core.html#v:respond)

When named type signatures are available, compare contextual binding in a parameter with a known producer signature against requiring explicit pipeline syntax. The fare example above supplies the same candidate workload for both designs. If overloads permit both a value call and a producer binding, compare the use of an argument name and an explicit pipe to express intent. [Deferred design topic](../docs/DesignTopics.md#named-signatures-and-pipelines)

## Preparation, control and storage

Prepare the source receiver and written arguments once in source order when binding the immediate traversal. Start the source body when a stage invokes its upstream producer. Thus a stage that skips its input still observes effects from preparing that input's arguments, and never enters the source body. Calling a source twice may observe different state or consume more input. A proposed reusable plan stores its own admissible configuration. A new traversal supplies a live source and consumer and performs its own preparation.

Establish yielded types, modes, labels, owners, feedback and checked failures before inspecting the final consumer body. A `func` stage must satisfy its own effect restrictions. Caller-owned work follows the existing handoff rule. A scalar or borrowed item uses the normal receiving rules. A borrowed output still needs one provable owner for its component throughout the traversal. Feedback matters in both directions. For example, a stage consuming a producer that requires Boolean feedback must provide a Boolean even for an item it filters out, when the downstream consumer never ran. A stage can declare that policy in its code. [Yield and feedback contracts](RangedForAndGenerators.md#generator-and-yield-signatures)

Consider an upstream source with a deferred close. If a prefix stage stops its own traversal of that source, the source runs its close and control resumes in the stage. If the final consumer executes an outward return, it exits the callable that supplied that body, through all entered cleanups. A consumer exception follows the existing generator catch-bypass rule. Completed effects remain observable on every path. A proposed elaboration must preserve these distinct control destinations through every intermediate stage. [Generator control and cleanup](RangedForAndGenerators.md#generalized-for-loops)

### Lexical variables for open pipelines

For a builtin partial connection, the local declaration is `here name = expression;`. It introduces a local binding of the existing bounded `here` callable kind. The initializer determines which endpoints remain open and their complete protocols, including labels, feedback, owner requirements and checked failures. A capture-free `func` value cannot express the general case: a particular connection may already refer to two processor instances, a source argument and the caller activation authorized to make a terminal call. The new permission is naming that bounded connection between statements in its live lexical scope. [Callable kinds and direct access](CallableStorageAndHereAccess.md#callable-value-kinds)

```cod
# p1 and p2 have no applicable library composition overload in this module world.
here stages = p1 |> p2;                  # Both ends open.
here today = invoices(day) !> stages;    # Needs a consumer.
here to_report = stages $> report;       # Needs a source.

today $> report;
invoices(tomorrow) !> to_report;
```

These initializers prepare their operands once. The call to `invoices` takes a scalar day in this example: preparing today fixes that day, and driving today enters the source. The prepared stages retain the selected functor targets; rebinding p1 later does not redirect them. An ordinary write to the selected processor's configuration remains visible under its contract. The builtin named terminal still makes its direct call on each offer. A fully connected expression executes and produces its terminal result under the ordinary rules. Using a local binding adds no intermediate list, cursor, implicit replay or shared closure slot. A second invocation of a live one-shot cursor still observes the cursor's actual progress.

The lexical owner holds the binding's storage. Its dependencies can have **different actual owners**. An implementation can use static entry selection or an entry and lexical descriptor with checked owner/locator pairs, prepared scalar inputs and any necessary activation or control-target evidence. Each construction site knows its own dependencies. A helper receiving the binding as a compatible here parameter needs only its checked protocol and bounded-use guarantee, regardless of the number of owners at different supplying sites. The representation grants no access to manufacture a new owner, and lexical backing growth must preserve correct dereferences.

Check lifetime through every invocation or synchronous forward, including deferred uses through their actual execution point. If p1's region is consumed and replaced between construction and use, a later use of stages rejects even if the replacement has the same type and the lexical binding is still in scope. Returning the bounded connection, installing it for use after its supplier exits, or embedding it in a region, including through a generic carrier, rejects. An owning input or active control target retains its ordinary consumption and lifetime obligations. Native specialization that eliminates every code pointer still leaves the same source-level restrictions.

The composite contract covers initialized local bindings and repeated valid use. Mutable replacement across branches remains open because it must preserve every possibly selected owner's identity, prepared state and active invocation.

### A regional functor returned by an overload

A library can choose an operator result that is an ordinary nominal ADT with a call-operator method. For example, an invoice policy library wants a reusable snapshot of a price floor and a row limit. Only two scalar fields need to survive. The call operator takes a fresh source for each execution, filters by the stored floor and counts accepted rows in invocation-local storage.

```cod
data InvoiceRule { minimum:u64, maximum_rows:u64 }

func InvoiceRule(cents:u64, limit:u64) {
    minimum <- cents;
    maximum_rows <- limit;
}

func (const InvoiceRule in R).(())(here rows() yields u64)
    yields u64 where {const owner R} {
    val left <- self.maximum_rows;
    if (left == 0u64) { return; }
    for val cents <- rows() {
        if (cents >= self.minimum) {
            yield(cents);
            left <- left - 1u64;
            if (left == 0u64) { break; }
        }
    }
}

func (|>)(floor:InvoiceFloor in O, cap:const FirstN in C)
    -> ref InvoiceRule in O where {owner O, const owner C} {
    return make InvoiceRule(floor.cents, cap.limit) in O;
}

# floor belongs to the writable catalog region; cap may be lexical.
# active_rule is a writable ref root of type InvoiceRule in catalog.
ref saved:InvoiceRule in catalog = floor |> cap;
catalog.active_rule = saved;
invoices(day) !> catalog.active_rule $> report;
```

The first parameter supplies allocation authority in O. The constructor stores scalar values, so cap's owner may end before the catalog is used. The reader calls the `(())` method of InvoiceRule in its selected module world; the catalog contains policy data and a regional root binding. A different domain might deliberately keep live references to regional stages instead. Each stored edge then has to satisfy its ADT's same-owner, access and relocation constraints. Construct in the intended destination, use bits transfer for eligible data, or use an explicit domain graph transfer across owners. Wrapping a function pointer, a here value or a lexical borrow in a functor field cannot make it regional.

Resolve the operator normally before applying the destination's binding or placement rules. An overload that returns a ref in catalog can be received as that ref and later supplied as a processor. Writing `ref saved` or assigning a root cannot turn a builtin here connection into a data functor. Test the regional form after the original lexical cap has expired and after remapping the catalog; compare it with the rejected stale-owner use of the builtin lexical form. [Regional field placement](RegionalAndLexicalStorageRefactor.md#consequences-of-one-nominal-adt) · [Functor lookup](DomainLanguageDesign.md#type-level-call-operator-lookup)

## Open decisions and acceptance

### Composite implementation target

Plan the full role checker, preparation and execution representation, and acceptance cases before writing compiler code or tests. The first end-to-end implementation target is a composite invoice-review pipeline. It exercises saved references under distinct owners, an overload with an immediate construction effect, a deferred traversal, a result-bearing terminal and its mirrored spelling:

```cod
here screening = select_policy() |> flag_disputes;
here reviewed = invoice_lines(select_day()) !> screening;

val count <- reviewed !> select_window() |> select_limit() $> audit;
# Alternative run, beginning from the same state:
val again <- audit <$ select_limit() <| select_window() <! reviewed;
```

For this workload, screening is builtin composition over two receivers with different live owners. The source offers a labelled invoice ID and a readonly reference to its line in one fixed ledger owner, and accepts Boolean feedback. Screening forwards that schema and supplies true for a skipped row. Creating reviewed prepares the selected day once, even on an empty input. Rebinding either original stage slot leaves the saved receiver intact; releasing its actual owner before use rejects.

The library composition of window and limit constructs a nominal, data-only review rule in a writable region during preparation. Its call operator performs the traversal when driven and preserves the row and feedback protocol. The audit object has only a whole-traversal terminal method. It opens the review, counts rows actually audited, returns zero on empty input and, in this workload, can reply false to stop the source. For input IDs 17, 24, 25, 42, configure screening to skip 17 and the window to admit the rest. With a limit of four and an audit reply of false after its second row, audit sees 24 and 25, returns two, and the source never offers 42. Compare against explicit constructor and nested handoff calls.

With stable selector results, the forward form logs window then limit and the mirror logs limit then window. Each calls the same composition method once with canonical operand roles. Only after preparation succeeds does audit enter the source in that example. For a preparation failure, verify the actual prefix of operand effects and temporary cleanup. For a checked audit failure in a supplied body, verify generator catch bypass, protected exits and each entered cleanup.

On the same implementation path, also compare a direct scalar generator and named sink with its explicit receiving body; vary empty and repeated offers, a consumer return, input-owner invalidation, all three partial shapes, and the two genuine ambiguity cases above. Before coding, mainline assesses whether one checked representation can carry the selected method at each connection, the schedule of written operand preparation, saved dependencies, temporary lifetimes, feedback and control destinations. Language design owns the equivalence and acceptance examples; mainline owns cost-driven local syntax, representation and lowering. [Division of work](../CHANGELOG.md#h-pipeline-design-remit-20261119)

- **Binding and syntax.** For the explicit pipeline, check preparation of a supplied source’s written arguments before traversal starts. Check pipe tokenization alongside case guards, sum alternatives and existing operators. The ordinary generator call still requires its consumer. Binding a general nested expression such as `budget(half_price(src))` is deferred with the general protocols.
- **Independent composition and storage.** Check all three partial shapes as lexical variables, forwarding a single binding to a helper and driving it on independently owned sources. Check grouping, repeated-attachment rejection, terminal header ownership and the regional overload idiom above. Mutable replacement of an existing here binding remains a separate design question.
- **Observable equivalence.** Compare the fare pipeline with hand-written nested handoffs on empty, repeated and filtered inputs. Compare both directions on prebound functors, fresh versus existing receiving controls, fresh versus persistent processor state, and computed operands with visible preparation effects. Check normal feedback, skipped input, an upstream-local stop, a final consumer return and a checked consumer failure. Record candidate order, effects, control destination and each release. Include a rejecting borrowed-owner case and a rejecting feedback mismatch.
- **Research boundary.** Enumerative choice preserves the order and multiplicity the producer offers. Scheduling fairly across infinite branches, transactional rollback and resuming a suspended traversal are separate operations. For the pure, finite, fully consumed subset, compare the answer sequence with list-comprehension semantics.

## Research question and prior work

The working paper [Closed Event Pipelines — Typed Feedback and Nonlocal Return](../papers/ClosedEventPipelines.md) asks whether a transformer can accept any source that satisfies its producer interface and compose dependent choices while preserving the feedback, exits and cleanup of explicit nested loops. A fare search illustrates the question. For each itinerary, a second source offers seats available on that itinerary. A filter may skip a seat, several seats may yield the same fare, and a caller asking for the first acceptable fare can finish while both sources are open.

| Comparison | What the paper should establish |
| --- | --- |
| Haskell list comprehensions translate nested generators through `concatMap`. Icon uses generators and goal-directed evaluation to explore alternative results. [Haskell Report, §3.11](https://www.haskell.org/onlinereport/haskell2010/haskellch3.html) · [Griswold, *Programming with Generators*](https://doi.org/10.1093/comjnl/31.3.220) | For pure, finite, fully consumed sources, prove equality with the list of answers from nested enumeration, including order, duplicates and empty branches. |
| Go composes push iterators and signals early termination through the result of `yield`. Strymonas fuses pipelines that include filtering and nested flat-map. [Go range over functions](https://go.dev/blog/range-functions) · [Kiselyov et al., *Stream Fusion, to Completeness*](https://yanniss.github.io/streams-popl17.pdf) | Compare the control required in each encoding when a source requires application-specific feedback, a stage stops a single upstream traversal, or the final consumer returns from its enclosing callable. |

Use hand-written nested loops as the reference for effectful searches. Instrument both itinerary and seat producers. On a skipped seat, check which feedback reaches the seat producer even though the consumer never ran. On a return from the first accepted fare, check that the seat source closes before the itinerary source, each closes once, and neither source offers another candidate. Repeat with a consumer exception and with an empty inner source. An executable model can compare those event sequences. A prototype can separately measure time to first answer and intermediate allocations for prefix and full enumeration.

The case for a research paper rests on a general rule for typed composition and a demonstration of what it guarantees across independently written stages. If existing iterator encodings provide the same guarantees with the same static checks, a comparative design note would fit the evidence. Coarse-grained ownership and relocatable graphs form a separate research topic grounded in the [regional storage design](RegionalAndLexicalStorageRefactor.md#shared-owner-information-from-checked-declarations).
