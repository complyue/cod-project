# Changelog

## 2026-11-14 — Confirmed scalar and owner design integration

<a id="k-651"></a>
**K-651 — Design integration continuity.** Agent assessment under H-420 and charter §§3.3/4.3/5.3/6.3/8/12: registry r651 on both branches→652 and source reference272/268→273 reconcile H-413–H-420's confirmed scalar results, declaration operators and unique/null-owner rules with K-646–K-650's compiler capability record. D-14r87/D-10r147/D-18r91/D-23r7/D-24r3 and their recorded scoped qualification remain; the integration supplies no whole-file approval or implementation evidence for pending migration. Acceptance requires the receiving-storage matrices, return snapshots versus retained targets, statically unique owners and explicit null-owner forms while preserving cleanup/access constraints. All human evidence and concurrent compiler continuity remain. Unrelated aggregate, inference, abstract-evidence, regional-representation, fraction and permanent-ABI questions retain their status. Integrated design source: `b644351aa9521f0dbcabe6460e3e5ded71d827ae`; prior compiler capability reference: `df8bd38618eab3dd4cd446598fd10ea0019ca169:tests/cod/language/README.md`.

## 2026-11-14 — Current design integration and compiler handoff

<a id="h-420"></a>
**H-420 — Integrate the confirmed rules and retain the design worktree.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable message URL unavailable. The user requests removal of obsolete owner-acquisition wording from current repository content, commit/push, and notification of the main development chat to merge at its next suitable point and “让祂按最新规则推进编译器开发”. The user also directs this worktree to retain its dedicated branch and rebase onto merged main before continuing design updates. This authorizes delivery/integration and compiler development under H-413–H-419's confirmed scalar, initializer and unique/null-owner rules and their established consequences. It does not approve the whole D-23/P-354 value path, unrelated pending mechanisms, a permanent ABI, fractions or build/toolchain reconfiguration. Historical evidence remains in this changelog under the repository's current-state rule.

<a id="k-current-design-20261114"></a>
**K-CURRENT-DESIGN-20261114 — Current-state cleanup and scoped integration.** Agent assessment under H-420 and charter §§3.3/4.3/5.3/6.3/8/12: registry r650→651; D-14r86→87, D-23r6→7 and source reference267→268 remove obsolete null-owner narrative from current documents, record the explicit integration/development scope and distinguish compiler migration gaps from acceptance requirements. H-413–H-419's contract, existing value/dependency paths, constraints and acceptance remain; D-10r147/D-18r91/D-24r3 dependencies are unchanged. No source, test, build or toolchain change. Committed starting material: `f3da9988f20c7e6d35643ad703a01b050160dd24:design/RegionalOwnership.md`, `design/ScalarCallResults.md`, `design/DesignAuthorityRegistry.md` and `tests/cod/language/README.md`; this commit includes the preceding design-discussion records.

## 2026-11-14 — Explicit null owners without outer syntax

<a id="h-418"></a>
**H-418 — Put null ownership on the declaration or literal.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable URL unavailable. The user states:

> 我建议引入 `null in here|<some-region>|outer[ outer [outer ...]]` 语法，`ptr p:T = null` 直接拒绝，必须在变量侧或者 null 侧声明要给owner。

Accepted direction: owner-qualified null and rejection of a direct null-initialized ptr declaration without owner information on either side. The proposed forms name the current scope or a region; outer traversal was separately questioned and then withdrawn by H-419. No target allocation, runtime owner metadata, lifetime extension or whole-file approval is inferred.

<a id="h-419"></a>
**H-419 — No outer owner selector.** Human/source: same user/chat/date, answer to `request_user_input_async` call `call_afc77220a09841b0ac465973d7f25e92`, item 0. Asked whether outer should walk enclosing lexical scopes without implicit caller-scope access, the user replies:

> 我想想算了，就不支持 outer 语法了，用户自己在想要的scope声明一个变量，内层可以直接 = 那个变量把 owner 隐含带过来

Accepted: omit outer/repeated-outer syntax from H-418's proposal. Use a variable declared in the desired scope as the reference initializer to carry its owner into an inner scope. Current forms retain here and named owners. Owner propagation is ordinary checked reference initialization, including nullable views; it does not alias a pointer binding slot, create a target or extend the target owner's lifetime. D-14r86/D-10r147/D-23r6 records this source scope; final document approval remains outstanding.

<a id="k-null-owner-20261114"></a>
**K-NULL-OWNER-20261114 — Null owner syntax integration.** Agent assessment under H-418/H-419 and charter §§3.3/4.3/5.3/6.3/8/12: registry r649→650; D-14r85→86, D-23r5→6 and D-10r146→147 extend P-282's statically unique-owner constraint with explicit declaration/literal forms. Here follows existing full lexical scopes, named generic/region owners retain their checked meaning, explicit mismatches reject, and null views preserve owner evidence through initialization/clearing. Outer is not part of the grammar. Existing scalar, reference, cleanup, non-bits placement and fraction contracts remain; no source implementation, tests, build change or broader qualification. H-417/K-UNIQUE-OWNER-20261114 records the underlying static-owner decision.

## 2026-11-14 — Statically unique reference owners

<a id="h-417"></a>
**H-417 — Ref/ptr owners must be statically unique.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable URL unavailable. The user states:

> CoD 要求 ref/ptr 必须有静态可证明的唯一 owner
>
> 声明不显式写出 `in xxx` 时，由编译器根据它的初始化表达式静态推断，推断结果不唯一则报错。

Accepted: each ref/ptr has one statically provable owner; omission requests inference from initialization, and non-unique inference is an error. This closes the different-owner reference-result alternative question instead of authorizing an owner union or keeping multiple candidate owners alive. H-271's fixed-owner rules remain. Symbolic generic owners and runtime target choice within one owner remain possible; neither physical owner addresses nor one common owner for all captures is required. D-14r85/D-18r91/D-23r5 carries this scope; D-10r146/D-24r3 carries dependency alignment. The strict rule supersedes H-314's ownerless-null first-assignment exception; the null-specific syntax is discussed separately. No abstract concrete-type representation, runtime owner metadata, whole-file approval or compiler implementation is selected.

<a id="k-unique-owner-20261114"></a>
**K-UNIQUE-OWNER-20261114 — Owner policy closure.** Agent assessment under H-417 and charter §§3.3/4.3/5.3/6.3/8/12: registry r648→649; D-14r84→85 and D-18r90→91 require unique reference-owner proof through initializers, aliases and borrowed-result joins, updating P-244/P-246/P-282/P-300. D-23r4→5 removes the pending different-owner policy; D-10r145→146/D-24r2→3 aligns null/inference dependencies. Null absence supplies no owner, so the universal rule requires owner evidence at initialization rather than first later assignment. The checker may use one symbolic owner and proved equality; live-but-distinct alternatives reject. Complete callable values, cleanup, scalar snapshots, non-bits placement and fraction constraints remain; abstract type evidence and full-document qualifications retain their separate boundaries. No code, build or test change. Earlier scoped decisions remain in H-413–H-416; committed starting texts are at `f3da9988f20c7e6d35643ad703a01b050160dd24` under `design/`.

## 2026-11-14 — Reference initializer simplification and hidden results

<a id="h-416"></a>
**H-416 — Ref declarations bind with equals.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable URL unavailable. Asked about simplifying ref initialization, the assistant recommended rejecting `ref r <- ...`, preserving existing-target aliasing with `ref r = existing`, and materializing only independent scalar RHS values. The user replied:

> 同意。

Accepted: the stated ref-initializer simplification in D-23r4/D-14r84. Conceptual construction plus transfer is one initialization operation, preserving once-only evaluation and preventing exposure of an uninitialized readable ref. This replaces the analogous ref transfer shorthand, not H-296's separately confirmed ptr shorthand, later payload writes, or alias/access/lifetime rules. The same message asks whether a hidden uninitialized val result slot is a useful model and asks about non-val results; those are explanation/review questions, not a mandatory ABI, new cleanup policy or whole-file approval. Scalar snapshots and the existing explicit completion-replacement rules remain independently applicable.

<a id="k-scalar-init-20261114"></a>
**K-SCALAR-INIT-20261114 — Reference initialization and result retention.** Agent assessment under H-416 and charter §§3.3/5.3/6.3/8/12: registry r647→648, D-23r3→4 and D-14r83→84 apply the ref syntax correction through P-255/P-257 while preserving H-413–H-415 scalar/default-owner/val rules. D-23 explains pending private bits versus retained target selection, abrupt return, successful delivery and replacement under H-361/H-362/D-17. It preserves D-18 complete callable/capture and D-17 unique owning-result cleanup contracts; different-owner alternatives, abstract evidence transport, aggregates and full document/value-path approval remain separate. No mandatory result allocation, code, build or test change. Earlier discussion decisions remain H-413–H-415/K-SCALAR-VAL-20261114; committed starting text is `f3da9988f20c7e6d35643ad703a01b050160dd24:design/ScalarCallResults.md`.

## 2026-11-14 — Scalar value results and call-site storage

<a id="h-413"></a>
**H-413 — Scalar results default to val.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact message timestamp, personal name and durable URL unavailable. The user states:

> 这要结合接收此调用的 call site 语义语法一起看，另外 scalar  返回类型我决定应该默认 val 性质而非 ref。 call-site 的接收变量是 val/ref ，以及用了 = 还是 <- 会有区别，你细化厘清各种组合，没有疑义的直接定，然后把需要和我讨论敲定的再过一遍

Accepted: ordinary bare scalar results have independent val semantics; examine receiving val/ref declarations and `=`/`<-` together, settle direct consequences and return genuine choices for discussion. This replaces H-201's bare-scalar borrowed-result default, not explicit borrowed results, non-bits regional rules or assignment-family destination views. It does not approve the whole D-23 file, an initializer exception, generic/aggregate result defaults, an ABI or implementation work.

<a id="h-414"></a>
**H-414 — Scalar owner annotations select views.** Human/source: same user/chat/date, subsequent message; exact timestamp and durable URL unavailable:

> 感觉 对于 scalar 类型 T， `T in o` 应该视为 `ref  T in o` ，仅 `T` 才视为 `val T` 。 val 性质不应该有 owner 限制/关联，也不允许声明 `val T in o` 这种类型。

Accepted: scalar T without an owner defaults to val; T in o means ref T in o; val carries no owner restriction/association and val T in o is invalid. This retains H-273's explicit-owner shorthand alongside H-413's new default. Storage containing bits still has a lifetime for references to it; no hidden regional placement, ownership transfer, aggregate policy or whole-file approval follows. Current scope: D-23r3/D-14r83, with D-24r2 as a constrained inference dependency.

<a id="h-415"></a>
**H-415 — Val initialization requires bits transfer.** Human/source: same user/design chat, recorded 2026-11-14 America/Los_Angeles, structured answer to `request_user_input_async` call `call_ca13c997039e490ebc76c72bb2cd1797`, item 0; exact timestamp unavailable. Asked whether a new scalar val requires `val n:T <- f()` or also permits `val n:T = f()` as a declaration-only copy, with later `n = f()` remaining invalid, the user selected:

> Require `<-` for val initialization (Recommended)

Accepted: scalar val declaration initialization requires `<-`; `=` is rejected for both initialization and later assignment to val storage. Ref initialization/materialization and rebinding retain their separate contracts. D-23r3/D-14r83 records this final topic choice; complete-file/value-path approval remains outstanding, and no compiler implementation or callable-declaration redesign is authorized.

<a id="k-scalar-val-20261114"></a>
**K-SCALAR-VAL-20261114 — Scalar call-site integration.** Agent assessment under H-413–H-415 and charter §§2–3/§5.3/§6/§8/§12: registry r646→647, D-23r2→3 and D-14r82→83 record the confirmed scalar replacement/val initializer rule and derive declaration/assignment, materialization, access, cleanup and failure consequences from H-270/H-292/H-296/H-313/H-361/H-362/H-366–H-369. P-172/P-246/P-255 reflect the bounded update; P-354 remains a pending value path and P-355 dependency-only. D-24r1→2 aligns its dependency without qualifying inference. Existing explicit borrowed, assignment-family, callable, regional and fraction contracts continue; scalar topic choices are resolved, while complete-file/value-path approval remains outstanding. No code, build or test change. Prior texts: `f3da9988f20c7e6d35643ad703a01b050160dd24` at `design/ScalarCallResults.md`, `design/RegionalOwnership.md` and `design/CallableResultInference.md`.

## 2026-11-14 — Prepared builtin bits swaps

<a id="k-650"></a>
**K-650 — Prepared bits-swap continuity.** Agent assessment: registry r650→651 and source reference271→272 continue D-14r82/H-306/C-69/P-268/P-269's checked bits exchange and D-17r54/H-356/H-365/C-63/P-235/P-236/P-321–P-324's prepared borrowing and cleanup, under D-18r90 owner/alias constraints, D-21r11/D-26r9 native coverage and charter §§3.3/6.3/12. Deferred builtin bits swaps retain selected addresses and lifetime/read promises; payloads exchange only at invocation through the shared primitive path. Acceptance preserves operand effects, failed registration, exception categories/causes, LIFO completion and exact release in both native modes. Caller-binding reference-swap methods, prepared reference swaps, broader mutation proofs, regional mechanisms, permanent ABI and pending designs remain excluded. Prior capability reference: `63bbf032b6763aff85bd77645e26b8991ef5507b:tests/cod/language/README.md`.

