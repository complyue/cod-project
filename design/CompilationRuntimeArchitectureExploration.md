# CoD execution architecture: AOT micro-dispatch assessment

Design revision: 15 · Recorded: 2026-11-13 America/Los_Angeles · Status: bounded experimental evidence and removal-first source migration; concrete full-language mechanisms remain separately scoped.

## Design authority

- **Registry / current scope:** [D-25](DesignAuthorityRegistry.md#d-25); the semantic revision stated above and the independently qualified scopes in the current registry. **Isolated delivery authorized** H-396/H-397; **source migration authorized** H-398/H-399; K-586/K-587/K-593/K-594/K-596. C-85/P-359–P-362 and C-86/C-87/C-88/P-363–P-370 detailed production paths remain pending-human.
- **Value path:** The registry’s named value relationships explain the charter benefit and retain their individual confirmation states; dependency relationships confer no additional authority.
- **Contract boundary:** Prior package preserved at 25ccc75 without final selection; new branch starts at prior main 8ff9576. No assumed checker/IR/backend/fallback, BBV or JIT requirement. Native experiment evidence covers only its finite explicit contract. Source migration begins at local multi-dispatch tag 2ab762d. H-399/H-408 permits a useful removal-first foundation with documented gaps; H-407 confirms D-26 coverage responsibility. H-409 clarifies specialization and independent local selections. No new budget or permanent ABI is authorized. Existing operational limits remain.
- **Acceptance:** The bounded experiment and source migration each satisfy their stated evidence and acceptance limits; unresolved physical ABI, type-system and effect choices remain separately reviewed. Current compiler availability is recorded in the [source capability reference](../tests/cod/language/README.md).
- **Evidence / continuity:** human baselines are the H records identified by the registry; [K-632](../CHANGELOG.md#k-632) covers this editorial consolidation without changing qualification or semantic scope.

## Decision and scope

The execution design separates **AOT execution preparation from runtime execution**, and distinguishes semantic method selection, implementation selection and local micro-dispatch within a body. Compilation prepares general/specialized implementations and selectors; proven facts may resolve choices and permit direct-call emission. Runtime executes the emitted calls and any remaining prepared selections using actual evidence. Preparing micro-dispatch code does not execute a local selection or select a user method. One semantic method can have multiple compiler-registered implementations, with a correct general path available without making specialization a correctness prerequisite. Multiple-argument unique-most-specific selection remains a language behavior. [D-21](AheadOfTimeExecutionPreparation.md) owns this execution-preparation contract.

The finite native evidence supports this baseline for explicitly selected nominal definitions, field choices and operation contracts. It exposes no major correctness or compilation-termination obstacle within that scope. It does not establish an arbitrary generic/type-inference solver, full source language, complete owner checker or production runtime ABI. Those are implementation/design obligations to carry forward, not capabilities supplied by the older compiler.

The source contract also admits valid runtime values whose actual types are outside the static catalog. Such values select through CoD `<any>` while preserving actual identity and owner evidence. Uninterpreted passage is permitted; unavailable required interpretation raises checked `StaticallyUnknownType`, and a breach of established typing raises unchecked `UnsoundRuntimeTyping`. The prepared executable methods and selected interpretations remain fixed. This boundary comes from D-26 and is not established by the finite experiment's decoder or reports. New method loading, runtime compilation and unvalidated opaque payload access remain excluded.

The experiment is self-contained within its stated finite contract; its workload planning and utilities supply no prerequisite compiler architecture or production ABI.

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
- The experiment's symbolic finite coverage avoids explicit tuple enumeration but can still grow; its exhausted work allowance rejects with a concrete limit diagnostic. Source-level generic/recursive analysis follows D-26's shared native-coverage responsibility. Neither type growth nor a resource stop proves invalidity or inevitable divergence, and no universal noncompletion detector is required.
- Generalized carriers, bits/borrowed results, mutable alias optimization, complete captures and checked native interfaces need production contracts. The baseline does not rely on solving them through speculative BBV or a hidden JIT.
- Entry-installation suppression preserves operand effects and procedure-local bindings in the fixture. Full conditional composition, source-world roots and final-image elimination are unimplemented obligations.
- Concrete discriminators preserve needed distinctions, and the fixture verifies persisted meanings across two independently prepared worlds, but general identity allocation and artifact/version compatibility remain unresolved. Root checksums alone cannot establish compatibility.

These limits do not defeat the explicitly delivered finite baseline. They prevent claiming full-language implementation or silently using this fixture as a production checker. A future discovered counterexample should narrow or revise that claim rather than be hidden behind optional specialization.

## Merge gate and next work

The user has authorized merge into main if the bounded scope passes the package gate; no repeated merge permission is needed. Unsupported physical ABI/type-system/effect decisions stay pending even if the scoped design/prototype is merged.

Source migration develops a checked abstract method/operation interface for a real recursive CoD source-to-native workload under D-26's confirmed coverage contract, carrying actual owners, concrete evidence, result and required context contracts into a thin emitter. Prioritize usable coherent source paths and inspectable evidence; stronger generic analysis and missing capabilities can develop alongside them. Routine implementation choices within confirmed scope do not require repeated confirmation. New language behavior or material unselected representation boundaries still require their scoped review. Keep the general path correct while supporting call-site-driven specialization; do not force known calls through runtime selection or turn local operation branches into whole-body versions.

Preserve method/callable/source associations for future profiling. ORC is an option only for future tracing-JIT hot paths; it is absent from the baseline and introduces no current dependency. No profiler, trace compiler, OSR/deoptimization, benchmark framework or engine installation is required by this assessment.
