# Ranged for-loops and synchronous generators

Design revision: 16 · Recorded: 2026-11-18 · Status: selected generator/feedback syntax and checked forwarding; remaining scope, effect proofs and transport pending.

## Design authority

- **Generalized for-loops.** [K-GENERALIZED-FOR-20261118](../CHANGELOG.md#k-generalized-for-20261118), D-29r16/C-96, D-18-HERE-PARAM and D-18-HERE-BREAK. H-603 and H-HERE-ORIGIN-HANDOFF-20261118 establish caller-owned execution. H-HERE-BREAK-CONTINUE-20261118 distinguishes feedback, completion of the receiving call and supplier exit. The [two-implementation example](#generalized-for-loops) records the user’s same-generator, two-call-forms direction under charter §§3.2/4.3/6.1/8.1 and P-521–P-527/P-558–P-560/P-621–P-622. The example checks equal visits, feedback and final completion. [D-29-GENERALIZED-FOR](DesignAuthorityRegistry.md#d-29-generalized-for) records the scope and candidate relationships P-623/P-624. H-600 still fixes each borrowed yield owner. A bare generator-loop break follows from void completion under either spelling. Detailed producer admission, protocol checking and native implementation qualification remain open.
- **Full scopes inside either body.** D-17-FULL-SCOPE and D-18-HERE govern direct access to enclosing slots, separate local return and independently drained cleanup in double braces. H-CALLABLE-MODEL-20261118 and the earlier scoped control qualification apply.

- **Source and implementation scope:** [K-TRACKER-BOUNDARIES-20261117](../CHANGELOG.md#k-tracker-boundaries-20261117) continues r13→14's selected H-577–H-604 scopes under H-604's renewed derivation instruction. The remaining-work matrix below separates fixed source obligations from checker/native proofs and optional extensions; no new source rule or mechanism qualification follows. Existing C-96/C-65/C-84 and P-521–P-560 relations retain their recorded scope and candidate charter §§2/3.2–3.3/4.3/6.3/8/12 rationale. Preserve current invariants/acceptance, D-16 and D-18's existing qualification boundaries.

- **Generator invocation owner:** [H-604](../CHANGELOG.md#h-604)/[K-GENERATOR-HERE-20261117](../CHANGELOG.md#k-generator-here-20261117), [D-29-LENDING](DesignAuthorityRegistry.md#d-29-lending)/P-552–P-554, selects r13's yields ref T in here contract: here names this generator invocation's lexical owner and remains fixed throughout the for. Candidate C-96/C-65 → charter §§2/3.2–3.3/4.3/6.3/8/12 serves reusable temporary storage with explicit, statically checkable lifetime responsibility. The source owner role is confirmed; binding, escape, metadata and cleanup consequences below preserve H-600/H-603, D-16 and D-18's existing qualification boundaries. Full mechanism/value-path/view/native qualification remains separate.

- **Caller-owned yield handoff:** [H-603](../CHANGELOG.md#h-603)/[K-GENERATOR-CALLER-EFFECTS-20261117](../CHANGELOG.md#k-generator-caller-effects-20261117), [D-29-EFFECTS](DesignAuthorityRegistry.md#d-29-effects)/P-558–P-560, selects r11's source boundary: yield transfers control to the caller's lexical loop body; its procedure operations are not indirect calls by a func generator. The generator's own operations retain D-10's function restrictions. C-96/C-37/C-65/C-84 → charter §§2/3.2–3.3/4.3/6.1–6.3/8/12 preserves reusable traversal and explicit context permissions. Acceptance below covers caller/generator kinds, nested forwarding, operands/feedback, cleanup and existing dynamic binding/restoration. H-603 confirms this source distinction and rationale, not the full value path, checker, view/native mechanism or future pure policy. Preserve H-600 owners, H-601/H-602 init, source422, D-16 and D-18's existing qualification boundaries.

- **Signature grouping:** [K-GENERATOR-TYPE-CONTEXT-20261117](../CHANGELOG.md#k-generator-type-context-20261117), D-29-SIGNATURE/P-528–P-530, records type-position grouping and retention of independently established generator metadata under H-584/H-592/H-595. Complete view/native mechanism qualification remains separate.

- **Fixed yielded owners:** [H-600](../CHANGELOG.md#h-600)/[K-GENERATOR-FIXED-OWNERS-20261117](../CHANGELOG.md#k-generator-fixed-owners-20261117) records r8→9: each borrowed yield component and its loop control requires one statically established actual owner across the generator invocation. [D-29-LENDING](DesignAuthorityRegistry.md#d-29-lending)/P-552–P-554 records this confirmed source boundary separately from complete mechanism/value-path qualification. C-96/C-65 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves predictable typed traversal and explicit storage responsibility. Ordinary lifetime/access/identity rules remain; no fresh per-handoff owner parameter or owner union is admitted. Acceptance includes same-owner changing targets, different-owner rejection, independent val reception and delayed/feedback lifetime checks. H-604 supplies generator-local owner naming; complete proof/view/native integration and D-18's value-input review remain separate; D-16 is unchanged.

- **Independent protocol establishment:** [K-GENERATOR-PROTOCOL-20261117](../CHANGELOG.md#k-generator-protocol-20261117) records r6→7 under H-592/H-595 delegation, H-584's explicit yield/default-void shape and D-24 independent evidence. [D-29-SIGNATURE](DesignAuthorityRegistry.md#d-29-signature)/P-528–P-530 retains the candidate C-96/C-65/C-84 value path to charter §§2/3.2–3.3/4.3/6.3/8/12: predictable typed composition and lexical control without body-driven selection or hidden continuation capture. The bounded protocol, labelled production and lexical-yield rules below are agent-derived source consequences; complete inference, generator-view checking, owner/effect/native proof and mechanism qualification remain separate. Preserve D-16 and D-18's separate value-input review.

- **Header receiving and materialization:** [K-GENERATOR-HEADERS-20261117](../CHANGELOG.md#k-generator-headers-20261117), [D-29-HEADERS](DesignAuthorityRegistry.md#d-29-headers)/P-540–P-542, records r5→6 source consequences selected under [H-592](../CHANGELOG.md#h-592) delegation. H-578 receiving-style headers, D-23's enclosing-full-scope scalar materialization and D-17's distinct loop-control/full-scope lifetimes determine per-handoff reception and fresh new-control initialization. Candidate C-96/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves visible operations, actual owners and observable target identity without an implicit iteration scope. Acceptance is below; complete header/native/lending proof and whole mechanism/value-path qualification remain pending. H-604 separately supplies invocation-owner lending; these header rules alone supply no regional materialization permission or ABI.

- **Receiving-overlap continuity:** [H-593](../CHANGELOG.md#h-593)/[K-RECEIVING-CORE-20261117](../CHANGELOG.md#k-receiving-core-20261117) aligns r4→5 with D-22-RECEIVE-OVERLAP/P-518–P-520: each yielded/feedback receiving spec prohibits cross-entry write/write overlap as UB as well as H-576 borrowed-read overlap. Existing warning/opt-in-error planning and defined single-operation/sequential/target-sharing behavior remain. C-96/C-65/C-72/C-73 and charter §§3.3/4.3/6.3/8/12 preserve receiving optimization and author responsibility without choosing signature variance or transport; the following header scope owns its separate receiving/lifetime derivation. Acceptance below changes only overlap cases; whole generator mechanism/value path remains pending-human.

- **Registry / scope:** [D-29](DesignAuthorityRegistry.md#d-29), r14, all sections. C-96/P-521–P-524 records a candidate value path and dependency constraints; the complete mechanism remains `pending-human`, not a delegated implementation contract.
- **Human baseline:** [H-577](../CHANGELOG.md#h-577) proposes a typed single-method loop body using its lexical context and confirms that explicit body return targets the surrounding callable. The later H-580–H-584 feedback decisions below define its normal-result and continue forms. [H-578](../CHANGELOG.md#h-578) requests receiving-spec reuse and the assignment-style headers below. [H-579](../CHANGELOG.md#h-579) prefers void-only generator completion; this draft adopts that bounded recommendation without inferring approval of the remaining signature/proof details. [H-580](../CHANGELOG.md#h-580) requires unrestricted normal body-result types for void feedback and static conformity of all normal branches for non-void feedback; [D-29-FEEDBACK](DesignAuthorityRegistry.md#d-29-feedback)/P-525–P-527 records that source boundary separately. [H-581](../CHANGELOG.md#h-581) adds explicit body feedback with continue; [H-582](../CHANGELOG.md#h-582) restricts void feedback to bare continue and rejects its operand-bearing form. [H-583](../CHANGELOG.md#h-583) selects multi-field yield results through existing structural result declarations and receiving specifications. [H-584](../CHANGELOG.md#h-584) accepts the presented yields Y [-> F] syntax, omitted-feedback void and contextual multi-field feedback/receiving model. [H-585](../CHANGELOG.md#h-585) selects automatic checked-body forwarding; [H-588](../CHANGELOG.md#h-588) refines it to bypass generator catches and resume ordinary exception handling at the lexical for site. Fixed explicit generator throws remains required. [D-29-SIGNATURE](DesignAuthorityRegistry.md#d-29-signature), [D-29-FAILURES](DesignAuthorityRegistry.md#d-29-failures) and [D-29-CONTROL](DesignAuthorityRegistry.md#d-29-control) keep those source decisions separate from complete implementation qualification. [H-586](../CHANGELOG.md#h-586) requests assessment of private/native propagation versus special exception wrappers; no public wrapper hierarchy is selected. [H-587](../CHANGELOG.md#h-587) requires entered try/finally cleanup on every protected exit, regardless of lowering. [H-589](../CHANGELOG.md#h-589) treats generator cleanup as part of the whole for operation; [H-590](../CHANGELOG.md#h-590) rejects using generator-local return to replace a pending body exception or enclosing-callable return. Exact source scope and provenance are in those records.
- **Value rationale:** candidate C-63/C-65/C-73 → C-96 → D-29 under charter §§2/3.2–3.3/4.3/6.1–6.3/8/12: express traversal, data exchange and lexical control in a checked, readable program, while reusing ordinary receiving operations and natural owner lifetimes. Native outlining must preserve that program rather than define its meaning.
- **Dependencies / invariants:** D-10 calls/effects, D-14/D-28 owners, D-17 scopes/completion, D-18 callable values, D-22 result/receiving specifications, D-23 operations, D-24 inference and D-16 numerical compatibility. Preserve unique actual owners, access/readiness, exact cleanup and the no-closure-cell/no-lifetime-extension boundary. Callable views remain lexical, never regional. There is no C++ object copying/moving model.
- **Non-goals:** no retained iterator object, async callback, persistent continuation, independently resumable generator frame, first-class result group, custom final generator result or physical ABI is selected. A synchronous native call stack is not a persistent execution-state contract. D-18 governs access to the actual supplying activation.
- **Acceptance / record:** the final section identifies observations and rejection cases for a future qualified implementation. [K-GENERATOR-DRAFT-20261117](../CHANGELOG.md#k-generator-draft-20261117) records this new draft and its open boundaries; it is not continuity qualification of a previously accepted generator mechanism. [K-GENERATOR-FEEDBACK-20261117](../CHANGELOG.md#k-generator-feedback-20261117) updates r1→2 with H-580 and a concrete pending signature proposal. [K-GENERATOR-FAILURES-20261117](../CHANGELOG.md#k-generator-failures-20261117) records r2→3 under H-584/H-585, with source-level failure composition, H-588 catch bypass, H-587 cleanup and outstanding proof scope. [K-GENERATOR-CLEANUP-20261117](../CHANGELOG.md#k-generator-cleanup-20261117) records r3→4: reuse ordinary outward failure rules while preserving the separate lexical return destinations. No compiler support is claimed.

## Data and control directions

The proposed synchronous model invokes the generator once, preparing its ordinary arguments once. Each `yield(...)` invokes the loop body with a freshly prepared payload. Normal body completion resumes that yield. The body uses the loop site's lexical bindings directly; ordinary callable value-capture snapshots are not a substitute for that access.

| Direction | Contract |
| --- | --- |
| Caller → generator | Ordinary explicit inputs, parameter initialization and effects. |
| Generator → body | Single yielded component or labelled component contracts, received through the header. |
| Body → yielding generator | Normal body-expression feedback or explicit continue completion. H-580 governs normal results; H-581/H-582 requires bare continue for void and a compatible operand for non-void. |
| Generator → caller | Void completion after required cleanup. |
| Explicit body `return` | Completion of the lexically surrounding callable, not feedback to yield. |

Describing the body as a typed anonymous single method does not make it an ordinary source callable scope or an escaping callable value. The generator may invoke it only while the enclosing loop invocation is active under the proposed synchronous boundary. Retention, deferred invocation after that boundary, or implicit boxing of outer variables requires a different design and is excluded here.

## Generalized for-loops

A generator decides when to enter the caller’s body, which items it supplies and what to do with the feedback. **The same generator accepts either a `for` header or a single-brace here call.** Selecting a call form does not select another traversal algorithm. The producer can be written with `yields Y -> F` and `yield(x)`, or as a void operation whose first `here` parameter has the corresponding inputs Y and result F. One implementation serves both call sites.

```cod
func ascending() yields u64 -> bool {
    if (not yield(1u64)) { return; } else {};
    yield(2u64);
}
func descending(here step(n:u64) -> bool) -> void {
    if (not step(2u64)) { return; } else {};
    step(1u64);
}

val by_for <- 0u64;
for val n <- ascending() {
    by_for <- by_for * 10u64 + n;
    continue true;
}
val by_call <- 0u64;
ascending() {=> n;
    by_call <- by_call * 10u64 + n;
    continue true;
};
```

Each accumulator records visit order. With ascending, both end at 12. Replacing ascending with descending in **either** call site records 21. If both bodies send `continue false;` on the first visit, ascending records 1 and descending records 2. Each producer explicitly tests that feedback. A third generator may ignore it and keep visiting. Empty, repeated, filtered, recursive and destructive traversals keep their own behavior in either spelling.

| Inside the single-brace body | Destination in both spellings |
| --- | --- |
| Normal tail or `continue f;` | Resumes the producer at the current handoff with F. Use bare `continue;` for void feedback. The producer chooses its next step. |
| `break;` | Finishes the whole void traversal, runs required cleanup, and resumes the supplier just after the original call or loop. A generator has no final payload for `break value;`. |
| `return value;` | Returns from the callable containing the supplied body, through traversal cleanup. |
| Escaping checked failure | Reaches the supplying call or loop site. Producer catches around the handoff cannot intercept it. Producer-originated failures retain the producer’s own contract. |

**What must match.** Both spellings bind the same ordinary arguments and defaults, use the same method-selection evidence, invoke the same selected implementation once, and check the supplied body against its established protocol. A `yields` declaration exposes the body as the implicit first here parameter in the here-call spelling. Written positional arguments still start with the first ordinary producer parameter. For a matching here-first declaration, a `for` body fills that first parameter. Its result type F is independent of the traversal’s final void result. A receiver with an unrelated non-void final result keeps its ordinary here-call contract. Switching spelling cannot resolve a genuine ambiguity or infer a missing protocol from a body.

At each handoff retain evaluation and reception order, labels, val/ref/ptr/owning modes, owner, access and cleanup. A `for` header can select or rename fields, transfer bits with `<-`, or rebind existing destinations with `=`. Compare the here-call body with **the same receiving actions**. Introducing an independent scalar snapshot in just one version can change the program. An equivalent call must preserve the effects of header reception even when the producer invokes the body several times or never invokes it. [Receiving headers](#for-headers)

Each borrowed generator output keeps its fixed actual owner for the traversal, including a generator-invocation owner spelled `in here`. A different owner on each callback invocation is an additional interface capability of an ordinary visitor. Calling a generator with braces cannot change its declared owner contract. A callback can also use double braces to request a local return and ordinary callback exception matching. Only the single-brace group supplies the loop control destinations above. These rules describe source behavior and do not require a stored callback, a coroutine, implicit function-value conversion or one native representation. [Fixed yield owners](#one-fixed-owner-per-borrowed-yield-component) · [Collection visits](CollectionsAndBuiltinContainers.md#observation-and-owner-scope) · [Here control destinations](CallableStorageAndHereAccess.md#break-and-continue-in-a-single-brace-here-body)

## Generator and yield signatures

Use an explicit `yields` clause with two directions:

```cod
proc scan(limit:u64) yields u64 -> bool;
```

The clause reads **yield u64, receive bool feedback**. The arrow following the yield contract belongs to that clause; scan itself has fixed void completion. An ordinary callable's `-> T` still denotes its own result. The bounded generator grammar omits a separate final-result annotation rather than exposing two result arrows at the same level.

H-584 selects this declaration shape:

```text
func/proc name (ordinary parameters)
    yields YieldContract [ -> FeedbackContract ]
    ordinary where/throws clauses
    body-or-semicolon
```

`yields` identifies a generator method without introducing another func/proc kind. A single YieldContract uses ordinary result-mode/type/owner syntax; multiple labelled components use the structural result schema below. This extends result-contract notation to the yield boundary, not the storage/use permissions of structural groups. Existing func/proc effect restrictions remain obligations. H-585 supplies the checked-exception forwarding rule below; dynamic effect eligibility and complete proof/transport remain separate.

The bounded syntax requires an independently declared yield contract, either on the definition or from an applicable already established declaration/expected generator contract. It introduces no bare untyped yields clause or discovery of generator status from incidental body syntax. Under H-584, omitted feedback means **void**, not inference from the consuming body. Explicit `-> void` is equivalent. Requiring a non-void feedback contract prevents accidental return requirements from a generator's incidental use of yield. Ordinary input defaults, named arguments and owner/type parameters keep their established rules. A declaration ending with a semicolon supplies the same complete yield/feedback contract as a definition.

Callable-valued component contracts use type-position parentheses to separate their own arrows/throws/where clauses from the surrounding yield protocol. The grouping has no expression evaluation or value-category effect. It does not create a positional tuple or runtime group.

### Grouped callable contracts and protocol preservation

```cod
proc callbacks() yields (func(u64) -> u64) -> bool;
proc exchanges() yields (func() -> u64) -> (func() -> bool);
```

Callbacks yields one function value and expects bool feedback. Exchanges yields one function value returning u64 and expects a function value returning bool as feedback; exchanges itself still finishes void. Parenthesize a callable type used as an entire single yield or feedback contract, keeping any inner where/throws clauses inside the parentheses. An outer arrow or clause after the closing parenthesis belongs to the generator. This local grouping rule avoids a second precedence interpretation; it does not revise ordinary numeric/UoM/fraction type grammar or expression grouping.

A labelled schema retains its explicit field boundaries and each component's own type/mode/owner contract. Parentheses group a type, not the labels into a first-class value. Nested structural result schemas, positional tuples and extra payload storage permissions remain excluded under D-22.

Where an existing callable-view position admits a generator contract, retain the complete protocol, for example `func(u64) yields u64 -> bool`. Do not erase that contract to `func(u64) -> void`, treat feedback as the generator's ordinary result, or invoke it without a compatible supplied body. `typeof` of a named generator preserves ordinary parameters, callable kind, yields/feedback, fixed owners, declared failures and the existing definition-bound default contracts; it does not infer missing information from the consuming body. Multi-method origin requirements remain D-10's ordinary complete-family requirement. These are metadata obligations, not a new callable-value storage/capture mechanism or ABI approval.

Method selection still precedes body/receiver checking. A different yield/feedback contract cannot by itself distinguish methods with otherwise indistinguishable inputs, resolve an overload tie or trigger a retry after body checking fails. Preserve existing ordinary component compatibility; this grouping rule does not select new generator variance, effect polymorphism or implicit adapters. Complete generic/view checking, native transport and supported storage remain independently qualified work.

### Independent protocol establishment

Establish Y and F before checking the consuming loop body. Ordinary input/type/owner evidence can resolve parameters inside those contracts; an unresolved receiver, body tail or continue operand cannot supply missing evidence. Omitted feedback on a declaration is void, never a type variable to infer from the body. A definition that inherits an independently established protocol does not replace its declared F with void merely because it omits a repeated clause. Conflicting declarations reject under the ordinary callable rule.

```cod
proc emit(x:T) yields T -> bool where {T};
# Calling emit(7u64) establishes yielded u64 and bool feedback.

proc unresolved(x:u64) yields u64 -> F where {F};
for val n <- unresolved(7u64) { true }
# Reject: the body cannot choose F for the selected generator.
```

Every yield site is checked against that one protocol, including recursive methods and paths not taken by a particular invocation. Zero runtime yields does not make the body or protocol untyped. Fixed void generator completion provides no payload-type inference evidence. Ordinary concrete result modes, independent expected signatures, permitted conversions and unique-owner checks remain; no new numeric join, owner union or general result variance is introduced. Select by ordinary inputs/declarations before validating receiving and feedback, preserving named/default argument ordering and declared failures. Complete checker support for these obligations remains work.

A yield belongs to its lexically enclosing generator callable. A nested for body inside that callable may yield to that generator's consumer; it does not retarget yield to the dynamically executing inner generator. An intervening ordinary callable definition starts its own boundary and cannot capture the outer body's yield entry implicitly. A separately declared nested generator uses its own protocol. Recursive invocations use their own active entry under the same lexical contract. This source target rule does not grant escaping continuation storage or prove native transport.

### Single-value yield and feedback use

```cod
proc first_two() yields u64 -> bool {
    if (not yield(1u64)) { return; } else {};
    yield(2u64);
}

for val n <- first_two() {
    consume(n);
    n < 2u64
}
```

`yield(expression)` prepares one value under YieldContract and hands control to the caller-owned body. Its normal expression result has FeedbackContract. Here the first response determines whether generator code proceeds to its second yield. Bool false has no implicit loop-control meaning. The generator may ignore feedback from a particular yield just as it can discard an ordinary expression result; that does not remove the declared requirement on the body.

A void-feedback yield can be used as a statement, but not as a value for `val response <- yield(...)`. Missing consumers do not create an implicit empty body: a traversal invocation needs the body supplied by the loop or here-call form. Zero-argument event yields remain a separate grammar extension rather than being inferred from void feedback.

### Labelled yield components

```cod
proc inspect_one(x:u64 in O)
    yields { count:u64; ref cell:u64 in O; } -> void
    where {owner O}
{
    yield(1u64 -> count, x -> cell);
}

# storage is an existing readable/writable scalar target.
for { val n <- count; ref selected = cell; } := inspect_one(storage) {
    selected <- n;
}
```

Use expression-first named arguments inside `yield(...)`, matching the existing `expression -> name` vocabulary. Each required yielded label must be supplied exactly once; unknown, duplicate or missing fields reject. The bounded labelled form names every component, preserving declaration-order-independent schema identity without positional field-order meaning. The producer still evaluates written expressions once in source order. Reception may select a subset or rename fields, without suppressing unreceived expressions or required cleanup. Yield labels have no default-argument/omission semantics, and introduce no generator-local variables.

The current proposal distinguishes one unnamed component (`yields u64`, `yield(x)`) from a labelled schema (`yields { value:u64; }`, `yield(x -> value)`). The single-value for header may receive that one labelled component as the unambiguous single-item shorthand below. This is a receiving mapping, not type equality between a schema and scalar; no implicit first-field selection exists for a larger schema.

## For headers

```cod
for { val count <-; ref selected = node; total <- count; } := inspect_all() {
    consume(count, selected);
}

for ref current = nodes() { visit(current); }
for val item <- values() { consume(item); }

# Existing destinations with independently valid types, owners and access:
for current = nodes() { visit(current); }
for total <- values() { consume(total); }
for target <- values() { consume(target); }
```

The braces before `:=` are a [receiving specification](ConstructionAndAcquisition.md#receiving-specifications), not executable code. The following braces are the loop body. Simple labels, explicit item operators, same-name selector omission, partial reception and field/local renaming reuse that proposal. A selector is a yielded label, not an arbitrary expression or another entry's newly introduced local. Neither the spec nor the receiver types select the producer's overload or infer its yield contract.

Single-value `=` / `<-` headers consume one yielded component; they never choose the first component of a multi-field yield or consume the generator's final completion. A one-label schema also admits this single-item shorthand: select its sole component, retaining its full contract without inventing a field name. A schema with multiple labels requires an explicit receiving specification, even if a receiver type would match only one of them.

| Header | Receiving operation |
| --- | --- |
| `for ref r = gen()` | New reference control binding; ordinary borrowed-target binding or independently admitted scalar materialization. |
| `for r = gen()` | Rebind an existing reference slot, preserving its fixed owner and access. |
| `for val v <- gen()` | New independent bits control binding from the yielded value or readable borrowed payload. |
| `for v <- gen()` | Write an existing val's bits. |
| `for r <- gen()` | Write the selected target of an existing writable reference. |
| `for ref r <- gen()` | Reject under the existing new-ref initialization rule. |
| `for val v = gen()` | Reject under the existing val initialization rule. |

The distinction between declarations and existing destinations is syntactic. Ptr/null handling, callable construction/replacement and owning operations retain their separate contracts; this table does not turn them into scalar bits transfer. A new declaration's materialization permission does not apply to rebinding an existing reference.

New header names are loop controls, not bindings published after the loop. Each actual handoff initializes fresh logical control bindings before entering that body invocation. A const control may therefore initialize at every handoff while retaining its ordinary restrictions within the body; later handoffs are not user rebinding of the previous const slot. Ordinary body mutation permissions remain unchanged. With zero yields there is no control initialization/materialization, and pre-existing receivers retain their values unless ordinary generator effects changed them. The control names are unavailable after the loop or in a zero-iteration else.

Distinguish control-binding lifetime from its selected referent. D-17's restrictions on delayed captures/borrows of loop-control storage still apply. A ref control bound to an already existing target retains that target's actual owner/lifetime; its own short-lived binding does not shorten or extend the referent's lifetime. For an independent scalar yield, a new ref declaration instead performs D-23's ordinary fresh materialization, described below. This distinction grants no implicit closure cell or retention of the loop-control slot.

### Repeated destination selection

Ordinary one-shot reception selects existing destinations before its producer call and delivers after that call's cleanup. A generator call instead contains multiple handoffs, so its final return cannot serve as every receiving boundary.

Select existing destinations anew at each actual yield handoff, after the yield arguments are prepared. Select all existing destinations for that spec before any receiving write; keep each selected binding slot or payload target fixed for the handoff. This preserves changes deliberately made by earlier body invocations:

```cod
# a and b are writable scalar targets with the same live owner O.
ref dst:u64 in O = a;
for dst <- values() {
    dst = b;
}
```

The first yield writes a and subsequent yields write b, assuming no other rebindings. Yield-argument effects also precede that handoff's selection. There is no speculative destination selection/check before a yield exists: the generator may yield zero times or perform more work without another handoff. Its ordinary input arguments are still prepared once. This receiving point follows the supplied-body entry boundary, not the generator's final return. Once a destination is selected for a handoff, ordinary owner-plus-locator retention applies across its preparation/delivery effects. Existing one-shot receiving rules are unchanged.

Each multi-field handoff preserves [H-576/H-593's receiving-overlap boundary](ConstructionAndAcquisition.md#receiving-overlap-and-diagnostics): cross-entry write/borrowed-read and write/write overlap are author-responsibility UB, with planned warnings and optional error escalation. This does not forbid ordinary target sharing through distinct reference slots or earlier/later sequential loop-body operations. The rule applies separately to each handoff; it is not a new cross-iteration no-sharing requirement.

### Fresh scalar materialization keeps the enclosing full-scope owner

D-23 places a new ref's independently returned scalar bits in its declaration's enclosing full lexical scope. D-17 specifies that the loop and its ordinary body are not full scopes. Therefore each `for ref item = values()` handoff with a scalar val payload creates a fresh semantic target in the enclosing full scope. There is no implicit per-iteration owner. The ref control's visibility ends with its body invocation, but references to that separately materialized target may survive under the target's ordinary owner/lifetime contract.

```cod
proc two_values() yields u64 {
    yield(1u64);
    yield(2u64);
}
proc distinct_targets() -> bool {
    ref first:u64 = 0u64;
    ref last:u64 = 0u64;
    for ref item = two_values() {
        if (item == 1u64) { first = item; } else {};
        last = item;
    }
    return first is not last;  # true: the two materializations are distinct.
}
```

This saves selected referents, not the loop-control binding. It does not authorize a delayed block capture of item or borrowing the control slot after its lifetime. The initial first/last targets and every per-yield materialization obey their ordinary enclosing-scope cleanup. A ref header receiving a borrowed yield performs no such construction and keeps the producer-selected target instead. Existing ref receivers do not acquire declaration-only materialization permission.

The full-scope target lifetime can accumulate storage when targets remain observable. That is the ordinary repeated-construction cost, not a hidden lifetime extension. Register/stack allocation or reuse is permitted only when complete reference/identity/cleanup observations are preserved; retained references in the example forbid collapsing both targets to one slot. Callable/owning result operations retain their separately qualified contracts. This scalar rule grants no regional allocation or wider owner-placement permission.

To request per-iteration storage, use a val header and an explicit body full scope:

```cod
for val item <- two_values() {
    {{
        val local <- item;
        ref current = local;
        consume(current);
    }}
}
```

Here consume must not retain a borrow beyond its admitted lifetime. Local and current end with the explicit full scope; no corresponding target is allocated by the val header's ordinary control initialization. A full scope around the whole loop instead bounds all header materializations to that surrounding scope. Explicit owner annotations must still match the actual selected/materialized owner under existing rules. Generator-local borrowed yields and their owner quantification remain a separate lending proof scope.

## Borrowed yields and owner scope

A receiving header does not replace the yielded target's actual owner with the caller's lexical owner. Under an already established input-owner contract, ordinary reference binding, target writes and independent bits reception need no new lending rule:

```cod
proc twice(x:u64 in O) yields ref u64 in O where {owner O} {
    yield(x);
    x <- 9u64;
    yield(x);
}
proc observe(x:u64 in O) where {owner O} {
    ref saved:u64 in O = x;
    for ref item = twice(x) { saved = item; }
    assert saved == 9u64;
}
```

The body may retain that reference wherever O's ordinary lifetime permits, including beyond the for operation. It names the live target rather than a snapshot. A val header instead establishes independent bits at each handoff. Yield introduces neither exclusive access nor a prohibition on the generator later editing the same target. Ordinary permissions, initialization and receiving-overlap rules still apply.

H-604 names the generator invocation owner through the in here yield contract. Synchronous execution keeps that scope live through body execution and required generator cleanup, without extending it after completion; other internal owners still need their own declared relationship and lifetime proof. Retaining such a target in an enclosing result, saved reference, callable view, symbolic binding or later deferred action requires the same actual-owner proof as any other reference. A directly registered body defer belongs to the caller's enclosing full scope and may run after the generator has ended; merely registering it during a yield does not make that delayed borrow valid. An independent bits snapshot or valid explicit graph transfer can outlive its source under its own contract.

Feedback has the same requirement in the reverse direction. A reference to storage whose scope ends during body cleanup cannot become readable feedback after that cleanup. A generator-owned target may remain live on resumption, but only an independently declared feedback owner relationship can justify retaining it. Ordinary receiving slots keep their fixed owners; no feedback operation can retarget an existing slot's owner.

The checker must preserve owner identity through cleanup, nested generators and permitted backing growth; a cached raw address or equality of nominal region types is insufficient. Retaining only identity without reading a payload does not waive lifetime or fixed-owner checks. These are consequences of existing owner contracts, not new runtime tags, per-pointer owner words or permission to implement an incomplete lending protocol.

### One fixed owner per borrowed yield component

H-600 requires each ref/ptr yield component to have one statically established actual owner throughout one generator invocation. The corresponding loop control retains that owner across all its handoffs. Fresh logical control initialization does not create a new owner parameter at every iteration. Ordinary `where {owner O}` continues to bind once per callable invocation; no `yield owner` form is admitted.

```cod
proc visit_both(a:u64 in O, b:u64 in O)
    yields ref u64 in O where {owner O}
{
    yield(a);
    yield(b);
}

for ref item = visit_both(left, right) {
    item <- item + 1u64;
}
```

This accepts when left and right provably belong to the same actual owner. They may be distinct targets, chosen at runtime within O. Two live owners of the same region type do not satisfy that equality. If the signature instead has independent A and B input owners and promises a ref in A, yielding the B-owned argument requires proof that A and B are equal; absent that proof, reject. A shared abstract name, runtime owner tag or checked lifetime alone cannot substitute for the fixed-owner proof.

Each labelled borrowed output may have its own fixed owner; all fields need not share one owner. The rule applies componentwise across branches, repeated yield sites, nested forwarding and stored/generic contracts. Result-bound owner labels do not provide an exception for a borrowed field whose actual owner changes between handoffs. D-22's ordinary single-call result-owner contract remains unchanged. Nullable ptr outputs, including null, retain the same fixed-owner requirement.

To produce independent scalar data from unrelated owners, declare a val yield contract:

```cod
proc visit_values(a:u64 in A, b:u64 in B)
    yields u64 where {owner A, owner B}
{
    yield(a);
    yield(b);
}
```

Each yield snapshots the scalar bits under its established val contract; no source owner travels with that payload. A caller's val receiver does not repair an invalid ref-yield protocol, because receiving cannot redefine the independently established generator contract. Ordinary access/readiness, effects, numerical validity and temporary cleanup remain.

A single generator activation may have a stable local owner while synchronous body execution occurs. Repeated fresh inner scopes or newly acquired regions cannot be treated as that owner merely because they use the same source declaration. No implicit lifetime extension or promotion follows from the fixed-owner rule.

### Generator invocation owner

H-604 selects the following yield contract. Here denotes the selected generator invocation's full lexical scope owner, fixed across the entire for operation; H-600's rejection of changing per-yield owners remains.

```cod
proc scratch() yields ref u64 in here {
    val cell:u64 <- 0u64;
    yield(cell);
    cell <- 1u64;
    yield(cell);
}

proc use_scratch() {
    for ref item = scratch() {
        item <- item + 1u64;
    }
}
```

Here in the generator's yield contract denotes its current invocation's full lexical owner. Both handoffs select targets in that one owner, independent of runtime addresses and backing growth. The caller's item binding retains this statically established activation-owner relationship; it does not create a fresh owner parameter per iteration. Body helpers can receive the reference under ordinary owner-parameter inference. This permits reusable local scratch graphs as well as the scalar example, subject to existing placement and readiness rules.

Bind the signature's invocation-owner role once when the selected generator invocation is entered, before its preamble/body can use it. It is a semantic owner relationship even when storage is optimized into registers or the stack; it does not prescribe a heap frame or per-pointer runtime word. Caller argument preparation keeps its ordinary scope and occurs without speculative generator-owner creation. Preparation failure creates no handoff. A zero-yield invocation still has ordinary activation cleanup but initializes no loop control.

The borrow cannot survive the generator's completion. An outer deferred call retaining item until the caller exits rejects, as does returning/storing it where its owner may already be dead. Same-owner target changes and reuse across handoffs do not imply snapshots or exclusive access. Distinct recursive/nested activations are distinct actual owners; one activation's contract cannot be substituted for another's. A target from a fresh inner scope or independently acquired region does not satisfy in here. There is no retained continuation, hidden closure storage, promoted lifetime or permission to borrow arbitrary callee locals through an ordinary return.

Required checking carries the selected activation-owner identity through direct/stored/generic generator contracts and native body entry, preserving it through feedback and required generator cleanup. The owner ends with that activation; a pending body return/exception neither transfers it to the caller nor keeps it alive after cleanup. Ordinary borrowed-return rules remain unchanged. Complete checker/native transport and the broader value path still require qualification; H-604 supplies the source relationship, not an implementation claim.

The here relationship is instantiated for each actual generator invocation; typeof/view metadata retains that relationship rather than capturing an owner from the declaration site. It names the selected generator's own scope, not any owner whose lifetime happens to enclose a yield. A forwarding generator cannot relabel an inner generator's target as its own in here target. The caller's separately written in here still denotes the caller's ordinary current scope and does not match this yielded owner merely because the keyword is identical.

The same owner relation applies componentwise to borrowed labelled yields and nullable ptr yields; a null value still retains the selected invocation owner. Val reception copies only the scalar payload and may outlive that owner when its receiving storage does. A helper receiving item through an ordinary owner parameter can use the same live owner, subject to its access contract. This grants neither ownership of the activation nor authority to release/resize it as a region.

```cod
proc observe(x:u64 in O) where {const owner O};
proc use_cleanup() {
    for ref item = scratch() {
        {{ defer observe(item); }}  # Runs while scratch's activation is live.
        # defer observe(item);      # Reject: caller-scope cleanup runs too late.
    }
}
```

The explicit inner full scope makes the demonstrated cleanup borrow valid; the deferred argument saves the target reference under ordinary preparation. It does not capture the loop-control slot or authorize broader callable capture. If the body provides borrowed feedback in the same declared invocation owner, generator-owned targets remain live at resumption; an expired body-local target still rejects.

## Void final completion

The bounded design fixes the generator's own normal result to **void**. Bare `return;` ends generation early; normal fallthrough also completes it. Both run required cleanup. A value-producing return belonging to the generator rejects; it does not imply a final yield. A separately nested ordinary callable may still have its own value result.

The whole `for` has no data result beyond void. There is no path-dependent result selected from the last yield, last feedback or a generator completion payload. Exceptional exits and a body return targeting an outer callable remain control completions, not values of the loop.

This removes a fourth user-defined data channel and its otherwise necessary zero-yield, early-exit, receiving and result-inference rules. It leaves useful feedback intact: a body can propose a bool response to each yield without making the generator itself return bool. Reporting aggregate outcomes can use ordinary explicitly admitted caller state or a separate ordinary callable contract; no new implicit shared slot is introduced.

The selected `yields Y -> F` clause fixes final completion to void without a separate final-result annotation. Its F describes the result of each yield invocation, not the generator's final return. Type-position grouping follows the signature rule above; broader callable-view integration remains separate.

## Yield payload and normal feedback

Payload preparation follows the established component modes: a val snapshot retains evaluated bits; a borrowed component retains its selected live target and access. Labelled components are contracts, not a stored structural object. No owning output may leave a surviving borrower without an independently proved dependency/lifetime contract.

Yield does **not** exit the generator. Its final defers therefore do not run before every body invocation. The generator activation remains live through normal body execution. H-604 permits a reference to generator-local storage under the in here yield contract while that activation remains live. The same reference is invalid as an ordinary escaping borrowed return; complete escape proof/transport remains implementation work.

H-580 requires the **normal value of the body expression** to follow the declared feedback contract. H-581 additionally permits an explicit `continue value;` to finish the current body invocation with feedback. H-582 requires bare `continue;` when feedback is void. Explicit `return feedback;` still targets the surrounding callable and is not a feedback-producing expression. The following rules apply regardless of how many times the generator yields at runtime; zero runtime invocations cannot excuse an invalid body.

### Void feedback is discarded-result context

When F is void, every normal fallthrough body result is discarded. Explicit continuation has the narrower H-582 syntax: only `continue;` is valid. `continue expression;` rejects even when the expression itself has type void; evaluate it separately before bare continue if its effects are wanted. The body may produce bits, an admissible reference/callable/owning value, or no value, under those categories' existing validity and cleanup rules. No value conversion to a synthetic universal void value is needed. In particular, terminal alternatives whose results are only discarded do not need a common value type:

```cod
proc observations() yields u64 -> void {
    yield(1u64);
    yield(2u64);
}

for val n <- observations() {
    if (n == 1u64) { true } else { 17u64 }
}
```

Discard is not nonexecution: expression effects and failures remain observable, and complete temporary values require their ordinary cleanup. It adds no hidden borrowed-payload read. Each expression must still be well formed; invalid access, unresolved inference, ownership consumption and checked failures do not disappear. An intermediate conditional whose value is actually consumed still needs ordinary compatible typing. This designated feedback rule does not make void a universal supertype, relax ordinary callable contracts or permit an escaping invalid reference.

### Non-void feedback checks all normal exits

When F is non-void, every normally completing terminal path must supply a result admissible under F. Check its type, value/reference mode, owner, access, initialization and permitted ordinary conversions. A bool contract admits bool feedback, not implicit numeric truthiness. No fabricated default, optional value or runtime type-check fallback repairs a missing/incompatible branch.

```cod
# Accept: both normal alternatives provide bool.
for val n <- first_two() {
    if (n == 1u64) { true } else { false }
}

# Reject: the empty alternative completes normally without bool.
for val n <- first_two() {
    if (n == 1u64) { true } else {}
}

# Reject: u64 has no ordinary implicit conversion to bool.
for val n <- first_two() {
    if (n == 1u64) { true } else { 0u64 }
}
```

Branches that explicitly return from the surrounding callable or propagate an exception do not resume yield normally, so they owe no feedback value. A local catch that recovers into normal completion does owe F. Nonreturning paths need not fabricate a value or prove termination. Existing rules for validating statically excluded paths are unchanged; these distinctions are completion-sensitive checking, not runtime tests of arbitrary values.

```cod
proc find() -> u64 {
    for val n <- first_two() {
        if (wanted(n)) { return n; } else { true }
    };
    return 0u64;
}
```

Only the else branch supplies bool feedback; the other branch returns u64 from find. The outer return is checked by find's own contract and cannot anchor or widen FeedbackContract. The same separation applies when a branch throws and is handled outside the body, while a recovered body-local catch must meet the normal-result requirement.

Feedback preparation and delivery preserve ordinary result-category rules: val snapshots retain their prepared bits; references retain the selected actual target rather than following later rebinding. Required explicit-scope cleanup and iteration restoration precede generator resumption. A feedback reference must survive that boundary; a copy at the generator cannot rescue a borrowed dead target. Callable/owning feedback needs complete-value preparation and exact cleanup. The header-control/generator-local owner proof and body-completion transport remain pending rather than being settled by type compatibility alone.

### Explicit continue feedback

`continue` completes the current invocation of the innermost lexically enclosing loop body. In this generator-loop form, the continuation destination is the active yield invocation: prepare feedback, leave the current body path, perform required local cleanup/iteration restoration, and resume the generator after yield. It does not unconditionally start another iteration; only another executed yield invokes the body again.

```cod
for val n <- first_two() {
    if (should_stop(n)) { continue false; } else {};
    consume(n);
    continue true;
}

for val n <- observations() {
    if (skip(n)) { continue; } else {};
    consume(n);
}
```

| Feedback contract | Explicit continuation | Rule |
| --- | --- | --- |
| void | `continue;` | Accept; no feedback payload. |
| void | `continue expression;` | Reject regardless of the operand type. Write `expression; continue;` to evaluate then continue. |
| Non-void F | `continue expression;` | Statically require the full F contract, including mode/access/owner/init and permitted conversions. |
| Non-void F | `continue;` | Reject: no feedback payload was supplied. |

The syntax rule does not change normal fallthrough: a void-feedback body may finish with any independently valid expression result, which is discarded. For non-void feedback, both explicit continue operands and every normal fallthrough path must satisfy F. An explicit continue ends its path; no second tail result is required. If its operand throws or returns from the surrounding callable, no feedback was prepared and that control completion follows its own rules.

Continue does not set either the generator's final-return record or the surrounding callable's return record. It supplies only body feedback. `if returning` therefore cannot treat continue as a return instruction. Pending feedback is delivered only after required body-local cleanup succeeds; an overriding outer return or escaping failure supplies no normal yield result. Generator-local returns cannot replace a body transfer targeting the caller; cleanup failures contribute to the whole for operation as specified below. Eligibility and proof of other cleanup-originating control transfers remain separately scoped.

Nested loops use the innermost lexical loop target; do not skip an inner loop merely because an outer generator accepts the operand type. A separate ordinary callable cannot continue its caller's loop. No labelled continue syntax or new value-bearing continuation for existing range/fold loops is selected here. A deferred action that can run after the body/yield lifetime cannot retain a continuation into that expired invocation.

### Multiple feedback fields

H-583 reuses structural result declarations and receiving specifications for the result of yield. YieldContract and FeedbackContract may independently be a single result contract or a labelled schema; neither requires a first-class product object. H-584 accepts the signature and explicit-feedback construction:

```cod
proc two_steps() yields u64 -> { again:bool; step:u64; } {
    { val again <-; val step <-; } := yield(1u64);
    if (again) { yield(step); } else {};
}

for val n <- two_steps() {
    continue { n < 10u64 -> again, n + 1u64 -> step };
}
```

`yield(1u64)` receives the first body's labelled feedback through an ordinary receiving spec in the generator. The feedback controls whether the generator performs its second yield and which value it supplies. Ignoring the second yield's result does not relax the body's declared feedback contract or skip feedback-expression effects/required cleanup.

The body uses contextual named construction in `continue { expression -> label, ... };`, reusing `return { ... };` field construction while retaining a different control destination. Supply every required field exactly once, reject missing/duplicate/unknown labels, and check each component's complete type/mode/access/owner contract. Written expressions evaluate once in source order. Bare continue rejects for this non-void schema. Compatibility and label identity follow D-22's current result-schema contract and its remaining questions, not a new independent record system.

The generator may select a subset, rename destinations, declare new receivers or update existing ones through the same declarative specification. Each item keeps its explicit `=` or `<-`; no arbitrary receiver expressions or new-binding dependencies appear. Omitted fields are still produced and require their ordinary cleanup. Repeated consumption, callable/owning values and owning-output dependencies retain D-22's restrictions rather than acquiring new permissions through yield.

Reception occurs at the completed body-feedback boundary, after required body-local cleanup and iteration restoration allow successful resumption. The generator's destinations are selected before that yield producer using the ordinary one-shot receiving rule and retained across body effects; loop-header reception instead selects its destinations at each actual yielded-payload handoff as specified above. The two boundaries must not be conflated. A body that returns from the outer callable or escapes with an error delivers no feedback and performs no final generator receiving updates. Earlier observable effects remain; there is no rollback.

H-576/H-593's write/borrowed-read and write/write UB and planned diagnostics also apply to generator receiving specs consuming feedback fields. Independent val snapshots remove borrowed payload reads but do not excuse two entries writing the same destination. Distinct-slot target sharing, fixed-owner/access/init rules and source-ordered producer effects remain. No body-local or iteration-owned reference may survive beyond its proved lifetime merely because the generator selected that field.

A normally completed producer expression with a compatible structural result can also be considered under D-22's direct-forwarding rules; it creates no group local. Explicit continue construction supplies the useful complete syntax without requiring a new general trailing record literal. Detailed direct-forwarding, schema typing and multi-output native transport inherit their existing pending qualification boundaries.

## Lexical scope and completion

The confirmed return target is independent of the generator feedback type:

```cod
proc search() -> u64 {
    for val x <- values() {
        if (wanted(x)) { return x; }
    }
    return 0u64;
}
```

`return x;` targets search. It is neither a local anonymous-method result nor a return value of values. Required active generator cleanup still has to run during this transfer; a source user exception must not be invented merely to encode it. A generator-local cleanup return completes only the generator and cannot replace that pending outer return. Void-only generator completion simplifies result transport but does not remove the need to preserve the actual destination. An exception that escapes generator cleanup is a failure of the whole for operation under D-17's ordinary cancellation/replacement rules; normal generator completion preserves an incoming body transfer.

Preserve [D-17 loop scope](ExceptionControl.md#loop-scoping-rules) while completing the new construct: ordinary body groups are not full scopes; direct body declarations remain restricted; explicit `{{ ... }}` permits local declarations and per-iteration cleanup. A direct body defer registers with the nearest lexically enclosing full scope, skipping the loop. It must not be silently redirected to the dynamically active generator. Deferred loop-control snapshots and forbidden delayed borrows retain D-17's distinctions. Each iteration restores effects and releases its fresh effect-owned regions on normal and abnormal exits.

These are constraints on integration, not a claim that an ordinary callable-value implementation already meets them. The new body-entry proof must preserve direct lexical access, its return target, lexical deferred-registration destination and actual-owner validity without implicit closure storage or lifetime extension. A capture-free function value cannot stand in for this live body access.

If the new form supports `else`, retain the existing zero-body-executions meaning rather than interpreting it as successful exhaustion; header controls are unavailable there. The [generalized for-loop contract](#generalized-for-loops) admits bare break for the void traversal and H-581/H-582 governs continue. Labelled control and a default for missing non-void feedback remain outside this scope. Nested generator composition uses the lexical yield target specified above; its complete control/owner transport proof remains separate.

## Checked declarations and exception execution

H-585/H-588 selects **automatic forwarding of the supplied body's unhandled checked obligations, bypassing generator catches**. The body belongs lexically to the caller. Its own local handlers work normally; once an exception escapes that body invocation, required generator cleanup runs and, unless cleanup itself escapes with a failure under the ordinary completion rules below, ordinary exception handling resumes at the caller's for site. A generator catch around yield cannot discharge or intercept that failure, even if it names the exact exception type, Error, Exception or a catch-all.

The generator remains an ordinary checked callable for its own operations. This additional body-dependent forwarding contract is specific to generators; it adds neither general exception inference nor conditional throws for ordinary callables.

### Fixed generator declarations and forwarded body obligations

The ordinary trailing throws clause is the generator's fixed outward declaration:

```cod
proc records() yields u64 -> bool throws ReadException {
    yield(read_one());
}
```

Assume read_one declares ReadException. The generator must catch that own-operation obligation or cover it with its ordinary declaration. Explicit throws remains a caller obligation under D-17/H-525 even if a current implementation/path/body appears unable to raise it. Feedback type and checkedness are independent: void feedback neither suppresses failures nor exempts body code from checking.

The body needs no new throws annotation. First select the generator from ordinary inputs/declarations; then check the consuming body. Its unhandled declared checked obligations belong to the enclosing for operation, in addition to the generator's fixed contract. Do not probe bodies to choose an overload or narrow explicit exception declarations using actual-value/optimizer evidence.

```cod
# ParseException is an already selected checked exception type.
proc parse(x:u64) -> void throws ParseException;

proc numbers() yields u64 {
    yield(1u64);
}

proc run() throws ParseException {
    for val x <- numbers() { parse(x); }
}
```

This accepts without adding ParseException to numbers. An enclosing covering catch could replace run's throws. Omitting both leaves an uncovered checked obligation and rejects. A body with no remaining checked operations contributes none. An explicit broad throws Exception on numbers would still require its full declared handling.

Conceptually the for operation must cover ordinary caller-side callee/argument/pre-entry failures, the generator's fixed explicit throws, and remaining checked body failures. This is a static obligation composition, not a source union type or a new exception class. Generator catches do not subtract body obligations. Body-local catches may handle them before forwarding; an enclosing caller catch may handle them after the for unwinds. Resolve type identities and admitted generic substitutions normally, preserving declared bounds independently of actual runtime category matching.

### Generator catches handle generator work

```cod
# load_one and read_body declare ReadException.
# report and finish_attempt have no escaping checked failure.
proc guarded() yields u64 {
    try { yield(load_one()); }
    catch (e:const ReadException) { report(e); }
    finally { finish_attempt(); }
}

proc run_guarded() throws ReadException {
    for val x <- guarded() { read_body(x); }
}
```

If load_one fails, the generator catch handles it: the body was never entered. If read_body fails, the same exception type bypasses that catch, finish_attempt executes, and the failure reaches run_guarded's for site. The run_guarded declaration therefore remains necessary unless a caller-side catch covers it. The distinction is the failure's execution boundary, not its class, construction history or filename.

A body can recover locally and provide normal feedback:

```cod
for val x <- first_two() {
    try { validate(x); true }
    catch (e:const ParseException) { false }
}
```

Assume validate declares ParseException and first_two expects bool feedback. The local catch supplies false before leaving the body. Without that local recovery, a generator catch cannot invent feedback or retry the failed body. Caller recovery outside the for resumes after that construct; it does not resume the abandoned body/yield.

The generator's checked calls or explicit throws in its own handlers/finally still need ordinary covering catches or explicit generator throws. A root-typed `catch (e:const Exception) { throw e; }` retains the declared Exception obligation; no narrower observed value excuses it. Existing bare throw/rethrow and wrapper-summary rules remain governed by D-17. Automatic body forwarding does not broaden own-generator checked permissions.

### Execution and nested-loop boundaries

For an exception escaping the body:

```text
body-local handlers and required local cleanup
    → iteration restoration / body exit
    → generator cleanup, bypassing its ordinary catches for this body escape
    → ordinary caller exception handling at the for site
```

The original exception identity, cause chain and construction trace are preserved by forwarding. It is not automatically changed into an Error or a newly constructed exception. Source-order matching and standard rethrow-wrapper transparency apply once ordinary handling resumes. A body handler cannot catch a generator failure that occurred before body entry or after that invocation completed.

Nested loops repeat this boundary locally. An inner body failure bypasses its inner generator and reaches the inner for in the outer body. A try in that outer body can handle it there. If it then escapes the outer body, it bypasses the outer generator and reaches the outer for. Use actual active invocation identity, not a process-wide current generator or a code-definition identity shared by recursive calls.

| Failure phase | Checked-contract/handler boundary |
| --- | --- |
| Selecting/evaluating the generator call and explicit caller-side inputs | Ordinary caller-side preparation; generator body handlers are not yet active. |
| Generator operations, including yield-argument evaluation and production conversions | Generator's own checked code; a surrounding generator catch may handle it before body entry. |
| Receiving the prepared yielded payload, body evaluation, feedback preparation, and required body-local completion | Loop-side body invocation. A failure escaping it bypasses generator catches and reaches the caller's for. Failed reception enters no body; failed body execution supplies no feedback. |
| Generator receiving updates after successful yield feedback | Ordinary generator-side receiving operations; generator handlers may catch these distinct failures. |
| Remaining generator operations/cleanup | Ordinary generator operations retain their own failure rules. A local return preserves an incoming body transfer; escaping cleanup failure contributes to the whole for operation under the rules below. |
| Later execution of direct loop-body defers in an outer full scope | That lexical scope's deferred pass; the completed generator cannot catch it retroactively. |

Successful feedback delivery follows required body-local cleanup/restoration. Val snapshots retain prepared bits; borrowed outputs retain selected targets. Failure or outer return cancels final feedback receiving writes and releases prepared outputs as required. Earlier observable effects are not rolled back. The mechanism may not re-execute the body, create an implicit cause or make an opaque unchecked wrapper satisfy a checked obligation.

### Finally and defer are independent of the transport

H-587 requires every entered try's finally to execute when control leaves its protected construct, including normal feedback, body continue, a forwarded body exception and return to the body's enclosing callable. A finally runs once for that dynamic exit. Several yields within one try do not each trigger its finally merely by handing back feedback; the finally runs when that try exits. Failure in a yield argument after entering the try also follows the ordinary finally rule.

```cod
proc once() yields u64 {
    try { yield(1u64); }
    finally { finish_attempt(); }
}

proc find() -> u64 {
    for val n <- once() { return n; };
    return 0u64;
}
```

Assuming finish_attempt completes normally, it runs before find delivers 1. It also runs if the body throws or completes normally. Native outlining/inlining, explicit status propagation or exception unwinding must preserve this observation; a direct nonlocal jump that skips cleanup is not an implementation of the contract.

Use ordinary try/finally or defer for resource cleanup. Generator authors need not name an internal propagation object to obtain that guarantee. H-589 includes that cleanup in the whole for operation; H-590 preserves lexical completion targets. A return belonging to the generator is not a return belonging to the consuming callable, and cannot handle the body's exception by completing the generator normally.

### Cleanup completion at the for boundary

Keep the body's outgoing transfer distinct from the generator's own completion. Apply ordinary D-17 cleanup/failure rules, with these destination constraints:

| Generator cleanup outcome | Effect on an already outgoing body exception or enclosing-callable return |
| --- | --- |
| Finishes normally, including a cleanup helper that returns normally | Preserve the body transfer. |
| Executes generator-local `return;` | Complete the generator's own void execution, run remaining required cleanup and preserve the body transfer. Never replace the caller's result with void or consume its exception. |
| A cleanup failure is handled internally and cleanup completes without an escaping failure | Preserve the body transfer. No caught internal failure alone changes the for's outward completion. |
| An ordinary cleanup exception escapes the generator | The whole for fails under D-17's ordinary exception replacement/cancellation rules. This is a new escaping failure, not normal generator completion handling the original body exception. |
| Generator-owned deferred failures remain after their pass | Apply D-17's FIFO/UnhandledDeferError boundary. Preserve an outstanding body exception in body_exception; it is not a deferred queue member. An undeliverable pending return requires ordinary cancellation and exact release. |

```cod
proc once() yields u64 {
    try { yield(1u64); }
    finally { return; }
}

proc find() -> u64 {
    for val n <- once() { return n; }
    return 2u64;
}
# find returns 1: the generator's return does not cancel find's return.
```

If the body instead throws, the same generator-local return leaves that exception propagating at the for site. Without an outgoing body transfer, generator return simply ends the for normally. It supplies no feedback and does not resume an abandoned body or restart traversal. A helper's return remains local to that helper. `if returning` in generator cleanup inspects the generator's own return state, never the consuming callable's saved result.

An explicit generator return may replace the generator's own ordinary failure under D-17; it cannot remove the separately pending body exception/outer return or unrelated deferred failures. Body exceptions remain outside generator catches and deferred handlers. The final for completion is delivered only after required cleanup; abandoned owning results are released exactly once, and no retained result is revived implicitly. Runtime cleanup replacement does not waive fixed declared throws or statically forwarded body obligations. The native representation must implement these distinctions; it cannot choose a different source priority.

D-17/H-550/H-551 still requires checked failures from deferred work to be handled by completion of their owning defer pass. A covering defer catch counts; generator/caller throws or an outer ordinary catch does not authorize residual checked queue members. UnhandledDeferError remains the unchecked aggregate of permitted residual deferred errors; ordinary body exceptions are not wrapped merely because a generator was involved.

A direct body defer registers in the nearest enclosing lexical full scope. Registration-time preparation can fail during the body invocation and forward through the loop boundary; its later action failure belongs to that outer scope's defer pass. An explicit body-local full scope drains its local defers before leaving; checked failures must still be handled within that pass. An escaping unchecked aggregate from that body-local completion follows the same body forwarding boundary. Generator-owned defers retain their own queue and coverage rules.

## Internal propagation and native lowering — assessment

H-586 asks whether special exceptions such as LoopBodyReturn <: LoopBodyPassthrough <: Error would help implement forwarding and outer return, while preferring a suitable non-exception LLVM mechanism if available. This is an implementation proposal, not approval of public exception types. H-588's catch bypass, H-587's cleanup guarantee and H-589/H-590's for-boundary/lexical-target rule constrain every candidate.

### Keep control transport outside ordinary exception matching

A public Error-derived carrier would match ordinary catch Error, catch Exception and catch-all clauses. Advising authors not to name the special types would not stop those handlers from swallowing a return or a forwarded body failure. An opaque wrapper would also change actual-type matching/checkedness, while ordinary construction would introduce trace/cause semantics. Automatically stripping it everywhere would instead let generator handlers intercept the original body exception, violating H-588.

The recommended starting point is a compiler-private completion protocol with distinct meanings, schematically:

```text
body normal completion: feedback components
body failure escape:    destination for-invocation + original exception
body enclosing return:  destination callable-activation + pending result
```

These are internal states, not CoD Exception subtypes, storable user values or a prescribed native struct. Generator ordinary catches handle only its ordinary failure path; body escape passes through required cleanup to the designated for boundary, where it resumes as the original exception. Outer return similarly remains control completion rather than a catchable Error. Actual activation identity matters under recursion and nested generators. All saved references/results still require valid owners through cleanup; no implicit closure cells or lifetime promotion follow.

### Existing compiler and LLVM evidence

The current [native emitter](../cod/lang_llvm.hh) already uses CompletionState for local pending control, an outcome aggregate for native call values/failures, and explicit control/cleanup branches. Native call outcomes currently omit the local return kind/marker; neither that outcome nor the [checked Completion](../cod/lang_completion.hh) establishes arbitrary lexical return targets. Extending that internal protocol or composing known direct control-flow graphs is therefore a plausible smaller starting change than introducing a new public exception hierarchy. This is an engineering inference, not implemented support, an ABI selection or a performance measurement.

LLVM's ordinary ret returns to its caller, and ordinary branches target blocks within the current function. Thus separate native generator/body functions need an explicit propagation protocol or unwinding; a merged native graph can instead branch through generated cleanup. These instructions do not supply CoD's lexical return/catch-bypass policy themselves. See the [LLVM language reference](https://llvm.org/docs/LangRef.html#ret-instruction).

LLVM also provides native exception-handling control flow and language-specific personalities; a compiler-private signal could use those facilities without becoming a CoD Error subtype. Such a route must still implement catch bypass, all cleanup, actual destination identity and the selected pending-completion rules across supported targets. Existing status-based CoD calls would need compatible integration. See [LLVM exception handling](https://llvm.org/docs/ExceptionHandling.html). Choose between qualified lowerings using actual complexity/performance evidence; no coroutine, unwinder ABI or exception allocation is mandated here.

### Remaining proof scope

Body checked obligations now compose directly with the generator's fixed declarations; generator handlers cannot remove them. Complete checking still needs separate ordinary generator versus body-escape paths, preparation/receiving effects, owner/initialization facts and cleanup. Nested/recursive generators, stored views and multiple selected contexts require the same proof. Unresolved coverage must reject or report a precise implementation gap rather than rely on a runtime exception check.

Dynamic symbolic effects and func/proc eligibility remain distinct from checked forwarding. H-603 checks the caller-owned body in its lexical context while retaining the generator's own callable-kind restrictions, as specified below. Nonescaping lexical entry, target-aware cleanup, body-control lifetime and complete native coverage remain pending. The public carrier hierarchy is not selected; the private protocol and native alternatives remain proposed mechanisms.

## Callable kind and supplied-body effects

H-603 defines yield as a synchronous control handoff to the caller's lexical loop body. That body's operations belong to the caller, not to the generator's ordinary direct/indirect call graph. The generator's own code retains its declared func/proc restrictions. This is a source control-flow boundary, independent of whether lowering implements the handoff with a native call, branch or another internal mechanism.

```cod
func numbers() yields u64 {
    yield(1u64);
    yield(2u64);
}
proc report(x:u64);  # May use dynamic context.

proc run() {
    for val n <- numbers() {
        report(n);  # Accept: this operation belongs to run.
    }
}
```

A func generator can therefore serve both function and procedure callers without separate traversal definitions. Func remains a restriction on its own operations and ordinary transitive calls, not a promise that no procedure-owned execution can occur between generator instructions. It already permits mutation through explicit memory and is not a purity annotation. No current rule for a future const func follows.

### Eligibility by lexical execution site

| Lexical caller | Selected generator | Supplied body's eligibility |
| --- | --- | --- |
| func | func | Function restrictions apply to the body. |
| func | proc | Reject the generator invocation itself. |
| proc | func | Procedure operations are allowed in the body; the generator stays func. |
| proc | proc | Procedure operations are allowed in each one's own code. |

A module execution site keeps its existing procedure/effect permissions. A separately defined ordinary callable always uses its own declared kind; the dynamic presence of a procedure caller grants no permission to its function helpers.

The operation's source site determines the boundary:

- Generator input expressions and caller-side defaults use their existing caller preparation rules. Definition-side defaults and the generator preamble retain their existing callee rules.
- Yield operands, generator-local feedback receiving operations and subsequent processing belong to the generator. A func generator cannot call a named proc in its yield operand. Only an eligible function value can travel as storable callable feedback.
- Loop-header reception, body statements, tail/continue feedback expressions and body-owned cleanup retain the lexical caller's permissions. They do not become generator code because a backend outlines them.
- Generator-local catches, finally and deferred actions retain the generator's restrictions on every exit. Body cleanup retains its own lexical permissions and existing registration destination. Yield does not turn a generator finally into a caller finally.
- Keep D-10's ordinary callable-argument and transitive-helper checks. An ordinary function cannot invoke a procedure parameter and cannot rename such an invocation to yield. The body entry has only its specific synchronous control lifetime; it cannot be stored, captured or returned as an ordinary escaping callable.

For a func generator with `yields u64 -> bool`, a procedure caller may compute `continue ask(n);` using a proc ask returning bool. The generator receives that bool as ordinary explicitly supplied feedback; it gains no dynamic-symbol access from the value's origin. Conversely, `yield(ask_value());` inside a func rejects if ask_value is a proc, even when every caller is a procedure. Owner/access/init checks, independently established feedback types and required cleanup remain unchanged.

### Nested forwarding preserves each lexical boundary

```cod
func forwarded() yields u64 {
    for val n <- numbers() {
        yield(n);  # Hand off to forwarded's caller.
    }
}

proc run_forwarded() {
    for val n <- forwarded() {
        report(n);  # Accept: still run_forwarded's own body.
    }
}
```

Numbers hands control to a body belonging to forwarded, so that intermediate body obeys func restrictions. Its yield then hands control to run_forwarded, whose body obeys proc rules. Adding `report(n)` directly to forwarded's body rejects. Likewise, a func cannot call a proc generator merely because the innermost consumer happens to need no effects. Recursive activation targets, stored views and generic specialization must preserve the same source boundaries; physical call-graph nesting does not determine source eligibility.

### Dynamic values and completion retain their existing rules

Lexical ownership determines names, permissions and control targets; it does not snapshot dynamic values. Yield alone installs no captured context and restores no earlier TLS state. Symbol identities remain definition-bound, while permitted symbolic reads and direct procedure calls use the bindings active when they execute. Existing explicit perform/behave and effect-scope rules remain authoritative.

For example, if a proc generator installs a temporary symbolic value before yield, that still-active binding is visible to an otherwise permitted caller-body read. An iteration-local shadow is restored to that body entry's binding before generator continuation; exiting the generator later restores its own enclosing binding. This follows ordinary nested dynamic scope, with no new capture or caller-context replay. A func generator cannot install or read those bindings itself. Required-symbol analysis must retain the caller body's demands without treating them as direct dynamic dependencies of the func generator or silently omitting them.

Separating call eligibility does not remove the body from the whole for operation's effect, initialization or lifetime analysis. It may mutate shared explicit targets before generator execution resumes:

```cod
func reread(x:u64 in O) yields u64 where {owner O} {
    yield(x);
    yield(x);
}
proc advance(x:u64 in O) where {owner O} {
    for val n <- reread(x) {
        x <- n + 1u64;
    }
}
```

Starting with x equal to 1, the val yield contract supplies 1 and then 2; x ends as 3. The second read must observe the intervening body write. A checker or optimizer must carry the body's relevant writes/readiness changes across each handoff, or conservatively invalidate facts it cannot preserve; func does not license caching mutable targets across yield. Ordinary active-borrow and owner-liveness rules still reject a body operation that would invalidate a reference needed by the suspended generator. This grants no new initialization or owner-retention mechanism.

Body checked failures still bypass generator catches, and fixed generator throws remains an obligation. Body return still targets its enclosing callable. H-589/H-590 cleanup composition, per-iteration effect restoration, H-600 fixed owners and deferred lifetime checks remain. Feedback can carry only ordinary admitted values/references; it cannot transport the caller's control entry or extend any owner lifetime.

Select the generator method from ordinary inputs and established declarations before checking the body. No body-driven kind inference, overload tie-breaking, retry or proc-to-func adapter follows. Where a generator view is otherwise admitted, its contract retains the declared kind and full protocol; the body is checked at each loop or equivalent here-call site. Complete view/nesting/effect/native proof remains work. Inlining and outlining must preserve lexical call eligibility, active dynamic bindings and actual completion targets; neither optimization qualifies a source program that violates them.

## Full scopes inside a loop body

H-FULL-SCOPE-CALLABLE-20261117 makes an explicit `{{ ... }}` an ordinary immediately invoked callable, even inside a caller-owned loop body. Its enclosing slot accesses follow D-18, its return is local, and its cleanup completes before the outer body resumes. A direct loop-body return still targets the callable containing that loop; the implicit nested callable does not inherit that control privilege. Continue/feedback must target a loop in the same callable, and yield must belong to that callable's own generator contract. Ordinary nested calls cannot reach a caller's suspended yield.

```cod
# For a generator with u64 feedback:
for val item <- source() {
    continue {{ return item + 1u64; }};  # Outer continue sends the local result.
}
# By contrast, {{ continue item; }} cannot continue the outer loop.
```

The implicit callable retains the checking site's func/proc permissions; this grants no new proc effects inside func code. A captured yielded reference may access its selected live target under the established owner, but cannot retain a loop-control slot or outlive a handoff without the ordinary proof. A local return neither cancels the generator nor returns from the loop's surrounding callable. Failure escaping the implicit invocation reaches the loop body and then follows the existing generator-bypass/cleanup contract. For a trailing here argument, single braces preserve the supplier’s control destinations and double braces establish local completion.

## Signature and proof work remaining

H-584 selects the `yields Y [-> F]` clause and default-void feedback while retaining fixed void final completion. H-585/H-588 supplies checked failure forwarding and generator-catch bypass; H-587 requires finally execution, and H-589/H-590 fixes its for-boundary composition without retargeting generator returns. Independent protocol establishment and lexical yield targets are specified above. Type-position callable grouping and protocol-metadata preservation are specified above. H-603 fixes the caller-owned body effect boundary. H-604 fixes the invocation-owner contract. Complete generator-view checking and effect/native proof/transport remain work; additional inference forms beyond the stated independent-evidence rule are optional extensions. Neither feedback discard nor body-specific exception analysis suppresses an explicit declared obligation.

Yield and feedback contracts must be independently established under D-24's inference principles. Select the generator method from ordinary explicit inputs and existing declaration rules before checking its body consumer; do not probe loop bodies/receivers to break an overload tie. Methods cannot be selected merely because their feedback type happens to fit the body. Under the bounded source rule, an unresolved generic feedback type must come from independent input/type evidence or an explicit contract; it is not inferred by circularly trying consuming bodies. Supporting additional noncircular feedback inference can be considered separately.

All yield sites in one selected generator method use the same established protocol. Explicit loop-body returns participate only in the surrounding callable's result checking. Ordinary actual-type dispatch, unique-owner/access rules, numeric conversions, UoM and D-16 validity continue. No generator runtime tag or automatic conversion can replace missing static branch evidence.

The following are remaining proof/implementation work for the selected source profile, not unanswered preferences about its behavior:

| Area | Fixed source obligation | Work still needed |
| --- | --- | --- |
| Protocol and selection | Independently established Y/F, complete labels, fixed void completion, no body-driven selection. | Complete direct/generic/view checking and proof coverage under the existing compatibility rules. |
| Storage and owners | Actual-handoff destination selection; distinct control-slot/target lifetimes; H-600 fixed owners and H-604 invocation owner. | Preserve actual owner relationships, growth-safe locations, readiness and escape evidence through every handoff and cleanup. |
| Caller-owned execution | H-603 lexical effect eligibility, direct lexical access, no escaping body entry or hidden lifetime extension. | Preserve those source sites, required-symbol demands and body writes/readiness through nested/recursive/stored execution and native outlining. |
| Feedback and control | Typed normal feedback; lexical outer return; body failure bypass; required cleanup and checked obligations. | Implement complete target-aware propagation and exact cleanup/cancellation without retargeting return or making nonlocal control catchable. |
| Native execution | Preserve all selected observations and current placement rules. | Qualify a concrete lowering and its acceptance evidence; existing ordinary callable transport is a building block, not proof of generator support. |

Under a qualified implementation scope, bounded sound acceptance may diagnose unsupported proof/transport cases. Such a limit must be distinguishable from a source-contract violation; it cannot silently choose a different owner, skip cleanup or waive an obligation. This matrix does not qualify a new mechanism by itself.

Additional generator variance/adapters, extra inference forms, generalized forwarding of internal owners, zero-argument event-yield grammar and additional labelled control forms remain separately scoped extensions. They are not prerequisites for implementing the selected profile, and existing partial receiving does not implicitly approve them. Normal `if returning` remains D-17's contract; field-level structural-return inspection is D-22's optional extension.
Do not expand the design with custom final results, resumable iterators or a coroutine runtime to avoid those checks.

## Acceptance before implementation qualification

- Run ascending and descending through both call forms with identical bindings. Require 12/12 and 21/21 respectively, plus the matching first-item results after false feedback. Check zero body entries, a bare break after one handoff, forwarded supplier return, checked failure bypass, cleanup order and rejection of a borrowed output with an unstable owner.

The following is a review checklist, not current passing-test evidence:

- Once-only invocation/argument preparation; zero, one and multiple yields; explicit labels/subsets/renaming; all scalar new/existing receiver combinations and invalid operators. No arbitrary spec expressions or forward receiver dependencies.
- Distinct val snapshots and live borrowed targets; fixed owners/access/init/null rules; destination retention across permitted backing growth; defined nonoverlapping reception and diagnostic-only checks for H-576/H-593 UB examples, including val-only conflicting writes.
- Input-owner borrowed yields may retain references through the ordinary proved owner lifetime; later generator writes remain observable through those references. Distinguish val snapshots, fixed reference-slot owners and runtime target sharing. Reject generator-local escapes, delayed outer-scope borrows and feedback to expired body storage. Check H-600 same-owner changing targets, different-owner rejection, per-field owners, null and declared val snapshots; neither fresh controls nor result-bound labels permit an owner change between yields.
- H-604 invocation-owner binding once at entry; zero-yield/preparation-failure cleanup; labelled/nullable component identity; distinct recursive activations; typeof/view metadata without captured definition/caller owners. Accept explicit body-local cleanup and valid independent bits extraction; reject delayed outer cleanup, ordinary returned/captured escapes, fresh-inner-scope substitution and forwarding that relabels another activation's owner.
- Per-handoff destination selection after yield preparation, zero-yield noninitialization, fresh const/new control initialization and enclosing-full-scope materialized target identity; distinct control-slot versus referent lifetime and omitted-output cleanup; no escaping generator-local, iteration-local or callable-bearing storage; complete cleanup on partial receiving or feedback failure.
- Void feedback accepts heterogeneous discarded fallthrough branches, preserving effects/failures/cleanup and ordinary intermediate typing; only bare continue is admitted. Reject valued continue even with a void-typed operand, and preserve an explicit separate operand evaluation followed by bare continue. Non-void feedback accepts compatible tail/continue results, rejects bare continue and missing/incompatible normal branches, and checks recovered catches. Escaping throw/enclosing return is not a missing normal value; no runtime check substitutes for the static contract.
- Distinguish feedback from enclosing return, generator fallthrough/early void return and discarded per-yield responses; reject a generator-owned non-void return. Generator cleanup runs on exit, not at every yield. Check explicit/omitted void, declaration/definition protocol agreement and labelled completeness/effect order; type-position callable grouping must preserve both protocol directions and all inner/outer clauses. Multi-field feedback must reuse complete production, partial/renamed/new/existing receiving, pre-yield destination selection, component owners and omitted cleanup; control exits must publish no final feedback writes.
- Grouped callable payload/feedback types retain their own arrows/clauses; typeof/view metadata preserves the generator protocol and definition-bound defaults, rejects void-result erasure and never uses a body to resolve method selection. No new view storage/capture/native mechanism or variance is implied. H-603 admits proc-body use with a func generator under the lexical caller's permissions; this is a source contract, not a passing implementation claim.
- Check the caller/generator kind matrix, procedure feedback versus rejected generator-owned procedure calls, nested forwarding, ordinary callback rejection, and source eligibility preserved through stored/generic/native calls. Distinguish generator/body cleanup; retain active dynamic bindings and exact iteration/generator restoration without implicit snapshot/replay or lost required-symbol demands. Preserve body writes/readiness and owner obligations across handoffs, including reread's 1/2 observations and final 3; never infer observational purity from func.
- Independent generic inputs establish yield/feedback parameters; unresolved F rejects even if a body supplies a concrete tail. Declaration/definition agreement, no receiver-based selection, zero-yield paths and recursive protocols retain complete checks. Direct lexical body access, ordinary body-scope restrictions, explicit full-scope cleanup, outer defer registration and iteration effect restoration remain. Nested for bodies yield to the lexical generator, ordinary nested callables cannot capture that entry, and nested/recursive generators retain their own active targets without implicit closure cells.
- Checked and unchecked body failures bypass exact-type, Error, Exception and catch-all generator handlers, while generator-own failures of the same type remain locally catchable. Cover nested for boundaries, caller/body local recovery, receiving/pre-entry phases and fixed throws. Reject uncovered obligations, broad-declaration narrowing, body-driven overload selection and defer-pass bypass. Preserve exception identity/trace/cause and no implicit wrapper/rollback.
- Every entered try/finally executes at protected exit on normal feedback, continue, body throw and outer return, once per exit rather than once per yield. Preserve generator defer cleanup and outer lexical registration. A public Error carrier must not silently change catch behavior; any private/native mechanism must prove the same control paths.
- Generator-local return preserves an outstanding body exception/outer return; absent such a transfer it ends the for normally. Locally handled cleanup failure preserves the transfer, while escaping ordinary failure and residual deferred aggregation follow D-17 at the for boundary. Verify lexical if-returning state, remaining cleanup and exact cancellation/release without changing destination. Target-aware transport/proof must qualify before these paths are implemented.
- D-16 comments/numerics/representation/transfer invariants; reference interpretation and native observations agree with the selected source contract. No build/test or permanent native layout is prescribed by this draft.