## 2026-11-14 — Bits-swap method dispatch

<a id="k-649"></a>
**K-649 — Destination-method continuity.** Agent assessment: registry r649→650 and source reference270→271 continue D-14r82/H-306/C-69/P-268/P-269's distinct storage exchanges through shared method selection, with D-18r90 complete values/borrowing/recursion, D-17r54 preparation/cleanup and D-21r11/D-26r9 native coverage under charter §§3.3/6.3/12. Bits-swap overloads preserve exact destination types/access, actual owners, ordered address selection and parameter read promises; builtin fallback and selected bodies use shared native dispatch. Acceptance includes generic/deferred calls, complete callable replacement, injected failures and exact release in both modes. Caller-binding reference-swap methods, broader mutation proofs, regional mechanisms, permanent ABI and pending designs remain excluded. Prior capability reference: `4c8b9c086117829176f22177494b3925f964a656:tests/cod/language/README.md`.

## 2026-11-14 — Cleanup target flow

<a id="k-648"></a>
**K-648 — Cleanup target-flow continuity.** Agent assessment: registry r648→649 and source reference269→270 continue D-14r82/H-297/H-313–H-315/C-65/P-258's initialization and null-view constraints and D-17r54/C-63/P-235/P-236/P-321–P-324's cleanup completion, under D-18r90 owner/alias constraints, D-21r11/D-26r9 native coverage and charter §§3.3/6.3/12. Completed cleanup refines continuation targets; actual null-check conditions distinguish reachable read obligations from contradictory paths without waiving declared callable failures or initializing absent payloads. Acceptance preserves earlier reads, failed registrations, saved targets and exact release in both native modes without runtime initialization flags. Complete alias/predicate/loop correlations, regional mechanisms, permanent ABI and pending designs remain excluded. Prior capability reference: `29e76663aa56d48b7326b8ecb1735ac78f1cb6f8:tests/cod/language/README.md`.

## 2026-11-14 — Cleanup entry-target initialization

<a id="k-647"></a>
**K-647 — Entry-target proof continuity.** Agent assessment: registry r647→648 and source reference268→269 continue D-14r82/H-297/C-65/P-258's initialization-before-read contract and D-17r54/C-63/P-235/P-236/P-321–P-324's cleanup ordering/completion, under D-18r90 owner/alias constraints, D-21r11/D-26r9 native coverage and charter §§3.3/6.3/12. Shared summaries retain initialization and proved presence of entry targets independently of later rebinding; read obligations retain their ordering. Acceptance includes generic nullable parameters, failed checks/registrations, guarded recovery, repeated actions and exact native release without runtime initialization flags. Complete overlapping-alias histories, wider saved-target/queue correlations, regional mechanisms, permanent ABI and pending designs remain excluded. Prior capability reference: `a8c2f5f2bd2ccfb0be67deb9a099ff1ecd27968c:tests/cod/language/README.md`.

## 2026-11-14 — Cleanup target relationships

<a id="k-646"></a>
**K-646 — Cleanup target continuity.** Agent assessment: registry r646→647 and source reference267→268 continue D-14r82/H-297/C-65/P-258's initialization-before-read contract and D-17r54/C-63/P-235/P-236/P-321–P-324's cleanup ordering, under D-18r90 alias/native constraints, D-21r11/D-26r9 coverage and charter §§3.3/6.3/12. Finite target relationships compose rebinding with physical initialization across actions; weak updates retain entry alternatives and nullness remains independent of cell initialization. Acceptance preserves earlier-read obligations, failed registration, selected-address retention, category-specific completion and exact release in both native modes without runtime initialization flags. Complete selected-write histories, wider saved-target/queue correlations, regional mechanisms, permanent ABI and pending designs remain excluded. Prior capability reference: `f3da9988f20c7e6d35643ad703a01b050160dd24:tests/cod/language/README.md`.

## 2026-11-14 — Selected-target cleanup initialization

<a id="k-645"></a>
**K-645 — Cleanup initialization continuity.** Agent assessment: registry r645→646 and source reference266→267 continue D-14r82/H-297/C-65/P-258's initialization-before-read contract and D-17r54/C-63/P-235/P-236/P-321–P-324's deferred timing/completion, with D-18r90 alias/native constraints and D-21r11/D-26r9 coverage under charter §§3.3/6.3/12. Shared completion summaries distinguish persistent physical-storage initialization from current-selection guarantees invalidated by rebinding. Reads, writes and overlapping aliases compose through nested cleanup and category-specific outcomes. Acceptance includes failed registration, replaced completion, prepared target retention, exact release and both native modes without runtime initialization flags. Conditional initialization implications, wider saved-target/queue correlations, regional mechanisms, permanent ABI and pending designs remain outside this proof increment. Prior capability reference: `f2b47d46b56f86f66ab4f2a5553ced1c42e6fc4c:tests/cod/language/README.md`.

## 2026-11-14 — Runtime-selected ordinary aliases

<a id="k-644"></a>
**K-644 — Selected alias continuity.** Agent assessment: registry r644→645 and source reference265→266 continue D-18r90/H-327/C-75/P-295's once-only ordinary-place aliases, D-14r82/P-327's independent permissions and initialization/owner constraints, D-17r54 cleanup, and D-21r11/D-26r9 shared native coverage under charter §§3.3/6.3/12. A selected alias retains an existing slot without a new user holder, copy or ownership transfer. Overlapping mutations preserve conservative target facts; lexical names do not extend storage lifetime. Acceptance covers both selection paths, cleanup timing, generic recursion, complete callable replacement and exact resource release in both native modes. Mixed kinds, unproved owner alternatives and post-cleanup selected-target initialization remain implementation/proof gaps. Regional fields/arrays, lexical-cell capture, permanent ABI, D-16 fractions and pending designs remain excluded. Prior capability reference: `3ce248dd0b3eb3f677d29c29f32c9fefd8d8a18b:tests/cod/language/README.md`.

## 2026-11-14 — Outer lexical declaration evidence

<a id="k-643"></a>
**K-643 — Lexical query continuity.** Agent assessment: registry r643→644 and source reference264→265 continue D-14r82/H-295/P-257's nonexecuting queries, D-18r90/H-329/H-331/P-297's explicit capture obligations and P-299/P-301/P-302's native callable contract under D-21r11/D-26r9 and charter §§5/6.3/12. Known outer declarations supply type/access/owner-relation evidence independently of runtime values. Checking and shared native selection preserve distinct contexts without outer payload reads, storage retention or runtime owner identities. Acceptance includes shadowing/availability, returned and recursive contexts, unknown-owner rejection, allocation-free queries and exact real-capture cleanup in both modes. Unavailable declarations, outer lexical method-family evidence, body inference, permanent ABI, regional mechanisms and pending designs remain excluded. Prior capability reference: `67c3d8be50b024e78e5a49224aca1cab4421d6ce:tests/cod/language/README.md`.

## 2026-11-14 — Anonymous callable signature queries

<a id="k-642"></a>
**K-642 — Declared callable query continuity.** Agent assessment: registry r642→643 and source reference263→264 continue D-14r82/H-295/P-257's compile-time query scope and D-18r90/P-299/P-301/P-302's callable contract through D-21r11/D-26r9 semantic selection under charter §§5/6.3/12. Anonymous expressions supply established declared signatures through the existing evidence path without body demand, capture construction or runtime effects. Acceptance covers contextual/concrete signatures, declared throws/access/owner checks, unchanged ordinary construction obligations, allocation-free queries and native inventory in both modes. Outer type-only context, body-result inference, arbitrary statement queries, permanent ABI and pending designs remain outside coverage. Prior capability reference: `16975e4306af33b11a72ae837f487335dbb167c0:tests/cod/language/README.md`.

## 2026-11-14 — Contextual callable type evidence

<a id="k-641"></a>
**K-641 — Callable evidence continuity.** Agent assessment: registry r641→642 and source reference262→263 continue D-14r82/H-295/P-257's compile-time query scope, D-18r90/H-337–H-344/P-299/P-301/P-302's complete callable/native contract and D-21r11/D-26r9 semantic selection under charter §§5/6.3/12. Declared operand relationships establish callable views through shared preparation; signature selection preserves kind/result/access/owner/throws checks separately from runtime construction. Acceptance covers nested queries without body demand, ordered real snapshots/failures, exact cleanup and native inventory/compilation observations in both modes. Outer type-only dependencies, anonymous query operands, unproved inference, ABI and pending designs remain outside coverage. Prior capability reference: `9d02aa0953f990cea42b25fd5cdd5b86bb6a4988:tests/cod/language/README.md`.

## 2026-11-14 — Compile-time type queries

<a id="k-640"></a>
**K-640 — Type-query continuity.** Agent assessment: registry r640→641 and source reference261→262 continue D-14r82/H-295/P-257's bounded compile-time query and local storage contract, with D-18r90 capture/native boundaries and D-21r11/D-26r9 shared semantic selection under charter §§5/6.3/12. Required queries reuse declaration/type evidence without runtime reads, calls, demands or inferred captures; separate initialization preserves its effects. Acceptance includes overload/access/owner rejection, absent query-only native bodies, empty caught-value capture payloads and both native modes. Outer type-only dependencies, broader source forms, body-result inference, RTTI and pending designs remain outside coverage. Prior capability reference: `b92d35825ee6a1526d4c38b14a6f5b45e492e05e:tests/cod/language/README.md`.

## 2026-11-14 — Selected lexical reference slots

<a id="k-639"></a>
**K-639 — Reference slot continuity.** Agent assessment: registry r639→640 and source reference260→261 continue D-14r82/H-297/H-299/H-302/H-305/H-306/C-65/C-69/P-258/P-259/P-266/P-269/P-327, D-17r54 completion, D-18r90 native values/recursion and D-21r11/D-26r9 shared native coverage under charter §§3.3/6.3/12. Runtime-selected lexical slots preserve once-only selection, ordered rebinding/swap and conservative initialization/owner facts through cleanup. Acceptance includes failure after mutation, null slots, callable images and both native modes. Borrowed results expose no binding slot. Runtime slot aliases, broader relational proofs, regional mechanisms and pending designs remain outside coverage. Prior capability reference: `d84f3eb1785bcea938d0f000f0a034cfab99e44f:tests/cod/language/README.md`.

## 2026-11-14 — Contextual deferred capture storage

<a id="k-638"></a>
**K-638 — Deferred capture continuity.** Agent assessment: registry r638→639 and source reference259→260 continue D-17r54/C-63/P-235/P-236/P-321–P-324, D-14's local storage constraints, D-18r90's native value/recursion scope and D-21r11/D-26r9/H-409's local selection contract under charter §§3.3/6.3/12. Checked generic contexts retain their own caught-bit records and snapshot/local addresses within shared native methods. Acceptance covers nested registration, causes, nullness, completion, exact record/image cleanup and direct versus shared execution. No owner union, lifetime extension, ordinary-cell capture, public ABI, wider proof or pending mechanism is qualified. Prior capability reference: `e829605ff4ee8631885412015a7e3eab6f3b6af8:tests/cod/language/README.md`.

## 2026-11-13 — Loop setup and completion evidence

<a id="k-637"></a>
**K-637 — Loop completion continuity.** Agent assessment: registry r637→638 and source reference258→259 continue D-17r54/H-356/H-361/H-362/H-365/C-63/P-321–P-324/P-326, D-14 initialization/access, D-18r90 complete values and D-21r11/D-26r9 native coverage under charter §§3.3/6.3/12. Abrupt bounds skip loop initialization and both branches while preserving earlier effects and cleanup. Non-executing evidence propagates known exits without payload/access/owner claims; actual source checking remains authoritative. Acceptance covers ordered failure, skipped copying/registration, invalid-source rejection and both native modes. Missing index/accumulator types and broader proof, numeric, representation and pending design choices remain unqualified. Prior capability reference: `fb128d4f891f3938baa77b7f41a08b943b7dc857:tests/cod/language/README.md`.

## 2026-11-13 — Storage operand completion

<a id="k-636"></a>
**K-636 — Storage completion continuity.** Agent assessment: registry r636→637 and source reference257→258 continue D-14r82's H-297/H-306/C-65/C-69/P-258/P-269/P-327, D-17r54/H-361/H-362/P-323/P-324, D-18r90 complete values and D-21r11/D-26r9 native coverage under charter §§2–3/6.3/12. Aborted typed initialization, assignment and swaps preserve earlier effects and cleanup without creating payload/target facts or performing the mutation. Acceptance covers grouped/constructor exits, fixed/access/initialization rejection, complete callable results and both native modes. Missing types, wider construction/alias proofs, numeric compatibility and pending designs retain their boundaries. Prior capability reference: `876c20877834e167ab5648395d9c4a0122c615d1:tests/cod/language/README.md`.

## 2026-11-13 — Named-call preparation completion

