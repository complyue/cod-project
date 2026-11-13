# AOT micro-dispatch: native evidence and scope

Revision: 1 · 2026-11-13 America/Los_Angeles.

## Design authority

Supporting implementation/assessment under [D-21r7](../../design/AheadOfTimeMultipleDispatch.md) and [D-25r11](../../design/CompilationRuntimeArchitectureExploration.md), H-396/H-397 and K-585. The user authorizes design replacement, focused native evidence and conditional merge of the supported scope. C-82/C-83 and H-378 preserve abstract family behavior and prepared native execution. C-86/C-87/C-88 with P-363–P-370 connect shared method bodies, concrete evidence and separate root compatibility to inspectable intent and affordable execution; detailed production qualification remains pending. Charter §§1–3, 4.3, 5.3–5.4, 6.3, 8 and 12 governs. Preserve conformance, actual owners, complete captures, dynamic context, ordered completion and relocation. Acceptance is an explained native route and discriminating observations with specialization disabled, including unchanged saved data across separately prepared worlds. No production ABI, full checker, loader/migration protocol, generic solver, performance class or ORC implementation is selected.

## Assessment

The delivered finite baseline works without BBV, whole-method tuple cloning, runtime compilation or an assumed fallback. It generates one native body per method/required interpretation, ordinary nominal method selectors and local operations from explicit contracts. There is no observed major correctness or compilation-termination obstacle within this scope. Continue this baseline while designing the full checked method/operation interface and generic closure separately.

The implementation is a scoped native prototype, not the CoD source compiler. `model.py` supplies finite semantic declarations and applicability checks; `generate.py` lowers authored operation templates and method records to C++20; existing Clang/LLVM emits native code. The resulting executable performs ordinary branches, calls and field accesses, with no runtime instruction interpreter. Every used native primitive is implemented in `runtime.hh`. The native code is compiled before fixture data is read or constructed.

## Domain workload and independent expectations

The compiler-domain planning and abstract-family requirements were selectively reused from `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8:experiments/local-dispatch/WORKLOAD.md`. They provide domain operations and falsifiers, not the prior BBV implementation or recommendation.

`Plan` has a nominal child `LiteralPlan`. Example conforms to LiteralPlan and SourceLocated; Other conforms to Plan and SourceLocated. They represent compiler plans with two independently polymorphic fields: x is an IntegerLiteral or BooleanLiteral; y is a LocalSlot or RegionalSlot. Example and Other have different field offsets. A separate destination argument also admits local/regional destinations. These are experimental representations, not proposed source syntax or a permanent compiler schema.

The lower family contains:

```text
lower(Plan, Destination)
lower(LiteralPlan, Destination)
lower(Plan, AddressDestination)
lower(LiteralPlan, AddressDestination)
```

The two middle methods are incomparable; the explicit intersection resolves Example/RegionalSlot. Each body reads x and y, encodes the operand, adds its method-specific behavior and selected context, and returns a stored abstract Summary. Encoding a boolean to a regional destination uses 0/255, while a local destination uses 0/1. This is a real joint type dependency inside one operation; independent field access elsewhere does not need a product of alternatives.

The no-field opcode family distinguishes IntegerLiteral/BooleanLiteral and Meters/Seconds even though their payload layouts match. A common Scalar method handles multiple alternatives identically. These cases distinguish observable method selection from shared field access. They do not justify another identity tag or metadata on statically known values.

Expected values and full effect traces are checked by a small independent business recurrence in `run.py`, not by interpreting generated instructions. The numerical fixture admits values in ±1,000,000, with boolean values limited to 0/1, so its arithmetic is representable. That boundary is enforced before construction and does not select CoD's general overflow or numeric-promotion policy.

## Executed observations

[observations.json](observations.json) contains exact commands, source hashes, results and complete traces. The [generated source](generated.inc) and [identity table](type_ids.inc) are review snapshots, regenerated from the implementation rather than edited by hand.

