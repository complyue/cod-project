# Compiler modes, selectable stages and persistent stage reuse

Revision 9 · 2026-11-26 · Compiler modes and stage controls confirmed. Persistent artifact design remains a planning draft.

## Design authority

At r9 the [production CLI decision](../../../CHANGELOG.md#h-cfe-output-directory-20261126) specifies the `cfe` command and its output destination under [D-20-COMPILER-MODES](../../../design/DesignAuthorityRegistry.md#d-20-compiler-modes), C-80/P-746/P-747. The release milestones and execution behavior carry forward from r8.

At r8 [D-20-COMPILER-MODES](../../../design/DesignAuthorityRegistry.md#d-20-compiler-modes) covers [default execution and optimized builds](#default-execution-and-optimized-builds). The [project user’s decision](../../../CHANGELOG.md#h-default-run-and-optimized-build-20261125) sets the compilation and runtime trade-off and release scope through C-80/P-746/P-747. It continues r7’s independent stage controls and leaves backend optimization choices open.

[**D-20-STAGE-CONTROLS**](../../../design/DesignAuthorityRegistry.md#d-20-stage-controls) covers [independently selectable compiler operations](#independently-selectable-compiler-operations) at r7. The [project user’s decision](../../../CHANGELOG.md#h-compiler-pipeline-replacement-20261125) derives this scope from C-80 and charter §§5.3 and 9, through P-744/P-745. The same compiler results serve editor feedback and build requests. Their revision, dependencies and validation status must remain identifiable. The r6 proposals for persistence, representation and cache policy continue with their original qualification.

- **Output-contract alignment:** [H-601](../../../CHANGELOG.md#h-601)/[H-602](../../../CHANGELOG.md#h-602)/[K-INIT-PLACE-CORRECTION-20261117](../../../CHANGELOG.md#k-init-place-correction-20261117) covers r5→6. D-22 keeps ordinary init reference semantics and explicit var for caller-slot declared place aliasing; no target modifier is introduced. Native handle storage, ownership and provider proof remain pending. Preserve C-81/C-65, charter §§4.3/6.3/8/12, D-16 and D-17 timing/cleanup. Only the dependency/initialization interpretation changes; pipeline behavior and full application/native/value-path qualification remain unchanged.

- **Artifact and persistence scope.** [D-20](../../../design/DesignAuthorityRegistry.md#d-20) carries the r6 proposals into r7. Detailed representation, caching and validation of persistent artifacts remain `pending-human`.
- **Baseline / assessment:** [H-353](../../../CHANGELOG.md#h-353) selects a CoD-native compiler pilot and investigation of DBMR stage caches. [K-473](../../../CHANGELOG.md#k-473) records the draft; [K-515](../../../CHANGELOG.md#k-515) preserves this pipeline scope while the set adopts H-356's separately authorized defer prerequisite. C-80/P-314/P-315/P-317 links typed compiler intent and reusable structured results to C-01/C-05 and charter §§2, 5.3–5.4, 7 and 12.
- **Dependencies / invariants:** P-316/P-318, D-01 regional representation, D-14 owners, D-15 independent module worlds and D-16 numerical/syntax compatibility. Preserve identities, selected interpretations and relocation; no native addresses in persistent artifacts and no cross-module world editing.
- **Acceptance / exclusions:** future cached and uncached paths must preserve applicable semantic observations and reject stale/incompatible reuse, with actual time/memory measurements. No cache schema, partition algorithm, checksum, file API or implementation is selected here.

<a id="default-execution-and-optimized-builds"></a>

## Running with `cod` and building with `cfe`

| Mode | User-visible result on a successful invocation | Compilation goal and availability |
| --- | --- | --- |
| Default `cod` invocation on a program entry | Compile and run it immediately, as with `go run`. Leave no executable as a user output. | Aim for Go-like fast compilation and machine code with moderate runtime performance in the seed, booter and first release of `e-wrks/cod`. |
| Default seed bootstrap build | Produce the `cod-booter` executable and keep it for subsequent invocations. | Use the fast compilation path. The seed build can request intermediate bitcode and provider files and link them into the tool. |
| `cfe` (Compile for Ever) in `e-wrks/cod` | Build an optimized executable for later use. Save it in `~/.local/bin/` by default. The user can set the output directory in `~/.cod/config.yaml`. | Spend more compile time on high-quality machine code. Experimental in the first release of `e-wrks/cod`, formally supported in 2.0. |

The ordinary edit–run cycle gives priority to the time from invoking the compiler to entering the program. Any executable needed internally for a run is a compiler implementation detail. Explicit requests for a binary, object, bitcode or provider files have their own output contract. The seed uses the fast path to build its persistent executable. A parse-only or check-only request ends at its selected operation.

Programs built with `cfe` preserve the observable semantics and applicable runtime checks of ordinary `cod` execution. For acceptance, use an entry that prints a recognizable value. An ordinary `cod` invocation prints it once and leaves no new or replaced user executable. Verify the seed default separately by invoking the resulting `cod-booter` in a subsequent process. Run `cfe` with the output directory unset and verify the executable appears in `~/.local/bin/`. Set the output directory to another location in `~/.cod/config.yaml` and build again. Verify the new executable appears in the configured directory. In both cases the build leaves the entry unexecuted. Launching either executable then prints the same value. A compilation failure prevents launch. Go-like compilation speed and moderate runtime performance are qualitative design targets. Backend settings and numeric targets remain open.

## Independently selectable compiler operations

A client can request parsing, type checking, lexical-access checking, ownership checking and emission separately. The replacement C++ seed should make these operations independently selectable wherever practical. The CoD booter should adopt the separations proved useful in the seed. The future `e-wrks/cod` compiler **must** provide independent controls to build clients and its LSP/IDE clients. Users choose the work they want for their available compute and desired feedback latency. Specific option spellings, defaults for editor profiles and a persistence format are separate implementation choices.

Each operation declares its required inputs and delivers its own result. For example, resolving the selected type and layout of an operand can be necessary to emit LLVM even with type diagnostics disabled. Emission still prepares its actual owner, cleanup and ABI operands. Work exclusive to a disabled check stays off. With parsing disabled, a downstream operation can consume a matching completed syntax result. If an input is missing or stale, identify the unavailable prerequisite. A request must not silently enable a disabled stage. A skipped check supplies no successful validation claim. Publishing an artifact as checked requires the evidence prescribed for that artifact.

For production acceptance, exercise parse-only feedback with damaged source, individual check selections without emission, and a build using completed inputs with parsing off. Enable ownership checking on the same revision and observe its diagnostics while reusing the available inputs. After an edit, refresh affected results using their recorded dependencies and preserve results whose inputs remain valid. Diagnostics belong to the revision and the checks that actually ran. On a valid input, selecting extra diagnostics preserves emitted program behavior. A stale syntax input with parsing off must be distinguishable from a successful check. These cases concern the compiler interface used by both the editor and the build. They require no particular storage format.

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

The [snapshot direction](../../../design/DomainLanguageDesign.md#dl-09) places file save/replacement support in the language/runtime and cache policy in the application. The initial manually assigned root-type compatibility identifier and later layout/checksum investigation remain the [D-07/H-352](../../../CHANGELOG.md#h-352) direction. A matching layout or root marker is not proof that source dependencies or selected methods are unchanged. A checksum can assist matching without becoming a new type identity or a proof of semantic validity.

Keep temporary analysis state distinct from results suitable for publication. Native LLVM contexts, modules, builders, pointer-backed handles and callable environments are not DBMR compiler artifacts. LLVM bitcode is a native serialization format, not an in-memory `LLVMModule` snapshot.

Start by evaluating coarse artifact boundaries. Add finer invalidation, deduplication, background rebuilding or dependency indices only when observed recomputation and storage costs justify them. Recomputable compiler caches must be distinguishable from authoritative source, annotations or other user-maintained semantic content before any eviction policy is designed.

Before a persistence checkpoint can be implemented, specify the required capture consistency, behavior when saving/replacing fails, and how incomplete or incompatible files are reported or treated as explicit cache misses. DBMR mapping alone does not supply transactions, durable replacement or crash recovery.

## Compilation scale and evidence

Demand-driven analysis and one selected semantic program do not require one indivisible LLVM optimization unit. Preserve useful callable boundaries and recursive cycles; do not duplicate a dynamic call tree into source expansions as a long-term architecture. The [selected mode](#default-execution-and-optimized-builds) sets the compile-time budget relative to runtime code quality. Physical LLVM partitioning and the choice of optimization passes remain implementation decisions.

Measure tokenization/parsing, semantic analysis, artifact I/O/validation and native optimization separately. Useful later comparisons include cold versus warm compilation, a leaf-module edit, different adapter selections over the same definitions, and shared/deep dependency graphs. Report peak memory, artifact size and the amount of actual reused work alongside elapsed time. Preserve diagnostics and ownership/effect rejection on both cached and uncached paths.

[D-16](../../../design/FixedWidthFractions.md) remains a compatibility obligation: reserve its established comment/token conventions, preserve numeric representations and UoM identities in artifacts, and do not use this pilot to choose deferred fraction grammar, promotion, layout or formatting semantics.
