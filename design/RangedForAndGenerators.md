# Ranged for-loops and synchronous generators

Design revision: 1 · Recorded: 2026-11-17 · Status: source-design draft; enclosing return target confirmed, header/feedback/signature integration pending.

## Design authority

- **Registry / scope:** [D-29](DesignAuthorityRegistry.md#d-29), r1, all sections. C-96/P-521–P-524 records a candidate value path and dependency constraints; the complete mechanism remains `pending-human`, not a delegated implementation contract.
- **Human baseline:** [H-577](../CHANGELOG.md#h-577) proposes a typed single-method loop body using its lexical context and confirms that explicit body return targets the surrounding callable. Normal body-expression feedback is proposed. [H-578](../CHANGELOG.md#h-578) requests receiving-spec reuse and the assignment-style headers below. [H-579](../CHANGELOG.md#h-579) prefers void-only generator completion; this draft adopts that bounded recommendation without inferring approval of the remaining signature/proof details. Exact source scope and provenance are in those records.
- **Value rationale:** candidate C-63/C-65/C-73 → C-96 → D-29 under charter §§2/3.2–3.3/4.3/6.1–6.3/8/12: express traversal, data exchange and lexical control in a checked, readable program, while reusing ordinary receiving operations and natural owner lifetimes. Native outlining must preserve that program rather than define its meaning.
- **Dependencies / invariants:** D-10 calls/effects, D-14/D-28 owners, D-17 scopes/completion, D-18 callable values, D-22 result/receiving specifications, D-23 operations, D-24 inference and D-16 numerical compatibility. Preserve unique actual owners, access/readiness, exact cleanup and the no-closure-cell/no-lifetime-extension boundary. Callable views remain lexical, never regional. There is no C++ object copying/moving model.
- **Non-goals:** no retained iterator object, async callback, persistent continuation, independently resumable generator frame, first-class result group, custom final generator result or physical ABI is selected. A synchronous native call stack is not a persistent execution-state contract. D-18r113 and its separate whole-file review remain unchanged.
- **Acceptance / record:** the final section identifies observations and rejection cases for a future qualified implementation. [K-GENERATOR-DRAFT-20261117](../CHANGELOG.md#k-generator-draft-20261117) records this new draft and its open boundaries; it is not continuity qualification of a previously accepted generator mechanism. No compiler support is claimed.

## Data and control directions

The proposed synchronous model invokes the generator once, preparing its ordinary arguments once. Each `yield(...)` invokes the loop body with a freshly prepared payload. Normal body completion resumes that yield. The body uses the loop site's lexical bindings directly; ordinary callable value-capture snapshots are not a substitute for that access.

| Direction | Contract |
| --- | --- |
| Caller → generator | Ordinary explicit inputs, parameter initialization and effects. |
| Generator → body | Single yielded component or labelled component contracts, received through the header. |
| Body → yielding generator | Proposed normal body-expression feedback; its type is separate from final generator completion. |
| Generator → caller | Void completion after required cleanup. |
| Explicit body `return` | Completion of the lexically surrounding callable, not feedback to yield. |

Describing the body as a typed anonymous single method does not make it an ordinary source callable scope or an escaping callable value. The generator may invoke it only while the enclosing loop invocation is active under the proposed synchronous boundary. Retention, deferred invocation after that boundary, or implicit boxing of outer variables requires a different design and is excluded here.

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

The exact declaration spelling for the fixed void completion remains part of signature design. A yield/feedback annotation must not overload the meaning of an ordinary final return annotation silently.

## Yield payload and normal feedback

Payload preparation follows the established component modes: a val snapshot retains evaluated bits; a borrowed component retains its selected live target and access. Labelled components are contracts, not a stored structural object. No owning output may leave a surviving borrower without an independently proved dependency/lifetime contract.

Yield does **not** exit the generator. Its final defers therefore do not run before every body invocation. The generator activation remains live through normal body execution. A reference to generator-local storage might consequently be lendable during that invocation even though it would be invalid as the generator's final borrowed return; the owner quantification and escape proof for that extension are still pending, not automatic permissions.

The feedback proposal is the normal value of the body expression, never an explicit `return feedback;`. A schematic body can end in a bool expression:

```cod
# Assuming a generator with scalar yields and bool feedback:
for val n <- numbers() {
    consume(n);
    n < 100u64
}
```

Here the comparison would be the yield expression's result. False does not implicitly break the loop; generator code decides how to use its feedback. Ordinary explicit returns and throws provide no normal feedback value. This proposes no implicit value return for ordinary callable definitions.

Normal feedback must remain valid after any explicit body-local full-scope cleanup and iteration resource/effect restoration required before generator resumption. A reference to released iteration storage cannot escape as feedback. Multi-field feedback, trailing-expression grammar, missing-feedback paths and category-specific preparation remain open; reuse of receiving specs inside generator code is a possible integration, not an approved general record type.

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

If the new form supports `else`, retain the existing zero-body-executions meaning rather than interpreting it as successful exhaustion; header controls are unavailable there. No break/continue syntax or non-void feedback default is selected here. Nested generator composition must resolve yield to its lexical generator definition, not whichever generator happens to execute most recently; the exact nesting contract remains part of signature/control review.

## Signature and proof work remaining

The source needs to distinguish ordinary generator inputs, yielded component contracts and normal body feedback while fixing final completion to void. A signature clause is likely sufficient alongside existing func/proc declarations, but no keyword, callable type constructor or ABI is selected. Keep func/proc effect restrictions explicit; do not assume that arbitrary procedure effects become valid inside a function generator.

Yield contracts must be independently established under D-24's inference principles. Receiver spelling cannot select a different producer signature. Explicit body returns participate only in the surrounding callable's result checking. Feedback needs an independent declared/expected contract or sound body evidence; a circular dependency between yield and feedback types cannot prove itself. Ordinary actual-type dispatch, unique-owner/access rules, numeric conversions, UoM and D-16 validity all continue.

The next substantive boundaries are:

1. Repeated destination timing and header-control storage/initialization lifetime, especially new ref materialization, const controls, generator-local borrows and zero yields.
2. Declaration/yield/feedback grammar, type/owner quantification, independent inference and checked effect/exception composition. A body failure reaching yield must have a defined catch boundary; a nonlocal return is not a catchable exception.
3. Target-aware propagation of body return through generator cleanup, including generator-local returns or failures during that cleanup, nested iteration and the lexical meaning of `if returning`.
4. Nonescaping body-entry proof and transport, including direct/stored/generic generator invocation if admitted. Ordinary callable values or annotations alone do not prove the body's required lexical permissions.

Do not expand the design with custom final results, resumable iterators or a coroutine runtime to avoid those checks.

## Acceptance before implementation qualification

The following is a review checklist, not current passing-test evidence:

- Once-only invocation/argument preparation; zero, one and multiple yields; explicit labels/subsets/renaming; all scalar new/existing receiver combinations and invalid operators. No arbitrary spec expressions or forward receiver dependencies.
- Distinct val snapshots and live borrowed targets; fixed owners/access/init/null rules; destination retention across permitted backing growth; defined nonoverlapping reception and diagnostic-only checks for H-576 UB examples.
- Loop-control lifetime/const/materialization and omitted-output cleanup; no escaping generator-local, iteration-local or callable-bearing storage; complete cleanup on partial receiving or feedback failure.
- Normal feedback versus enclosing return; generator fallthrough/early void return; reject a generator-owned non-void return. Generator cleanup runs on exit, not at every yield.
- Direct lexical body access, ordinary body-scope restrictions, explicit full-scope cleanup, outer defer registration and iteration effect restoration. Nested loops and generator invocations preserve lexical targets without implicit closure cells.
- Checked/unchecked failures, normal feedback preparation failure and nonlocal completion across generator cleanup; retained return inspection/replacement and cleanup priorities must be settled before these paths can qualify.
- D-16 comments/numerics/representation/transfer invariants; reference interpretation and native observations agree with the selected source contract. No build/test or permanent native layout is prescribed by this draft.
