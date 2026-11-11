# Structural regions and regional closures: experimental comparison

Design revision: 1 · Recorded: 2026-11-10 · Status: authorized experimental scope; final language selection pending.

## Design authority

- Registry: [D-18](DesignAuthorityRegistry.md#d-18), revision 1, all sections; [H-324](DesignAuthorityRegistry.md#h-324)/[K-447](DesignAuthorityRegistry.md#k-447).
- Value path: D-18/P-293 to C-73/C-74 explores practical, explicit storage and lifetime contracts serving charter §§1–3 through C-65/C-39. H-324 explicitly authorizes autonomous alternative design, branch implementation and empirical comparison within repository principles; this does not approve every experimental mechanism as the final language.
- Dependencies: D-14 owners/field kinds, D-13 storage policy, D-10 callables/effects, D-17 failures/scopes and D-16 fixed bits/future fractions. D-01 relocation/allocator invariants and D-15 module-world/TLS separation remain constraints.
- Non-goals: new RTTI, bootstrap/toolchain work, SNN learning/model selection, per-object ownership, hidden heap closures, cross-region stored pointers, copying/moving regional objects or merging a final design without review.
- Acceptance: native examples and meaningful rejection cases for at least two alternatives, full applicable language regression checks, targeted lifetime/relocation evidence, compact reproducible artifacts and a comparison of practical source use, complexity and limitations. Stop for human review at or before 48 active work hours; exclude waiting, with a conservative allowance for unmeasured baseline preparation.

## Baseline and branch roles

Baseline commit: `96468d9`. The compiler and 338 source hashes match the [colon/nullable acceptance](../tests/cod/language/colon-nullable-validation.json), including 254/254 language cases and native regional, exception, ownership and composition checks.

| Branch | Experimental question |
| --- | --- |
| `codex/region-closure-common` | Can field-subset region requirements preserve actual layouts, owners and existing nominal field identities with a small compiler model? |
| `codex/region-closure-owners` | How far can closures go by retaining explicit region access and using field paths, without capturing ordinary lexical binding cells? |
| `codex/region-closure-slots` | Does explicit selection of regional variable slots materially improve source ergonomics enough to justify its additional place/capture semantics? |

Branches are developed sequentially in the existing SSD checkout using its active build tree. Each compiler is built with `cmake --build ./build`; no configure, clean, build symlink, new bootstrap or NFS copying is part of the experiment. Shared foundations may be committed before alternatives diverge. Alternative branches are evidence, not simultaneous backends or permanent compatibility modes.

## Common contract to exercise

A region definition describes a product of named val/ref/ptr fields. A consumer's requirement is a compatible field subset, not a byte prefix or a different owner. Concrete field layouts remain available to WPO. Nominal data/UoM identities inside fields remain exact. A region instance remains distinct from every other instance satisfying the same shape.

A regional val slot contains bits. Regional ref/ptr slots hold same-region references; capturing such a slot observes later rebinding. Capturing a target value is different. Null does not erase the regional slot's known owner. Readability, access, initialization and checked-NPE obligations remain in force. Captures borrow access without extending lifetime; automatic lexical-cell boxing/promotion is excluded.

Owner-based closures may retain an actual region and project its fields at each use. Slot-based closures explicitly identify stable regional storage, carrying the same owner proof. Captured environment representation must follow ordinary storage/lifetime rules; no implicit host-heap allocation is permitted. The first useful WPO subset may keep callable identity and captures in compiler metadata, but any restrictions on returning, selecting, storing or reusing callbacks must be tested and reported, not hidden by passing a single inline example.

Functions retain their explicit-memory/effect boundary. Procedure lexical state is separate from perform's captured dynamic values and behave/direct calls' current bindings. New syntax must respect single-colon annotations, reserved literals/keywords, assignment versus transfer, and planned fractions. The confirmed double-brace scope boundary belongs to any scope/callback work that actually requires it; unrelated syntax changes are not a reason to expand this experiment.

## Evidence and comparison

Use a small shared matrix: two concrete layouts satisfying one subset, mutation through a helper, val and ref/ptr slot updates observed by a callback, null before/after rebinding, nested/cross-module calls, valid external-owner return, invalid ended/sent owner, lexical-cell rejection and read-only rejection. Add a repeated A/B state-selection or typed-pool example to expose an actual usability difference; do not tune SNN transition equations.

Measure changed compiler logic, semantic cases/special paths, LLVM verification and representative native behavior. Retain exact compiler/source identities with results. Compare emitted code or timing only where it answers a practical design question; no micro-optimization target is implied. Negative findings and unimplemented cases are part of the result.

Progress and active-time checkpoints are recorded in the exploration evidence directory. Final review must distinguish measured active work from waiting and estimates, name each branch/commit, and identify which decisions the user would need to adopt. A promising result does not authorize merging its language contract into main.

## Shared structural foundation evidence

The [structural validation](../tests/cod/language/exploration/structural-validation.json) records 259/259 passing language checks, including different concrete layouts used through one cross-module field requirement, owner inference, readonly rejection, field-kind/type rejection and unique-most-specific versus ambiguous shape dispatch. Compilation uses the existing native LLVM path and unchanged regional runtime.

Requirements act as lower bounds; WPO retains the concrete layout and type for nested dispatch. They do not currently hide extra fields from a demanded helper body. Treating a requirement as a restricted access view is a separate comparison dimension, not a runtime layout cast. Existing data-root syntax remains for baseline comparison, and this checkpoint provides no closure implementation evidence.
