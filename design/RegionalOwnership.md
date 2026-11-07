# CoD regional ownership: syntax, semantics and use

Design revision: 2 · Recorded: 2026-11-07 · Status: design draft.

## Design authority

- **Implemented callable/resource subset:** [H-188/H-189](DesignAuthorityRegistry.md#h-188) authorizes implementation/validation and selects current dynamic bindings for direct lexical procedure calls. [K-305](DesignAuthorityRegistry.md#k-305) records typed/grouped symbols, `proc(T, ...) -> RT` signatures, lexical procedures, combined symbolic declarations, captured-value-preserving installation and rejection of ownership extraction from dynamic resources. [Native evidence](../tests/cod/language/typed-procedure-validation.json) covers this subset. General optional sums, ordinary-owner syntax and packet transfer remain separate design work; full-contract qualification is unchanged.

- **Registry / scope:** [D-14](DesignAuthorityRegistry.md#d-14), revision2, all sections. This is a `pending-human` ownership-language proposal, not an implementation contract. [K-302](DesignAuthorityRegistry.md#k-302) records the review and dependencies.
- **Human baseline:** H-177 permits ordinary variables to hold regions; H-179 requires relinquishment when installing a dynamic resource. H-180 preserves definition-time value capture and owner-preserving perform shadows, separately from packet transport. H-181 prohibits extracting ownership from dynamic resource variables and distinguishes WPO binding availability from business optionality. [H-182](DesignAuthorityRegistry.md#h-182) proposes fresh construction or a writable optional reference as the resource initializer source. [H-183](DesignAuthorityRegistry.md#h-183) requires ownership to be designed coherently across syntax, semantics and practical use before treating “ordinary owner” as a defined language construct. [H-184](DesignAuthorityRegistry.md#h-184) permits lexical and symbolic procedures, assignment of an existing lexical procedure to an effect, and requires explicit types on every symbol declaration. H-185 restricts perform/behave to symbolic dynamic calls and direct procedure calls to lexical lookup.
- **Candidate value lineage:** P-155 connects D-14 to C-10’s explicit language semantics: readable source should expose resource lifetime/transfer obligations rather than inherit host-language move behavior. P-154 proposes an expression-source constraint serving C-54/P-143’s checkable ownership and C-41/P-97’s typed composition. P-156/P-157/P-158 depend respectively on D-10, D-13 and D-01. Dependencies do not grant design authority. Charter §§1–4/6 distinguish programs, execution context, storage lifetime and access permission.
- **Invariants / non-goals:** no copy/move of regional objects; no hidden user heap lifetime; preserve resource owners during shadowing; no implicit Maybe on dynamic variables; no taking ownership out of a dynamic resource. Packet send/receive is a separate scenario. No new pointer format, runtime ownership table, generalized borrowing framework or reference-counting mechanism is selected.
- **Acceptance:** a future qualified increment must state the source syntax, ownership transition, missing-value behavior and cleanup responsibility for each supported form, with native evidence for valid construction/installation/shadowing and rejection of duplicated or extracted ownership. This draft and the existing compiler are not that evidence.

## Agreed behavior and undefined vocabulary

The current constraints are stronger than the source-level ownership vocabulary:

- A region is a lifetime unit for a graph; individual regional objects do not copy/move or reclaim storage independently.
- An ordinary variable may hold a region, but its declaration form, writable-place rules, optional representation and consuming operations are not yet a complete CoD contract.
- Installing such a region as an effect resource consumes the source’s ownership. The resource cannot subsequently be taken out of the dynamic binding.
- A procedure captures used dynamic values. Perform temporarily shadows lookup with those values; it does not consume, replace or destroy either the captured or hidden resource owner. Behave uses current values.
- WPO rejects an unavailable required binding. An explicitly absent optional value is different from an unavailable binding.

Here **ownership responsibility** means responsibility for retaining, releasing or transferring a region. It does not prescribe a source type, pointer representation or runtime object. “Ordinary owner” alone cannot answer whether a declaration is mutable, how transfer is written, what remains afterwards, or whether a reference can escape. Those are the design questions below. The prototype’s `holding_id` and owned/empty/conditional facts do not settle them.

## Types, expression categories and storage permissions

The following notation is for analysis, not proposed runtime wrapper types:

| Notion | Meaning |
| --- | --- |
| `T` | The full declared value type of a binding. |
| `R` | A typed whole-region resource. It is distinct from a regional object type allocated inside another region. |
| `Opt(T)` | Semantic optionality, independent of whichever CoD spelling is selected. `Maybe<T>` is an example, not the required type constructor. |
| Bits value | An independent copyable value. Physically pointer-sized bits are not enough to classify a resource or regional reference as bits. |
| Transferable temporary | A transient expression result with one available region-lifetime responsibility that has not been installed in a dynamic binding. Precisely which results qualify remains to be chosen. |
| Writable optional place | A storage location whose contained resource may be removed and whose value can be replaced with its defined absent alternative. This is a candidate permission, not merely a pointer to mutable bytes. |
| Resource access | Permission to read or edit the region’s contents under the existing owner’s lifetime, without taking ownership. |

A resource’s type alone therefore cannot decide whether an expression may initialize an owning dynamic binding. A dynamic `@y :: R` provides access but no consuming capability. A dynamic `@y :: Opt(R)` likewise cannot have its optional storage emptied by extraction. Procedure access to the region’s contents can still be mutable: the optional slot and the graph it refers to have different permissions.

Aliases, casts, wrappers and pure functions cannot upgrade access to consuming permission. Constructing a fresh wrapper around a reference is not construction of a fresh region or a new owner. Captures retain their existing lifetime dependencies; they do not acquire an independent destruction responsibility.

## Candidate rule for effect-resource initialization

H-182’s example is:

```text
effect @x :: R <- rhs
```

The annotation syntax itself is proposed; the current parser only accepts the unannotated effect form. The following is a candidate contract specifically for new resource installation. It does not redefine packet send, every ordinary assignment, or implicit perform replay.

For a bits-valued binding, evaluate the RHS as a value and bind its checked result. There is no region-ownership extraction. For a direct regional resource, the proposed source alternatives are:

1. A newly constructed transferable resource temporary of the required type.
2. A writable optional source place that owns a present resource of the required type and supports replacement by absence.

The second condition is stronger than “writable reference to an optional type.” The source must actually hold the resource, the contained reference must carry its lifetime responsibility, and the current path must establish presence or explicitly handle absence. An unknown runtime optional value does not become a direct `R` merely because the reference is writable. Requiring a present match arm is one candidate; a checked extraction with explicit failure is another. No implicit trap, default region or universal automatic unwrap is selected.

On successful installation from a present source place:

```text
before: source = Present(r); source is responsible for r
 after: source = Absent;     the new dynamic owning scope is responsible for r
```

The old binding of `@x`, if any, is only shadowed and retains its owner. Source clearing and destination acquisition form one semantic transfer with one remaining cleanup responsibility. This does not prescribe a multithreaded atomic primitive or runtime transaction structure.

| RHS situation | Candidate result for a direct `R` target |
| --- | --- |
| Verified fresh whole-region construction result | Transfer its temporary lifetime responsibility into the effect. |
| Writable optional place holding an owned `R`, with presence established | Remove the resource, leave absence and install it. |
| Writable optional place whose presence is unknown | Need an explicit present path or defined checked-failure behavior. |
| Read-only optional place | Reject as a consuming source. |
| Dynamic `@y :: R` | Reject: access does not supply a transferable temporary. |
| Dynamic `@y :: Opt(R)` | Reject: the symbolic slot is not writable/extractable. |
| A call, cast, projection or fresh wrapper returning a reference to an existing dynamic resource | Reject: expression shape does not create ownership. |
| A named ordinary, non-optional `R` variable | Unresolved. H-183 requires a coherent declaration/transfer/post-state contract before assigning this category behavior. |

An owning constructor result and a writable optional place need to be compiler-distinguishable from an ordinary value read. Whether the effect initializer recognizes the place contextually, or source syntax explicitly requests its consuming use, remains a syntax decision. A pure conversion function cannot secretly perform this extraction; CoD’s existing pure UoM conversion semantics are not a suitable implementation shortcut.

### Perform is not an ownership-source initializer

Perform already has a different contract. It uses a procedure’s saved values to shadow the corresponding identities without creating new region owners. It does not need its captured resource to satisfy the fresh/writable-optional source alternatives. Applying the public new-resource initializer rule to replay would wrongly reject capture or consume ownership. Lowering can share ordinary lookup machinery only while preserving that semantic difference.

## Optionality must be specified by meaning

Any selected optional spelling, nullable-type shorthand, sum form, presence test, pattern, unwrap and optional projection must preserve the same resource permissions. Examples such as `Maybe<R>`, `R?` or an explicit absent/present sum are possible notations to evaluate; listing them does not admit them all into CoD grammar.

The source and target optionalities also differ:

- For a direct `R` target, a source optional must provide its present resource. Absence cannot establish the required value.
- For an explicitly optional `Opt(R)` target, an explicitly constructed absent value is valid and owns no region. A present initializer must still obtain its resource from a legitimate source. H-181’s optional domain modeling must not be excluded by a literal “every non-bits type is a bare R” formulation.
- Optional bits values can be classified by the ordinary semantic bits rules; an optional resource does not become bits because its representation fits in a scalar.
- Nested optionality need not be flattened: outer absence and a present inner absence can have different business meaning. A consuming source path must identify which place and which resource it selects.
- An arbitrary resource-bearing sum is not automatically an optional owner with a uniquely defined empty value. If it has several resource-free alternatives or requires data in those alternatives, explicit case handling must determine the post-transfer value; the compiler must not guess it.

A read-only pattern or projection from `@y :: Opt(R)` cannot yield a writable owning source place. It may permit access to the selected resource under the dynamic binding’s lifetime. Shadowing a present dynamic value with explicit absence preserves the hidden owner and restores that value at scope exit. These checks apply to semantic operations, not just to a type named Maybe.

## Procedure-valued effects retain their own contract

The bare rule “bits or region only” does not describe all effect bindings. H-184 explicitly permits both ordinary lexical procedure definitions and symbolic dynamic procedure definitions, and installing an already defined lexical procedure as an effect. These carry lexical meaning and captured dynamic values. They cannot be declared independent bits merely because machine code has an address, nor assumed to be freshly allocated regions.

For this draft, the fresh/optional-source rule applies to resource-valued data effects. Procedure declarations, using/import and procedure-value bindings retain their own callable rules while their representation is designed. A unified type/category account may cover them later, but this draft does not silently remove them or invent a runtime closure allocation.

### Typed symbols and procedure installation

H-184 gives the symbol-declaration form `symbol a :: T1, b :: T2`; every symbol definition must declare its type, including procedure types. A symbol declaration introduces a compile-time identity/alias and its value type, not a runtime lexical variable or an initial value. Reads/writes remain `@a` and `effect @a <- ...`. A symbol selected from another vocabulary retains that identity’s declared type.

An ordinary named procedure enters lexical scope. A symbol-named procedure supplies a dynamic binding. Naming does not determine purity: both are procedures, may use effects and retain H-180 definition-time captures. Assigning an existing lexical procedure to a symbol selects that existing procedure value; it does not redefine the body or recapture current values. This is different from procedure `import`, which reinstantiates a definition.

The implemented callable forms are:

```text
proc step(value::i64) -> i64 { value }
symbol step_effect :: proc(i64) -> i64;
effect @step_effect <- step;
proc @direct(value::i64) -> i64 { value }
```

`step(...)` is a lexical call using current dynamic bindings under H-189. `perform @step_effect(...)` restores the existing procedure's captures; `behave @step_effect(...)` uses current bindings. H-186/H-187's `proc @direct` also declares its typed symbol if absent. Neither `perform step(...)` nor `behave step(...)` is admitted; pure functions cannot call either procedure form. Callable types and this syntax do not settle whole-region owning-result types or optional extraction.

This gives the ownership design a concrete boundary: procedure-valued effects are admitted separately from bits and region-resource data. A lexical procedure used as a factory still needs a defined owning-result contract; a lexical name alone does not grant freshness, purity or consuming authority. No arbitrary lexical capture lifetime or hidden heap closure is introduced.

## Whole-region construction needs a separate contract

A “newly constructed R value” must not mean an ordinary rvalue expression, a new view of existing storage, or a stack-allocated regional object. The construction must establish a region’s storage/lifetime responsibility and initialize its contents in regional storage. Returning an owning handle does not copy/move the graph.

Two notions of “fresh” need comparison:

| Interpretation | Consequence |
| --- | --- |
| Newly allocated region instance | Direct construction and a factory proven to allocate a new region can supply it. Merely forwarding an existing resource cannot. |
| Exclusively transferable temporary result | Also permits a result whose responsibility has been transferred out of a legitimate source. It needs a precise result/cleanup contract, not just proof that the syntax is a call. |

H-182 proposes the first wording; the second is an agent alternative, not a confirmed generalization. H-183 leaves the overall ownership language open. A factory or block result must not be deemed fresh merely because it has type R.

### Syntax candidates

| Form to explore | Semantic requirement | Practical trade-off |
| --- | --- | --- |
| Explicit resource expression, illustratively `region R(...) { ... }` | Create and initialize one whole region; yield a checked owning temporary. | Makes effectful construction visible at the use site, but requires syntax for construction inputs, internal access and root completion. |
| Procedure factory, illustratively `behave @make_R(...)` | Its checked result must satisfy the same temporary/ownership contract. | Reuses named, testable procedural construction and composition; owning return semantics must be defined first. |
| Contextual optional source, illustratively `effect @x :: R <- source_place` | The initializer consumes a writable present optional place rather than reading/copying its value. | Concise and consistent with the transfer arrow, but diagnostics and source typing must expose the consumption. |
| Explicit extraction expression before effect installation | Extraction produces a valid transferable result with a specified absent-source behavior. | Makes removal visible outside the effect form, but introduces another operation and broadens the temporary-result contract. No such operation is selected without a concrete need. |

These are alternatives for discussion, not implemented grammar. A declaration of writable local optional storage is also missing: existing `let` and current parser behavior do not select its syntax or mutation contract. The design must settle that declaration together with references to its storage, presence tests and installation; a standalone ownership keyword would not resolve the rest.

### Required construction semantics

1. **Effectful creation:** a procedure or intrinsic performs regional allocation/initialization. Pure functions may calculate input values; a pure factory cannot hide allocation or ownership transfer.
2. **Construction access:** the body needs access to the region while building it. It must not obtain that access by irreversibly installing the same owner into an ordinary user dynamic resource and later moving it back out; H-181 forbids that. The construction-local access syntax still needs design.
3. **Final regional placement:** create root and nested objects in their region, preserving sharing, cycles and self-relative references. No temporary regional object is copied from the stack into its final position.
4. **Completion:** decide what makes a construction result usable, including its typed root and required initialized fields. This draft selects neither a graph-wide validator nor an auxiliary runtime structure.
5. **Transfer and cleanup:** completion yields one responsibility. Successful effect installation takes it; abandonment or a supported construction-failure path must have a defined cleanup responsibility. How source failure/control flow is expressed remains a separate language choice.
6. **Scope order:** RHS evaluation precedes installation. A failed construction must not be treated as a successful new binding or as destruction of the previously visible resource. Owning-scope exit and removal of captured shadows remain distinct.
7. **Composition:** factories, branches and block results must preserve the source category on every reachable path. A branch returning a fresh region and another returning a dynamic reference cannot be unified into transferable ownership merely because both are spelled R.

## Practical acceptance and the next coherent slice

Three use cases keep the draft grounded:

- **Local effect resource:** construct it, install it, capture its value in a procedure, call that procedure under another resource binding, restore lookup, then release each region at its actual owning scope.
- **Resource prepared before installation:** hold it in explicitly writable optional storage, establish presence, install it and observe an explicit absent source afterwards. This use requires the local declaration/reference/pattern syntax to be designed together.
- **Regional packet:** create, edit, send or receive it under its own transfer protocol. This draft does not impose effect installation or its optional-source rule on that separate scenario.

The next useful design slice is the smallest complete set of forms for the first two cases: a resource type, effectful construction, temporary result, writable optional declaration/reference, present/absent handling, installation and scope exit. It should also demonstrate rejection of dynamic-source extraction and wrapper laundering. No implementation should fill gaps by inheriting C++ move semantics, treating every call as fresh, or inventing implicit moved-out values for an undefined ordinary-variable category.

The current prototype supplies regional allocation, scoped effects, capture/replay, pure snapshots and WPO binding checks. It does not supply this ownership-source grammar, typed effect annotations, writable optional locals, generic optional forms or owning factory results. Its old consuming effect-to-effect moves conflict with H-181. No code changes or tests implement or validate this draft.
