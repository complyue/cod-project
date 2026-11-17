# Ranged for-loops and synchronous generators

Design revision: 2 · Recorded: 2026-11-17 · Status: source-design draft; enclosing return target confirmed, feedback checking selected; signature/transport integration pending.

## Design authority

- **Registry / scope:** [D-29](DesignAuthorityRegistry.md#d-29), r2, all sections. C-96/P-521–P-524 records a candidate value path and dependency constraints; the complete mechanism remains `pending-human`, not a delegated implementation contract.
- **Human baseline:** [H-577](../CHANGELOG.md#h-577) proposes a typed single-method loop body using its lexical context and confirms that explicit body return targets the surrounding callable. Normal body-expression feedback is proposed. [H-578](../CHANGELOG.md#h-578) requests receiving-spec reuse and the assignment-style headers below. [H-579](../CHANGELOG.md#h-579) prefers void-only generator completion; this draft adopts that bounded recommendation without inferring approval of the remaining signature/proof details. [H-580](../CHANGELOG.md#h-580) requires unrestricted normal body-result types for void feedback and static conformity of all normal branches for non-void feedback; [D-29-FEEDBACK](DesignAuthorityRegistry.md#d-29-feedback)/P-525–P-527 records that source boundary separately. [H-581](../CHANGELOG.md#h-581) adds explicit body feedback with continue; [H-582](../CHANGELOG.md#h-582) restricts void feedback to bare continue and rejects its operand-bearing form. [H-583](../CHANGELOG.md#h-583) selects multi-field yield results through existing structural result declarations and receiving specifications. Exact source scope and provenance are in those records.
- **Value rationale:** candidate C-63/C-65/C-73 → C-96 → D-29 under charter §§2/3.2–3.3/4.3/6.1–6.3/8/12: express traversal, data exchange and lexical control in a checked, readable program, while reusing ordinary receiving operations and natural owner lifetimes. Native outlining must preserve that program rather than define its meaning.
- **Dependencies / invariants:** D-10 calls/effects, D-14/D-28 owners, D-17 scopes/completion, D-18 callable values, D-22 result/receiving specifications, D-23 operations, D-24 inference and D-16 numerical compatibility. Preserve unique actual owners, access/readiness, exact cleanup and the no-closure-cell/no-lifetime-extension boundary. Callable views remain lexical, never regional. There is no C++ object copying/moving model.
- **Non-goals:** no retained iterator object, async callback, persistent continuation, independently resumable generator frame, first-class result group, custom final generator result or physical ABI is selected. A synchronous native call stack is not a persistent execution-state contract. D-18r113 and its separate whole-file review remain unchanged.
- **Acceptance / record:** the final section identifies observations and rejection cases for a future qualified implementation. [K-GENERATOR-DRAFT-20261117](../CHANGELOG.md#k-generator-draft-20261117) records this new draft and its open boundaries; it is not continuity qualification of a previously accepted generator mechanism. [K-GENERATOR-FEEDBACK-20261117](../CHANGELOG.md#k-generator-feedback-20261117) updates r1→2 with H-580 and a concrete pending signature proposal. No compiler support is claimed.

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

## Proposed generator and yield signatures

Use an explicit `yields` clause with two directions:

```cod
proc scan(limit:u64) yields u64 -> bool;
```

The clause reads **yield u64, receive bool feedback**. The arrow following the yield contract belongs to that clause; scan itself has fixed void completion. An ordinary callable's `-> T` still denotes its own result. The bounded generator grammar omits a separate final-result annotation rather than exposing two result arrows at the same level.

The proposed declaration shape is:

```text
func/proc name (ordinary parameters)
    yields YieldContract [ -> FeedbackContract ]
    ordinary where/throws clauses
    body-or-semicolon
```

`yields` identifies a generator method without introducing another func/proc kind. A single YieldContract uses ordinary result-mode/type/owner syntax; multiple labelled components use the structural result schema below. This extends result-contract notation to the yield boundary, not the storage/use permissions of structural groups. Existing func/proc effect restrictions remain obligations; the body's effect/throws channel still needs the separately described integration.

Recommend requiring an explicit yield contract for this initial design. Omitted feedback means **void**, not inference from the consuming body. Explicit `-> void` is equivalent. Requiring a non-void feedback contract prevents accidental return requirements from a generator's incidental use of yield. Ordinary input defaults, named arguments and owner/type parameters keep their established rules. A declaration ending with a semicolon supplies the same complete yield/feedback contract as a definition.

Callable-valued component contracts need grouping to distinguish an inner callable arrow/throws from the yield protocol; proposed spelling is `yields (proc() -> u64) -> bool`. This is signature grouping, not a positional tuple or a new runtime object. Exact grouped-type grammar remains part of the syntax proposal. Generator callable-view types, if admitted, must retain the protocol, e.g. `proc(u64) yields u64 -> bool`; they cannot erase it to `proc(u64) -> void`. That notation alone grants no new value/capture/ABI support.

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

`yield(expression)` prepares one value under YieldContract and invokes the body. Its normal expression result has FeedbackContract. Here the first response determines whether generator code proceeds to its second yield. Bool false has no implicit loop-control meaning. The generator may ignore feedback from a particular yield just as it can discard an ordinary expression result; that does not remove the declared requirement on the body.

A void-feedback yield can be used as a statement, but not as a value for `val response <- yield(...)`. Missing consumers do not create an implicit empty body: a generator invocation needs the body supplied by `for` under this bounded proposal. Zero-argument event yields remain a separate grammar extension rather than being inferred from void feedback.

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

Use expression-first named arguments inside `yield(...)`, matching the existing `expression -> name` vocabulary. Each required yielded label must be supplied exactly once; unknown, duplicate or missing fields reject. For the bounded labelled form, recommend naming every component, avoiding positional field-order meaning. The producer still evaluates written expressions once in source order. Reception may select a subset or rename fields, without suppressing unreceived expressions or required cleanup. Yield labels have no default-argument/omission semantics, and introduce no generator-local variables.

The current proposal distinguishes one unnamed component (`yields u64`, `yield(x)`) from a labelled schema (`yields { value:u64; }`, `yield(x -> value)`). Whether single-value for shorthand also accepts a one-label schema remains a receiving grammar question. There is no implicit first-field selection from a larger schema.

## Proposed for headers

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

Single-value `=` / `<-` headers consume one yielded component; they never choose the first component of a multi-field yield or consume the generator's final completion. Whether a one-label schema also admits the shorthand can be resolved with the schema-compatibility grammar; no invented field name is required.

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

New header names are loop controls, not bindings published after the loop. Their exact repeated-initialization, const, storage and materialization lifetime must be specified with D-17's loop-control rules before implementation. Do not silently treat all iterations after the first as ordinary assignment: that would change new-ref materialization and const initialization. Do not infer a fresh full scope or fresh target owner from each invocation either.

### Repeated destination selection

Ordinary one-shot reception selects existing destinations before its producer call and delivers after that call's cleanup. A generator call instead contains multiple handoffs, so its final return cannot serve as every receiving boundary.

The proposed generator-specific rule is to select existing destinations anew at each yield handoff, after the yield arguments are prepared and before the receiving operations. This preserves changes deliberately made by earlier body invocations:

```cod
# a and b are writable scalar targets with the same live owner O.
ref dst:u64 in O = a;
for dst <- values() {
    dst = b;
}
```

Under this recommendation, the first yield writes a and subsequent yields write b, assuming no other rebindings. Selecting the payload target once at loop entry would write a on every yield. This observable timing choice remains pending; header syntax alone does not settle it. Once a destination is selected for a handoff, ordinary owner-plus-locator retention applies across its preparation/delivery effects. Existing one-shot receiving rules are unchanged.

Each multi-field handoff preserves [H-576's borrowed-read overlap boundary](ConstructionAndAcquisition.md#borrowed-read-overlap-and-diagnostics): cross-entry writes overlapping another entry's borrowed payload reads are author-responsibility UB, with planned warnings and optional error escalation. This does not make ordinary sharing, earlier/later loop-body accesses or all overlapping writes undefined. The unresolved write/write-only rule stays a D-22 dependency.

## Void final completion

The bounded design fixes the generator's own normal result to **void**. Bare `return;` ends generation early; normal fallthrough also completes it. Both run required cleanup. A value-producing return belonging to the generator rejects; it does not imply a final yield. A separately nested ordinary callable may still have its own value result.

The whole `for` has no data result beyond void. There is no path-dependent result selected from the last yield, last feedback or a generator completion payload. Exceptional exits and a body return targeting an outer callable remain control completions, not values of the loop.

This removes a fourth user-defined data channel and its otherwise necessary zero-yield, early-exit, receiving and result-inference rules. It leaves useful feedback intact: a body can propose a bool response to each yield without making the generator itself return bool. Reporting aggregate outcomes can use ordinary explicitly admitted caller state or a separate ordinary callable contract; no new implicit shared slot is introduced.

The proposed `yields Y -> F` clause fixes final completion to void without a separate final-result annotation. Its F describes the result of each yield invocation, not the generator's final return; the grammar remains a proposal.

## Yield payload and normal feedback

Payload preparation follows the established component modes: a val snapshot retains evaluated bits; a borrowed component retains its selected live target and access. Labelled components are contracts, not a stored structural object. No owning output may leave a surviving borrower without an independently proved dependency/lifetime contract.

Yield does **not** exit the generator. Its final defers therefore do not run before every body invocation. The generator activation remains live through normal body execution. A reference to generator-local storage might consequently be lendable during that invocation even though it would be invalid as the generator's final borrowed return; the owner quantification and escape proof for that extension are still pending, not automatic permissions.

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

Continue does not set either the generator's final-return record or the surrounding callable's return record. It supplies only body feedback. `if returning` therefore cannot treat continue as a return instruction. Pending feedback is delivered only after required body-local cleanup succeeds; an overriding outer return or escaping failure supplies no normal yield result. Exact crossed-target cleanup policies and eligibility of control transfers originating inside cleanup remain part of the outstanding completion review.

Nested loops use the innermost lexical loop target; do not skip an inner loop merely because an outer generator accepts the operand type. A separate ordinary callable cannot continue its caller's loop. No labelled continue syntax or new value-bearing continuation for existing range/fold loops is selected here. A deferred action that can run after the body/yield lifetime cannot retain a continuation into that expired invocation.

### Multiple feedback fields

H-583 reuses structural result declarations and receiving specifications for the result of yield. YieldContract and FeedbackContract may independently be a single result contract or a labelled schema; neither requires a first-class product object. The proposed signature and explicit-feedback construction are:

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

Reception occurs at the completed body-feedback boundary, after required body-local cleanup and iteration restoration allow successful resumption. The generator's destinations are selected before that yield producer using the ordinary one-shot receiving rule and retained across body effects; the still-pending per-yield destination-selection question concerns the opposite, loop-header receiving direction. The two boundaries must not be conflated. A body that returns from the outer callable or escapes with an error delivers no feedback and performs no final generator receiving updates. Earlier observable effects remain; there is no rollback.

H-576's borrowed-read/write UB and planned diagnostics also apply to generator receiving specs consuming feedback fields. Independent val snapshots, ordinary target sharing, fixed-owner/access/init rules, source-ordered producer effects and the unresolved write/write-only rule are unchanged. No body-local or iteration-owned reference may survive beyond its proved lifetime merely because the generator selected that field.

A normally completed producer expression with a compatible structural result can also be considered under D-22's direct-forwarding rules; it creates no group local. Explicit continue construction supplies the useful complete syntax without requiring a new general trailing record literal. Detailed direct-forwarding, schema typing and multi-output native transport inherit their existing pending qualification boundaries.

## Lexical scope and completion

The confirmed return target is illustrated independently of the pending generator signature:

```cod
proc search() -> u64 {
    for val x <- values() {
        if (wanted(x)) { return x; }
    }
    return 0u64;
}
```

`return x;` targets search. It is neither a local anonymous-method result nor a return value of values. Required active generator cleanup still has to run during this transfer; a source user exception must not be invented merely to encode it. How a generator-local cleanup return interacts with that already pending outer return needs an explicit target-aware rule. Void-only generator completion simplifies result transport but does not remove this control-flow obligation. Ordinary exceptions escaping cleanup remain subject to D-17's cancellation/replacement rules at the applicable boundary.

Preserve [D-17 loop scope](ExceptionControl.md#loop-scoping-rules) while completing the new construct: ordinary body groups are not full scopes; direct body declarations remain restricted; explicit `{{ ... }}` permits local declarations and per-iteration cleanup. A direct body defer registers with the nearest lexically enclosing full scope, skipping the loop. It must not be silently redirected to the dynamically active generator. Deferred loop-control snapshots and forbidden delayed borrows retain D-17's distinctions. Each iteration restores effects and releases its fresh effect-owned regions on normal and abnormal exits.

These are constraints on integration, not a claim that an ordinary callable-value implementation already meets them. The new body-entry proof must preserve direct lexical access, its return target, lexical deferred-registration destination and actual-owner validity without implicit closure storage or lifetime extension. D-18r113's independent pending capture review supplies no shortcut.

If the new form supports `else`, retain the existing zero-body-executions meaning rather than interpreting it as successful exhaustion; header controls are unavailable there. H-581/H-582 selects the generator-loop continue forms above; no break, labelled control or non-void feedback default is selected here. Nested generator composition must resolve yield to its lexical generator definition, not whichever generator happens to execute most recently; the exact nesting contract remains part of signature/control review.

## Signature and proof work remaining

The proposed `yields Y [-> F]` clause separates ordinary inputs, yielded contracts and body feedback while fixing final completion to void. The yields spelling, default-void shorthand and generator callable types remain under review; H-580–H-583 selects feedback checking, explicit continue forms and schema/receiving reuse without approving those separate syntactic details. Keep func/proc effect restrictions explicit. Ordinary generator `throws` describes outward failures; the body-to-yield checked-failure/effect interface must also be specified so a generator cannot erase a caller obligation simply by having a void result. The void-feedback discard rule grants no exception suppression.

Yield and feedback contracts must be independently established under D-24's inference principles. Select the generator method from ordinary explicit inputs and existing declaration rules before checking its body consumer; do not probe loop bodies/receivers to break an overload tie. Methods cannot be selected merely because their feedback type happens to fit the body. For the bounded recommendation, an unresolved generic feedback type must come from independent input/type evidence or an explicit contract; it is not inferred by circularly trying consuming bodies. Supporting additional noncircular feedback inference can be considered separately.

All yield sites in one selected generator method use the same established protocol. Explicit loop-body returns participate only in the surrounding callable's result checking. Ordinary actual-type dispatch, unique-owner/access rules, numeric conversions, UoM and D-16 validity continue. No generator runtime tag or automatic conversion can replace missing static branch evidence.

The next substantive boundaries are:

1. Repeated destination timing and header-control storage/initialization lifetime, especially new ref materialization, const controls, generator-local borrows and zero yields.
2. Exact declaration/grouped-type grammar and structural-schema compatibility, type/owner quantification, independent inference and checked effect/exception composition. A body failure reaching yield must have a defined catch boundary; a nonlocal return is not a catchable exception.
3. Target-aware propagation of body return through generator cleanup, including generator-local returns or failures during that cleanup, nested iteration and the lexical meaning of `if returning`.
4. Nonescaping body-entry proof and transport, including direct/stored/generic generator invocation if admitted. Ordinary callable values or annotations alone do not prove the body's required lexical permissions.

Do not expand the design with custom final results, resumable iterators or a coroutine runtime to avoid those checks.

## Acceptance before implementation qualification

The following is a review checklist, not current passing-test evidence:

- Once-only invocation/argument preparation; zero, one and multiple yields; explicit labels/subsets/renaming; all scalar new/existing receiver combinations and invalid operators. No arbitrary spec expressions or forward receiver dependencies.
- Distinct val snapshots and live borrowed targets; fixed owners/access/init/null rules; destination retention across permitted backing growth; defined nonoverlapping reception and diagnostic-only checks for H-576 UB examples.
- Loop-control lifetime/const/materialization and omitted-output cleanup; no escaping generator-local, iteration-local or callable-bearing storage; complete cleanup on partial receiving or feedback failure.
- Void feedback accepts heterogeneous discarded fallthrough branches, preserving effects/failures/cleanup and ordinary intermediate typing; only bare continue is admitted. Reject valued continue even with a void-typed operand, and preserve an explicit separate operand evaluation followed by bare continue. Non-void feedback accepts compatible tail/continue results, rejects bare continue and missing/incompatible normal branches, and checks recovered catches. Escaping throw/enclosing return is not a missing normal value; no runtime check substitutes for the static contract.
- Distinguish feedback from enclosing return, generator fallthrough/early void return and discarded per-yield responses; reject a generator-owned non-void return. Generator cleanup runs on exit, not at every yield. Explicit/omitted void, declaration/definition protocol agreement, labelled completeness/effect order and nested callable-type grouping need syntax acceptance. Multi-field feedback must reuse complete production, partial/renamed/new/existing receiving, pre-yield destination selection, component owners and omitted cleanup; control exits must publish no final feedback writes.
- Direct lexical body access, ordinary body-scope restrictions, explicit full-scope cleanup, outer defer registration and iteration effect restoration. Nested loops and generator invocations preserve lexical targets without implicit closure cells.
- Checked/unchecked failures, normal feedback preparation failure and nonlocal completion across generator cleanup; retained return inspection/replacement and cleanup priorities must be settled before these paths can qualify.
- D-16 comments/numerics/representation/transfer invariants; reference interpretation and native observations agree with the selected source contract. No build/test or permanent native layout is prescribed by this draft.
