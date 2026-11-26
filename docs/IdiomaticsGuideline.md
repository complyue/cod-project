# Idiomatics guideline

Guideline revision: 5 · Updated: 2026-11-25

## Design authority

[D-DOC-IDIOMATICS](../design/DesignAuthorityRegistry.md#d-doc-idiomatics) covers this guideline at
r5 under the [grouped method decision](../CHANGELOG.md#h-grouped-method-dispatch-20261125), including the [normal-completion formatting rule](../CHANGELOG.md#h-normal-completion-20261124) and applying the [single-directive procedure import](../CHANGELOG.md#h-proc-import-identity-20261124) to the contributor example. [D-10-FORMAT-POLICY](../design/DesignAuthorityRegistry.md#d-10-format-policy) covers its
[canonical code style](#canonical-code-style). The project user's
[quality-standard approval and formatting decisions](../CHANGELOG.md#h-idiomatics-guideline-20261123)
establish both scopes, including the initial maximum of 100 source columns.

The value is readable business intent and useful programming judgment, following
[the charter's source-readability goals](../design/RegionalSemanticPlatformCharter.md#8-文本源码快速可读可往返的语义投影).
The guideline depends on the registry's scoped design authority and the applicable language contracts.
It preserves their identities, permissions, lifetimes, effects and observable control. It introduces no
additional language mechanism or unconfirmed formatting detail. Acceptance requires the quality,
example, coverage and style checks below. The project user's [symbolic procedure guidance](../CHANGELOG.md#h-symbolic-selector-guideline-20261123)
applies the authoring standard to module examples. Review the consumer, installer and extension
roles against D-15-DEFAULT-EXPORTS and D-10-HANDLER-INSTALL.

## Purpose and reader

This document governs the preparation and review of [Idiomatic CoD](IdiomaticCoD.md). That guide is
for readers who already have programming experience and want to make sound choices in CoD. It should
help them express business intent through the language's types, storage, ownership, calls, dynamic
context and control flow, and explain why a particular choice fits a particular problem.

A reader should be able to apply the reasoning to a different application. The central question is:
"Why should I write this way here, and which changed assumptions would justify another choice?"

Write the guide in English. Organize it around programming decisions and their conceptual
dependencies. Give core language relationships appropriate prominence; keep local formatting rules
concise and easy to find. Their smaller explanatory footprint does not reduce their mandatory status.

## Confirmed design is the sole language baseline

1. Trace every normative semantic statement to the current confirmed decision and its exact scope.
   Use the [authority registry](../design/DesignAuthorityRegistry.md#1-audit-and-reading-rules) to
   establish qualification and supersession, and the corresponding design section for the contract.
   Resolve genuine evidence gaps against the recorded human decision.
2. Adopt confirmed source behavior directly. Implementation progress, compiler coverage, engineering
   schedules and unfinished representation or proof work do not change the guide's language or
   examples. Keep implementation-status commentary and temporary implementation workarounds out of
   the guide.
3. Apply scope precisely. Whole-document review status does not cancel independently confirmed
   rules. Approval of one rule does not approve adjacent proposals. A surviving old example is not
   authority to revive a superseded rule.
4. Resolve discrepancies already settled by recorded decisions without reopening those decisions.
   List genuinely undecided source rules, conflicting current decisions and new semantic commitments
   separately for discussion with the project user before writing dependent guidance.

## Quality requirements

### 1. Organize around a programming decision

Each substantive section should address a recognizable task or choice: representing state, choosing
a call contract, preserving a resource lifetime, handling absence, or selecting a control boundary.
The reader must learn how to choose a form, beyond recognizing its syntax.

### 2. Derive recommendations from CoD's model

Explain which identity, access, owner, lifetime, binding or control relationship motivates the idiom.
Connect that relationship to the application's intent. Generic advice such as "keep code clean" is
insufficient without a concrete CoD reason and consequence.

### 3. State the kind of requirement

Distinguish required language semantics, mandatory canonical code style and recommendations whose
correct application depends on the task. State the relevant obligation or trade-off. Both semantic
and formatting violations fail review; contextual recommendations need their stated conditions.
Do not turn an editor's preference into a language rule or an unconfirmed formatting requirement.

### 4. Explain applicability and boundaries

For each major idiom, establish the problem, the preferred choice, its reason and the assumptions
that could change that choice. Qualify broad titles as carefully as their accompanying prose.
"Always", "never" and "prefer" require an adequate basis. Do not manufacture a counterexample or an
exception merely to fill a fixed presentation template.

### 5. Use minimal, sufficient examples

Supply the declarations and preconditions needed to understand the example. Introduce only the
concepts needed for its argument. A local code fragment is acceptable when its context is explicit.
An omitted body, unexplained helper or assumed owner must not hide the fact on which the conclusion
depends. Business names should make the operation understandable without knowledge of compiler
internals or a large surrounding application.

### 6. Compare the same problem

When contrasting forms, keep the business goal and relevant inputs and preconditions consistent.
Make the important difference visible. If a recommended change also changes observable behavior,
identify that change and justify it. Formatting comparisons must preserve program meaning.

### 7. Explain interactions that affect the choice

Cover the combinations that determine whether an idiom is appropriate, such as owner allocation
permission with writable target access, a supplied `here` body's lexical permissions, full scopes
with return and cleanup, and module selection with identity. Ensure the recommended forms compose
correctly. Explain the interactions needed for the decision without reproducing an entire language
specification.

### 8. Make behavior derivable

Where binding, rebinding, dynamic context, argument preparation, exit or cleanup matters, state the
observable result or guarantee. Readers should be able to identify the selected target, evaluation
order, control destination or release responsibility. Assertions that a form is "safer" or
"clearer" need a concrete explanation.

### 9. Use a consistent conceptual vocabulary

Distinguish bits transfer, reference binding, target writes, declared place aliasing, runtime target
aliasing, name/identity aliasing, ownership transfer and graph transfer. Use the
[qualified operation and alias terminology](../design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer)
throughout. Keep names, formatting and abstraction levels coherent across examples. Similar-looking
syntax must not erase different contracts.

### 10. Maintain a readable explanatory progression

Introduce concepts when the reader needs them. Use a small set of understandable example contexts
where continuity helps, while allowing each section's decision to be found independently. Link to
precise design sections for detailed rules. Keep decision histories, authority bookkeeping and
unresolved proposals out of the guide's explanatory mainline. Allocate space according to the
importance and difficulty of the programming choice.

## Example acceptance

Every positive example must be valid as a whole under confirmed design. Review all applicable
syntax, types, permissions, actual owners, initialization, nullability, evaluation order, control
destinations, checked failure obligations and cleanup responsibilities. Examining only the feature
named by the section is insufficient. Expected observations must follow from those contracts.

Classify each contrasting or negative example explicitly:

- **Legal but unsuitable here:** it adds unnecessary coupling or reading burden under the stated
  requirements. Explain the cost and the circumstances in which it could be appropriate.
- **Legal with different behavior:** identify the difference and why it does not express this intent.
- **Rejected by the language:** identify the violated rule and the relevant prerequisite.
- **Nonconforming code style:** identify the canonical formatting rule it violates. Use this only
  when the formatting difference is itself the subject of the example.

Positive examples must meet the canonical code style. Other examples must also follow it except for
the explicitly identified violation being demonstrated. Do not use unsupported claims of execution
as evidence; this guide's acceptance rests on the confirmed source contracts and accurate reasoning.

## Canonical code style

**Code style is a hard acceptance requirement. The initial maximum CoD source line width is
100 columns.** Apply this limit to the guide's CoD examples as well as the source conventions it
teaches. A violation is not excused as a personal formatting preference. "Initial" records a
confirmed baseline that may be changed by a subsequent design decision; it is not an optional limit.

CoD's formatting policy is uncompromising. The plan is an **almost configuration-free formatter**
that produces a common canonical form. Recommend automatic formatting in
commit hooks and independent formatting checks as submission or integration gates. These are the
chosen tooling direction and recommended workflow; this document does not install them or specify a
command, configuration schema or CI provider.

Preserve meaning while formatting. In particular, retain necessary separators and callback
attachment, and preserve the distinction between grouping and a full scope. Follow the confirmed
[semicolon and whitespace rules](../design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux).
Canonical layout cannot replace an author's semantic choices about ownership, callable kind,
dynamic context or control flow.

The planned formatter must preserve the extent of every method and its callbacks when it wraps
a group at 100 columns. This boundary has no formatting option. Check the following two inputs
as distinct programs (assume `read_fee` accepts either a plain call or a trailing `here` callback).

```cod
proc rate(n:u64) -> u64 of {
    (0u64) => read_fee();
    (x:u64) => { x + 1u64 }
}
```

```cod
proc rate(n:u64) -> u64 of {
    (_) => { read_fee() (x:u64) => { x + 1u64 } }
}
```

Format and reparse each program. The first must still have two methods. The second must still
have one method containing one callback with the same control destinations. Preserve a required
`;` at a bare expression boundary, the outer method braces around a direct-level parameterized
trailing callback, and the callback's own single or double braces. A second format must give the
same text. Removing the first program's separator leaves a rejected boundary. Diagnose it and
let the author choose whether to end the method or enclose the entire callback-bearing body.
[Method arm boundaries](../design/SymbolicDynamicScope.md#method-arm-boundaries)

A formatter may remove a trailing empty return when fallthrough completes the same invocation
with the same result, scope exits, cleanup order and observable completion at every deferred
entry. An `if returning ()` in that invocation’s defer observes both normal exits identically.
Preserve early exits, value returns and returns from a single-brace supplied callback to its
lexical caller. In a func or full-scope `{{ ... }}`, also check the tail that deletion would expose:

```cod
func set_count(ref n:u64 in O) where {owner O} {
    n <- 1u64;
    return
}
```

Here the function infers void. Deletion would expose the transfer’s readonly destination view
to result inference. Keep the return. The same check applies to `{{ n <- 1u64; return }}`.
A semicolon alone does not discard a value tail.

Use all other confirmed style rules consistently. Bring unconfirmed formatting details to design
discussion when needed; do not invent extra options or exceptions to the 100-column limit. This
decision concerns CoD source and does not impose a line-width gate on Markdown prose or links.

## Coverage review

Review the following decisions across the guide. This is a coverage checklist; chapters may combine
or separate topics to support a coherent reading order. Only their confirmed scope may supply
normative guidance or examples.

| Decision area | What the reader should be able to decide |
| --- | --- |
| Domain modeling | Use domain types, sums, enums, constructors and cases to make state, payloads and operations explicit. |
| Values, storage and ownership | Choose independent bits, a borrowed target, construction or ownership transfer, with the required access and lifetime visible. |
| Computation and caller-supplied work | Choose `func`, `proc`, `here` and suitable data representations according to dependencies and lifetime. |
| Dynamic context and adaptation | Choose lexical calls, symbol identities, installation, `behave` and `perform`, and derive their scoped behavior. |
| Results and control | Choose guards, cases, result reception, explicit discard, grouping and full scopes with the intended return destination. |
| Resources and failures | Distinguish ordinary absence, domain outcomes and exceptions; identify acquisition, cleanup and failure responsibilities. |
| Traversal and composition | Express observation, feedback, early completion and synchronous forwarding using confirmed generator, here and composition contracts. |
| Modules and public interfaces | Choose publication, reuse and extension operations while preserving identity, meaningful dependency groups and definition documentation. |
| Source presentation | Apply the 100-column limit and confirmed separator, layout, naming and comment rules consistently. |

### Selecting a symbolic procedure family

Discourage routine `using proc` and `import proc` in application examples. Show a named callable for
a fixed calculation. A billing helper that follows the active tariff needs only the vocabulary
identity (`symbol from`) and a dynamic call (`behave` or `perform`).

A family selector needs a concrete reason. An application entry may use `using proc` to select a
carrier's complete tariff and install it for a billing run. A package adding a refrigerated-shipment
method to that operation can select its identity and gain extension permission in one
`import proc` directive. An explicit `symbol from` is optional for that same identity. Explain who
selects, who installs and who calls. A `using` selection grants no extension
permission. A lexical call to the selected family bypasses a dynamic replacement for that callee.
[Guide example](IdiomaticCoD.md#keep-symbolic-procedure-families-at-integration-boundaries) ·
[Module contract](../design/ModuleWorlds.md#selecting-a-symbolic-procedure-family)

## Handling undecided design

Keep a separate discussion list for choices needed by prospective guidance. Each item should state:

- the already confirmed behavior and its source;
- the exact unanswered source-language or API question;
- the viable alternatives and their effect on how application code reads and behaves;
- the decision and concrete examples needed to resolve it;
- the design documents and authority records that need updating after the decision.

Discuss that choice with the project user before writing guidance that depends on it. Keep
implementation progress out of this list. Treat text that contradicts an already settled decision as
documentation maintenance, with the settled contract as its correction target. Do not ask the user
to approve the same semantic rule again.

## Final review gate

Before accepting a revision of Idiomatic CoD, verify:

- [ ] Every normative claim has a current confirmed scope and a precise design reference.
- [ ] Major recommendations explain their rationale, assumptions and relevant alternatives.
- [ ] Examples are sufficient, jointly valid and classified accurately.
- [ ] Comparisons preserve their stated intent and disclose any behavioral change.
- [ ] Binding, ownership, effects, control and cleanup interactions are consistent throughout.
- [ ] CoD examples satisfy the 100-column maximum and every other confirmed style rule, apart from
  an explicitly identified style violation being taught.
- [ ] Coverage follows the important programming decisions and supports a coherent reading order.
- [ ] Terminology, links and the distinction between rules and conditional recommendations agree.
- [ ] Genuine open design choices have been discussed separately before dependent guidance is added.
- [ ] No implementation-status commentary or temporary implementation workaround defines an idiom.

Semantic accuracy, valid authority, correct examples and canonical style are mandatory. The central
measure of explanatory quality is whether readers can make and justify a choice in their own code.
