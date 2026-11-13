# Compilation/runtime boundary experiments

These are isolated research prototypes under [architecture exploration revision2](../../design/CompilationRuntimeArchitectureExploration.md), registry [D-25](../../design/DesignAuthorityRegistry.md#d-25), authorized by [H-387](../../design/DesignAuthorityRegistry.md#h-387). Candidate mechanisms and final architecture remain pending whole-file review. [K-574](../../design/DesignAuthorityRegistry.md#k-574) records the evidence; no production contract is qualified by these experiments.

`world_execution.mjs` constructs a finite fused semantic artifact and compares interpreted kernels, prepared shared native kernels and selective native compilation. Node/V8 executes the WebAssembly kernels against the same linear memory used by the model. Binding, owner, copy and completion behavior share a host scaffold. This is neither a CoD frontend nor a complete semantic-program interpreter. Region remapping is a byte-preserving simulation; the shilos allocator and pointers are untouched.

`run_experiments.py` runs five repetitions in shuffled mode order, checks equal observations and generates a separate LLVM TLS/materialization probe. The existing `build/bin/lli` runs that probe with function-level or whole-module materialization. Each native probe uses a temporary directory outside the build tree. Neither script configures, installs, cleans or changes the toolchain.

Run from the repository root with the existing toolchain:

```sh
python3 experiments/architecture/run_experiments.py --report /tmp/cod-architecture-results.json
```

The checked-in [results](results.json) retain all90 model and30 LLVM runs, source/tool hashes, raw traces, measured ranges and limitations. Native module sizes count WebAssembly bytes, not machine-code bytes. Process times include startup; model loop times include host dispatch, owner checks and calls across the native boundary. Neither is an isolated CoD throughput benchmark. The `--repetitions` option supports a smaller smoke check without changing experiment semantics.

Deliberately incomplete controls reject an entry-only fused artifact and demonstrate the missing case in a source-observed-tuples-only dispatcher. External inputs in this model use existing declared types; admitting new definitions is outside its assumptions. The experimental layout checksum and finite JSON encoding do not select a type identity, recursive hash scheme, ABI or persistent format.

The next evidence gap is whole-program semantic control and completion represented as data, followed by actual regional/native integration. Framework performance and engineering-effort comparisons remain unmeasured.
