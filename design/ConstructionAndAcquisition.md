# CoD construction, bits ADTs and region acquisition

Design revision: 1 · Recorded: 2026-11-12 · Status: confirmed requirements; integrated contract and value paths pending-human; implementation incomplete.

## Design authority

- **Registry / coverage:** [D-22](DesignAuthorityRegistry.md#d-22), revision1. [H-383–H-385](DesignAuthorityRegistry.md#h-383) records the reviewed RP-CONSTRUCTION-2026-11-12 revision2 handoff. The requirements in the next section are confirmed propositions. P-348–P-350 are candidate value relationships; this document is not yet a qualified delegated implementation contract. [K-542](DesignAuthorityRegistry.md#k-542) records evidence reconciliation and affected scopes, not a new human approval.
- **Candidate value path:** C-39/C-65/C-72 → C-01/C-06 (P-348) makes storage, access and lifetime intent explicit and checkable in readable source; C-09/C-54 → C-05 (P-349) separates persistent data from acquisition responsibility and preserves inspectable preparation effects. D-22 organizes these responsibilities through P-350. Charter §§2–5/§7–8 governs those boundaries; §§6/12 requires practical native execution and measured costs. These scoped paths do not qualify the wider ownership or DBMR designs.
- **Dependencies:** P-351: D-01 physical allocation/relative references; D-14 owners, capabilities, fields and qualifiers; D-17 initialization, failure and cleanup; D-10/D-15/D-18/D-21 dynamic context, values and native dispatch; D-16 bits validity, layout and numeric compatibility. Dependency edges grant no authority.
- **Invariants / non-goals:** no ordinary reference ownership, per-object cleanup, non-bits lexical/general-heap allocation, hidden region, cross-owner stored edge, implicit graph copy, lifetime extension or native callable environment in a regional field. Regional C++ types remain region-only and noncopyable. Actual-owner unions, abstract-result RTTI, fraction execution, pure/TCO and the compiler pilot remain separate.
- **Human baseline:** current project user in [the construction side chat](codex://threads/01a1d051-f608-7c13-b98a-684f065884d7), recorded 2026-11-12 America/Los_Angeles; exact message timestamps/personal name unavailable. H-383 preserves the ownership/allocation corrections; H-384 confirms R1, explicit R2 owner syntax/mismatch rejection and autonomous R3/R4 comparison with final selection reserved; H-385 confirms result-only constness. Handoff approval does not select candidate inference, DBMR protocols, migration or ABI.
- **Acceptance:** the matrix below requires composed positive/rejection cases, exact acquisition/release accounting, relocation and measured compilation/runtime costs. No implementation or new performance result is claimed by this revision.

## Confirmed requirements

Only designated intrinsic resource-reference kinds distinguish owned and borrowed; the current kind is the region reference. Ordinary ref/ptr is borrowed, with target provenance, lifetime and access, without independent release or transfer authority. Storage ownership, reference-slot storage and the holder of region cleanup responsibility are distinct. User ADTs cannot contain intrinsic ownership carriers, including through wrappers.

Every ADT instance/storage view has an actual owner. Direct/val bits fields occupy that owner's storage; ref and non-null ptr fields target the same actual owner. Nominal type identity does not identify an owner instance. A bits copy across owners copies neither owner identity nor cleanup responsibility. Ordinary lexical borrows of outer storage do not become stored graph edges.

R1 admits a concrete ADT as bits when its finite fixed representation recursively consists only of bits fields/alternatives/tags, without stored references, ownership carriers, managed environments or implicit cleanup. This includes finite products, sums and qualifying generic instantiations. A one-word reference is still non-bits. An unresolved/abstract type requires sufficient bits evidence; R1 does not choose the source spelling for that constraint, a sum encoding, ABI, packing or validity of arbitrary bit patterns.

Bits ADTs may occupy lexical or regional storage, including a region root. Fresh lexical bits construction belongs to the current full scope; an ordinary grouping block adds no owner. Non-bits ADTs remain region-only with relative stored references. Adding a reference field invalidates dependent bits uses rather than selecting a hidden allocator. The current compiler's product-only/scalar-bits subset is an implementation limit, not the language definition.

The ordinary construction direction is `make T [in o]`; intrinsic resource acquisition uses `make region`. Explicit `in o` selects the actual owner and a receiving-variable owner mismatch is a compile error. Equal root types or lifetimes do not establish owner equality. Existing-reference initializers preserve provenance and access. An inferred `where {owner o}` binding supplies provenance, not implicit region allocation capability.

`make const region` restricts the published result view. Explicit preparation writes, including selected create/initialize/expand/eligible-trim operations, may precede that result subject to permissions and the selected protocol. Constness does not freeze the file, imply a snapshot or remove the owning acquisition's release obligation. Final DBMR grammar, sharing/defaults and operation/failure protocols remain unselected.

Callable payloads retain D-18's complete intrinsic value category and scope-owned backing; they are not bits ADTs or ordinary independently owned references. Local ADTs do not authorize lexical-cell captures or lifetime extension. H-370's deferred owning-argument transfer at invocation and H-365/H-371's scope restrictions remain unchanged.

## WP1: classification and representation obligations

The proposed compiler separation is nominal type identity → resolved representation facts → placement/view → native transport. Classification must not replace nominal identity, actual-owner facts or abstract dispatch evidence.

| Obligation | Proposed integration / unresolved boundary |
| --- | --- |
| Concrete product | Check each instantiated direct field recursively; reject a ref/ptr or other non-bits constituent. Cache by resolved type identity and arguments, not source alias or actual owner. |
| Finite sum | Check all payloads and tags for finite bits representation. Preserve active-alternative validity. Concrete source/tag/layout work remains required; no tagged-union ABI is selected here. |
| Generic or recursive type | Distinguish unresolved evidence from proved non-bits. Detect infinite by-value expansion; pointer recursion is non-bits rather than a recursion escape in the classifier. Source constraints and diagnostics need an explicit contract. |
| Storage and field access | Address lexical and regional payloads through a common place abstraction retaining owner, access and initialization facts. Relative stored references keep D-01 encoding; a temporary native address is not a persisted CoD edge. |
| Copy and borrow | A bits read/copy is independent payload; a ref/ptr view aliases its target. Whole-value copy requires initialized valid payload. Field writes/read proofs must distinguish individual fields from the complete object. |
| Layout and native calls | Separate aggregate payload layout from reference transport. Preserve existing regional offsets; choose local aggregate argument/result transport with recursive callable and AOT requirements visible. No ABI choice follows from adding a classifier predicate. |
| Numeric validity | Bits eligibility promises no zero-validity or numeric conversion. D-16 fractions retain fixed payload/nonzero denominator and representation-independent UoM; array SoA/layout policy remains separate. |

The implementation dependency order is representation facts, aggregate places/initialization, payload operations, construction, then parameter/result/capture/TLS integration. Each increment must name incomplete admitted forms rather than present them as forbidden language. Classifying bits ADTs is not permission to make C++ `regional_*` objects stack-copyable.

## WP2: owner-directed construction proposal

The proposed shared construction check carries the resolved target type, actual destination owner, allocation capability, ordered field initializers and result access. Source forms feed that check; ordinary storage construction and intrinsic acquisition have distinct cleanup responsibility. This is a compiler organization proposal, not a new runtime object or universal allocator interface.

| Source situation | Proposed result |
| --- | --- |
| Explicit `make T in r` | Use r only if it supplies the required placement/capability. Reject a conflicting receiver owner before runtime evaluation. H-384 confirms explicit selection and mismatch rejection; generic capability acquisition is not inferred. |
| Omitted owner, receiver has one unique owner and suitable allocation access | Construct there. A field may supply its containing owner to an explicitly written nested make. This contextual inference requires confirmation. |
| Omitted owner, no destination | Bits use the current full lexical scope. Non-bits diagnose the missing regional destination. No TLS fallback or hidden region. The bits baseline is confirmed; the integrated inference rule requires confirmation. |
| Receiver names provenance without allocator capability | Do not manufacture capability from identity. Request an explicit usable region path for non-bits construction. |
| Initializer is an existing reference | Check its actual owner/access; never promote, allocate or copy a graph to satisfy an expected owner. |
| Ref/ptr field initialized without explicit make | Bind an existing admissible target; no automatic target allocation. |

Illustrative candidate source, not current parser acceptance:

```text
ref p:Point = make Point { x <- 1; y <- 2 };
ref n:Node in r = make Node in r { ... };
ref bad:Node in r = make Node in s { ... }; # Reject distinct actual owners.
owned region arena:Root <- make region Root(capacity = bytes) { ... };
```

Construction must preserve initializer order and effects and publish only a valid initialized result. Existing D-17 failed acquisition cleanup does not promise rollback of effects or arbitrary reservations in an already-live region. Nested constructor naming/access, partial construction and allocator-policy interaction require review before implementing an uncovered case. Explicit owner spelling cannot bypass readonly allocation checks or same-owner stored edges.

## WP3 / R4: source migration comparison

Both alternatives use one semantic construction model. Neither chooses allocation by guessing from a receiving variable's `owned` qualifier or treats the same bare make as both object and resource construction.

| Criterion | M1: coordinated cutover | M2: bounded compatibility period |
| --- | --- | --- |
| Ordinary / resource form | `make T [in o]` / `make region Root(...)` exclusively after one migration checkpoint. | Same target forms; temporarily recognize `make local T` and legacy `make Root(capacity=...)` as explicitly marked legacy syntax. |
| Existing source | Update tracked fixtures, generated programs and current examples together; old forms get actionable diagnostics. | Old source compiles with migration warnings until a published removal checkpoint; ambiguity with the legacy capacity shape requires a temporary reserved form. |
| User mistakes | Diagnostic distinguishes object construction from region acquisition and suggests the exact replacement. | Warning must explain which semantic form the old source denotes; new ordinary constructors cannot silently acquire regions. |
| Technical debt | One maintained grammar/check path; one coordinated source update. | Additional parser branches, warnings, acceptance/rejection cases and a later removal; both lower to the same checks. |
| Runtime / generated code | No syntax-compatibility runtime cost expected. | No extra cost expected if compatibility resolves entirely during parsing; this is an estimate, not benchmark evidence. |
| Compile / maintenance cost | Smaller ongoing parser surface; migration review is concentrated. | Additional parse/diagnostic work expected to be small but unmeasured; two supported spellings enlarge documentation and validation until removal. |

**Recommendation: M1**, after owner inference and representation integration are qualified. The tracked prototype is amenable to a coordinated update; a permanent dual surface would obscure ordinary storage versus resource acquisition. M2 is credible if consumers outside the repository need a migration window. No external-consumer inventory or user selection is inferred. R4 selection does not approve incomplete aggregate ABI or DBMR protocols. Historical registry quotations/reports retain their original source; only current examples and live fixtures migrate.

## WP4 / R3: DBMR alternatives and protocol work

These are comparison candidates. Both preserve confirmed result-only constness and keep acquisition, live resize, compaction and snapshot publication distinct.

| Criterion | A: clauses on acquisition | B: typed acquisition request |
| --- | --- | --- |
| Illustrative surface | `make const region Root from existing path expand to bytes` | `make const region Root from existing_file(path, expand_to = bytes)` |
| Source intent | Source and capacity effects visible at the acquisition site; diagnostics can underline conflicting clauses. | Named request operations compose source/options; reusable helpers can describe acquisition without acquiring ownership themselves. |
| Ordinary use | Few concepts for one acquisition; modifier combinations need a finite grammar with explicit defaults. | Familiar call arguments and named options; users must distinguish request construction from acquisition and result access. |
| Complexity | Dedicated syntax lowered once into a checked plan. | Request typing, helper evaluation and admissible dynamic choices need a contract; a user-defined record must not contain an owning region handle. |
| Native/runtime cost | Static options can choose one native acquisition path. | Equivalent static options can select the same path; runtime-selected options would require checking/branches, without inherently requiring another mapping or heap object. |
| Failure diagnosis | Acquisition errors point at the source/capacity clause. | Must retain origins through helper-produced requests to diagnose the failed acquisition. |

**Provisional recommendation: A** for the initial explicit acquisition surface. It introduces less request-value machinery and keeps persistent-file effects visible. B is useful if real applications need reusable runtime-selected acquisition descriptions. No latency/code-size benchmark supports a performance preference yet; both can lower to the same acquisition operations. This is not a final R3 selection or frozen syntax.

The operation matrix below applies to either candidate and is still proposed. A source-form preference alone cannot qualify these file protocols.

| Situation | Proposed outcome / contract still needed |
| --- | --- |
| New source, destination exists | Reject without overwriting; creation must be exclusive rather than an existence-check/open race. |
| Existing source missing, invalid or incompatible | Reject; no silent create, rebuild or clear. Specify root/layout/reference validation and failure types. |
| Const result with requested preparation writes | Obtain authorized preparation access, perform specified writes, then publish the restricted result. Decide physical mapping protection independently of the already-confirmed language access rule. |
| Expansion/trim request already satisfied | Specify exact-size versus minimum-capacity semantics, total/payload/free-byte units, limits and idempotence; do not inherit the C++ reserve-free API accidentally. |
| Trim intersects occupied storage | Reject. Tail trim preserves retained offsets and is not compaction. Validate under the selected coordination boundary. |
| Partial acquisition/preparation failure | Release every acquired handle/mapping exactly once; specify remaining file contents/extent. File rollback is not supplied by region cleanup. |
| Shared mapping/concurrent resize | Define admissible access and coordination. Shared mapping supplies neither shared ownership, locking nor transactions; relative offsets alone do not make concurrent unmapping safe. |
| Release/flush failure | Define explicit observable failure and cleanup behavior; closing an acquisition does not delete data or guarantee durable publication. |

Before final R3 selection, compare a concrete capacity/failure/coordination contract and its costs against the existing primitives. Include a state-by-state failure table, access requirements and publication point. Existing snapshot save/replace (C-78) and compatibility-marker planning (C-77) remain separate inputs, not completed protocols.

## Repository evidence and implementation dependencies

Read-only audit baseline: `43260856432484ce0dbe9016a9d1a3e7b3315ad6`. Counts are regex occurrences in tracked text, including comments/string literals, not parsed-program counts or runtime measurements.

| File class | `make local` occurrences / files | Legacy `make Name(capacity=...)` occurrences / files |
| --- | --- | --- |
| `.cod` | 70 / 33 | 215 / 140 |
| `.py` | 2 / 1 | 31 / 6 |
| `.md` | 42 / 5 | 17 / 5 |
| `.hh` | 1 / 1 | 0 / 0 |

| Current source | Observed boundary and required integration |
| --- | --- |
| [Syntax](../cod/lang_syntax.hh), `Type`, make parsing | `Type::regional` represents nominal data products; `make_local` and `make_region` are separate AST cases. Legacy bare make is resource acquisition. Preserve source locations while normalizing the chosen forms. |
| [Frontend](../cod/lang_frontend.hh), `local_bits`, `object_type`, `local_storage` | Local bits are numeric representations, bool and intrinsic exceptions. Nominal products are object types. A predicate-only extension would leave call, owner and storage paths inconsistent. |
| Frontend `record_field`, `make_expression`, `allocate_expression` | Product field access requires a region pointer; root make allocates a whole region and checks every field; alloc uses an explicit region. Separate ordinary payload placement from acquisition while preserving field order and cleanup. |
| [LLVM emission](../cod/lang_llvm.hh), `type_of`, `read`, `write`, `field` | Regional products transport offsets; reads produce references and writes reject aggregate copies. Record layouts exist, but aggregate bits transport and lexical field places need explicit integration. |
| Frontend initialization/native/callable flow | Allocation cells, field reads, copies, native arguments/results, dynamic bindings and captures must agree on payload versus borrowed view. Defer completion facts need no new runtime initialization flag merely because payloads are aggregates. |
| [DBMR](../include/shilos/dbmr.hh), `create_or_open`, `create` | Creation opens existing paths (`OPEN_ALWAYS`/`O_CREAT`) and resizes them. It cannot implement proposed exclusive-new semantics unchanged. |
| DBMR validation and writable constructor | Checks header bounds, occupation, root extent/alignment and UUID. Insufficient free capacity performs unmap → resize → remap → metadata write/flush. This is evidence about existing operations, not full graph/schema validation or a failure-atomic protocol. |
| DBMR readonly/destructor/file helpers | Readonly maps without write access; optional close-time trimming writes capacity metadata. Flush/unmap/close return errors are ignored and truncate errors are printed. The API lacks the explicit access/failure contract required for a language acquisition surface. |

No DBMR mutation, compiler change, build or test is part of this audit. No new performance result or fresh sanitizer coverage is claimed. R3 measurements must identify compiler/source versions and distinguish counted operations from observed latency/resource totals.

## Acceptance and decisions before implementation

| Scope | Required evidence |
| --- | --- |
| Bits and owners | Local/regional nested products, finite sums and generic instances; copies independent across owners; aliases retain owner/access; reference/owner/callable-bearing types fail bits classification; invalid local escape and cross-owner edge reject. |
| Construction | Explicit owner agrees with receiver; equal root type never hides distinct owners; selected inference requires usable capability; existing borrows retain provenance; once-only ordered initialization, premature-read rejection and partial-failure cleanup. |
| Composition | Aggregate arguments/results through native recursion, callable captures/dynamic bindings, deferred execution and loops preserve value/view semantics and existing ownership transfer timing. |
| File acquisition | Selected source/access/capacity operation matrix, invalid image rejection, exact handle/mapping release, relocation preserving sharing/cycles and observable preparation effects before const publication. |
| Costs | Compile time, generated text/bitcode, stack/backing bytes, region/mapping count and peak extent; no accidental per-object cleanup or graph copy. No numeric target is assumed. |

Review items: **CA-Q1** confirms P-348 and the ordinary-construction slice of P-350 for the requirements/constraints/acceptance above; DBMR P-349 and its P-350 slice remain pending until protocol review. **CA-Q2** selects the WP2 omitted-owner/capability rule. **CA-Q3** selects M1 or M2 under R4. R3 remains an authorized exploration task until a concrete operation/failure/coordination proposal is ready for final selection. Aggregate layout/ABI and unsupported sum/generic mechanisms remain explicitly separate decisions where existing contracts do not determine them.
