# CoD execution architecture: AOT micro-dispatch assessment

Design revision: 12 · Recorded: 2026-11-13 America/Los_Angeles · Status: validated bounded AOT micro-dispatch implementation and design replacement; full-language mechanisms remain unqualified.

## Design authority

**Source migration task:** [H-398](DesignAuthorityRegistry.md#h-398)/[K-586](DesignAuthorityRegistry.md#k-586) authorizes autonomous work from baseline `2ab762d617b742f4fba7c2c617bacd303739ccef`, preserved by the local annotated `multi-dispatch` tag. It includes the isolated result below, while its source compiler still requires migration. [The closure proposal](GenericAndRecursiveClosure.md) is pending whole-file review; [the migration work log](MicroDispatchMigrationWorkLog.md) tracks changing scope and evidence. Task authorization does not approve a new generic rule or permanent ABI. [H-399](DesignAuthorityRegistry.md#h-399)/[K-587](DesignAuthorityRegistry.md#k-587) requires removal of debt-producing historical machinery first. Current stage acceptance is a coherent foundation with honest capability gaps, not feature parity or a fully passing legacy suite.

[D-25](DesignAuthorityRegistry.md#d-25), all sections, and the supporting [micro-dispatch package](../experiments/micro-dispatch/README.md). [H-396](DesignAuthorityRegistry.md#h-396)/[H-397](DesignAuthorityRegistry.md#h-397) authorizes the new work package revision 5, actual replacement of D-21's architecture, independent-branch execution and conditional merge. [K-585](DesignAuthorityRegistry.md#k-585) records alignment. The current project user supplied the instructions in [the source side chat](codex://threads/01a1d51f-3ef4-7101-a86c-94666675f5b6); precise message timestamps and personal identity are unavailable. Production mechanism/value-path qualification remains scoped, not inferred from a merge or successful prototype.

Candidate value path: C-85/P-359 → C-01/C-04 and D-25/P-360 → C-85/C-02, with C-86/C-87/C-88 and P-363–P-370 connecting shared native methods and concrete field evidence to preserved business meaning and affordable execution. Charter §§1–3, 4.3, 5.3–5.4, 6.3, 8 and 12 governs. H-378's D-21 behavioral path remains qualified; detailed candidate paths are pending whole-file review. D-01/D-07/D-10/D-11/D-14/D-15/D-16/D-17/D-18/D-21 constrain owners, conformance, complete values, selected interpretations, effects and completion. Dependency edges grant no additional authority.

Acceptance is a complete explained AOT route for the delivered subset, generated native method/local-operation execution with specialization disabled, discriminating observations, honest limits, coherent replacement documents and the authorized merge-gate audit. There is no assumed existing checker, IR, native backend or fallback. A full CoD compiler, permanent discriminator/ABI, arbitrary generic closure, ORC/tracing/OSR/deoptimization, engine installation, performance campaign and bootstrap are excluded. The existing 48-active-hour checkpoint is shared with preceding work; no new allowance is inferred.

## Decision and scope

The candidate is **AOT method selection, one general native implementation per method in its required semantic context, and local micro-dispatch where an operation needs concrete information**. Optional specialization can improve it but is not a correctness prerequisite. Multiple-argument unique-most-specific selection remains a language behavior. The [replacement D-21](AheadOfTimeMultipleDispatch.md) owns the execution path and preserved contract.

The finite native evidence supports this baseline for explicitly selected nominal definitions, field choices and operation contracts. It exposes no major correctness or compilation-termination obstacle within that scope. It does not establish an arbitrary generic/type-inference solver, full source language, complete owner checker or production runtime ABI. Those are implementation/design obligations to carry forward, not capabilities supplied by the older compiler.

The prior local-dispatch/Static-BBV exploration is preserved on `codex/local-dispatch-exploration` at `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8`. It was not merged as a final architecture decision. This package starts directly from prior main `8ff957610a455d0f242c45108777dd68a77561c6`. Selective reuse of domain planning, dependency analysis and small utilities is attributed in the experiment; its versioner, IR and recommendation are not prerequisites or imported infrastructure.

## Candidate route

| Stage | Delivered mechanism | Limit |
| --- | --- | --- |
| Selected semantic input | Explicit nominal definitions/conformance, field contracts, family methods and two interpretations. Explicit compact concrete-type discriminator mapping; root checksums are separate planning. | Authored fixture, not a CoD parser or general module composer. |
| Checking | Declared finite family coverage, nominal dominance, ambiguity rejection, known field/layout and input contracts. | General inference, native contract proof, ownership and alias-effect analysis remain outside the checker. |
| AOT preparation | Generate family selectors, 13 method bodies and local field/scalar/encoding/update operations; compile before reading runtime fixture data. | No generic type generator or implicit unlimited instantiation. |
| Native execution | Existing Clang/LLVM compiles emitted C++20, with specialization disabled and no runtime instruction interpreter. Regional inputs/results and native resource primitives are real. | Temporary carrier/lowering vehicle, not a permanent ABI or assumed fallback. |
| Context/completion | Complete projected family values, TLS/current/captured selection, actual policy owners, scoped restoration and explicit pending completion/cleanup. | Supplied dependency contracts and finite failure categories; full source effect inference/replay/exception payloads remain open. |

Independent field alternatives are not object variants. Example has separate x/y evidence; an enclosing Holder carries TypeId(Example) only when needed. Its concrete identity does not change when x changes. A local operation that genuinely depends on both field alternatives retains those checks. Code generation depends on reachable methods and relevant operation alternatives, not mandatory whole-method tuple cloning.

## Concrete identity, call-site facts and root metadata

The current user decision uses concrete-type discriminators for abstract/polymorphic values. The experiment carries one explicit stable 64-bit ID bound to the CoD semantic definition; it has no second runtime identity tag. Concrete externally selected conformance does not alter that identity. Static known fields need no tag, and a common-offset scalar read does not discriminate.

At a call site, proven exact or finite-set facts can narrow or eliminate method selection while calling the same general body. The baseline runs with these simplifications disabled. A control build proves an exact Meters argument and a two-alternative scalar set with one common target, then bypasses those selectors without adding bodies. Definition-site abstract annotations never overwrite actual evidence. Selected family/interpretation remains a separate determinant.

The no-field compiler-opcode methods demonstrate an observable need for some concrete distinctions; they do not justify a tag on every object. Types with a shared access contract share local access code. The existing compiler's identity component is not assumed to be a complete universal runtime encoding; the fixture's injective map is explicit and temporary.

Type checksums now have a separate planned DBMR responsibility: associate relevant root/type version metadata with stored data and help assess compatibility with current fused-world types. They do not dispatch values. An optional named-graph checksum observation handles finite recursive/shared descriptions but is no production algorithm. Adding an unrelated type preserves the chosen stable IDs and root checksum; a naive build-order encoding would not preserve meaning. Equal root checksums can also coexist with an invalid ID binding. Equality is therefore insufficient: stored-code interpretation, identity/schema versions, relevant dependencies, target ABI/file format and ordinary reference validation need separate consideration. This package changes no DBMR format, loader or migration behavior.

## Evidence and obstacles

[Native observations](../experiments/micro-dispatch/observations.json) cover 13 selected cases: independent field choices, a different enclosing layout, crossed method selection, two interpretations, captured/current values and shared actual owners, successful/failed alias updates, acquisition/verifier failures and deferred completion. Separately compiled older-writer/newer-reader programs preserve the results and every saved byte despite added/reordered definitions; a reader with conflicting ID meanings rejects before payload access. Native symbol inspection checks one definition per generated method. Reversed method declarations preserve the full trace. Unknown/mismatched/abstract evidence, duplicate discriminator mappings, missing intersections, insufficient coverage work and missing entry installation reject for their intended reason. One emitted LLVM module executes natively and returns 313.

Real region allocation and relative links preserve readonly input across two mapping bases. Mutable cases use a separate scratch region with actual same-owner links and observe a fresh field load after alias mutation. Output has its own actual region. No regional object is copied/moved; native handles and code addresses stay outside stored graphs. The sequential field update preserves its old pair if preparation fails; concurrency is not claimed.

The main costs and remaining risks are explicit:

- Method selection and repeated local checks remain runtime work. Small finite sets are favorable; larger sets grow comparisons and coverage fragments. There is no measured throughput claim.
- Symbolic finite coverage avoids explicit tuple enumeration but can still grow. An exhausted proof budget rejects as unproved. Full generic/recursive closure needs a separate sound strategy.
- Generalized carriers, bits/borrowed results, mutable alias optimization, complete captures and checked native interfaces need production contracts. The baseline does not rely on solving them through speculative BBV or a hidden JIT.
- Entry-installation suppression preserves operand effects and procedure-local bindings in the fixture. Full conditional composition, source-world roots and final-image elimination are unimplemented obligations.
- Concrete discriminators preserve needed distinctions, and the fixture verifies persisted meanings across two independently prepared worlds, but general identity allocation and artifact/version compatibility remain unresolved. Root checksums alone cannot establish compatibility.

These limits do not defeat the explicitly delivered finite baseline. They prevent claiming full-language implementation or silently using this fixture as a production checker. A future discovered counterexample should narrow or revise that claim rather than be hidden behind optional specialization.

## Merge gate and next work

The [work log](../experiments/micro-dispatch/WORK_LOG.md) records actual candidate validation, branch ancestry, elapsed work and merge state. The user has authorized merge into main if the bounded scope passes the package gate; no repeated merge permission is needed. Unsupported physical ABI/type-system/effect decisions stay pending even if the scoped design/prototype is merged.

Source migration first reviews the generic/recursive-closure contract, then designs a checked abstract method/operation interface for a real recursive CoD source-to-native workload, carrying actual owners, concrete evidence, result and required context contracts into a thin emitter. Prioritize the semantic/ownership interface and generic closure evidence before adding broad language syntax. Keep the unspecialized path correct as optional simplifications are introduced.

Preserve method/callable/source associations for future profiling. ORC is an option only for future tracing-JIT hot paths; it is absent from the baseline and introduces no current dependency. No profiler, trace compiler, OSR/deoptimization, benchmark framework or engine installation is required by this assessment.