<a id="k-635"></a>
**K-635 — Preparation completion continuity.** Agent assessment: registry r635→636 and source reference256→257 continue D-17r54/H-356/H-361/H-362/C-63/P-321–P-324, D-18r90's declared callable/borrowing contracts and D-21r11/D-26r9's native coverage under charter §§2–3/6.3/12. An established parameter contract checks an abruptly completing named operand without supplying a runtime payload; preparation preserves prior effects and cleanup while skipping invocation and registration. Acceptance covers explicit returns/failures, borrowed preparation, callable images, intrinsic null/cause operands and shared native bodies. Missing type/owner evidence stays unproved; no inference, numeric compatibility, lifetime, representation or pending design choice is selected. Prior capability reference: `b6e9ed8ce812b93f68aee172d51631c9a6356a5e:tests/cod/language/README.md`.

## 2026-11-13 — Shared callable signature preparation

<a id="k-634"></a>
**K-634 — Signature preparation continuity.** Agent assessment: registry r634→635 and source reference255→256 continue D-18r90/H-337–H-344/P-299/P-301/P-302, D-14's confirmed passing/owner/initialization rules, D-17r54's ordered preparation and D-21r11/D-26r9's native coverage under charter §§2–3/6.3/12. One named-method lookup preserves lexical recursive context across evidence, preparation and selection; existing structural relations use available later operand types for concrete callable views. Acceptance preserves evaluation order, ref/ptr/read borrows, captures, checked failures, exact cleanup and native recursion in both execution modes. No new inference, variance, numeric compatibility, lifetime or ABI contract is selected; pending D-23/D-27 and fraction/pure/TCO boundaries remain. Prior capability reference: `9da20f930607f9772605cba6d488b2f534b94923:tests/cod/language/README.md`.

## 2026-11-13 — Checked unknown-type exception contracts

<a id="k-633"></a>
**K-633 — Common exception integration continuity.** Agent assessment: registry r633→634, D-17r53→54 and source reference254→255 continue D-26r9/H-400/H-401/H-404/H-405/H-407, C-90/P-374/P-375 and D-17/C-63/P-235/P-236/P-321–P-324 under charter §§2–3/5.3–5.4/6.3/12. The shared category supports selected declarations, fixed caller obligations, handlers, causes and cleanup through existing D-18 callable and D-21 native contracts. Acceptance covers source checking and declared failure routing in both native modes; injected provider failures do not prove outside-catalog input or interpretation detection. The compact source-reference acceptance summary preserves its detailed capability sections. No new value relationship, carrier/ABI, generic inference, runtime compilation or pending D-23/D-27/fraction/pure/TCO scope is qualified. Prior current-state records: `55667c37cd67131a50bdca6264478ab03084ae4f:design/ExceptionControl.md` and the same commit at `tests/cod/language/README.md`.

## 2026-11-13 — Current-state repository and authority audit

Only changelogs retain repository history. Current contracts, scoped qualification, value/dependency edges and open decisions stay in their owning documents; historical payloads use immutable commit + path locators. This is editorial maintenance, not product-design approval.

<a id="h-411"></a>
**H-411 — Latest-state-only principle.** The current project user explicitly instructs that all repository content except changelogs show only the latest state, changelogs use brief descriptions and historical commit + path references, and an inspection follow. Source: this hygiene chat; recorded 2026-11-13 America/Los_Angeles; personal identity, exact message time and durable message URL unavailable. Scope: repository presentation and maintenance, including authority evidence. Product decisions, charter semantics and build/test permissions are unchanged.

<a id="h-412"></a>
**H-412 — Audit completion workflow.** Current project user in this hygiene chat, recorded 2026-11-13 America/Los_Angeles (exact time and durable message URL unavailable): repeat self-review after each inspection until no outstanding cleanup findings remain, then commit and push precise hunks. Minimize overlap with mainline documentation work; a small amount of valid overlapping documentation may be included. This authorizes the maintenance delivery workflow, not unrelated source changes.

<a id="k-632"></a>
**K-632 — Current-state audit continuity.** Agent assessment under H-05/H-388/H-391/H-411, charter §§1/3/5.3/8/10/11/12: registry r632→633; the inspected semantic revisions in its scope table retain their qualification, value/dependency endpoints, invariants, exclusions and open choices. Local authority blocks are consolidated around those current scopes. Decision journals, checkpoint narratives and retired validation payloads are recoverable below; evidence relocation and editorial summaries grant no additional qualification. The source capability reference owns implementation availability. This covers editorial revisions to D-07/D-08/D-10–D-18/D-21–D-26 and their linked references; unchanged semantic revision labels retain their prior scope. The D-18 result-representation question remains explicit. No source implementation, test execution, build configuration or toolchain action belongs to this audit.

<a id="source-compiler-validation-history"></a>
**Source compiler validation history.** The 129 retired JSON reports under `tests/cod/language/` (excluding the active `cases.json` manifest) are byte-identical to `2ab762d617b742f4fba7c2c617bacd303739ccef:tests/cod/language/`. That commit also preserves reference revision 218 and the compiler baseline. These reports do not establish current support. Current capabilities and validation commands are in [the source reference](tests/cod/language/README.md).

<a id="design-discussion-history"></a>
**Design discussion history.** 55667c37cd67131a50bdca6264478ab03084ae4f:`design/DomainLanguageDesign.md` retains DD-001 onward and the authority/checkpoint narratives; the same commit's `applications/steady-snn/docs/Decisions.md` retains AD-001–AD-092, original excerpts and proposal history. Current language topics and application questions remain in those documents. Per-document authority and implementation narratives are recoverable from this commit at their original paths.

<a id="micro-dispatch-experiment-history"></a>
**Micro-dispatch experiment history.** 55667c37cd67131a50bdca6264478ab03084ae4f:`experiments/micro-dispatch/WORK_LOG.md` preserves branch ancestry, validation, merge accounting and delivery receipt. The current [experiment](experiments/micro-dispatch/README.md) retains its bounded contract, results and limitations.

**Additional audit history.** `55667c37cd67131a50bdca6264478ab03084ae4f:tests/shilos/region-construction-validation.json` preserves the mixed allocator/retired-CoD report. At the same commit, `design/ConstructionAndAcquisition.md` retains migration counts and the compatibility comparison, and `design/CallableResultInference.md` retains the omitted-result observation. `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8:experiments/local-dispatch/` preserves the workload, `families.py`, `context.py` and `runtime.hh` sources selectively reused by micro-dispatch; its starting main was `8ff957610a455d0f242c45108777dd68a77561c6`.

## Authority evidence index

H records identify human decisions; K records identify agent assessments. Read the original event when its exact scope matters. Locators preserve original statements, provenance, time uncertainty, exclusions and supersession; an index is not additional approval. Corrections and withdrawals receive new IDs.

| Source | Immutable commit + path | Events |
| --- | --- | --- |
| A | `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md` | H-01–H-387, K-00–K-577 |
| B | `061a4a684770447ba674e54079f9a157aa75e43e:design/DesignAuthorityRegistry.md` | H-388/H-389, K-578 |
| C | `55667c37cd67131a50bdca6264478ab03084ae4f:design/DesignAuthorityRegistry.md` | H-390–H-410, K-579–K-631 |

Retrieve an event with `git show <commit>:<path>` and its source line range. Line numbers below refer to the immutable source, not this file.

### Human decisions

