# Scalar call results: views, values and caller storage

Discussion revision: 2 · Recorded: 2026-11-12 · Status: pending-human topic review.

## Design authority

- **Registry and scope:** [D-23](DesignAuthorityRegistry.md#d-23), revision 2, all sections. This file has no human-confirmed whole-file baseline and is not an implementation contract. [K-556](DesignAuthorityRegistry.md#k-556) records the investigation and [K-558](DesignAuthorityRegistry.md#k-558) updates implementation evidence without qualifying this draft; final approval is requested for the complete file after topic discussion, following H-386.
- **Candidate value path:** P-354 organizes C-65's visible storage/lifetime, C-72's bits storage and C-54's resource/value distinctions. A call's aliasing must follow its checked contract, so source business behavior survives native staging and optimization under charter §§2–3/§5.3/§6/§8/§12. This document relationship remains pending-human.
- **Confirmed constraints:** H-201 makes ordinary unqualified results borrow and ordinary const bits results independent; H-272 changes default parameter targets, not that result rule. H-202/H-273 preserves live actual owners and access. H-313/H-319 permits lexical bits storage with explicit lifetime boundaries. H-361/H-365 requires explicit returns and remaining cleanup after replacement. H-342's independent complete callable values are a separate category.
- **Dependencies and exclusions:** P-355 depends on D-14 storage/access/initialization, D-17 completion, D-18 native calls, D-16 fixed bits/validity and D-22 construction. No regional object copying, hidden allocator, lifetime extension, runtime owner union, RTTI, fraction implementation or aggregate ABI is selected.
- **Acceptance:** Once the open topic is resolved, the same programs must preserve aliases, scalar snapshots, initialization, null handling, cleanup and escape rejection through named calls, callable values, recursion and deferred invocation. Existing prototype output is evidence of implementation behavior, not human approval.

## Existing storage and copied results

The established distinction is observable:

```text
proc borrow(x:u64 in o) -> u64 where {owner o} {
  return x;
}
proc copy(x:u64 in o) -> const u64 where {owner o} {
  return x;
}
```

A view returned by borrow keeps x's actual owner and observes later writes. The result of copy contains the evaluated scalar bits. A ref declaration can materialize an independent bits value into lexical storage; that does not make the original expression an existing external borrow. Default parameters still receive independent callee-local bits, so returning a view to their local storage cannot escape the call.

Cleanup preserves this distinction. If a returning body defers a write to x, a borrowed view observes the updated storage; an independent result preserves its evaluated scalar unless a later return replaces it. All remaining cleanup still executes. Const target access and const binding remain separate, and this topic does not redefine either qualifier.

## Open topic: fresh expressions in an ordinary result

The result-storage rule for this source needs explicit resolution:

```text
proc increment(x:u64 in o) -> u64 where {owner o} {
  return x + 1u64;
}
```

The arithmetic produces bits without an existing storage owner. The confirmed ordinary-result borrowing rule does not itself select storage for those fresh bits. Two coherent choices are available:

| Choice | Fresh expression | Existing storage | Consequences |
| --- | --- | --- | --- |
| Require an independent-value result annotation | Use `-> const u64` for this definition; a borrowed result must select live existing storage. | `-> u64` retains the alias. | The signature determines value versus view. A common copied result can be requested through an explicit const-result wrapper. No result-owner alternative is introduced. This may require migrating existing prototype definitions. |
| Permit caller-owned fresh result storage | Materialize fresh bits in a defined caller destination and return its view. | Existing storage retains its original owner. | The destination's lifetime, initialization, materialization timing and referenceability must be specified. A selected callable returning either external storage or fresh caller storage also needs an explicit actual-owner rule; the current unresolved owner-selection boundary cannot be bypassed. |

The first choice is the proposed direction for discussion because it makes the signature sufficient to distinguish borrowing from independent values and preserves the established unqualified-result rule. It is not activated by this draft. The second can provide concise source, but caller storage and mixed-owner behavior must be settled together before implementation.

Silently copying an existing borrowed result merely because another body computes a fresh value would change its aliasing. That behavior requires an explicit revision of the ordinary-result contract; it cannot be adopted as a lowering convenience.

## Prototype coverage and remaining boundary

[Native scalar evidence](../tests/cod/language/callable-native-scalar-validation.json) covers the independently confirmed borrowing/value contracts:

- Existing regional and lexical targets retain their aliases through named, captured, selected, recursive and open deferred calls. Writing a result updates its original field or target.
- Borrowed results observe cleanup writes; explicit const results preserve evaluation-time contents. Return replacement, current/captured contexts, readonly access and expired-owner rejection remain distinct.
- The compiler shares argument-relative return summaries across owning, borrowed and scalar queries. Independent value category is explicit; absence of a resolved owner cannot manufacture that category. Internal native scalar transport carries payload and address, with category proofs closed before emission and no runtime tag.
- Fresh-only ordinary results retain existing prototype behavior as an observation. Native mixed value/view categories or different actual owners report a result-staging gap. Neither behavior chooses the language policy discussed above.

[Earlier investigation](../tests/cod/language/callable-result-provenance-validation.json) preserves its compiler identity and historical observations. Implementation evidence does not approve this document or resolve fresh-result storage. Broader staging, actual-owner selection and aggregate representation remain separate work.

## Acceptance after the decision

1. Existing regional and lexical scalar results preserve their targets across direct, selected, captured/current and recursive calls; both recursive branch orders agree.
2. Independent scalar results retain evaluation-time contents across later writes and cleanup. A consuming ref declaration obtains the specified lexical storage.
3. The fresh-expression example and mixed selected bodies have the explicitly chosen result, diagnostic and owner behavior, regardless of native outlining or source expansion.
4. Nullable forwarding does not read or initialize its target. Reads, writes, borrowing and scope exits retain owner/access/initialization checks.
5. Deferred invocation preserves operand timing, pending return/exception replacement and remaining cleanup. Failed calls release only their owned resources.
6. No new regional allocation, object copying, lifetime extension or runtime type/owner mechanism appears without its own approved scope. Future bits ADTs and fractions retain their separate layout and validity requirements.

Discussion of the fresh-expression choice precedes final review and approval of this complete file. The existing borrowed-result requirement and unrelated qualified development remain active.