| Observation | Established result |
| --- | --- |
| General bodies | 13 generated method definitions: eight lower methods across two interpretations, four no-field opcode methods and one common Scalar method. Native symbol inspection finds one definition of each with `-O0 -fno-inline`; no specialization pass runs. |
| Abstract selection | Crossed specificity selects the explicit intersection; removing it rejects before emission. Reversed declaration order preserves the full native result/trace. |
| Local evidence | x/y retain separate concrete IDs. The outer object's identity is unchanged by x mutation. `read_x` and `read_y` each have two access paths for their enclosing layouts; scalar value access at a common offset has no discriminator check. |
| Runtime-only combinations | The compiled native algorithm handles the BooleanLiteral/RegionalSlot field combination from later input, producing 313. No body was generated for that particular tuple. |
| Call-site facts | A control build proves one exact Meters input and a two-alternative scalar set with a common target, bypassing selectors while calling the same bodies. Baseline execution has these simplifications disabled. |
| Abstract forwarding/result | An input view passes through TLS with its concrete ID and actual owner. A Summary is constructed in a separate output region, stored, returned and read through an abstract view. |
| Aliases | A permitted scratch-region alias changes x from IntegerLiteral to BooleanLiteral; subsequent access sees the new tag/payload. An injected pre-commit failure preserves the old pair. |
| Captures and interpretations | Two interpretations use the same concrete identities. Current calls follow replacement; a copied capture retains its bias and borrowed policy owner. Referent mutation remains visible. Distinct views can share the same actual owner. |
| Completion | Normal return, explicit zero return, acquisition/verifier failure, recovery without return, deferred failure preserving a return, and catch/finally replacement behave as specified. |
| Native resources | Actual LLVM context/module/builder acquisition, module verification and ordered release execute. One emitted LLVM module is compiled separately and returns 313 natively. |

There are 13 selected native cases, not an exhaustive product or performance campaign. Additional controls reject unproved coverage, a missing intersection, ambiguous discriminator mapping, an unresolved type/generic reference, missing entry installation, unknown/mismatched/abstract value evidence and values outside the numerical fixture range. Some checks consume authored contracts; this is not a full CoD static checker.

## Stable discriminators and saved regions

Concrete IDs are explicit 64-bit assignments owned by the fixture definitions in `STABLE_IDS`. They are independent of discovery order, type inventory, layout and checksums. This is a compact experimental identity mechanism, not a permanent allocation policy or globally coordinated registry. The existing [Type record](../../cod/lang_syntax.hh) has a category-local `size_t identity` within a richer type; that component alone is not assumed universally persistent.

One 8-byte discriminator accompanies a polymorphic field's relative payload link. Concrete Example/Other payloads have no extra object tag merely because their fields vary. A host view retains that same discriminator and a `global_ptr` carrying the actual owner. No second identity tag is introduced. Static policy fields need no discriminator.

A small catalog stored once in the fixture root binds eight relevant IDs to qualified semantic keys and the supported size/alignment. Writer and reader do not require all unrelated world types to match. Preparation rejects duplicate IDs; admission verifies the root's stored bindings before following payloads. A conflicting binding is a rejection, never a silent reinterpretation. The catalog is ordinary fixture regional data using the existing DBMR interface; the production header, library loader and file-format code are unchanged.

The persistence control uses separately compiled executables:

1. The older writer creates a file, then closes its mapping.
2. A newer reader has reversed declaration order and an additional unrelated concrete type. Existing definition IDs are unchanged. It reads the saved file and produces the same result and trace as the older reader.
3. A separately compiled invalid reader swaps Example/Other ID meanings. The root catalog rejects it with `REJECT 938` before typed payload access.
4. A full file SHA-256 comparison before and after both readers confirms that no stored byte changed. Neither tags nor the graph are rewritten or reconstructed for new numbering.

Both accepted readers also map the readonly region twice at different addresses and compare its bytes, results and traces. Mutable cases use an independently constructed scratch region, not writes to mapped input. All regional records are constructed with the actual `memory_region`; intra-region links use `regional_ptr`, host borrows use `global_ptr`, and owning regions use RAII. No regional record copy/move or persisted native/code address is used.

This proof is deliberately bounded to the documented unchanged root layouts and declared identities. The catalog does not establish arbitrary schema evolution, general untrusted-file validation, every native contract or global collision-free ID allocation. Production composition must diagnose conflicting definition identities rather than assuming these fixture literals solve every namespace problem.

## Root checksum planning is separate

