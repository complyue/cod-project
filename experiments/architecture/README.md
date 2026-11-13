# Compilation/runtime boundary experiments

These are isolated research prototypes under [architecture exploration revision3](../../design/CompilationRuntimeArchitectureExploration.md), registry [D-25](../../design/DesignAuthorityRegistry.md#d-25), authorized by [H-387](../../design/DesignAuthorityRegistry.md#h-387). Candidate mechanisms and final architecture remain pending whole-file review. [K-574](../../design/DesignAuthorityRegistry.md#k-574)/[K-575](../../design/DesignAuthorityRegistry.md#k-575) record the evidence; no production contract is qualified by these experiments.

`world_execution.mjs` constructs a finite fused semantic artifact and compares interpreted kernels, prepared shared native kernels and selective native compilation. Node/V8 executes the WebAssembly kernels against the same linear memory used by the model. Binding, owner, copy and completion behavior share a host scaffold. This is neither a CoD frontend nor a complete semantic-program interpreter. Region remapping is a byte-preserving simulation; the shilos allocator and pointers are untouched.

`run_experiments.py` runs five repetitions in shuffled mode order, checks equal observations and generates a separate LLVM TLS/materialization probe. The existing `build/bin/lli` runs that probe with function-level or whole-module materialization. Each native probe uses a temporary directory outside the build tree. Neither script configures, installs, cleans or changes the toolchain.

Run from the repository root with the existing toolchain:

```sh
python3 experiments/architecture/run_experiments.py --report /tmp/cod-architecture-results.json
```

The checked-in [results](results.json) retain all90 model and30 LLVM runs, source/tool hashes, raw traces, measured ranges and limitations. Native module sizes count WebAssembly bytes, not machine-code bytes. Process times include startup; model loop times include host dispatch, owner checks and calls across the native boundary. Neither is an isolated CoD throughput benchmark. The `--repetitions` option supports a smaller smoke check without changing experiment semantics.

Deliberately incomplete controls reject an entry-only fused artifact and demonstrate the missing case in a source-observed-tuples-only dispatcher. External inputs in this model use existing declared types; admitting new definitions is outside its assumptions. The experimental layout checksum and finite JSON encoding do not select a type identity, recursive hash scheme, ABI or persistent format.

`semantic_program.py` checks a small explicit-signature [whole program graph](semantic-program-results.graph.json), executes it with explicit reference frames and lowers each reachable definition once to [LLVM](semantic-program-results.ll). Business branches, selected effects, recursive calls, deferred bodies/handlers/finally and returns are represented as data. Both engines preserve pending completion and logical resource tokens. The [report](semantic-program-results.json) records66 full trace comparisons, independent business-result checks and five negative controls. A signature-compatible flattening changes behavior; a wrong resource token fails in both engines. Native exit97 denotes an experiment invariant violation, not a CoD exception policy.

```sh
python3 experiments/architecture/semantic_program.py --report /tmp/cod-semantic-program.json
```

That command also writes the exact input graph and generated LLVM alongside its report. It uses the existing `build/bin/lli`, with no build-tree mutation or new dependencies. The graph and code stay fixed through recursion depth1000; execution frames, events and cleanup remain distinct occurrences. Native process timing includes startup, compilation and complete trace output, so it is not directly comparable to reference interpretation time.

The next evidence gap is actual regional graph/state storage and native boundary integration. The finite checker does not implement general CoD inference or owner proofs; dynamic experiment assertions do not authorize weakening required static checks. Framework performance and engineering-effort comparisons remain unmeasured.