| Event | Source: lines |
| --- | --- |
| <a id="h-01"></a>H-01 | A:1201–1207 |
| <a id="h-02"></a>H-02 | A:1208–1214 |
| <a id="h-03"></a>H-03 | A:1215–1221 |
| <a id="h-04"></a>H-04 | A:1222–1228 |
| <a id="h-05"></a>H-05 | A:1229–1235 |
| <a id="h-06"></a>H-06 | A:1236–1247 |
| <a id="h-07"></a>H-07 | A:1248–1259 |
| <a id="h-08"></a>H-08 | A:1260–1271 |
| <a id="h-09"></a>H-09 | A:1272–1283 |
| <a id="h-10"></a>H-10 | A:1284–1292 |
| <a id="h-11"></a>H-11 | A:1293–1301 |
| <a id="h-12"></a>H-12 | A:1302–1310 |
| <a id="h-13"></a>H-13 | A:1311–1319 |
| <a id="h-14"></a>H-14 | A:1320–1328 |
| <a id="h-15"></a>H-15 | A:1329–1337 |
| <a id="h-16"></a>H-16 | A:1338–1346 |
| <a id="h-17"></a>H-17 | A:1347–1355 |
| <a id="h-18"></a>H-18 | A:1356–1364 |
| <a id="h-19"></a>H-19 | A:1365–1373 |
| <a id="h-20"></a>H-20 | A:1374–1382 |
| <a id="h-21"></a>H-21 | A:1383–1393 |
| <a id="h-22"></a>H-22 | A:1394–1402 |
| <a id="h-23"></a>H-23 | A:1403–1413 |
| <a id="h-24"></a>H-24 | A:1414–1426 |
| <a id="h-25"></a>H-25 | A:1427–1437 |
| <a id="h-26"></a>H-26 | A:1438–1446 |
| <a id="h-27"></a>H-27 | A:1447–1457 |
| <a id="h-28"></a>H-28 | A:1458–1468 |
| <a id="h-29"></a>H-29 | A:1469–1479 |
| <a id="h-30"></a>H-30 | A:1480–1490 |
| <a id="h-31"></a>H-31 | A:1491–1501 |
| <a id="h-32"></a>H-32 | A:1502–1512 |
| <a id="h-33"></a>H-33 | A:1513–1523 |
| <a id="h-34"></a>H-34 | A:1524–1534 |
| <a id="h-35"></a>H-35 | A:1535–1545 |
| <a id="h-36"></a>H-36 | A:1546–1557 |
| <a id="h-37"></a>H-37 | A:1558–1568 |
| <a id="h-38"></a>H-38 | A:1569–1579 |
| <a id="h-39"></a>H-39 | A:1580–1590 |
| <a id="h-40"></a>H-40 | A:1591–1602 |
| <a id="h-41"></a>H-41 | A:1603–1614 |
| <a id="h-42"></a>H-42 | A:1615–1626 |
| <a id="h-43"></a>H-43 | A:1627–1638 |
| <a id="h-44"></a>H-44 | A:1639–1650 |
| <a id="h-45"></a>H-45 | A:1651–1664 |
| <a id="h-46"></a>H-46 | A:1665–1676 |
| <a id="h-47"></a>H-47 | A:1677–1688 |
| <a id="h-48"></a>H-48 | A:1689–1700 |
| <a id="h-49"></a>H-49 | A:1701–1712 |
| <a id="h-50"></a>H-50 | A:1713–1724 |
| <a id="h-51"></a>H-51 | A:1725–1737 |
| <a id="h-52"></a>H-52 | A:1738–1749 |
| <a id="h-53"></a>H-53 | A:1750–1762 |
| <a id="h-54"></a>H-54 | A:1763–1774 |
| <a id="h-55"></a>H-55 | A:1775–1786 |
| <a id="h-56"></a>H-56 | A:1787–1806 |
| <a id="h-57"></a>H-57 | A:1807–1818 |
| <a id="h-58"></a>H-58 | A:1819–1831 |
| <a id="h-59"></a>H-59 | A:1832–1843 |
| <a id="h-60"></a>H-60 | A:1844–1856 |
| <a id="h-61"></a>H-61 | A:1857–1869 |
| <a id="h-62"></a>H-62 | A:1870–1884 |
| <a id="h-63"></a>H-63 | A:1885–1897 |
| <a id="h-64"></a>H-64 | A:1898–1910 |
| <a id="h-65"></a>H-65 | A:1911–1928 |
| <a id="h-66"></a>H-66 | A:1929–1944 |
| <a id="h-67"></a>H-67 | A:1945–1958 |
| <a id="h-68"></a>H-68 | A:1959–1982 |
| <a id="h-69"></a>H-69 | A:1983–1994 |
| <a id="h-70"></a>H-70 | A:1995–2006 |
| <a id="h-71"></a>H-71 | A:2007–2026 |
| <a id="h-72"></a>H-72 | A:2027–2038 |
| <a id="h-73"></a>H-73 | A:2039–2050 |
| <a id="h-74"></a>H-74 | A:2051–2063 |
| <a id="h-75"></a>H-75 | A:2064–2077 |
| <a id="h-76"></a>H-76 | A:2078–2089 |
| <a id="h-77"></a>H-77 | A:2090–2101 |
| <a id="h-78"></a>H-78 | A:2102–2117 |
| <a id="h-79"></a>H-79 | A:2118–2129 |
| <a id="h-80"></a>H-80 | A:2130–2141 |
| <a id="h-81"></a>H-81 | A:2142–2153 |
| <a id="h-82"></a>H-82 | A:2154–2169 |
| <a id="h-83"></a>H-83 | A:2170–2185 |
| <a id="h-84"></a>H-84 | A:2186–2201 |
| <a id="h-85"></a>H-85 | A:2202–2217 |
| <a id="h-86"></a>H-86 | A:2218–2231 |
| <a id="h-87"></a>H-87 | A:2232–2247 |
| <a id="h-88"></a>H-88 | A:2248–2259 |
| <a id="h-89"></a>H-89 | A:2260–2271 |
| <a id="h-90"></a>H-90 | A:2272–2283 |
| <a id="h-91"></a>H-91 | A:2284–2295 |
| <a id="h-92"></a>H-92 | A:2296–2305 |
| <a id="h-93"></a>H-93 | A:2306–2319 |
| <a id="h-94"></a>H-94 | A:2320–2335 |
| <a id="h-95"></a>H-95 | A:2336–2345 |
| <a id="h-96"></a>H-96 | A:2346–2361 |
| <a id="h-97"></a>H-97 | A:2362–2371 |
| <a id="h-98"></a>H-98 | A:2372–2381 |
| <a id="h-99"></a>H-99 | A:2382–2391 |
| <a id="h-100"></a>H-100 | A:2392–2401 |
| <a id="h-101"></a>H-101 | A:2402–2411 |
| <a id="h-102"></a>H-102 | A:2412–2421 |
| <a id="h-103"></a>H-103 | A:2422–2431 |
| <a id="h-104"></a>H-104 | A:2432–2441 |
| <a id="h-105"></a>H-105 | A:2442–2451 |
| <a id="h-106"></a>H-106 | A:2452–2461 |
| <a id="h-107"></a>H-107 | A:2462–2471 |
| <a id="h-108"></a>H-108 | A:2472–2481 |
| <a id="h-109"></a>H-109 | A:2482–2491 |
| <a id="h-110"></a>H-110 | A:2492–2501 |
| <a id="h-111"></a>H-111 | A:2502–2512 |
| <a id="h-112"></a>H-112 | A:2513–2523 |
| <a id="h-113"></a>H-113 | A:2524–2534 |
| <a id="h-114"></a>H-114 | A:2535–2545 |
| <a id="h-115"></a>H-115 | A:2546–2556 |
| <a id="h-116"></a>H-116 | A:2557–2567 |
| <a id="h-117"></a>H-117 | A:2568–2577 |
| <a id="h-118"></a>H-118 | A:2578–2588 |
| <a id="h-119"></a>H-119 | A:2589–2599 |
| <a id="h-120"></a>H-120 | A:2600–2610 |
| <a id="h-121"></a>H-121 | A:2611–2621 |
| <a id="h-122"></a>H-122 | A:2622–2632 |
| <a id="h-123"></a>H-123 | A:2633–2647 |
| <a id="h-124"></a>H-124 | A:2648–2658 |
| <a id="h-125"></a>H-125 | A:2659–2669 |
| <a id="h-126"></a>H-126 | A:2670–2680 |
| <a id="h-127"></a>H-127 | A:2681–2691 |
| <a id="h-128"></a>H-128 | A:2692–2702 |
| <a id="h-129"></a>H-129 | A:2703–2713 |
| <a id="h-130"></a>H-130 | A:2714–2724 |
| <a id="h-131"></a>H-131 | A:2725–2735 |
| <a id="h-132"></a>H-132 | A:2736–2746 |
| <a id="h-133"></a>H-133 | A:2747–2757 |
| <a id="h-134"></a>H-134 | A:2758–2768 |
| <a id="h-135"></a>H-135 | A:2769–2779 |
| <a id="h-136"></a>H-136 | A:2780–2790 |
| <a id="h-137"></a>H-137 | A:2791–2801 |
| <a id="h-138"></a>H-138 | A:2802–2813 |
| <a id="h-139"></a>H-139 | A:2814–2824 |
| <a id="h-140"></a>H-140 | A:2825–2835 |
| <a id="h-141"></a>H-141 | A:2836–2845 |
| <a id="h-142"></a>H-142 | A:2846–2855 |
| <a id="h-143"></a>H-143 | A:2856–2865 |
| <a id="h-144"></a>H-144 | A:2866–2875 |
| <a id="h-145"></a>H-145 | A:2876–2885 |
| <a id="h-146"></a>H-146 | A:2886–2895 |
| <a id="h-147"></a>H-147 | A:2896–2905 |
| <a id="h-148"></a>H-148 | A:2906–2917 |
| <a id="h-149"></a>H-149 | A:2918–2929 |
| <a id="h-150"></a>H-150 | A:2930–2941 |
| <a id="h-151"></a>H-151 | A:2942–2951 |
| <a id="h-152"></a>H-152 | A:2952–2961 |
| <a id="h-153"></a>H-153 | A:2962–2971 |
| <a id="h-154"></a>H-154 | A:2972–2981 |
| <a id="h-155"></a>H-155 | A:2982–2991 |
| <a id="h-156"></a>H-156 | A:2992–3001 |
| <a id="h-157"></a>H-157 | A:3002–3013 |
| <a id="h-158"></a>H-158 | A:3014–3023 |
| <a id="h-159"></a>H-159 | A:3024–3033 |
| <a id="h-160"></a>H-160 | A:3034–3043 |
| <a id="h-161"></a>H-161 | A:3044–3053 |
| <a id="h-162"></a>H-162 | A:3054–3063 |
| <a id="h-163"></a>H-163 | A:3064–3073 |
| <a id="h-164"></a>H-164 | A:3074–3083 |
| <a id="h-165"></a>H-165 | A:3084–3093 |
| <a id="h-166"></a>H-166 | A:3094–3103 |
| <a id="h-167"></a>H-167 | A:3104–3113 |
| <a id="h-168"></a>H-168 | A:3114–3123 |
| <a id="h-169"></a>H-169 | A:3124–3133 |
| <a id="h-170"></a>H-170 | A:3134–3143 |
| <a id="h-171"></a>H-171 | A:3144–3153 |
| <a id="h-172"></a>H-172 | A:3154–3167 |
| <a id="h-173"></a>H-173 | A:3168–3177 |
| <a id="h-174"></a>H-174 | A:3178–3187 |
| <a id="h-175"></a>H-175 | A:3188–3197 |
| <a id="h-176"></a>H-176 | A:3198–3207 |
| <a id="h-177"></a>H-177 | A:3208–3217 |
| <a id="h-178"></a>H-178 | A:3218–3227 |
| <a id="h-179"></a>H-179 | A:3228–3237 |
| <a id="h-180"></a>H-180 | A:3238–3249 |
| <a id="h-181"></a>H-181 | A:3250–3261 |
| <a id="h-182"></a>H-182 | A:3262–3271 |
| <a id="h-183"></a>H-183 | A:3272–3281 |
| <a id="h-184"></a>H-184 | A:3282–3291 |
| <a id="h-185"></a>H-185 | A:3292–3301 |
| <a id="h-186"></a>H-186 | A:3302–3311 |
| <a id="h-187"></a>H-187 | A:3312–3321 |
| <a id="h-188"></a>H-188 | A:3322–3331 |
| <a id="h-189"></a>H-189 | A:3332–3341 |
| <a id="h-190"></a>H-190 | A:3342–3351 |
| <a id="h-191"></a>H-191 | A:3352–3361 |
| <a id="h-192"></a>H-192 | A:3362–3371 |
| <a id="h-193"></a>H-193 | A:3372–3385 |
| <a id="h-194"></a>H-194 | A:3386–3395 |
| <a id="h-195"></a>H-195 | A:3396–3405 |
| <a id="h-196"></a>H-196 | A:3406–3415 |
| <a id="h-197"></a>H-197 | A:3416–3427 |
| <a id="h-198"></a>H-198 | A:3428–3437 |
| <a id="h-199"></a>H-199 | A:3438–3447 |
| <a id="h-200"></a>H-200 | A:3448–3457 |
| <a id="h-201"></a>H-201 | A:3458–3469 |
| <a id="h-202"></a>H-202 | A:3470–3481 |
| <a id="h-203"></a>H-203 | A:3482–3491 |
| <a id="h-204"></a>H-204 | A:3492–3501 |
| <a id="h-205"></a>H-205 | A:3502–3511 |
| <a id="h-206"></a>H-206 | A:3512–3523 |
| <a id="h-207"></a>H-207 | A:3524–3535 |
| <a id="h-208"></a>H-208 | A:3536–3545 |
| <a id="h-209"></a>H-209 | A:3546–3559 |
| <a id="h-210"></a>H-210 | A:3560–3569 |
| <a id="h-211"></a>H-211 | A:3570–3581 |
| <a id="h-212"></a>H-212 | A:3582–3591 |
| <a id="h-213"></a>H-213 | A:3592–3601 |
| <a id="h-214"></a>H-214 | A:3602–3611 |
| <a id="h-215"></a>H-215 | A:3612–3621 |
| <a id="h-216"></a>H-216 | A:3622–3631 |
| <a id="h-217"></a>H-217 | A:3632–3641 |
| <a id="h-218"></a>H-218 | A:3642–3651 |
| <a id="h-219"></a>H-219 | A:3652–3661 |
| <a id="h-220"></a>H-220 | A:3662–3671 |
| <a id="h-221"></a>H-221 | A:3672–3681 |
| <a id="h-222"></a>H-222 | A:3682–3691 |
| <a id="h-223"></a>H-223 | A:3692–3701 |
| <a id="h-224"></a>H-224 | A:3702–3711 |
| <a id="h-225"></a>H-225 | A:3712–3723 |
| <a id="h-226"></a>H-226 | A:3724–3735 |
| <a id="h-227"></a>H-227 | A:3736–3747 |
| <a id="h-228"></a>H-228 | A:3748–3757 |
| <a id="h-229"></a>H-229 | A:3758–3767 |
| <a id="h-230"></a>H-230 | A:3768–3777 |
| <a id="h-231"></a>H-231 | A:3778–3787 |
| <a id="h-232"></a>H-232 | A:3788–3799 |
| <a id="h-233"></a>H-233 | A:3800–3810 |
| <a id="h-234"></a>H-234 | A:3811–3820 |
| <a id="h-235"></a>H-235 | A:3821–3830 |
| <a id="h-236"></a>H-236 | A:3831–3841 |
| <a id="h-237"></a>H-237 | A:3842–3852 |
| <a id="h-238"></a>H-238 | A:3853–3862 |
| <a id="h-239"></a>H-239 | A:3863–3872 |
| <a id="h-240"></a>H-240 | A:3873–3882 |
| <a id="h-241"></a>H-241 | A:3883–3893 |
| <a id="h-242"></a>H-242 | A:3894–3903 |
| <a id="h-243"></a>H-243 | A:3904–3913 |
| <a id="h-244"></a>H-244 | A:3914–3923 |
| <a id="h-245"></a>H-245 | A:3924–3935 |
| <a id="h-246"></a>H-246 | A:3936–3945 |
| <a id="h-247"></a>H-247 | A:3946–3955 |
| <a id="h-248"></a>H-248 | A:3956–3965 |
| <a id="h-249"></a>H-249 | A:3966–3975 |
| <a id="h-250"></a>H-250 | A:3976–3985 |
| <a id="h-251"></a>H-251 | A:3986–3995 |
| <a id="h-252"></a>H-252 | A:3996–4005 |
| <a id="h-253"></a>H-253 | A:4006–4017 |
| <a id="h-254"></a>H-254 | A:4018–4039 |
| <a id="h-255"></a>H-255 | A:4040–4049 |
| <a id="h-256"></a>H-256 | A:4050–4059 |
| <a id="h-257"></a>H-257 | A:4060–4072 |
| <a id="h-258"></a>H-258 | A:4073–4084 |
| <a id="h-259"></a>H-259 | A:4085–4094 |
| <a id="h-260"></a>H-260 | A:4095–4106 |
| <a id="h-261"></a>H-261 | A:4107–4122 |
| <a id="h-262"></a>H-262 | A:4123–4134 |
| <a id="h-263"></a>H-263 | A:4135–4146 |
| <a id="h-264"></a>H-264 | A:4147–4158 |
| <a id="h-265"></a>H-265 | A:4159–4170 |
| <a id="h-266"></a>H-266 | A:4171–4180 |
| <a id="h-267"></a>H-267 | A:4181–4191 |
| <a id="h-268"></a>H-268 | A:4192–4201 |
| <a id="h-269"></a>H-269 | A:4202–4211 |
| <a id="h-270"></a>H-270 | A:4212–4223 |
| <a id="h-271"></a>H-271 | A:4224–4233 |
| <a id="h-272"></a>H-272 | A:4234–4245 |
| <a id="h-273"></a>H-273 | A:4246–4255 |
| <a id="h-274"></a>H-274 | A:4256–4265 |
| <a id="h-275"></a>H-275 | A:4266–4277 |
| <a id="h-276"></a>H-276 | A:4278–4287 |
| <a id="h-277"></a>H-277 | A:4288–4297 |
| <a id="h-278"></a>H-278 | A:4298–4307 |
| <a id="h-279"></a>H-279 | A:4308–4317 |
| <a id="h-280"></a>H-280 | A:4318–4327 |
| <a id="h-281"></a>H-281 | A:4328–4337 |
| <a id="h-282"></a>H-282 | A:4338–4347 |
| <a id="h-283"></a>H-283 | A:4348–4357 |
| <a id="h-284"></a>H-284 | A:4358–4367 |
| <a id="h-285"></a>H-285 | A:4368–4377 |
| <a id="h-286"></a>H-286 | A:4378–4391 |
| <a id="h-287"></a>H-287 | A:4392–4401 |
| <a id="h-288"></a>H-288 | A:4402–4411 |
| <a id="h-289"></a>H-289 | A:4412–4421 |
| <a id="h-290"></a>H-290 | A:4422–4431 |
| <a id="h-291"></a>H-291 | A:4432–4441 |
| <a id="h-292"></a>H-292 | A:4442–4451 |
| <a id="h-293"></a>H-293 | A:4452–4463 |
| <a id="h-294"></a>H-294 | A:4464–4473 |
| <a id="h-295"></a>H-295 | A:4474–4487 |
| <a id="h-296"></a>H-296 | A:4488–4499 |
| <a id="h-297"></a>H-297 | A:4500–4511 |
| <a id="h-298"></a>H-298 | A:4512–4521 |
| <a id="h-299"></a>H-299 | A:4522–4533 |
| <a id="h-300"></a>H-300 | A:4534–4543 |
| <a id="h-301"></a>H-301 | A:4544–4553 |
| <a id="h-302"></a>H-302 | A:4554–4569 |
| <a id="h-303"></a>H-303 | A:4570–4579 |
| <a id="h-304"></a>H-304 | A:4580–4591 |
| <a id="h-305"></a>H-305 | A:4592–4601 |
| <a id="h-306"></a>H-306 | A:4602–4615 |
| <a id="h-307"></a>H-307 | A:4616–4625 |
| <a id="h-308"></a>H-308 | A:4626–4635 |
| <a id="h-309"></a>H-309 | A:4636–4647 |
| <a id="h-310"></a>H-310 | A:4648–4659 |
| <a id="h-311"></a>H-311 | A:4660–4671 |
| <a id="h-312"></a>H-312 | A:4672–4681 |
| <a id="h-313"></a>H-313 | A:4682–4691 |
| <a id="h-314"></a>H-314 | A:4692–4701 |
| <a id="h-315"></a>H-315 | A:4702–4711 |
| <a id="h-316"></a>H-316 | A:4712–4721 |
| <a id="h-317"></a>H-317 | A:4722–4731 |
| <a id="h-318"></a>H-318 | A:4732–4743 |
| <a id="h-319"></a>H-319 | A:4744–4759 |
| <a id="h-320"></a>H-320 | A:4760–4769 |
| <a id="h-321"></a>H-321 | A:4770–4779 |
| <a id="h-322"></a>H-322 | A:4780–4791 |
| <a id="h-323"></a>H-323 | A:4792–4799 |
| <a id="h-324"></a>H-324 | A:4800–4809 |
| <a id="h-325"></a>H-325 | A:4810–4817 |
| <a id="h-326"></a>H-326 | A:4818–4825 |
| <a id="h-327"></a>H-327 | A:4826–4837 |
| <a id="h-328"></a>H-328 | A:4838–4845 |
| <a id="h-329"></a>H-329 | A:4846–4865 |
| <a id="h-330"></a>H-330 | A:4866–4873 |
| <a id="h-331"></a>H-331 | A:4874–4881 |
| <a id="h-332"></a>H-332 | A:4882–4889 |
| <a id="h-333"></a>H-333 | A:4890–4897 |
| <a id="h-334"></a>H-334 | A:4898–4905 |
| <a id="h-335"></a>H-335 | A:4906–4913 |
| <a id="h-336"></a>H-336 | A:4914–4921 |
| <a id="h-337"></a>H-337 | A:4922–4929 |
| <a id="h-338"></a>H-338 | A:4930–4940 |
| <a id="h-339"></a>H-339 | A:4941–4949 |
| <a id="h-340"></a>H-340 | A:4950–4958 |
| <a id="h-341"></a>H-341 | A:4959–4966 |
| <a id="h-342"></a>H-342 | A:4967–4974 |
| <a id="h-343"></a>H-343 | A:4975–4982 |
| <a id="h-344"></a>H-344 | A:4983–4990 |
| <a id="h-345"></a>H-345 | A:4991–4998 |
| <a id="h-346"></a>H-346 | A:4999–5008 |
| <a id="h-347"></a>H-347 | A:5009–5018 |
| <a id="h-348"></a>H-348 | A:5019–5026 |
| <a id="h-349"></a>H-349 | A:5027–5034 |
| <a id="h-350"></a>H-350 | A:5035–5042 |
| <a id="h-351"></a>H-351 | A:5043–5052 |
| <a id="h-352"></a>H-352 | A:5053–5072 |
| <a id="h-353"></a>H-353 | A:5073–5090 |
| <a id="h-354"></a>H-354 | A:5091–5100 |
| <a id="h-355"></a>H-355 | A:5101–5112 |
| <a id="h-356"></a>H-356 | A:5113–5122 |
| <a id="h-357"></a>H-357 | A:5123–5134 |
| <a id="h-358"></a>H-358 | A:5135–5150 |
| <a id="h-359"></a>H-359 | A:5151–5160 |
| <a id="h-360"></a>H-360 | A:5161–5174 |
| <a id="h-361"></a>H-361 | A:5175–5184 |
| <a id="h-362"></a>H-362 | A:5185–5198 |
| <a id="h-363"></a>H-363 | A:5199–5212 |
| <a id="h-364"></a>H-364 | A:5213–5222 |
| <a id="h-365"></a>H-365 | A:5223–5259 |
| <a id="h-366"></a>H-366 | A:5260–5272 |
| <a id="h-367"></a>H-367 | A:5273–5281 |
| <a id="h-368"></a>H-368 | A:5282–5290 |
| <a id="h-369"></a>H-369 | A:5291–5303 |
| <a id="h-370"></a>H-370 | A:5304–5318 |
| <a id="h-371"></a>H-371 | A:5319–5327 |
| <a id="h-372"></a>H-372 | A:5328–5342 |
| <a id="h-373"></a>H-373 | A:5343–5355 |
| <a id="h-374"></a>H-374 | A:5356–5364 |
| <a id="h-375"></a>H-375 | A:5365–5373 |
| <a id="h-376"></a>H-376 | A:5374–5382 |
| <a id="h-377"></a>H-377 | A:5383–5403 |
| <a id="h-378"></a>H-378 | A:5404–5414 |
| <a id="h-379"></a>H-379 | A:5415–5423 |
| <a id="h-380"></a>H-380 | A:5424–5438 |
| <a id="h-381"></a>H-381 | A:5439–5449 |
| <a id="h-382"></a>H-382 | A:5450–5462 |
| <a id="h-383"></a>H-383 | A:5463–5481 |
| <a id="h-384"></a>H-384 | A:5482–5496 |
| <a id="h-385"></a>H-385 | A:5497–5509 |
| <a id="h-386"></a>H-386 | A:5510–5532 |
| <a id="h-387"></a>H-387 | A:5533–5543 |
| <a id="h-388"></a>H-388 | B:1611–1623 |
| <a id="h-389"></a>H-389 | B:1625–1630 |
| <a id="h-390"></a>H-390 | C:1028–1036 |
| <a id="h-391"></a>H-391 | C:1038–1045 |
| <a id="h-392"></a>H-392 | C:1047–1054 |
| <a id="h-393"></a>H-393 | C:1056–1073 |
| <a id="h-394"></a>H-394 | C:1075–1082 |
| <a id="h-395"></a>H-395 | C:1084–1093 |
| <a id="h-396"></a>H-396 | C:1095–1122 |
| <a id="h-397"></a>H-397 | C:1124–1133 |
| <a id="h-398"></a>H-398 | C:1135–1144 |
| <a id="h-399"></a>H-399 | C:1146–1155 |
| <a id="h-400"></a>H-400 | C:1157–1164 |
| <a id="h-401"></a>H-401 | C:1166–1171 |
| <a id="h-402"></a>H-402 | C:1173–1178 |
| <a id="h-403"></a>H-403 | C:1180–1183 |
| <a id="h-404"></a>H-404 | C:1185–1190 |
| <a id="h-405"></a>H-405 | C:1192–1197 |
| <a id="h-406"></a>H-406 | C:1199–1204 |
| <a id="h-407"></a>H-407 | C:1206–1213 |
| <a id="h-408"></a>H-408 | C:1215–1220 |
| <a id="h-409"></a>H-409 | C:1222–1234 |
| <a id="h-410"></a>H-410 | C:1236–1243 |

