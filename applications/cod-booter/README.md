# CoD booter: a compiler application written in CoD

Document set: revision 4 · Recorded: 2026-11-12 · Phase: application planning; prerequisite defer implementation authorized separately.

## Design authority

- **Registry / scope:** [D-20](../../design/DesignAuthorityRegistry.md#d-20), revision 4, this README and both linked planning documents. Detailed application designs are `pending-human`; the selected defer call/block rule is governed by D-17, not by a broader implementation qualification of this set.
- **Human baseline:** [H-353](../../design/DesignAuthorityRegistry.md#h-353) requests a parallel compiler application, CoD-native design, regional stage caches and incremental LLVM bindings, with documentation only at this stage. [H-354](../../design/DesignAuthorityRegistry.md#h-354) proposes write-only `out` parameters and allocated-but-uninitialized local handle storage. [H-355](../../design/DesignAuthorityRegistry.md#h-355) supplies registration-time terminology. [H-356](../../design/DesignAuthorityRegistry.md#h-356) selects call/block defer timing and immediate language implementation; [K-515](../../design/DesignAuthorityRegistry.md#k-515) records this application-plan update.
- **Value:** C-80/P-314–P-318 connects inspectable compiler semantics to C-01, reusable structured artifacts to C-05, and native integration to C-02/C-11/C-79. The charter's §§2, 5.3–5.4, 6–7 and 12 provide the context; detailed derivation and trade-offs remain pending.
- **Boundaries / acceptance:** preserve regional relocation, owner/access checks, independent module worlds and explicit TLS effects. Keep confirmed direction, proposals and missing evidence distinguishable. D-17 supplies the separately authorized defer work. No compiler-application source port, native ABI, toolchain/bootstrap operation or performance claim is authorized by this plan.

## Purpose

Use compiler construction as a second application alongside [steady-snn](../steady-snn/docs/README.md). Progressively write CoD's tokenizer, parser, analyzer and compilation pipeline in CoD, selecting and binding LLVM APIs as real compiler operations require them. The eventual objective is a CoD compiler capable of compiling its own CoD implementation, with native services remaining at explicit boundaries.

The unit of design is the compiler operation and its data, not a C++ class translated into CoD. Existing C++ code can supply the seed compiler, implementation evidence and a behavioral comparison. Its container choices, frontend/backend coupling and internal helper structure do not prescribe the new application's architecture. The language contracts remain authoritative when an existing implementation disagrees.

This application should expose useful language opportunities through real work: typed source positions and indices, ADT-based syntax and diagnostics, identity-based module information, regional intermediate artifacts, owner-checked operations and scoped native resources. A feature needs a concrete compiler use and a demonstrated reduction in complexity or cost; a compiler pilot is not a reason to add a general framework in advance.

## Document map

| Document | Responsibility |
| --- | --- |
| [Compiler artifacts and pipeline](docs/CompilerPipeline.md) | Stage boundaries, regional representations, DBMR reuse, module worlds and compilation scale. |
| [LLVM bindings and native resources](docs/LLVMInterop.md) | Selective bindings, opaque handles, owner-aware initialization and registration-time evaluation within `defer`. |

Reusable language decisions belong in the relevant `design/` contracts after discussion. These application documents describe requirements and candidate uses without silently extending those contracts. The application name does not refer to the repository's existing native toolchain bootstrap scripts.

## Proposed progression

The immediate language prerequisite is [D-17's scope/defer implementation](../../design/ExceptionControl.md#immediate-implementation-and-acceptance): `defer init(...)()` evaluates acquisition and saves its returned callable now, then invokes it at scope exit. `defer { ... }` delays the whole block. This work does not need to wait for the application's cache schemas, native-handle representation or `out` parameter proposal.

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
| Initialization | Explicit generic-owner reference access, with a proposed write-only `out` modifier; a bare local handle declaration allocates uninitialized storage. | Exact modifier grammar, supported storage categories and proof that every normal return initializes the output. |
| One-line cleanup | Use `defer init(...)()` under D-17's selected call/block timing. | Native cleanup capture/release contract, leak-free registration failure and actual binding evidence. |

No implementation, cache artifact, LLVM binding or runtime validation is supplied by this initial document set.
