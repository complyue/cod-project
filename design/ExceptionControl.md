# CoD exception control and regional cleanup

Design revision: 75 · Recorded: 2026-11-16 · Status: confirmed bounded exception, lifecycle, scope, defer and return contracts; current implementation availability is recorded in the source capability reference.

## Design authority

- **Disjoint declarations and exempt paths:** [H-527–H-531](../CHANGELOG.md#h-527)/[K-DISJOINT-PATHS-20261116](../CHANGELOG.md#k-disjoint-paths-20261116), [D-10-DISJOINT](DesignAuthorityRegistry.md#d-10-disjoint), records n-ary pair constraints, declaration-point exemptions, public/type-carried evidence and a frozen fused-world inheritance graph. Redundant shorter declarations accept with a warning identifying the existing path and the intermediate method contracts they do not bypass. Candidate C-56/C-10/P-476–P-478 under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12 preserves explicit nominal intent, stable interpretation and actionable diagnostics. D-10 owns examples/acceptance and remaining foreign-declaration authority, cyclic evidence and formal path-checking boundaries. Preserve owner/capture, selected contexts, H-525/H-526 checked contracts, demanded method checking and D-16; no new compiler support or full value-path/algorithm qualification is inferred.

- **Exception roots and declared contracts:** [H-524–H-526](../CHANGELOG.md#h-524)/[K-ERROR-ROOTS-20261116](../CHANGELOG.md#k-error-roots-20261116), [D-17-ERROR-ROOTS](DesignAuthorityRegistry.md#d-17-error-roots), records r74's scoped addition under candidate C-56/C-63/C-65/P-473–P-475 and charter §§2/3.1/3.3/4.3/6.3/8/12. Error <: Exception enables user unchecked exceptions; explicit Exception declarations/throws remain checked despite narrower proven values, and adapters cannot change an established checkedness contract. Preserve H-523 wrapper agreement, H-522 identity, owners/captures and D-16; acceptance is below. General disjoint syntax, arbitrary payload representation and broader throws/inspection policies remain pending. This replaces the prior neutral-root interpretation; existing concrete classifications and unrelated qualifications continue.

- **Checkedness-preserving wrapper construction:** [H-523](../CHANGELOG.md#h-523)/[K-RETHROW-FROM-20261116](../CHANGELOG.md#k-rethrow-from-20261116), [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), records r73→74: standard wrappers require a cause of the same checkedness; mismatches reject with a suggestion to use rethrow from e. The new explicit-cause shorthand selects the standard wrapper automatically, retaining H-522's local identity normalization and creation-time trace. Candidate P-464–P-466/P-470–P-472 under C-36/C-63/C-65 and charter §§2/3.3/4.3/6.3/8/12 preserves inspectable provenance without changing handling obligations. Acceptance covers both wrapper categories, missing/unknown causes, aliases, explicit/default causes, once-only effects and actionable diagnostics. Ordinary business exception translation, owners/captures and D-16 continue; inspection, transparent throws summaries, physical identity and source support remain separate.

- **Transparent wrappers and retained tip:** [H-518–H-522](../CHANGELOG.md#h-518)/[D-17-CATCH-WRAPPERS](DesignAuthorityRegistry.md#d-17-catch-wrappers), [K-WRAPPER-CATCH-20261116](../CHANGELOG.md#k-wrapper-catch-20261116), records wrapper-prefix matching in source order and the preference to preserve the incoming wrapper in exception operations, including from e. H-521/H-522 selects a hidden tip reference alongside the matched binding and recognizes the same exception identity within that catch, including unchanged helper results, without copying the chain. Candidate P-470–P-472 under charter §§2/3.2–3.3/4.3/6.3/8/12 preserves typed payload access, immutable provenance and valid owners. H-523 settles canonical-wrapper admission below. Inspection, wider transport and checked summary relations remain under review; no general value ABI, ordinary type-inference change or new compiler support is approved.

- **Construction, propagation and OOM diagnostics:** [H-510–H-517](../CHANGELOG.md#h-510), [D-17-RETHROW](DesignAuthorityRegistry.md#d-17-rethrow)/[D-17-OOM-RESERVE](DesignAuthorityRegistry.md#d-17-oom-reserve), [K-EXCEPTION-CONSTRUCTION-20261116](../CHANGELOG.md#k-exception-construction-20261116), records r72→73. H-510–H-515 separates immutable throw propagation from exception construction/trace capture, explicit constructor-form rethrow, implicit caught causes, bare wrapper rethrow and construction-only readonly cause. H-516/H-517 selects bounded process-level OOM information with the earliest record pinned and later records retained in a rolling window. Candidate C-36/C-63/C-65/P-464–P-469 relates inspectable failure provenance and bounded emergency diagnostics to charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve semantic checkedness, actual owners, immutable exception values, current capture rules, deferred-stack boundaries and D-16. Acceptance and open physical/trace/aggregate policies are below; no source support, permanent ABI, cause-storage bound or full value-path qualification is inferred.

- **Deferred exception stack:** [H-503/H-505–H-509](../CHANGELOG.md#h-503)/[D-17-DEFER-DISPATCH](DesignAuthorityRegistry.md#d-17-defer-dispatch) records the current distinction: attached catches handle their own deferred body; bodyless handlers handle queued deferred failures at their list position, potentially handling several errors, and their optional finally executes once on that visit. Ordinary scope-body errors are excluded. Candidate P-461–P-463 connects C-36/C-44/C-63/C-65 to charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit failure coverage, observable order and valid resources. [K-DEFER-STACK-20261116](../CHANGELOG.md#k-defer-stack-20261116) records r71→72 and bounds list-position dispatch without immediate look-ahead. Preserve preparation, actual registration, type identity, contexts, nested scopes, owners and D-16. H-509 additionally requires UnhandledDeferError to retain all residual deferred exceptions; H-510–H-515 fixes explicit construction/rethrow forms below. Acceptance and remaining stack order, original-error combination, explicit returns and new-registration policies are below; this does not qualify those choices or establish compiler support. H-505 supersedes original-body recovery through deferred catches; unaffected exception/ownership contracts continue.

- **Module shutdown order:** [H-501](../CHANGELOG.md#h-501)/[D-15-SHUTDOWN](DesignAuthorityRegistry.md#d-15-shutdown) selects application-wide reverse registration for module-level deferred actions after interleaved initialization. [K-MODULE-SHUTDOWN-20261116](../CHANGELOG.md#k-module-shutdown-20261116) records r69→70. Candidate C-36/C-44/C-59/C-63/C-65/P-458–P-460 serves charter §§2/3.3/4.3/6.3/12 through observable cleanup order and live owner obligations. Existing qualified per-action/per-scope preparation, failure, owner and nested-scope contracts continue. D-15 owns the 3/2/1 and partial-startup acceptance targets; D-17-DEFER-DISPATCH owns the handler distinction and remaining completion/representation questions. No runtime startup scheduler or new compiler-support claim follows.

- **Mainline continuity:** [K-719](../CHANGELOG.md#k-719) continues r67→68 while integrating the scoped assertion/naming decisions below. Current source/native capabilities, qualified value paths and unresolved mechanisms retain their separate boundaries; current compiler support is tracked in the source capability reference.

- **Built-in failure naming:** [H-472/H-473](../CHANGELOG.md#h-472), [D-17-NAMES](DesignAuthorityRegistry.md#d-17-names) and P-414/P-415 preserve canonical Error/Exception names and the common root Exception. [K-720](../CHANGELOG.md#k-720) continues D-17r68→69 with the implemented vocabulary and typed-root catches recorded in [source coverage](../tests/cod/language/README.md). Type identity governs checkedness regardless of aliases. Existing propagation, owner/access, initialization and cleanup contracts remain; assertion implementation, physical representation and broader qualification are separate.

- **Type and predicate assertion failures:** [H-467–H-471](../CHANGELOG.md#h-468)/[K-ASSERT-SPECIALIZATION-20261115](../CHANGELOG.md#k-assert-specialization-20261115), [D-10-TYPED-EXPR](DesignAuthorityRegistry.md#d-10-typed-expr)/[D-10-ASSERT](DesignAuthorityRegistry.md#d-10-assert), P-411/P-413, integrates unchecked runtime mismatch and predicate failure with static resolution and explicit non-null refinement. Existing C-63/P-235/P-236 and charter §§3.3/4.3/6.3/12 preserve effects, ordinary recovery and cleanup. TypeAssertionFailedError/AssertionFailedError follow H-472 and select no ABI. Static failure rejects; unresolved failures need no throws/catch. Concrete-type operation selection preserves D-26 coverage and unknown-interpretation distinctions. Acceptance includes proof success/failure, runtime cleanup, checked operand failures and path-sensitive facts; broader syntax/evidence mechanisms and source implementation remain separate.

- **Default preparation and initialization:** [H-458](../CHANGELOG.md#h-458)/[K-DEFAULT-PHASE-20261115](../CHANGELOG.md#k-default-phase-20261115) records the selected entry, prepared-call and cleanup contract in [D-10-ARGUMENTS](SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding). [K-715](../CHANGELOG.md#k-715) continues this scope together with the current bounds policy. Caller-context defaults prepare before entry; definition-environment defaults initialize in the entered callee. C-63/P-235/P-236 and charter §§3.3/6.3/12 preserve observable effect order and exact cleanup responsibility. Acceptance covers both failure phases and deferred registration/invocation; this adds no rollback, ownership restoration or implementation claim.

- **Bounds policy:** [D-17-BOUNDS](DesignAuthorityRegistry.md#d-17-bounds), r69 section below, records [H-448](../CHANGELOG.md#h-448)/[K-BOUNDS-POLICY-20261115](../CHANGELOG.md#k-bounds-policy-20261115). C-63/C-65/P-401–P-402 serves charter §§4.3/6.3/8/12: recoverable default bounds failures without mandatory throws annotations, with an explicit user-selected check opt-out. IndexOutOfBoundsError is unchecked; builtin indexing checks bounds by default. The opt-out transfers the range obligation to the program; invalid unchecked execution has no defined-result/exception/cleanup guarantee. Other type, owner/access, initialization, null and numeric contracts remain. Acceptance covers default recovery/cleanup, empty and boundary indices, read/write/view selection, proved check elimination and explicit opt-out isolation. [K-709](../CHANGELOG.md#k-709) continues this scope through cause_at and the bounded whole-artifact option below. Broader indexing, mixed-artifact integration and varargs qualification remain separate.

- **Registry / current scope:** [D-17](DesignAuthorityRegistry.md#d-17); the semantic revision stated above and the independently qualified scopes in the current registry. **Confirmed/continued bounded exception, lifetime, scope, defer and return contract.** C-63/P-235/P-236; H-267/H-269/H-313–H-315/H-319/H-356–H-362/H-365/H-370/H-371; P-321–P-324/P-326/P-328/P-329. D-26r9/H-400/H-401/H-404/H-405/H-407 through C-90/P-374/P-375 governs the unknown-type failure categories and explicit declaration obligations.
- **Value path:** C-63/P-235 → C-10 and D-17/P-236 → C-63: recoverable business failure must preserve explicit regional responsibility, effects and inspectable lifetimes. H-267 confirms this bounded path and scope under charter §§2–4, 6.2–6.3 and 12; it does not qualify unrelated ownership proposals.
- **Contract boundary:** Acceptance: construction failure, exact release, explicit causes, owner/effect restoration, pre-entry versus callee failure, completion replacement and deferred queues. Complete integration/proof coverage remains work. No regional copy, lifetime extension, native-handle/out policy, RTTI, persistent transaction or broad D-04 guarantee follows.
- **Invariants / non-goals:** P-237 links D-14's ownership/borrowing, P-238 links D-10's scoped bindings, and P-239 links D-01's allocation, allocator symmetry and relocation. H-180/H-181 preserves hidden/captured owners; H-262/H-263 separates consumed names from valid local views. H-265/P-240 supplies the unchecked-OOM constraint. H-251 reserves RTTI decisions for human discussion. one cleanup responsibility per region; no per-object ownership, implicit owner resurrection, allocation-dependent OOM delivery, graph rollback, foreign-world editing or general RTTI decision. No C++ exception type or ABI becomes a CoD user type by implication.
- **Acceptance:** Nested normal and exceptional paths preserve values, cleanup and effect restoration; controlled construction failures neither leak nor double-release; invalid exceptional-path borrows reject. Deferred registration, execution and completion replacement preserve their specified ordering. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines remain the H records identified by the registry. [K-RESIZE-20261114](../CHANGELOG.md#k-resize-20261114) continues r56→57: H-431 retains E4 ordinary consuming-call cleanup for the H-432 resize proposal, with pre-entry preparation and post-cleanup result delivery kept distinct. H-428 allowed allocation effects and the qualified completion contract remain unchanged; no language allocation checkpoint or ownership recovery is added. [K-REFERENCE-IDENTITY-20261114](../CHANGELOG.md#k-reference-identity-20261114) integrates H-439/H-440 owner-plus-locator identity and is null, preserving established access, result and lifetime behavior. Current source/native coverage and proof boundaries are recorded in the [source capability reference](../tests/cod/language/README.md). [K-IS-NOT-20261114](../CHANGELOG.md#k-is-not-20261114) adds H-441 is not as exact identity negation, preserving evaluation, contextual null ownership, access and lifetime rules; implementation coverage remains bounded by the [source capability reference](../tests/cod/language/README.md).
- **Current implementation alignment:** [K-716](../CHANGELOG.md#k-716) continues the qualified caller-binding, operation-identity, bounds and prepared-defer contracts. The [source capability reference](../tests/cod/language/README.md) records direct/prepared scalar, intrinsic-exception and complete callable-holder methods through shared invocation evidence. Saved slots retain invocation-time read, initialization, owner, permission, pending-completion and cleanup obligations; complete payload copies retain ordinary failure/release behavior. Materializing caller-binding methods as ordinary callable values, wider preservation proofs, allocation qualifiers and physical storage mechanisms remain separate.

## Built-in failure names and assertion responsibility

H-472 selects **AssertionFailedError** and a canonical built-in naming convention: **unchecked names end in Error; checked names end in Exception**. H-473 names the general root **Exception**. H-524 adds **Error <: Exception** as the root of unchecked exceptions, including user-defined ones. Exception is a checked declaration contract; Error and its exception descendants are unchecked. Exception still accepts both branches as a general exception type, and catch (e:Exception) can handle both.

Checkedness is a semantic type contract, not a test of the identifier's suffix. Canonical built-ins follow the naming convention; arbitrary user exception names are not restricted. Aliases preserve the original contract. Unchecked means no mandatory checked handling obligation, not fatal, unrecoverable or necessarily the programmer's fault. The root relation selects no concrete field inheritance, native address layout or general RTTI ABI.

**Assertion responsibility.** A program reaching a false assertion has violated its author's assertion; raising AssertionFailedError is the correct CoD behavior, not evidence of a CoD soundness defect. A failed explicit type assertion likewise raises TypeAssertionFailedError. Both differ from the established-typing backstop UnsoundRuntimeTypingError. Soundness would instead be at issue if a required check were omitted or a false/invalidated assertion fact were used to permit unsafe work. Successful-continuation facts, static rejection under H-470, once-only effects and ordinary cleanup retain their contracts. Diagnostic message/payload layout remains engineering work.

For currently implemented categories, the [compiler descriptors](../cod/lang_exceptions.hh) supply canonical names to vocabulary selection, native emission and services. Concrete supported categories retain their checkedness. The current common-root descriptor and exact-category machinery do not implement the Error root or establish the new declared-contract behavior. Only canonical imports are admitted; no compatibility aliases for retired names are provided. User-selected aliases retain the resolved type's obligations.

| Canonical name | Classification | Current compiler support |
| --- | --- | --- |
| Exception | Checked declaration contract; general root including Error | Common-root values/catches exist; the complete new root/declaration contract is not established by that support |
| Error | Unchecked root; Error <: Exception | Design only; root/conformance integration unimplemented |
| AssertionFailedError | Unchecked predicate failure | Predicate assert construct not implemented |
| TypeAssertionFailedError | Unchecked type-assertion mismatch | Expression type assertion not implemented |
| OutOfMemoryError | Unchecked | Callable, deferred-record and cause allocation failures |
| RegionCapacityExceededError | Unchecked | Category/vocabulary; source regional construction remains outside the current compiler slice |
| InvalidCapacityError | Unchecked | Category/vocabulary; source regional construction remains outside the current compiler slice |
| IndexOutOfBoundsError | Unchecked | cause_at bounds checking; broader indexed access remains separate |
| MissingReturnError | Unchecked | Required-value fallthrough |
| RethrownException | Checked standard wrapper; requires a checked cause | Design only; constructor, trace and ownership integration unimplemented |
| RethrownError | Unchecked standard wrapper; requires an unchecked cause | Design only; constructor, trace and ownership integration unimplemented |
| UnhandledDeferError | Unchecked aggregate of residual deferred exceptions | Design only; exceptions field and propagation integration are unimplemented |
| UnsoundRuntimeTypingError | Unchecked | Category/vocabulary; outside-catalog typing validation remains separate |
| NullPointerException | Checked | Nullable target access |
| StaticallyUnknownTypeException | Checked | Category/declaration obligations; outside-catalog interpretation remains separate |

**Acceptance:** every concrete canonical built-in name agrees with its existing checkedness; root Exception accepts both categories; aliases retain obligations; assertion failure is attributed to the author's assertion without weakening checks/refinement; existing catches, declared failures, causes and cleanup preserve their semantics. Vocabulary, diagnostics and services use the same canonical descriptors. A typed Exception catch follows the ordinary catch-all path, including aliases and generic substitutions, without catching sibling-handler failures or weakening checked rethrow obligations. It does not by itself implement missing assertion/bounds operations or qualify broader user exception graphs. No backward-compatibility aliases are selected here.

## Checked Exception and unchecked Error roots

H-524 provides the semantic root relation; H-525 requires source acceptance to honor the declared type rather than silently substituting a more precise inferred type. H-526 rejects additional conformance that changes an existing exception's checkedness.

```cod
# Illustrative standard abstract roots and user data headers.
type Exception;
type Error <: Exception;
data CacheUnavailable <: Error { code:u64; }       # Unchecked.
data RecordRejected <: Exception { code:u64; }     # Checked.
```

These headers use existing nominal abstraction and explicit data conformance; they do not add concrete-layout inheritance or establish the construction/owner protocol for arbitrary user exception payloads. User-defined Error types are the intended supported extension point. Source implementation, payload/trace/cause ownership and abstract-root constructibility remain within their separate contracts. Error-derived exceptions remain exceptions: root catches accept them, while Error catches accept only the unchecked branch. Both follow source-order matching and the standard-wrapper transparency rule. RethrownError and UnhandledDeferError belong to the Error branch; RethrownException belongs to the checked branch. Existing concrete built-in classifications do not change.

### Declared handling obligations

A throw operand declared Exception has a checked obligation even if dataflow proves its current value is an IndexOutOfBoundsError. A broad catch binding declared Exception likewise stays checked when propagated. The compiler may diagnose that a more specific declaration is available; it must not silently narrow the declaration or accept an otherwise invalid throw based on that proof. An explicit Error or concrete unchecked declaration has the unchecked contract. Ordinary validation of the narrower declaration still applies.

```cod
proc forward(e:Exception) throws Exception { throw e; }
proc forward_error(e:Error) { throw e; }
# Without throws or covering catch, forward's throw must reject,
# even if every currently visible call passes an Error.
```

An explicit throws Exception contract imposes a checked obligation on callers. A particular implementation, specialization or optimizer proof that only unchecked values occur does not erase that declaration. Ordinary checked propagation is allowed only through an appropriate explicit throws contract; module/entry code must handle its checked obligations. A type alias is not a narrowing operation. Explicitly selecting a narrower declaration or an admitted type assertion is a source change expressing the author's intent; diagnostic tooling must not apply it silently. Ordinary generic substitution follows the declared generic contract and does not rewrite an explicit Exception annotation. This boundary does not remove actual-type dispatch or other established flow proofs where their contracts permit them.

Keep **runtime failure classification** distinct from the **declared propagation obligation**. A root-typed value can actually be an Error, and catch matching must still use its actual type. Automatic rethrow from that value chooses the runtime-appropriate standard wrapper, but retains the checked source obligation of the Exception declaration; it is not a way to evade handling. Likewise a hidden tip selection does not narrow the declared obligation. Explicit fixed-wrapper construction must satisfy its cause constraints and cannot use inferred Error identity to bypass a broader incompatible declared cause contract. An explicitly named business exception may deliberately translate a failure under its own type's contract, as already selected; that is different from silently changing the source declaration.

**Design counterexample:** accepting a rejected Exception-typed throw merely because an initializer or current callers happen to supply Errors guesses an unexpressed narrower contract. Use a diagnostic to recommend the narrower declaration instead. Exact diagnostic formatting/severity and richer transparent throws summaries are separate.

### Classification is not adapter-local

Once an exception type's defining contract establishes checkedness, adding conformance in an adapter cannot change it. In particular, importing a checked Failure and adding data Failure <: Error rejects, even though ordinary data-to-interface conformance is extensible. Aliases, selection context and load order cannot change that contract. An adapter may define its own new Error type and explicitly construct it with the original failure as cause; that preserves both identities and makes translation visible. Ordinary added conformance remains available when it preserves checkedness and satisfies its other contracts.

[D-10](SymbolicDynamicScope.md#disjoint-declarations-and-exempt-inheritance-paths) records H-527–H-531 disjoint declarations with existing-path exemptions. Declaring type disjoint Error, Exception after Error <: Exception captures that ancestor path as exempt; it therefore permits new Error-derived types without asserting an empty Error/Exception intersection. A genuinely new independent checked-to-Error route still rejects. A redundant shorter declaration warns and preserves intermediate contracts. H-526's fixed checkedness rule and H-525's explicit Exception obligations continue; no hidden CheckedException root is introduced.

**Acceptance:** user-defined checked and Error-derived types; inherited uncheckedness and alias stability; Exception/Error/typed catches in source order; root-typed known Error throws still requiring checked coverage; actionable narrower-declaration suggestions; unchanged explicit throws Exception contracts across specialization/callers; automatic wrapper runtime category with conservative declared obligations; and rejection of checkedness-changing adapter conformance. General user payload construction/transport, full wrapper-aware throws inference and a physical type/identity ABI remain unimplemented or separately scoped. Existing finite intrinsic-category evidence does not prove these extensions.

## Exception construction, throw and rethrow

H-510–H-515 gives construction and propagation separate responsibilities. An exception's trace is captured when the exception is newly constructed. Propagating, copying, storing, returning or catching an existing exception does not replace that trace or create a new exception merely to transport it. This is a semantic value/provenance rule, not a selected address identity or payload-copy ABI.

| Form | Result |
| --- | --- |
| `make SomeEx(args...) [from cause]` | Construct an exception, capture its creation trace and return the value. Without from, no cause is supplied. Construction itself can fail under its ordinary contract. |
| `throw e;` | Evaluate e once, select an existing exception through the catch-local identity rule below, and propagate it unchanged, including its type, fields, cause and trace. The operand may independently perform construction or other effects. |
| `throw;` | Propagate the nearest lexically available catch context's incoming tip unchanged. Requires that context under ordinary scope/capture rules. |
| `throw SomeEx(args...) [from cause];` | Construct the explicitly named exception operand, then propagate it. Construction captures the trace; throw adds no further exception or trace. Without from, no implicit caught cause is supplied. |
| `rethrow SomeEx(args...) [from cause];` | Construct and propagate the explicitly named exception with a fresh creation trace. An explicit from supplies its cause; omission uses the nearest lexically available catch context's incoming tip. |
| `rethrow;` | Construct a new RethrownException around a checked caught value, or RethrownError around an unchecked caught value, with the retained incoming tip as cause and a new creation trace; propagate the wrapper. Requires a caught context. |
| `rethrow from cause;` | Evaluate the explicit cause once, apply the catch-local identity selection below, then construct and propagate a fresh RethrownException for a checked cause or RethrownError for an unchecked cause. Capture a new creation trace; no caught context is required when the cause is explicit. |

Here SomeEx must resolve to an admitted constructible exception type; spelling an ordinary value or factory callable like a type does not make it constructor syntax. Type aliases and generic specialization follow their ordinary semantic identity rules. The constructor-form throw is explicit operand construction, not permission to attach a cause to an existing exception. `throw e from cause` and `rethrow e` for an already created value are not the selected forms. An ordinary factory expression can still be evaluated by plain throw; any exception construction occurs inside the factory under its own context.

**Cause initialization.** The canonical cause field is not an ordinary constructor parameter: it cannot be supplied positionally, through a named argument or through a parameter default. A from clause on exception construction is its single source-visible initialization mechanism, including construction via make, constructor-form throw and rethrow. The cause becomes readonly at publication and cannot be rebound or mutated through an alias or assertion. A business payload containing another exception remains distinct from the reserved cause relationship. Absence of cause and its representation are separate; no pointer/null-owner ABI is selected here.

Evaluate constructor arguments in their normal specified order, then an explicit cause expression with the catch-local identity selection below, and construct the exception once with the resulting values and trace. Omitted rethrow cause resolves to the caught context's retained incoming tip under the rules below, not an ambient dynamically replaced exception. If operand preparation fails, no partially established requested exception is propagated and prior permitted effects are not rolled back. Constructor failure follows its ordinary failure path; it does not mutate either the supplied cause or another existing exception. General defaults/construction rules retain their own contract.

**Implicit caught value.** A helper defined outside a catch cannot acquire an implicit cause merely because a catch calls it. Supply from explicitly there. Inside nested catches, use the nearest valid caught binding. A delayed action referring to an enclosing catch retains that value through the ordinary checked capture rules; it does not select whichever unrelated exception happens to be pending when it runs. Each invocation of a bodyless deferred catch has its own matched value and retained incoming tip. Its sibling attached finally has no implicit last-handled exception; it may use a different enclosing catch binding if one is valid, otherwise it needs an explicit cause. Missing required caught context is a compile-time error.

```cod
# Inside a catch with matched value e and incoming tip:
throw e;                         # Incoming tip and its trace, unchanged.
throw;                           # Same incoming tip, unchanged.
rethrow;                         # New checked/unchecked wrapper; cause is tip.
rethrow DomainError(details);     # New DomainError; implicit cause is tip.
rethrow DomainError(details) from another;
# New DomainError; cause is another, or tip if another has e's identity.
```

The named constructor's semantic type determines matching and checkedness. A checked cause does not make an explicitly constructed unchecked business type checked, nor does an unchecked cause remove a checked business constructor's obligation. Standard rethrow wrappers have the additional cause constraint below. Bare rethrow and rethrow from cause choose the corresponding wrapper using the selected cause's actual semantic checkedness, even through the root Exception type or aliases. Catch matching uses the wrapper-prefix rule below: only consecutive standard rethrow wrappers are transparent. A named business exception or UnhandledDeferError stops that traversal; arbitrary causes and aggregate members are not searched. Checked failure coverage must reflect both the incoming checked type and the selected transparent matching rule; the exact throws-summary relation remains under review below.

### Standard wrapper construction and diagnostics

H-523 requires RethrownException to have a checked cause and RethrownError to have an unchecked cause. These constraints apply to semantic type identities, including aliases, and to every construction form: make, constructor-form throw and constructor-form rethrow. A standard wrapper without a cause rejects. Constructor-form rethrow may supply that cause implicitly from a valid lexical catch; ordinary make/throw construction does not gain an implicit cause. Business exceptions may deliberately translate a failure under their own type contracts; this restriction belongs to the two transparent standard wrappers.

`rethrow from cause;` is the explicit-cause form of automatic standard wrapping. Prepare its expression once under ordinary evaluation/owner/access rules, use the current lexical catch's tip if H-522's identity rule matches, and choose the wrapper from that selected exception's actual checkedness. Allocate/construct one new wrapper and capture its creation trace; retain the existing cause chain unchanged. An explicit cause also works outside a catch and does not import a caller's hidden context. A failure while preparing or constructing the wrapper follows ordinary exception/cleanup rules; an invalid source construction does not execute merely to discover its mismatch.

For an explicitly named standard wrapper, static cause evidence must establish its checkedness constraint. Known mismatches reject. If the admitted cause can be either checked or unchecked, reject the fixed wrapper choice and recommend the automatic form; ordinary declared generic substitution or an explicitly narrower source declaration can establish the constraint. H-525 forbids using a more precise runtime-value proof to bypass an incompatible explicit cause contract. The automatic form may choose between the two prepared wrapper categories at runtime when necessary. This does not erase possible checked obligations, create a new runtime type family or specialize per wrapper depth. The remaining transparent throws-summary contract is below.

```cod
catch (e:const NullPointerException) {
    rethrow RethrownError() from e;  # Reject: checked cause, unchecked wrapper.
}
# Suggested replacement for that statement:
catch (e:const NullPointerException) {
    rethrow from e;                 # Constructs RethrownException from tip.
}
```

Required diagnostic content for the mismatch includes the selected wrapper, the cause's conflicting checkedness and a direct `rethrow from e;` suggestion. Illustrative text: `RethrownError requires an unchecked cause, but e is checked. Use rethrow from e; to select the matching wrapper automatically.` An unknown checkedness diagnostic must say that it could not establish the fixed wrapper's requirement rather than claiming a proved mismatch. Preserve the user's actual cause expression in a suggested replacement. For a make expression where replacing an expression with a statement would be invalid, explain the automatic propagation form without applying an invalid statement-shaped fix. Exact formatting is not a new language ABI.

Every valid consecutive standard-wrapper prefix consequently has one checkedness and one canonical wrapper category. An explicit wrapper catch matches and binds the outermost incoming wrapper of that category; there is no differently classified inner standard wrapper to search for. Ordinary catches still match the first non-wrapper exception, and all clauses retain source order. A business exception ends that prefix even if its own cause has a different checkedness.

**Acceptance:** checked-to-checked and unchecked-to-unchecked named construction; symmetric mismatch rejection and diagnostic suggestions; missing causes; implicit constructor-rethrow causes; root-typed known/refined versus mixed causes; automatic selection inside/outside catches; aliases; same-identity helper results; one evaluation and one new wrapper trace; unchanged source exceptions; allocation/preparation failure; and no checkedness change along a valid standard-wrapper prefix. Physical storage and the remaining throws/inspection contracts are not settled by these rules.

**Trace and storage boundary.** A value made in a helper and thrown later retains the helper's construction trace. A new rethrow wrapper records the new construction context while preserving the original exception and its trace as cause. A new UnhandledDeferError likewise has its own construction trace; its exceptions field preserves each member's existing information. No first-throw capture, automatic trace refresh, implicit captured-value recapture or mutation is allowed. Logical/native frame selection, optimized-frame reconstruction, maximum trace detail, access API and persistent representation remain engineering/design scope; the bounded source-site metadata in the current compiler does not establish full traces. Construction can require storage; plain propagation must not require a new wrapper or recapture allocation. Ordinary creation/chain-allocation failure and deferred-stack membership/replacement must be kept distinct.

**Acceptance:** compare direct propagation with aliases, helper calls, copies, stored returns and delayed propagation; distinguish creation trace from throw site; cover named/bare rethrow, checked/root-typed values, explicit/implicit/absent causes, nested catches, captured causes, rejection of cause constructor parameters and invalid outside-catch defaults, unchanged input values on construction failure, and finally without a sibling catch binding. Ordinary return/owner rules, explicit exception identities, independent deferred entries and D-16 remain constraints. The selected forms and trace rule are not implemented by this documentation revision.

## Wrapper-aware catch matching

H-518/H-519 selects source-order clause evaluation. Let tip denote the incoming exception. For ordinary clauses, follow consecutive standard RethrownException/RethrownError causes until the first non-wrapper exception and use that exception for matching. For a clause explicitly naming a standard wrapper type, match the incoming outer wrapper rather than its non-wrapper cause. Aliases of those types retain the same meaning; spelling does not change identity. Take the first matching clause in source order. There is no preliminary priority pass for explicit wrapper catches.

```cod
try { run(); }
catch (e:const IndexOutOfBoundsError) { handle_original(e); }
catch (e:const RethrownError) { inspect_wrapper(e); }
# For RethrownError -> IndexOutOfBoundsError, the first clause wins.
# Reverse these clauses to inspect the incoming wrapper first.
```

Stop traversal at a business exception even if it has a cause. Stop at UnhandledDeferError rather than searching its exceptions field. An explicit wrapper clause binds the incoming outer wrapper. H-523 excludes mixed standard-wrapper categories within a consecutive prefix, so no deeper category search is needed. An untyped/root catch retains its ordinary catch-all role and source-order restrictions. Matching must not construct a new exception, duplicate a wrapper graph or capture a new trace. Immutable chains allow reuse of the unwrapped locator/type during one clause scan.

### Matched binding and hidden tip

H-520 prefers retaining wrappers wherever the operation can use them, including explicit from e. H-521/H-522 selects a hidden tip reference with the matched catch binding e and **exception identity within the current lexical catch context**, including an unchanged exception returned by a helper. Keep e's payload access and ordinary nominal/actual type evidence consistent with the matched exception; retaining tip must not make typeof, generic inference or ordinary overload selection silently switch e to the wrapper's type.

The model keeps two existing targets: the matched exception and the incoming outer exception. No exception graph copy is required. Retaining/copying a small stable handle when required by capture/lifetime rules is not duplication of the graph. The hidden tip must refer to valid exception backing, not a mutable pending slot or an ended catch frame. Repeated deferred-handler invocations receive separate associations; nested catch exit restores the enclosing context rather than leaving a mutable last-caught value.

**Identity selection.** Evaluate the exception operand once. At a throw-value or explicit from operation with a current lexical catch context, compare its exception identity with that context's matched exception. If they are the same, select that context's tip; otherwise select the operand itself. Without such a context, use the operand unchanged. Bare throw, bare rethrow and omitted rethrow cause directly select that context's tip. This is selection of an existing exception, not mutation or implicit wrapper creation.

Identity is not equal type, fields, message or trace, and is not a relocatable raw address. Newly constructing an exception creates a distinct identity even with identical fields. Ordinary aliases, permitted payload copies, storage, arguments and unchanged returns retain the existing exception's identity; they do not transport an ambient catch context or secretly change the nominal type. The selected behavior requires a stable identity/handle implementation, in addition to retaining tip. Current five-word intrinsic values do not establish that representation. No universal public identity ABI, general RTTI or physical heap allocation policy is selected. Runtime construction in a loop or recursive call must produce a fresh identity on each execution, even at an identical source site with equal fields. Value/capture/field/outcome transport and extracted causes or aggregate members preserve existing identities. Source-site IDs and reusable OOM diagnostic slots do not satisfy this requirement. A finite token/handle implementation needs a live-identity uniqueness and exhaustion/reuse contract; its observable fallback remains unselected. Identity comparison may be folded when proved, but that proof does not waive H-525's declared checked obligation. The compiler review identifies transport through the current five-word carrier and retained backing as real implementation work; one local hidden reference alone does not implement the full value-identity contract.

```cod
# echo is defined outside the catch and returns its argument unchanged.
catch (e:const IndexOutOfBoundsError) {
    rethrow DomainError() from echo(e);  # Same identity: cause is incoming tip.
}
# Constructing another IndexOutOfBoundsError with equal fields is different:
# using that new exception as the cause keeps that new exception.
```

| Operation | Selected exception / boundary |
| --- | --- |
| Ordinary typing, type-specific payload access, arguments and results involving e | The matched exception; ordinary type/owner/access rules stay intact. |
| Bare throw, bare rethrow, or an omitted rethrow cause | The lexical catch context's incoming tip, preserving full wrapper history. Bare throw propagates it; rethrow constructs the selected new exception. |
| Explicit from operand or throw-value operand | Evaluate once; use tip exactly when the operand has the current context's matched exception identity. This includes unchanged helper results and aliases. |
| An operation in a separately defined helper | No access to its caller's catch context. A result returned unchanged can still match identity at a subsequent operation inside the caller's catch. |
| Cause/trace/category inspection, or an explicit request to bypass tip selection | Pending below; ordinary payload typing does not decide these APIs. |

The current context is the nearest lexically enclosing catch available at the operation, with its normal capture/lifetime checks. Do not search every active catch by identity or implicitly import a caller's context. If two nested catches have the same matched exception with different tips, the inner context governs operations in its body; leaving it restores the outer context. Identifier spelling, grouping and helper expression shape do not override this choice. A delayed operation retains the selected matched identity and tip/owners; it does not consult the unrelated catch or pending error active when invoked.

### Remaining wrapper-matching decisions

1. **Inspection and explicit extraction.** Specify which exception cause/trace/category inspection observes and how a programmer explicitly selects the matched object or an actual cause without tip normalization. Under H-522, an expression yielding the same matched identity is normalized at a catch-local from/throw operation regardless of its syntax; assigning it to an alias alone is not an escape hatch. Reading a cause is not itself construction and must not mutate a chain or produce a graph cycle. No generic exception-view ABI or new inspection API is selected.
2. **Transparent checked obligations.** H-523 fixes cause presence and equal checkedness along the standard wrapper prefix. Static coverage must still correlate incoming outer type with matchable leaf type rather than treating independent category sets as arbitrary pairs. Decide the transparent meaning of throws contracts and retained cause evidence; RethrownException alone does not prove a particular leaf category. Runtime depth must not create one generic instantiation per wrapper level.

**Acceptance:** direct and multiply wrapped inputs; source-order ordinary/root/explicit-wrapper clauses; stopping at business errors and aggregates; matched e versus retained tip; once-only operand evaluation; identity preserved by an unchanged helper return and permitted copies but not by equal-field reconstruction; nested same-leaf/different-tip contexts, shadowing and delayed/repeated catches; absence of implicit caller context in separate helpers; checked outer/leaf correlation; no graph duplication; and each resolved inspection/wrapper case above. Existing exact-category matching and five-word intrinsic values do not establish this behavior. Owner-safe stable identities, hidden locators and normalized leaf caches are implementation candidates, not qualified public representations.

## Selected direction

CoD provides `try`, `catch` and `finally` control in the accepted contract. Ultimate out-of-memory failure is an **unchecked exception**: calling allocation does not require a checked-error result, a `throws` declaration or an enclosing catch merely to compile. A caller may catch the failure; otherwise it propagates toward the execution boundary. Unchecked does not mean ignoring ownership validity or turning invalid references into runtime exceptions.

`make` continues to produce its normal `owned region :Root` result when construction succeeds. OOM is an exceptional exit, not an implicit null owner or a mandatory optional/result wrapper. H-265 does not select automatic allocation retries, region growth, compaction or a process recovery policy. “Ultimate” failure describes exhaustion after any separately defined policy has failed; no such policy is introduced here.

The initial error taxonomy distinguishes `OutOfMemoryError` (the memory provider cannot supply storage), `RegionCapacityExceededError` (placement exceeds an existing region's fixed budget), and `InvalidCapacityError` (an invalid or unrepresentable capacity request). All three are unchecked; none implies automatic growth or retry.

H-428 confirms allocation as an allowed initialization side effect. A failing initialization group does not rewind an existing region or undo earlier valid publication/effects. Its incomplete bindings are not successful results, while an already initialized target published into surviving same-owner state remains live with its region. Failed acquisition of a new whole region still releases its acquired resources exactly once. Application/library undo frameworks require their own contracts; the language supplies no allocation checkpoint or automatic rollback.

Compile-time type, moved-name, escape and binding errors remain compile errors. Other runtime checks are not all reclassified as catchable exceptions merely because OOM becomes catchable.

## Unchecked bounds exceptions and optional checks

H-448 selects IndexOutOfBoundsError as an unchecked exception for builtin indexed access, including admitted array indexing and executable variadic-pack indexing. A callee or caller need not declare throws or provide a catch merely to compile; a program may catch the exception, and ordinary exception propagation, defer/finally and cleanup apply when it is thrown. Empty sequences have no valid element index. Index-type admission/conversion remains a separate contract; this decision admits no fractional index or new numeric conversion.

Runtime bounds checking is enabled by default, including optimized builds unless the user explicitly opts out. A failing bounds check prevents the invalid element access or view construction and raises IndexOutOfBoundsError. Reads, writes and reference-producing builtin selection use the same policy. Preserve each operation's established operand preparation, conversion and evaluation order; adding a check must not reevaluate operands or silently change unrelated effects. User-defined indexing retains its selected method's own failure contract; any builtin indexing that method performs follows this policy.

```text
# Design example; executable varargs integration remains separate.
proc pick(i:u64, rest:u64...) -> u64 {
    return rest[i];           # No throws declaration required for bounds failure.
}
# With default checking, pick(0u64) raises IndexOutOfBoundsError.
```

An explicit compiler option may disable generated bounds checks. The programmer then assumes the range precondition for affected builtin accesses. Executing an out-of-range access with checking disabled is outside the defined safe execution contract: no result, exception delivery or cleanup behavior is guaranteed. In particular, a catch cannot be relied on to intercept that access. This is the interpretation of H-448's explicit at-own-risk opt-out, not a claim that unchecked exceptions themselves bypass cleanup. In-range execution retains its ordinary semantics.

The option controls source-level builtin bounds checking only; compiler/runtime internal consistency checks are not its target. It does not disable type/owner/lifetime/access/definite-initialization checking, checked NPE obligations, numeric validity or other runtime checks. It is not a compiler fallback for an unproved initialization or aliasing case under D-28-INIT/P-400. Static proof that an access is in range permits ordinary elimination of its redundant bounds check with no opt-out; lack of a range proof in the default mode requires the runtime check rather than unsafe acceptance.

The current source compiler exposes `--no-bounds-checks` for an entire compiled source artifact, including imported modules and every general/specialized implementation. Generated calls retain the selected policy independently of runtime-source emission. The admitted cause_at builtin reports the call site, requested index and available cause count through the existing exception fields; it adds no automatic cause or exception allocation. The source capability reference owns runnable coverage. Broader indexing, separate artifact/link/cache integration and static range diagnosis remain implementation work. Generated/shared/cached code must preserve the requested policy: an unchecked variant cannot silently satisfy a request for default-checked execution. This policy selects no permanent ABI or runtime registry.

Acceptance requires default no-throws invocation, optional catch with normal cleanup, empty/boundary accesses, preserved operand effects, consistent read/write/view behavior, proven check elision, and an explicit opt-out affecting only its chosen bounds checks. Opt-out acceptance must not assert a particular result from invalid unchecked execution. Current capability evidence must distinguish implemented source/native indexing paths from the remaining acceptance scope.

## Checked NullPointerException

H-313 accepts checked NPE: every potentially null dereference must be covered by a matching catch or declared by its immediately enclosing function/procedure with `throws NullPointerException`. Module top-level execution must handle it explicitly. A catch-all handles NPE when it covers the operation; no automatic boundary handler satisfies the source obligation. Null/default absence is not a substitute for definite payload initialization or valid owner lifetime.

```text
using "std:exceptions" NullPointerException;
function following(n:Node in O) -> const i64
  throws NullPointerException where {owner O} { n.next.count }
try { following(node) } catch (e : const NullPointerException) { 0 }
```

The exception name is selected and aliased through the normal type-identity rules. Throws may follow the result annotation or the where clause. It does not distinguish overloads. Bodyless contracts and symbolic procedure signatures retain the checked obligation; an implementation cannot expose more checked failure than its applicable declaration permits. A caller handles declared NPE even if a particular specialization does not throw. An outer caller's catch cannot excuse an undeclared callee body, and WPO inlining does not erase that boundary. Functions may throw/catch without gaining dynamic-symbol access.

Only a dereference creates the null-access obligation. `null` is a reserved literal, never a binding. H-440/H-441’s `p is null` and `p is not null` evaluate the reference operand once and examine its selected owner/locator without loading the target; merely rebinding a pointer with `=` does not dereference it. Ordinary payload argument evaluation, including copying bits into val, must account for null access in the caller. A valid non-null ref needs no NPE obligation. Stable pointer tests may establish a non-null view on the corresponding branch; mutations invalidate the fact, and checking a mutable field does not prove a later reload unchanged.

A handler's NPE or rethrow and NPE from finally must satisfy the enclosing callable's own contract. A business exception's NPE cause does not alter its matching category; standard rethrow wrappers follow the transparent prefix rule above. Existing E1–E5 cleanup, explicit causes and replacement semantics remain. OutOfMemoryError, RegionCapacityExceededError and InvalidCapacityError remain unchecked; type, expired-owner and definite-initialization errors remain compile errors. Checked metadata introduces no payload RTTI, allocation on throw, general exception-effect polymorphism or runtime initialization registry.

Acceptance covers direct/module and callee-boundary rejection, explicit propagation, local handling and finally cleanup, bodyless and symbolic signatures, literal/test/rebind operations without target access, val argument evaluation, branch refinement and nullable loop paths. D-14 retains the reference initialization, same-owner, fixed-owner and current relative-encoding constraints. General exception payload graphs and unrelated deferred exception mechanisms remain outside this increment.

## Unknown-type and typing-contract failures

[D-26](GenericAndRecursiveClosure.md) classifies unavailable required type interpretation as checked `StaticallyUnknownTypeException` and a detected violation of an established typing contract as unchecked `UnsoundRuntimeTypingError`. Both use the ordinary exception control, explicit causes and cleanup rules. Static type errors remain compile errors; incomplete compiler reasoning cannot be substituted with an arbitrary runtime exception and claimed as successful native coverage.

Generic syntax or valid uninterpreted passage alone creates no unknown-type obligation. A voluntary `throws StaticallyUnknownTypeException` declaration retains the fixed caller obligation, including known-type calls whose bodies succeed. Selected exception types, callable views, handlers and deferred invocations preserve this contract. Acceptance includes declaration coverage, retained categories and causes, ordered failure routing and exact release in specialized and shared native execution.

The source capability reference distinguishes category/declaration support from outside-catalog input and required-interpretation detection. This integration selects no new input validation protocol, carrier, permanent ABI, dynamic cast or runtime compilation mechanism; those D-26 boundaries remain separately scoped.

## Nonnumeric type assertion failures

H-467/H-468 selects an **unchecked assertion failure** for an admitted nonnumeric type assertion whose runtime check finds a mismatch. H-470 resolves provable assertions first: success needs no runtime validation, and failure rejects compilation. [D-10](SymbolicDynamicScope.md#nonnumeric-assertion-contract) owns the operation; TypeAssertionFailedError is its runtime category under H-472. H-469 separately adds [predicate assertions](SymbolicDynamicScope.md#predicate-assertions-and-flow-refinement), with unchecked category AssertionFailedError under H-472. Both preserve normal propagation, explicit causes, defer/finally, replacement and cleanup, without mandatory throws/catch. Neither unchecked classification nor numeric/bounds settings removes an unresolved assertion check.

A valid Entity holding an Edge can fail a request for a Node view without breaching Entity's established typing contract. Such mismatch is not UnsoundRuntimeTypingError. Lack of required outside-catalog interpretation follows D-26's StaticallyUnknownTypeException boundary rather than pretending that an unknown relationship is false. Operand evaluation keeps its own failure categories and obligations. No failed assertion undoes prior effects, restores consumed ownership or rewinds allocation.

Caller argument and caller-default failures precede entry; definition-default/body failures after entry use ordinary callee cleanup under H-458. H-469 requires an established non-null fact before requesting a ref view from a ptr: `assert p is not null` may supply it, and `:` itself adds no null-to-ref runtime check. Failed predicate assertions do not create facts on a handled-failure path. Operand dereferences retain checked NPE obligations; ordinary null-access rules are unchanged. H-470 compile-time rejection cannot be caught. H-471 concrete-type specialization preserves covered runtime failure paths when the actual input is unresolved; compiling a possible failing branch is not proof that the call necessarily fails. No parser/native implementation, object-header RTTI or broad source admission follows.

## Deferred work and explicit scopes

H-319 distinguishes expression grouping from scope creation. `{ ... }` groups expressions without starting a lexical or dynamic-effect scope. `{{ ... }}` starts a full scope: it owns its local storage lifetimes, restores its effects on exit and runs its deferred work. Calls and module top levels remain inherent full scopes. The [loop scoping rules](#loop-scoping-rules) specify loop-control bindings, iteration effect/resource restoration and enclosing-scope defer registration.

For resource-management idioms, `defer` is the explicit counterpart to C++ destructor-based scope cleanup. The cleanup obligation belongs to a registered action in a CoD scope; it is not an implicitly selected method on every object's type. Acquisition can supply the action with `defer init(...)()`. This analogy does not add independent destruction lifetimes to regional objects, replace intrinsic whole-region release, or make copied handles/callables acquire independent release authority.

H-356 selects two forms:

| Form | Registration | Scope exit |
| --- | --- | --- |
| `defer f(args...);` | Evaluate the callable and prepare its arguments once using normal call evaluation/conversion order. | Invoke the saved callable with the prepared arguments. |
| `defer { expressions... }` | Register the block without evaluating its body. | Evaluate the block. |

H-371 prohibits variable declarations and effect installation directly in a deferred block, including through ordinary grouping and control flow. Programs needing either operation must introduce an explicit nested `{{ ... }}` scope. The deferred action itself does not acquire an implicit invocation scope. Explicit nested scopes retain their normal local lifetime, effect restoration and deferred-work responsibilities; a return still targets the enclosing callable under H-362.

Outside the explicit block form, the deferred operand must be a function/procedure call. In `defer init(...)();`, the callable expression for the outer call is `init(...)`: it executes now, and its returned callable is retained for invocation at scope exit. In `defer factory(a())(b());`, factory acquisition and both argument computations run at registration in ordinary call order; only the final invocation waits. Parentheses have no special timing meaning. Arbitrary expressions require the explicit block form.

Saving an argument follows its declared passing mode. A bits value is prepared now; an externally borrowed argument retains its selected target and owner/access constraints, without copying the target's contents. A callable value retains its complete environment under D-18, so replacing its source holder later does not select different deferred code. Deferred direct procedure calls keep current-binding semantics when invoked; selecting their code at registration does not implicitly request `perform` or freeze unrelated TLS bindings. Deferred code retains the surrounding function/procedure permissions and explicit-only exception-chain rules.

Registration succeeds only after the callable and arguments have been prepared. An exception during preparation does not register a partial call; already completed preparations retain their ordinary cleanup responsibilities. Skipped control-flow paths register nothing. Registered work runs in reverse registration order within its owning scope. Borrowed values must remain valid until invocation; saving a view never extends its region or lexical owner's lifetime. H-370 keeps owning arguments under E4: registration identifies and saves the selected owning reference and corresponding actual region without consuming it. The original owner remains available for legal inspection and updates. Ownership transfers only when that registered call actually enters its callee at scope exit. Invocation uses the saved identity instead of resolving the source name again. Later operations must leave that ownership obligation valid; premature transfer/release and duplicate consumption must be identified by existing ownership checks. No owner copy, implicit lifetime extension, reservation protocol or new runtime exception follows. Preparation failure registers nothing and retains the existing preparation cleanup rules.

[Go's specification](https://go.dev/ref/spec#Defer_statements) similarly saves the function value and arguments before deferring the call. CoD retains its own explicit block scopes, checked exceptions, owner checks and delayed-block form. The earlier capture-list proposal is not needed for ordinary deferred calls; additional block-capture syntax is outside this bounded implementation item.

### Module-level deferred actions

[Module shutdown](ModuleWorlds.md#cross-module-shutdown-order) follows H-501: module-level actions across the selected program execute in reverse order of actual successful registration, preserving each action's module context and valid owners. Registration A1 → B1 → A2 therefore drains A2 → B1 → A1, even though A1 and A2 belong to the same module. Entry registrations participate in the same order; ordinary nested full scopes retain their local cleanup boundaries.

Startup failure prevents later initialization/entry work and unwinds only established registrations/resources. Preparation failure registers no partial action; skipped paths register nothing. Apply the same ordinary-action/deferred-exception-stack rules below across the module-level sequence. Each entry retains its module's selected context; exception matching retains semantic type identity. Original startup/body exceptions are outside this deferred-error stack and cannot be handled by a bodyless deferred catch. Cleanup does not resume aborted startup, restore transferred owners or roll back initialization effects. Residual deferred errors, final process completion and intrinsic owner/effect restoration must follow the resolved boundary policy; no ordinary module-local catch acquires ambient cross-module coverage.

### Exceptions between deferred actions

The forms have distinct exception coverage under H-503/H-505:

| Form | Execution and coverage |
| --- | --- |
| `defer { body(); } catch (e:const E) { recover(e); } finally { cleanup(); }` | Run this body's own work once at its reverse-registration position, independently of errors from other actions. Its catches handle only its body's failure. Its finally runs once under ordinary try/catch/finally rules. |
| `defer catch (e:const E) { handle(e); } finally { finish(); }` | At this registration's position, inspect queued deferred errors newest first and invoke a matching catch for each applicable error. Leave unmatched errors pending. Optional finally runs once after that visit, including when the stack is empty or no type matches. |

H-506 supplies the pending-deferred-exception-stack model used by H-507's once-per-entry finally rule. Ordinary actions and bodyless-handler entries are visited in reverse actual successful registration order. An exception escaping an ordinary deferred action is pushed onto this stack; the walk then proceeds to the next list entry. The bodyless handler is not invoked by jumping ahead over intervening entries. One visit may execute its catch several times, with a fresh catch-variable binding for each selected error. The same rules apply to H-501's global module-level list.

```cod
# Helpers and supplied error values are illustrative ordinary CoD values.
defer catch (e:const Exception) { report(e); }  # H
      finally { close_log(); }
defer { throw error_one; }                    # D1
defer { throw error_two; }                    # D2
# Registration: H, D1, D2.
# Drain: D2 pushes error_two; D1 pushes error_one;
# H handles error_one, then error_two; close_log runs once.
```

A successful matching invocation removes its selected error. Nonmatching entries retain their relative order. A previously pending error neither skips a bodied action nor enters its attached catches; successful execution or local recovery cannot silently handle an unrelated error. Catch clauses cannot catch failures from their own handlers or attached finally. In particular, `defer catch (e:const Exception) { throw e; } finally { cleanup(); }` must not repeatedly recatch its own rethrow. Process only the incoming entries for that handler visit; outgoing handler/finally failures remain for later list entries. This bounds the visit by the incoming error set, not by whether user handler code terminates.

**Finally and scope.** The optional finally belongs to the list entry, not to each exception handled. It executes once after the entry's scan, even with zero matches; a bodied action likewise retains one ordinary finally execution. The bodyless entry is finished after this visit, not retained as a look-ahead handler for older ordinary actions. Normal nested/full scopes still complete their own required cleanup before an escaping exception reaches the enclosing deferred invocation boundary. H-371's explicit-scope requirements remain: per-invocation declarations/effect installation require a nested full scope rather than an implicit handler-body allocation scope.

**Which failures enter the stack.** Only exceptions escaping deferred execution belong here: ordinary deferred calls/blocks, their attached catch/finally, and bodyless-handler execution/finally. A failure in a helper invoked by that execution is still part of the deferred invocation. An ordinary scope-body exception or a failure preparing a defer registration during ordinary body execution remains on its normal propagation path; a deferred catch cannot handle it. Registration attempted during a deferred invocation can itself fail as part of that invocation. Its admission and ordering are reviewed below. Handling every deferred error does not by itself handle an original body/startup error. Ordinary try/catch remains the mechanism for those errors.

**Context and ownership.** Use actual successful registrations, not textual existence. A skipped or failed registration supplies no entry or coverage. Preserve the selected module/callable context, current-versus-captured binding behavior and semantic error type identity on every catch invocation. Registration captures are not recreated for each error. Mutations and transfers performed by one invocation constrain subsequent invocations; an unproved second consumption of the same owner rejects. Each error value and catch binding must remain valid through its admitted use. Explicit nested scopes can supply invocation-local work and normal release. No owner resurrection, automatic capture copy, lifetime extension or implicit cause chain follows.

A handler has ordinary local exception control for its own work, but its escaping failure must not re-enter that handler or its sibling catch clauses during the same visit. H-510–H-515 fixes the construction/propagation distinction; the stack-order treatment of outgoing errors remains under review below. No unhandled error may be discarded merely because another error is handled.

**Residual deferred failures.** H-509 requires a single unchecked `UnhandledDeferError` containing all exceptions left after the defer list has been drained, exposed through its exceptions field. An empty stack supplies no such failure. Use the same aggregate boundary for any nonempty residual stack, including one element; do not select a different public type based on the count. Preserve the stack's resolved order and complete exception values, including any existing explicit causes. Ordinary type matching sees the aggregate itself; its elements are a collection to inspect, not automatic cause links or implicit recursive catch targets. Collection API/layout, owner transport, checked-obligation integration and combination with an original body exception remain under review. This selects an aggregate contract, not a native ABI or permission to allocate while reporting OOM.

**Standalone finalization.** No separate `defer finally { ... }` form is needed: `defer { ... }` already registers unconditional delayed work. Attached finally has a distinct purpose on a bodyless handler: it executes once after the scan rather than once per matching exception. Keep explicit grouping/full-scope rules unchanged.

### Remaining deferred-stack decisions

These choices are pending, not delegated implementation contracts:

1. **Outgoing error order.** Throw preserves an existing exception, while constructor-form/bare rethrow creates a new one under the contract above. This does not alone choose how a deferred catch's outgoing error occupies the pending stack: preserve the selected input position, or push it as a newly escaping event. Pushing each propagated value during a multi-error handler scan reverses that subsequence for the next handler. Define the rule without requiring hidden caught-value ancestry merely to decide what throw means. In either policy outgoing errors cannot be reconsidered by the same handler entry. Constructor failure is an outgoing failure; whether it replaces or additionally preserves the selected input is part of the remaining stack integration.
2. **Aggregate integration.** H-509 settles retaining every residual error in UnhandledDeferError. The remaining boundary is how to preserve an original ordinary body/startup exception when that aggregate also escapes, how an outer ordinary catch versus an outer defer-catch sees nested aggregates, and how checked obligations relate to the unchecked aggregate. Do not silently make every checked deferred failure acceptable through automatic wrapping, or flatten nested aggregates/causes without a rule. H-505 still excludes ordinary body errors from bodyless deferred handlers.
3. **Explicit enclosing return.** H-362's target and remaining-cleanup obligations continue, but its single pending-completion replacement must be reconciled with several errors. Proposed: a handler return handles its current input and updates the retained callable completion; unrelated queued errors remain subject to their handlers and final aggregate boundary. Decide whether remaining input errors of the same handler entry are still scanned, and whether an explicit return is intended to discard other queued errors or an original body error. Do not infer blanket clearing from the compiler's existing single pending slot. Abandoned owning results still require exact cleanup.
4. **New entries during a drain.** Existing source support permits a delayed block to register more work. Preserve that fact without assuming the new stack interaction: specify when a newly registered same-scope action/handler is visited and which already queued errors it may handle. Registration failure supplies no entry and follows the containing deferred invocation's error path. Explicitly nested full scopes retain their own ordinary local sequence; returning through them cannot bypass cleanup. No blanket syntax ban or indefinite same-entry rescan is implied.

**Acceptance and implementation boundary.** Cover D2/D1/H(error_one)/H(error_two)/finally order; zero and multiple matching errors; stable unmatched entries; independent bodied execution/local catches; handler rethrow without self-reentry; finally exactly once; failed/skipped registrations; exclusion of ordinary body/startup errors; module contexts and semantic identities; repeated mutation/owner consumption; and each resolved policy above on normal/exceptional exit. The existing single-pending-error/body-skipping implementation does not establish this stack behavior. Handler effects need repeated-invocation flow checks even though each list entry is visited once. Exception propagation/cleanup must retain allocation-free failure delivery: a pending-stack push cannot depend on a fresh allocation from an already exhausted allocator. Reusing completed action storage or preparing capacity at registration are possible mechanisms, not selected ABIs. The required UnhandledDeferError collection and proposed rethrow causes need dedicated ownership/allocation analysis; current bounded intrinsic-exception evidence does not establish them. General user exception payload graphs remain separate.

### Explicit callable returns

H-361 requires proc/func brace bodies to provide returned values with `return`. A value supplied from finally likewise requires explicit return syntax. A callable that finishes without a required explicit return raises an unchecked exception at runtime; the compiler should diagnose statically detectable missing-return paths through errors or warnings. The intrinsic vocabulary names this failure `MissingReturnError`; it carries the existing bounded system-exception details. Reachable required-value fallthrough produces `-Wmissing-return`, which `-Werror=missing-return` or `--warnings-as-errors` can reject. Static diagnosis does not turn this failure into a checked throws obligation.

H-362 confirms that `return value` inside a deferred catch or finally targets the enclosing func/proc and remaining deferred actions still run. Within the qualified single-completion scope it replaces a pending return/exception; a handler without return preserves a retained normal result. The new deferred-error-stack integration must follow the explicit-return question above rather than treating this as authority to erase every queued error. If no required result is supplied before callable completion, the missing-return rule applies. Void callables may complete normally without `return;`.

H-363 selects the `pure` declaration keyword for the const-function form whose body does not require explicit return: the final body expression supplies its result. Its boundary preserves readonly pre-existing external state, allows private mutation and constrains nested helper calls as described in [D-19](ConstFunctionsAndTailCalls.md). Pure-function implementation and TCO remain deferred by explicit user selection. The proposed `=>` form is not selected. Ordinary block-expression values remain separate from callable control-flow returns, and every return retains existing result-type, owner/access, storage and cleanup requirements. The executable proc/func fixtures use explicit returns; historical native checkpoint evidence retains its recorded source scope.

### Implementation availability

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Prepared values retain their independent snapshots; borrowed arguments retain the selected target and actual owner. Payload reads occur at invocation. Deferred registration contributes effects only after successful preparation; queue choices, failure paths and completion determine whether initialization is established. These are qualified behavioral obligations, not a mandated solver or runtime representation.

### Callable construction, installation and cleanup

H-380 cancels only affected dependency initialization TLS writes. Operand evaluation and callable construction retain their ordinary effects and exceptional exits; a produced value that is not installed follows existing temporary cleanup. No canceled binding is published or later restored. Procedure-local installations retain normal scoped cleanup. This supplies no permission to erase observable allocation failures or unrelated effects merely to shrink the final image.

H-372–H-376 distinguishes compile-time family registration from runtime callable materialization, TLS installation and invocation. A module definition is not a runtime resource acquisition to release during cleanup. Repeated combined effect execution creates separate materialization/installation events, not repeated method registration. Those events retain the existing preparation/publication boundary, restoration on every exit, captured-owner validity and exact release requirements.

Deferred calls retain the selected callable view and prepared operands; a family view must preserve its source interpretation and later actual-type dispatch. Deferred bodies remain delayed, with explicit nested scopes for declarations/effect installation. New abstract evidence and family dispatch must preserve checked NPE, pending normal/return/error completion and explicit causes. Missing/ambiguous method behavior not covered by existing static diagnostics needs its own decision; no new exception category follows from the AOT vision. [D-21](AheadOfTimeExecutionPreparation.md) tracks that architecture work while the bounded defer implementation continues.

### Immediate implementation and acceptance

H-356 authorizes implementing this language work independently of the LLVM binding pilot. The compiler distinguishes ordinary grouping from full scopes and iteration effect boundaries. Complete scope-owned deferred work and explicit returns while preserving the existing exception and regional invariants:

1. Parse deferred calls separately from deferred blocks. Validate the call shape without executing the final invocation or assigning timing significance to redundant parentheses.
2. Reuse typed call preparation for callee evaluation, ordered argument conversions, stored callable values and borrowed operands. Retain prepared state in the owning scope and register it only on successful preparation. Keep block bodies delayed.
3. Integrate deferred execution with normal and exceptional scope exit, pending results, effects and intrinsic owner cleanup. Use existing LLVM call/cleanup machinery and D-18 storage; no new general closure or persistence framework is implied.
4. Verify a compact set of composed cases: initializer-returned cleanup; saved values versus delayed block reads; later callable replacement; argument/conversion order and preparation failure; reverse-order cleanup on normal/exceptional paths; whole-loop versus per-iteration explicit scopes, direct loop defer accumulation and invalid borrowed-owner use. Record allocation and emitted-IR costs for repeated scoped use and accumulated registrations.

Before claiming complete coverage, verify H-370’s owning-argument boundary and resolve any uncovered integration choice involving checked exceptions from cleanup or cleanup failure while another completion is pending. Existing D-17 rules remain authoritative where they already apply. Neither a parser-only change nor execution of a standalone cleanup sample satisfies acceptance.

H-320 additionally proposes that an expected `func():void` or `proc():void` parameter can interpret `{{ ... }}` as an anonymous callable body instead of executing the block while evaluating the argument. The expected kind must retain function/effect restrictions; unresolved function/procedure overload ambiguity cannot be settled by silently executing the block. H-322 replaces the isolated inline-callback review with a unified owner-aware proposal for named and anonymous callables. Region-only reference captures, explicit copied bits and closure storage/lifetime requirements are reviewed in [D-14](RegionalOwnership.md#unified-callable-captures-and-region-owners); neither live lexical binding capture nor a blanket receiving-call-only restriction is selected. This is a language contract question, not permission to expose compiler callable-routing objects to source programs.

## Loop scoping rules

H-365 defines loops directly, without introducing a general-purpose scope abstraction. Ordinary grouping, explicit `{{ ... }}` scopes and inherent callable/module scopes retain their contracts. A loop combines loop-control bindings, per-iteration effects/resource lifetime and deferred work registered to a full scope; these responsibilities have distinct boundaries.

### Bindings and declarations

Loop-header index/accumulator bindings belong to the loop construct and follow its initialization/update rules. They are not fresh body variable declarations. An ordinary loop body forbids variable declarations, including storage, aliases, callable variables/named local callable definitions and region bindings. The restriction applies through ordinary nested groups and conditionals. A nested explicit `{{ ... }}` inside that body allows local declarations with the explicit scope's lifetime and cleanup. A full scope surrounding the whole loop does not authorize direct body declarations. Each nested loop needs its own per-iteration explicit boundary for body declarations; called functions/procedures retain their independent inherent scopes.

### Iteration effects and resources

Every iteration restores its entry effect bindings on every exit, including throw and return. A temporary binding cannot leak into the next iteration. Restoration changes bindings, not mutations already made to regional data. A fresh region installed through an effect, such as `effect @arena <- region(bytes)`, belongs to that iteration's resource lifetime: release it at iteration exit and restore the prior binding. Captured and hidden pre-existing owners remain alive; shadowing never frees the enclosing owner.

Existing access, transfer and provenance checks apply. An outer deferred action cannot borrow an iteration-owned region or a value dependent on it. Saving a view does not extend lifetime, and a dynamic binding does not permit ownership extraction. No allocator, regional representation or zero-cost-relocation rule changes.

### Deferred work and accumulation

Direct `defer` in an ordinary loop body is allowed. The iteration's effect restoration boundary does not own user deferred actions. Each successful dynamic registration goes to the nearest enclosing full scope (`{{ ... }}` or an inherent callable/module scope), skipping loops and ordinary grouping. Every registration remains distinct, and the owning scope runs all actions in global reverse registration order, including registrations before and after the loop. Accumulation is intentional for dynamic resource management; an implementation must not conceal its cost by cleaning up early.

```text
{{
  defer finish_batch();
  for (i in 0 .. count) { defer consume(i); }
  defer begin_cleanup();
}} # begin_cleanup, consume(count - 1) ... consume(0), finish_batch

for (i in 0 .. count) { {{ defer consume(i); }} } # one cleanup per iteration
```

Putting `{{ ... }}` around the whole loop forms a cleanup batch. Putting it inside the body creates per-iteration cleanup. Callee evaluation and argument preparation remain eager under H-356; partial preparation registers no action and preserves prior registrations and acquired cleanup obligations. D-17's pending return/error, deferred catches and finally rules apply. Outer deferred calls run after iteration effects have restored; direct procedure invocation sees its current bindings, without an implicit TLS snapshot.

### Loop-control snapshots and borrowing

A delayed defer block cannot capture loop-control variables, including the index and fold accumulator. `defer consume(i)` saves the current bits argument through ordinary call preparation; `defer { consume(i); }` is invalid. There is no implicit final-value capture, lexical boxing or lifetime extension. A borrowed argument saves the selected reference rather than a deep copy, so its owner must outlive eventual invocation. Call syntax does not make borrowing loop-control storage or iteration-owned resources valid for an outer deferred action.

### Zero-iteration else

Range `for ... else` executes its else branch only when the body executes zero times, not on exhaustion. The else branch follows the same rules: direct variable declarations are forbidden; its effects restore and fresh effect regions release at branch exit; direct defer registers to the enclosing full scope. A nested explicit `{{ ... }}` permits local declarations and local deferred work. The loop index is unavailable in else. Once-only bound evaluation, initialization joins and fold's initial accumulator are unchanged. This contract does not add `fold ... else`, break/continue or new loop forms.

### Acceptance

Composed programs must establish outer/iteration effect values, normal/throw/return restoration, exact releases with captured/hidden owner retention, eager snapshots and global LIFO over dynamic registrations. Validate whole-loop and per-iteration cleanup batches; reject declarations through ordinary body/else groups, delayed loop-control captures and outer deferred borrows of iteration resources; accept nested explicit and callable scopes. Cover zero iterations and bound evaluation. Measure live registrations, peak storage and execution/IR costs as registration count grows. These checks must compose with return/error replacement and remaining cleanup. D-16 syntax/comments, numeric promotion, bits layout, transfer and formatting remain unchanged; fraction implementation stays deferred.

## Expression-oriented control

`try` is an expression, consistent with CoD's existing expression model. Normal completion and handled completion provide its result; an unhandled exception provides no normal result. A `finally` block performs completion work without replacing the result merely through its final expression value. No public bottom type or general continuation API is needed to express this planning requirement.

The exception identities are explicitly selected into the module before use:

```text
try {
  owned region packet:Packet <-
    make Packet(capacity = bytes) { sequence <- initial };
  process_packet(packet)
} catch (error : const OutOfMemoryError) {
  handle_allocation_failure(error)
} finally {
  finish_attempt()
}
```

The same construct can occur in a function or a procedure. Each body's existing restrictions still apply: a function may operate on explicit memory but cannot access dynamic bindings or call a procedure. A lexical catch does not make dynamic lookup available inside a function. Exception capabilities do not create module exports, change callable identities or expose compiler call-routing machinery.

The control sequence is:

1. Execute the protected expression. On an exceptional exit, run cleanup for scopes being exited before entering the selected handler.
2. Execute at most one applicable catch. Unmatched exceptions continue outward. An exception raised by a catch is not caught again by that same try's sibling catches.
3. Execute `finally` once when leaving this try, after normal or handled completion, or while propagating an exception. Then deliver the pending result or propagate the pending exception.

This does not promise `finally` execution after forcible process termination or loss of the execution environment. Native resource release and TLS restoration are compiler obligations even when the program writes no `finally`; user finally code must not be required to free every intrinsic owner manually.

## Ownership and effects on exceptional paths

Exceptional control must preserve the actual ownership history; it is not a transaction that rewinds the program:

| Situation | Required preservation under the existing ownership/effect constraints |
| --- | --- |
| Region allocation fails before obtaining storage | No owner exists to expose, consume or release. |
| Allocation succeeds, then root initialization fails | The unfinished region still has one construction cleanup responsibility. Release it before propagation; never publish an uninitialized owner. |
| An effect's RHS construction fails | Do not install a new binding. Preserve the previous binding and owner; do not roll back unrelated edits made while evaluating the RHS. |
| A local owner transfers into an inner effect and that scope exits exceptionally | Release the region at its actual new owning scope. The original name stays consumed; old views cannot be used after that owner ends. |
| A transfer retains a live, accessible local owner | Preserve existing valid views under H-263, regardless of the old name's consumed state. |
| Perform temporarily shadows current bindings with captured values | Restore lookup on exceptional exit without freeing either pre-existing captured or hidden owner. Release only resources actually acquired by the exiting scope. |
| A caller-owned region was already edited before a later exception | Retain those edits unless the business algorithm separately implements rollback. |

Cleanup follows scope exit and reverse acquisition/restoration order within each scope. The handler's incoming ownership facts must include every throwing path that can reach it. If one such path transferred a region and another retained it, the handler cannot unconditionally use the old owning name. This extends the existing branch-validity requirement to exceptional edges; WPO does not make runtime failure positions statically known.

Pending expression results need explicit attention. If a try body produces an owning region but its finally subsequently throws, that not-yet-delivered result must still be released once. A borrowed result remains dependent on its actual owner through finally. Neither a stale pointer nor an abandoned owning result may escape by treating the normal result as delivered before finally completes. E4 below fixes the call/result transfer points; D-14 retains the wider ownership design.

## Exception data and low-memory operation

Raising OOM, routing it, restoring effects, releasing regions and entering a catch must not require a fresh allocation from the exhausted memory source. A handler can itself allocate and fail. Intrinsic system failures carry bounded scalar details; no borrowed payload may outlive its owner. General user exception payload graphs remain separate.

## Accepted first implementation contract

H-267 accepts E1–E5 together. Their value is a complete construction/edit/transfer/release workflow with usable failure handling and correct view validity, without per-object ownership or changes to regional pointer encoding. The explicit-cause amendment is part of E2/E3.

### E1 — Intrinsic exception categories and payload

The initial unchecked system failures are: `OutOfMemoryError`, `RegionCapacityExceededError` and `InvalidCapacityError`. The first means the memory provider cannot satisfy allocation; the second means an existing region's fixed budget cannot satisfy placement; the third means an invalid/unrepresentable capacity request. No automatic growth or retry is implied.

Expose their type identities through an ordinary standard vocabulary module, with normal explicit selection and local aliases. Their intrinsic payload is bounded bits: category, a source-site identifier and relevant scalar size information. The source-site identifier is resolved against immutable metadata for the executing program; it contains no native address and does not promise meaning under a different program image. Error formatting must not turn it into a stored cross-owner pointer. Catch selection uses a small runtime category discriminator. General type reflection, arbitrary user exception payloads and native C++ type matching remain outside this increment. H-267 accepts this bounded discrimination under H-251; it does not approve general RTTI. Matching needs a type identity at the throw/handler boundary, not a runtime type descriptor embedded in each payload. This bounded intrinsic payload describes current implementation support; the constructor-time trace contract above requires separately qualified trace storage and does not follow from a source-site identifier alone.

### E2 — Expression syntax, matching and completion

Use this source family, with exception identities already selected into the current module:

```text
try {
  operation()
} catch (error : const OutOfMemoryError) {
  recover(error)
} catch (error) {
  report_or_rethrow(error)
} finally {
  finish_attempt()
}
```

For currently implemented intrinsic categories, a typed catch matches its exact category. The selected standard-wrapper extension follows the source-order prefix rules above and is not yet implemented. Duplicate categories reject; an optional final catch-all matches the remaining supported categories. No subtype search or clause-order priority among distinct exact categories is needed. Catch parameters receive immutable bits values; the catch-all has the intrinsic common system-exception value type. This does not change ordinary parameter borrowing rules. Public field/accessor spelling is a routine API detail within the bounded payload, not authority to expose arbitrary runtime metadata.

Normal and handled arms must have the same result type under the existing conditional-expression rule; exceptional paths supply no normal value. Require `finally` to yield void. `try` may have catches, finally, or both, but at least one is required. `throw error` and bare `throw;` propagate existing values unchanged. New cause relationships are established at exception construction through from; rethrow uses the constructor/bare-wrapper forms above. Ordinary catch/finally does not add a cause automatically. Source creation of arbitrary user exception values/graphs is deferred. Catch bodies are outside their own sibling catch coverage. After all relevant cleanups/finally blocks, an uncaught exception gets a minimal diagnostic and a nonzero exit status at the program boundary.

### E3 — Failure from finally

For ordinary try/finally completion, a new exception from finally replaces that construct's pending exception or normal result. Outer cleanups/finally blocks still run; any abandoned owning result releases once. A deferred entry's escaping finally failure instead enters the deferred-error stack; it does not thereby replace every independently queued error. The replaced exception is not retained automatically. A program constructs a cause-bearing exception, for example with `rethrow SomeEx(...) from cause`, when it wants that relationship. A business exception's cause does not participate in catch matching; standard rethrow wrappers follow the transparent prefix contract above.

### Lexical backing and completion integration

[D-28](RegionalAndLexicalStorageRefactor.md#integrated-operations-and-lifetimes) records the relocation obligations for the selected lexical storage direction. Prepared references retain their selected targets/places; saved val results retain independent bits. Required deferred work, completion replacement and intrinsic release keep their existing order. Necessary owner metadata/backing remains valid through each admitted use, including cleanup that allocates, and ends with its semantic scope. Failure while preparing replacement lexical backing cannot invalidate existing targets before replacement is ready; it does not undo earlier allowed effects. This does not change E4 consuming region parameters or qualify a physical cleanup/anchor protocol.

### E4 — Consuming calls and results

An admitted owning parameter transfers responsibility at callee entry, after argument evaluation and conversion succeed. An external borrowed parameter retains the original owner without acquiring cleanup responsibility. H-271/H-272 requires explicit owner-generic declarations for external borrowing; the native prototype's unqualified `region :Root` borrowing spelling is a migration boundary, not the new default. The call itself has not consumed an owning argument if a later argument fails before entry; explicit transfers performed inside earlier argument expressions retain their actual effects. Validate availability and provenance again at the call boundary, including duplicate ownership transfers and later-argument invalidation of earlier views.

H-458 applies this boundary to parameter defaults: explicit inputs prepare in written order, then omitted caller-context defaults prepare in declaration order, all before entry. Definition-environment defaults run during parameter initialization after entry. A failure there cleans every input whose ownership transferred at entry, including an input whose named binding has not yet initialized. For an admitted plain deferred call, caller-default preparation belongs to registration and callee-default initialization to invocation; failed registration retains ordinary preparation cleanup and registers no partial call. Earlier valid effects are not rolled back. Named/default source support remains implementation work.

After entry, an owning parameter's region belongs to the callee scope until returned or otherwise transferred; an exceptional callee exit releases what that scope still owns. The caller's original name remains consumed. An owning result transfers only after the callee's required finally work completes successfully; a result abandoned by finally failure remains subject to cleanup. This preserves the existing per-argument evaluation/conversion order without treating ownership transfer as a numerical conversion.

H-431 explicitly applies this rule to consuming replacement, now proposed as resize(r, new_cap) under H-432 in D-28. “Call transfers ownership” means callee entry, not the beginning of argument evaluation. “Only successful completion returns ownership” includes required finally/deferred cleanup. A return selected before throwing cleanup is not delivered; its abandoned owning resource is released. Catching an exception does not restore the caller's consumed input. A function may explicitly return the same region on success, but that is receipt of an owning result, not reversal of the input transfer. Resize's separate replacement contract ends old-region views rather than retargeting them. Its proposed capacity checks happen after entry: even an undersized request that throws does not restore the input. D-28's exact mapping of resize failures to existing categories remains a scoped proposal; E4/E5 is unchanged.

The borrowed and consuming call forms are:

```text
function edit(region packet:Packet) -> void { ... }
function transform(owned region packet:Packet) -> owned region :Packet { ... }

edit(packet); # Borrow; the caller still owns packet.
owned region next:Packet <- transform(packet);
# packet is consumed after callee entry; next owns the successful result.
```

The first increment does not select overloads whose only distinction is owning versus borrowed mode; reject such a competing pair with an explicit unsupported-mode-overload diagnostic. Otherwise adding an overload could silently change whether a call consumes its operand. General mode-based overload ordering remains a separate language decision.

### E5 — Bounded construction and acceptance scope

This increment completes typed make/root access, fresh owning declarations, factory results, borrowed/owning calls, effect installation, branch/exception ownership joins, and try/catch/finally together. The required capacity is a runtime byte budget for the root and subsequent payload allocations, including their internal alignment consumption but excluding the fixed region header. No automatic growth is introduced. The first root initializer names every declared field exactly once and evaluates initializers in written order; omitted/duplicate fields reject. More general initializer bodies and defaults remain separate.

Ordinary owner reassignment/reinitialization, occupied owner destinations, general reference-assignment/const-view overloads, mailbox transport, user exception graphs and general RTTI are outside this increment. Existing typed memory operations can demonstrate root edits without pretending that the complete assignment surface is implemented. Exceptions must not roll back prior graph edits or ownership transfers.

Implementation includes the actual allocation-to-publication cleanup repair, the direct LLVM control path, the required build and focused CoD-language/shilos-region validation. Native system-call failure translation is internal; the implementation must preserve the user semantics above without exposing an ABI or adding an alternative backend. Acceptance covers complete construction/edit/call/transfer/release, captured/hidden owner preservation, normal and exceptional control, pre-entry versus in-callee failure, invalid owner/view use and deterministic low-memory failure injection. Source, diagnostics and evidence should be reviewable before any broader feature is added.

Cancellation, asynchronous faults, restartable/resumable exceptions and cross-process exception delivery remain outside this first contract. The compiler/runtime route described below is an implementation recommendation subject to the accepted semantics, not another user-visible mechanism to approve.

### Bounded process OOM information

H-516 selects a bounded information area statically reserved at process level for OOM handling. H-517 retains the earliest OOM record unchanged and uses the rest as a rolling window: publishing a new later record evicts the oldest retained rolling record. The first record remains pinned for the process lifetime. This is bounded diagnostic retention, not an expanding region or an unlimited emergency heap.

The reserved path must not allocate from the failing allocator to record basic OOM information. Separate exception semantics from diagnostic availability: a retained exception's category, scalar details, readonly cause and creation identity cannot change because a diagnostic slot is reused. A handle to an evicted trace/information record reports unavailable; it must never expose a different newer event now occupying that slot. Any public read must obtain a coherent bounded snapshot or equivalent checked view, not an escaping raw reference into overwritable storage. Internal record identity/generation and synchronization are engineering choices; no counter width or pointer ABI is selected.

Use one defined publication order across concurrent runtime users of the process reserve, with exactly one pinned earliest published OOM record. Readers cannot observe partial publication or associate another thread's record with an older exception. In-flight recording/reading, truncation indicators and rollover must not corrupt the pinned record or create allocation recursion. Bounded per-record trace capacity must expose unavailable/truncated diagnostic detail honestly; it cannot pretend to retain an unbounded stack. Exact capacity, formatting and synchronization protocol remain implementation work within this visible retention policy.

Rolling records do not own semantic exception fields or the only copy of a cause needed by a still-live exception. UnhandledDeferError must still retain every required member, and newly constructed rethrow wrappers must retain their immutable causes. Their owner/carrier protocol, repeated-wrapper storage growth and construction failures remain separate: the information window does not make E errors wrapped by H handlers fit into E+H semantic nodes or authorize overwriting live exception values. Prepared/reused deferred carriers are possible mechanisms for that separate obligation, not a consequence of this reserve.

**Acceptance:** basic OOM reporting with ordinary allocation unavailable; first record unchanged after more events than the window can hold; retained older exceptions whose diagnostics become unavailable rather than misidentified; stable cause/aggregate values; simultaneous recording/reading; bounded/truncated trace disclosure; and no implicit region growth, owner resurrection, raw dangling diagnostic view or allocation-dependent basic propagation. The current native cause helper can allocate a vector entry and the current drain releases registration records before invocation; neither implements this reserve or the required aggregate/cause lifetime protocol. No compiler code changes or new runtime validation are supplied here.

## Compiler and runtime implementation

Current supported and missing behavior, implementation entry points and validation commands are maintained in the [source capability reference](../tests/cod/language/README.md). Language requirements in this document remain subject to their recorded qualification; examples specify acceptance targets, not evidence that every target is implemented.

Native exception handling must preserve checked obligations, explicit causes, scope cleanup, effect restoration and exact release. Intrinsic system-failure information is not general reflection or a mandated user-visible C++ exception ABI. Concrete payload and control-flow representation remain implementation concerns.

### Retained explicit-cause lifetime

H-269 requires explicit cause relationships to survive returning or storing a caught exception beyond the catch. H-510–H-515 establishes those relationships only when constructing a new exception, with readonly cause thereafter. Runtime-managed storage must retain the relationship without mutating a supplied exception/cause. Ordinary cause-bearing construction may itself raise OutOfMemoryError; basic OOM propagation does not allocate and construction failure does not automatically attach either operand to the new OOM. Existing saved values retain their information. Deferred-stack retention of a selected input when handler construction fails remains its separate integration question. The current compiler's throw-from operation and numeric cause handles are implementation coverage, not acceptance of the new constructor/rethrow/trace contract.

The bounded runtime uses execution-local numeric handles for immutable cause records. Regional fields contain bits and handles rather than host addresses; copying a regional image within the same execution does not require pointer patching. Generic exception storage also preserves its bounded failure-category discriminator. These handles, like source-site IDs, are not a cross-process persistence or delivery format. General user exception graphs and general RTTI remain excluded.

Retained backing storage is released at execution end, including uncaught failure. The initial implementation retains discarded records until that boundary; repeated explicit chaining consumes that storage and may raise OOM. It does not add per-value destructors, graph ownership or eager collection. Public inspection uses a bounded cause count and bounds-checked indexed access; inspection and propagation allocate no cause records. Exact reclamation optimizations can preserve this source lifetime contract later.

The native cause table is created only by explicit chaining. Its TLS slot is a trivial pointer, with no lazy destructor registration on basic OOM propagation or cleanup. Failed table creation, first-record allocation or later growth leaves existing histories intact. The native fault harness fails the actual allocation at each of these three boundaries, verifies a fresh unchained OOM, retries successfully and accounts for all backing allocations/releases.

### Allocation-to-publication cleanup

The regional allocator must release its byte block if root construction throws, using the matching allocator and exact capacity. [The construction fixture](../tests/shilos/region_construction_tests.py) checks failure before root entry, both throwing-root overloads and successful retention/release. This allocation-to-publication boundary supplies no existing-region rollback or graph transaction.

## Bounded acceptance and remaining scope

Acceptance requires nested normal/exceptional completion, pre-entry versus callee failure, explicit cause retention, cleanup replacement, deferred queues, failed construction and invalid-borrow rejection. Check exact acquisition/release on supported paths.

Mailbox transport, general user exception graphs, RTTI, cross-process causes and remaining ownership choices retain separate scope. Current implementation limits are maintained in the source capability reference.

## Type routing and handler scope

A try scope supplies its accepted exception type identities and cleanup/handler destinations. At a statically typed throw, WPO can resolve the destination and emit a direct branch. The routing information is distinct from the exception payload; it does not require general RTTI in each exception value. Plain throw propagation from a catch-all must preserve the arriving exception identity in the propagation context; explicit rethrow instead constructs the selected new primary exception. Merely retaining the handler's accepted identity cannot distinguish otherwise identical incoming values.

The user's H-268 suggestion motivates this separation. A runtime handler-registration table is not required by the source semantics or selected merely by the analogy; compiler-local handler scopes and direct LLVM edges are the first implementation route. Explicit-cause lifetime remains a correctness obligation: no dangling catch-local references, silent truncation or allocation-dependent basic OOM delivery.