### Agent continuity

| Event | Source: lines |
| --- | --- |
| <a id="k-00"></a>K-00 | A:5546–5557 |
| <a id="k-01"></a>K-01 | A:5558–5567 |
| <a id="k-02"></a>K-02 | A:5568–5577 |
| <a id="k-03"></a>K-03 | A:5578–5588 |
| <a id="k-04"></a>K-04 | A:5589–5598 |
| <a id="k-05"></a>K-05 | A:5599–5607 |
| <a id="k-06"></a>K-06 | A:5608–5615 |
| <a id="k-07"></a>K-07 | A:5616–5624 |
| <a id="k-08"></a>K-08 | A:5625–5633 |
| <a id="k-09"></a>K-09 | A:5634–5643 |
| <a id="k-10"></a>K-10 | A:5644–5652 |
| <a id="k-11"></a>K-11 | A:5653–5661 |
| <a id="k-12"></a>K-12 | A:5662–5670 |
| <a id="k-13"></a>K-13 | A:5671–5680 |
| <a id="k-14"></a>K-14 | A:5681–5689 |
| <a id="k-15"></a>K-15 | A:5690–5696 |
| <a id="k-16"></a>K-16 | A:5697–5705 |
| <a id="k-17"></a>K-17 | A:5706–5715 |
| <a id="k-18"></a>K-18 | A:5716–5723 |
| <a id="k-19"></a>K-19 | A:5724–5730 |
| <a id="k-20"></a>K-20 | A:5731–5737 |
| <a id="k-21"></a>K-21 | A:5738–5744 |
| <a id="k-22"></a>K-22 | A:5745–5752 |
| <a id="k-23"></a>K-23 | A:5753–5761 |
| <a id="k-24"></a>K-24 | A:5762–5770 |
| <a id="k-25"></a>K-25 | A:5771–5779 |
| <a id="k-26"></a>K-26 | A:5780–5788 |
| <a id="k-27"></a>K-27 | A:5789–5798 |
| <a id="k-28"></a>K-28 | A:5799–5810 |
| <a id="k-29"></a>K-29 | A:5811–5816 |
| <a id="k-30"></a>K-30 | A:5817–5827 |
| <a id="k-31"></a>K-31 | A:5828–5839 |
| <a id="k-32"></a>K-32 | A:5840–5851 |
| <a id="k-33"></a>K-33 | A:5852–5862 |
| <a id="k-34"></a>K-34 | A:5863–5876 |
| <a id="k-35"></a>K-35 | A:5877–5890 |
| <a id="k-36"></a>K-36 | A:5891–5903 |
| <a id="k-37"></a>K-37 | A:5904–5916 |
| <a id="k-38"></a>K-38 | A:5917–5929 |
| <a id="k-39"></a>K-39 | A:5930–5942 |
| <a id="k-40"></a>K-40 | A:5943–5955 |
| <a id="k-41"></a>K-41 | A:5956–5968 |
| <a id="k-42"></a>K-42 | A:5969–5981 |
| <a id="k-43"></a>K-43 | A:5982–5994 |
| <a id="k-44"></a>K-44 | A:5995–6007 |
| <a id="k-45"></a>K-45 | A:6008–6020 |
| <a id="k-46"></a>K-46 | A:6021–6033 |
| <a id="k-47"></a>K-47 | A:6034–6046 |
| <a id="k-48"></a>K-48 | A:6047–6059 |
| <a id="k-49"></a>K-49 | A:6060–6072 |
| <a id="k-50"></a>K-50 | A:6073–6085 |
| <a id="k-51"></a>K-51 | A:6086–6098 |
| <a id="k-52"></a>K-52 | A:6099–6111 |
| <a id="k-53"></a>K-53 | A:6112–6124 |
| <a id="k-54"></a>K-54 | A:6125–6137 |
| <a id="k-55"></a>K-55 | A:6138–6150 |
| <a id="k-56"></a>K-56 | A:6151–6163 |
| <a id="k-57"></a>K-57 | A:6164–6177 |
| <a id="k-58"></a>K-58 | A:6178–6190 |
| <a id="k-59"></a>K-59 | A:6191–6203 |
| <a id="k-60"></a>K-60 | A:6204–6215 |
| <a id="k-61"></a>K-61 | A:6216–6228 |
| <a id="k-62"></a>K-62 | A:6229–6241 |
| <a id="k-63"></a>K-63 | A:6242–6254 |
| <a id="k-64"></a>K-64 | A:6255–6267 |
| <a id="k-65"></a>K-65 | A:6268–6280 |
| <a id="k-66"></a>K-66 | A:6281–6293 |
| <a id="k-67"></a>K-67 | A:6294–6306 |
| <a id="k-68"></a>K-68 | A:6307–6319 |
| <a id="k-69"></a>K-69 | A:6320–6331 |
| <a id="k-70"></a>K-70 | A:6332–6343 |
| <a id="k-71"></a>K-71 | A:6344–6356 |
| <a id="k-72"></a>K-72 | A:6357–6368 |
| <a id="k-73"></a>K-73 | A:6369–6382 |
| <a id="k-74"></a>K-74 | A:6383–6395 |
| <a id="k-75"></a>K-75 | A:6396–6410 |
| <a id="k-76"></a>K-76 | A:6411–6423 |
| <a id="k-77"></a>K-77 | A:6424–6437 |
| <a id="k-78"></a>K-78 | A:6438–6452 |
| <a id="k-79"></a>K-79 | A:6453–6466 |
| <a id="k-80"></a>K-80 | A:6467–6481 |
| <a id="k-81"></a>K-81 | A:6482–6494 |
| <a id="k-82"></a>K-82 | A:6495–6508 |
| <a id="k-83"></a>K-83 | A:6509–6524 |
| <a id="k-84"></a>K-84 | A:6525–6538 |
| <a id="k-85"></a>K-85 | A:6539–6552 |
| <a id="k-86"></a>K-86 | A:6553–6566 |
| <a id="k-87"></a>K-87 | A:6567–6579 |
| <a id="k-88"></a>K-88 | A:6580–6592 |
| <a id="k-89"></a>K-89 | A:6593–6608 |
| <a id="k-90"></a>K-90 | A:6609–6620 |
| <a id="k-91"></a>K-91 | A:6621–6631 |
| <a id="k-92"></a>K-92 | A:6632–6641 |
| <a id="k-93"></a>K-93 | A:6642–6652 |
| <a id="k-94"></a>K-94 | A:6653–6662 |
| <a id="k-95"></a>K-95 | A:6663–6673 |
| <a id="k-96"></a>K-96 | A:6674–6683 |
| <a id="k-97"></a>K-97 | A:6684–6694 |
| <a id="k-98"></a>K-98 | A:6695–6706 |
| <a id="k-99"></a>K-99 | A:6707–6718 |
| <a id="k-100"></a>K-100 | A:6719–6730 |
| <a id="k-101"></a>K-101 | A:6731–6742 |
| <a id="k-102"></a>K-102 | A:6743–6752 |
| <a id="k-103"></a>K-103 | A:6753–6762 |
| <a id="k-104"></a>K-104 | A:6763–6772 |
| <a id="k-105"></a>K-105 | A:6773–6782 |
| <a id="k-106"></a>K-106 | A:6783–6793 |
| <a id="k-107"></a>K-107 | A:6794–6804 |
| <a id="k-108"></a>K-108 | A:6805–6815 |
| <a id="k-109"></a>K-109 | A:6816–6826 |
| <a id="k-110"></a>K-110 | A:6827–6837 |
| <a id="k-111"></a>K-111 | A:6838–6848 |
| <a id="k-112"></a>K-112 | A:6849–6859 |
| <a id="k-113"></a>K-113 | A:6860–6870 |
| <a id="k-114"></a>K-114 | A:6871–6880 |
| <a id="k-115"></a>K-115 | A:6881–6890 |
| <a id="k-116"></a>K-116 | A:6891–6900 |
| <a id="k-117"></a>K-117 | A:6901–6910 |
| <a id="k-118"></a>K-118 | A:6911–6920 |
| <a id="k-119"></a>K-119 | A:6921–6930 |
| <a id="k-120"></a>K-120 | A:6931–6940 |
| <a id="k-121"></a>K-121 | A:6941–6950 |
| <a id="k-122"></a>K-122 | A:6951–6960 |
| <a id="k-123"></a>K-123 | A:6961–6966 |
| <a id="k-124"></a>K-124 | A:6967–6976 |
| <a id="k-125"></a>K-125 | A:6977–6985 |
| <a id="k-126"></a>K-126 | A:6986–6991 |
| <a id="k-127"></a>K-127 | A:6992–6999 |
| <a id="k-128"></a>K-128 | A:7000–7009 |
| <a id="k-129"></a>K-129 | A:7010–7017 |
| <a id="k-130"></a>K-130 | A:7018–7027 |
| <a id="k-131"></a>K-131 | A:7028–7036 |
| <a id="k-132"></a>K-132 | A:7037–7045 |
| <a id="k-133"></a>K-133 | A:7046–7055 |
| <a id="k-134"></a>K-134 | A:7056–7064 |
| <a id="k-135"></a>K-135 | A:7065–7074 |
| <a id="k-136"></a>K-136 | A:7075–7084 |
| <a id="k-137"></a>K-137 | A:7085–7093 |
| <a id="k-138"></a>K-138 | A:7094–7103 |
| <a id="k-139"></a>K-139 | A:7104–7114 |
| <a id="k-140"></a>K-140 | A:7115–7125 |
| <a id="k-141"></a>K-141 | A:7126–7136 |
| <a id="k-142"></a>K-142 | A:7137–7147 |
| <a id="k-143"></a>K-143 | A:7148–7158 |
| <a id="k-144"></a>K-144 | A:7159–7169 |
| <a id="k-145"></a>K-145 | A:7170–7180 |
| <a id="k-146"></a>K-146 | A:7181–7191 |
| <a id="k-147"></a>K-147 | A:7192–7202 |
| <a id="k-148"></a>K-148 | A:7203–7212 |
| <a id="k-149"></a>K-149 | A:7213–7222 |
| <a id="k-150"></a>K-150 | A:7223–7232 |
| <a id="k-151"></a>K-151 | A:7233–7243 |
| <a id="k-152"></a>K-152 | A:7244–7253 |
| <a id="k-153"></a>K-153 | A:7254–7263 |
| <a id="k-154"></a>K-154 | A:7264–7273 |
| <a id="k-155"></a>K-155 | A:7274–7283 |
| <a id="k-156"></a>K-156 | A:7284–7293 |
| <a id="k-157"></a>K-157 | A:7294–7303 |
| <a id="k-158"></a>K-158 | A:7304–7313 |
| <a id="k-159"></a>K-159 | A:7314–7323 |
| <a id="k-160"></a>K-160 | A:7324–7333 |
| <a id="k-161"></a>K-161 | A:7334–7343 |
| <a id="k-162"></a>K-162 | A:7344–7353 |
| <a id="k-163"></a>K-163 | A:7354–7362 |
| <a id="k-164"></a>K-164 | A:7363–7371 |
| <a id="k-165"></a>K-165 | A:7372–7380 |
| <a id="k-166"></a>K-166 | A:7381–7389 |
| <a id="k-167"></a>K-167 | A:7390–7397 |
| <a id="k-168"></a>K-168 | A:7398–7406 |
| <a id="k-169"></a>K-169 | A:7407–7415 |
| <a id="k-170"></a>K-170 | A:7416–7424 |
| <a id="k-171"></a>K-171 | A:7425–7433 |
| <a id="k-172"></a>K-172 | A:7434–7442 |
| <a id="k-173"></a>K-173 | A:7443–7451 |
| <a id="k-174"></a>K-174 | A:7452–7460 |
| <a id="k-175"></a>K-175 | A:7461–7469 |
| <a id="k-176"></a>K-176 | A:7470–7478 |
| <a id="k-177"></a>K-177 | A:7479–7487 |
| <a id="k-178"></a>K-178 | A:7488–7496 |
| <a id="k-179"></a>K-179 | A:7497–7505 |
| <a id="k-180"></a>K-180 | A:7506–7514 |
| <a id="k-181"></a>K-181 | A:7515–7523 |
| <a id="k-182"></a>K-182 | A:7524–7532 |
| <a id="k-183"></a>K-183 | A:7533–7541 |
| <a id="k-184"></a>K-184 | A:7542–7550 |
| <a id="k-185"></a>K-185 | A:7551–7559 |
| <a id="k-186"></a>K-186 | A:7560–7568 |
| <a id="k-187"></a>K-187 | A:7569–7577 |
| <a id="k-188"></a>K-188 | A:7578–7586 |
| <a id="k-189"></a>K-189 | A:7587–7595 |
| <a id="k-190"></a>K-190 | A:7596–7604 |
| <a id="k-191"></a>K-191 | A:7605–7613 |
| <a id="k-192"></a>K-192 | A:7614–7622 |
| <a id="k-193"></a>K-193 | A:7623–7631 |
| <a id="k-194"></a>K-194 | A:7632–7640 |
| <a id="k-195"></a>K-195 | A:7641–7649 |
| <a id="k-196"></a>K-196 | A:7650–7658 |
| <a id="k-197"></a>K-197 | A:7659–7667 |
| <a id="k-198"></a>K-198 | A:7668–7676 |
| <a id="k-199"></a>K-199 | A:7677–7685 |
| <a id="k-200"></a>K-200 | A:7686–7694 |
| <a id="k-201"></a>K-201 | A:7695–7703 |
| <a id="k-202"></a>K-202 | A:7704–7712 |
| <a id="k-203"></a>K-203 | A:7713–7721 |
| <a id="k-204"></a>K-204 | A:7722–7730 |
| <a id="k-205"></a>K-205 | A:7731–7739 |
| <a id="k-206"></a>K-206 | A:7740–7748 |
| <a id="k-207"></a>K-207 | A:7749–7758 |
| <a id="k-208"></a>K-208 | A:7759–7770 |
| <a id="k-209"></a>K-209 | A:7771–7782 |
| <a id="k-210"></a>K-210 | A:7783–7795 |
| <a id="k-211"></a>K-211 | A:7796–7807 |
| <a id="k-212"></a>K-212 | A:7808–7817 |
| <a id="k-213"></a>K-213 | A:7818–7828 |
| <a id="k-214"></a>K-214 | A:7829–7842 |
| <a id="k-215"></a>K-215 | A:7843–7855 |
| <a id="k-216"></a>K-216 | A:7856–7867 |
| <a id="k-217"></a>K-217 | A:7868–7880 |
| <a id="k-218"></a>K-218 | A:7881–7893 |
| <a id="k-219"></a>K-219 | A:7894–7906 |
| <a id="k-220"></a>K-220 | A:7907–7919 |
| <a id="k-221"></a>K-221 | A:7920–7931 |
| <a id="k-222"></a>K-222 | A:7932–7943 |
| <a id="k-223"></a>K-223 | A:7944–7957 |
| <a id="k-224"></a>K-224 | A:7958–7968 |
| <a id="k-225"></a>K-225 | A:7969–7981 |
| <a id="k-226"></a>K-226 | A:7982–7994 |
| <a id="k-227"></a>K-227 | A:7995–8007 |
| <a id="k-228"></a>K-228 | A:8008–8021 |
| <a id="k-229"></a>K-229 | A:8022–8033 |
| <a id="k-230"></a>K-230 | A:8034–8045 |
| <a id="k-231"></a>K-231 | A:8046–8056 |
| <a id="k-232"></a>K-232 | A:8057–8068 |
| <a id="k-233"></a>K-233 | A:8069–8077 |
| <a id="k-234"></a>K-234 | A:8078–8086 |
| <a id="k-235"></a>K-235 | A:8087–8096 |
| <a id="k-236"></a>K-236 | A:8097–8105 |
| <a id="k-237"></a>K-237 | A:8106–8115 |
| <a id="k-238"></a>K-238 | A:8116–8125 |
| <a id="k-239"></a>K-239 | A:8126–8135 |
| <a id="k-240"></a>K-240 | A:8136–8145 |
| <a id="k-241"></a>K-241 | A:8146–8164 |
| <a id="k-242"></a>K-242 | A:8165–8173 |
| <a id="k-243"></a>K-243 | A:8174–8183 |
| <a id="k-244"></a>K-244 | A:8184–8193 |
| <a id="k-245"></a>K-245 | A:8194–8203 |
| <a id="k-246"></a>K-246 | A:8204–8213 |
| <a id="k-247"></a>K-247 | A:8214–8223 |
| <a id="k-248"></a>K-248 | A:8224–8233 |
| <a id="k-249"></a>K-249 | A:8234–8242 |
| <a id="k-250"></a>K-250 | A:8243–8252 |
| <a id="k-251"></a>K-251 | A:8253–8262 |
| <a id="k-252"></a>K-252 | A:8263–8272 |
| <a id="k-253"></a>K-253 | A:8273–8281 |
| <a id="k-254"></a>K-254 | A:8282–8292 |
| <a id="k-255"></a>K-255 | A:8293–8303 |
| <a id="k-256"></a>K-256 | A:8304–8312 |
| <a id="k-257"></a>K-257 | A:8313–8322 |
| <a id="k-258"></a>K-258 | A:8323–8332 |
| <a id="k-259"></a>K-259 | A:8333–8342 |
| <a id="k-260"></a>K-260 | A:8343–8352 |
| <a id="k-261"></a>K-261 | A:8353–8362 |
| <a id="k-262"></a>K-262 | A:8363–8373 |
| <a id="k-263"></a>K-263 | A:8374–8383 |
| <a id="k-264"></a>K-264 | A:8384–8394 |
| <a id="k-265"></a>K-265 | A:8395–8405 |
| <a id="k-266"></a>K-266 | A:8406–8415 |
| <a id="k-267"></a>K-267 | A:8416–8425 |
| <a id="k-268"></a>K-268 | A:8426–8435 |
| <a id="k-269"></a>K-269 | A:8436–8444 |
| <a id="k-270"></a>K-270 | A:8445–8453 |
| <a id="k-271"></a>K-271 | A:8454–8463 |
| <a id="k-272"></a>K-272 | A:8464–8473 |
| <a id="k-273"></a>K-273 | A:8474–8483 |
| <a id="k-274"></a>K-274 | A:8484–8493 |
| <a id="k-275"></a>K-275 | A:8494–8503 |
| <a id="k-276"></a>K-276 | A:8504–8512 |
| <a id="k-277"></a>K-277 | A:8513–8520 |
| <a id="k-278"></a>K-278 | A:8521–8528 |
| <a id="k-279"></a>K-279 | A:8529–8537 |
| <a id="k-280"></a>K-280 | A:8538–8545 |
| <a id="k-281"></a>K-281 | A:8546–8553 |
| <a id="k-282"></a>K-282 | A:8554–8563 |
| <a id="k-283"></a>K-283 | A:8564–8572 |
| <a id="k-284"></a>K-284 | A:8573–8581 |
| <a id="k-285"></a>K-285 | A:8582–8589 |
| <a id="k-286"></a>K-286 | A:8590–8598 |
| <a id="k-287"></a>K-287 | A:8599–8607 |
| <a id="k-288"></a>K-288 | A:8608–8617 |
| <a id="k-289"></a>K-289 | A:8618–8626 |
| <a id="k-290"></a>K-290 | A:8627–8636 |
| <a id="k-291"></a>K-291 | A:8637–8645 |
| <a id="k-292"></a>K-292 | A:8646–8653 |
| <a id="k-293"></a>K-293 | A:8654–8663 |
| <a id="k-294"></a>K-294 | A:8664–8673 |
| <a id="k-295"></a>K-295 | A:8674–8684 |
| <a id="k-296"></a>K-296 | A:8685–8696 |
| <a id="k-297"></a>K-297 | A:8697–8706 |
| <a id="k-298"></a>K-298 | A:8707–8715 |
| <a id="k-299"></a>K-299 | A:8716–8724 |
| <a id="k-300"></a>K-300 | A:8725–8734 |
| <a id="k-301"></a>K-301 | A:8735–8743 |
| <a id="k-302"></a>K-302 | A:8744–8753 |
| <a id="k-303"></a>K-303 | A:8754–8761 |
| <a id="k-304"></a>K-304 | A:8762–8769 |
| <a id="k-305"></a>K-305 | A:8770–8779 |
| <a id="k-306"></a>K-306 | A:8780–8789 |
| <a id="k-307"></a>K-307 | A:8790–8800 |
| <a id="k-308"></a>K-308 | A:8801–8810 |
| <a id="k-309"></a>K-309 | A:8811–8820 |
| <a id="k-310"></a>K-310 | A:8821–8833 |
| <a id="k-311"></a>K-311 | A:8834–8844 |
| <a id="k-312"></a>K-312 | A:8845–8855 |
| <a id="k-313"></a>K-313 | A:8856–8867 |
| <a id="k-314"></a>K-314 | A:8868–8878 |
| <a id="k-315"></a>K-315 | A:8879–8889 |
| <a id="k-316"></a>K-316 | A:8890–8901 |
| <a id="k-317"></a>K-317 | A:8902–8913 |
| <a id="k-318"></a>K-318 | A:8914–8925 |
| <a id="k-319"></a>K-319 | A:8926–8936 |
| <a id="k-320"></a>K-320 | A:8937–8948 |
| <a id="k-321"></a>K-321 | A:8949–8961 |
| <a id="k-322"></a>K-322 | A:8962–8972 |
| <a id="k-323"></a>K-323 | A:8973–8982 |
| <a id="k-324"></a>K-324 | A:8983–8993 |
| <a id="k-325"></a>K-325 | A:8994–9004 |
| <a id="k-326"></a>K-326 | A:9005–9015 |
| <a id="k-327"></a>K-327 | A:9016–9025 |
| <a id="k-328"></a>K-328 | A:9026–9035 |
| <a id="k-329"></a>K-329 | A:9036–9045 |
| <a id="k-330"></a>K-330 | A:9046–9054 |
| <a id="k-331"></a>K-331 | A:9055–9063 |
| <a id="k-332"></a>K-332 | A:9064–9072 |
| <a id="k-333"></a>K-333 | A:9073–9081 |
| <a id="k-334"></a>K-334 | A:9082–9090 |
| <a id="k-335"></a>K-335 | A:9091–9099 |
| <a id="k-336"></a>K-336 | A:9100–9108 |
| <a id="k-337"></a>K-337 | A:9109–9116 |
| <a id="k-338"></a>K-338 | A:9117–9124 |
| <a id="k-339"></a>K-339 | A:9125–9133 |
| <a id="k-340"></a>K-340 | A:9134–9141 |
| <a id="k-341"></a>K-341 | A:9142–9152 |
| <a id="k-342"></a>K-342 | A:9153–9161 |
| <a id="k-343"></a>K-343 | A:9162–9170 |
| <a id="k-344"></a>K-344 | A:9171–9179 |
| <a id="k-345"></a>K-345 | A:9180–9187 |
| <a id="k-346"></a>K-346 | A:9188–9195 |
| <a id="k-347"></a>K-347 | A:9196–9203 |
| <a id="k-348"></a>K-348 | A:9204–9214 |
| <a id="k-349"></a>K-349 | A:9215–9224 |
| <a id="k-350"></a>K-350 | A:9225–9236 |
| <a id="k-351"></a>K-351 | A:9237–9247 |
| <a id="k-352"></a>K-352 | A:9248–9258 |
| <a id="k-353"></a>K-353 | A:9259–9268 |
| <a id="k-354"></a>K-354 | A:9269–9278 |
| <a id="k-355"></a>K-355 | A:9279–9288 |
| <a id="k-356"></a>K-356 | A:9289–9298 |
| <a id="k-357"></a>K-357 | A:9299–9310 |
| <a id="k-358"></a>K-358 | A:9311–9320 |
| <a id="k-359"></a>K-359 | A:9321–9331 |
| <a id="k-360"></a>K-360 | A:9332–9339 |
| <a id="k-361"></a>K-361 | A:9340–9348 |
| <a id="k-362"></a>K-362 | A:9349–9357 |
| <a id="k-363"></a>K-363 | A:9358–9366 |
| <a id="k-364"></a>K-364 | A:9367–9375 |
| <a id="k-365"></a>K-365 | A:9376–9383 |
| <a id="k-366"></a>K-366 | A:9384–9390 |
| <a id="k-367"></a>K-367 | A:9391–9400 |
| <a id="k-368"></a>K-368 | A:9401–9408 |
| <a id="k-369"></a>K-369 | A:9409–9417 |
| <a id="k-370"></a>K-370 | A:9418–9424 |
| <a id="k-371"></a>K-371 | A:9425–9433 |
| <a id="k-372"></a>K-372 | A:9434–9442 |
| <a id="k-373"></a>K-373 | A:9443–9452 |
| <a id="k-374"></a>K-374 | A:9453–9461 |
| <a id="k-375"></a>K-375 | A:9462–9470 |
| <a id="k-376"></a>K-376 | A:9471–9479 |
| <a id="k-377"></a>K-377 | A:9480–9488 |
| <a id="k-378"></a>K-378 | A:9489–9497 |
| <a id="k-379"></a>K-379 | A:9498–9506 |
| <a id="k-380"></a>K-380 | A:9507–9516 |
| <a id="k-381"></a>K-381 | A:9517–9526 |
| <a id="k-382"></a>K-382 | A:9527–9535 |
| <a id="k-383"></a>K-383 | A:9536–9545 |
| <a id="k-384"></a>K-384 | A:9546–9558 |
| <a id="k-385"></a>K-385 | A:9559–9567 |
| <a id="k-386"></a>K-386 | A:9568–9575 |
| <a id="k-387"></a>K-387 | A:9576–9585 |
| <a id="k-388"></a>K-388 | A:9586–9594 |
| <a id="k-389"></a>K-389 | A:9595–9605 |
| <a id="k-390"></a>K-390 | A:9606–9615 |
| <a id="k-391"></a>K-391 | A:9616–9629 |
| <a id="k-392"></a>K-392 | A:9630–9638 |
| <a id="k-393"></a>K-393 | A:9639–9652 |
| <a id="k-394"></a>K-394 | A:9653–9664 |
| <a id="k-395"></a>K-395 | A:9665–9677 |
| <a id="k-396"></a>K-396 | A:9678–9689 |
| <a id="k-397"></a>K-397 | A:9690–9699 |
| <a id="k-398"></a>K-398 | A:9700–9707 |
| <a id="k-399"></a>K-399 | A:9708–9718 |
| <a id="k-400"></a>K-400 | A:9719–9731 |
| <a id="k-401"></a>K-401 | A:9732–9741 |
| <a id="k-402"></a>K-402 | A:9742–9753 |
| <a id="k-403"></a>K-403 | A:9754–9764 |
| <a id="k-404"></a>K-404 | A:9765–9774 |
| <a id="k-405"></a>K-405 | A:9775–9784 |
| <a id="k-406"></a>K-406 | A:9785–9795 |
| <a id="k-407"></a>K-407 | A:9796–9805 |
| <a id="k-408"></a>K-408 | A:9806–9814 |
| <a id="k-409"></a>K-409 | A:9815–9823 |
| <a id="k-410"></a>K-410 | A:9824–9833 |
| <a id="k-411"></a>K-411 | A:9834–9844 |
| <a id="k-412"></a>K-412 | A:9845–9855 |
| <a id="k-413"></a>K-413 | A:9856–9864 |
| <a id="k-414"></a>K-414 | A:9865–9875 |
| <a id="k-415"></a>K-415 | A:9876–9885 |
| <a id="k-416"></a>K-416 | A:9886–9895 |
| <a id="k-417"></a>K-417 | A:9896–9905 |
| <a id="k-418"></a>K-418 | A:9906–9914 |
| <a id="k-419"></a>K-419 | A:9915–9924 |
| <a id="k-420"></a>K-420 | A:9925–9933 |
| <a id="k-421"></a>K-421 | A:9934–9941 |
| <a id="k-422"></a>K-422 | A:9942–9951 |
| <a id="k-423"></a>K-423 | A:9952–9961 |
| <a id="k-424"></a>K-424 | A:9962–9971 |
| <a id="k-425"></a>K-425 | A:9972–9980 |
| <a id="k-426"></a>K-426 | A:9981–9990 |
| <a id="k-427"></a>K-427 | A:9991–10000 |
| <a id="k-428"></a>K-428 | A:10001–10010 |
| <a id="k-429"></a>K-429 | A:10011–10020 |
| <a id="k-430"></a>K-430 | A:10021–10030 |
| <a id="k-431"></a>K-431 | A:10031–10039 |
| <a id="k-432"></a>K-432 | A:10040–10047 |
| <a id="k-433"></a>K-433 | A:10048–10055 |
| <a id="k-434"></a>K-434 | A:10056–10063 |
| <a id="k-435"></a>K-435 | A:10064–10072 |
| <a id="k-436"></a>K-436 | A:10073–10080 |
| <a id="k-437"></a>K-437 | A:10081–10088 |
| <a id="k-438"></a>K-438 | A:10089–10096 |
| <a id="k-439"></a>K-439 | A:10097–10104 |
| <a id="k-440"></a>K-440 | A:10105–10112 |
| <a id="k-441"></a>K-441 | A:10113–10123 |
| <a id="k-442"></a>K-442 | A:10124–10134 |
| <a id="k-443"></a>K-443 | A:10135–10142 |
| <a id="k-444"></a>K-444 | A:10143–10151 |
| <a id="k-445"></a>K-445 | A:10152–10160 |
| <a id="k-446"></a>K-446 | A:10161–10168 |
| <a id="k-447"></a>K-447 | A:10169–10175 |
| <a id="k-448"></a>K-448 | A:10176–10183 |
| <a id="k-449"></a>K-449 | A:10184–10191 |
| <a id="k-450"></a>K-450 | A:10192–10199 |
| <a id="k-451"></a>K-451 | A:10200–10207 |
| <a id="k-452"></a>K-452 | A:10208–10214 |
| <a id="k-453"></a>K-453 | A:10215–10220 |
| <a id="k-454"></a>K-454 | A:10221–10227 |
| <a id="k-455"></a>K-455 | A:10228–10233 |
| <a id="k-456"></a>K-456 | A:10234–10239 |
| <a id="k-457"></a>K-457 | A:10240–10246 |
| <a id="k-458"></a>K-458 | A:10247–10253 |
| <a id="k-459"></a>K-459 | A:10254–10260 |
| <a id="k-460"></a>K-460 | A:10261–10269 |
| <a id="k-461"></a>K-461 | A:10270–10276 |
| <a id="k-462"></a>K-462 | A:10277–10283 |
| <a id="k-463"></a>K-463 | A:10284–10289 |
| <a id="k-464"></a>K-464 | A:10290–10297 |
| <a id="k-465"></a>K-465 | A:10298–10306 |
| <a id="k-466"></a>K-466 | A:10307–10315 |
| <a id="k-467"></a>K-467 | A:10316–10326 |
| <a id="k-468"></a>K-468 | A:10327–10338 |
| <a id="k-469"></a>K-469 | A:10339–10348 |
| <a id="k-470"></a>K-470 | A:10349–10360 |
| <a id="k-471"></a>K-471 | A:10361–10372 |
| <a id="k-472"></a>K-472 | A:10373–10383 |
| <a id="k-473"></a>K-473 | A:10384–10393 |
| <a id="k-474"></a>K-474 | A:10394–10401 |
| <a id="k-475"></a>K-475 | A:10402–10408 |
| <a id="k-476"></a>K-476 | A:10409–10420 |
| <a id="k-477"></a>K-477 | A:10421–10433 |
| <a id="k-478"></a>K-478 | A:10434–10446 |
| <a id="k-479"></a>K-479 | A:10447–10460 |
| <a id="k-480"></a>K-480 | A:10461–10471 |
| <a id="k-481"></a>K-481 | A:10472–10485 |
| <a id="k-482"></a>K-482 | A:10486–10498 |
| <a id="k-483"></a>K-483 | A:10499–10511 |
| <a id="k-484"></a>K-484 | A:10512–10523 |
| <a id="k-485"></a>K-485 | A:10524–10534 |
| <a id="k-486"></a>K-486 | A:10535–10542 |
| <a id="k-487"></a>K-487 | A:10543–10553 |
| <a id="k-488"></a>K-488 | A:10554–10565 |
| <a id="k-489"></a>K-489 | A:10566–10577 |
| <a id="k-490"></a>K-490 | A:10578–10588 |
| <a id="k-491"></a>K-491 | A:10589–10599 |
| <a id="k-492"></a>K-492 | A:10600–10610 |
| <a id="k-493"></a>K-493 | A:10611–10620 |
| <a id="k-494"></a>K-494 | A:10621–10629 |
| <a id="k-495"></a>K-495 | A:10630–10638 |
| <a id="k-496"></a>K-496 | A:10639–10648 |
| <a id="k-497"></a>K-497 | A:10649–10659 |
| <a id="k-498"></a>K-498 | A:10660–10669 |
| <a id="k-499"></a>K-499 | A:10670–10679 |
| <a id="k-500"></a>K-500 | A:10680–10689 |
| <a id="k-501"></a>K-501 | A:10690–10697 |
| <a id="k-502"></a>K-502 | A:10698–10706 |
| <a id="k-503"></a>K-503 | A:10707–10715 |
| <a id="k-504"></a>K-504 | A:10716–10723 |
| <a id="k-505"></a>K-505 | A:10724–10731 |
| <a id="k-506"></a>K-506 | A:10732–10740 |
| <a id="k-507"></a>K-507 | A:10741–10749 |
| <a id="k-508"></a>K-508 | A:10750–10758 |
| <a id="k-509"></a>K-509 | A:10759–10767 |
| <a id="k-510"></a>K-510 | A:10768–10776 |
| <a id="k-511"></a>K-511 | A:10777–10785 |
| <a id="k-512"></a>K-512 | A:10786–10795 |
| <a id="k-513"></a>K-513 | A:10796–10803 |
| <a id="k-514"></a>K-514 | A:10804–10812 |
| <a id="k-515"></a>K-515 | A:10813–10819 |
| <a id="k-516"></a>K-516 | A:10820–10826 |
| <a id="k-517"></a>K-517 | A:10827–10832 |
| <a id="k-518"></a>K-518 | A:10833–10838 |
| <a id="k-519"></a>K-519 | A:10839–10846 |
| <a id="k-520"></a>K-520 | A:10847–10852 |
| <a id="k-521"></a>K-521 | A:10853–10861 |
| <a id="k-522"></a>K-522 | A:10862–10869 |
| <a id="k-523"></a>K-523 | A:10870–10878 |
| <a id="k-524"></a>K-524 | A:10879–10887 |
| <a id="k-525"></a>K-525 | A:10888–10898 |
| <a id="k-526"></a>K-526 | A:10899–10910 |
| <a id="k-527"></a>K-527 | A:10911–10922 |
| <a id="k-528"></a>K-528 | A:10923–10930 |
| <a id="k-529"></a>K-529 | A:10931–10937 |
| <a id="k-530"></a>K-530 | A:10938–10943 |
| <a id="k-531"></a>K-531 | A:10944–10951 |
| <a id="k-532"></a>K-532 | A:10952–10960 |
| <a id="k-533"></a>K-533 | A:10961–10968 |
| <a id="k-534"></a>K-534 | A:10969–10976 |
| <a id="k-535"></a>K-535 | A:10977–10984 |
| <a id="k-536"></a>K-536 | A:10985–10993 |
| <a id="k-537"></a>K-537 | A:10994–11001 |
| <a id="k-538"></a>K-538 | A:11002–11010 |
| <a id="k-539"></a>K-539 | A:11011–11018 |
| <a id="k-540"></a>K-540 | A:11019–11026 |
| <a id="k-541"></a>K-541 | A:11027–11036 |
| <a id="k-542"></a>K-542 | A:11037–11046 |
| <a id="k-543"></a>K-543 | A:11047–11055 |
| <a id="k-544"></a>K-544 | A:11056–11064 |
| <a id="k-545"></a>K-545 | A:11065–11073 |
| <a id="k-546"></a>K-546 | A:11074–11082 |
| <a id="k-547"></a>K-547 | A:11083–11090 |
| <a id="k-548"></a>K-548 | A:11091–11098 |
| <a id="k-549"></a>K-549 | A:11099–11107 |
| <a id="k-550"></a>K-550 | A:11108–11116 |
| <a id="k-551"></a>K-551 | A:11117–11125 |
| <a id="k-552"></a>K-552 | A:11126–11135 |
| <a id="k-553"></a>K-553 | A:11136–11145 |
| <a id="k-554"></a>K-554 | A:11146–11154 |
| <a id="k-555"></a>K-555 | A:11155–11165 |
| <a id="k-556"></a>K-556 | A:11166–11176 |
| <a id="k-557"></a>K-557 | A:11177–11186 |
| <a id="k-558"></a>K-558 | A:11187–11195 |
| <a id="k-559"></a>K-559 | A:11196–11205 |
| <a id="k-560"></a>K-560 | A:11206–11213 |
| <a id="k-561"></a>K-561 | A:11214–11221 |
| <a id="k-562"></a>K-562 | A:11222–11228 |
| <a id="k-563"></a>K-563 | A:11229–11236 |
| <a id="k-564"></a>K-564 | A:11237–11244 |
| <a id="k-565"></a>K-565 | A:11245–11252 |
| <a id="k-566"></a>K-566 | A:11253–11261 |
| <a id="k-567"></a>K-567 | A:11262–11270 |
| <a id="k-568"></a>K-568 | A:11271–11278 |
| <a id="k-569"></a>K-569 | A:11279–11286 |
| <a id="k-570"></a>K-570 | A:11287–11294 |
| <a id="k-571"></a>K-571 | A:11295–11302 |
| <a id="k-572"></a>K-572 | A:11303–11310 |
| <a id="k-573"></a>K-573 | A:11311–11317 |
| <a id="k-574"></a>K-574 | A:11318–11325 |
| <a id="k-575"></a>K-575 | A:11326–11332 |
| <a id="k-576"></a>K-576 | A:11333–11340 |
| <a id="k-577"></a>K-577 | A:11341–11348 |
| <a id="k-578"></a>K-578 | B:2217–2223 |
| <a id="k-579"></a>K-579 | C:1829–1835 |
| <a id="k-580"></a>K-580 | C:1837–1843 |
| <a id="k-581"></a>K-581 | C:1845–1850 |
| <a id="k-582"></a>K-582 | C:1852–1857 |
| <a id="k-583"></a>K-583 | C:1859–1863 |
| <a id="k-584"></a>K-584 | C:1865–1870 |
| <a id="k-585"></a>K-585 | C:1872–1878 |
| <a id="k-586"></a>K-586 | C:1880–1885 |
| <a id="k-587"></a>K-587 | C:1887–1891 |
| <a id="k-588"></a>K-588 | C:1893–1897 |
| <a id="k-589"></a>K-589 | C:1899–1905 |
| <a id="k-590"></a>K-590 | C:1907–1912 |
| <a id="k-591"></a>K-591 | C:1914–1918 |
| <a id="k-592"></a>K-592 | C:1920–1924 |
| <a id="k-593"></a>K-593 | C:1927–1934 |
| <a id="k-594"></a>K-594 | C:1936–1941 |
| <a id="k-595"></a>K-595 | C:1943–1947 |
| <a id="k-596"></a>K-596 | C:1950–1955 |
| <a id="k-597"></a>K-597 | C:1957–1961 |
| <a id="k-598"></a>K-598 | C:1963–1967 |
| <a id="k-599"></a>K-599 | C:1969–1973 |
| <a id="k-600"></a>K-600 | C:1975–1979 |
| <a id="k-601"></a>K-601 | C:1981–1985 |
| <a id="k-602"></a>K-602 | C:1987–1991 |
| <a id="k-603"></a>K-603 | C:1993–1997 |
| <a id="k-604"></a>K-604 | C:2000–2004 |
| <a id="k-605"></a>K-605 | C:2007–2011 |
| <a id="k-606"></a>K-606 | C:2014–2018 |
| <a id="k-607"></a>K-607 | C:2021–2025 |
| <a id="k-608"></a>K-608 | C:2028–2032 |
| <a id="k-609"></a>K-609 | C:2035–2039 |
| <a id="k-610"></a>K-610 | C:2042–2046 |
| <a id="k-611"></a>K-611 | C:2049–2053 |
| <a id="k-612"></a>K-612 | C:2056–2060 |
| <a id="k-613"></a>K-613 | C:2063–2067 |
| <a id="k-614"></a>K-614 | C:2070–2074 |
| <a id="k-615"></a>K-615 | C:2077–2081 |
| <a id="k-616"></a>K-616 | C:2084–2088 |
| <a id="k-617"></a>K-617 | C:2091–2095 |
| <a id="k-618"></a>K-618 | C:2098–2102 |
| <a id="k-619"></a>K-619 | C:2105–2109 |
| <a id="k-620"></a>K-620 | C:2112–2116 |
| <a id="k-621"></a>K-621 | C:2119–2123 |
| <a id="k-622"></a>K-622 | C:2125–2130 |
| <a id="k-623"></a>K-623 | C:2132–2137 |
| <a id="k-624"></a>K-624 | C:2139–2144 |
| <a id="k-625"></a>K-625 | C:2146–2151 |
| <a id="k-626"></a>K-626 | C:2153–2158 |
| <a id="k-627"></a>K-627 | C:2160–2165 |
| <a id="k-628"></a>K-628 | C:2167–2172 |
| <a id="k-629"></a>K-629 | C:2174–2179 |
| <a id="k-630"></a>K-630 | C:2181–2186 |
| <a id="k-631"></a>K-631 | C:2188–2193 |

