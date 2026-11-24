# CoD applications

See the [CoD compiler application roadmap](cod-booter/README.md) and the [steady-snn application notes](steady-snn/docs/README.md) for their respective plans.

## Deferred application: hlvm-basic

Revisit HLVM (High Level Virtual Machine) projects **only after CoD’s basic lexer and parser work together on representative CoD source**. `hlvm-basic` is a candidate application, not a compiler milestone.

The idea comes from the [warehouse policy and its executor](../papers/AlgebraicEffectsIsAllYouNeed.md#two-entries-share-one-fallback). Where the paper uses a warehouse policy, `hlvm-basic` would use a BASIC program: its frontend would resolve line numbers into edges of a typed graph; one BASIC executor would run either the freshly parsed graph or a compatible graph reopened from a published DBMR, without the BASIC source or parser. HLVM would preserve the language-level program; LLVM could later specialize its execution without making machine code the stored program.

A small experiment would apply the paper’s [effect handlers](../papers/AlgebraicEffectsIsAllYouNeed.md#5-why-algebraic-effects-belong-in-this-account) to BASIC’s `PRINT`: a console handler displays each value, while a recording handler captures the values in order. For a loop with `IF`, `GOTO` and `PRINT`, compare the recorded output from source with that from the DBMR in a fresh process. Then try the paper’s [graph edit](../papers/AlgebraicEffectsIsAllYouNeed.md#4-the-program-edit-and-the-business-result): redirect one branch in a draft, publish it, and compare its output with the equivalent source edit. For this text-origin BASIC program, the second edit supplies an independent check on the redirected branch.