`compatibility.py` is a small optional planning observation, not a loader or schema implementation. A checksum of a finite named root-description graph preserves recursive/shared references without hashing object values. Reordering definitions or adding an unrelated type does not change it. The chosen stable IDs also remain unchanged. A naive build-order ordinal scheme would not preserve meaning.

A deliberately invalid ID binding can still have the same root checksum. Therefore checksum equality cannot repair discriminator meaning or alone establish compatibility. Root identity/schema versions, relevant dependencies, stored-code interpretation, target ABI, file-format and bounds/reference validation remain separate inputs. Different checksums also do not by themselves decide compatible evolution. No checksum is read by a native method selector or local access operation.

## Context, installation and cleanup limits

`context.py` demonstrates a conservative fixed point over supplied contracts: transitive/recursive dependencies, touch without a current-value read, installation writes and deferred diagnostic requirements. Empty projections need no implicit context. Unknown native contracts reject rather than silently appearing context-free. It does not infer arbitrary CoD/C++ effects.

The generated runtime path evaluates and constructs a suppressed dependency initialization operand, omits its TLS write, installs entry context and permits procedure-local replacements. Current/captured selection preserves the full fixture value; restoration retains unrelated context. This checks the settled installation boundary, not a general module composer, conditional initialization proof or final-image dependency elimination.

Native resources are real LLVM C API objects. Acquisition failure is injected before the specified call; the verifier failure is a real missing-terminator rejection. Generated completion/control drains registered actions in reverse order and matches all acquisitions/releases. Finite Failure codes stand in for general exception objects. Nested perform replay, complete arbitrary captures, touch owner proof, checked exception payload graphs, native ownership transfer and full source-level defer matching remain separate work.

## Compilation costs and remaining obligations

No loop in the emitter enumerates concrete argument tuples or products of nested field alternatives. The fixed family code consists of method bodies plus applicability tests and nominal dominance exclusions. At most M² method-pair relationships are considered for a family; access paths depend on relevant layouts. Genuine joint operation dependencies remain. Body count can grow with actual methods, interpretations and explicit generic instances, not solely with runtime input combinations.

Finite coverage uses symbolic rectangles and a work cap; fragmentation can still be expensive. Budget exhaustion is “unproved,” not success or permission for a runtime missing-method fallback. A fixed nominal universe does not bound arbitrarily growing generic instantiations. The fixture has no implicit instantiator, variadics or general recursive inference. Its recursive descriptor planning is not evidence of a recursive generic solver.

Repeated selectors/local checks, descriptor admission, carrier bytes and catalog costs are real trade-offs. There is no timing, peak-memory, throughput or universal code-size claim. A broader checked operation interface and semantic/owner/context proof remain the next work before production integration. Optimizations must not discard owner/effect/completion obligations.

Method/callable/source associations survive in native symbol names and the generation report. ORC, profiling, tracing JIT, OSR and deoptimization are absent. No obvious source-mapping obstacle is introduced, but optimized address mapping and state reconstruction are future obligations. D-16 unit/identity constraints remain; no fractions or new numeric policy is implemented.

## Reproduction and provenance

From the repository root under the package's focused validation authorization:

```sh
python3 experiments/micro-dispatch/run.py --output /tmp/cod-micro-dispatch-observations.json
```

The runner uses a fresh temporary directory, generates/compiles the baseline, call-site/declaration-order control and independently prepared readers, and runs the focused observations. It uses existing Linux `build/bin/clang++`, `llvm-nm` and `libLLVM.so.23.1`. It configures or installs no toolchain. Full generated sources, canonical planning descriptions and binaries remain in the reported temporary directory. Checked-in source hashes identify the evidence revision; generated snapshots are for inspection.

Selective reuse from archive `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8` is limited to the independent workload requirements; the small symbolic-rectangle proof adapted from `families.py`; `context.py`; and the reseatable borrow/LLVM lifecycle utility patterns from `runtime.hh`. The BBV IR, versioner, composed CFG, cap and recommendation are not imported. Existing shilos and LLVM libraries are reused unchanged. No external engine, new dependency or production language feature is added.

The [work log](WORK_LOG.md) records the exact repository build, current validation/merge gate, branch ancestry and checkpoint accounting. The required `cmake --build ./build` is distinct from these isolated native observations; it does not by itself validate their semantics.