## 2026-10-22 — Patch-free LLVM 23 bootstrap

- Pin the Linux and native macOS three-stage bootstrap to unmodified LLVM
  `llvmorg-23.1.3` (`0d261d1ca552c95a8f007e061c787ac7132fbcbc`), with source
  revision and cleanliness checks. No LLVM patches are applied.
- Unify native macOS arm64 and x86_64 configuration: select the SDK through
  `xcrun`, build compiler-rt for the host architecture, and use Apple ld for
  runtime libraries while bundled Clang drivers use Mach-O LLD for programs.
- Select bundled C++ runtimes through Clang configuration files and
  loader-relative search paths. Use classic dyld bindings to support coexistence
  with system libc++. Scope the LLVM 23.1.3 debugserver `<memory>` workaround to
  its C++ and Objective-C++ targets without editing upstream sources.
- Support incremental refresh of retained bootstrap stages with
  `COD_RECONFIGURE_BOOTSTRAP=1`. Extend the portable toolchain smoke checks with
  macOS runtime selection, AddressSanitizer and system-first libc++ coexistence.

Validation and evidence:

- Native macOS arm64 and x86_64 completed all three stages, installation and
  `cmake --build ./build`. Toolchain smoke, CoD expression evaluation, LLDB Python
  and debugging with Xcode's signed debugserver passed. The unified Intel rerun
  validated source commit `a956a2173e53c5e9f1a123a606d083a6b1b4b005`.
  See the [macOS environment guide](docs/MacOSBuildEnvironment.md),
  [arm64 record](toolchain/macos-validation.json) and
  [Intel record](toolchain/macos-x86_64-validation.json).
- The Intel debugger check passed after initial timeouts; the transient stall's
  cause is unconfirmed. The bundled ad-hoc debugserver remains unvalidated on
  Intel and was denied attachment on arm64. Intel functional smoke used `build/`;
  installed binaries were inspected. Full macOS project suites, installation
  relocation and universal distributions were not validated.
- The inherited Linux baseline records a successful three-stage bootstrap,
  installation, eight test suites, relocation and IDE acceptance on Debian 13
  x86_64. Those results apply to the recorded Linux revision; Linux was not rerun
  for this macOS integration. See the
  [Linux environment guide](docs/LinuxBuildEnvironment.md) and
  [Linux validation record](toolchain/linux-validation.json).
