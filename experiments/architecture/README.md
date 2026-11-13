# Compilation/runtime boundary experiments

These are isolated research prototypes under [architecture exploration revision5](../../design/CompilationRuntimeArchitectureExploration.md), registry [D-25](../../design/DesignAuthorityRegistry.md#d-25), authorized by [H-387](../../design/DesignAuthorityRegistry.md#h-387). Candidate mechanisms and final architecture remain pending whole-file review. [K-574](../../design/DesignAuthorityRegistry.md#k-574)/[K-575](../../design/DesignAuthorityRegistry.md#k-575)/[K-576](../../design/DesignAuthorityRegistry.md#k-576)/[K-577](../../design/DesignAuthorityRegistry.md#k-577) record the evidence; no production contract is qualified by these experiments.

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

That command also writes the exact input graph and generated LLVM alongside its report. It uses the existing `build/bin/lli`, with no build-tree mutation or new dependencies. The graph and code stay fixed through recursion depth 1000; execution frames, events and cleanup remain distinct occurrences. Native process timing includes startup, compilation and complete trace output, so it is not directly comparable to reference interpretation time.

`regional_program.py` generates typed node records for the same program and shared native code with two borrowed-policy interfaces: copy a scalar or return a checked address. `regional_program.cc` and the managed bridge share `regional_fixture.hh`, which uses existing shilos containers, `regional_ptr`, temporary `global_ptr` views and `auto_region` ownership. The existing DBMR mapper opens a known fixture at two simultaneous addresses; its bytes remain unchanged. Interpreter state occupies a separate actual region. The test does not remap active state or introduce an external-input admission protocol.

```sh
python3 experiments/architecture/regional_program.py --output /tmp/cod-regional-program.json
```

The runner uses `build/bin/clang++` in temporary directories, following the repository's existing isolated native-harness pattern. It neither changes main-build configuration nor installs tools. The [regional report](regional-program-results.json) records 96 complete comparisons: 38 regular cases and 10 AddressSanitizer cases for each interface. Every case compares two regional executions, native execution and a logical reference projection. ASAN instruments C++ and generated LLVM; object-symbol checks verify the latter. Source/tool/generated-artifact hashes and per-case allocation counts are retained. `--smoke` runs one regular case per interface and is not full validation.

Real root-construction failure, owner expiry, shared-owner aliases, copied selection, body/defer failures and completion replacement are covered. All observed regions are freed, including partially constructed roots. A fixed 16,056-byte program uses 1,859,768 bytes of monotonic execution storage at depth 1000; finished frames/tasks are retained, with no reclamation design selected. Native callbacks catch C++ allocation/root failures before returning status; C++ exceptions do not cross LLVM frames. The simple image model does not establish a complete CoD capture ABI. Dynamic guards do not replace required static checks.

General inference/owner proof, complete captures, active-state remapping and optimizing-framework adapters remain unverified. Process duration includes three engines, file mapping, comparison and output, so it cannot rank engine throughput. Framework performance and engineering-effort comparisons remain unmeasured.

`managed_runtime.py` uses the installed OpenJDK21 preview foreign-memory/linker APIs and the existing LLVM compiler. `ManagedRegionalProgram.java` interprets the actual regional node records, with temporary Java execution frames. It compares direct scoped memory reads, per-field native accessors, and a single coarse call into the shared LLVM program. `managed_adapter.cc` coordinates shilos regions with Java owner scopes and exposes only the known fixture's layout/operations; it is not a production ABI or loading protocol.

```sh
python3 experiments/architecture/managed_runtime.py --output /tmp/cod-managed-runtime.json
```

The runner compiles everything into a temporary directory without installation or build-configuration changes. The [managed report](managed-runtime-results.json) records342 full-trace comparisons (38 cases × three modes × three repetitions), plus30 validated warmups, hashes, raw output and per-case timings/crossing/allocation counts. `--smoke --repetitions 1` runs four discriminating cases. Every reported execution compares the complete trace with the earlier logical reference, including real failure/owner cleanup and both mapped addresses. Read-only/bounds/closed-segment/expired-borrow guards are checked. The separate native harness remains ASAN-validated; this JVM is not an ASAN build.

At depth1000/context1, direct views avoid389,528 foreign calls while retaining the same result and7,000 events. Host scopes do not automatically know native owners: the adapter explicitly releases those owners and invalidates aliases. Java heap allocation is cumulative thread allocation, distinct from native-region occupation and from retained heap. Timings exclude setup and native trace transfer, include different trace allocation paths, and do not predict Truffle/RPython performance. HotSpot FFM demonstrates physical integration, not optimizing-framework integration.

The architecture document consolidates empirical criteria, practical reuse/maintenance responsibilities, a proposed native-timing change, and a bounded framework pilot. It is ready for whole-file review; no production runtime or ABI is selected by these results.

The [deliverable audit](exploration-deliverable-audit.json) maps the complete work package to its evidence, verifies current hashes and prior authority history, and lists production work that remains outside these experimental claims.
