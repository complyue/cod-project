# Compiler artifacts and persistent stage reuse

Revision: 4 · Recorded: 2026-11-12 · Status: planning draft.

## Design authority

- **Registry / coverage:** [D-20](../../../design/DesignAuthorityRegistry.md#d-20), revision 4, all sections; detailed representation, caching and validation contracts are `pending-human`.
- **Baseline / assessment:** [H-353](../../../design/DesignAuthorityRegistry.md#h-353) selects a CoD-native compiler pilot and investigation of DBMR stage caches. [K-473](../../../design/DesignAuthorityRegistry.md#k-473) records the draft; [K-515](../../../design/DesignAuthorityRegistry.md#k-515) preserves this pipeline scope while the set adopts H-356's separately authorized defer prerequisite. C-80/P-314/P-315/P-317 links typed compiler intent and reusable structured results to C-01/C-05 and charter §§2, 5.3–5.4, 7 and 12.
- **Dependencies / invariants:** P-316/P-318, D-01 regional representation, D-14 owners, D-15 independent module worlds and D-16 numerical/syntax compatibility. Preserve identities, selected interpretations and relocation; no native addresses in persistent artifacts and no cross-module world editing.
- **Acceptance / exclusions:** future cached and uncached paths must preserve applicable semantic observations and reject stale/incompatible reuse, with actual time/memory measurements. No cache schema, partition algorithm, checksum, file API or implementation is selected here.

## Design from compiler work

A lexer classifies source spans; a parser constructs typed syntax; an analyzer establishes identities, resolves selected definitions and checks demanded operations. Represent those results in CoD's own data and methods. Keep LLVM objects at native emission boundaries rather than putting LLVM pointers into syntax or semantic records.

Useful candidates should be evaluated against a direct implementation:

| Compiler need | CoD capability to examine | Expected value and limit |
| --- | --- | --- |
| Source positions, token indices and ranges | Distinct typed indices and available UoM arithmetic | Prevent mixing address spaces without choosing an elaborate coordinate framework. |
| Syntax alternatives and diagnostics | ADTs with explicit methods and source associations | Make cases and error information inspectable; do not preselect a full GADT encoding. |
| Reusable stage results | Regions saved and loaded as DBMR | Avoid mandatory serialization into a second object graph and reconstruction on every load. |
| Definition and method selection | Identity-centered module-world data | Preserve aliases and receiving-world interpretations without a shared mutable method table. |
| Temporary compilation services | Explicit inputs or typed dynamic effects where appropriate | Make service scope clear; do not turn all compiler state into TLS globals. |
| Artifact reconstruction/compaction | Selected library `(<-)` graph-transfer methods | Rebuild the retained graph under its actual owner/layout rules instead of demanding one universal graph copier. |

These are opportunities, not a list of new required abstractions. If current language facilities suffice, use them. Keep ordinary compiler algorithms in CoD libraries and reserve intrinsic/provider work for capabilities that genuinely cross the language/runtime boundary.

## Candidate artifact boundaries

The table describes information that may be reusable; it does not require one file or region per row.

| Stage | Candidate saved information | Inputs that may determine reuse |
| --- | --- | --- |
| Source / tokens | Source identity/content association, token kind/value and source ranges | Source contents, lexical rules, token representation and required source lifetime. |
| Syntax | AST, declaration structure and source/diagnostic associations | Token/source revision, grammar and involved layouts. |
| Module information | Identities, exported aliases, declarations and selected method/conformance information | Module contents, selected dependency exports/versions, language rules. |
| Demanded analysis | Checked operations or specializations with their required context | Concrete types, selected module worlds/methods/conformances, relevant effects and owner/access assumptions. |
| Native artifacts | LLVM bitcode or object output through their own formats | Analyzed semantics, target/ABI, provider/toolchain version and compilation options. |

WPO determines what a complete program demands and whether calls are valid. It does not require recomputing every stage on every invocation. A saved module artifact is not an independently compiled executable module. Context-dependent checks must not be reused merely because the source text matches.

Module worlds and TLS retain their distinct roles from [D-15](../../../design/ModuleWorlds.md): each module constructs its own information; TLS holds the runtime typed symbolic bindings whose flattened shape is established at compilation. Loading an artifact does not edit another module's world or restore an arbitrary prior thread's bindings. Transient owner instances and native addresses cannot serve as durable semantic identities.

## DBMR reuse boundary

The intended benefit is direct access to a valid regional representation after loading, without mandatory per-node reconstruction or fixup of intra-region pointers. Loading, I/O and validation still cost time. Source buffers, dependencies and any cross-artifact references need valid lifetime/identity arrangements; this plan does not select a new cross-region pointer scheme.

The [snapshot direction](../../../design/DomainLanguageDesign.md#dl-09) places file save/replacement support in the language/runtime and cache policy in the application. The initial manually assigned root-type compatibility identifier and later layout/checksum investigation remain the [D-07/H-352](../../../design/DesignAuthorityRegistry.md#h-352) direction. A matching layout or root marker is not proof that source dependencies or selected methods are unchanged. A checksum can assist matching without becoming a new type identity or a proof of semantic validity.

Keep temporary analysis state distinct from results suitable for publication. Native LLVM contexts, modules, builders, pointer-backed handles and callable environments are not DBMR compiler artifacts. LLVM bitcode is a native serialization format, not an in-memory `LLVMModule` snapshot.

Start by evaluating coarse artifact boundaries. Add finer invalidation, deduplication, background rebuilding or dependency indices only when observed recomputation and storage costs justify them. Recomputable compiler caches must be distinguishable from authoritative source, annotations or other user-maintained semantic content before any eviction policy is designed.

Before a persistence checkpoint can be implemented, specify the required capture consistency, behavior when saving/replacing fails, and how incomplete or incompatible files are reported or treated as explicit cache misses. DBMR mapping alone does not supply transactions, durable replacement or crash recovery.

## Compilation scale and evidence

Demand-driven analysis and one selected semantic program do not require one indivisible LLVM optimization unit. Preserve useful callable boundaries and recursive cycles; do not duplicate a dynamic call tree into source expansions as a long-term architecture. Physical LLVM partitioning and optimization policy need measurement and a separate decision, not a second backend.

Measure tokenization/parsing, semantic analysis, artifact I/O/validation and native optimization separately. Useful later comparisons include cold versus warm compilation, a leaf-module edit, different adapter selections over the same definitions, and shared/deep dependency graphs. Report peak memory, artifact size and the amount of actual reused work alongside elapsed time. Preserve diagnostics and ownership/effect rejection on both cached and uncached paths.

[D-16](../../../design/FixedWidthFractions.md) remains a compatibility obligation: reserve its established comment/token conventions, preserve numeric representations and UoM identities in artifacts, and do not use this pilot to choose deferred fraction grammar, promotion, layout or formatting semantics.
