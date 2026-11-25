# Compilation/runtime boundary experiments

These isolated prototypes support [architecture exploration revision7](../../design/CompilationRuntimeArchitectureExploration.md), registry [D-25](../../design/DesignAuthorityRegistry.md#d-25). The scope is comparative evidence under H-387/H-390/H-392 and K-581. Production mechanisms, performance acceptance and the final architecture remain pending whole-file review.

`native_materialization.py` generates a small LLVM TLS/binding program and compares function-level with whole-module emission using the existing `build/bin/lli`. It writes temporary files outside the build tree and does not configure, install or clean tools. Both modes receive identical complete IR, so materialization savings establish no frontend savings.

```sh
python3 experiments/architecture/native_materialization.py --report /tmp/cod-native-materialization.json
```

The [native materialization report](native-materialization-results.json) contains30 historical executions with their original commands, outputs, timings and IR hashes. Its provenance locates the exact source rows in immutable Git history; extraction is not a new run. The standalone generator reproduces the recorded IR. Process times include startup, compilation and output and are not steady-state throughput.

`semantic_program.py` checks a small explicit-signature [whole program graph](semantic-program-results.graph.json), executes it with explicit reference frames and lowers each reachable definition once to [LLVM](semantic-program-results.ll). Business branches, selected effects, recursive calls, deferred bodies/handlers/finally and returns are represented as data. Both engines preserve pending completion and logical resource tokens. The [report](semantic-program-results.json) records66 full trace comparisons, independent business-result checks and five negative controls. A signature-compatible flattening changes behavior; a wrong resource token fails in both engines. Native exit97 denotes an experiment invariant violation, not a CoD exception policy.

```sh
python3 experiments/architecture/semantic_program.py --report /tmp/cod-semantic-program.json
```

That command also writes the exact input graph and generated LLVM alongside its report. It uses the existing `build/bin/lli`, with no build-tree mutation or new dependencies. The graph and code stay fixed through recursion depth 1000; execution frames, events and cleanup remain distinct occurrences. Native process timing includes startup, compilation and complete trace output, so it is not directly comparable to reference interpretation time.

The retired shilos experiments and their runners are preserved under `experiments/architecture` at commit `0d859151`. The historical [regional report](regional-program-results.json) records 96 trace comparisons, including AddressSanitizer coverage, two mapped addresses and owner cleanup. The [managed report](managed-runtime-results.json) records 342 trace comparisons and 30 warmups using OpenJDK foreign-memory and native calls. The reports retain their original source and artifact hashes.

The architecture document recommends a retained checked semantic boundary and shared-native validation against an equivalent optimized native reference. Optimizing-framework integration remains unmeasured. The native, regional and managed timings have different instrumentation and execution boundaries; none establishes a production performance ratio. Source-unobserved ADT combinations, same-layout distinct semantic types and complete callable-value transport remain explicit validation gaps.
