# CoD booter: a compiler application written in CoD

Roadmap revision: 7 · Recorded: 2026-11-18 · Phase: application and repository planning.

## Design authority

- **Next-stage roadmap.** [H-TOOLCHAIN-REPOSITORIES-20261118](../../CHANGELOG.md#h-toolchain-repositories-20261118) records the four proposed repository responsibilities and a future C implementation of native services. [D-20-REPOSITORIES](../../design/DesignAuthorityRegistry.md#d-20-repositories), r7, covers [repository and native-runtime planning](#next-stage-repositories-and-native-runtime). C-80/P-314–P-318 and charter §§4.3/6.3 connect a self-hosted compiler to explicit native services. Compare seed-built and self-built behavior and retain owner, relocation and cleanup semantics across the C boundary. Repository creation, bootstrap stages, toolchain releases, native ABI and implementation remain future work.

- **Output-contract continuity:** [H-571](../../CHANGELOG.md#h-571)/[K-OUTPUT-ACCEPT-20261117](../../CHANGELOG.md#k-output-accept-20261117) covers document-set r4→5. D-22’s selected init declared-place roles replace a separate out spelling; native handle storage, target-initializing helper syntax, ownership and provider proof remain pending. Preserve C-81/C-65, charter §§4.3/6.3/8/12, D-16 and existing D-17 call/block timing. No application implementation, physical ABI or broader qualification follows; pipeline semantics are unchanged.

- **Registry / scope:** [D-20](../../design/DesignAuthorityRegistry.md#d-20), roadmap revision 7. The pipeline and LLVM interop documents retain their own scoped revisions. Detailed application designs are `pending-human`; the selected defer call/block rule is governed by D-17, not by a broader implementation qualification of this set.
- **Human baseline:** [H-353](../../CHANGELOG.md#h-353) requests a parallel compiler application, CoD-native design, regional stage caches and incremental LLVM bindings, with documentation only at this stage. [H-354](../../CHANGELOG.md#h-354) identifies output-initialization and uninitialized-handle storage requirements; H-571 supplies the current init language roles, while handle classification remains pending. [H-355](../../CHANGELOG.md#h-355) supplies registration-time terminology. [H-356](../../CHANGELOG.md#h-356) selects call/block defer timing and immediate language implementation; [K-515](../../CHANGELOG.md#k-515) records this application-plan update.
- **Value:** C-80/P-314–P-318 connects inspectable compiler semantics to C-01, reusable structured artifacts to C-05, and native integration to C-02/C-11/C-79. The charter's §§2, 5.3–5.4, 6–7 and 12 provide the context; detailed derivation and trade-offs remain pending.
- **Boundaries / acceptance:** preserve regional relocation, owner/access checks, independent module worlds and explicit TLS effects. Keep confirmed direction, proposals and missing evidence distinguishable. D-17 supplies the separately authorized defer work. No compiler-application source port, native ABI, toolchain/bootstrap operation or performance claim is authorized by this plan.

## Purpose

Use compiler construction as a second application alongside [steady-snn](../steady-snn/docs/README.md). Progressively write CoD's tokenizer, parser, analyzer and compilation pipeline in CoD, selecting and binding LLVM APIs as real compiler operations require them. The eventual objective is a CoD compiler capable of compiling its own CoD implementation, with native services remaining at explicit boundaries.

The unit of design is the compiler operation and its data, not a C++ class translated into CoD. Existing C++ code can supply the seed compiler, implementation evidence and a behavioral comparison. Its container choices, frontend/backend coupling and internal helper structure do not prescribe the new application's architecture. The language contracts remain authoritative when an existing implementation disagrees.

This application should expose useful language opportunities through real work: typed source positions and indices, ADT-based syntax and diagnostics, identity-based module information, regional intermediate artifacts, owner-checked operations and scoped native resources. A feature needs a concrete compiler use and a demonstrated reduction in complexity or cost; a compiler pilot is not a reason to add a general framework in advance.

## Next-stage repositories and native runtime

| Planned repository | Responsibility |
| --- | --- |
| `cod-seed` | The seed compiler in C++. CoD's native components, including the region core and LLVM bindings, are planned in C. |
| `cod` | The CoD compiler written in CoD. The seed builds the initial compiler, which can then compile its own sources. |
| `codup` | Toolchain version management in the role of rustup. CoD package dependency resolution and lockfiles have their own [codp design](../../design/CodProjectStructureAndPkgMgmt.md). |
| `codmate` | The Language Server Protocol server and IDE plugins. |

The proposed split gives the seed and the self-hosted compiler a common native region service. When designing its C interface, distinguish fixed logical capacity, host address-space reservation, backing enabled on demand, and resident pages. Choose suitable host VM operations behind that interface. Preserve owner identity and locators while enabling pages, and retain allocation/release symmetry through partial failure. The C runtime must release owned exception-array members as well as the aggregate region. The native provider should support the [exception capacity rules](../../design/ExceptionControl.md#image-setting-for-exception-region-capacity), including site budgets if their syntax is adopted. Measure virtual address space and resident use separately on target systems. The current C++ allocator is implementation evidence; its API need not constrain the C design.

Deferred work includes specifying the region and LLVM binding interfaces the two compilers actually need, checking that both toolchains agree on those interfaces, and choosing toolchain distribution and editor integration details when those repositories are established. The plan leaves repository creation, source migration and the current bootstrap independent of this design record.

## Document map

| Document | Responsibility |
| --- | --- |
| [Compiler artifacts and pipeline](docs/CompilerPipeline.md) | Stage boundaries, regional representations, DBMR reuse, module worlds and compilation scale. |
| [LLVM bindings and native resources](docs/LLVMInterop.md) | Selective bindings, opaque handles, owner-aware initialization and registration-time evaluation within `defer`. |

Reusable language decisions belong in the relevant `design/` contracts after discussion. These application documents describe requirements and candidate uses without silently extending those contracts. The application name does not refer to the repository's existing native toolchain bootstrap scripts.

## Proposed progression

The immediate language prerequisite is [D-17's scope/defer implementation](../../design/ExceptionControl.md#immediate-implementation-and-acceptance): `defer init(...)()` evaluates acquisition and saves its returned callable now, then invokes it at scope exit. `defer { ... }` delays the whole block. This work does not need to wait for the application's cache schemas, native-handle representation or native init-place integration.

These are planning checkpoints, not an approved implementation schedule. A checkpoint can advance only when its necessary language/runtime contract is selected and qualified.

| Checkpoint | Useful result | Evidence to seek |
| --- | --- | --- |
| Regional frontend | CoD tokenization and parsing produce inspectable source-linked artifacts. | Correct tokens, syntax and diagnostics on representative CoD modules; measured storage and processing cost. |
| Persistent reuse | Selected stage artifacts save as DBMR and load in a fresh process. | Equivalent observations after relocation; changed inputs or incompatible formats cannot cause stale reuse; separately measured load/validation and recomputation cost. |
| Semantic pipeline | CoD builds module worlds and analyzes demanded calls with explicit identities and context. | Multi-module aliases, methods, owners and effects agree with accepted language contracts; cache reuse preserves the selected world. |
| Native execution | A small selected LLVM binding surface supports verification and executable output. | Generated program behavior and exceptional native-resource cleanup; frontend and LLVM time/memory reported separately. |
| Self-hosting | The CoD compiler builds its CoD implementation through the selected native boundary. | Seed-built and self-built compilers agree on relevant compilation outcomes; repeatability, diagnostics and large-program costs assessed. |

Full LLVM coverage, byte-identical compiler binaries, independent module compilation, distributed builds and an immediate replacement of the seed compiler are not checkpoint requirements. Exact acceptance corpora and supported language scope remain to be selected. Large multi-module programs and deep dependency graphs matter alongside small examples; a demonstration alone is not self-hosting or performance acceptance.

## Decisions to revisit before implementation

| Topic | Planning position | Still needed |
| --- | --- | --- |
| Stage caches | Preserve suitable regional results directly as DBMR. | Stage-specific dependency/compatibility rules, publication behavior and measured payoff. |
| LLVM surface | Bind APIs as compiler operations need them. | First end-to-end operation, provider boundary, typed ownership/error signatures. |
| Native handles | Investigate pointer-backed opaque CoD handles with scoped release. | Storage admissibility, native dependency lifetimes, release authority and invalidation. |
| Initialization | Use D-22’s explicit init-place contract and checked actual owner/access. | Select the handle’s storage category and allocation/declaration form; prove initialization and release obligations on all completion paths. |
| One-line cleanup | Use `defer init(...)()` under D-17's selected call/block timing. | Native cleanup capture/release contract, leak-free registration failure and actual binding evidence. |

No implementation, cache artifact, LLVM binding or runtime validation is supplied by this planning document set.
