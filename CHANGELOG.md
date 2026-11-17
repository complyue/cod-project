# Changelog

## 2026-11-17 — Refresh the current design discussion index

<a id="k-discussion-scan-20261117"></a>
**K-DISCUSSION-SCAN-20261117 — Route real choices without reopening settled behavior.** Agent editorial continuity under the user's repo-rescan instruction: D-12r32→33/D-14r122→123/D-23r24→25/registry855→856 corrects stale consumed-source, named owning-return and receiving-overlap summaries from H-262/H-263/H-267/H-576/H-593. Preserves C-52/C-54/C-65/C-72, P-131–P-135/P-143/P-155/P-354–P-355 and charter §§2/3.2–3.3/4.3/6.3/8/12; no new value edge, semantic choice or qualification. The registry links five current discussion topics and separates proof/implementation reviews, deferred increments and application research. Original H-262/H-263 evidence was checked at `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:4123–4146`; current baseline is `dca4f4815170148ba4d841c4d354c1edee734e0d` at the affected design paths. D-16, byte-identical D-18r113 and source423 remain unchanged. Documentation only; no code/build/tests.

## 2026-11-17 — Separate source decisions from remaining implementation work

<a id="k-tracker-boundaries-20261117"></a>
**K-TRACKER-BOUNDARIES-20261117 — Keep the bounded profile and its open work explicit.** Agent continuity under H-604's renewed derivation instruction: D-22r40→41/D-29r13→14/registry854→855 organizes selected result/receiving/init/root/generator obligations into proof/transport matrices. Existing C-65/C-72/C-95/C-96 and scoped relationships retain their charter §§2/3.2–3.3/4.3/6.3/8/12 rationale; no new semantic choice or implementation qualification. Additional variance/inference, internal-owner forwarding and field-level if returning remain optional extensions; bounded sound coverage does not replace an unproved obligation with runtime assumptions. Preserve source423, D-16, unchanged D-18r113 review and the separately recorded H-552 static-enforcement qualification gap. Baseline: `31f5c4e040b865a5e7d883e50bf7651d49bd733d` at the affected design paths. Documentation only; no local code/build/tests.

## 2026-11-17 — Bind yielded here references to the generator invocation

<a id="h-604"></a>
**H-604 — Here in a yield contract names this generator's lexical owner.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “同意，`yields ref T in here` 的 `here` 就指本次 generator 调用的 lexical scope owner，整个 for 期间保持不变。落文档，提交推送，然后继续处理追踪文档，没疑义的直接定。” Confirms the proposed source role and fixed invocation lifetime under D-29-LENDING/P-552–P-554, with ordinary liveness and H-600's changing-owner rejection retained. Authorizes publication and continued straightforward design derivation. Does not select a physical owner carrier, new capture/continuation storage, generalized forwarding relationship or complete proof/native/value-path qualification.

<a id="k-generator-here-20261117"></a>
**K-GENERATOR-HERE-20261117 — Preserve the invocation owner across every handoff.** D-29r12→13/D-14r121→122/registry853→854 under H-604, C-96/C-65 and charter §§2/3.2–3.3/4.3/6.3/8/12. Derives once-at-entry owner binding, caller-versus-generator here, protocol-role preservation through typeof/views, distinct recursion, valid body-local cleanup versus rejected delayed escape, and ordinary feedback/labelled/nullable consequences. Complete mechanism/value-path/native qualification remains separate; ordinary returns, source423, D-16 and byte-identical D-18r113 unchanged. Mainline's source review found no intrinsic fixed-owner conflict, while requiring this public role and actual invocation binding; that is engineering evidence, not human confirmation or implemented support. Baseline: `bafe3c8dde28dfd801a06651f956ae1918c03a49` at affected design paths. Documentation only; no local code/build/tests.

## 2026-11-17 — Close bounded init cases and review stable generator-local borrowing

<a id="k-init-flow-owner-review-20261117"></a>
**K-INIT-FLOW-OWNER-REVIEW-20261117 — Separate settled flow consequences from a public owner choice.** Agent derivation under H-445/H-447/H-601/H-602/H-592: D-22r39→40/D-29r11→12/registry852→853, P-555–P-557/P-552–P-554. Bounded init rules distinguish pointer-slot null writes from missing target writes, retain write-before-read despite pre-existing payloads, preserve const/readonly access and account for actual cleanup before successful completion. C-65/C-72/C-95 and charter §§2/3.3/4.3/6.3/8/12 preserve explicit storage/flow responsibility; full modifier/publication/proof/native/value-path qualification remains separate. The proposed generator-activation owner qualifier in here is pending-human under C-96/C-65: one invocation owner differs from H-600's rejected changing per-yield owner; no source permission or lifetime extension is inferred. Correct the stale D-22 local coverage revision and retain current D-29 scope. Baseline: `e28d2c42b7a7a2fc2563d9c970c1f8f3cc0a5aff` at the affected design paths, integrating mainline source423 and both prior histories. D-16, D-18r113 and source423 remain unchanged. Documentation only; no local code/build/tests.

## 2026-11-17 — Integrate caller-owned generator effect boundaries

<a id="k-generator-effect-integration-20261117"></a>
**K-GENERATOR-EFFECT-INTEGRATION-20261117 — Preserve source permissions independently of native handoff.** Agent continuity registry850/851→852 reconciles `318d2cd79adf37c7df6a22a033b6ad7b3894ca2b:design/DesignAuthorityRegistry.md` with `ea8174e68b574b8efd98acab9a23188b05ab160a` at the affected design paths. D-29r11/D-10r199 retains H-603's lexical caller-body permissions and generator-owned function restrictions, grouped protocol metadata, active dynamic bindings and whole-for owner/readiness obligations under C-96/C-37/C-65/C-84 and charter §§2/3.2–3.3/4.3/6.1–6.3/8/12. Source423/K-795's root-name migration and original human records remain intact. Corrects the registry's stale D-29 revision label; full generator value-path/checker/view/native qualification, D-16 and byte-identical D-18r113 remain separate. Documentation integration only; no compiler or fixture changes.

## 2026-11-17 — Keep generator bodies under caller permissions

<a id="h-603"></a>
**H-603 — Yield hands control to the caller-owned body.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “我倾向于允许，loop body 属于 caller，yield 只是把控制交回 caller，不应该算 func generator 间接调用 proc。generator 自己的代码仍遵守 func 限制，按这个边界推演看看。” Selects acceptance of a proc caller's procedure-using body with a func generator, with lexical control ownership as its rationale; generator-owned operations retain function restrictions. Replaces the pending strict-transitivity recommendation in K-GENERATOR-TYPE-CONTEXT-20261117. Does not approve a general callback/effect calculus, future pure semantics, complete view/native mechanism or full value path.

<a id="k-generator-caller-effects-20261117"></a>
**K-GENERATOR-CALLER-EFFECTS-20261117 — Preserve lexical eligibility through handoff.** D-29r10→11/D-10r198→199/registry850→851 updates D-29-EFFECTS/P-558–P-560 under H-603, C-96/C-37/C-65/C-84 and charter §§2/3.2–3.3/4.3/6.1–6.3/8/12. Derives caller/generator kind matrix, operand/feedback/cleanup attribution and nested forwarding without source permissions depending on native calls. Existing dynamic bindings and iteration restoration remain, with no implicit snapshot/replay; caller symbol demands are retained separately and body writes/readiness/owner facts remain in the whole for analysis. Ordinary callback restrictions, independent selection, checked forwarding, H-600 owners, H-601/H-602 init, D-16, source422 and byte-identical D-18r113 remain. Complete effect/view/native/value-path qualification stays pending. Baseline: `477d33b681db14b2b7e09d4beb4b3d21b62f2418:design/RangedForAndGenerators.md`, `design/SymbolicDynamicScope.md` and registry. Documentation only; no local code/build/tests.

## 2026-11-17 — Delimit generator callable types and isolate effect eligibility

<a id="k-generator-type-context-20261117"></a>
**K-GENERATOR-TYPE-CONTEXT-20261117 — Preserve protocol metadata and review the supplied-body effect boundary.** Agent derivation under H-584/H-592/H-595: D-29r9→10/registry849→850 specifies type-position parentheses for callable yield/feedback contracts, full typeof/view metadata and independent selection without void-result erasure. C-96/C-65/C-84/P-528–P-530 → charter §§2/3.3/4.3/6.3/8/12 preserves readable typed composition; complete view/variance/native/value-path qualification remains pending. New D-29-EFFECTS/P-558–P-560 isolates strict function transitivity versus a special caller-body yield handoff under C-37; no relaxation is selected. Baseline: `708031d8dcb0be60a7f3c7c1259869b5cf4b0350:design/RangedForAndGenerators.md` and registry. Mainline source422/K-794 and H-601/H-602 integration is incorporated with both histories preserved; D-16, D-22r39 and byte-identical D-18r113 unchanged. Documentation only, no local code/build/tests.

## 2026-11-17 — Retire the source root intrinsic

<a id="k-795"></a>
**K-795 — Use ordinary lookup for root.** Agent continuity source422→423/D-13r34→35/registry849→850 implements H-595/D-13-ROOT-ACCESS/P-546–P-548’s explicit intrinsic removal. User callables, storage and imports can use root; intrinsic forcing rejects, with no compatibility builtin. C-74/C-65/C-72 → charter §§2/3.1–3.3/4.3/6.3/8/12 and D-16 preserve readable named storage, owners/access/init and ordinary dispatch. Acceptance covers those names, normal calls and unrelated intrinsic controls. Baseline: `708031d8dcb0be60a7f3c7c1259869b5cf4b0350:cod/lang_syntax.hh` and the affected design paths. C++ root-address helpers remain; this grants no named-region/receiving/native mechanism or broader qualification. D-18r113 remains unchanged and pending.

## 2026-11-17 — Integrate init-place clarification with ready-ref proofs

<a id="k-init-source-integration-20261117"></a>
**K-INIT-SOURCE-INTEGRATION-20261117 — Preserve separate readiness and mandatory-write obligations.** Agent continuity registry847/848→849 reconciles `e3c81af8b205b6b2054813a9ae28554ecddf359c:design/DesignAuthorityRegistry.md` with `ac77b227de0f33511af3a885d862ddf5e1205e7b` at the affected design paths. H-601/H-602's D-22r39/D-28r35/D-20r6 init-reference and explicit-var scope coexists with K-794/source422's ordinary ready-ref invariant. An established init target identity is not initialized payload evidence; entry readiness alone does not discharge promised writes. Preserve C-65/C-72/C-95, charter §§2/3.3/4.3/6.3/8/12, actual owners, normal-completion publication and exact failure cleanup. Human records, source422, D-16 and pending D-18r113 remain unchanged; no new modifier/proof/native/value-path qualification follows.

## 2026-11-17 — Clarify init references and explicit caller places

<a id="h-601"></a>
**H-601 — Preserve init reference shorthand; reject the extra target distinction.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “参数里的 `init n:Node in O` 总是 `init ref n:Node in O` 语义吧？ 不存在 target 与否的区别，你想多了？” Confirms shorthand equality and rejects the extra target-modifier split. Agent evidence audit finds that `bded2d864ece6640fdfbdfef418817ae3cabf5db:design/ConstructionAndAcquisition.md`, “Init destinations and declared place aliasing”, explicitly left the declaration-directed reference-slot role as a proposal requiring reconciliation. The later H-571/K-OUTPUT-ACCEPT-20261117 summaries overstated that detailed role's settled scope. This new correction preserves their original evidence and the unaffected general output contract.

<a id="h-602"></a>
**H-602 — Express the caller place explicitly with var.** Same human/source/date, following H-601: “`init ref var n:Node in O` 大概更准确”. Selects the clearer explicit declared-place direction for the caller reference-slot form. Ref describes the binding, var supplies caller-place aliasing and init adds initialization responsibility. Ordinary operator/flow consequences and analogous modifier combinations are agent-derived, not a separate human approval of every modifier, nullability, aggregate, ABI or proof detail. The separate target-modifier proposal has no current contract.

<a id="k-init-place-correction-20261117"></a>
**K-INIT-PLACE-CORRECTION-20261117 — Reconcile ordinary references and explicit var places.** D-22r37/pending-r38→39, D-28r34→35, D-20 LLVMInterop/CompilerPipeline r5→6 and registry846/pending847→848 updates D-22-INIT-PLACES/P-555–P-557 and P-512–P-514/P-319–P-320 under H-601/H-602. Removes the pending target helper; plain init references retain selected-target semantics, while explicit ref var/ptr var names caller reference slots and val var names caller bits places. Preserve common identity/readiness/mandatory-write flow, ordinary = versus field/<- operations, immediate public writes, exact cleanup/no rollback and C-65/C-72/C-95 → charter §§2/3.3/4.3/6.3/8/12. Full combinations/nullability/publication/view/native/value-path qualification remain open. Committed baseline: `44ff01c8afcf84a76a0075b52aececf82c10a3a4` at affected paths; K-INIT-TARGET-REVIEW-20261117 records the intervening unpublished proposal. Source421, D-16, D-29r9 and byte-identical D-18r113 unchanged; documentation only, no local code/build/tests.

## 2026-11-17 — Isolate the target-initialization helper interface

<a id="k-init-target-review-20261117"></a>
**K-INIT-TARGET-REVIEW-20261117 — Keep target initialization distinct from reference-slot initialization.** Agent assessment under H-592/H-595: D-22r37→38/registry846→847 adds pending D-22-INIT-TARGET/P-555–P-557. H-445/H-447/H-571's common flow and declared-place roles determine identity-only destination preparation, actual owner/access, fixed target, mandatory writes, delegation and no rollback; they do not select explicit init target versus a bare-init shorthand exception. Candidate C-65/C-72/C-95 → charter §§2/3.3/4.3/6.3/8/12 preserves visible storage responsibility. Baseline: `44ff01c8afcf84a76a0075b52aececf82c10a3a4:design/ConstructionAndAcquisition.md` and registry. Complete grammar/proofs/native/value-path qualification remain pending; source421, D-16, D-29r9 and byte-identical D-18r113 unchanged. Documentation only, no local code/build/tests.

## 2026-11-17 — Prove caller ref invariants through pointer methods

<a id="k-794"></a>
**K-794 — Retain intermediate target and absence provenance.** Agent continuity source421→422/registry846→847 under D-14/H-306/H-366–H-369/P-269/P-327/H-420/H-423, existing D-17/D-18 and D-26/H-407 extends K-776’s finite source proof to caller ref slots passed to ptr-formal methods. Every source must retain physical ref initialization/non-null guarantees; independently introduced absence rejects even after restoration. Nullable input origins remain distinct through recursive assumptions and nested/prepared calls. Exact access/type/owners, ordered preparation, declared checked obligations and cleanup continue under charter §§3.3/4.3/5.3/6.3/8/12 and D-16. Acceptance includes independent intermediate-write histories, source-preserved prior gap cases, recursive native sharing and exact failure release. Baselines: `2a042da4f5ea818eae9285520d7641794b13e214:cod/lang_checked.hh` and `44ff01c8afcf84a76a0075b52aececf82c10a3a4:design/DesignAuthorityRegistry.md`. No new source rule, runtime permission, owner/capture mechanism, permanent domain/ABI or broader qualification; D-18r113 remains byte-identical and pending.

## 2026-11-17 — Keep generator loop-control owners fixed

<a id="h-600"></a>
**H-600 — Reject a fresh owner parameter per yield.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Responds to the per-handoff yield owner Y proposal: “我感觉不能支持，会让 loop var `item` 的owner无法静态分析证明”. Rejects the proposed change of owner between handoffs under CoD's statically established owner model. D-29r9 records the fixed-owner consequence; this is not a claim about impossibility in other type systems or confirmation of complete generator/native mechanisms. Replaces the pending proposal in K-GENERATOR-BORROWS-20261117; original payload: `cc9c18d8d07e3717a4c2e912c2d09da4fc28a3ca:design/RangedForAndGenerators.md`, section “Pending decision: a fresh owner parameter per handoff”.

<a id="k-generator-fixed-owners-20261117"></a>
**K-GENERATOR-FIXED-OWNERS-20261117 — Preserve one actual owner across borrowed handoffs.** D-29r8→9/registry845→846 updates D-29-LENDING/P-552–P-554 under H-600; C-96/C-65 → charter §§2/3.2–3.3/4.3/6.3/8/12 retains typed traversal and explicit storage responsibility. Derived consequences distinguish varying targets from varying owners, per-field fixed owners, declared val snapshots, null and fixed-owner checks through forwarding/fresh controls/result-bound labels. Ordinary D-22 single-call result owners remain unchanged. Removes the proposed syntax from current design; generator-local public owner naming and complete proof/view/native/value-path qualification remain open. Baseline: `cc9c18d8d07e3717a4c2e912c2d09da4fc28a3ca` at D-29/registry. Source421, D-16 and byte-identical D-18r113 unchanged; documentation only, no local code/build/tests.

## 2026-11-17 — Bound borrowed yields by ordinary owners

<a id="k-generator-borrows-20261117"></a>
**K-GENERATOR-BORROWS-20261117 — Separate existing lifetime rules from fresh yield-owner quantification.** Agent derivation under H-592/H-595: D-29r7→8/registry844→845 records input-owner retention, live-target rather than snapshot behavior, delayed-defer/feedback lifetime constraints and fixed receiver owners. Candidate C-96/C-65 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves typed traversal and explicit storage responsibility. New D-29-LENDING/P-552–P-554 keeps the per-handoff owner binder and its alternative pending-human; it is not continuity qualification or a human decision. Corrects D-29's stale local revision summary. Baseline: `4d727341c3354d9d1e993f97d0ca049eb0a0c8b9:design/RangedForAndGenerators.md` and that commit's registry. Scope excludes new lending permissions, exact binder/view grammar, complete effect/native proof and ABI. Source421, D-16 and byte-identical D-18r113 unchanged; documentation only, no local code/build/tests.

## 2026-11-17 — Select named region access and compositional initialization

<a id="h-595"></a>
**H-595 — Keep root receiving and direct destinations; remove root(r).** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable. Requests evaluating redundant whole-root access and region receiving, then answers “两者同时支持呗，让 `r.<root-name>` 可以作为 `=` `<-` 的lhs不就行了” and explicitly directs “`root(r)` 直接删了吧”. Confirms both root access forms and removal of the source intrinsic; ordinary standalone var place binding supplies a named slot without the agent's extra receiving operator. Renews autonomous settlement/documentation of unambiguous tracker items. Does not direct removal of C++ root-address internals or qualify the complete regional native mechanism.

<a id="h-596"></a>
**H-596 — Initialize each region root independently.** Same human/source/date: “make region 语法语义也往细了推演一下，每个root应该可以单独指定 initializer”. Confirms the per-root construction requirement and requests detailed composition with existing allocation, field and cleanup rules; does not by itself approve every constructor grammar/proof detail.

<a id="h-597"></a>
**H-597 — Allow a construction-local owner name.** Same human/source/date, reply to call_8fb14ebb355f4562b1d7717f73175fd7 selects “Allow optional construction-local region binder” for make region r:State(capacity) with ordered count/head/next initialization. Confirms r is available only during construction, the destination becomes available after success, and unfinished-root reads reject. Full init/helper/physical proofs remain separate.

<a id="h-598"></a>
**H-598 — Permit init helpers and keep whole-region readiness explicit.** Same human/source/date: “以 `init` 修饰符的参数传给helper去初始化应该也是允许的，否则跟本地变量未初始化不能读的静态规则一致处理”, and proposes treating the region as incomplete until all roots are initialized to avoid complex field analysis. Confirms helper delegation under initialization restrictions; the coarse call-boundary/local finite-root-fact strategy is an agent-derived implementation direction, not a human-selected algorithm or relaxation of soundness.

<a id="h-599"></a>
**H-599 — Allow a free initialization block to delegate all roots.** Same human/source/date: “你的自由初始化块建议，能内部调用helper完成所有roots的初始化的话，也挺好”. Accepts the free-block/helper direction. Integration requires complete root initialization on successful completion rather than one literal initializer entry per root; normal ordered effects, initialization checks and failed-new-region cleanup remain. No allocation rollback or early readable/owning publication is approved.

<a id="k-named-roots-20261117"></a>
**K-NAMED-ROOTS-20261117 — Unify root access and initialization responsibilities.** D-13r33→34/D-22r36→37/registry843→844 adds D-13-ROOT-ACCESS/P-546–P-548 and D-22-REGION-INIT/P-549–P-551 under H-595–H-599. Derived source-category cleanup, field-kind/owner and staged whole-region readiness boundaries preserve C-74/C-65/C-72/C-63 → charter §§2/3.1–3.3/4.3/6.3/8/12; full value-path/proof/native qualification remains pending. D-07r167→168/D-10r197→198/D-11r35→36/D-14r120→121/D-17r87→88/D-28r33→34 aligns root examples, policy-object constraints and lifecycle wording, preserving unrelated qualified contracts. Baseline: `c507c2c4a84a324956d9ef3e9bd67c6d141cca83` at these paths; original structural direction is H-323 at `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:4792`. Source421, D-16 and byte-identical D-18r113 unchanged; documentation only, no local code/build/tests.

<a id="k-generator-protocol-20261117"></a>
**K-GENERATOR-PROTOCOL-20261117 — Establish the protocol independently of its body consumer.** Agent settlement under H-592/H-595, H-584 and D-24: D-29r6→7/P-528–P-530 retains declared/default-void feedback, independent generic/declaration evidence, explicit labelled production and lexical yield targets. Nested ordinary callables cannot implicitly capture an outer yield entry; nested for bodies retain lexical targets. Preserve C-96/C-65/C-84 → charter §§2/3.2–3.3/4.3/6.3/8/12, receiving effects/owners and fixed void final completion. Baseline: `c507c2c4a84a324956d9ef3e9bd67c6d141cca83:design/RangedForAndGenerators.md`. Grouped callable-view grammar, additional inference and complete native/value-path proof remain separate; no implementation qualification.

## 2026-11-17 — Apply ordinary liveness to result-bound owners

<a id="h-594"></a>
**H-594 — Allow result-bound owners without paired-field reception.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Accepts the arena:owned region:Graph / root in arena result relationship: “拒绝只接收 `root` 不接收 `arena` 的判据不需要特殊化”, because root requires proof its owner remains live; “否则也可以选择不接收一个 owned region，让它当场release就完了。” Confirms the source owner/lifetime and omitted-cleanup rule, replacing the agent's suggested special receipt-pair restriction. Does not qualify complete proofs, native transport, new ADT fields or the separate D-18 review.

<a id="k-result-owners-20261117"></a>
**K-RESULT-OWNERS-20261117 — Preserve dependencies rather than mandate receipt pairs.** D-22r35→36/registry842→843 adds D-22-RESULT-OWNERS/P-543–P-545 under H-594. Agent integration under H-592 derives actual-owner-preserving renaming, declaration-order-independent metadata, ordinary temporary bits reception before omitted cleanup, projection evidence and exact abandonment/replacement cleanup. Candidate C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves explicit storage responsibility and composed results without hidden retention; full mechanism/value-path qualification remains pending. Baseline: `2a042da4f5ea818eae9285520d7641794b13e214:design/ConstructionAndAcquisition.md` and the same commit's registry. Source421, D-16, D-23r24, D-29r6 and byte-identical D-18r113 unchanged. Documentation only; no local code/build/tests.

## 2026-11-17 — Prepare checked body storage independently of LLVM

<a id="k-793"></a>
**K-793 — Keep body storage preparation at the checked/native boundary.** Agent continuity source420→421/registry841→842 under K-790’s D-26/H-407, existing D-14/D-18 contracts and H-420/H-423 moves the existing context-storage unification/remapping into LLVM-independent checked-program code. Preserve each general/specialized body’s place order, input/capture/owner roles, instance-associated operation facts, cleanup and LLVM/diagnostic behavior. Charter §§2/3.3/5.3/6.3/8/12 and D-16 continue. Baseline: `a3f6927f642e714a092bf1b936f6fc619074e9f8:cod/lang_llvm.hh` and the same commit’s checked-program/source-reference/registry paths. This transient preparation introduces no source permission, new layout algorithm, persistent representation, ABI or broader qualification; D-18r113 remains byte-identical and its review pending.

## 2026-11-17 — Derive generator header reception from existing scope rules

<a id="k-generator-headers-20261117"></a>
**K-GENERATOR-HEADERS-20261117 — Keep control bindings distinct from materialized targets.** Agent settlement under H-592 using H-578, D-17 loop scopes and D-23/H-416 scalar materialization: D-29r5→6/registry840→841 adds D-29-HEADERS/P-540–P-542. Select all existing destinations at each actual post-argument handoff before writes; fresh new-control initialization preserves const/zero-yield behavior. New scalar ref targets belong to the enclosing full scope and retain fresh identity; borrowed targets keep their owner, while control-slot lifetime/capture restrictions remain. Sole labelled outputs admit single-item header mapping without type equality. Candidate C-96/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 preserves explicit operation/lifetime responsibility. Read-only [mainline review](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6), recorded2026-11-17, found no contract conflict or simpler consistent owner model; this is not human confirmation, measured cost or implementation evidence. Baseline7c1b41f44f1a6b9d54803170052985c75054458b at affected paths. Complete lending/native/value-path proof remains pending; source420, D-16 and D-18r113 unchanged. Documentation only; no local code/build/tests.

## 2026-11-17 — Settle derived receiving and result rules

<a id="h-592"></a>
**H-592 — Continue the tracker autonomously until a substantive decision is needed.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Requests continued work on the external active tracker: “能自主settled的你直接处理，如果还有需要我参与的决策，再停下来讨论”. Authorizes derivation/recording of straightforward consequences and coherent publication; does not approve an unpresented material trade-off, all pending mechanisms or compiler implementation.

<a id="h-593"></a>
**H-593 — Receiving write/write conflicts are author-responsibility UB.** Human/source: same current design chat/user, recorded 2026-11-17 America/Los_Angeles; exact timestamp unavailable. Answers the val-only pair/fill(n,n) example: “这种和类似的，我觉得都该按UB处理，因为是程序作者有相应reasoning依据可以去推断的，也有充分的变通做法可以去避免的。” Selects UB rather than either complete-write order or source-order delivery. Applies to cross-entry receiving write/write conflicts and analogous overlap, not all ordinary target sharing or sequential assignments. Integration reuses H-576 warning/opt-in-error planning under H-592 delegation; implementation/analysis precision remains separate.

<a id="k-receiving-core-20261117"></a>
**K-RECEIVING-CORE-20261117 — Normalize receiving and settle overlapping writes.** Agent assessment under H-571–H-575/H-591/H-592: D-22r34→35/registry839→840 derives explicit item grammar, independent lookup/name-collision rejection, empty reception, ordinary temporary full-scope equivalence, componentwise independent inference and one pending result with exact preparation/abandonment cleanup. H-593 additionally selects cross-entry write/write UB; D-23r23→24 and D-29r4→5 align dependency/acceptance summaries. Existing P-509–P-520/P-537–P-539 serves typed readable composition, explicit lifetimes, author responsibility and preserved effects under charter §§2/3.2–3.3/4.3/6.3/8/12. Field inspection and dependent-owner/native proof extensions remain separate. Baseline23a3a70d8e98cbc6d4933afb101775b8b66d8230 at affected paths. Source420, D-16 and D-18r113 unchanged; documentation only, no local code/build/tests or complete mechanism qualification.

## 2026-11-17 — Allow structural callable-result projection

<a id="h-591"></a>
**H-591 — Allow extra-result projection and derive straightforward consequences.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Responds to the {count, ready} → {count} callable-adaptation example: “好像没有不允许的理由吧？ 感觉这种问题不需要问我确认吧”. Accepts projection under the presented component/effect/cleanup/owner constraints and renews delegation of straightforward consequences instead of repeated confirmation. Does not qualify all structural-result mechanisms, broader variance or native ABI. Baseline: db3bee68093db532f64169909c2c50adb39467f0:design/ConstructionAndAcquisition.md and the preceding chat example.

<a id="k-result-projection-20261117"></a>
**K-RESULT-PROJECTION-20261117 — Compose existing partial-result rules.** D-22r33→34/registry838→839 adds D-22-RESULT-COMPAT/P-537–P-539 under H-591. Agent-derived order-independent identity, directional projection and declared direct forwarding preserve C-10/C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12: complete result contracts, observable effects/checked failures, exact cleanup and valid owners, with no group object or hidden retention. Records source acceptance and exclusions separately from pending proof/value-path/native qualification. Integrates mainline4c8350108f873435324eed6cfe156446cc63e741/source420/K792 by fast-forward; D-16, D-29r4 and D-18r113 unchanged. Documentation only; no local code/build/tests.

## 2026-11-17 — Stabilize contextual callable coverage

<a id="k-792"></a>
**K-792 — Keep fixed callable coverage independent of mutable analysis hints.** Agent continuity source419→420/registry837→838 under K-789’s D-26/H-407, D-14/H-295, existing D-18 complete values, D-24/H-421–H-424 and H-420/H-423. Fixed expected-result/type-only declaration evidence remains per materialization; preparation preserves input, query/context, result/throws/body and cleanup obligations. Acceptance includes the existing explicit/contextual LLVM equality assertion, wide anonymous/returned views, distinct lexical query environments and preserved scalar/native controls. Charter §§2/3.3/5.3/6.3/8/12 and D-16 continue. Baselines: `db3bee68093db532f64169909c2c50adb39467f0:cod/lang_checked.hh`, `db3bee68093db532f64169909c2c50adb39467f0:tests/cod/language/README.md` and `d8a6878edcf0f2f331802e2e351b9121298bf00b:cod/lang_checked.hh`. No new inference/conversion, scalar capture transport, permanent proof/ABI or broader qualification; D-18r113 remains byte-identical and pending, D-29r4 unimplemented.

## 2026-11-17 — Preserve lexical returns across generator cleanup

<a id="h-589"></a>
**H-589 — Treat generator cleanup as part of the whole for operation.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “认同。 ‘generator cleanup itself throws or returns’ 就作为 for-loop 的整体行为就行吧，感觉没有疑义吧？” Selects reuse of ordinary completion rules at the for boundary; does not select a native carrier or qualify the whole generator mechanism. Baseline: f35469b85b67c64510fd93e80271f6eb4837e880:design/RangedForAndGenerators.md.

<a id="h-590"></a>
**H-590 — Reject cancellation by a generator-local return.** Human/source: same chat/user, recorded 2026-11-17 America/Los_Angeles. Responds “‘replace a pending body exception or outer return’ 不该允许吧？” to the agent's claim that an explicit generator return in cleanup could do that. Rejects that agent interpretation of H-589: generator return completes its own void invocation and preserves the independently outgoing body exception/enclosing-callable return. The rejected interpretation was never committed as a contract. This correction does not waive cleanup failures, checked obligations, remaining cleanup or exact result release; it selects no implementation mechanism.

<a id="k-generator-cleanup-20261117"></a>
**K-GENERATOR-CLEANUP-20261117 — Keep cleanup completion targets distinct.** D-29r3→4/registry836→837 records H-589/H-590 in D-29-CONTROL/P-534–P-536. Candidate C-96/C-63/C-65 → charter §§2/3.2–3.3/4.3/6.2–6.3/8/12 preserves lexical control, observable cleanup and valid ownership: local generator return preserves body passthrough; escaping cleanup failures compose at for under D-17. Adds an acceptance example and scoped consequences, removes the superseded open source-priority question, and retains pending target-aware transport/proof qualification. D-17, source419, D-16 and D-18r113 are unchanged. Documentation only; no code/build/tests or broader approval.

## 2026-11-17 — Accept generator signatures and checked body forwarding

<a id="h-584"></a>
**H-584 — Accept the presented generator/feedback syntax.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “同意。再看看 checked exception 声明和执行过程中的语义解释”. Accepts the preceding yields YieldType -> FeedbackType proposal, omitted feedback defaulting to void, independent single/multi-field directions, contextual continue field construction, ordinary receiving reuse, fixed void generator completion and outer-callable return distinction. Requests the next checked-exception analysis. This does not approve unpresented grouped-type grammar, complete generator views, all inference/lifetime/effect mechanisms, native transport or D-18r113's separate review. Baseline: fa56eb44bc1092c0e59d4f06ddcd483067d373f3:design/RangedForAndGenerators.md, with confirmation limited to the preceding chat presentation.

<a id="h-585"></a>
**H-585 — Forward the supplied body's remaining checked obligations.** Human/source: same current design chat/user, recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Selects “Forward body obligations (Recommended)” for the numbers/parse/ParseException example. Accepts checking the body after ordinary generator selection, statically composing its declared failures through yield sites and generator catches, and forwarding remaining obligations without requiring ParseException in numbers' own declaration. Explicit generator throws still remains a fixed caller obligation. Confirms this generator-specific conditional failure boundary; does not select general ordinary-callable effect polymorphism, runtime exception checking instead of static proof, a summary algorithm/ABI or new dynamic-effect permissions.

<a id="h-586"></a>
**H-586 — Assess a special propagation carrier and native alternatives.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Proposes considering “LoopBodyReturn <: LoopBodyPassthrough <: Error” to assist body exception/return propagation, suggests cleanup through defer rather than explicit wrapper catches, and prefers a better LLVM non-exception mechanism if available. This requests comparison, not adoption of public wrapper types, an Error classification, catch-filter exceptions or a native ABI.

<a id="h-587"></a>
**H-587 — Preserve finally independently of propagation machinery.** Human/source: same current design chat/user, recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. States that for generator `try { yield ...; } finally { ... }`, “finally 块应该总是确保执行的吧，不管最终用什么机制去实现body穿透”. Reaffirms D-17's cleanup invariant at the new generator boundary: leaving an entered protected region executes its finally on normal/abnormal body completion. This does not select an ABI or resolve competing cleanup return/throw priorities across distinct targets.

<a id="h-588"></a>
**H-588 — Body exceptions bypass generator catches.** Human/source: same current design chat/user, recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Refines the source boundary: “loop body exception 最好不能被 generator 的 `try { yield ... } catch (...) {}` 捕获，总是透穿回loop所在callable更正确一些”. Replaces H-585's generator-handler discharge/interception part: after body-local handling, escaping body exceptions pass through required generator cleanup to ordinary handling at the lexical for site. Automatic checked forwarding and fixed explicit generator throws remain; the generator may still catch its own failures. Does not select a public exception wrapper, native carrier, hidden lifetime extension or competing cleanup-completion priority. H-587 finally guarantees continue.

<a id="k-generator-failures-20261117"></a>
**K-GENERATOR-FAILURES-20261117 — Keep body escapes outside generator catches.** D-29r2→3/registry835→836 adds D-29-SIGNATURE/P-528–P-530, D-29-FAILURES/P-531–P-533 and D-29-CONTROL/P-534–P-536 under H-584–H-588. Candidate C-96/C-63/C-65/C-84 → charter §§2/3.3/4.3/6.1–6.3/8/12 preserves typed traversal, lexical failure responsibility and cleanup. H-588 supersedes only H-585's generator-catch handling of body failures; current design distinguishes generator-own failures, nested lexical for boundaries and fixed declared obligations. Records finally/defer coverage and recommends evaluating private completion transport before a public Error hierarchy; native choice and competing cleanup priorities remain pending. Mainline 4f1cc02eb17efdc88a56d272e59639362a3ad8dc/source419 is fast-forward integrated with K-790/K-791 and all parent evidence preserved. Design baseline: fa56eb44bc1092c0e59d4f06ddcd483067d373f3 at affected paths. Preserve D-16 and byte-identical D-18r113; documentation only, no local code/build/tests or broader qualification.

## 2026-11-17 — Bound source preparation depth

<a id="k-791"></a>
**K-791 — Diagnose source-depth exhaustion before recursive processing.** Agent continuity under D-26/H-407, existing D-10/D-14/D-17 contracts and H-420/H-423: source418→419/registry834→835 adds private completed-structure and parser-entry allowances with source-located resource diagnostics. Preserves source meaning, token rules, actual owners, completion, no partial execution/output and supported-input native behavior; deep operator/group/call/type/body, boundary and flat-source acceptance applies. Charter §§2/3.3/5.3/6.3/8/12 and D-16 continue. Restores K-790’s standalone registry summary and H-577–H-579 range after a paragraph insertion split, retaining their recorded scopes and append-only evidence. Baselines: `465344562a217f3162be84ca1f606340002ce5aa:cod/lang_syntax.hh` and `fa56eb44bc1092c0e59d4f06ddcd483067d373f3:design/DesignAuthorityRegistry.md`. No semantic depth ban, new source/capture permission, permanent analysis/ABI or broader qualification follows; D-18r113 remains byte-identical and pending, and D-29r2 remains unimplemented.

## 2026-11-17 — Specify generator feedback checking and propose signatures

<a id="h-580"></a>
**H-580 — Void feedback discards; other feedback requires static branch conformity.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Requests concrete generator syntax including the yield result declaration: “其中声明为void时loop body的结果类型不限，否则就需要静态检查loop body各种分支的结果要符合generator要求”. Selects that normal-feedback type-checking boundary. H-577’s explicit outer return and H-579’s void final completion remain distinct. This does not confirm the agent’s yields Y [-> F] spelling, omitted-feedback default, callable-type grouping, multi-field feedback syntax, full effects/owner transport or native implementation.

<a id="h-581"></a>
**H-581 — Explicit feedback through continue.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. States “`continue xxx;` 语法也可以支持了，loop body中用于直接回馈给yield”. Adds explicit body feedback without replacing H-580 normal body-expression results or H-577 enclosing return. Does not select a new break/labelled-control policy, arbitrary cross-call continuation or native transport. H-582 further specifies void feedback.

<a id="h-582"></a>
**H-582 — Void feedback permits only bare continue.** Human/source: same current design chat/user, recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Corrects the assistant's proposed discard behavior for an operand-bearing continue: “void feedback 时，还是reject `continue xxx` 吧，只允许 `continue;`”. Confirms a syntax distinction: void permits only bare continue, while H-580 still allows arbitrary independently valid normal fallthrough result types. An expression may be evaluated separately before bare continue. Non-void explicit feedback retains H-580 static conformity; ordinary type/access/effects/cleanup remain required.

<a id="h-583"></a>
**H-583 — Reuse structural results for yield feedback.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Requests “yield结果也考虑支持多字段，照搬对字段返回类型声明和receiving spec好像就差不多”. Selects the design direction of labelled yield results with existing structural result declaration and receiving-specification rules. D-29 integrates it with H-581 explicit feedback as contextual `continue { expression -> label, ... };`, retaining H-582 bare-only void continuation. This creates no first-class group or new ordinary ADT field permission and does not qualify pending schema identity, owner/effect proofs or native transport.

<a id="k-generator-feedback-20261117"></a>
**K-GENERATOR-FEEDBACK-20261117 — Draft a separate yield/feedback clause.** D-29r1→2/registry833→834 adds D-29-FEEDBACK/P-525–P-527 under H-580–H-583 while retaining pending C-96/P-521–P-524. Candidate C-96/C-63/C-65/C-84 → charter §§2/3.3/4.3/6.1–6.3/8/12 relates explicit feedback obligations to typed interaction and preserved control/effects. Proposes yields Y [-> F], default void, expression-first named yield components and grouped callable contracts. Records explicit continue feedback, bare-only continue for void, and H-583 structural feedback/receiving reuse with contextual multi-field continue construction. Preserves pre-yield destination selection for feedback receivers separately from the pending loop-header timing question. Distinguishes discarded fallthrough alternatives, ordinary consumed intermediate typing, tail/continue/caught/exceptional/enclosing-return paths and category-specific preparation/cleanup; no runtime enforcement substitutes for static conformity. Mainline source418/465344562a217f3162be84ca1f606340002ce5aa is fast-forward integrated and supplies the unchanged r1 baseline at design/RangedForAndGenerators.md. Read-only developer review identifies targeted completion, lexical frame/defer access and handoff lifetime/effect proofs as substantive costs, without requiring a coroutine runtime or qualifying a mechanism. Preserve D-16 and byte-identical D-18r113; documentation only, no local code/build/tests. Existing signature/lifetime/cleanup choices remain open.

## 2026-11-17 — Separate checked programs from source analysis

<a id="k-790"></a>
**K-790 — Make the checked/native phase boundary explicit.** Agent continuity under D-26/H-407, existing D-14/D-18 contracts and H-420/H-423: source417→418/registry832→833 separates shared checked definitions from mutable source checking, with explicit driver dependencies and native emission consuming the shared artifact. Definitions/checker behavior and selected-context/owner/completion evidence are preserved; validation fingerprints the complete header and native-fixture inputs. Charter §§2/3.3/5.3/6.3/8/12 and D-16 continue. Baseline: `d8a6878edcf0f2f331802e2e351b9121298bf00b` at compiler/source-reference/registry paths. No new source feature, analysis algorithm, persistent representation, ABI or broader qualification follows; D-18r113 remains byte-identical and pending.

## 2026-11-17 — Consolidate ranged-for and generator design

<a id="h-577"></a>
**H-577 — Lexical return target and generator/body proposal.** Human/source: current project user in the [generator side chat](codex://threads/01a1e9b0-d9c2-7223-87c0-72b964f19118), original user messages verified through native chat reading; recorded 2026-11-17 America/Los_Angeles, exact timestamps/personal name unavailable. Proposes yield as invoking a typed anonymous single-method for-body with the loop site's lexical context. Explicitly states: “我认为 `return` inside this `for` to return from the surrounding callable”. Also proposes considering normal body-expression evaluation as feedback to yield and requests signature design. Confirms the return target, not the full synchronous/nonescape, feedback, signature, owner/proof or ABI mechanism. Directs the side chat to draft and hand over without further exploration. Original `for (...) from gen(...)` spelling is superseded in the current proposal by H-578; no historical draft is retained as another authority.

<a id="h-578"></a>
**H-578 — Assess receiving specifications as generator-loop headers.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable. Requests generator/for analysis alongside return reception, asks whether loop variables can reuse receiving specs, then proposes `for { ... } := gen() { ... }` for multiple yielded fields and `for <ref-var> = gen() { ... }` / `for <val-or-ref-var> <- gen() { ... }` for one value. Requests suitability assessment, not blanket approval of repeated destination timing, header lifetime or full mechanism. H-574–H-576 and D-23 remain dependencies; the proposed header does not make new ref declarations with `<-` valid.

<a id="h-579"></a>
**H-579 — Prefer void-only final generator completion.** Human/source: current project user in this design chat, recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Asks “generator只能return void 类型吧？” and observes that a custom final type could become the overall for evaluation result but has little practical value. D-29 adopts void-only final completion as the bounded recommendation, with no custom final data result of for. This preference does not prohibit proposed non-void per-yield body feedback or change H-577's outer return target; it is not confirmation of a complete signature/cleanup implementation contract.

<a id="k-generator-draft-20261117"></a>
**K-GENERATOR-DRAFT-20261117 — Register the integrated pending proposal.** New D-29r1/C-96/P-521–P-524 and registry831→832 integrate the side-chat evidence with H-578/H-579 and latest D-22r33/D-23r23. Candidate C-63/C-65/C-73 → C-96 → D-29 under charter §§2/3.2–3.3/4.3/6.1–6.3/8/12 preserves readable traversal, explicit receiving and natural lexical lifetime/control. Records header combinations, separate payload/feedback/void completion, no final result-bearing for, and pending repeated destination/lifetime, signature/effect/borrow and target-aware cleanup choices. New value paths remain pending rather than borrowing ancestor qualification. Mainline `d8a6878edcf0f2f331802e2e351b9121298bf00b`/source417 is fast-forward integrated; its prior evidence remains intact. D-16 and byte-identical D-18r113 are unchanged; no compiler/build/test/ABI work. The temporary outside-repository handoff is removed after canonical integration/publication, not archived.

## 2026-11-17 — Prove uniform scalar callable coverage

<a id="k-789"></a>
**K-789 — Share fixed scalar family coverage.** Agent continuity under D-26/H-407/C-89/C-90/P-371/P-372/P-374/P-375, D-14/H-295, existing D-18 complete values and H-420/H-423: source416→417/registry830→831 preserves admitted domains, actual identity/selected context, prepared signatures, declaration/result/throws/body checks and cleanup. One fixed scalar method with invariant declarations uses independent input proofs and a shared checked dispatch target; dependent/overloaded/borrowed obligations retain bounded enumeration. Charter §§2/3.3/5.3/6.3/8/12 and D-16 continue. Source/staging/native acceptance covers wide views, conversions, nonexecuting inspection, complete captures/replay, recursion, prepared callees and exact failure release; supported-input LLVM and diagnostics remain equivalent. Baseline: `33f00bc00dfb7011e889233b1ca824bf886462b6` at compiler/source-reference/registry paths. This private implementation refinement selects no permanent analysis domain, carrier/ABI, new source/capture permission or broader qualification; D-18r113 stays byte-identical and pending, and receiving work remains separate.

## 2026-11-17 — Define receiving borrowed-read conflicts as undefined behavior

<a id="h-576"></a>
**H-576 — Author responsibility for overlapping receiving writes and borrowed reads.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Responding to the two-field borrowed a/b receiving example, states: “CoD语言层面把这种overlapping定义为undefined behavior，属于用户责任，编译器有选择 snapshot 或者 乱序读写的自由”; requests analysis warnings with opt-in promotion to errors and permits postponing difficult implementation. Confirms that scoped source boundary, optimization/responsibility trade-off and diagnostic requirement, replacing the pending order/snapshot choice. Does not define all runtime target sharing or write/write-only overlap as UB, relax other owner/type/access/init rules, or qualify the complete receiving/analysis/ABI mechanism. D-18r113’s separate review is unanswered.

<a id="k-receive-overlap-20261117"></a>
**K-RECEIVE-OVERLAP-20261117 — Scope the UB and diagnostic contract.** D-22r32→33/D-23r22→23/registry829→830 adds D-22-RECEIVE-OVERLAP/P-518–P-520 and updates receiving dependencies. H-576’s C-65/C-72/C-73 → charter §§3.3/4.3/6.3/8/12 trade-off permits optimized handling of invalid receiving conflicts while preserving defined-program observations and explicit programmer responsibility. Documents val-snapshot/sharing/single-entry controls, no guaranteed UB output/runtime trap, staged diagnostics and warning-as-error. Exact detection/flag policy and wider receiving/ownership/native coverage remain separate; write/write-only conflicts stay open. Baseline: `ced1e7bdb7ef51bff129dcf42fd2d4290eea2474` at affected design/registry/changelog paths. Preserve D-16, source416, no copying/moving/promotion/rollback and byte-identical D-18r113. Documentation only; no code/build/tests.

## 2026-11-17 — Require a declarative receiving specification

<a id="h-575"></a>
**H-575 — Receiving describes destinations, not an executable body.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Rejects the recommended receiving-block model: “我不倾向于 "receiving block" 心智模型，我更倾向于 ‘receiving specification’ 的语义，只是澄清返回字段的去向关系，语义清晰后留给编译器做优化处理。‘receiving spec’ 不允许任意expr”. Replaces the H-574 assessment’s imperative/call-first recommendation with a declarative no-arbitrary-expression requirement. Does not select unspecified overlap outcomes, automatic borrowed snapshots, an optimization-dependent language meaning or full proof/ABI qualification; D-18r113’s separate review remains unanswered.

<a id="k-receiving-spec-20261117"></a>
**K-RECEIVING-SPEC-20261117 — Keep receiving declarative.** D-22r31→32/D-23r21→22/registry828→829 updates pending P-515–P-517 under H-573–H-575. Candidate C-65/C-72/C-73 → charter §§3.3/4.3/6.3/8/12 uses simple field/destination names and explicit receiving operations; no executable receiving body, arbitrary expressions or forward dependence on a new receiver. Preserve ordinary pre-RHS destination selection and post-cleanup delivery; exact overlap/borrowed-read and temporary-scope details remain open rather than delegated to optimizer choice. Supersedes the unselected imperative recommendation in K-RECEIVER-ITEMS-20261117, without altering its historical evidence. Public baseline: `908e6d1abb0e7ccc995b87cb6ad1e76b3cc80db0` at affected design/registry/changelog paths. Preserve D-16, source416, ordinary ADT/owner rules and byte-identical D-18r113; documentation only, no code/build/tests or new mechanism qualification.

## 2026-11-17 — Assess explicit receiving items and same-name omission

<a id="h-574"></a>
**H-574 — Put receiving intent in each item.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Proposes “`{ val count<-; ref selected=node; } := inspect();` 同名的可以省略rhs，lhs也可以是之前已经定义的变量”. Requests another syntax assessment, not either implicit slot/target policy from H-573. The current proposal distinguishes declarations from existing destinations and names `=`/`<-` per item. It preserves the earlier existing-variable/renaming and temporary-scope goals. Call-first sequential group execution, exact scope/grammar, complete proofs and native transport remain unconfirmed; D-18r113’s separate review is unanswered.

<a id="k-receiver-items-20261117"></a>
**K-RECEIVER-ITEMS-20261117 — Replace implicit receiving policy with explicit item operations.** D-22r30→31/D-23r20→21/registry827→828 updates pending D-22-RECEIVE-SCOPE/P-515–P-517. Candidate C-65/C-72/C-73 → charter §§3.3/4.3/6.3/8/12 reuses ordinary declaration, initialization, rebinding and payload-write intent. Assesses same-name omission, mixed new/existing receivers, selector lookup, optional temporary scope, repeated consumption and exact failure cleanup. Recommends one complete producer followed by sequential items, explicitly distinguishing this pending choice from ordinary pre-RHS destination selection; no group-wide borrowed snapshot is silently introduced. Mainline `908e6d1abb0e7ccc995b87cb6ad1e76b3cc80db0`/source416/K-788 was fast-forward integrated with all prior evidence preserved; public design baseline remains that commit’s D-22r29/D-23r19. H-573 records the preceding unpublished draft. Preserve D-16, H-571 completion, ordinary ADT/owner rules and byte-identical D-18r113; no code/build/tests or new implementation qualification here.

## 2026-11-17 — Assess temporary mappings into existing result receivers

<a id="h-573"></a>
**H-573 — Existing destinations and temporary field aliases.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable. Corrects the declaration-only example: “不是新定义变量，而是用已有变量去接收返回字段，包括变量名与返回字段名不同时的匹配”; requests syntax/semantic combinations and quirks. Proposes `{ var <rtn-field-name> -> <local-var-name>, ... }`, then `{{ ... }} := inspect();` with temporary names that do not escape. This establishes the existing-receiver requirement and candidate syntax/scope, not a selected slot-versus-target policy, borrowed-read ordering or full mechanism/ABI. H-571’s earlier declaration form is not the complete receiving requirement; its other source decisions and H-572’s return-only assessment remain. The independent D-18r113 review is unanswered.

<a id="k-receiver-scope-20261117"></a>
**K-RECEIVER-SCOPE-20261117 — Assess existing-place result maps.** D-22r29→30/D-23r19→20/registry825→827 adds pending D-22-RECEIVE-SCOPE/P-515–P-517. Candidate C-65/C-72/C-73 → charter §§3.3/4.3/6.3/8/12 relates temporary label-to-place aliases to explicit storage/lifetime intent and readable named composition. Records same-name/renamed/subset/overlapping destinations, fixed-owner and const boundaries, private completion, per-operation failure and omitted cleanup. Slot delivery versus field-mode-directed target writes, borrowed read timing and exact scope grammar remain open; no candidate is silently qualified. Preserve D-23 scalar matrices, D-16, no implicit copy/move/promotion/rollback, source415 and byte-identical D-18r113. Baseline: `ab40e3fdb2b9dbd76767b8a607233f360daefea6` at the affected design/registry paths and `CHANGELOG.md#h-571`. Documentation only; no code/build/tests.

## 2026-11-17 — Identify callable-family coverage exhaustion

<a id="k-788"></a>
**K-788 — Report the exhausted work allowance and active callable view.** Source415→416/registry825→826 continues D-26/H-407/C-89/C-90/P-371/P-372/P-374/P-375, D-14/H-295 and existing D-18 family coverage under H-420/H-423. Detected exhaustion stops compilation as a resource limit with coverage unproved; retain the 4096-step threshold, input enumeration, admission and native behavior. Acceptance spans ordinary construction, symbolic installation, declaration-only queries, below-budget native controls and stable supported-input LLVM/diagnostics. Preserve charter §§2/3.3/5.3/6.3/8/12 and D-16; no source policy, new mechanism/ABI qualification or answer to D-18r113's separate review follows. Baseline: `ab40e3fdb2b9dbd76767b8a607233f360daefea6:cod/lang_checked.hh` and that commit's registry/source reference. H-572 remains a separate proposal.

## 2026-11-17 — Assess return-only structural result signatures

<a id="h-572"></a>
**H-572 — Propose structural results without a stored group.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable. Proposes “structural data types” instead of “lexical ADTs”, no sum form, use only in return types, and field mapping to ordinary lexical variables without a separate structural group; observes that individual lexical refs can already have distinct in O owners. Initially retains data braces, then refines: “放在返回类型声明的位置， `data` 关键词感觉都可以不要”. The current assessment uses `-> { ... }` and preserves H-571 named return/receiving syntax. This narrows the pending anonymous-data direction; it does not approve a full result-use/type/proof/ABI contract, new ordinary ADT field permissions or the independent D-18r113 review.

<a id="k-structural-results-20261117"></a>
**K-STRUCTURAL-RESULTS-20261117 — Replace the aggregate-field question with a return-only assessment.** D-22r28→29/D-07r166→167/D-12r31→32/D-28r32→33/registry824→825 updates D-22-ANON-DATA/P-509–P-511 while preserving H-571/D-22-RESULT-DESTINATION. Candidate C-10/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 derives readable named outputs from existing lexical/result contracts without a first-class group, group owner or stored cross-owner edge. Product-only/return-only use, direct forwarding, type-only schema metadata, per-component effects/cleanup and one pending completion are assessed; exact compatibility, owning-output dependencies and inspection remain open. Independent mainline review finds less work than general lexical products but substantive multi-output callable checking/transport/cleanup; no source implementation or ABI is selected. Preserve ordinary nominal ADT rules, D-16, no copying/moving/promotion/rollback, source415 and byte-identical D-18r113. Baseline: `412e127e623debf28fb828bf52f815b37b5172e5` at affected design/registry paths and `cod/lang_syntax.hh`, `cod/lang_checked.hh`, `cod/lang_llvm.hh`. Documentation only; no local code/build/tests.

## 2026-11-17 — Keep pending completion fields together

<a id="k-787"></a>
**K-787 — Share native completion state across suspension and restoration.** Source414→415/registry823→824 continues D-17/P-235–P-236/H-361/H-362, D-18/P-300–P-303 and D-26/H-407 under H-420/H-423. Live slots and suspended values share one internal result/failure/control record; cleanup restores corresponding fields without an address-keyed map. Preserve native layout/instruction order, explicit-return markers, complete-result ownership and source/staging/native release evidence. D-28r31→32 editorially qualifies declared place aliasing in the init summary. Charter §§2/3.2–3.3/4.3/6.3/8/12 and D-16 continue; no source policy, new mechanism/ABI qualification or answer to D-18r113's separate review follows. Baseline: `bded2d864ece6640fdfbdfef418817ae3cabf5db:cod/lang_llvm.hh` and that commit's source reference; `008724fec56b16f81e1ea475902e89bb9a286fd3:design/RegionalAndLexicalStorageRefactor.md` and its registry/changelog. H-571's selected source forms retain separate implementation boundaries.

## 2026-11-17 — Select logical output destinations and named reception

<a id="h-571"></a>
**H-571 — Accept the output/initialization and named-result recommendations.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “认同，就按你的建议决定。然后下一议题”. Refers to the preceding recommendation: logical caller-provided result destinations with pending preparation before post-cleanup delivery; init as explicit declared place aliasing plus an initialization obligation, distinguishing bits storage, reference slots and referents; `return { expr -> field, ... };`; new-declaration-only `{ ... } := expression;`; per-field ordinary operations, partial named reception, once-only effects and required cleanup, with existing-variable `=`/`<-` intent explicit. Confirms that bounded source model/rationale. Target-initializing helper spelling, complete modifier combinations, structural shape/field-owner permissions, native handles, general aggregate/init proofs and physical ABI were not resolved by that recommendation. The independent D-18r113 review is not answered by this confirmation. Baseline proposal: `bded2d864ece6640fdfbdfef418817ae3cabf5db:design/ConstructionAndAcquisition.md`, caller-provided/init/named-reception sections; H-570 remains the proposal record.

<a id="k-output-accept-20261117"></a>
**K-OUTPUT-ACCEPT-20261117 — Integrate the selected output source contract.** D-22r27→28/D-23r18→19/D-28r30→31/D-20 document-set r4→5/registry822→823 records H-571 in P-509–P-514 and reconciles C-81/P-319–P-320. The C-54/C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 path preserves explicit place/owner responsibility, named composition, result-mode/inference independence, init versus private-return publication, effects and exact cleanup. Replace the separate out proposal in current application text with the selected init roles, leaving handle categories/provider proof pending; pipeline semantics are unchanged. Preserve D-16, existing same-owner ADT fields, no implicit object copy/move/promotion/rollback, source414 and the byte-identical D-18r113 review. D-22 records external-owner lexical fields as the next unconfirmed boundary, not an accepted extension. Baseline: `bded2d864ece6640fdfbdfef418817ae3cabf5db` at affected paths; prior qualifications remain scoped. Documentation only; no code/build/tests or full new mechanism qualification.

## 2026-11-17 — Reconcile result assessment with declaration completion

<a id="k-786"></a>
**K-786 — Keep the output proposal pending beside current completion support.** D-22r26→27/registry820–821→822 integrates H-570/P-512–P-514 with source414/K-785. Editorial scope qualifies declared place aliasing and presents the shared aggregate-cost assessment as current evidence; no output/init syntax, ABI, value path or new mechanism is accepted. Preserve charter §§2/3.2–3.3/4.3/6.3/8/12, D-16, D-17 completion, D-23/D-24 modes/results, actual owners and the byte-identical D-18r113 pending review. Human evidence and preceding qualifications are unchanged. Baselines: `cb976840c2583626e50672f7918d417bad9a7ac1:design/ConstructionAndAcquisition.md` and that commit's registry/changelog; `adcef3584f588ecd32bb1f54498ffbb8957b02df:tests/cod/language/README.md` and its registry/changelog. Documentation integration only.

## 2026-11-17 — Preserve abrupt declaration initialization

<a id="k-785"></a>
**K-785 — Keep initializer completion independent of receiving payload classification.** Source413→414/registry819→820 continues D-17/H-361/H-362/P-235–P-236, D-14/H-297/P-258, independent D-24 result evidence and D-26/H-407 under H-420/H-423. Untyped ordinary declarations preserve an initializer return/throw; no value is constructed or published on that path. Known declaration contracts, initialization/owner/return checks and H-525 obligations remain. Acceptance covers grouped effects, all declaration kinds, generic/recursive/borrowed/complete results, independent typed LLVM controls and native preparation-failure release. Preserve D-16 and charter §§2/3.2–3.3/4.3/5.3/6.3/8/12; no source non-returning type, new payload/capture permission or broader qualification. D-18r113’s separate review remains pending. Baseline: `2300d905de6316a1a0b16505290500ebc359f7eb:cod/lang_checked.hh` and that commit’s source reference/tests/registry.

## 2026-11-17 — Assess caller-provided outputs and named result construction

<a id="h-570"></a>
**H-570 — Propose unified output destinations, init place parameters and named result groups.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable. Requests assessment of all result forms under “caller allocated output var”, mixed named reception such as `{ val count, val ready } =<< inspect();`, and declaration-derived `=`/`<-` operations; explicitly treats `=<<` as provisional. Corrects remembered `out` to the discussed `init` direction, then suggests init means “caller allocated slot aliasing”. Provisionally proposes `return { n -> count, true -> ready }`, adding “先这么说着”. These are proposals/assessment instructions, not confirmation of the full result/parameter contract, grammar, ABI or value path.

<a id="k-output-assessment-20261117"></a>
**K-OUTPUT-ASSESSMENT-20261117 — Preserve result categories under a common destination proposal.** D-22r25→26/registry819→821 adds pending D-22-RESULT-DESTINATION/P-512–P-514 and extends pending D-22-ANON-DATA. Candidate C-54/C-63/C-65/C-72/C-73 → charter §§2/3.2–3.3/4.3/6.3/8/12 separates caller-owned hidden result storage from visible receivers, reference carriers from referents, and init place identity from payload readiness. Reviews named return construction, new-binder-only receiving with candidate `:=`, per-field operations, once-only effects, omitted-field cleanup and growth-safe destination forwarding. Local inspection and mainline's bounded read-only review find no general out/init implementation; existing callable destinations and assignment slots are scoped precedents. Shared ADT/owner/cleanup work remains substantial. Preserve D-23/D-24 modes/inference, D-17 completion, D-18r113's unchanged separate review, D-16 and no object copy/move/rollback. Baseline: `2300d905de6316a1a0b16505290500ebc359f7eb:design/ConstructionAndAcquisition.md`, its registry/source413, `cod/lang_syntax.hh`, `cod/lang_llvm.hh`, and `applications/cod-booter/docs/LLVMInterop.md`. Documentation only; no local code/build/tests or new human qualification.

## 2026-11-17 — Keep native binding roles with their slots

<a id="k-784"></a>
**K-784 — Consolidate native binding evidence.** Source412→413/registry818→819 continues D-26/H-407, existing D-14/D-18 frame/value/access contracts and unaffected D-17 completion under H-420/H-423. Keep the checked common place mode, reference slot and selected place in one native binding record; context-dependent interpretation facts and physical payload indices remain separate. Acceptance requires equivalent LLVM/diagnostics and established source/staging/native behavior with exact release. Preserve actual owners, qualifiers, initialization, prepared selections, D-16 and charter §§2/3.2–3.3/4.3/5.3/6.3/8/12. No new source policy, ABI, allocation contract or capture qualification; D-18r113’s human review remains pending. Baseline: `d96c4403d98b8b8d67751ef940dcd956afbd3c16:cod/lang_llvm.hh` and that commit’s source reference/registry.

## 2026-11-17 — Consolidated ordinary value-input review

<a id="k-783"></a>
**K-783 — Expose the bounded capture implementation contract for whole-file review.** D-18r112→113/registry817→818 preserves H-553’s source model and recorded H-327/H-329/H-342/H-344/H-417 qualifications, while making the new ordinary value-input implementation/value-path proposal explicitly pending-human under P-485–P-487. The profile covers existing scalar/intrinsic-exception and complete callable value contracts, declaration/access evidence, independent locals, ordinary ownership/failure cleanup and native/performance acceptance. C-65/C-73/C-75 serves charter §§2/3.2–3.3/4.3/6.3/8/12; dependency reuse and the supported-source reference experiment do not grant qualification. The compact authority summary preserves current scopes and exclusions; new borrowed/place capture proofs, symbols, aggregates, pure policy and permanent ABI remain separate. Existing source412/compiler9c195ce and D-16 are unchanged; no build/tests or human approval. Baseline: `9c195cecb9a6994edc28cef544674d2de784809c:design/CallableViewsAndCaptures.md` and that commit’s registry/changelog; original H-342/H-344 evidence remains at `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md#h-342`/`#h-344`.

## 2026-11-17 — Preserve declared places at the capture boundary

<a id="k-782"></a>
**K-782 — Reject unproved declared place captures.** Source411→412/registry816→817 continues D-18/H-327/H-329/C-75/P-295/P-297 and D-26/H-407 under H-420/H-423. Check original capture-source identity before ordinary place canonicalization; an explicit place cannot silently become a callable value snapshot. Explicit independent callable construction remains supported. Preserve owner/access/init, existing complete values/cleanup, D-16 and charter §§2/3.2–3.3/4.3/6.3/8/12. Source/native acceptance covers fixed/chained/selected places, declaration/capture forms, independent-value controls and exact failure cleanup. No new borrowed capture transport, lifetime extension or broader qualification. Baseline: `841222622ca30a9fd26de1a88091231193423131:cod/lang_checked.hh` and that commit’s source reference/tests; original human declared place alias evidence remains at `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md#h-327`/`#h-329`.

## 2026-11-17 — Partial named receiving and anonymous structural typing

<a id="h-569"></a>
**H-569 — Prioritize partial named receiving; assess structural anonymous data.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/name unavailable. “anonymous `data { ... }` 是不是适用 structural typing 规则最好？ 接收方有方便的语法去接收部分字段是很重要的ux价值，评估时考虑上”. Confirms partial reception's UX value and requests the structural typing assessment. Agent D-22r24→25/registry815→816 distinguishes structural anonymous products from named nominal ADTs, once-only subset unpacking from general width/depth subtyping, and per-field reception from non-bits whole-value copying. Exact identity/order/variance, result/receiving syntax and full value-path qualification remain pending. Preserve producer effects/cleanup, owner/access, H-563/H-567/H-568 and D-16; no implementation claim.


## 2026-11-17 — Consolidate mainline evidence and the design checkpoint

<a id="k-design-mainline-20261117"></a>
**K-DESIGN-MAINLINE-20261117 — Preserve both evidence histories and current contracts.** Merge main `2a385f2a1f4818ddebe964efb6c4303f3035c00a` with design `ea4ae30e3963773b04dfef87a25127748ec2f329` without rewriting commits cited by decision records. Registry814/811→815 retains source411/K-780/K-781, H-561–H-568 and the current D-18r112/D-22r24/D-28r30 scopes. Mainline's prepared-target fix and tests are unchanged; no new compiler feature, local build/test result or qualification is inferred. Preserve both parents' changelog payloads, actual owners, independent callable construction, bits transfer/graph-transfer distinctions, no C++ object copying/moving, D-16 and unrelated qualified contracts under charter §§2/3.2–3.3/4.3/6.3/8. Anonymous-data identity/result/unpacking and the full new ordinary-capture value path remain separately pending. Documentation/link/history checks only.


## 2026-11-17 — Callable storage, transfer semantics and anonymous-data direction

Human/source for H-563–H-568: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamps/personal name unavailable.

<a id="h-563"></a>
**H-563 — Keep callable-view values out of regions.** “region （为了relocatability）不能存 callable view value，只有 lexical-scope 可以存 ... 要落实到相关文档位置”. Confirms lexical callable-view storage and the regional prohibition, with relocation as the stated rationale. Does not select a regional encoding or extend captured owners.

<a id="h-564"></a>
**H-564 — Propose restricted tuples for multiple results.** Initially says ADTs cannot contain callable fields and suggests tuples excluded from regions and ADT fields, with proc/func returns and unpacking. H-565/H-566 subsequently reopens/replaces that proposed packaging approach; do not publish an unconditional all-ADT callable-field ban or an independent tuple mechanism as the final choice.

<a id="h-565"></a>
**H-565 — Consider storage-classified ADTs instead.** “另外一个方案是允许ADT包含 callable 类型字段，但这种ADT不能进region，要增加ADT kind： bits, with relocatable ptrs, with callables。” Proposes lexical-only callable-bearing ADTs and storage classification as an alternative to H-564. Exact field/type/result contracts remain under review.

<a id="h-566"></a>
**H-566 — Prefer anonymous data over positional-only tuples if costs are comparable.** “如果允许匿名 `data { ... }` 作为 "named tuple" 等价机制，就不需要 "positional-only tuple" 类型了，综合看工程成本，不是太悬殊的话优先选这个方向吧”. Establishes the preferred engineering direction and requests comparison. Does not independently choose anonymous structural identity, result allocation/ABI or unpacking syntax.

<a id="h-567"></a>
**H-567 — Only pure bits permit whole-value payload copying.** “在CoD的心智模型里，只有纯bits value满足 ‘safe to copy as a whole value.’，其它都不行，这点可以明确讲出来。” Rejects inferring non-bits aggregate copying from a lexical/callable-bearing storage category. Existing specifically defined callable construction/capture behavior remains separate, without granting generic aggregate copying.

<a id="h-568"></a>
**H-568 — No C++ object copying/moving model.** “CoD没有C++的 moving/copying 语义概念，相近的只有 graph transfer 概念”. Confirms the source-language principle. Agent integration distinguishes bits transfer, reference binding, release-responsibility transfer and dedicated callable construction from explicit graph transfer and from native implementation byte/C++ operations; it does not withdraw prior independent capture/lifetime contracts.

<a id="k-lexical-products-20261117"></a>
**K-LEXICAL-PRODUCTS-20261117 — Keep fixed boundaries separate from compound proposals.** D-18r111→112/D-22r23→24/D-28r29→30/D-12r30→31/D-14r119→120/D-10r196→197/D-15r46→47/D-07r165→166/D-13r32→33/registry813→814/AGENTS adds D-18-LEXICAL-STORAGE/P-503–P-505, D-22-TRANSFER/P-506–P-508 and pending D-22-ANON-DATA/P-509–P-511. Preserve lexical callable lifetimes, existing symbolic replay, bits-only payload operations, no implicit object copying/moving, actual owners, same-owner edges, prior qualified contracts and D-16 under charter §§2/3.2–3.3/4.3/6.3/8. Independent design/mainline source reviews find bounded marginal cost for fixed-name anonymous products versus equally capable tuples, with substantial shared construction/result/lifetime work; D-22 records the unresolved anonymous identity/result/unpacking contracts; no new aggregate implementation or full proof/physical/value-path qualification. Baseline/source inspection: `7701bc70316d08ee4cff7470ae67632e041e36e5:design/ConstructionAndAcquisition.md`, that commit's affected docs/registry and `cod/lang_syntax.hh`, `cod/lang_checked.hh`, `cod/lang_llvm.hh`. Documentation only; no build/tests.


## 2026-11-17 — Typed reservation uses the existing block distinction

<a id="h-562"></a>
**H-562 — Bare make versus completed construction needs no new question or keyword.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/name unavailable. Responding to bare `make T in o` reservation versus an extra init form, states “后面跟不跟一个 `{}` 就是够明显的语法区分了吧？ 这个你不该作为问题提吧”. Selects the existing initializer-block distinction and reiterates delegation of obvious derived choices. The preceding example uses ptr-held identities, field writes and a completed required-reference cycle before ordinary readable-ref use; no new runtime readiness or rollback contract follows.

<a id="k-typed-reservation-20261117"></a>
**K-TYPED-RESERVATION-20261117 — Integrate allocation identity and construction completion.** D-22r22→23/D-14r118→119/D-28r28→29/registry812→813 adds D-22-RESERVATION/P-500–P-502 under H-562 and existing H-428/H-445–H-447. Preserve eligible placement, owner/access, field/target readiness, empty-block completion checks, finite staged acceptance, failure allocation effects and D-16 under charter §§3.3/4.3/6.3/8. Ordinary constructor calls and intrinsic resource/exception contracts are not reinterpreted. Detailed aggregate algorithms/native support and full value-path qualification remain separate. Baseline: `720f828e73bb70cfe2bc4f19e88ab45a061e92a8:design/ConstructionAndAcquisition.md` and that commit's D-14/D-28/registry. Documentation only; no code/build/tests.


## 2026-11-17 — Accept owner-parameter reference symbols

<a id="h-561"></a>
**H-561 — Accept the owner-parameter form and bounded implementation direction.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “同意。 下个议题” accepts the preceding cost assessment and recommendation: `ref/ptr symbol @x:T in S where {owner S}` obtains one actual owner from each installation RHS, admitting lexical or ordinary regional targets without installing an owner symbol; begin with common proved calls/shadows/restoration and expand captured/returned-view acceptance as lifetime proofs become available. Invalid/unproved relationships reject without extending storage lifetime. This resolves H-560's conditional source direction; it does not approve a complete proof algorithm, runtime representation, combined group grammar or whole-document value path.

<a id="k-owner-symbol-accept-20261117"></a>
**K-OWNER-SYMBOL-ACCEPT-20261117 — Integrate the selected owner source.** D-10r195→196/D-07r164→165/D-12r29→30/D-13r31→32/D-14r117→118/D-15r45→46/D-18r110→111/D-22r21→22/D-28r27→28/registry811→812 updates D-10-OWNER-SYMBOL/P-497–P-499 and P-249 under H-561. The explicit owner-parameter form relaxes the backing-symbol prerequisite; region-selector/shared-region forms retain their declared constraints. Dependent changes preserve target versus binding-slot identity, actual-owner resolution, saved/captured lifetimes, permissions, no owner extension, natural nesting, prior qualifications and D-16 under charter §§2/3.2–3.3/4.3/6.3/8. D-10 owns source acceptance and remaining algorithms/ABI; no code support or complete value-path qualification is inferred. Baseline and cost evidence: `292c662e6ec65ed254451a5b9492f2674153ad07:design/SymbolicDynamicScope.md`, owner-parameter section, and that commit's affected documentation paths/registry. Documentation checks only; no build/tests.


## 2026-11-17 — Owner-parameter reference-symbol feasibility

<a id="h-560"></a>
**H-560 — Favor owner-parameter reference symbols subject to bounded incremental cost.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Responding to the proposed module declaration `ref symbol @current:u64 in S where {owner S};` with RHS-selected lexical or ordinary regional backing, says “我也觉得允许这种是非常好的，最开始想限制在symbolic是怕支持成本攀升，你仔细推演实现成本看看，不是悬殊的成本差异，我非常乐于见到这个形式受到支持”. Favors support conditional on a cost comparison with the region-symbol-backed form, and authorizes that review. Does not independently confirm a complete proof algorithm, native carrier, grouped grammar or implementation schedule.

<a id="k-owner-symbol-cost-20261117"></a>
**K-OWNER-SYMBOL-COST-20261117 — Separate shared reference support from owner-binding increment.** D-10r194→195/registry810→811 adds D-10-OWNER-SYMBOL/P-497–P-499 and its proposed bounded profile. Design and authorized mainline-developer read-only reviews find no inherent semantic blocker or separate backend, and recommend support under H-560's criterion. Substantial mode/provenance/save/restore/capture work is common with H-482; binder/RHS owner sources and lifetime cases add bounded work, while D-18/D-28's transitive borrow/current-base obligations remain substantial shared prerequisites. Preserve one actual owner, permissions, active/saved/captured validity, no owner extension, H-482 selector semantics, unrelated qualifications and D-16 under charter §§2/3.2–3.3/4.3/6.3/8. Detailed new source/proof/profile and full value-path/physical qualification stay pending-human; no code or native acceptance claim. Baseline/source evidence: `44bc1765be05f255e576cccce0f5ec4a2209546e:design/SymbolicDynamicScope.md` (reference-symbol sections), that commit's registry and `cod/lang_syntax.hh`, `cod/lang_checked.hh`, `cod/lang_llvm.hh`; mainline assessment received through the authorized chat, not human confirmation. Documentation checks only; no build/tests.


## 2026-11-17 — Prepared-readiness and terminology integration

<a id="k-781"></a>
**K-781 — Preserve readiness proofs and explicit identity distinctions.** Source410/409→411 and registry809/810→811 integrates K-780 with H-558/H-559's qualified glossary, canonical callable document and D-26r22 coverage within D-21r27. Prepared rebinds update selected destination slots without writing payloads; saved readiness supplies no proof for a different later target. Declared place aliasing, runtime target aliasing and name/identity aliasing retain separate contracts; overlap grants no exclusivity or lifetime extension. Preserve H-407's acceptance, P-258/H-525, owners/access, D-16 and charter §§2/3.2–3.3/4.3/5.3/6.3/8/12. Capture/borrow proof and physical/value paths, FIFO, return inspection and nested-defer enforcement remain separately scoped. No new human approval or runtime mechanism. Parents: `0642146b89e1574c1a0e7903f3c9f6894f48d0a0` compiler maintenance and `44bc1765be05f255e576cccce0f5ec4a2209546e` terminology/consolidation; their changelog records remain intact.

## 2026-11-17 — Canonical alias glossary and current design documents

<a id="h-558"></a>
**H-558 — Adopt qualified alias terminology and developer usage.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. “认同，落实你的词汇表，也提醒 CoD compiler 开发者，使用 alias 概念时，必须明确是哪一种情况，不可泛称”. Accepts declared place versus runtime target aliasing, their semantic-layer caveat, and mandatory qualified usage. The existing name/identity mapping remains separately named. Also requests considering a rename of design/RegionalClosures.md. This confirms terminology and documentation scope, not additional capture/escape or optimizer mechanisms.

<a id="k-alias-glossary-20261117"></a>
**K-ALIAS-GLOSSARY-20261117 — Publish the glossary and developer rule.** D-18r108→110/D-12r27→29/registry808→810 incorporates the two preceding terminology refinements under H-558/P-494–P-496. Rename D-18 to CallableViewsAndCaptures.md, add its canonical glossary and AGENTS rule, and repair current links; referenced documents retain their semantic revisions for link-only maintenance. Preserve runtime-indexed place selection, target overlap, independent bindings, name resolution, owner/access/lifetimes and D-16. C-59/C-65/C-75 and charter §§2/3.2–3.3/8 retain readable identity/storage distinctions; no new operational qualification or source implementation. Baseline: `aff17e114c195cce32f2426834cbb8807e699232:design/RegionalClosures.md`, compiler alias section, and that commit's affected documentation paths. Historical locators remain unchanged. Documentation only; no build/tests.

<a id="h-559"></a>
**H-559 — Remove resolved historical design trackers.** Same human/chat/date: “`GenericAndRecursiveClosure.md` 感觉是曾经悬疑，后来已经澄清解决的历史问题追踪，这种应该删除仅保留简单chengelog条目吧”. Requests review/removal of resolved issue history, with a brief changelog reference. This is documentation organization, not withdrawal of the file's independently confirmed active requirements.

<a id="k-aot-contract-consolidation-20261117"></a>
**K-AOT-CONTRACT-CONSOLIDATION-20261117 — Keep active native coverage in the AOT contract.** Remove GenericAndRecursiveClosure.md and its historical migration narrative; consolidate current requirements/acceptance as D-26r22 within D-21r27, with stable D-26/P-371–P-376 authority and repaired current links. H-407's qualified behavior, H-400–H-406/H-409/H-410 boundaries, actual identity/same-T, unavailable-interpretation failures, finite recursive coverage and concrete compiler diagnostics remain. Preserve all acceptance cases, pending algorithms/carriers/ABI, owner/access/completion and D-16 under charter §§2/3.1–3.3/4.3/5.3/6.3/8/12. No source or implementation change. Prior document: `aff17e114c195cce32f2426834cbb8807e699232:design/GenericAndRecursiveClosure.md`; verified H-407 evidence: `55667c37cd67131a50bdca6264478ab03084ae4f:design/DesignAuthorityRegistry.md#h-407`. Other dependent design revisions receive link/continuity updates only. Documentation checks only; no build/tests.

## 2026-11-17 — Declared place aliases versus runtime target identity

<a id="k-alias-layers-20261117"></a>
**K-ALIAS-LAYERS-20261117 — Qualify compile-time/runtime aliasing by semantic layer.** Refines the unpublished D-18r109/D-12r28/registry809 checkpoint and K-FORMAL-ALIASING-20261117's overbroad “all aliasing is explicit” wording. Current project user in [the design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles (exact timestamp/name unavailable), states runtime object-identity aliasing through ref slots is allowed without a formal explicit alias relation, and asks whether compile-time/runtime terminology should distinguish it. Agent clarification retains H-327/H-329/H-553–H-557/P-488–P-493: declared place alias semantics are explicit and statically checked, but runtime indices may select the place; independent references may coincide without a pairwise declaration, even when that equality happens to be statically provable. Runtime behavior remains formally specified. This is terminology/continuity, not a new alias restriction or exact-target solver. Acceptance includes runtime-indexed place aliases, coincident indices, same-target parameters and independent ref slots. Preserve owner/access/lifetime checks, permitted inference, overlapping writes, current qualification/support limits and D-16 under charter §§2/3.2–3.3/4.3/6.3/8. Committed baseline: `aff17e114c195cce32f2426834cbb8807e699232:design/RegionalClosures.md`, alias section, and that commit's vision/registry. No code/build/tests.

## 2026-11-17 — Explicit formal aliasing terminology

<a id="k-formal-aliasing-20261117"></a>
**K-FORMAL-ALIASING-20261117 — No hidden slot aliasing, without an exclusivity promise.** D-18r108→109/D-12r27→28/registry808→809 continues H-327/H-329/H-553–H-557 and P-488–P-493 after the current project user asks whether CoD aliasing should always be “explicit formal aliasing”, with no implicit aliasing. Source: [design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact timestamp/personal name unavailable. Agent clarification: explicit means licensed by a formal source storage/borrow contract, including permitted inference; the same-target call example remains legal and observes overlapping writes through independent parameter bindings. Unique owner and readonly access do not prove distinct targets or exclusivity. This continues the existing identity/storage value under charter §§2/3.2–3.3/4.3/6.3/8, not a newly human-approved alias restriction, precise-target solver or optimizer mechanism. Preserve lifetimes, assignment-family destination contracts, current qualification/support limits and D-16. Baseline: `aff17e114c195cce32f2426834cbb8807e699232:design/RegionalClosures.md`, compiler alias section; that commit's vision and registry. Documentation only; no code/build/tests.

## 2026-11-17 — Preserve prepared source-target initialization

<a id="k-780"></a>
**K-780 — Keep saved target readiness separate from live slots.** Source409→410/registry808→809 continues H-297/P-258, existing D-17 prepared operands/cleanup, H-525 and D-26/H-407 under H-420/H-423. Prepared rebind summaries detach registration-time source identity from invocation-time slots; rebinds supply no physical payload writes. Acceptance includes independent slot/payload models, unchanged initialization rejection tests, prepared/returned-view composition and native cleanup. Broad declared-Exception diagnostic expectations retain H-525's rejection contract. Charter §§3.2/3.3/4.3/5.3/6.3/8/12 and D-16 continue; no capture/FIFO/return-state policy, runtime flag or new qualification. Baseline: `7a6847fa1f771bc9acd32a80a13fb8fac0c4083b:cod/lang_checked.hh` and that commit's source reference/authority registry.

## 2026-11-17 — Explicit captures, aliases and lifetimes

<a id="h-553"></a>
**H-553 — Ordinary captured values are invocation inputs.** Human/source for H-553–H-557: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-17 America/Los_Angeles; exact message timestamps/personal name unavailable. After the implicit-value-parameter explanation (saved scalar/callable snapshot, independent local on each call), the user confirms: “对，allocation failure （OOM）是极端情况，不要让它冲击正常语义推演；allocation该谁分配就谁分配，没有内部引用 outliving，没有 lexical-scope-back-storage / region 泄漏，就不影响 CoD soundness。” Requests the general scalar rule and quirks. Confirms the value model and allocation/lifetime criterion, not a particular layout, allocation count or new pure-callable policy.

<a id="h-554"></a>
**H-554 — No hidden shared captured-variable cells.** Same human/chat: “CoD的原则是不创建传统的closure提供共享可更新slots，这方面需求要求用户通过显式的 dynamic-scoped symbolic variables 表达，共享可更新slots必须是显式静态追踪的某region作为owner，也就是 `in @region_owner` 声明”. Confirms explicit dynamic context/owner responsibility instead of closure-created shared slots; existing explicit object/place borrowing is not withdrawn.

<a id="h-555"></a>
**H-555 — Permit proved lexical-owner borrows.** Same human/chat: “静态可以证明不会逃逸的 `in some_lexical_owner` 引用也允许”. Extends the preceding lifetime criterion to explicit lexical-owner references with a static non-escape proof. Does not approve a new lexical symbol-declaration grammar, owner binder, escape-analysis algorithm or source-lifetime extension.

<a id="h-556"></a>
**H-556 — Put explicit lifetimes in the vision.** Same human/chat: “CoD的设计目标之一就是消除这种隐式生命周期机制： lexical-scope 的自然嵌套结构生命周期自然允许，打破这种well nesting的生命周期，必须是形式化可静态检查的 dynamic scoping。 GlobalVariablesDoneRight vision 应该明确阐述这个设计原则”. Explicitly requests compiler-developer guidance against traditional closure lifetime extension. Confirms the principle/value and its place in D-12, not all dependent operational mechanisms or Actor/continuation lifetimes.

<a id="h-557"></a>
**H-557 — Review explicit aliases and strengthen compiler guidance.** Same human/chat asks whether “`var v -> some_slot` 才是传统的 alias ... 而且是显式声明的” and requests broad analysis; then instructs “这个说法确认成立以后，提醒 CoD compiler 开发者时刻抵制传统 变量 aliasing 的语义诱惑”. The later question “是否可以更坚决地说： CoD中没有 ‘closure’ 设计？” requests assessment of stronger terminology. These are review/guidance instructions, not independent approval of every derived mechanism. Agent reconciliation below retains the existing confirmed explicit-place contract and defines “no closure abstraction” without denying captured values.

<a id="k-capture-lifetimes-20261117"></a>
**K-CAPTURE-LIFETIMES-20261117 — Separate values, reference targets and explicit places.** D-18r107→108/D-12r26→27 records H-553–H-557; D-14r116→117/D-10r193→194/D-21r25→26/D-22r20→21/D-28r26→27/source408→409/registry807→808 aligns dependencies. P-485–P-493 serves inspectable storage, identity and lifetime responsibility under charter §§2/3.2–3.3/4.3/6.3/8; source decisions and the explicit-lifetime vision have stated confirmation, full new proof/physical/value-path qualification remains separate. Agent elaborations preserve qualifiers, snapshot timing, independent nested values, transitive owner proofs, failure cleanup and conservative rejection. A ptr snapshot reads its reference value, not pointee bits; ref retains its ordinary obligations. Explicit var aliases preserve selected places through capture; ordinary reference captures retain targets. This preserves H-327/H-329 rather than silently converting alias captures to snapshots. The draft's blanket alias-snapshot interpretation and callable-only readonly recommendation are withdrawn; neither is an accepted contract. “No closure abstraction” names the confirmed no-hidden-cell/no-extension model, while allowing callable views, value captures and explicit borrows. Existing symbol declaration restrictions, assignment-family destination slots and D-16 remain. Acceptance covers scalar/callable repeated calls, local mutation, explicit alias observation, target/slot independence, null/uninitialized-pointer selection and rejected owner escape/promotion. Mainline peer review supplied proof/cost checks, not human confirmation. New source paths remain implementation work; no code/build/tests. Baseline: `fb765cf2305ad713b127c653ea4c16d4fcc2838c` at the affected documentation paths. Verified alias evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md#h-327` and `#h-329`; complete-value/heap-backing evidence at `#h-342`/`#h-344` in that commit. K-779 and unrelated qualified behavior continue.

## 2026-11-17 — Preserve checked native emission contexts

<a id="k-779"></a>
**K-779 — Keep native operation evidence with its interpretation.** Source407→408/registry806→807 continues D-26/H-407, D-18, D-14 and unaffected D-17/H-525 under H-420/H-423. Paired operation/instance records and scoped restoration replace parallel native-emission state. Acceptance preserves LLVM/diagnostics for unchanged programs in both native modes and established capture/owner/cleanup observations; H-481 copy-failure injection uses a stored callable value. Charter §§3.2/4.2/5.3/6.3/12 and D-16 continue; no language policy, runtime representation or new qualification. Baseline: `a98a57745809ebb6850bc6191576ffa68d8da2c1:cod/lang_llvm.hh` and that commit's source reference/authority registry.

## 2026-11-17 — Swap-method signature inspection

<a id="k-778"></a>
**K-778 — Keep physical invocation proofs out of signature-only queries.** Source406→407/registry805→806 continues H-295/P-257, H-306/P-269/H-366–H-369/P-327, D-18's query boundary, D-24 independent result evidence and D-26/H-407 under H-420/H-423. Shared method selection validates expression access and owners for swap queries; actual invocation alone requires the body's physical binding-preservation proof. Acceptance covers readonly aliases, generic/retained/imported contexts, rejected later unsafe calls, equivalent LLVM and unchanged native resource observations. Builtin fallback, fixed slots, owners, initialization and D-16 remain; charter §§3.3/4.3/5.3/6.3/8/12 continues. No new policy, runtime type facility or qualification. Baseline: `bf15155dc8c5b04973460b70a8cba726101cc751:cod/lang_checked.hh` and that commit's source reference/authority registry.

## 2026-11-16 — Caller-access and defer-nesting integration

<a id="k-777"></a>
**K-777 — Align caller access with independent cleanup boundaries.** Source405 on both branches→406/registry804→805 preserves K-776's qualified caller-binding/access implementation with H-552/D-17r87/D-15r45/D-10r193/D-21r25. Nested defer declarations reject lexically; separately checked helpers retain independent cleanup. Existing nested-registration compiler acceptance remains an enforcement gap and does not qualify that source contract. Unaffected owner/access/initialization, ordinary completion, native coverage and D-16 continue under charter §§2/3.3/4.3/5.3/6.3/8/12; full queue/value-path/physical qualification remains separate. No new runtime mechanism or human confirmation. Integrated design: `9881df9be5d1facebd6ecbff9de8316052f24f74`; compiler baseline: `022272847bd7c31a7512f774894463add7fc78ac:cod/lang_checked.hh` and that commit's source reference.

## 2026-11-16 — Caller-binding target-access preservation

<a id="k-776"></a>
**K-776 — Prove physical slot access through readonly aliases.** Source404→405/registry803→804 continues D-14r116/P-269/P-327 under H-306/H-366–H-369, D-18r107, unaffected D-17 completion/H-525 and D-26r21/H-407. Finite input-source summaries retain intermediate selections through nested/recursive calls and cleanup; every source must preserve the physical destination's access. Exact expression/formal qualifiers and ref validity remain. Acceptance covers independent write-history models, restoration-negative controls, prepared slots, callable targets, shared native bodies and exact release. Charter §§3.3/4.3/5.3/6.3/8/12 and D-16 continue. Alias-conditioned mixed-access precision, caller ref/non-ref adaptation, method-value and pending runtime/ABI protocols remain separate; no new policy, permission flag or human confirmation. Baseline: `261f0960538fce7caeaa468afebb864e4be2daf4:cod/lang_checked.hh` and that commit's source reference/authority registry.

## 2026-11-16 — Reject nested defer declarations

<a id="h-552"></a>
**H-552 — Reject nested defer declarations.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamps/personal name unavailable. “reject嵌套defer声明”, stated directly and repeated in the answer to whether a handler registered during cleanup may inspect earlier pending errors. Selects rejection instead of either proposed dynamic-registration eligibility policy. This settles the nested-declaration source boundary; it is not a ban on calling helpers that have independent cleanup, on automatic owner release, or a full queue/ABI approval.

<a id="k-defer-nesting-20261116"></a>
**K-DEFER-NESTING-20261116 — Bound registration before draining.** D-17r86→87/D-15r44→45/D-10r192→193/D-21r24→25/source404→405/registry803→804 records H-552 under D-17-DEFER-DISPATCH/P-461–P-463. Reject nested declarations in deferred bodies, catches and finally through grouping, control flow and explicit full scopes. Separately checked callable bodies retain independent cleanup; inlining preserves that boundary, and intrinsic owner/effect cleanup remains. No owning action list grows during its drain. Candidate C-36/C-44/C-63/C-65 serves explicit registration, observable order and checkable failure responsibility under charter §§2/3.1–3.3/4.3/6.3/8/12. Preserve FIFO, H-525/H-550/H-551, return semantics, owners and D-16. Acceptance covers rejected nesting forms, valid independent helpers and ordinary pre-cleanup loops/scopes. Existing nested-registration and throws-only compiler acceptance is identified as a source enforcement gap; it does not qualify either target behavior. Full queue/value-path/physical qualification remains separate; unaffected qualified contracts continue. Baseline: `261f0960538fce7caeaa468afebb864e4be2daf4:design/ExceptionControl.md`, deferred-work/open-registration sections, and the other affected documentation paths at that commit. Documentation only; no code/build/tests.

## 2026-11-16 — Native-validation and deferred-coverage integration

<a id="k-775"></a>
**K-775 — Integrate checked defer-pass coverage with native validation.** Source403 on both branches→404/registry801 and802→803 preserves K-774's immutable fixture evidence with H-549–H-551 and D-17r86/D-15r44/D-10r192/D-21r24. H-550/H-551 replaces declared unhandled deferred escape with handling proved at completion of the owning pass; covering deferred catches count, throws alone does not. Current compiler tests do not establish this new boundary, FIFO aggregation or return inspection. Unaffected owner/access/initialization, ordinary declared obligations, native coverage and D-16 continue under charter §§2/3.3/5.3/6.3/8/12. No new source implementation or qualification. Integrated design: `d90b20ab6dd2f72ec7933e19032e0680876436e0`; validation baseline: `f9e394bc590a2f34ac37d8b02be34205fc98d62f:tests/cod/language/README.md`.

## 2026-11-16 — Independent native validation fixtures

<a id="k-774"></a>
**K-774 — Isolate native fixtures and centralize validation support.** Source402→403/registry800→801 continues the existing qualified compiler/native acceptance under H-420/H-423 and D-26/H-407. Immutable fixture derivation preserves source, wrapper/assertion and observation inputs across import orders; shared support owns commands, diagnostics, native execution, provider injection and helper provenance. Acceptance compares all 128 fixture inputs with isolated baselines, checks shuffled imports and transitive dependencies, and preserves current source/native results. Charter §§5.3/6.3/8/12, owner/access/completion obligations and D-16 continue. No language, runtime, ABI or qualification change; return inspection remains design-only. Baseline: `d7ba556989aee827d7690dd54cca116a52c7de9b:tests/cod/language/exception_value_runtime_tests.py` and `e7f310b22e5467bb45329a9e6a78b8491e6ae8e0:tests/cod/language/README.md`.

## 2026-11-16 — Require checked handling before defer-pass completion

<a id="h-550"></a>
**H-550 — Reject unchecked packaging of unhandled checked deferred failures.** Human/source for H-550/H-551: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamps/personal name unavailable. Answering the caller-side FlushException/UnhandledDeferError mismatch: “checked exception 不允许从 defer 块抛出吧，要求用户处理，静态守住以后 UnhandledDeferError 就只体现 unchecked exception”. Replaces H-536's permission to declare unhandled checked deferred escape with a static handling requirement. H-551 below fixes the boundary: covering defer catch is valid handling; an individual deferred entry need not itself be nonthrowing.

<a id="h-551"></a>
**H-551 — Count defer catch as user handling.** Same human/chat/date: “defer catch 认为是用户handle了的呀”. Corrects the agent's too-early individual-entry boundary. Checked failures may enter the pending FIFO and be handled by later-visited covering deferred handlers; none may remain at completion of the owning pass. Enclosing throws alone does not satisfy H-550. Existing local/attached catches also retain their coverage. This preserves H-536's covering-handler direction while replacing its declared-escape permission; original evidence is located by K-DEFER-CHECKED-COVERAGE-20261116.

<a id="k-defer-checked-coverage-20261116"></a>
**K-DEFER-CHECKED-COVERAGE-20261116 — Apply the completed-pass rejection boundary.** D-17r85→86/D-15r43→44/D-10r191→192/D-21r23→24/source402→403/registry801→802 records H-550/H-551 under D-17-DEFER-DISPATCH/P-461–P-463 and module shutdown P-458–P-460. Candidate C-36/C-44/C-63/C-65 serves explicit recoverability, ordered handling and valid resources under charter §§2/3.1–3.3/4.3/6.3/8/12. Acceptance includes covering local/deferred catches; rejected throws-only, wrong-order and unproved registration paths; handler/finally checked outputs; H-525 declared-root obligations; and unchecked-only residual members. Nested aggregates remain opaque; H-538's independent body_exception and explicit causes may retain ordinary checked failures without making them queue members. Preserve return replacement, FIFO, owner/access/init and D-16. Only the prior declared deferred-escape permission loses authority; unrelated qualified contracts continue. Full queue value-path/proof/physical qualification and runtime support remain separate. Baseline and verified earlier evidence: `a823d12cd35bfcc3ff3b7b33cdc36ccefff3d545:design/ExceptionControl.md` and that commit's `CHANGELOG.md#h-536`/`#h-537`/`#h-538`; affected dependent paths at the same commit. Documentation only; no code/build/tests.

## 2026-11-16 — Return-pattern typing and materialization

<a id="h-549"></a>
**H-549 — Keep return inference independent of inspection.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. “if returning 只消费已经 inferred 返回类型，不参与 return type inference。” Confirms the bounded profile's one-way type dependency. The accompanying request for binding-failure examples is a clarification request, not confirmation of a new runtime-check mechanism.

<a id="k-returning-binding-20261116"></a>
**K-RETURNING-BINDING-20261116 — Explain existing complete-value allocation failure.** D-17r84→85/registry800→801 preserves H-548's bounded profile, with H-549 explicitly confirming inference direction. Matching adds no conversion, dereference, allocation or assertion; bits/ref/ptr binding adds no special fallible operation. D-18's ordinary complete-callable environment copy can allocate and raise unchecked OutOfMemoryError before branch entry; no fallback retry. The example distinguishes this from invoking the successfully bound callable. Candidate C-54/C-63/C-65/P-482–P-484 and charter §§2/3.2–3.3/4.3/6.3/8/12 preserve observable completion and valid storage; no new mechanism or value-path qualification. Owners/lifetimes, FIFO, declared obligations and D-16 continue. Baseline: `e7f310b22e5467bb45329a9e6a78b8491e6ae8e0:design/ExceptionControl.md`, return-state section; D-18 dependency at that commit's `design/RegionalClosures.md`, lexical payload/copy contract. Documentation only; no source/build/tests.

## 2026-11-16 — Bounded return-inspection profile

<a id="h-548"></a>
**H-548 — Delegate remaining return-inspection choices.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. “`if returning` 还有剩余的设计问题？  按实现简单且语义自洽的原则选方案”. Delegates the remaining if-returning choices using implementation simplicity and semantic coherence. This is confirmation of the decision scope/criterion, not a direct human selection of each outcome or blanket qualification of the full value path, ownership transport or future pure-callable contract.

<a id="k-returning-profile-20261116"></a>
**K-RETURNING-PROFILE-20261116 — Close the bounded source profile under H-548.** Agent selections: explicit return; establishes a no-payload marker, void fallthrough does not; existing modal/typed declarators supply patterns without new inference seeds; bits/complete-callable copies and borrowed targets reuse ordinary binding lifetimes; binding failure propagates without branch retry; owning results permit non-capturing inspection only. Reuse existing completion/copy/borrow rules and conservatively reject unproved lifetime/evidence instead of adding ownership or hidden-slot aliases. Continues H-542–H-547's recovery, filters and scoped cancellation; preserves FIFO, declared checked obligations, exact release and D-16. D-17r83→84/D-23r17→18 records acceptance, with D-10r190→191/D-15r42→43/D-21r22→23/source401→402 aligning dependencies, registry799→800. Candidate C-54/C-63/C-65/P-482–P-484 serves author-controlled completion, observable state and valid storage under charter §§2/3.2–3.3/4.3/6.3/8/12; full mechanism/value-path and physical qualification remains pending. Owning capture, new pure-callable rules and general regional copying are excluded; caller/nested aggregation and drain registration remain separate. Baseline: `d7ba556989aee827d7690dd54cca116a52c7de9b:design/ExceptionControl.md` and that commit's other affected documentation paths; prior human evidence: `907d93ef836db06c723ca00fe0166efede31c0c9:CHANGELOG.md#h-542` through H-547. Documentation only; compiler support is not implied, no code/build/tests.

## 2026-11-16 — Borrowed-result initialization

<a id="k-773"></a>
**K-773 — Preserve sufficient initialization of saved borrowed results.** Source400→401/registry798→799 continues K-772's qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-356/H-525, D-28-IDENTITY, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Pending results use the shared initialization relation through cleanup, replacement and alternative returns; read/result summaries share formal projection and actual substitution. Acceptance covers saved targets, late writes versus rebinding, failed-registration prefixes, direct/generic/recursive/callable forwarding, null/access/owner rejection, shared native bodies and exact release. Charter §§2/3.3/4.3/5.3/6.3/8/12, ordered reads, ref return-site validity, declared obligations and D-16 continue. Wider caller-slot relationships and new runtime/ABI/queue/return-inspection or aggregate protocols remain separate. Baseline: `e797d303d314e11b6a850ddd294b7239017426a8:tests/cod/language/README.md` and that commit's `design/DesignAuthorityRegistry.md`. No new source policy, runtime flag or human confirmation.

## 2026-11-16 — Prepared initialization provenance

<a id="k-772"></a>
**K-772 — Retain prepared initialization through owning frames and calls.** Source399→400/registry797→798 continues K-771’s qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-356/H-525, D-28-IDENTITY, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Shared sufficient conditions distinguish payload-place facts from captured entry selections and cross nested cleanup at the matching proof frame. Borrowed-input read summaries project them onto prepared actuals. Acceptance covers independent repeated-capture models, direct/generic/recursive/callable invocation, late writes versus slot changes, weak/alias destinations, failed registration, native sharing and exact release. Preserve charter §§2/3.3/4.3/5.3/6.3/8/12, owners/access/nullness, fresh storage, ordered reads/writes, declared obligations and D-16. Wider conditional invariants, caller-slot relationships and new runtime/ABI/queue/return-inspection or aggregate protocols remain separate. Baseline: `79ab06728798226a2847dca2a7e7af6d53b57415:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new source policy, runtime initialization flag or human confirmation.

## 2026-11-16 — Inductive selected-target initialization

<a id="k-771"></a>
**K-771 — Compose deferred-loop initialization conditions.** Source398→399/registry796→797 continues K-770’s qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-28-IDENTITY, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. One sufficient-condition representation covers selected views, loop assumptions and composed reads. Acceptance covers invocation-time rebinding, copied/borrowed results, weak and handler joins, independent loop models, prepared snapshot boundaries, failed work, native sharing and exact release. Preserve charter §§2/3.3/4.3/5.3/6.3/8/12, actual owners/access/nullness, fresh iteration storage, declared obligations and D-16. Conditional registration-entry transport, wider invariants and new runtime/ABI/queue/return-inspection or aggregate protocols remain separate. Baseline: `83f4c7a1606cf8407347d9e9112df7f9035f3c82:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

<a id="k-770"></a>
**K-770 — Preserve initialized selections across loop and cleanup joins.** Source397→398/registry795→796 continues K-769/K-738's qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-28-IDENTITY, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Finite loop assumptions require initial-entry and reaching-backedge evidence before read validation. Shared selection summaries retain sufficient entry-target initialization through conditional cleanup. Acceptance covers independent direct/deferred binding models, nested loops/folds, snapshots, failed work, aliases/caller slots, native sharing and exact release. Preserve charter §§2/3.3/4.3/5.3/6.3/8/12, actual owners/access/nullness, fresh iteration storage, ordered read/write obligations and D-16. Wider invariants and new runtime/ABI/queue/return-inspection or aggregate protocols remain separate. Baseline: `83e5da3201f18746328e090c5049a47c9a39b03b:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

## 2026-11-16 — Selected identities and concrete targets

<a id="k-769"></a>
**K-769 — Connect observed identities with known target bounds.** Source396→397/registry794→795 continues K-768's qualified D-28-IDENTITY, D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. The shared condition relation combines finite target bounds with immutable equality/difference observations and prepared-argument substitution. Acceptance covers selected-cell writes, conditional borrowed reads, independent cell models, snapshots, failed work, fresh iterations, generic/recursive native sharing and exact release. Distinct local-cell comparisons retain their actual impossible outcomes; borrowed placeholders remain potentially aliasing. Preserve charter §§2/3.1–3.3/4.3/6.3/8/12, owner/access/ref-entry/init, conservative null/unknown bounds, declared completion obligations and D-16. Wider caller-slot proofs and new runtime/ABI/return-inspection/queue or aggregate initialization protocols remain separate. Baseline: `f48037aecafb88e50a29bf114921f6e36f113c07:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

## 2026-11-16 — Complete reference-comparison evidence

<a id="k-768"></a>
**K-768 — Preserve both observed identity outcomes through the shared proof path.** Source395→396/registry793→794 continues K-767's qualified D-28-IDENTITY, D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Equality and inequality observations share predicate constraints with guarded result joins; unknown and impossible evidence remain distinct. Acceptance covers complementary writes, finite reference partitions, saved/transitive observations, changed selections, ordinary/prepared calls, cleanup/failure paths, native sharing and exact release. Preserve charter §§2/3.1–3.3/4.3/6.3/8/12, owners/access/ref-entry/init, declared completion obligations and D-16. Wider caller-slot proofs and new runtime/ABI/return-inspection/queue or aggregate initialization protocols remain separate. Baseline: `739018306e019117df1477e8a334c33f3091db12:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

## 2026-11-16 — Borrowed selected initialization

<a id="k-767"></a>
**K-767 — Compose selected writes without assuming borrowed inputs are disjoint.** Source394→395/registry792→793 continues K-766's qualified D-28-IDENTITY, D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Immutable incoming-reference relationships extend the shared condition proof; finite origin alternatives retain necessary missing-write conditions across calls. Acceptance covers alias partitions, guarded reads, registration/recovery alternatives, mutation/failure controls, generic/recursive native sharing and injected failures with exact release. Preserve charter §§2/3.1–3.3/4.3/6.3/8/12, owners/access/init, conservative unknown origins and terminal/exceptional outcomes, and D-16. Wider caller-slot proofs and new runtime/ABI/return-inspection/queue or aggregate initialization protocols remain separate. Baseline: `f7042aaf7a3eb29ca5e5516ee3f8e8b111caf181:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

## 2026-11-16 — Conditional selected initialization

<a id="k-766"></a>
**K-766 — Compose selected writes across cleanup registration alternatives.** Source393→394/registry791→792 continues K-765's qualified D-28-IDENTITY, D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 results and D-26r21/H-407 under H-420/H-423. Immutable target conditions extend the shared initialization relation; cleanup joins retain completed conditional writes. Acceptance covers guarded reads, registration/recovery alternatives, saved/prepared selections, mutation/failure controls, generic/recursive native sharing and injected failures with exact release. Preserve charter §§2/3.1–3.3/4.3/6.3/8/12, owners/access/init, potentially aliasing borrowed inputs, conservative terminal/exceptional call outcomes and D-16. Wider alias proofs and new runtime/ABI/return-inspection/queue or aggregate initialization protocols remain separate. Baseline: `f69732d04bea232e6af00c2930b6e76d55bdd950:tests/cod/language/README.md` and that commit’s `design/DesignAuthorityRegistry.md`. No new semantic decision or human confirmation.

## 2026-11-16 — Nullable reference joins

<a id="k-765"></a>
**K-765 — Preserve path evidence through reference results and cleanup entry.** Source392→393/registry790→791 continues K-764 under H-420/H-423 and its qualified D-28-IDENTITY, D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 and D-26r21/H-407 scopes. Nullable conditional/handler results share outcome evidence; recursive closure distinguishes provisional values from unknown outcomes; deferred reads use proven equal incoming selections with retained preceding writes. Acceptance covers mutation/failure controls, generic/mutual recursion, native sharing and exact release. Preserve charter §§2/3.1–3.3/4.3/6.3/8/12, owner/access/init and D-16. Wider proof precision, new runtime/ABI/return-inspection/queue and aggregate initialization protocols remain separate. Baseline: `4ed7f4dadca17ed3becf1cc87a7d13719be48db1`, affected compiler/source-reference paths. No new semantic decision or human confirmation.

## 2026-11-16 — Selected-reference equality proofs

<a id="k-764"></a>
**K-764 — Carry observed reference equality through shared proof boundaries.** Source391→392/registry789→790 continues K-763 and D-28-IDENTITY/C-95/P-391–P-392 under H-439–H-441/H-420/H-423. Immutable selection equalities compose with Boolean/call summaries, preceding-write obligations and branch joins without merging binding slots or capabilities. Acceptance covers saved/transitive guards, generic/recursive/callable and borrowed-output paths, ordered snapshots, failed work, cleanup/loops, native sharing and exact release. Preserve qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280/H-525, D-18r107, confirmed D-23r17 results, D-26r21/H-407 and charter §§2/3.1–3.3/4.3/6.3/8/12. Source exclusions for recursive materialization are reconciled with H-481's implemented contract. Wider caller-slot/multi-origin proofs, D-16 and new runtime/ABI/return-inspection/queue scopes remain separate. Baseline: `4122d591914f4f6e5a05c1c92f6a501d7a4d2d48`, affected compiler/source-reference paths. No new semantic decision or human confirmation.

## 2026-11-16 — Returned selection initialization

<a id="k-763"></a>
**K-763 — Preserve selected-reference identity through copies and results.** Source390→391/registry788→789 continues K-762's qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280, D-18r107, confirmed D-23r17 scalar/ref/ptr results and D-26r21/H-407 under H-420/H-423. Compact immutable selection evidence composes with shared initialization/return summaries and prepared-argument substitution; binding slots, possible-target bounds and nullness remain separate. Acceptance covers copied/returned writes, branch/weak/loop/slot changes, ordered operands, failed work, cleanup replacement, shared native bodies and exact release. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owner/access/init and D-16 continue. Wider caller-slot/multi-origin proofs and new runtime/ABI/return-inspection/queue contracts remain separate; no new source rule or human confirmation. Baseline: `3e5209d3a4825a22f8fb128be8913627600b998f`, affected compiler/source-reference paths.

## 2026-11-16 — Nullable result conditions

<a id="k-762"></a>
**K-762 — Retain borrowed-result nullness through shared summaries.** Source389→390/registry787→788 continues K-761's qualified D-14r116/P-258, D-17r83/P-235/P-236/P-280, D-18r107, confirmed D-23r17 scalar/ref/ptr results and D-26r21/H-407 scope under H-420/H-423. Result-nullness conditions compose through preparation, branches/handlers, cleanup and argument substitution while mutable slots, actual targets and payload facts remain independent. Acceptance covers forwarding/recursion/callable values, independent alternatives, changed operands, recovery/replacement, native sharing and exact release. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owners/access/init and D-16 continue. Wider caller-slot/multi-origin proofs and new runtime/ABI/return-inspection/queue contracts remain separate; no new source rule or human confirmation. Baseline: `c6fd1ca63b45d369e683a72222ea20cd3b55ab82`, affected compiler/source-reference paths.

## 2026-11-16 — Return-state design and compiler evidence integration

<a id="k-returning-sync-20261116"></a>
**K-RETURNING-SYNC-20261116 — Preserve scoped completion decisions and copied-reference evidence.** Integrates H-542–H-547/K-RETURNING-20261116 in `907d93ef836db06c723ca00fe0166efede31c0c9` with K-761 in `a4635e7424560cfde087f8040d3e7d71acc8a2e0`, affected design/source-reference paths. Source388→389/registry786→787 preserves D-17r83/D-23r17/D-10r190/D-15r42/D-21r22 and existing qualified compiler contracts. Explicit return recovery and scoped ordinary cancellation remain separate from FIFO failures; return inspection, full value-path qualification and owned/no-value/physical protocols remain pending. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525/H-536–H-541, owners/access/init and D-16 continue. Both decision histories and compiler validation scope are retained; the merge adds no executable support or new human confirmation.

## 2026-11-16 — Return inspection, explicit recovery and scoped cancellation

<a id="h-542"></a>
**H-542 — Permit explicit cleanup-return recovery.** Human/source for H-542–H-547: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamps/personal name unavailable. After proposing if returning to retain author control over returned results, the user answered the recovery-versus-normal-only question: “允许 `else` 块里的 `return xxx;` 让body throw的情况也正常return”. Confirms that explicit cleanup return can turn an ordinary body failure into normal return; it does not erase unrelated deferred failures or itself recover module startup.

<a id="h-543"></a>
**H-543 — Inspect an established return.** Same human/chat/date: “心智模型是只要执行到 `return xxx;` 就算数， `if returning (xxx:T) { ... } else { ... }` 可以判断之前是否已经有了 return 指令。” Selects invocation-local established-return inspection and replacement, with the latest completed return supplying the result. The initially discussed persistence across ordinary exceptions is narrowed by H-546/H-547 below; this event is not a permanent-history or physical-retention ABI approval.

<a id="h-544"></a>
**H-544 — Optional return filters and non-capturing inspection.** Same human/chat/date: “对的，补充一下： `if returning () { ... } else { ... }` 允许匹配任何返回值，且不capture； 或者 `if returning (in o) ...` 要求明确的owner内引用；或者 `if returning (val rtn) ...` 。另外指定了类型也要匹配，否则走下一个分支。 你推演看看”. Accepts the presented successful-preparation/result-slot model and adds unrestricted non-capturing, owner and val filters, with typed mismatch trying the next branch. Complete grammar, owned-result binding and physical transport are not confirmed by this statement.

<a id="h-545"></a>
**H-545 — Match an existing owner.** Same human/chat/date; explicit choice for whether in o filters a visible owner or binds a fresh owner: “Filter by an existing owner”. Confirms the filter interpretation, not a new existential owner declaration, owner union, ownership transfer or stronger access.

<a id="h-546"></a>
**H-546 — Review clearing a return on an ordinary body failure.** Same human/chat/date, replying to the interrupted-return/outer-catch/fallthrough example: “body里的异常抛出，我倾向于直接清return，你推演看看”. Requests the ordinary-failure clearing direction; the agent then distinguishes escaping cancellation from a failure handled wholly inside finally. This proposal alone does not select clearing on every locally caught throw.

<a id="h-547"></a>
**H-547 — Delegate the local-recovery boundary by implementation cost.** Same human/chat/date, answering whether a reporting failure caught entirely inside finally preserves a pending return 7: “以实现成本考量最简单的选项吧”. The human delegates the choice/criterion rather than explicitly naming either outcome. K-RETURNING-20261116 records the agent's selection and rationale; broader new mechanisms and pending ownership/pattern scopes are not blanket-approved.

<a id="k-returning-20261116"></a>
**K-RETURNING-20261116 — Select scoped cancellation and align return inspection.** Under H-547's delegation, choose preserving return 7 after successful local recovery, and clearing when an escaping ordinary exception cancels the return. Reuse scoped completion suspension/restoration; every-throw invalidation would need extra propagation through suspended outer states. Current evidence: `f8eccfdf4e9f0852f9ef2e9e441e990544bdea11:cod/lang_llvm.hh`, complete_cleanup/emit_finally; this is feasibility evidence, not implementation acceptance. D-17r82→83/D-23r16→17 records H-542–H-547 and D-17-RETURN-STATE/P-482–P-484; D-15r41→42/D-10r189→190/D-21r21→22/source387→388 aligns dependencies and support boundaries, registry785→786. Candidate C-54/C-63/C-65 paths serve author-controlled completion, observable return state and valid storage under charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve H-525/H-536–H-541, unrelated FIFO failures, owner/access/capture constraints, exact release and D-16. Acceptance includes ordinary cancellation versus local recovery, explicit body-error replacement, first/repeated/failed return preparation, typed/owner fallback, null evidence and binding failure. Owned-result capture, no-value pattern details, complete grammar/physical protocols and full value-path qualification remain pending. Baseline: the same immutable commit at affected documentation paths. Documentation only; no source edits, builds or tests.

## 2026-11-16 — Copied nullable reference evidence

<a id="k-761"></a>
**K-761 — Preserve selected values across reference copies.** Source387→388/registry785→786 continues K-760's qualified D-14r116/P-258, D-17r82/P-235/P-236/P-280, D-18r107 and D-26r21/H-407 scope under H-420/H-423. Shared conditions retain copy observations and guarded predecessor relations independently of mutable slot versions, including saved guards and borrowed read predecessors. Acceptance covers independent choices, weak/loop/alias rebinding, payload mutation, shared native code and exact release. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owners/access/init and D-16 continue. Wider caller-slot/multi-origin proofs and new runtime/ABI/return-inspection/queue contracts remain separate; no new source rule or human confirmation. Baseline: `f8eccfdf4e9f0852f9ef2e9e441e990544bdea11`, affected compiler/source-reference paths.

## 2026-11-16 — Nullable input predicate evidence

<a id="k-760"></a>
**K-760 — Preserve absence and correlated handler paths.** Source386→387/registry784→785 continues K-759's qualified D-14r116/P-258, D-17r82/P-235/P-236/P-280, D-18r107 and D-26r21/H-407 scope under H-420/H-423. Shared conditions carry immutable input absence and versioned null selections; lexical/catch joins retain alternatives and impossible predicate paths remain empty. Acceptance covers presence/category models, changed selections/payloads, recursive/callable/prepared calls, failed work, native sharing and exact release. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owner/access/init contracts and D-16 continue. Wider caller-slot/local-alias/multi-origin proofs and new runtime/ABI/return-history/queue contracts remain separate; no new source rule or human confirmation. Baseline: `31761dc9c8eb9108a066b5cfc8ba3cf7b71133e8`, affected compiler/source-reference paths.

## 2026-11-16 — Exception-derived predicate evidence

<a id="k-759"></a>
**K-759 — Share Boolean/category facts through results and cleanup.** Source385→386/registry783→784 continues K-758's qualified D-14r116/P-258, D-17r82/P-235/P-236/P-280, D-18r107 and D-26r21/H-407 scope under H-420/H-423. Shared snapshots, guarded joins and call projection/substitution retain exception-derived Boolean evidence; resumed finally paths compose prior initialization and successful nullable-target writes retain the saved destination. Recursive value seeds preserve base evidence while empty observed summaries remain conservative. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owner/access/ref contracts and D-16 continue. Acceptance covers independent category/Boolean models, mutation/alias/failure controls, recursive native sharing and exact release. Wider input-null correlations and queue/aggregate contracts remain separately scoped; no new runtime, ABI, source rule or human confirmation. Baseline: `373156efdb5bace9a4d5dcec8b788ea8d2cf7090`, affected compiler/source-reference paths.

## 2026-11-16 — Consistent initialization conditions

<a id="k-758"></a>
**K-758 — Exclude inconsistent immutable path facts.** Source384→385/registry782→783 continues the K-757 qualified D-14r116/P-258, D-17r82/P-235/P-236/P-280, D-18r107 and D-26r21/H-407 scope under H-420/H-423. Shared conditions reject conflicting truths and intersect category filters on one immutable origin; branch joins promote the absence of missing-write paths to initialization. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525, owner/access/ref contracts and D-16 remain unchanged. Acceptance covers independent truth/category models, snapshot/mutation/alias negatives, failure prefixes, recursive native sharing and exact cleanup. Wider correlations and revised queue/aggregate policies retain their separate qualification; no new source rule, runtime representation or human confirmation. Baseline: `4b015ca1bb570dbe152c4cfa0d2da33f1c30d99d`, affected compiler/source-reference paths.

## 2026-11-16 — Conditional initialization and deferred FIFO integration

<a id="k-defer-fifo-sync-20261116"></a>
**K-DEFER-FIFO-SYNC-20261116 — Preserve compiler evidence with FIFO cleanup decisions.** Source383 from both parents continues as source384; registry780/781 continues as registry782. Integrates compiler `3834a41a3b1fea518a100a5dd3f7394cd563b47e` with design `3191b1d42254fccc95ae889e838a6cc482fe758d`, preserving both histories. K-757's shared conditional-initialization relation remains within the qualified D-14r116, D-17 ordinary completion, D-18r107 and D-26r21 contracts. D-17r82/D-15r41/D-10r189/D-21r21 retain H-539–H-541's separately selected return/FIFO decisions under candidate P-461–P-463. Charter §§2/3.1–3.3/4.3/6.3/8/12, H-525/H-536–H-538, owner/access/initialization and D-16 continue. Existing compiler validation establishes no queue/aggregate implementation; caller/nested checked coverage, return/body-error precedence, new registrations, physical support and full value-path qualification remain separate. Acceptance preserves exact compiler/test inputs and both parents' historical records, checks documentation links/anchors/revisions, and keeps selected semantics distinct from executable evidence. No new source rule, mechanism selection or human confirmation follows.

## 2026-11-16 — Conditional initialization across completion boundaries

<a id="k-757"></a>
**K-757 — Share conditional initialization through cleanup and calls.** Source382→383/registry779→780 continues D-14r116/C-65/P-258, D-17r80/C-63/P-235/P-236 and C-72/P-280, D-18r107 and D-26r21/H-407 under H-420/H-423. One finite relation retains paths lacking writes and conditional predecessors for each read across lexical flow, cleanup and callable summaries. Sequential composition, alternative joins and actual-argument substitution preserve write-before-read order and immutable input timing. Charter §§2/3.1–3.3/4.3/6.3/12, H-525 declared obligations, ref entry promises, actual owner/access rules and D-16 continue. Acceptance covers source/negative paths, input-snapshot models, aliases, generic/recursive/callable/prepared calls, failure prefixes, shared native bodies and exact cleanup. Unsupported local/slot correlations remain conservative; new roots/ABI and revised deferred-stack behavior retain their separate qualification. No runtime carrier, new source rule or human confirmation. Baseline: `912d2d7a5cc669987a66194386d1cb6b216c17ba`, affected compiler and source-reference paths.
## 2026-11-16 — FIFO pending deferred failures

<a id="h-541"></a>
**H-541 — FIFO pending errors with outgoing tail insertion.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. After requesting FIFO reevaluation, the user accepted the presented model: “这样更好，就这样决定吧。更新提交推送，下一议题”. Confirms reverse-registration deferred actions with FIFO pending errors, incoming-only handler visits, every escaping handler/finally failure appended at the tail, continued handling after return, and FIFO residual collection. The accepted examples are B → A → catch FlushError → catch CloseError, stable all-matching forwarding, and selective [A, B, C] → [B, A′, C′]. Replaces only the pending-error FILO order of H-506/H-540 and settles outgoing-error placement; preserves list-position dispatch, once-only finally, existing exception identity/trace, H-538 body_exception and H-539 return-result replacement. Earlier order/choice evidence: `f70397944d4a3845edf2a7ab4ab67ec8dd1e4d5b:CHANGELOG.md#h-540` and `design/ExceptionControl.md` at that commit, “Exceptions between deferred actions” / “Remaining deferred-stack decisions”. Does not decide caller/nested aggregate coverage, return/body-error precedence, new registration or physical storage.

<a id="k-defer-fifo-20261116"></a>
**K-DEFER-FIFO-20261116 — Align cleanup contracts with FIFO propagation.** D-17r81→82/D-15r40→41 integrates H-541 under D-17-DEFER-DISPATCH/P-461–P-463 and module P-458–P-460; D-10r188→189/D-21r20→21/source382→383 aligns dependent terminology and evidence boundaries, registry780→781. Candidate C-36/C-44/C-59/C-63/C-65 paths serve charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit propagation order, stable complete forwarding, context/identity preservation and valid resources. Acceptance covers the confirmed traces, selective/nonmatching entries, replacement/construction failures, repeated identity occurrences, return continuation, once-only finally, module cleanup and FIFO aggregation. H-525/H-536–H-540, qualified owner/initialization/native contracts and D-16 retain their unaffected scope. Remaining queue/aggregate/return/new-registration and allocation-free physical mechanisms stay separately bounded; neither existing compiler evidence nor this local decision qualifies the complete mechanism/value path. Baseline: `f70397944d4a3845edf2a7ab4ab67ec8dd1e4d5b`, affected paths. Documentation only; no source edits, builds or tests.

## 2026-11-16 — Retained return results with queued deferred handling

<a id="h-539"></a>
**H-539 — Later returns replace retained results while deferred work continues.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Reply: “不是继续扫描吧，而是继续处理后续 defer，又抛错，又处理，这个心智模型才对吧？ `return` 可以替代之前运行的 `return` 结果。” Confirms retained return-result replacement and continued deferred work. The wording prompted a separate timing clarification; this event alone does not select immediate exception dispatch or clearing of an ordinary body error.

<a id="h-540"></a>
**H-540 — Keep the queued B/A/Close/Flush order.** Same human/chat/date; answer to the two explicit execution traces: “保持之前的处理方式，B → A → catch CloseError → catch FlushError”. Reaffirms the earlier stack model for the example whose catch executes return on both invocations. Both ordinary deferred actions run before the handler's list position, and the second queued handler invocation still runs after the first return. Rejects the proposed immediate B/catch-Flush/A/catch-Close interpretation. H-507's once-per-entry finally and remaining deferred execution continue; return/body-error precedence remains separate.

<a id="k-defer-return-order-20261116"></a>
**K-DEFER-RETURN-ORDER-20261116 — Retain queued order through result replacement.** D-17r80→81/registry779→780 integrates H-539/H-540 under D-17-DEFER-DISPATCH/P-461–P-463 and C-63/P-323/P-324. Charter §§2/3.3/4.3/6.3/12 and existing candidate C-36/C-44/C-63/C-65 paths preserve explicit control, observable handler order and exact result ownership. Acceptance covers the B/A/Close/Flush trace, distinct later return values, a following invocation without return, continued deferred entries, once-only finally, preserved nonmatching errors and failed return preparation. Do not restart retired handlers or erase unrelated queued failures; body-error precedence, outgoing-error placement, caller/nested coverage and physical implementation remain separate. H-525/H-536–H-538 and D-16 continue; no full-stack qualification or source support is inferred. Baseline: `912d2d7a5cc669987a66194386d1cb6b216c17ba`, affected paths; source382 unchanged. Integrated K-756/main without changing its compiler evidence or either history. Documentation only; no source edits, builds or tests.

## 2026-11-16 — Integrate retained body-failure design with conditional-read checking

<a id="k-body-exception-sync-20261116"></a>
**K-BODY-EXCEPTION-SYNC-20261116 — Preserve compiler proofs and both failure channels.** Source381→382/registry778 branches→779 integrates K-756 with H-538 in D-17r80/D-15r40. Qualified D-14/C-65/P-258/P-280, ordinary D-17/C-63/P-235/P-236, D-18 and D-26 checking continues; candidate P-458–P-463 retains its separate stack scope. Charter §§2/3.1–3.3/4.3/6.3/12 supports explicit initialization, execution order, inspectable failure identity and live owners. H-538's readonly body_exception retains the still-pending body failure independently of residual deferred members, without automatic causes. Preserve all four completion combinations and aborted-startup behavior. Current compiler validation establishes conditional reads and unaffected completion, not the new stack/aggregate runtime. Caller/nested coverage, explicit returns, outgoing-error order, new registrations and physical ownership/absence encoding remain separately scoped. H-525/H-536/H-537, concrete unchecked behavior and D-16 continue. No new human confirmation or source implementation from the merge. Parents: `47829462752dad9faa42b31f1fa16d888b4ee028` (compiler) and `bfe12d4a52bbf0273b97d9531e7910267d336995` (design).

## 2026-11-16 — Preserve the ordinary body exception beside deferred failures

<a id="h-538"></a>
**H-538 — Retain a separate readonly body_exception without automatic causes.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “同意，保留独立只读的 `body_exception` 字段，不自动建立 cause 关系。落文档，然后继续下一个议题。” Accepts the presented combination: UnhandledDeferError retains residual deferred failures in exceptions and the still-pending ordinary body exception separately; if deferred failures are all handled, propagate the original body exception unchanged. No implicit cause relationship or body-error handling by defer catch is selected. Physical absence/owner representation and explicit-return interactions are separate.

<a id="k-defer-body-exception-20261116"></a>
**K-DEFER-BODY-EXCEPTION-20261116 — Preserve both failure channels at the aggregate boundary.** D-17r79→80/D-15r39→40/registry777→778 integrates H-538 under D-17-DEFER-DISPATCH/P-461–P-463 and D-15-SHUTDOWN/P-458–P-460. Candidate C-36/C-59/C-63/C-65 paths preserve charter §§2/3.1–3.3/4.3/6.3/12 through inspectable failure provenance, unchanged exception identity and valid retained owners. Acceptance covers all four body/deferred-presence combinations, readonly field association, retained trace/wrapper/explicit causes, no automatic cause or member dispatch, and aborted startup with cleanup failures. Keep absence encoding, backing/ABI, caller/nested coverage, outgoing-error order and explicit-return/new-registration policies separately scoped; no whole-stack qualification or runtime support follows. H-525/H-536/H-537, ordinary completion and D-16 remain intact. Baseline: `ff93fabf77a282a2563e6b7983ad0f7ef7f369c2`, affected paths; source380 remains the implementation reference. Documentation only; no source edits, builds or tests.

## 2026-11-16 — Conditional initialization in deferred actions

<a id="k-756"></a>
**K-756 — Resolve conditional reads through shared cleanup evidence.** Source380→381/registry777→778 continues D-14r116/C-65/P-258/P-280, D-17r79/C-63/P-235/P-236, D-18r107 and D-26r21/H-407 under H-420/H-423. Finite necessary conditions retain the paths on which a deferred local read lacks initialization; the existing cleanup equations substitute boolean values, exception categories and selected targets. Handler/branch snapshots, call/registration failure prefixes and unknown provisional summaries preserve conservative checking. Charter §§3.2/3.3/6.3/12, actual owner/access/initialization, H-525 declared obligations, concrete unchecked contracts and D-16 continue. Acceptance covers positive/negative source paths, independent snapshot models, nested/prepared/loop cleanup and shared recursive native bodies. Wider correlations, cross-action declared coverage, new roots/ABI and revised deferred-stack policies remain separate. No runtime guard, representation change or new human confirmation. Baseline: `ff93fabf77a282a2563e6b7983ad0f7ef7f369c2`, affected compiler/source-reference paths.

## 2026-11-16 — Integrate declared-defer design with compiler enforcement

<a id="k-declared-defer-sync-20261116"></a>
**K-DECLARED-DEFER-SYNC-20261116 — Preserve source obligations and post-handler aggregation boundaries.** D-17r78 branches→79/registry777/source380 integrates K-755's qualified selected-payload and declared-root enforcement with H-536/H-537's scoped deferred-failure decisions. C-63/P-235/P-236/P-280 and candidate P-461–P-463 preserve explicit typed responsibility, actual handler ordering and ordinary propagation under charter §§2/3.1/3.3/4.3/6.3/8/12. Covering deferred handlers may discharge checked obligations; unhandled checked failures may be declared. Residual aggregation follows the completed handler pass. The current compiler's cross-action root-coverage proof and new aggregate runtime remain unimplemented; caller coverage, nested/original-error composition, other stack policies and complete value-path qualification retain their recorded boundaries. Existing owner/access/initialization, concrete unchecked behavior, H-532–H-535 and D-16 continue. No new human confirmation or runtime implementation. Parents: `43cbb1b8975e6ccb0f9fa9849603693757bd30dd` (compiler/source reference) and `14a1daedd93f410cbc5229f1cdc638281863bdeb` (D-17/registry/changelog).

## 2026-11-16 — Declared checked failures from deferred work

<a id="h-536"></a>
**H-536 — Keep throws propagation available for checked deferred failures.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “不 defer catch 时就声明 `throws FlushException`，defer catch FlushException 时就不需要声明，我没看出有问题呀”. Confirms declaring the deferred operation's unhandled checked failure on the enclosing callable and discharging it with covering deferred handling. Rejects the agent's proposed mandatory local recovery/translation and its claim that a throws declaration cannot suffice. This does not itself choose how an outer typed catch receives a failure retained inside UnhandledDeferError, nor change H-509's preservation requirement.

<a id="h-537"></a>
**H-537 — Construct the aggregate after the defer-handler pass.** Same human/chat/date as H-536; clarification: “只有走过一遍 defer handlers 才会生成 `UnhandledDeferError` 包装，不存在把 `UnhandledDeferError` 交给 defer catch 去处理的场景；要有也是一个callable 抛出 `UnhandledDeferError`，普通 try catch 捕获”. Confirms the handler-before-packaging boundary and ordinary callable propagation/catching of the completed aggregate. It is not fed back into the completed defer-handler pass. No implicit member-wise ordinary catch dispatch follows. The composition of a deferred invocation with a callee that independently aggregates, and static coverage of the declared member obligation by a container catch, remain review boundaries rather than reasons to reopen H-536.

<a id="k-defer-throws-20261116"></a>
**K-DEFER-THROWS-20261116 — Separate source obligation from aggregate delivery.** D-17r77→78/registry775→776 records H-536/H-537 in D-17-DEFER-DISPATCH/P-461–P-463. C-63's explicit typed-failure responsibility under charter §§2/3.1/3.3/6.3/12 continues through ordinary throws declarations and actual covering handler registrations; root declarations retain H-525 obligations and recovery failures retain their own contracts. Acceptance includes declared unhandled cleanup, handled cleanup without that declaration, missing coverage/declaration and skipped/misordered handlers. Place residual aggregate creation after the completed handler pass and route it through ordinary catch. Preserve container matching without implicitly dispatching its members; keep caller-side checked coverage and nested deferred-invocation composition explicit review boundaries. Preserve all residual exceptions, body/deferred separation, ordinary owner/return rules and D-16. Full stack/value-path qualification, nested/original-error handling and physical support remain separate. Baseline: `33fd5d208772126df9aa6a0e2030ed9924ad8301`, affected paths. Documentation only; no implementation, builds or tests.

## 2026-11-16 — Selected payload proofs and declared failure obligations

<a id="k-755"></a>
**K-755 — Preserve declared contracts while composing selected values.** Source378→379/D-17r77→78/registry775→776 continues D-14r116/C-65/P-258, D-17r78/C-63/P-235/P-236/P-280, D-18r107 and D-26r21/H-407 under H-420/H-423 and H-525/K-CHECKED-DECLARATION-BOUNDARY-20261116. Shared finite selection conditions retain boolean/exception read/write alternatives through cleanup; complete boolean summaries distinguish impossible outcomes. Declared root obligations remain independent of actual categories and require explicit coverage, with coordinated fixture migration. Charter §§3.2/3.3/4.3/6.3/8/12, owner/access/initialization, concrete unchecked contracts and D-16 continue. Acceptance includes finite write/rebind models, root aliases/substitutions, selected-category initialization, recursive/native sharing and surrounding completion checks. Wider alias proofs, narrower-catch coverage, new roots/ABI and revised deferred-stack policy remain separate. No runtime carrier or new human confirmation. Baseline: `33fd5d208772126df9aa6a0e2030ed9924ad8301`, affected source and reference paths.

## 2026-11-16 — Preserve declared exception obligations during proof improvements

<a id="k-checked-declaration-boundary-20261116"></a>
**K-CHECKED-DECLARATION-BOUNDARY-20261116 — Bound enforcement without qualifying the pending root package.** Agent continuity review after the developer identified a potential H-525 regression: D-17r76→77/registry774→775 clarifies the existing C-63/P-235/P-236/P-280 checking path, confirmed by H-267/H-313 and constrained by H-524/H-525. Explicit typed intent and native equivalence under charter §§2/3.1/3.3/6.3/12 require that actual-value proofs not erase declared checked obligations. Aliases and admitted generic substitutions use resolved declared identity; they do not infer a narrower annotation from payloads. Acceptance covers root-typed initializer/catch/borrow/helper cases, explicit throws obligations, ordinary covering root handlers and unchanged concrete unchecked propagation. This records no new human confirmation and does not qualify the pending whole Error-root/value path, ABI, transparent throws, narrower-category coverage of broad obligations or new deferred-stack suppression/aggregation. Unrelated qualified proofs, ordinary completion, owner rules and D-16 continue. Baseline: `de639db7e569dafb61858e1da15de04f10b336c6`, affected paths; source378 remains the implementation reference. Documentation only; no source edits, builds or tests.

## 2026-11-16 — Integrate endpoint constraints and boolean output evidence

<a id="k-disjoint-endpoint-sync-20261116"></a>
**K-DISJOINT-ENDPOINT-SYNC-20261116 — Preserve confirmed endpoint rules and compiler proof scope.** Agent continuity/integration under H-532–H-535/H-420/H-423 and charter §§3.2/3.3/4.3/6.3/8/12: source377→378/registry773→774 preserves K-754 and both histories with D-07r164/D-10r188/D-15r39/D-17r76/D-21r20. Listed-endpoint ancestry, per-pair authoring grants and the independent-parent versus already-established relation distinction retain their selected scope. Ordinary declaration evidence must preserve that distinction; no separate exemption catalogue or exception-only rule is introduced. Shared world-pool representation, finite generic/cyclic checking and full physical qualification remain separate. Current compiler validation establishes boolean result/output, owner/initialization and unaffected ordinary-completion behavior; it does not establish disjointness or revised exception/deferred-stack support. D-16 and unrelated qualified contracts continue. Baselines: `2b1eaa39a7e3305359bfde9a329dd2adb7cf326a` (compiler) and `84e92fae8b553208eb662657e4a16347cc5bea28` (design), affected paths. Documentation merge only; no new human confirmation.

## 2026-11-16 — Boolean output evidence through calls and cleanup

<a id="k-754"></a>
**K-754 — Continue finite boolean payload and completion proofs.** Source376→377/registry772→773 under D-14r116/C-65/P-258, D-17r75’s unaffected checked-NPE/explicit-return/completion scope, D-18r107 and D-26r21/H-407; H-420/H-423. Branch writes retain predicate facts; weak aliases and outcome summaries preserve every possible target value. Canonical finite alternatives keep preserve-or-write relations until incoming payload substitution across ordinary/prepared calls, transfer/swaps, repeated/nested cleanup and pending/replaced returns. No recursive predicate expansion or runtime carrier; exported call facts use input ordinals. Charter §§3.2/3.3/4.3/6.3/8/12, D-16 and owner/access/initialization/declaration constraints continue. Acceptance includes direct/prepared truth tables, aliases, output/result timing, recursive/captured calls, failures and native sharing. Dynamic binding-selection/payload correlations remain unproved; revised exception/deferred-stack/disjoint contracts gain no implementation qualification. Baseline: `3bcb2998085a3e47010fdc4be88a9bd55da8d9f9`, `cod/lang_checked.hh`, `cod/lang_completion.hh` and source reference.

## 2026-11-16 — Endpoint ancestry, independent parents and shared constraint information

<a id="h-533"></a>
**H-533 — Evaluate shared disjoint information per module world.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “type disjoint 信息，可以在每个模块世界中维护一个共享副本，该模块 def/imp 的类型都引用这个共享副本， using 的类型还是引用它原本所在模块世界的副本，这样是不是实现上简单有效？ 有没有潜在问题？” Requests feasibility review of this sharing/association model; physical representation and its complete value path remain pending. D-15 records upstream retention, combined local/source checks, publication filtering and frozen backing as necessary boundaries, not additional human approvals.

<a id="h-534"></a>
**H-534 — Exemptions are existing ancestry among listed types, without a separate path store.** Same human/chat/date; statement: “豁免路径就是列表包括的类型它们之间之前就存在，今后也不可能再被增加（被disjoint规则拒绝）的继承关系，应该没有额外处理、存储的需求才对吧？” Narrows H-527 to relations among the listed endpoints, already available in the type graph. Supersedes the broader agent draft's arbitrary old-path/common-conformer grandfathering and separate snapshot catalogue, recorded at `3368e76e0b953738f8cd97d235dbf6d4a0dbeb8e:design/SymbolicDynamicScope.md`, section Disjoint declarations and exempt inheritance paths. Ordinary declaration/origin evidence remains necessary; no additional exemption-path storage is required.

<a id="h-535"></a>
**H-535 — Reject a subtype's second independent listed parent.** Same human/chat/date; answer to B <: A; disjoint A/B; X <: A followed by X <: B: “这就是我们为了 Error <: Exception 问题想的解决方案嘛，它拒绝了才能同时解决原始问题呀。 心智模型也简单： disjoint 列表里的类型，就是不能被一个子类同时声明为父类”. Confirms rejection by the general disjoint mechanism, directly serving H-526. H-530's already-established redundant relationship remains acceptance with warning: X <: B followed by X <: A is different because A was already reachable. No exception-only special case, retroactive deletion of an earlier independent relation or separate exemption-path store is selected.

<a id="k-disjoint-endpoints-20261116"></a>
**K-DISJOINT-ENDPOINTS-20261116 — Integrate the narrower graph rule and bounded sharing review.** Alongside H-532 in D-07r164/D-10r188/D-15r39/D-17r76/D-21r20/source377/registry773, H-534/H-535 correct D-10-DISJOINT/P-476–P-478; H-533 adds pending D-15-DISJOINT-POOL/P-479–P-481. Candidate C-56/C-59/C-10 chains preserve charter §§2/3.1–3.3/4.2–4.3/6.3/8/12 through explicit conformance intent, source-world identity and shared checked metadata. Reuse ordinary ancestry/admission evidence; preserve prior-relation warnings, intermediate method contracts, complete public records, per-pair grants and frozen validation. Main-developer review supports pool feasibility while requiring upstream retention, applicable local/source checking and selective publication; it is agent evidence, not human qualification. Acceptance includes the X/Y order distinction, Error/Exception, unrelated old common conformers, aliases, local constraints on using types and n-ary publication. Finite generic/cyclic proofs and physical representation remain separate. Preserve H-525, demanded checking, owners/captures, D-16 and K-753; invalidate only the broad exemption-history draft. Baseline: `3bcb2998085a3e47010fdc4be88a9bd55da8d9f9`, affected paths. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Per-pair authority for disjoint declarations

<a id="h-532"></a>
**H-532 — Require one locally defined or imported endpoint per pair.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer: “Own/import at least one endpoint per pair (Recommended)”. Confirms that authoring a new disjoint constraint requires local definition or explicit import with extension permission for one endpoint of every pair; merely using both foreign types rejects. In an n-ary list, a local endpoint cannot authorize another pair between two foreign types. Consumers still receive existing public constraints through using/import without a new authoring grant. “Own” here concerns defining the type, not a runtime storage owner.

<a id="k-disjoint-authority-20261116"></a>
**K-DISJOINT-AUTHORITY-20261116 — Bound new constraint authorship without restricting consumption.** D-07r163→164/D-10r187→188/D-15r38→39/D-17r75→76/D-21r19→20/source376→377/registry772→773 integrates H-532 in D-10-DISJOINT/P-476–P-478. Existing candidate C-56/C-10 paths under charter §§2/3.1–3.3/4.2–4.3/6.3/8/12 preserve explicit definition/extension responsibility and published semantic evidence. Derive the equivalent at-most-one-unprivileged-endpoint test, resolved-identity grants, no alias/n-ary permission laundering, unchanged-record consumption/re-export and unauthorized-pair diagnostics without automatic selection rewrites. H-527–H-531 frozen paths/warnings, H-525/H-526, demanded method checking, owner/capture and D-16 continue. Full value-path qualification, cyclic proof establishment, formal graph/generic validation and physical implementation remain separate. Baseline: `3bcb2998085a3e47010fdc4be88a9bd55da8d9f9`, affected paths. Integrated K-753/main while preserving both histories and its unchanged compiler evidence. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Integrate disjoint-path design and boolean input proofs

<a id="k-disjoint-path-sync-20261116"></a>
**K-DISJOINT-PATH-SYNC-20261116 — Preserve scoped design and compiler evidence.** Source376/registry772 integrates `3368e76e0b953738f8cd97d235dbf6d4a0dbeb8e` (D-07r163/D-10r187/D-15r38/D-17r75/D-21r19) with compiler `df1c4b71333a7a2411be9c83dc50199d37335820`/K-753. Preserve both parent decision histories and the qualified boolean-call/owner/initialization/completion contracts under charter §§3.1–3.3/4.2–4.3/6.3/8/12 and D-16. Disjoint records retain source snapshots, prior-route witnesses and frozen-world validation; no generic path algorithm or implementation is qualified. A design-chat message received 2026-11-16 reports H-532's per-pair authoring rule: own or explicitly import at least one endpoint of every pair; consumption of an existing record needs no new grant. This is an attributed agent receipt pending that checkpoint's evidence integration, not another human confirmation or a reopened permission choice. Multiple abstract parents remain deferred; mixed-bridge review examples cannot broaden that scope. Mainline compiler and native evidence are unchanged by this documentation merge.

## 2026-11-16 — Preserve boolean input evidence across call results

<a id="k-753"></a>
**K-753 — Continue finite boolean call evidence.** Source374→375/registry770→771 under D-14r116/C-65/P-258, D-17r74’s unaffected checked-NPE/explicit-return/completion scope, D-18r107 and D-26r21/H-407; H-420/H-423. Necessary truths of immutable boolean inputs share saved-result composition with target presence. Instantiate copied preparation snapshots or borrowed entry payloads; retain versioned target facts and conservative alias/write invalidation. Borrowed results read surviving target predicates after cleanup. Charter §§3.2/3.3/4.3/6.3/8/12, D-16 and unchanged owner/access/initialization/declaration constraints continue. Acceptance covers finite truth tables, ordered operand effects, recursion/captures, cleanup and native body sharing. No new source rule, runtime carrier, boolean output-write summary or exception-root/wrapper/deferred-stack qualification. Baseline: `5c9f5620af372228ffb08677c0721995343f13b0`, `cod/lang_checked.hh`, `cod/lang_completion.hh` and source reference.

## 2026-11-16 — N-ary disjoint constraints, exempt paths and method-aware warnings

<a id="h-527"></a>
**H-527 — Explore n-ary disjoint declarations with public, type-carried exemptions.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “我觉得可以朝 `type disjoint A, B, C;` 甚至同时声明更多的语法方向推演，加一个规则是声明这条之前，同模块已知的继承链路确认为豁免链路 —— 子类型经由这些豁免链路产生的继承关系不被拒绝；同时 disjoint 关系和豁免链路随任何一个类型的 export 声明认为是public，且跟随任何类型被其它模块 using/import 。 你继续推演看看”. Selects the n-ary direction, declaration-point known-path exemptions and public carriage; replaces the unqualified empty-intersection proposal. Full path validation, foreign declaration authority and metadata protocol remain under review.

<a id="h-528"></a>
**H-528 — Inheritance is frozen in the fused world.** Same human/chat/date; correction: “类型继承关系是在 fused world 冻结了的哈，不要假定允许运行时变更”. Confirms static validation/preparation; later-source/import examples concern fusion, not runtime mutation. Exemption snapshots retain their declaration provenance; no runtime graph scheduler or change protocol is selected.

<a id="h-529"></a>
**H-529 — Reject new independent paths.** Same human/chat/date; asynchronous selection: “Reject new independent paths (Recommended)” for a post-disjoint direct MyError <: Exception path in addition to MyError <: Error through an exempt Error → Exception path. Selects path-preserving enforcement; H-530 immediately refines this example's already-established redundant relationship to acceptance with warning. The genuinely new independent-route rejection remains current.

<a id="h-530"></a>
**H-530 — Accept redundant shorter declarations with an existing-path warning.** Same human/chat/date; statement: “对于已经间接形成了父子关系的，再次声明更短路径，不拒绝但是要给出警告诊断，提醒用户已经通过某某路径具备继承关系了”. Supersedes H-529's rejection for the already-established indirect relationship only. Warn with the witness; new independent relationships remain constrained. The draft derives noncircular prior evidence, no new capability/exemption and no retroactive deletion of older paths.

<a id="h-531"></a>
**H-531 — Explain intermediate method contracts in the redundant-path warning.** Same human/chat/date; clarification: “不仅是 redundant declaration 隐患，要防止的是用户心智里确实中间父类的相关方法实现，这一条也许表明用户以为只需要实现高层父类的方法就够了”. The warning should address possible omission of intermediate abstractions in the author's mental model, not merely redundant syntax. Show the existing path and explain that intermediate method requirements/selection remain; do not assert guessed intent or newly require eager completeness. Existing H-216/H-218 demand-based method checking remains intact.

<a id="k-disjoint-paths-20261116"></a>
**K-DISJOINT-PATHS-20261116 — Integrate frozen constraints, publication and diagnostic purpose.** D-07r162→163/D-10r186→187/D-15r37→38/D-17r74→75/D-21r18→19/source374→375/registry770→771 records H-527–H-531 in D-10-DISJOINT/P-476–P-478. Candidate C-56/C-10 paths preserve charter §§2/3.1–3.3/4.2–4.3/6.3/8/12 through explicit nominal intent, established graph relationships, stable publication and readable method obligations. Agent derivations: unordered pair constraints for n-ary lists; complete original-record carriage with semantic identities rather than implicit source aliases; no receiver resnapshot or weakened exemption union; redundant assertions need prior witnesses; final fusion rejects incompatible applicable records. Error/Exception can retain their exempt ancestor path without an empty-intersection claim or a hidden CheckedException root. Foreign authority, cyclic evidence and full generic/path proofs remain pending; H-525/H-526, demanded implementation checking, owner/capture and D-16 continue. Baseline: `5c9f5620af372228ffb08677c0721995343f13b0`, affected paths. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Unchecked Error root and explicit checked declaration contracts

<a id="h-524"></a>
**H-524 — Add Error <: Exception for user-defined unchecked failures.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “对了，为了让用户能自定义 unchecked exception ，是不是应该提供 unchecked `Error <: Exception` 类型？ 作为 root of unchecked exceptions。 `Exception` 本身保持 checked，如果抛了或者声明了 `throws Exception` ，就要求 catch 处理”. Selects the unchecked subtype root and explicit checked Exception contract for user-defined failures. Replaces the neutral-root interpretation recorded with H-473 without renaming the common root or changing existing concrete built-in categories. Physical hierarchy, payload/owner representation and general user exception construction are not thereby implemented or fully qualified.

<a id="h-525"></a>
**H-525 — Do not silently narrow an explicit Exception contract using value proofs.** Same human/chat/date; answer to accepting an Exception-typed throw proven to contain an Error: “能静态正面类型的地方，可以通过诊断通道提醒用户声明更具体的类型，达到你说的目的，已经写出来的代码该拒绝就拒绝，原则上这种猜用户意图的处理认为是耍小聪明，判为设计反例”. Rejects the proposed proof-based waiver. Diagnostics may suggest a more precise declaration; source acceptance must retain the written checked contract. Explicit throws Exception likewise remains a caller obligation. This does not revoke actual-type dispatch or unrelated qualified flow proofs; runtime matching and declared propagation obligations must be distinguished.

<a id="h-526"></a>
**H-526 — Reject adapter reclassification and request general incompatible-type declarations.** Same human/chat/date; answer to an adapter adding Error conformance to a checked Failure: “要reject。 最好引入一个通用的声明两（抽象）类型不兼容的机制和语法，比如 `type assert Error is not Exception`；这个例子有点反直觉，你想想更好的语法”. Confirms the checkedness-changing adapter rejection and requests design of a general mechanism/better syntax. It does not accept the agent candidate type disjoint A, B or erase Error <: Exception; true disjointness of that pair would contradict the selected relation. Permission, visibility and negative-evidence rules remain pending, with no hidden CheckedException root selected.

<a id="k-error-roots-20261116"></a>
**K-ERROR-ROOTS-20261116 — Record roots, explicit obligations and the separate disjointness proposal.** D-17r74/D-10r186/D-15r37/D-21r18/source374/registry770 integrates H-524–H-526 alongside K-RETHROW-FROM-20261116. D-17-ERROR-ROOTS/P-473–P-475 candidate C-56/C-63/C-65 paths under charter §§2/3.1/3.3/4.3/6.3/8/12 support declared domain failures and stable, visible checked obligations. Supersede only the earlier neutral-root/proof-based-waiver scope; preserve unrelated qualified checking. Keep runtime failure matching/wrapper category distinct from an explicit source contract, without a proof-based bypass; aliases and adapters cannot change established classification. Preserve existing nominal conformance outside this boundary, owners, trace/cause identity and D-16. Mainline K-752 and both prior histories remain intact. Developer review confirms identity selection coherence but requires end-to-end identity transport, non-reused live identities, retained backing and capture validity; record these physical obligations without selecting an ABI or exhaustion policy. D-10-DISJOINT/P-476–P-478 is pending-human: symmetric no-common-conformer syntax is a candidate, with foreign-type authority, visibility/negative evidence and conflicting worlds unresolved. Source hierarchy/payload support and remaining wrapper/throws/inspection policies remain separate. Baseline: `7e62b50a3fdb42e9585168234180f102c25343d4`, affected paths. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Preserve wrapper checkedness and suggest explicit automatic rethrow

<a id="h-523"></a>
**H-523 — Reject mismatched standard wrappers and suggest rethrow from e.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Reply: “reject，并且信息文本包含直接 `rethrow from e;` 的建议” to the proposed requirement that both standard wrappers have a cause, RethrownException wraps checked causes and RethrownError wraps unchecked causes. Confirms rejection of RethrownError from a caught NullPointerException and requires the diagnostic's explicit automatic-wrapper suggestion. The suggested form supplies an explicit cause and automatically selects the appropriate standard wrapper; ordinary business exception translation remains separate. Physical representation, inspection and transparent throws summaries are not thereby selected.

<a id="k-rethrow-from-20261116"></a>
**K-RETHROW-FROM-20261116 — Integrate cause-preserving standard wrappers.** D-17r73→74/D-15r36→37/D-10r185→186/D-21r17→18/source373→374/registry769→770 records H-523 under D-17-RETHROW/D-17-CATCH-WRAPPERS/P-464–P-466/P-470–P-472. Candidate C-36/C-63/C-65 paths preserve charter §§2/3.3/4.3/6.3/8/12 through inspectable provenance, unchanged handling category and actionable source diagnostics. Agent derivations: constrain all canonical construction forms/aliases; require static evidence for a fixed category, recommend automatic selection when unproved; explicit causes work outside catch; H-522 identity normalization and once-only preparation precede wrapper selection; construction captures one fresh trace. Valid standard-wrapper prefixes have one category, so explicit catches bind the outer wrapper without a deeper-category search. Do not emit an invalid statement replacement for an expression context. Inspection, transparent throws summaries, physical identity and deferred-stack residual questions remain separate; D-16 and unaffected qualifications continue. Baseline: `7e62b50a3fdb42e9585168234180f102c25343d4`, affected paths. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Integrate exception identity and boolean-result evidence

<a id="k-exception-identity-sync-20261116"></a>
**K-EXCEPTION-IDENTITY-SYNC-20261116 — Preserve scoped design and compiler evidence.** Agent continuity/integration under H-510–H-522/H-420/H-423 and charter §§4.3/6.3/12: source372→373/registry768→769 preserves K-752 and both histories with D-17r73/D-15r36/D-10r185/D-21r17. Exception construction/trace/cause rules, bounded OOM diagnostics, source-order wrapper matching and catch-local same-identity tip selection retain their recorded scope. Stable identity/backing, capture retention, inspection, canonical-wrapper/mixed-prefix/checked-summary choices and deferred-stack completion integration remain separate. Current compiler evidence establishes boolean-result/owner/initialization/ordinary-completion invariants, not these new exception mechanisms. D-16 and unrelated qualified scopes continue. Baselines: `af2dc438f820dd9afec1dbc3afa4a36f9054bc91` (compiler) and `aec8401e2635e1ad931e50c75e569f1c3c835f79` (design), at affected paths. Documentation merge only; no new runtime behavior or human confirmation.

## 2026-11-16 — Preserve non-null proofs through boolean helper results

<a id="k-752"></a>
**K-752 — Saved boolean result evidence.** Agent continuity under H-420/H-423, D-14r116/C-65/P-258, D-17r72/C-63/P-235/P-236, D-18r107’s qualified call/capture scope and D-26r21/H-407: source371→372/registry767→768 retains conditional incoming-target presence through explicit-return preservation/replacement and checked input ordinals. Literal, null-test, stored and call-result proofs share immutable target snapshots and binding versions; recursive/callable alternatives retain common guarantees. Charter §§4.3/6.3/12 supports composable checked intent without runtime predicate state. Independent initialization/access, owner validity, declared checked failures and D-16 remain unchanged. Borrowed boolean results, broader correlations and revised exception/stack policies remain separate. Baseline: `0443fe9fab607b6410ff72b8fcb18db42f68269a` at affected paths. No new source contract or human confirmation.
## 2026-11-16 — Transparent rethrow matching and catch-local exception identity

<a id="h-518"></a>
**H-518 — Match through standard rethrow wrappers unless explicitly catching the wrapper.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Statement: “嗯，`catch` 匹配的时候，应该是在cause链上找到第一个非 RethrownException/Error 的进行匹配，除非显式 `catch (e : RethrownException/Error)` 这样才对吧？” Selects transparency of the standard wrapper prefix, not arbitrary business causes or aggregate member search. Mixed-wrapper construction/matching and checked summary representation remain separate.

<a id="h-519"></a>
**H-519 — Preserve source order across ordinary and explicit wrapper catches.** Same human/chat/date; answer: “我感觉按source order处理上简单直接，所以倾向于这个。 你看如果另一种其实技术上更简单，跟我解释一下”. Selects the first matching clause in source order rather than an explicit-wrapper priority pass. Agent and developer review found no technical simplicity reason to override that preference; no new ABI is inferred.

<a id="h-520"></a>
**H-520 — Prefer the incoming wrapper for compatible exception operations.** Same human/chat/date; answer: “我倾向于连 `from e` 也隐式使用wrapper，能用wrapper的地方都用wrapper”. Extends preservation beyond bare propagation/implicit causes to explicit from e. It does not approve changing ordinary nominal/generic typing or a universal value envelope; H-521/H-522 bounds the association mechanism.

<a id="h-521"></a>
**H-521 — Retain a hidden incoming tip instead of copying the wrapper chain.** Same human/chat/date; statement: “应该不需要copy吧？ 多增加一个隐藏的 `tip` wrapper 引用， `from e` 等地方判断是当前 e ，就用当前tip就好了嘛”. Selects a hidden incoming reference alongside the matched exception and existing-chain reuse. H-522 resolves whether recognition means direct binding or same exception identity. Stable handle retention/capture is distinct from graph copying; physical representation remains separate.

<a id="h-522"></a>
**H-522 — Recognize the same exception identity within the current catch.** Same human/chat/date; asynchronous reply selected “Same exception identity within this catch (Recommended)” to `rethrow DomainError() from echo(e)`, with echo returning e unchanged. Confirms comparing the resulting exception identity with the current catch's matched exception and selecting tip on equality; the check is catch-local and imports no caller catch context into separately defined helpers. Rejects limiting the rule to a direct reference to the catch binding. Ordinary helper typing remains unchanged. Equal-field reconstruction, stable physical identities, nested/captured context derivations and inspection/bypass APIs are bounded in D-17; no general value ABI is selected.

<a id="k-wrapper-catch-20261116"></a>
**K-WRAPPER-CATCH-20261116 — Integrate transparent matching and same-identity tip selection.** D-17r73/D-15r36/D-10r185/D-21r17/source372/registry768 records H-518–H-522 in D-17-CATCH-WRAPPERS/P-470–P-472, alongside K-EXCEPTION-CONSTRUCTION-20261116. Candidate C-36/C-63/C-65 paths preserve charter §§2/3.2–3.3/4.3/6.3/8/12 through typed recovery, immutable provenance, observable order and valid owners. Agent derivations: once-only operands; construction-distinct identity retained by ordinary transport; nearest lexical catch governs nested same-leaf/different-tip cases; delayed/repeated handlers retain their own matched identity/tip without a mutable ambient cell. Developer review supports bounded lexical association and identifies stable identity, retained backing and correlated checked facts as implementation requirements; it does not qualify the broader identity rule as implemented. Inspection/bypass, canonical wrapper/mixed-prefix and checked-summary choices remain pending, as do deferred-stack residual policies. D-16 and unaffected contracts continue. Baseline: `0443fe9fab607b6410ff72b8fcb18db42f68269a` at affected paths; K-751 history/source evidence preserved. Documentation only; no source edits, builds or test scripts.

## 2026-11-16 — Explicit exception construction and bounded OOM diagnostics

<a id="h-510"></a>
**H-510 — Separate throw propagation from rethrow construction and capture traces at creation.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Reply: “或者就明确引入 rethrow 关键词，`throw` 不修改已创建的Exception，也不创建新的Exception，stack tracek 只跟着新建Exception的动作捕获； rethrow 相当于新建Exception捕获stacktrace”. Confirms the explicit operation distinction and creation-time trace capture; no automatic wrapping based on caught-value ancestry. Trace frame/format/storage and constructor failure policy are not independently selected.

<a id="h-511"></a>
**H-511 — Move the optional cause clause to rethrow.** Same human/chat/date; answer: “make it `rethrow e [from cause]`”. Moves cause-bearing creation out of plain existing-value throw. H-512/H-513 subsequently refines the operand to a named constructor and supplies the omitted-cause rule; this event does not preserve arbitrary existing-value rethrow as current syntax.

<a id="h-512"></a>
**H-512 — Use an explicit exception constructor in rethrow.** Same human/chat/date; correction: “哦，`rethrow SomeEx(...) from cause` 这样才对吧”. Selects an explicit new exception type/construction rather than rewriting an already created value. H-513 supplies optional-from semantics; H-514 separately adds bare rethrow.

<a id="h-513"></a>
**H-513 — Omitted rethrow cause uses the caught value.** Same human/chat/date; statement: “`rethrow SomeEx(...) [from cause]` ，省略 `from cause` 时使用当前捕获值”. Confirms that construction form and its default cause. Existing lexical caught-value/capture contracts supply nearest-scope selection and invalid missing-context diagnostics; no new ambient exception binding is selected.

<a id="h-514"></a>
**H-514 — Bare rethrow creates a wrapper.** Same human/chat/date; statement: “也允许仅仅 `rethrow;` 这时候包装捕获的异常作为cause，抛出新的包装异常对象”. Confirms a fresh wrapper with the caught value as cause, combined with H-510 creation-time trace capture and H-508's RethrownException/RethrownError checkedness distinction. Bare throw remains unchanged propagation. No physical allocation or arbitrary cause graph protocol follows.

<a id="h-515"></a>
**H-515 — Initialize readonly cause only through exception construction's from clause.** Same human/chat/date; statement: “Exception 的 `cause` 字段是不是最好禁止作为构造参数传？ 只能通过 `throw/rethrow/make` 一个 Exception ctor 时的 `from cause` 子句指定，从此只读”. Records the reserved construction-only cause direction, exclusion from ordinary constructor parameters and readonly lifetime. Constructor-form throw constructs its operand before unchanged propagation; this does not restore throw-from on an existing value. General exception payload/owner/trace representation is not approved by the syntax rule.

<a id="h-516"></a>
**H-516 — Statically reserve bounded process-level OOM information.** Same human/chat/date; proposal: “关于OOM，我建议进程层面预先静态分配一个 bounded OOM 信息区，专门用于处理OOM时使用。” Selects the bounded emergency-information direction, with H-517 supplying retention under capacity pressure. It does not select an unbounded cause/aggregate allocator, trace format or concurrency ABI.

<a id="h-517"></a>
**H-517 — Pin earliest OOM information and roll later records.** Same human/chat/date; asynchronous answer to reserve exhaustion: “最早的OOM信息保留不动，其它OOM信息当作滚动窗口，新记录挤出旧记录”. Selects a first-pinned/remaining-rolling retention policy instead of the offered termination or fixed trace-less fallback. The D-17 draft derives stale-record detection/coherent reads and separation from immutable semantic exception fields; no permission to overwrite live cause or aggregate values is inferred.

<a id="k-exception-construction-20261116"></a>
**K-EXCEPTION-CONSTRUCTION-20261116 — Integrate constructor/rethrow syntax and OOM record retention.** D-17r72→73/D-15r35→36/D-10r184→185/D-21r16→17/source371→372/registry767→768 records H-510–H-517 in D-17-RETHROW/D-17-OOM-RESERVE/P-464–P-469. Candidate C-36/C-63/C-65 paths preserve charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit creation/provenance, readonly cause, checked semantic identity and bounded failure diagnostics. Derive lexical cause defaults, fresh per-catch invocation values, no sibling-finally last error, ordinary copies/throws preserving trace, once-only construction/operand effects, missing-context and cause-parameter rejection, and safe rolling diagnostic expiry. Preserve H-509's full aggregate members and pending stack order/body-error/return/new-registration integration. Main developer feasibility review and local source inspection show current cause-vector allocation and early registration-record release are not evidence of the new lifetime/reserve protocol. Actual cause/aggregate ownership, trace/window details and new source support remain separate; D-16 and unaffected qualified contracts continue. Baseline: `0443fe9fab607b6410ff72b8fcb18db42f68269a` at affected paths. Integrated K-751 mainline presence evidence without changing its qualified scope. Documentation only; no source edits, build or tests.

## 2026-11-16 — Retain incoming-target presence across calls

<a id="k-751"></a>
**K-751 — Outcome-specific borrowed-target presence.** Agent continuity under H-420/H-423, D-14r116/C-65/P-258, D-17r72/C-63/P-235/P-236, D-18r107’s qualified call/capture scope and D-26r21/H-407: source370→371/registry766→767 preserves nullable reference selection, independent payload initialization, actual owners/access, prepared arguments and declared checked obligations. Immutable input presence composes through the existing dependency-directed call/completion summaries; unchanged reference snapshots survive source rebinding. Charter §§4.3/6.3/12 supports one checked source-to-native path without runtime proof state. Boolean-result correlations, broader alias relations, permanent representations and the revised deferred-stack integration remain separate. D-16 and unrelated qualified scopes continue. Baseline: `169901d66150dc7075160a0384fcda2cce92ad5b` at affected paths. No new source rule or human confirmation.

## 2026-11-16 — Integrate deferred-stack decisions with identity guards

<a id="k-defer-stack-sync-20261116"></a>
**K-DEFER-STACK-SYNC-20261116 — Preserve scoped design and compiler evidence.** Agent integration/continuity under H-503–H-509/H-439–H-441/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/12: source369→370/registry765→766 preserves K-750 and both histories while integrating D-17r72/D-15r35/D-10r184/D-21r16. List-position FILO error handling replaces the agent's immediate-look-ahead interpretation; ordinary body errors remain separate, attached finally runs once and residual errors require the selected aggregate. Wrapper scope/order, aggregate ownership/checkedness/original-error integration, explicit returns and cleanup-time registrations stay pending. Existing validation does not establish the revised stack contract. D-16 and unrelated qualified scopes continue. Baselines: `a4a4cb6b35e4b47ff40bd032dbde3d3787184b1e` (compiler) and `407634910ff0c073ea22c3fff3f3ca86943a10e6` (design) at affected paths. Documentation merge only; no new runtime behavior or human confirmation.

## 2026-11-16 — Preserve identity guard evidence

<a id="k-750"></a>
**K-750 — Nullness relationships from selected reference identity.** Agent continuity under H-439–H-441/H-420/H-423, D-28-IDENTITY/C-95/P-391–P-392, D-14r116 and D-17r71’s unaffected checked-NPE scope: source368→369/registry764→765 retains owner-plus-locator identity, ordered operand snapshots, stable binding versions and independent access/initialization obligations. Existing boolean evidence composes necessary nullness relationships and closes finite target refinements in either condition order. Charter §§4.3/6.3/8/12 supports checked source intent without runtime proof state. No exact locator/alias inference, new layout/key protocol, owner permission, D-16 change or revised handler acceptance follows. Baseline: `317f59fa9c7168397bd60f3cb1c36dd1e7195ff5` at affected paths.

## 2026-11-16 — Separate deferred errors and scan them at handler entries

<a id="h-503"></a>
**H-503 — Distinguish local deferred catches from handlers of other deferred actions.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Reply: “只有 `defer catch ...` 语法才catch其它defer的异常， `defer {} catch ...` 不理之前执行造成的异常，自己独立执行；模块在全局范围的 defer 倒序执行列表采用相同的规则。 这意味着 靠前的 `defer catch ...` 有可能被执行多次，后抛出的 defer 异常先传递到它先执行。” Confirms the syntax/coverage distinction, multiple catch executions and application to the global module list. The assistant initially interpreted this as immediate look-ahead dispatch; H-506/H-507 supplies the subsequent stack-scan model. The quoted answer alone does not confirm that agent-specific timing mechanism or a physical walker.

<a id="h-504"></a>
**H-504 — Initially omit finally from a bodyless deferred handler.** Same human/chat/date; asynchronous answer: “`defer catch ...` 不许有 `finally` 吧，按此推演”. This restriction was subsequently replaced by H-507's once-per-list-entry finally in the new execution model; it is not the current contract.

<a id="h-505"></a>
**H-505 — Deferred catches do not handle ordinary body errors.** Same human/chat/date; asynchronous answer: “`defer catch` 不应该处理非 defer 造成的异常”. Confirms exclusion of non-deferred exceptions, superseding H-360's original-scope-body recovery through deferred handlers. Ordinary try/catch remains available; cleanup failures are distinct from an original body/startup failure. No completion-erasure or process-success policy follows.

<a id="h-506"></a>
**H-506 — Propose a deferred-error stack scanned at list entries.** Same human/chat/date; asynchronous reply: “这个心智模型怎么样： defer body 发生异常时，是向 pending defer ex stack 压栈并继续下一个 defer 列表项；遇到 `defer catch ...` 项就 FILO 匹配 stack 中所有异常，类型对的就给它handle，类型不匹配的保持留在栈里”. Supplies the stack model for evaluation: proceed through the defer list, queue deferred failures, scan all matching errors newest first at a bodyless entry, and retain nonmatches. H-507 uses this new model to specify finalization. Residual failures across scope boundaries, rethrow order, explicit returns and new registrations are not decided by this proposal.

<a id="h-507"></a>
**H-507 — Allow bodyless finally once per handler-entry visit.** Same human/chat/date; message: “新的执行模型倒是支持 `defer catch ...` 有 `finally` 块，就是扫过它的时候执行一次，这样 `defer catch (e) {throw e;} finally { ... }` 可以比较有用”. Restores optional finally, superseding H-504, with one execution when the list passes the entry, not once per handled exception. Uses H-506's stack-scan model and identifies pass-through handling as useful. Exact ordering of rethrown/replacement errors and residual completion remains separate.

<a id="h-508"></a>
**H-508 — Propose checked/unchecked rethrow wrappers.** Same human/chat/date as H-503; asynchronous answer: “rethrow `e:Exception` 的 `throw e;` 解释为 `throw make RethrownException(e -> cause)` 如何？ checked 是 RethrownException，unchecked 是 RethrownError”. Requests evaluation of a cause-bearing wrapper classified by checkedness. This does not independently settle which throw expressions count as rethrow, bare throw, fresh construction, stack order, matching or allocation. D-17-RETHROW/P-464–P-466 records the proposal without qualifying the broader operation.

<a id="h-509"></a>
**H-509 — Preserve all residual deferred errors in UnhandledDeferError.** Same human/chat/date; asynchronous answer: “合并一个 UnhandledDeferError() 保留所有 exceptions 作为其字段吧”. Selects a single aggregate with all remaining exceptions as its field content, replacing the proposed newest-only/discard-rest boundary. Error follows the established unchecked naming contract. Collection API/layout, original-body-error combination, checked-obligation integration, nested aggregates and owner/allocation realization remain separate; no implicit cause links are selected merely by collection membership.

<a id="k-defer-stack-20261116"></a>
**K-DEFER-STACK-20261116 — Integrate the current stack direction and correct the agent timing interpretation.** D-17r71→72/D-15r34→35/D-10r183→184/D-21r15→16/source368→369/registry764→765 records H-503/H-505–H-509 in D-17-DEFER-DISPATCH/P-461–P-463 and D-17-RETHROW/P-464–P-466. Candidate C-36/C-44/C-63/C-65 paths preserve charter §§2/3.2–3.3/4.3/6.3/8/12 through explicit contexts, cleanup order and owner validity. Replace K-DEFER-CONTRACT-NOTICE-20261116's agent-reported immediate-dispatch interpretation with queued list-position handling; correct current authority/source summaries without rewriting that historical receipt. Record once-only optional finally, no ordinary-body recovery, handler self-exclusion and repeated-invocation ownership obligations. H-509 selects UnhandledDeferError retaining all residual exceptions; H-508 proposes cause-bearing checked/unchecked rethrow wrappers. Wrapper scope/order, original-body/checkedness integration, explicit returns and cleanup-time registration remain pending; no physical stack or compiler support is approved. Derive that standalone defer finally adds no behavior beyond defer block, while attached finally remains meaningful. Main developer review identifies the affected consumed-once/single-pending-error implementation and owner/registration concerns, not human approval of a replacement. Preserve both merged histories and unrelated qualified contracts, including D-16. Baseline: `317f59fa9c7168397bd60f3cb1c36dd1e7195ff5` at affected paths. Documentation only; no source edits, build or tests.

## 2026-11-16 — Mark the defer handler contract under revision

<a id="k-defer-contract-notice-20261116"></a>
**K-DEFER-CONTRACT-NOTICE-20261116 — Suspend delegation of superseded incoming-error behavior.** Agent receipt/authority assessment, not a new human confirmation. Source: an authorized [design-chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) message received 2026-11-16 America/Los_Angeles, reporting the user's correction: only bodyless defer catches handle other actions' failures, immediately and potentially repeatedly; a bodyful defer runs its own body and catches its own execution. The design chat is preparing the human decision record and unresolved retirement/finally/outstanding-exception choices. D-17r70→71 marks that affected H-360/P-323 dispatch/repeatability scope as requiring revised-contract integration, including global module cleanup, under charter §§3.3/4.3/5.3/6.3/12. Unaffected preparation, owner/access, explicit-return and ordinary per-action contracts continue. Current single-pass source evidence, including K-749, is not acceptance of the revised handler model. D-14/D-18/D-15 dependents inherit this scoped exclusion; no repeatability mechanism, handler-retirement policy or new runtime behavior is activated. Baselines: `ea5b1c1b1fae40ac3a6dad5ee775c60ebcc1d6b7` and `32b366093859990e66565b85187107ee756055b5` at affected paths; direct human evidence remains with the forthcoming design record.

## 2026-11-16 — Integrate module visibility and cleanup decisions

<a id="k-module-drain-sync-20261116"></a>
**K-MODULE-DRAIN-SYNC-20261116 — Preserve design scope and compiler closure evidence.** Agent integration/continuity under H-501/H-502/H-420/H-423 and charter §§2/3.3/4.3/5.3/6.3/8/12: D-15r33→34/source367→368/registry762/763→764 retains D-10r183, D-17r70’s unaffected scope and K-749's validated call-effect closure; the separate notice above governs D-17r71’s handler revision. Qualify the ready-node paragraph's proof uncertainty by actual context, alias and initialization evidence; legal intermediate observations do not imply binding-version ambiguity or a last-writer barrier. Preserve global reverse successful module-defer registration, nested-scope cleanup, complete captures, valid owners and D-16. Cross-module completion/handler policy and startup implementation remain separate. Baselines: `ea5b1c1b1fae40ac3a6dad5ee775c60ebcc1d6b7` (compiler) and `32b366093859990e66565b85187107ee756055b5` (design) at affected paths. Both decision histories remain intact. Documentation integration adds no compiler acceptance or human confirmation.

## 2026-11-16 — Close call effects through their dependencies

<a id="k-749"></a>
**K-749 — Dependency-directed call summaries.** Agent continuity under D-17r69/C-63/P-235/P-236, D-14r116’s confirmed initialization/alias scope, D-18’s qualified capture/call contracts and D-26r21/H-407: source366→367/registry761→762 retains the existing write-before-read fixed points, returned/payload joins, reaching-callable epochs and recursive obligations. Named/value calls and complete-value captures register their summary dependencies; new coverage rejoins closure and final validation verifies stability. Charter §§3.3/5.3/6.3/12 supports reusable checked dependencies without altering observable effects. D-16, source admission, permanent analysis/ABI choices and pending startup/storage scopes remain unchanged. Baseline: `ff3c790828bbc278596311d01363273a1f8affa0` at affected paths. No new language rule, performance promise or human confirmation.

## 2026-11-16 — Integrate the startup visibility and cleanup batch

<a id="k-startup-batch-integration-20261116"></a>
**K-STARTUP-BATCH-INTEGRATION-20261116 — Preserve current compiler evidence with H-501/H-502.** Agent continuity/integration, registry761/762→763, retaining D-15r33/D-10r183/D-17r70 and candidate P-455–P-460 under charter §§2/3.2–3.3/4.3/6.3/8/12. Preserve main's K-747 result-obligation and K-748 stored null-test proof coverage, D-18r107/source366, complete contexts, owners, captures, failure invariants and D-16. Startup block visibility and global module-defer order are confirmed at their stated scope; general scheduling proofs and cross-module completion/handler integration remain separate. Both histories are retained. Baselines: `ff3c790828bbc278596311d01363273a1f8affa0` (main) and `a315aee831747be10303d2e83cc4b5c1409e625e` (design checkpoint) at affected paths. Documentation only; no new runtime evidence, qualification or build/test execution.

## 2026-11-16 — Initialization blocks determine binding visibility

<a id="h-502"></a>
**H-502 — Use the same-block versus separate-block distinction.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Responds to the x=1/x=2 and B-read comparison: “这个不需要纠结吧，就看 `@x = 1` 和 `@x = 2`; 是同块还是分在两个 postpon block 里”. Resolves it through existing whole-block scheduling: another scheduled block cannot run between writes in one block, while separate blocks permit intermediate observation at a legal boundary. D-15 derives that no extra source-final-binding barrier or separately named intermediate resource is required. This does not choose an earliest-reader/latest-writer priority, refresh stored captures, turn blocks into transactions or approve a general alias proof mechanism.

<a id="k-block-binding-visibility-20261116"></a>
**K-BLOCK-BINDING-VISIBILITY-20261116 — Remove the redundant final-publication policy.** D-15r32→33/D-10r182→183/registry761→762 applies H-502 to D-15-PUBLICATION/P-455–P-457. Candidate C-35/C-36/C-44/C-59/C-65 paths preserve charter §§2/3.2–3.3/4.3/6.3/8: explicit block boundaries, actual contexts, observable effects and live owners. Replace the K-STARTUP-PEER-REVIEW-20261116 final-binding candidate with normal-completion block-state reasoning; retain checked identities/aliases, selected/captured values, ordinary within-block effects and conservative unknown-effect rejection. H-501 shutdown order, existing qualified scopes and D-16 continue. General proof/representation and cross-module completion handling remain separate. Baseline: `535e1fef99565ce4e99166fd9a8fa47ad0cd1f43` at affected paths; prior uncommitted decision records remain below. Documentation only; no build/tests or implementation claim.

## 2026-11-16 — Global order for module-level deferred cleanup

<a id="h-501"></a>
**H-501 — Drain module-level defers in application-wide reverse registration order.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer to the A1/B1/A2 example: “Use application-wide reverse registration: 3, 2, 1”. Confirms that observable order, each action's retained module context and valid owners, and unwinding only established work on startup failure. Whole-module grouped draining is not selected. This concerns module-level cleanup, not runtime startup scheduling; no physical queue ABI, new cross-module handler visibility or imported-binding publication policy is confirmed.

<a id="k-module-shutdown-20261116"></a>
**K-MODULE-SHUTDOWN-20261116 — Integrate global module cleanup ordering.** D-15r31→32/D-17r69→70/D-10r181→182/registry760→761 applies H-501 to D-15-SHUTDOWN/P-458–P-460. Candidate C-36/C-44/C-59/C-63/C-65 paths serve charter §§2/3.3/4.3/6.3/12 through reverse observable registration and valid context/owner cleanup. Derive actual successful registration order, entry participation, skipped/failed-registration exclusion and ordinary nested-scope boundaries. Preserve existing qualified defer preparation, capture/current-binding, transfer, failure and intrinsic-release rules plus D-16. Full value-path/physical qualification and cross-module pending-completion/handler integration remain separate; D-15-PUBLICATION stays pending. This decision joins the startup-design batch for later synchronization. Published baseline: `535e1fef99565ce4e99166fd9a8fa47ad0cd1f43` at affected paths; the uncommitted peer-review event remains below. Documentation only; no build/tests or implementation claim.

## 2026-11-16 — Developer review of initialization publication and shutdown

<a id="k-startup-peer-review-20261116"></a>
**K-STARTUP-PEER-REVIEW-20261116 — Record feasibility and isolate observable choices.** Agent assessment/continuity, D-15r30→31/registry759→760, under unchanged H-493–H-500/H-423 and charter §§2/3.2–3.3/4.3/6.3/8/12. Source: the [main development chat](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6), received 2026-11-16 America/Los_Angeles; no human approval is claimed. The developer confirms main/origin at `143ad9957c14876a39eec12da0666d3c710324fe` and recommends shared evolving module contexts, with existing call/cleanup summaries reused. Final-binding publication is considered feasible using last-possible-write and normal-completion evidence by actual context/binding identity, including block cleanup but excluding process-exit defers. Preserve complete captures, live owners, mutable referents and nonexecuting type queries. D-15-PUBLICATION/SHUTDOWN and candidate P-455–P-460 keep final versus intermediate binding publication and cross-module cleanup order pending-human; examples distinguish x=1/2 and global 3/2/1 versus grouped module cleanup. Existing qualified scope, fixed startup order and D-16 continue; runtime representation and compiler support remain separate. Integration preserves K-746 and current D-18r107/source364 coverage. Baseline: `535e1fef99565ce4e99166fd9a8fa47ad0cd1f43` at affected paths. Documentation only; no build/tests.

## 2026-11-16 — Retain stored null-test evidence

<a id="k-748"></a>
**K-748 — Boolean storage and mutation evidence.** Agent continuity under D-17r69/H-313/H-440/H-441/C-63/P-235/P-236 and D-14r116’s confirmed storage/alias scope: source365→366/registry760→761 preserves checked null access, stable tested bindings, independent initialization/owner/access obligations and ordered cleanup. Stored boolean snapshots retain existing expression proofs; shared possible-write footprints invalidate affected aliases across calls, loops and deferred actions even when a write preserves its provisional value. Charter §§3.1/3.3/5.3/6.3/8/12 supports inspectable, sound semantic evidence and native equivalence. Arbitrary helper-result correlations, delayed boolean postconditions, pending broader designs and D-16 remain unchanged. Baseline: `1da8accf933acfcbbace7337b632e54618ed83d5` at affected paths. No new syntax, runtime predicate representation or human confirmation.

## 2026-11-16 — Reconsider result evidence through its dependencies

<a id="k-747"></a>
**K-747 — Incremental result obligations.** Agent continuity under D-24r8/H-421–H-424/K-723/K-724 and D-26r21/H-406/H-407: source364→365/registry759→760 preserves independent recursive anchors, selected method/type/context/owner identity, existing conversions and explicit resource-stop behavior. Method indexing, reverse dependencies and an iterative component walk avoid rebuilding unrelated result evidence without using a cycle's provisional result as its anchor. Charter §§3.3/5.3/6.3/8/12 supports checked, reusable semantic dependencies; D-16, D-18 captures, module startup and pending inference/ABI scopes remain unchanged. Baseline: `535e1fef99565ce4e99166fd9a8fa47ad0cd1f43` at the affected paths. No new language rule, performance budget or human confirmation.

## 2026-11-16 — Retain definition evidence by dependency

<a id="k-746"></a>
**K-746 — Dependency-limited checking and current coverage.** Agent continuity under H-329/H-331/H-342/H-344/H-481/K-743 and P-297/P-301–P-303: D-18r106→107/source363→364/registry758→759 preserves capture timing, definition identity, type queries, owner/access evidence, nested shadow diagnostics and cleanup, serving charter §§3.2/3.3/4.3/6.3/8/12. The checker projects retained evidence and diagnostic names onto actual dependencies; D-18's stale recursive-support statements now delegate to current source coverage. Wide declared/inferred definition sets and composed semantic/native checks supply acceptance without a numeric performance promise. D-16, D-15 startup and pending broader capture/value/ABI scopes are unchanged. Baseline: `143ad9957c14876a39eec12da0666d3c710324fe` at the affected paths; no new human confirmation.

## 2026-11-16 — Rebase startup decisions onto current compiler work

<a id="k-design-startup-sync-20261116"></a>
**K-DESIGN-STARTUP-SYNC-20261116 — Preserve both source coverage and design decisions.** Agent integration/continuity under H-492–H-500/H-420/H-423 and charter §§2/3.1–3.3/4.2–4.3/6.3/8/12: source362→363/registry757→758 retains D-15r30/D-07r162/D-10r181/D-03r1 and P-443–P-454 while incorporating main's K-737–K-745 source evidence. Preserve H-481/K-743/K-745 fresh local/recursive callable construction, selected module contexts, owner/capture/failure invariants and D-16. Initialization scheduling and namespace entry remain unimplemented; merge resolution grants no new design qualification or executable acceptance. Main baseline: `faeac4f6dac0d4c8a63b92c0752dad3bd4b5bef6` and design checkpoint `6640f498a661108085fc43e78505c5efa5351b91` at affected documentation paths. Both changelog payloads remain intact; code/build/tests unchanged.

## 2026-11-16 — Static startup and namespace-selected entries

<a id="h-495"></a>
**H-495 — Schedule dependency prefixes as postpon blocks with module contexts.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Says “除了入口模块特殊，被依赖的所有模块的 init prefix 都看作一个等效的全局 postpon 块去排序就行，排序对象都是 "postpon block"，把所属模块上下文看作它们的capture”. Selects a common scheduling model for dependency prefixes and explicit blocks, with the entry exception separate. D-15 integrates each dependency prefix as one node followed by its ordered suffix, sharing its evolving module context. No ordinary closure-capture expansion or physical context ABI is confirmed.

<a id="h-496"></a>
**H-496 — Entry executable prefix runs last.** Human/source: same project user/chat/date; asynchronous answer to the entry-role question. Says “我倒觉得 entry modu prefix 需要最后执行，不能提前”. Rejects entry-prefix-first startup. With H-495's explicit exception, D-15 derives that entry postpon blocks participate in initialization before its ordinary executable prefix, and earlier nodes cannot rely on values installed only by that prefix. No implicit statement hoisting is selected.

<a id="h-497"></a>
**H-497 — Static definitions populate runtime module namespaces before initialization.** Human/source: same project user/chat/date. Says “模块静态定义静态填入运行时模块namespace，在所有 postpon 块之前完成”. Confirms runtime module definition readiness before all initialization nodes, including implicit prefixes. Definition registration remains distinct from dynamic binding/value initialization and callable capture. Does not select a runtime dictionary representation, namespace expression syntax or arbitrary compile-time execution.

<a id="h-498"></a>
**H-498 — Emit a static native initialization order.** Human/source: same project user/chat/date. Says “顺序排布” should be “静态完成，作为结果程序的物理机器码顺序体现，不做运行时动态排布”, with any difficulty requiring runtime ordering raised as a separate issue. Confirms a compiler-chosen fixed native sequence, excluding runtime scheduling or fallback among schedules. Unproved cases reject pending separate review; ordinary runtime branches/calls inside a block retain their semantics.

<a id="h-499"></a>
**H-499 — Reserve the local entry name and exclude source reuse.** Human/source: same project user/chat/date. Proposes “所有入口模块都局部命名为 `__main__` 且不允许被其它模块 using/export”; after writing __main___ in a subsequent CLI remark, answers the spelling question “`__main__` — two trailing underscores (Recommended)”. Confirms canonical `__main__` and entry modules as non-reusable source roots. The original CLI phrase “使用 ... 模块的pkg名” has its package-versus-namespace meaning clarified in H-500; it is not evidence for one entry per dependency package.

<a id="h-500"></a>
**H-500 — Select the entry's containing namespace within a dependency package.** Human/source: same project user/chat/date. Distinguishes package as “依赖解析和版本管理单元” from internal directory/module hierarchy and requests suitable terminology. After the namespace/subnamespace/segment proposal, answers the explicit CLI question “Select the containing namespace, allowing several entry namespaces in one dependency package”. Confirms that selection granularity and multiple independent entries per package. D-15/D-03 use package, namespace, segment, module and directory for their distinct roles. Exact CLI/path grammar, physical namespace representation and general package-management policy are not approved by this answer.

<a id="k-static-module-startup-20261116"></a>
**K-STATIC-MODULE-STARTUP-20261116 — Integrate startup order and entry identity.** D-15r29→30/D-07r161→162/D-10r180→181/source360→361/registry755→756 and D-03r1 terminology integrate H-495–H-500. D-15-POSTPON/P-446–P-448 and new D-15-STATIC-NS/ENTRY/P-449–P-454 retain candidate C-10/C-35/C-36/C-44/C-45/C-59/C-65 paths to charter §§2/3.1–3.3/4.2/6.3/8/12: explicit identity, shared context, readiness, ownership and observable native order. Update C-44/P-104/P-105's encounter-order boundary while preserving once-only incorporation, H-166 lifetimes and H-379 installation obligations; static namespaces do not install captured values. Entry source-selection restrictions extend consistently through aliases/re-export. D-15 owns acceptance and unresolved proof/representation/CLI mechanisms. D-03's broader design and all candidate value paths remain unqualified; existing qualified owner/capture/failure scopes and D-16 are unchanged. Correct the stale D-GOV revision label; retain prior H/K evidence. Published baseline: `fa92f9cdf76e503ef832dd112885a15f777a02e8` at affected paths; D-03's registered baseline blob is `4146412997de5179146a7010b5b034e32fcebfb9`. Documentation only; no source/build/test changes or new implementation evidence.

## 2026-11-16 — Declaration cycles and ordered postponed initialization

<a id="h-492"></a>
**H-492 — Allow declaration-only module cycles.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Replies “同意” to allowing mutually dependent declaration-only modules after identities/signatures are registered, illustrated by even.cod/odd.cod. The proposal kept runtime initialization cycles separate. Confirms that bounded permission; no blanket acceptance of unanchored identities, invalid type recursion or general cyclic interpretation is implied.

<a id="h-493"></a>
**H-493 — Propose statically scheduled top-level postpon blocks.** Human/source: same project user/chat and recording date. Continues “顺便讨论 initializaion cycles，我考虑增加 `postpon { ... }` 语法，允许出现在模块 toplevel，由编译器根据其内部提及引用的关系，静态排布这些初始化块的执行顺序，排得出来线性初始化执行顺序的就accept，否则reject，你推演细节看看”. Requests design derivation for static initialization scheduling; raw name mentions alone, implicit resource publication, runtime retries and a complete dependency algorithm are not approved. H-494 supplies the subsequent source-order restriction.

<a id="h-494"></a>
**H-494 — Postpon blocks form ordered module suffixes.** Human/source: same project user/chat and recording date; asynchronous answers to placement among ordinary statements and competing providers. Says “postpone block 只允许出现在模块末尾，但是允许多个这种块；多个 postpone 块之间的顺序最好是不允许打破，只允许其间被编译器隐含插入其它模块的 postpone 块。 但是保留进一步设计语法，允许乱序执行的postpone块这种机制，如果有切实价值的话”, and “普通 `postpon { ... }` 块之间的顺序要保障，乱序的即使允许也要不同语法区分开。” Confirms trailing blocks with mandatory per-module order and whole other-module blocks interleaved. Refines H-493 and rejects arbitrary same-module block reordering; the repeated-write example retains its ordered versions rather than needing a blanket single-provider rule. The exact source spelling remains postpon. A distinct out-of-order form is only a future design option; the global prefix/postpon phase boundary is not selected by these replies.

<a id="k-module-init-review-20261116"></a>
**K-MODULE-INIT-REVIEW-20261116 — Separate registration, readiness and source order.** D-07r160→161/D-10r179→180/D-15r28→29/source359→360/registry754→755 adds D-15-CYCLES/POSTPON and P-443–P-448 under H-492–H-494. Candidate C-10/C-36/C-59/C-65 paths serve charter §§2/3.1–3.3/4.2/6.3/8/12: finite identity construction, retained interpretations, explicit state dependencies and valid owners. D-15 records confirmed declaration-only cycles/ordered suffixes, readiness and whole-block analysis, same-module binding versions, failure/scope constraints, acceptance and the pending global phase. Preserve lookup/publication, H-481/H-482 captures/owners, D-16 and existing downstream qualification. No compiler support or full operational qualification follows. Original scan/lifetime evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:2825–2833,3094–3102`; brace-grouping scope remains D-17/H-319. Baseline: `fa92f9cdf76e503ef832dd112885a15f777a02e8` at affected paths. Documentation only; no build/tests.

## 2026-11-16 — Construct callable views from definition context

<a id="k-745"></a>
**K-745 — Separate callable definitions from stored snapshots.** Agent continuity under H-481/K-743/H-329/H-331/H-342/H-344/H-420/H-423 and charter §§3.2–3.3/4.3/6.3/8/12: source361→362/registry756→757 continues the qualified C-65/C-73/C-75/P-297/P-301/P-303 capture/value scopes. Local and recursive view construction retains lexical identities and current required TLS; stored values preserve complete environments. Direct calls prepare captures without an intermediate callable image, and queries retain nonexecuting evidence and availability checks. Native cleanup includes failed nested construction, copying and deferred registration. D-18r106/D-10r179, candidate P-419–P-421 qualification boundaries, regional capture restrictions and D-16 remain unchanged; no lexical-cell promotion, lifetime extension or permanent ABI is selected. Baseline: `dae1dabf261a664485f831bce5ab3427cc8544e2` at affected paths.

## 2026-11-16 — Integrate reference-symbol and module-selection decisions

<a id="k-744"></a>
**K-744 — Preserve current compiler scope while integrating published designs.** Agent continuity under H-482–H-491/H-420/H-423 and charter §§3.1–3.3/4.2–4.3/6.3/8/12: source360→361/registry755→756 integrates D-07r160/D-10r179/D-14r116/D-15r28/D-18r106/D-28r26. C-36/C-54/C-65/C-93/P-422–P-424 preserves explicit reference mode and installation-selected owners; C-35/C-45/C-57/C-59/P-425–P-442 preserves identity/context, explicit selection, public barriers and whole callable families. Retain scoped decisions, candidate qualification boundaries, H-481/K-743 construction semantics and K-741/K-742 proof coverage. No source implementation, general graph proof, lifetime extension, new ABI, fraction scope or broad qualification is inferred. Both histories are preserved. Baselines: `fcc1cae7a193c31825ccbf7ec5b7d93c4093872c` and `fa92f9cdf76e503ef832dd112885a15f777a02e8` at affected paths. Documentation only; executable sources and compiler remain unchanged.

## 2026-11-16 — Reconcile callable construction with the synchronized decision

<a id="k-743"></a>
**K-743 — Apply H-481 to lexical construction context.** Agent continuity, source359→360/registry754→755; D-18r105/D-10r174 remain unchanged. The current project user directs the pending question to the design synchronization: “参考设计决策线程即将发来的同步内容，包含相关决策更新”. The relay from chat `01a1e3b0-18b1-7e01-933e-5052a696b59d` identifies committed `6038adc2f9346c3c838195ea3813170dcd1a600e:design/RegionalClosures.md` and excludes uncommitted work. Under H-481 and H-329/H-331/H-342/H-344/P-297/P-301/P-331, fresh construction copies required lexical values through established binding identities; an earlier stored view remains independent, and escaped reconstruction uses retained dependencies. Current required symbolic bindings refresh. C-65/C-73/C-75/P-419–P-421 and charter §§3.2/3.3/4.3/6.3/8 preserve explicit identity, complete values and valid lifetimes. This records application of the synchronized rule, not a new human approval, executing-self value, lexical-cell capture, ABI or compiler-support claim. Local/recursive construction remains implementation work. Baseline: `4d6009db5418286044dd9e91a0bef4817b935376` at affected paths.

## 2026-11-16 — Resolve cleanup failure categories

<a id="k-742"></a>
**K-742 — Distinguish impossible cleanup failures from unknown values.** Agent continuity under H-267/H-269/H-420/H-423 and charter §§3.3/5.3/6.3/12: source358→359 and registry753→754 continue D-17r69/C-63/P-235/P-236. Resolve explicit failure relations against payload facts before checking an exceptional exit; retain uncertainty through composition and preserve declared throws, partial registration, initialization and cleanup. Source/native coverage includes narrow deferred handlers, nested/loop queues and recursive summaries. Shared source checks match messages without filenames and assert requested locations separately; negative fixtures use current construction syntax and precise failure expectations. Wider outcome correlations and D-16 remain separate. Baseline: `5130789ed831ca3c326f07d084c8b1b086c265e3` at affected paths.

## 2026-11-16 — Loop exception payload invariants

<a id="k-741"></a>
**K-741 — Preserve exception facts across non-writing loops.** Agent continuity under H-267/H-269/H-420/H-423 and charter §§3.3/5.3/6.3/12: source357→358 and registry752→753 continue D-17r69/C-63/P-235/P-236 and D-14r115’s qualified loop/target/alias contract. Separate payload-write effects from slot rebinding; include later-selected destinations and possible borrowed aliases. Zero/single iterations retain sequential facts; modified payloads and writable call contracts remain conservative. Source/native checks cover preserved categories, required checked failures and exact cleanup. Broader action/value correlations, source semantics, D-16, regional/runtime protocols and pending design qualification are unchanged. Baseline: `a1a2dead064a44ba46bb6fb71e028a84f2c56935` at affected paths.

## 2026-11-16 — Callable mention conformance boundary

<a id="k-740"></a>
**K-740 — Record local-definition construction gaps.** Agent evidence/continuity under H-481/H-420/H-423 and charter §§3.2/3.3/5.3/6.3/8/12: source356→357 and registry751→752 distinguish module-definition freshness from local definitions' current declaration-time snapshots and early symbolic-capture requirement. Recursive materialization remains unsupported. Both native modes reproduce the bounded mismatch; it is not a language policy or a reason to weaken the confirmed fresh-symbolic-view rule. Mutable lexical dependency timing is a separately identified clarification; stored copies, current symbolic captures and all existing qualifications remain unchanged. No source implementation, physical context or ABI is selected. Baseline: `85b7a4e04276442a541f7636a712a129ad623700` at affected paths; semantic design revisions remain D-18r105/D-10r174.

## 2026-11-16 — Callable mention and compiler-scope integration

<a id="k-739"></a>
**K-739 — Integrate the confirmed callable definition-mention decision.** Agent continuity under H-481/H-420/H-423 and charter §§3.2/3.3/4.3/6.3/8/12: source355→356 and registry749/750→751 reconcile D-18r105/D-10r174 with current module/loop/cleanup coverage. Preserve definition identity, fresh construction timing, complete stored copies, admitted lexical environments, current symbolic captures, valid owners/access/init and nonexecuting queries. P-419–P-421 retain the confirmed local source contract and pending full value-path/physical qualification. The former recursive capture-time choice is settled; recursive construction still lacks implementation. No runtime, ABI, build or test change. Baselines: `6038adc2f9346c3c838195ea3813170dcd1a600e` at incoming design paths and `c8d5f662d91f0aeaf91411841eed2ecc5b23e65b:tests/cod/language/README.md`; both event histories remain intact.

## 2026-11-16 — Preserve invariant loop targets

<a id="k-738"></a>
**K-738 — Use loop rebinding effects for target proofs.** Agent implementation/continuity under H-272/H-293/H-299/H-302/H-313/H-356–H-362/H-366–H-371/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/12: source354→355 and registry749→750 preserve D-14r115’s target/access/owner invariants and D-17r69’s C-63/P-235/P-236 cleanup contract. Shared physical-slot mutation paths identify loop-carried rebinding; unchanged targets retain established evidence, while modified slots and expired owners retain their checks. Prepared guards preserve the distinction between invariant slots and repeated mutations. Acceptance covers ordinary/generic/alias/callable reads, nested loops, initialization, deferred operations and native cleanup. Wider target/payload/branch correlations remain unproved; no source-policy, runtime, ABI, regional or fraction change. Baseline: `24badd22a62650ad0fa706720cc30b169945ee9f` at affected paths.

## 2026-11-16 — Explicit source-module traversal

<a id="k-737"></a>
**K-737 — Preserve source graph semantics without recursive host traversal.** Agent implementation/continuity under H-106/H-109/H-166/H-245/H-247–H-249/H-375–H-378/H-398/H-420/H-423 and charter §§2/5.3/6.3/12: source353→354 and registry748→749 preserve D-10/D-15/D-21's scoped source identity, publication, isolation and once-only initialization through explicit discovery/planning continuations. Canonical source reuse, dependency-first construction, initializer order, cycle/unsupported-scope diagnostics and shared native methods remain intact. Acceptance covers deep shared graphs and initialization failure; retained module scopes, conditional first-reference policy, symbolic import/cancellation, broader composition and persistent ABI retain their separate boundaries. D-16 and regional/runtime contracts are unchanged; no new design qualification. Baseline: `3ceed5c20050e5ff4fb231288e679f3b8a04ff84` at affected paths.
## 2026-11-16 — Explicit callable bundles and independent re-export

<a id="h-488"></a>
**H-488 — Publish the upstream object while retaining a private local namesake.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Says “允许中间模块选择 reexport 其上游模块的对象，保持其私有同名对象私有，让其下游模块可以使用间接隐含导入的对象，将来的诊断信息这时应提示从实际定义的模块显式导入”. Refines H-487: an explicit public re-export can supply the demanded spelling with an upstream identity while the local namesake remains private. Future information recommends selection from the actual defining module. This does not allow skipping private barriers without publication or discarding selected callable interpretations.

<a id="h-489"></a>
**H-489 — Add optional explicit associated-callable selection.** Human/source: same project user/chat and recording date. Says “感觉 using/import/symbol-from 可以支持可选的 `with methods` 子句，视为显式带入所有相关 methods，你一起细化推演”. Selects the clause direction; the exact selection unit is confirmed in H-490.

<a id="h-490"></a>
**H-490 — Using/import always selects whole callables.** Human/source: same project user/chat and recording date; asynchronous answer to the Node-associated score family also containing score(u64). Says “对，`with methods` 不是仅相关method，而是相关method对应的所有callable； using/import 总是以callable为单位，而不是特定method，这与显式按 callable 名称导入简单一致。” Confirms whole callable families for the clause and ordinary using/import, accepting score(7u64) in that example. Supersedes any type-associated method-subset selection interpretation; signature association identifies callables, and aliases share identity rather than own filtered method sets.

<a id="h-491"></a>
**H-491 — Adopt publication-only export from.** Human/source: same project user/chat and recording date; asynchronous reply “Adopt export from for publication-only re-export (Recommended)”. The presented contract was `export from "core.cod" answer;`, optionally `as public_name`, preserving the upstream identity/context without creating an own local alias or extension target; a local answer remains private, and ordinary export answer still publishes the local resolution. Confirms that surface and scope. Existing export import still performs import plus publication; no compiler support or broader publication grammar is approved.

<a id="k-reexport-methods-20261116"></a>
**K-REEXPORT-METHODS-20261116 — Separate explicit selection and publication.** D-07r159→160/D-10r178→179/D-15r27→28/source358→359/registry753→754 adds D-15-METHODS-CLAUSE/REEXPORT and P-437–P-442, refining D-15-LOOKUP/INFO/TRANSITIVE under H-488–H-491. Candidate C-35/C-45/C-57/C-59 paths preserve charter §§3.1/3.2/4.2/6.3/8: whole callable identity, usable explicit behavior, retained interpretation and independent private/public intent. Remove method-subset source selection; record mode-specific extension, collisions, alias provenance, public-map precedence over private namesakes and definition-origin diagnostics. Acceptance is in D-15. H-247/H-248 identity evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:3946–3965`. Preserve H-481/H-482, D-16 and unrelated qualified scopes. General graph/context proofs, physical representation and compiler implementation remain separate. Published baseline: `6038adc2f9346c3c838195ea3813170dcd1a600e` at affected paths; preceding local decision records remain below. Documentation only; no build/tests.

## 2026-11-16 — Source fallback, private barriers and deferred diagnostics

<a id="h-484"></a>
**H-484 — Use source namespaces as fallback.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Proposes “与其在当前模块创建shadowable namespace，不如直接把源模块注册为当前模块的下层namespace，当前模块namespace找不到的，去源模块找” and directs “容易区分跟正式 using/import 对象距离远近的，以近的为准；远近不容易区分的，报ambiguous拒绝，要求用户显式 using/import”. Selects source fallback and conservative proximity instead of copied implicit-name namespaces. Refines H-483's mechanism and the prior type-name closure question; no general graph-distance algorithm, runtime namespace, private access or full implementation qualification is approved.

<a id="h-485"></a>
**H-485 — Reserve information at implicit-use sites.** Human/source: same project user/design chat and recording date as H-484. Says “保留后期加强能力： 依赖隐含解析的提及使用位置，报information级别诊断信息，建议用户显式 using/import。” Confirms a deferred informational diagnostic at mentions depending on implicit resolution, recommending explicit selection. Does not request immediate compiler implementation or select category names, controls, error escalation or automatic fixes.

<a id="h-486"></a>
**H-486 — Explore recursively composed source fallback.** Human/source: same project user/design chat and recording date. Says “src modu fallback 也自然建立其多层 fallback namespace，感觉也是比较好的结果，你细推导看看有没有quirks”. Requests multilevel derivation and examination of consequences; the publication clarification is recorded separately in H-487, rather than inferred as blanket approval of every graph mechanism.

<a id="h-487"></a>
**H-487 — An intermediate private shadow blocks downstream implicit lookup.** Human/source: same project user/design chat and recording date; asynchronous answer to the app → facade → core public-helper example, where facade does not re-export helper. Says “中间层以非public（未export）对象shadow其自身源模块的public对象，被app这种下游模块隐含解析时，报错，要求中间模块export同名对象”. Establishes the private-shadow barrier in the proposed multilevel model: do not skip an intermediate own binding to reach a hidden public ancestor; require that intermediate module to export the same-named binding. Public fallback without such a shadow remains the proposed path; explicit selection from facade still requires its own export. Refines H-231/H-233's interaction with transitive reachability without making private objects selectable or automatically publishing imported aliases.

<a id="k-source-fallback-20261116"></a>
**K-SOURCE-FALLBACK-20261116 — Integrate namespace paths and observable provenance.** D-07r158→159/D-10r177→178/D-15r26→27/source357→358/registry752→753 adds D-15-LOOKUP/INFO/TRANSITIVE and P-428–P-436, updates D-15-TYPE-METHODS/P-425–P-427 and related current scope. Candidate C-35/C-45/C-57/C-59 paths serve charter §§3.1/3.2/4.2/6.3/8/9: usable public definitions, identity/context, explicit local intent and inspectable dependency provenance. H-484–H-487 replaces recursive type-name closure with source fallback and records private barriers plus deferred diagnostics. Derive name-before-dispatch, peer ambiguity, equivalent-context deduplication, private alias barriers, non-declaring caches, explicit export versus implicit reachability and unchanged helper/initialization/capture behavior; D-15 records acceptance and general graph/proximity boundaries. Preserve H-481/H-482, D-16 and other qualifications. This is source-design integration and continuity for unchanged scopes, not compiler implementation, full value-path qualification or retrospective approval. Original export/context evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:3778–3810`. Published baseline: `6038adc2f9346c3c838195ea3813170dcd1a600e` at affected paths; prior local decision records remain below. Documentation only; no build/tests.

## 2026-11-16 — Narrow the type-selection question queue

<a id="k-type-selection-review-20261116"></a>
**K-TYPE-SELECTION-REVIEW-20261116 — Preserve existing identity consequences.** Agent continuity assessment under H-225/H-232/H-234/H-248/H-249/H-483: D-07r157→158/D-10r176→177/D-15r25→26/source356→357/registry751→752 removes two unnecessary open choices from D-15-TYPE-METHODS/P-425–P-427. Associated signatures and explicit full selections contribute to one receiver record per identity; independent local shadows do not replace a source helper dependency by spelling. Explicit family extension preserves existing helper adaptation and conflict checks. C-35/C-45/C-57/C-59 and charter §§3.1/3.2/4.2/6.3/8 retains the confirmed contract; the scored example distinguishes local7/helper2 from explicit extension/helper14. Only dependent-type name closure remains pending here, with a bounded candidate awaiting the user. H-483's decision payload and H-481/H-482 remain unchanged; no new human confirmation, compiler support or qualification. Original identity evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:3811–3820,3956–3974`; H-232 is at 3788–3799. Published baseline: `6038adc2f9346c3c838195ea3813170dcd1a600e` at affected paths; prior local integration records remain below. Documentation only; no build/tests.

## 2026-11-16 — Type selections carry shadowable method names

<a id="h-483"></a>
**H-483 — Bring type-associated methods and symbol-carried types into an implicit name layer.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Says “随着从其它模块 using/import 一个 type，所有相关的 methods 也视同一起 using/import，只是进入当前模块一个可以被shadow的命名空间，后续命名相同的直接定义视为shadow而非collision关系；那么 symbol from 对于type的带入，也走类似规则。 设计文档把这些明确讲出来，发现需要讨论的安排上后续议题”. Confirms associated method selection with both type reuse forms, local shadowing rather than collision with those implicit names, analogous symbol-from type/name integration and explicit queuing of unresolved questions. This replaces H-226's automatic extension through an implicitly imported type-associated name (original evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:3724–3735`). Explicit callable import/extension, privacy, identity and source-context contracts remain. Exact dependent-type closure, ordinary-family granularity and local-shadow/helper adaptation are not selected by this statement.

<a id="k-type-method-names-20261116"></a>
**K-TYPE-METHOD-NAMES-20261116 — Align lookup without silently choosing composition.** D-07r156→157/D-10r175→176/D-15r24→25/source355→356/registry750→751 adds D-15-TYPE-METHODS/P-425–P-427 and updates P-190/P-193/P-200 under H-483. Candidate C-35/C-45/C-57/C-59 preserves charter §§3.1/3.2/4.2/6.3/8/12: type usability, identity, source-world independence and explicit local lookup. Preserve H-225 associated signatures, H-231/H-233/H-374 publication, explicit callable extension, name-before-dispatch, no hidden-family fallback, repeated identities, captures and D-16/UoM boundaries. D-15 queues three material questions with CoD examples; only the new local-shadow/imported-helper combination remains pending, while existing H-232/H-253 explicit composition stays in force. H-481/H-482 and their local evidence remain intact; D-15's TLS summary distinguishes independent bits from explicit scalar reference bindings. Published baseline: `6038adc2f9346c3c838195ea3813170dcd1a600e` at affected paths; the preceding H-482 integration record below remains intact. Documentation only; no compiler changes, build/tests or new implementation claim.

## 2026-11-16 — Explicit regional reference symbols

<a id="h-482"></a>
**H-482 — Bind symbolic references to the installation-selected region.** Human/source: current project user, via the user-authorized [side-chat handoff](codex://threads/01a1e356-a316-7680-a28e-97dc7434d4ca) received in this design chat, recorded 2026-11-16 America/Los_Angeles; exact source-message timestamp/personal name unavailable. Proposal excerpt: “感觉需要增加 `ref/ptr symbol in @some_region_sym` 机制，编译器要分析证明这些 ref/ptr sym effects 不 outlive 它们的owner region effect”. The handoff quotes confirmation: “按 effect 安装时确定实际 owner，后续 shadow 不改变已有引用；只要原 owner region effect 还活着就允许，capture 和恢复也遵循这个规则。 通知主线对话： 落实这项新增，细化推演记录无疑义设计，并继续讨论需要决策的议题。” Accepts explicit ref/ptr symbolic-reference mode, including scalar targets, and original-owner preservation through shadowing/capture/restoration with compiler lifetime proof. Retains separately declared shared-owner groups. Excludes implicit ownership transfer/extension, old-locator reinterpretation under a new region base, blanket val-symbol syntax, arbitrary lexical-owner backing and permanent ABI. This is relayed human confirmation with the stated provenance, not an independent agent approval. H-481's callable-mention decision remains unchanged.

<a id="k-reference-symbols-20261116"></a>
**K-REFERENCE-SYMBOLS-20261116 — Integrate binding mode, provenance and restoration.** Under H-482 and existing H-180/H-277–H-281/H-284–H-286/H-437/H-439/H-481, D-10r174→175/D-14r115→116/D-18r105→106/D-28r25→26/source354→355/registry749→750 adds D-10-REF-SYMBOL/P-422–P-424 and aligns P-248/P-249. Candidate C-36/C-54/C-65/C-93 preserves charter §§3.2/3.3/4.3/6.3/8/12: explicit storage mode, actual-owner interpretation, scoped effects and valid native use. Derive null/context checks, retained alias/capture identity, target writes versus TLS rebinding, ordered publication, hidden-owner restoration, group independence and owner-information sharing; D-10 records acceptance. Preserve H-481 capture timing, D-16 numeric/bits constraints and qualified downstream scopes. Exact modifier/mixed-group grammar, runtime carriers and proof coverage remain separate; explicit reference-symbol syntax is unimplemented. Baseline: `6038adc2f9346c3c838195ea3813170dcd1a600e` at affected paths. Documentation only; no code/build/tests.

## 2026-11-16 — Definition mentions construct callable views

<a id="h-481"></a>
**H-481 — Definition names and stored callable values have distinct roles.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-16 America/Los_Angeles; exact timestamp/personal name unavailable. Proposes “让 callable mention 总是引用其 definition ，需要 callable view 的地方则新建” to avoid a source-level “currently executing callable value”; after the precise rule and examples, confirms “按此记录，提交推送”. Accepts definition-bound names with fresh view construction from established lexical context/current required symbolic bindings, stored-value/parameter copy semantics preserving captures, direct/type-only mentions without obligatory views, preserved lexical/owner constraints and no mandatory heap allocation. The recursive example returns 20; repeated definition construction captures 10 then 20, while copying the first value retains 10. No new capture eligibility, ownership extension, physical ABI or compiler-support claim is selected.

<a id="k-callable-mentions-20261116"></a>
**K-CALLABLE-MENTIONS-20261116 — Integrate definition lookup and capture timing.** Under H-481, D-18r104→105/D-10r173→174/source353→354/registry748→749 adds D-18-MENTION/P-419–P-421. The candidate C-65/C-73/C-75 path preserves charter §§3.2/3.3/4.3/6.3/8/12 and established P-297/P-301/P-331 capture/value boundaries: explicit definitions and environments, complete stored copies, valid owners/access/init, function exclusion, cleanup, default origins and nonexecuting queries. H-481 settles K-680/K-682's capture-time choice; recursive view construction remains an implementation gap. Other scopes, D-16, native coverage and pending physical/value-path qualification continue unchanged. Acceptance is in D-18; source-reference edits claim no new runtime support. Baseline: `3ceed5c20050e5ff4fb231288e679f3b8a04ff84` at affected paths. Documentation only; no build/tests.

## 2026-11-16 — ADT placement and mainline integration

<a id="k-736"></a>
**K-736 — Integrate confirmed placement decisions with current compiler scope.** Agent continuity assessment under H-478–H-480/H-420/H-423 and charter §§3.1/3.3/4.3/5.3/6.3/7.1/8/12: source352→353 and registry746/747→748 reconcile the one-nominal-ADT direction and its C-10/C-39/C-65/C-91/P-416–P-418 paths with existing checked scalar/call/cleanup capabilities. Preserve actual owners, bits classification, same-owner fields, construction/access/initialization, regional relocation, capture restrictions and explicit graph transfer. The first-field quirk retains its user workaround without additional analysis or repair. D-07r156/D-10r173/D-14r115/D-22r20/D-28r25 keep their scoped decisions and pending formal value-path/full-layout qualification; D-16/D-17/D-18/D-26 contracts remain. No source, ABI, build or runtime change; current aggregate/native-layout gaps remain explicit. Baselines: `49496546ed7c2c05e8a9bc62fd051c36480a4d1a:design/RegionalAndLexicalStorageRefactor.md` and `5fe41e756f1ca0febdaaa758a138ca8dbd4cdd88:tests/cod/language/README.md`; decision history from both commits is preserved.

## 2026-11-15 — Keep only the quirk note and user workaround

<a id="h-480"></a>
**H-480 — Leave first-field quirk avoidance to users.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “不要浪费分析复杂度考虑这个quirk，前面放个无用字段就能避免的问题，留给用户注意就行了。” Directs no additional analysis complexity for this quirk; document the preceding-field workaround and leave avoidance to users.

<a id="k-quirk-workaround-20261115"></a>
**K-QUIRK-WORKAROUND-20261115 — Remove dedicated quirk integration work.** Agent integration under H-480: D-28r24→25/D-14r114→115 and registry r746→747 simplify the current note and P-417/P-418. Supersede K-SELF-REF-QUIRK-20261115's added quirk-specific flow/acceptance requirements; retain a nonzero-sized preceding-field example and unchanged existing implementation guards. Remove proposed repair protocols and special acceptance obligations from the active design. Other owner, initialization, one-ADT, relocation and numeric contracts continue under the existing candidate C-65/C-91 path; no code, encoding, qualification or implementation-support change. Baseline: `19c0aaa5f9ff631301b73fd6e948bc1e05ef0e45` at affected paths; preceding human/agent evidence remains append-only. Documentation only; no build/tests.

## 2026-11-15 — Keep the first-field self-reference quirk nonblocking

<a id="h-479"></a>
**H-479 — Track the self/null behavior as an implementation quirk.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “首字段 self-ref 赋值相当于 null 赋值，记为已知实现 quirk ，不影响其它设计和实现吧？”. Directs recording the known first-field null behavior and asks whether other work can proceed. Retains H-146's bounded current behavior; no new encoding or universal self-reference rule is selected.

<a id="k-self-ref-quirk-20261115"></a>
**K-SELF-REF-QUIRK-20261115 — Remove the broad implementation gate.** Agent integration/assessment under H-479 and the original H-146 scope: D-28r23→24/D-14r113→114 and registry r745→746 revise D-28-ADT/P-417/P-418. Correct K-SINGLE-ADT-20261115's broad blocker characterization: the affected regional nullable edge reads back as null, while other design/implementation and acceptance may proceed with that quirk explicitly reported. Preserve owner identity, RHS effects, ordinary null/flow checks and required-ref guarantees; no lexical generalization or claim of topology preservation for the affected self-edge. Candidate C-65/C-91 paths retain charter §§3.3/4.3/6.3/12; unrelated qualifications and D-16 remain. A future encoding fix is separate engineering work. Published baseline: `19c0aaa5f9ff631301b73fd6e948bc1e05ef0e45` at affected paths; H-478 and its integration history remain intact. Documentation only; no code/build/tests.

## 2026-11-15 — One nominal ADT across storage domains

<a id="h-478"></a>
**H-478 — One ADT is settled; derive unambiguous consequences directly.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Replying to the same nominal ADT across lexical/regional placement proposal, says “one ADT 是必然选择，类似这种没有疑义的，你直接决定然后继续展开细化推演设计，没有疑义的直接记录，需要讨论的回来找我”. Confirms nominal unity and directs continued design derivation with only material uncertainty returned for discussion. This does not approve every physical format, ABI, capture extension or implementation mechanism; existing same-owner and lifecycle constraints remain.

<a id="k-single-adt-20261115"></a>
**K-SINGLE-ADT-20261115 — Integrate placement without duplicating semantic types.** Agent integration under H-478/H-423: D-28r22→23/D-22r19→20/D-14r112→113/D-07r155→156/D-10r172→173 and registry r744→745 add D-28-ADT/P-416–P-418. Candidate C-10/C-39/C-65/C-91 value path preserves charter §§3.1/3.3/4.3/6.3/7.1/8/12. Record shared nominal identity/conformance/methods, actual-owner layout evidence, automatic addressing, common construction and all four graph-transfer pairs; retain the bits predicate, same-owner fields, explicit destination when no context exists, initialization, cleanup, capture/TLS restrictions and C++ regional-only rules. Bits-only lexical support is an implementation fallback. H-146's temporary self/null limitation remains a tracked blocker to complete encoding acceptance; original evidence is `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:2886–2895`, not an authorization for a new header/format. Physical protocols and full qualification remain pending; preserve unrelated mainline inference/failure contracts and source350. Composed acceptance and engineering obligations are in D-28. Baseline: `19c0aaa5f9ff631301b73fd6e948bc1e05ef0e45` at affected paths. Documentation only; no compiler, build, tests or performance claim.

## 2026-11-15 — Handler queues and finally composition

<a id="k-735"></a>
**K-735 — Retain registration histories through completion.** Agent continuity assessment under H-267/H-269/H-272/H-313/H-327/H-356–H-362/H-366–H-371/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source351→352 and registry745→746 preserve C-63/P-235/P-236/P-280's ordered cleanup and C-65/C-75/P-255/P-257/P-295/P-327's target/initialization obligations. Queues belong to each incoming state; finally registration sequences compose with entering histories. Finite graph composition preserves loop backedges and action identity. Acceptance covers nested/conditional registration, loops, ordinary and deferred handlers, finally replacement, failed registration, shared recursive native bodies and exact release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged. No new language, ABI, allocation or qualification. General branch/state and post-join value correlations remain conservative. Baseline: `f64ca7bbf38c711497c7f9d3bb1dbe4cb806cf1f:cod/lang_checked.hh`, `cod/lang_completion.hh` and `tests/cod/language/README.md` at that commit.

## 2026-11-15 — Deferred handler payload state

<a id="k-734"></a>
**K-734 — Preserve payload facts for each deferred input.** Agent continuity assessment under H-267/H-269/H-272/H-313/H-327/H-356–H-362/H-366–H-371/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source350→351 and registry744→745 preserve C-63/P-235/P-236/P-280's completion contract with C-65/C-75/P-255/P-257/P-295/P-327's actual target and snapshot obligations. Unified per-input facts retain handler writes through completion joins; explicit payload-map simulation removes temporary checker-state substitution. Acceptance covers fallback recovery, skipped bodies, finally replacement, nested scope cleanup, checked rejection, recursive shared native bodies and exact release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged. No new language, ABI, allocation or qualification. Queue/target correlations, post-join reads and prepared caller-slot payloads remain conservative. Baseline: `19c0aaa5f9ff631301b73fd6e948bc1e05ef0e45:cod/lang_checked.hh` and that commit’s `tests/cod/language/README.md`.

## 2026-11-15 — Cleanup payload transformations

<a id="k-733"></a>
**K-733 — Compose borrowed payload effects through cleanup.** Agent continuity assessment under H-267/H-269/H-272/H-313/H-327/H-356–H-362/H-366–H-371/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source349→350 and registry743→744 preserve C-63/P-235/P-236/P-280's completion/snapshot contract with C-65/C-75/P-255/P-257/P-295/P-327's independent values and actual borrowed targets. Shared cleanup effects compose sparse payload transformations; prepared values retain registration snapshots, borrowed inputs read at invocation, and nested action-entry origins remain distinct. Simultaneous substitution preserves saved exceptions/returns across writes. Acceptance covers ordered actions, failures, aliases, recursive helpers, loops, nested scopes, native code sharing and exact release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged; the source reference also distinguishes prohibited lexical-cell capture from unimplemented regional captures. No new language, ABI, allocation or qualification. Target/handler/queue correlations and prepared caller-slot payloads remain conservative. Baseline: `f64fafd5fb64ab7ddd45699e0223f8ceef63415f` at affected paths.

## 2026-11-15 — Borrowed exception call postconditions

<a id="k-732"></a>
**K-732 — Retain borrowed payload evidence across calls.** Agent continuity assessment under H-267/H-272/H-313/H-327/H-366–H-369/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source348→349 and registry742→743 preserve C-65/C-75/P-255/P-257/P-295/P-327's independent values and actual borrowed targets with C-63/P-235/P-236/P-280's failure/completion contract. Existing call equations retain per-completion payload facts for original targets, compose entry snapshots and combine alias alternatives; recursive summaries converge without changing declared failure paths or native identity. Acceptance covers normal/exceptional mutation, copied/borrowed operand timing, unchanged targets, local rebinding, direct/indirect/symbolic calls, recursion, negative alias/access/init cases and exact native release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged. No new semantics, ABI, regional representation or qualification; prepared/deferred mutable-state precision, thrown-value summaries and broader correlations remain separate. Baseline: `a1de69c01da3cd6ee6cacfd2426dadad6273b2a8` at affected paths.

## 2026-11-15 — Caught exception-value evidence

<a id="k-731"></a>
**K-731 — Retain value relationships through handlers and cleanup.** Agent continuity assessment under H-267/H-269/H-272/H-313/H-327/H-356/H-361/H-362/H-366–H-369/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source347→348 and registry741→742 continue C-63/P-235/P-236/P-280's immutable exception, handler and completion contract with C-65/C-75/P-255/P-257/P-295/P-327's independent values and callable coverage. Abrupt paths and cleanup equations retain the same finite value facts as ordinary results; typed matching filters origins, explicit failure replaces the pending value, and nested deferred arrivals remain distinct from earlier caught snapshots. Acceptance covers rethrows, finally/defer recovery and replacement, pending returns, explicit causes, prepared failures, recursive forwarding, equivalent root-handler spellings and exact native release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged. No new source capture rule, runtime metadata, ABI, regional representation or qualification; declared checked obligations, interprocedural thrown-value proofs and broader correlations retain their separate boundaries. Baseline: `564dd822f4e20afd80319786fb30718f3fcb0dfd` at affected paths.

## 2026-11-15 — Callable exception-result evidence

<a id="k-730"></a>
**K-730 — Preserve exception values across callable boundaries.** Agent continuity assessment under H-267/H-269/H-272/H-313/H-327/H-356/H-361/H-362/H-366–H-369/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source346→347 and registry740→741 continue C-63/P-235/P-236/P-280's checked-failure and cleanup contract with C-65/C-75/P-255/P-257/P-295/P-327's independent values, borrowed targets and callable coverage. Existing return equations retain finite category facts and entry-argument relationships through copies, alias mutation, recursive calls and completion replacement. Borrowed result reads follow final cleanup; value returns keep their saved payloads. Acceptance covers ordered operands, direct/indirect and symbolic calls, recursion, finally/defer replacement and recovery, declared throws, shared code and native resource release. D-14r112/D-17r69/D-18r104/D-26r21 and D-16 remain unchanged. No new source result type, selection rule, capture policy, ABI, regional representation or qualification; full thrown-value provenance and broader correlations remain unproved. Baseline: `7f892609d2ced3334c0d8bfa6f6cf885dc8cf3a6` at affected paths.

## 2026-11-15 — Exception result read timing

<a id="k-729"></a>
**K-729 — Distinguish saved values from selected-target reads.** Agent continuity assessment under H-ASSIGNMENT-RESULT-20261115/H-267/H-272/H-313/H-356/H-361/H-362/H-366–H-369/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source345→346 and registry739→740 preserve C-63/P-235/P-236/P-280's checked-failure/cleanup contract alongside C-65/P-255/P-257/P-260/P-327's target-only assignment, independent values and access rules. Assignment-result category evidence follows the selected target after RHS cleanup; ordinary value results keep their saved payload. Delayed immutable owned values retain known facts, while readonly borrowed paths do not imply immutability. Acceptance covers cleanup mutation, rebinding/nulling of source slots, saved and caught copies, checked recovery, shared recursion and exact native release. D-14r112/D-17r69/D-18r104/D-26r21, D-16 and other qualification boundaries remain unchanged. No new result-summary policy, storage representation, ABI or lifetime rule. Baseline: `fded4e5a6155296d1978b051db299893e08d3144` at affected paths.

## 2026-11-15 — Borrowed exception payload evidence

<a id="k-728"></a>
**K-728 — Invalidate category facts across possible target aliases.** Agent continuity assessment under H-267/H-272/H-313/H-327/H-366–H-369/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source344→345 and registry738→739 preserve C-63/P-235/P-236/P-280's checked-failure boundary alongside C-65/C-75/P-255/P-257/P-295/P-327's borrowed targets, independent values and access rules. A write through one borrowed input cannot leave another potentially aliased input with stale exception-category evidence. Distinct formal owners and readonly paths do not prove separation; definite destinations and independent copies retain their existing facts. Acceptance covers direct/prepared writes, swaps, calls, cleanup, checked recovery, shared recursion and native resource accounting. D-14r112/D-17r69/D-18r104/D-26r21, D-16 and other qualification boundaries remain unchanged. No new exception policy, owner representation, ABI or lifetime rule. Baseline: `03bca3d53135681a28092cc1df09d092354d7b9f` at affected paths.

## 2026-11-15 — Alias selector completion

<a id="k-727"></a>
**K-727 — Preserve exits before alias selection.** Agent continuity assessment under H-327/H-361/H-362/H-398/H-407/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source343→344 and registry737→738 preserve C-65/C-73/C-75/P-295's ordinary-storage alias contract and the established D-17/D-24/D-26 completion, independent-result and native-coverage scopes. Selectors that return or throw before producing storage create no alias, retain cleanup and contribute no unreachable result anchor. Acceptance covers normal-path kind/type/owner/access checks, annotation/owner names, explicit declaration boundaries, complete-image failure cleanup and shared recursion across equivalent alias spellings. D-14r112/D-17r69/D-18r104/D-24r8/D-26r21, D-16 and other qualification boundaries remain unchanged. No new capture, inference, storage, ABI or lifetime policy. Baseline: `ccf75d9b215c16823dda51539bc95e0fc8e871e1` at affected paths.

## 2026-11-15 — Declared builtin input evidence

<a id="k-726"></a>
**K-726 — Reuse independent signatures during builtin preparation.** Agent continuity assessment under H-258/H-261/H-272/H-304/H-356/H-361/H-362/H-398/H-407/H-420/H-423, K-658/K-699 and charter §§3.3/5.3/6.3/8/12: source342→343 and registry736→737 preserve declared signature obligations and ordered completion. C-10/C-63/C-89/C-90 and existing D-10/D-17/D-18/D-24/D-26 paths support using consistent declared value-input types for definite eager builtins whose operands return or throw. Other independently known operands may determine generic inputs; declarations remain constraints, without selecting methods, retargeting preparation or manufacturing runtime values. Acceptance preserves arity/known-type/result checks, normal-path checked failures, earlier effects and complete-value cleanup; aborted invocation/registration adds no callee failure or action. Declared and undeclared forms retain equivalent native behavior and shared generic coverage. Missing/conflicting inputs, borrowed-only evidence, broader generic/aggregate defaults, D-16 and other design/qualification boundaries remain unchanged. No new source policy, inference authority, ABI or storage representation. Baseline: `97ae90dc9963410e4b710735c5680345c35e9880` at affected paths.

## 2026-11-15 — Composed null-test evidence

<a id="k-725"></a>
**K-725 — Preserve boolean result proofs across expression composition.** Agent continuity assessment under H-317/P-284, H-439–H-441/P-391–P-392, H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source341→342 and registry735→736 preserve C-65/C-95's stable null tests and the established D-17/D-18/D-26 completion/native contracts. Checked expression evidence composes builtin negation and short-circuit operations, forwards block results and joins conditional/guarded alternatives; binding versions invalidate stale tests after mutation or cleanup. Acceptance covers both truth paths, abrupt operands, selected builtin identity, nullable/initialized/readonly/lifetime boundaries, loop/defer use and shared generic recursion without runtime evidence allocation. D-14r112, D-17r69, D-18r104, D-26r21, D-16 and other design revisions/qualification boundaries remain unchanged. No assertion surface, stored/callee predicate contract, owner representation, ABI or capture policy is selected. Baseline: `892c2254273d479700ddfbff91af609fb961e068` at affected paths.

## 2026-11-15 — Recursive type-only signatures

<a id="k-724"></a>
**K-724 — Retain declaration evidence in required and nested queries.** Agent continuity assessment under H-421–H-424/H-423, H-331/H-342/H-381/K-680 and charter §§3.3/5.3/6.3/8/12: source340→341 and registry734→735 preserve D-24r8’s independent result/context contract and D-18r104’s established query-only lexical signatures. C-84/P-356–P-358 retains its scoped source policy and pending broader value path; C-65/C-73/C-75/P-297–P-298 retains definition identity and valid owner/access evidence. Required recursive queries use retained declaration context rather than body-local shadows; nested query-only definitions can infer results from independently established enclosing signatures. Acceptance covers captured types, nearest-name/module identity, generic context, access/owner relationships, both branch orders, unanchored-cycle rejection and shared native recursion with no query-time effects or environment allocation. Existing runtime capture restrictions and the unresolved recursive self-value capture-time decision remain. D-24/D-18/D-26 revisions, D-16 and other qualification boundaries are unchanged; no new ABI, capture policy, owner representation or inference rule. Baseline: `8f73bd07f38c55f31f7d6bd3f4d97d29f512032a` at affected paths.

## 2026-11-15 — Recursive declaration evidence

<a id="k-723"></a>
**K-723 — Preserve definition context during recursive result inference.** Agent continuity assessment under H-421–H-424/H-423 and charter §§3.3/5.3/6.3/8/12: D-24r7→8’s independently confirmed result/context scope, source339→340 and registry733→734 preserve a callable’s definition-site evidence across body-local shadows. C-84/P-356–P-358 retains its existing source-policy scope and pending broader value path. New nested definitions retain their own visible context; query-only evidence creates no runtime capture, initialization or lifetime extension. Acceptance covers branch order, kind, captures, symbols, generic/nested contexts, independent anchors and stable shared native recursion. D-16 and other design revisions remain unchanged. Whole-document qualification, recursive self-value capture-time semantics, defaults and broader representations remain separate; no new capture, ABI or inference policy is selected. Baseline: `595a061f0b636d4a539175a0abaa5f517166d8b3` at the affected document paths.

## 2026-11-15 — Default selection and inference integration

<a id="k-722"></a>
**K-722 — Preserve signature choice before default analysis.** Integrate H-475–H-477/P-403–P-406 from `a0326fc2a5fb3daa83374f2236737c2dbb7d238f` at this file and the D-10/registry paths. D-10r169/r171→172/source338→339/registry731/r732→733 preserves C-10/C-57/C-89's candidate paths under charter §§3.1/3.3/5/6.3/8/12: explicit caller intent, typed signature selection and separate analysis/evaluation phases. Crossed fixed/default preferences reject; explicit named-default and contextually typed empty-pack requests preserve existing binding rules. Analyze only the uniquely selected overload's needed defaults, allowing independently established types to complete inference without candidate filtering, retry or execution. Owner/access/init, default phase/cleanup and qualified compiler contracts remain; physical context transport and full operational/value-path qualification are separate. Current compiler support remains unchanged and does not include these call forms. Mainline baseline: `bb29b260b68351f23c7896e6da192c5e15559cde` at affected paths. Acceptance preserves both decision histories, scoped qualification and current capability limits; D-16 remains unchanged.

## 2026-11-15 — Select the overload before analyzing defaults

<a id="h-477"></a>
**H-477 — Default analysis follows unique overload selection.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer to `call_5738fe9d510e4bb58d25f64704cdffbd`, item 0: “default expr 只有在选定 overload 以后才分析，这个例子报ambiguous错”. Selects ambiguity for the equally matched choose example before checking its unavailable caller-local fallback. More generally, default-expression semantic analysis occurs only after selecting the overload. This clarifies H-476's inference phase: selected defaults may complete generic inference, but their contents cannot supply candidate-selection evidence.

<a id="k-post-selection-defaults-20261115"></a>
**K-POST-SELECTION-DEFAULTS-20261115 — Separate signature choice, default checking and execution.** Agent integration under H-477: D-10r170→171 and registry r731→732 update D-10-ARGUMENTS/P-405/P-406. The candidate C-10/C-57 path preserves charter §§3.1/3.3/5/6.3/8: inspectable declared choice and distinct analysis/evaluation phases. Binding and ranking use signatures, supplied evidence and default-presence/count metadata; only selected needed defaults undergo semantic analysis, including H-476 inference. Retain definition/caller contexts, origin identities, unique-owner/access/effect obligations and H-458 execution timing. Ambiguity precedes default checks; an invalid selected default rejects without another candidate being tried. Acceptance includes generic seed inference after unique selection, the ambiguous/missing-name/explicit-valid choose cases, and no inspection of excluded defaults. Physical context/solver mechanisms remain pending; D-16 and qualified downstream scopes are unchanged. Published baseline: `6553709a47380c5bb657d738570d6011543dc253` at affected paths; H-475/H-476 records remain intact, with any candidate-default-evidence interpretation narrowed by this event. Documentation only; no build/tests or implementation claim.

## 2026-11-15 — Generic inference from established default types

<a id="h-476"></a>
**H-476 — Used defaults may provide type-inference evidence.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer to `call_db625733bdb341a18e25e1f98217914c`, item 0: “Infer from established default types (Recommended)”. Accepts `seed()` and `seed(-> x)` for `func seed(x:T { 0u64 }) -> T where {T}` with T=u64, based on established default-expression types without executing defaults during overload selection; circular or unresolved inference rejects. Does not select a new solver, runtime value probing or handling of ill-formed defaults in competing candidates.

<a id="k-default-type-evidence-20261115"></a>
**K-DEFAULT-TYPE-EVIDENCE-20261115 — Preserve initialization, context and execution boundaries.** Agent integration under H-476: D-10r169→170 and registry r730→731 update D-10-ARGUMENTS/P-405/P-406. Candidate C-10/C-57 path serves charter §§3.1/3.3/5/6.3/8 through typed initialization and stage/effect separation. Used default evidence combines with existing generic constraints; skipped defaults supply no call-specific inference, contradictions/unanchored cycles reject, and helper result types need no invocation. Preserve definition/caller lookup, typeof origin, default count, H-475 ambiguity, ownership and phase/cleanup boundaries. D-24 result evidence and D-26 coverage are dependencies, not borrowed design authority; D-16 is unchanged. Acceptance is recorded in D-10; invalid-default candidate diagnostics, physical context transport and solver mechanisms remain open. Published baseline: `6553709a47380c5bb657d738570d6011543dc253` at affected paths; H-475's local record remains intact. Documentation only; no build/tests or implementation claim.

## 2026-11-15 — Explicit choice between defaults and variadic packs

<a id="h-475"></a>
**H-475 — Reject crossed preferences and let the caller select the form.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer to `call_ba3e4f7efb0743be8a472fc0b50daab4`, item 0, comparing an equally matched fixed signature using one default with a variadic signature using none: “这个case reject，要求caller选择 `pick(7, ->y)` 或者 `pick(7,[]...)`”. Rejects both proposed total priority orders for this crossing case, selects ambiguity, and supplies explicit named-default and empty-pack call forms. This does not reject H-450/H-474's unopposed cases or select general array-literal storage/representation.

<a id="k-explicit-default-selection-20261115"></a>
**K-EXPLICIT-DEFAULT-SELECTION-20261115 — Integrate explicit selection with existing binding and phases.** Agent integration under H-475: D-10r168→169 and registry r729→730 update D-10-ARGUMENTS/D-10-VARARGS and P-403–P-406. Candidate C-10/C-57/C-89 value paths retain charter §§3.1/3.3/6.3/8/12: typed caller intent, runtime count independence and preserved effects. Type ranking remains first; unopposed fixed/default preferences continue, crossed preferences reject. `-> parameter` requires the declaration/origin default, occupies one named slot, counts as one used default and runs in the existing default phase; `[]...` selects pack form with established element/owner context and cannot invent missing evidence. Acceptance includes both explicit forms, ambiguity, duplicate/required/unknown-slot rejection, named ordering, default counts and phase/cleanup preservation. Other prefix, generic inference, context transport and pack representation questions remain pending; no broader qualification, D-16 change or compiler support is claimed. Published baseline: `6553709a47380c5bb657d738570d6011543dc253` at affected paths. Documentation only; no build/tests.
## 2026-11-15 — Default-ranking contract integration

<a id="k-721"></a>
**K-721 — Preserve overload intent and current implementation boundaries.** Integrate H-474/P-405–P-406 from `6553709a47380c5bb657d738570d6011543dc253` at this file and the D-10/registry paths. D-10r168→169/source337→338/registry730→731 preserves C-10/C-57's candidate value path through inspectable binding and observable effect order under charter §§3.1/3.3/6.3/8/12. Ordinary type ranking precedes fewer omitted default slots; equal remaining candidates are ambiguous, selection precedes default evaluation, and callable views use the origin contract. The crossing preference with implicit variadic collection, physical context transport and full operational/value-path qualification remain pending. Current canonical failures, root handlers and all qualified compiler scopes continue unchanged; this merge claims no default-argument implementation. Mainline baseline: `aa7633b055c95966dbe1b961916ae48f7fc6707c` at affected paths. Acceptance preserves both human histories, current capability coverage, scoped qualifications and explicit open boundaries; D-16 is unchanged.

## 2026-11-15 — Default-count overload priority

<a id="h-474"></a>
**H-474 — Type matching first, fewer defaults on a tie, then ambiguity.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “同意，类型匹配优先，平局时优先选择使用更少默认值的候选，仍然平局就报歧义。落文档，提交推送。 然后看看剩余议题，是不是适合先跟开发主线同步一次？”. Confirms the proposed default-ranking rule and requests publication and a synchronization assessment. It does not separately decide the crossing case with H-450's fixed-versus-implicit-pack preference.

<a id="k-default-ranking-20261115"></a>
**K-DEFAULT-RANKING-20261115 — Count omitted defaults after ordinary type ranking.** Agent integration under H-474: D-10r167→168 and registry r728→729 update D-10-ARGUMENTS/P-405–P-406 and the bounded variadic interaction. Candidate C-10/C-57 path serves charter §§3.1/3.3/6.3/8 through declaration-directed choice and preserved effect order. Count used formal defaults after binding, equally across definition/caller forms; keep exact-before-widening, reject residual ambiguity and never evaluate defaults to choose candidates. Callable views use origin defaults, not target-local defaults. Acceptance includes zero/one-default ties, an exact match with a default beating widening without one, equal-count ambiguity and losing-default nonexecution. Preserve H-450 and mark its crossing preference with default count pending instead of silently ordering them. D-16, ownership/cleanup and qualification boundaries remain unchanged. Published baseline: `019a71071ff8d1b0325872ad0a512b82bb3727d7` at affected paths. Documentation only; no compiler/build/tests.

## 2026-11-15 — Canonical failures and common-root catches

<a id="k-720"></a>
**K-720 — Keep names, identity and handling coherent.** Under H-472/H-473/H-423, D-10r167→168/D-14r111→112/D-15r23→24/D-17r68→69/D-18r103→104/D-21r14→15/D-25r16→17/D-26r20→21/D-28r21→22/source336→337/registry729→730 continues the existing qualified failure/native contracts through C-63/P-235–P-236 and P-414–P-415’s scoped naming direction. Compiler vocabulary, generated services and current fixtures use canonical Error/Exception names and common root Exception without compatibility aliases; category IDs, checkedness and payloads remain. Explicit root handlers, including aliases and generic substitutions, share ordinary catch-all matching and cleanup. Descriptor invariants prevent naming/classification drift. Charter §§3.1/3.3/4.3/6.3/8/12 and D-16 remain intact. No predicate/type assertion execution, new hierarchy, ABI, regional support or pending value-path qualification follows. Baseline: `46ca9635dceafac7bbcdbd9c634962e276a66e1a` at affected paths; current acceptance covers aliases, obligations, causes, handler ordering and native failure cleanup.

## 2026-11-15 — Assertion and failure-name contract integration

<a id="k-719"></a>
**K-719 — Preserve scoped decisions and mainline capabilities.** Integrate H-467–H-473/P-411–P-415 from `019a71071ff8d1b0325872ad0a512b82bb3727d7` at this file and its design paths. D-10r167/D-14r111/D-15r23/D-17r68/D-18r103/D-21r14/D-25r16/D-26r20/D-28r21/source335→336/registry727→729 combines assertion/naming decisions with mainline target-only assignment, bounds, selected calls and complete-value receiving. Charter §§3.1/3.3/4.3/5.3/6.3/8/12 retains semantic identities, effect order, owner/access/init and native coverage; D-16 remains unchanged. Canonical names preserve existing checkedness, and assertion failures retain author responsibility. Complete assertion/value-path qualification and physical mechanisms remain pending; compiler spelling migration is not claimed. Correct D-17's source-status table to include the existing cause_at bounds category. Both histories remain intact; no build/tests or wider design qualification.

## 2026-11-15 — Callable target/value receiving

<a id="k-718"></a>
**K-718 — Share complete callable reception and cleanup.** D-18r101→102/D-23r15→16/source334→335/registry726→727 continues H-342/H-344/H-417/H-423 through P-300–P-303 and the established receiving dependencies. Runtime callable ptr alternatives retain an existing target or construct a complete independent lexical value under one proven owner; fresh-image failure participates in ordinary completion. Charter §§3.3/4.3/5.3/6.3/8/12 preserves identity, captures, checked initialization and exact release through shared lowering. D-14r110/D-17r67/D-21r13/D-26r19 and D-16 remain unchanged; no new result classification, regional storage, lifetime extension, ABI or broader qualification. Current source/native acceptance covers both modes, recursive sharing and failed construction/copy cleanup. Baseline: `86bacdf826be4d73603b83d55cdbcf139dc55864` at affected paths.

## 2026-11-15 — Numeric conversion contract integration

<a id="k-717"></a>
**K-717 — Preserve the explicit numeric-conversion boundary.** Integrate committed H-463–H-466 from `34aff090d5a92ab28346dd63a9189776bfd9770d` at this file and the D-10/D-11/registry paths, retaining all decision evidence. D-10r164/D-11r34→35/source333→334/registry725→726 consistently distinguish proof-sensitive warnings and opt-in errors from defined exact results and undefined invalid execution, without runtime enforcement or a specified lossy output. Charter §§3.1/4.3/6.3/8/12, H-261 and D-16 remain unchanged; candidate value-path and broader expression-typing qualification stays pending. D-14r110/D-17r67/D-18r101/D-21r13/D-26r19 and existing compiler support continue unchanged. Editorial acceptance alignment grants no implementation authority. Documentation only; no build or test execution.

## 2026-11-15 — Callable payloads through caller slots

<a id="k-716"></a>
**K-716 — Share callable invocation evidence across target and binding arguments.** Agent continuity under H-306/H-342/H-344/H-356/H-407/H-423 and charter §§3.3/4.3/6.3/8/12: source332→333, registry724→725, D-14r109→110/D-17r66→67/D-18r100→101 retain C-65/C-69/P-268–P-269/P-301–P-303 and C-63/P-235–P-236/P-321–P-324. Baseline: `893bf526ca61be55327adfc47d48eaf61b5231a6` at affected paths. Direct/prepared reference-swap methods admit complete callable holders through shared borrowed-target evidence and finite callable flow. Acceptance preserves post-operand/invocation-time targets, selected captures, independent copies, recursive code sharing, owner/access/init checks and exact failure cleanup. The D-18 index retains the integrated H-459–H-462 scope at its current revision. Caller-binding method values, wider permission/alias proofs, default/context mechanisms, regional representation and fractions retain their separate boundaries; no new ABI or whole-file qualification follows. Process evidence remains outside the repository.

## 2026-11-15 — Callable contract integration

<a id="k-715"></a>
**K-715 — Preserve selected contracts and current compiler boundaries.** Agent continuity integrates `452b5a8dceec08d10f5cabac55d5c4743732afb6` at CHANGELOG.md and design/{SymbolicDynamicScope,RegionalClosures,AheadOfTimeExecutionPreparation,ExceptionControl,DesignAuthorityRegistry}.md with main `12847ad8260bec6ddc7295416e65a62ec2ede34e` at those paths and tests/cod/language/README.md. H-458–H-462/P-405–P-406/P-409 retains caller-before-callee defaults, named origin, type-bound default context and complete family compatibility under charter §§3.1/3.3/4.3/5/6.3/8/12. Source331→332/registry723→724/D-17r65→66 preserves bounds, assignment/receiving and current native coverage alongside D-10r161/D-18r100/D-21r13. Full argument/value-path qualification, default/context transport, local-family grammar and other open mechanisms remain pending. Acceptance requires coherent current scopes, recoverable human evidence and accurate implementation boundaries; no new compiler capability, regional representation or fraction rule follows.
## 2026-11-15 — Built-in failure names and assertion responsibility

<a id="h-472"></a>
**H-472 — AssertionFailedError and concrete checkedness suffixes.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “对，assert 失败导致运行时出错，是程序作者的责任，不归为 CoD soundness 缺陷。 然后 `AssertionFailedError` 作为 unchecked exception 吧，然后CoD约定内置错误 unchecked 都以 `Error` 结尾，checked 都以 `Exception` 结尾，看看怎么样”. Selects author responsibility for failed assertions, the unchecked predicate-failure name and the built-in suffix direction; requests assessment. Does not reclassify existing categories, mandate user-defined/alias spellings or select a representation.

<a id="h-473"></a>
**H-473 — Exception is the general exception-handling root.** Same human/source/date; asynchronous answer to `call_34650142f59546258c57328e45861979`, item 0, asking about the existing common SystemException type: “改为 `Exception` 如何？ 最general的对应exception handling概念”. Selects Exception for the common root rather than the suggested SystemFailure. Its scope is the general handling abstraction over admitted checked/unchecked categories, not a new concrete checked failure or broader user-exception implementation.

<a id="k-failure-names-20261115"></a>
**K-FAILURE-NAMES-20261115 — Align canonical names without changing failure contracts.** Agent integration under H-472/H-473: D-10r166→167/D-14r104→105/D-15r22→23/D-17r66→67/D-18r100→101/D-21r13→14/D-25r15→16/D-26r17→18/D-28r20→21 and registry r727→728 add D-17-NAMES/P-414/P-415. Candidate C-63/C-10 value path serves charter §§3.1/6.3/8; naming-only continuations preserve already qualified C-63/C-90 and dependent contracts, while pending scopes remain pending. Checkedness follows semantic type identity; the general root Exception spans both classes. Align concrete names, distinguish author assertions from UnsoundRuntimeTypingError, and retain static resolution, checks, causes, cleanup, native coverage and D-16. D-17 separately records currently implemented spellings; compiler/source/tests are unchanged, new spellings are not advertised as supported, and no compatibility alias is selected. Acceptance covers name/class agreement, alias obligations, common-root matching and unchanged semantics. Published baseline: `34aff090d5a92ab28346dd63a9189776bfd9770d` at affected paths; pending H-467–H-471 records remain intact. Documentation only; no build/tests.

## 2026-11-15 — Concrete-type specialization and explicit predicate assertions

<a id="h-468"></a>
**H-468 — Reserve warning-only conversion for purely numeric types.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Asynchronous answer to `call_43d2a24e3cdd413baee977d6ea18d492`, item 0: “只有纯numeric类型适用仅编译warning规则，只要涉及了其它类型，全都runtime抛错”. Retains H-466 only for pure numeric conversion; other admitted assertion mismatches retain runtime checking, subject to H-470 static resolution. H-471 clarifies generic/concrete selection.

<a id="h-469"></a>
**H-469 — Establish non-nullness through an explicit predicate assertion.** Same human/source/date, item 1: “配合 `assert p is not null` 额外赋予 non-null 性质。 `assert <predict> [, "err txt"]` 语法如果还没有要规划上”. Selects separate non-null refinement and requests predicate assertion syntax with optional quoted error text. Does not select hidden ptr-to-ref checking inside `:`, arbitrary message expressions or a physical diagnostic representation.

<a id="h-470"></a>
**H-470 — Resolve provable assertions statically.** Same human/source/date, item 2, concerning a provably false nonnumeric type assertion: “能静态证明的就不留到运行时，这个原则不学Julia”. Chooses compile-time rejection for proved failure and elimination of redundant validation for proved success. Observable operand effects remain governed by the charter; the separately selected pure numeric warning/UB policy remains.

<a id="h-471"></a>
**H-471 — Choose using concrete types at specialization.** Same human/source/date; answer to follow-up `call_b02343772a7b451ca941a832832f2dda`, item 0: “这个在 specialization 口径上处理，根据concrete type是否是umeric，有没有技术困难？”. Directs generic operation selection through concrete types and requests a feasibility assessment. A numeric concrete pair uses numeric conversion even through a generic wrapper. This does not request per-type native bodies, a new JIT or an implementation in this documentation task.

<a id="k-assert-specialization-20261115"></a>
**K-ASSERT-SPECIALIZATION-20261115 — Integrate static resolution, refinement and prepared selection.** Agent integration under H-468–H-471: D-10r165→166/D-14r103→104/D-17r65→66 and registry r726→727 update D-10-TYPED-EXPR/P-407–P-411 and add D-10-ASSERT/P-412–P-413. Candidate C-10/C-65 paths and qualified C-63/C-90 contracts preserve charter §§3.1/3.3/4.3/6.3/8/12: bool assertions establish sound success-path facts without removing effects or granting owner/access/init privileges. Static failure rejects; runtime checks remain enforced when unresolved, with ordinary unchecked failure/cleanup and no facts on handled-failure paths. Concrete numeric selection fits D-26 local dispatch/native sharing; preparing a possible failure branch does not prove a call fails. Source inspection finds numeric classification and native widening, but no assert construct or explicit colon execution; new lossy paths, parsing, diagnostics and proof integration remain. Optional text is initially a diagnostic literal; AssertionFailed is a working category. Preserve H-466, D-16, ordinary NPE and all qualification boundaries. Published baseline: `34aff090d5a92ab28346dd63a9189776bfd9770d` at affected paths; H-467 review evidence remains intact. Documentation only; no build/tests.

## 2026-11-15 — Unchecked nonnumeric type assertions

<a id="h-467"></a>
**H-467 — Throw an unchecked error for nonnumeric runtime assertion mismatch; review Julia as reference.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “数值以外的 type assertion 运行时不匹配都抛错，unchecked吧。 跟 julia 语义核对一下细节，作为参考把edge cases尽量想周全些。 CoD语义上有疑义的地方跟我讨论”. Confirms unchecked runtime mismatch outside numeric conversion and requests comparative review, not wholesale adoption of Julia typing/conversion rules. Generic numeric selection, nullable-to-nonnull assertions and admission of statically false assertions are not yet settled.

<a id="k-type-assertion-review-20261115"></a>
**K-TYPE-ASSERTION-REVIEW-20261115 — Integrate mismatch and preserve separate typing obligations.** Agent integration: D-10r164→165/D-17r64→65 and registry r725→726 update D-10-TYPED-EXPR/P-407–P-408 and add P-411 under H-467. Candidate C-10 value path and existing C-63/C-90 contracts serve charter §§3.1/3.3/4.3/6.3/8/12 through inspectable membership and ordinary recovery without fabricated identity or permission. Preserve the tested target, actual dispatch evidence, owners, initialization, operand effects, H-458 phases and D-26 unknown-interpretation/soundness categories; runtime mismatch is neither UB nor a conversion probe. TypeAssertionFailed is a working name, not an implemented ABI. Official Julia documentation and v1.12.0 `src/builtins.c:588–596` support the comparison; no Julia interpreter was available. Three substantive choices and broader grammar/evidence mechanisms remain pending. H-466 numeric policy and D-16 stay unchanged; no new qualification, source changes, build or tests. Published baseline: `34aff090d5a92ab28346dd63a9189776bfd9770d` at affected paths.

## 2026-11-15 — Undefined behavior for invalid explicit numeric conversion

<a id="h-466"></a>
**H-466 — State undefined behavior explicitly; omit runtime enforcement.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “warning明确是 undefined behavior，后果自负，运行时就不要抛错了”. Confirms undefined behavior rather than a pending numeric output, while retaining the compile-time warning/opt-in-error direction. Replaces H-464's lossy output promises and the H-465/K-UNCHECKED-CONVERSION-20261115 interpretation excluding undefined behavior. This concerns explicit numeric conversion, not fraction arithmetic, ordinary arithmetic overflow or general type assertions.

<a id="k-numeric-conversion-ub-20261115"></a>
**K-NUMERIC-CONVERSION-UB-20261115 — Make the exactness precondition and undefined boundary consistent.** Agent integration under H-466: D-10r163→164/D-11r33→34 and registry r724→725 update D-10-TYPED-EXPR/P-407–P-408/P-410. Candidate C-10 value path under charter §§3.1/6.3/8 exposes programmer responsibility without mandatory runtime conversion checks. A warning about an unproved input is distinct from actual undefined execution; exactly representable actual inputs retain their value and ordinary evaluation rules. Remove specified lossy outputs, pending output-mapping questions and no-crash/nonthrowing guarantees from current contracts; undefined behavior is not confined to a numeric result. Preserve H-261, owner/access and D-16; D-17 gains no new exception. Acceptance covers diagnostics, defined-path results/effects and absence of inserted conversion enforcement, with no output/crash assertion for undefined cases. Published baseline: `452b5a8dceec08d10f5cabac55d5c4743732afb6` at affected paths; H-463–H-465 pending-checkpoint evidence remains intact. Documentation only; no compiler/build/tests or broader qualification.

## 2026-11-15 — No runtime error after numeric conversion warnings

<a id="h-465"></a>
**H-465 — Warn about programmer responsibility; do not reject the conversion at runtime.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. In response to whether `300.0f64 : u8` should wrap or fail, with NaN/infinity also raised, says “warning明确了后果自负以后，运行时就不要抛错了”. Selects no conversion-generated runtime error after warning-and-continue; the warning communicates the numerical risk. Does not choose exact outputs for remaining floating cases, blanket whole-program undefined behavior, assertion semantics or a fraction policy.

<a id="k-unchecked-conversion-20261115"></a>
**K-UNCHECKED-CONVERSION-20261115 — Close the runtime rejection alternative.** Agent integration under H-465: D-10r162→163/D-11r32→33 and registry r723→724 update D-10-TYPED-EXPR/P-407–P-408/P-410. Candidate C-10 value path and charter §§3.1/6.3/8 preserve visible responsibility and selected execution. No numeric-loss/range/non-finite conversion error is added in runtime or constant evaluation; opt-in warnings-as-errors remains compile-time policy. Preserve H-464's specified results, once-only operand effects and unrelated failures. Exact remaining outputs stay pending under the nonthrowing constraint, without importing backend traps or whole-program undefined behavior. H-261, owner/access, D-17 and D-16 remain unchanged; no new qualification or exception category. Published baseline: `452b5a8dceec08d10f5cabac55d5c4743732afb6` at affected paths; H-463/H-464's pending checkpoint records remain intact. Documentation only; no compiler/build/tests.

## 2026-11-15 — Explicit lossy numeric conversion

<a id="h-463"></a>
**H-463 — Warn at compile time, with opt-in errors.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. In response to explicit `:` numeric narrowing and fractional-loss examples, says “编译时抛警告，可以opt-in视为错误”. Selects warning severity and configurable error promotion; the initial statement alone does not determine proof coverage or numeric results.

<a id="h-464"></a>
**H-464 — Warn unless proven lossless; perform the lossy conversion.** Same human/source/date; asynchronous answers to `call_5285e998dde44db49682173619855a35`, items 0/1: “Warn unless proven lossless (Recommended)” for unknown `x:u64` converted to u8, and “Produce 44 and 3” for `300u64 : u8` and `3.5f64 : u64`, rather than failing a runtime exactness check. Does not select out-of-range floating conversion, NaN/infinity behavior, all signed/rounding cases, fraction conversion or assertion semantics.

<a id="k-lossy-conversion-20261115"></a>
**K-LOSSY-CONVERSION-20261115 — Separate explicit loss from implicit compatibility.** Agent integration under H-463/H-464: D-10r161→162/D-11r31→32 and registry r722→723 update D-10-TYPED-EXPR/P-407–P-408 and add P-410. The candidate C-10 value path serves charter §§3.1/6.3/8 through visible numerical intent and deterministic results; its broader qualification remains pending. Generalize the accepted examples to unsigned narrowing modulo destination width and finite float-to-int truncation toward zero when the truncated integer fits; keep other numeric domains/failures explicit as pending. Preserve H-261's whole-type implicit losslessness, units, once-only ordered evaluation, owner/access and D-16 constraints. Acceptance covers proof-sensitive warnings, opt-in errors and constant/runtime agreement; D-17 gains no new exception category. Published baseline: `452b5a8dceec08d10f5cabac55d5c4743732afb6` at affected paths. Documentation only; no compiler/build/tests.

## 2026-11-15 — Complete multi-method callable contracts

<a id="h-462"></a>
**H-462 — Implement every origin signature; permit caller-local named adapters.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says: “对了。 另外可以明确要求 `:typeof <some-multi-method-callable>` 要求参数实现所有origin的method签名。 传这个参数的 caller 可以本地定义 named callable 适配。” Confirms the type-bound default direction and requires complete implementation of a multi-method origin's signatures, with caller-local named adaptation allowed. This does not select exact local family syntax, generic coverage algorithms, type equality or a physical default-context carrier.

<a id="k-callable-family-contract-20261115"></a>
**K-CALLABLE-FAMILY-CONTRACT-20261115 — Keep interface completeness independent of use and code generation.** Agent integration under H-462: D-10r160→161/D-18r99→100/D-21r12→13 and registry r721→722 add P-409 and full-family acceptance. Charter §§3.1/3.3/6.3/8/12, C-57/C-89 and existing D-21/D-26 coverage preserve checked contracts and code sharing. Required signatures cannot be dropped because the receiver currently calls only one; check the actual selected method over required domains. Compatible generic coverage and additional methods remain possible without one native body per signature; ambiguous or incompatible interception rejects. Origin defaults/names remain in the type contract under H-461, with ordinary owner/effect/capture obligations on local adapters. Keep detailed local family grammar, default-overload applicability and proof/carrier mechanisms pending; do not infer arbitrary local variable redefinition or broader qualification. D-16 and the selected phases remain unchanged. Published baseline: `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` at affected paths; prior pending decision records are preserved. Documentation only; no compiler/build/tests.

## 2026-11-15 — Type-bound default expressions

<a id="h-461"></a>
**H-461 — The callable type retains the default expression and its definition context.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Corrects the proposed extra adapted-value capture: “`:typeof origin` 这个type应该capture在其定义context eval `fallback` 的表达式吧，而不是 `adapted` value 去capture”. Selects the type contract as the home of the definition-context default expression, without an independent capture in the adapted value. H-459 origin and H-458 phases continue; runtime physical representation, type-context equality/transport and lifetime proof mechanisms are not selected.

<a id="k-type-default-expression-20261115"></a>
**K-TYPE-DEFAULT-EXPRESSION-20261115 — Separate default-contract dependencies from body captures.** Agent correction under H-461: D-10r159→160/D-18r98→99 and registry r720→721 replace K-COMPATIBLE-DEFAULT-TARGETS-20261115's per-value provider-retention proposal in current docs. Charter §§3.1/3.3/4.3/5.1–5.2/6.3/8 separates semantic type level from evaluation stage while preserving origin, effects and valid runtime references. Retain the checked default expression/context association; evaluate only on omission, reading the original alias target's current contents rather than snapshotting bits or retargeting through a changed index. The target keeps its ordinary body captures; type/default context evidence remains an independent owner/access/lifetime obligation. Physical transport and overloaded/type-equality details remain scoped review work, with no new ABI, hidden lifetime extension or whole-design qualification. D-16 and unrelated qualified contracts continue. Published baseline: `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` at affected paths; earlier pending decision records are preserved. Documentation only; no compiler/build/tests.

## 2026-11-15 — Compatible callable targets and default-provider review

<a id="h-460"></a>
**H-460 — Allow compatible identities, subject to concrete quirk review.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Replies “我觉得应该允许，但你再多想想有没有quirks，没有就这样定，有的话继续讨论” to allowing run(other) through typeof original while retaining original's defaults. Authorizes settling straightforward compatibility and asks for discussion of substantive issues. D-10 records the ordinary case with established default dependencies; additional capture-bearing default-provider transport is a pending issue, not assumed human approval.

<a id="k-compatible-default-targets-20261115"></a>
**K-COMPATIBLE-DEFAULT-TARGETS-20261115 — Separate target body from inherited default contract.** Agent review under H-460: D-10r158→159/D-18r97→98 and registry r719→720 distinguish the origin's public names/defaults from the compatible target's body. Preserve complete signature/effect/owner/init obligations, slot mapping, once-only preamble/cleanup and D-16 under charter §§3.1/3.3/4.3/6.3/8. Other's own defaults do not run for supplied inherited arguments. Review identifies a concrete runtime-provider gap: typeof plus shared owner evidence does not supply origin's captured regional locator when only another target environment is retained. Propose separate default-provider retention during adapted-view construction, pending human decision; missing provider proof cannot be accepted. Existing qualified capture/owner contracts continue, without selecting a new carrier or widening pending implementation scope. Published baseline: `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` at affected paths; prior uncommitted phase/origin records remain. Documentation only; no build/tests or implementation.

## 2026-11-15 — Named callable origin for default expressions

<a id="h-459"></a>
**H-459 — Author defaults on named declarations and inherit them through typeof.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Responding to an inline callable-view default proposal, says: “我感觉最好是仅允许 (symbolicly) named callable 声明参数默认表达式，作为参数类型的 callable type 声明，最多允许通过 `typeof <some-named-callable>` 声明默认值，且按 named callable 的 definition site 来”. Selects named-declaration default authorship and typeof-based reuse at callable parameter type sites, retaining the named declaration's definition origin; inline callable types cannot invent defaults. This does not choose cross-identity target compatibility, overloaded-origin selection or a runtime representation. H-454–H-458 still governs each block form's evaluation environment and phase.

<a id="k-named-default-origin-20261115"></a>
**K-NAMED-DEFAULT-ORIGIN-20261115 — Retain default provenance without new authoring sites.** Agent integration under H-459: D-10r157→158/D-18r96→97 and registry r718→719 update D-10-ARGUMENTS/P-405–P-406 and the callable-view boundary. Charter §§3.1/3.3/4.3/6.3/8 and existing C-65/C-73/C-75 capture/owner paths preserve inspectable declaration identity and valid environments; candidate value paths and complete mechanisms retain their qualification limits. Unevaluated typeof does not execute defaults or rebind their definition site; caller-context blocks still use the actual omitted-call context and original symbolic identities. Plain structural views do not recover erased defaults through optimization; inherited captures must remain valid. D-16 and H-458 entry/cleanup phases remain unchanged. Published baseline: `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` at affected paths; H-458's phase record remains in this pending checkpoint. Documentation only; no compiler/build/tests or ABI choice.

## 2026-11-15 — Default evaluation phases

<a id="h-458"></a>
**H-458 — Caller preparation precedes callee parameter initialization.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Replies “同意。 下一个议题” to the concrete proposal: prepare explicit arguments, then caller-context defaults; enter the callee and initialize parameters in declaration order, evaluating definition-environment defaults as needed. Confirms the mixed-default example prints 2 then 1, with caller-default failure before input ownership transfer and callee-default failure after entry receiving ordinary cleanup. This settles the phase choice left open by H-454/H-455, not remaining declaration/view/typing mechanisms.

<a id="k-default-phase-20261115"></a>
**K-DEFAULT-PHASE-20261115 — Integrate phase ordering with existing cleanup.** Agent integration under H-458: D-10r156→157/D-17r63→64 and registry r717→718 update D-10-ARGUMENTS/P-405–P-406 and D-17's qualified entry/cleanup contract under C-63/P-235/P-236 and charter §§3.3/6.3/12. Remove the unselected cross-phase alternative from current docs; preserve initializer lookup, value/borrow/init modes, stable symbolic identity, nontransactional effects and D-16. Existing deferred preparation rules place caller-defaults at registration and callee-defaults at invocation, with unchanged entry ownership transfer and lifetime obligations. Broader grammar, declaration/view and expression-typing mechanisms retain their qualification boundaries. Baseline: `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` at affected paths. Documentation only; implementation and proof coverage remain outstanding, with no build/tests run.
## 2026-11-15 — Scalar target/value receiving

<a id="k-714"></a>
**K-714 — Preserve receiving identity through runtime alternatives.** Agent continuity under H-416/H-417/H-420/H-423 and charter §§3.3/4.3/5.3/6.3/8/12: source reference330→331, registry722→723, D-14r108→109 and D-23r14→15 retain C-65/P-255/P-257/P-258 and the scoped receiving matrix. Baseline: `782b0a94e52118dffed9f16adc211446f893a99f` at affected paths. Scalar initializer alternatives retain either an existing target or evaluated bits through cleanup, with materialization in the receiving full scope and one provable owner. Shared initialization/completion preserves snapshots, access, identity, nullability and failure behavior, including abruptly completing initializers. Acceptance covers branch order, cleanup/deferred paths, owner/access/init rejection, shared recursive native code and unchanged resource behavior. No callable-mixture transport, general fresh borrow, external allocation, conditional widening, regional representation, fraction rule or whole-file qualification follows. Process evidence remains outside the repository.

## 2026-11-15 — Assignment queries and target-only results

<a id="k-713"></a>
**K-713 — Preserve target capabilities through non-executing inspection.** Agent continuity under H-295/H-ASSIGNMENT-RESULT-20261115/H-407/H-420/H-423 and charter §§3.3/4.3/6.3/8/12: D-14r107→108, source reference329→330 and registry721→722 align builtin assignment results and bounded queries with C-64/C-69/P-241/P-260/P-268–P-269. Baseline: `c59bb9eee13302431f54f47cb578f495c8992749` at affected paths, plus the scoped human resolution below. Shared destination/type/access checking preserves owner, initialization and cleanup contracts; results expose targets without slots. Queries execute no operands, copy no values and create no initialization facts. Acceptance includes right-associative chaining, target writes, rejected result-based slot operations, equivalent query/explicit-type native code and unchanged resource behavior. D-16 numeric boundaries and all pending mechanisms/whole-file qualifications remain. Process evidence stays outside the repository.

## 2026-11-15 — Assignment results retain only target views

<a id="h-assignment-result-20261115"></a>
**H-ASSIGNMENT-RESULT-20261115 — Target-only assignment results.** Human/source: current project user in [the compiler mainline chat](codex://threads/01a1cefe-fe25-7251-b25b-b3908d803ff6), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. In response to `call_0447e4a289f34da9ac07d1eec4304b71`, chooses “Target view only (Recommended)” after reviewing the assignment-result topic in `design/RegionalOwnership.md`. The question explicitly preserves `p = q = r` and `(p = q) <- value`, rejects `(p = q) = r`, and contrasts that rule with exposing p's slot for further swaps/aliases. This confirms the target-only option and rejects the builtin-slot alternative, resolving K-712 for D-14r106→107/P-241/P-260. Destination type/owner/access, ordinary slot aliases and non-executing queries continue unchanged; no new owner, copy, broader assignment-method ABI or whole-file qualification follows.

## 2026-11-15 — Assignment-result capability review

<a id="k-712"></a>
**K-712 — Separate target-view authority from binding-slot implementation.** Agent review under AGENTS and charter §§3.3/4.3/8: D-14r105→106 and registry719→720 identify a contract ambiguity against baseline `c59bb9eee13302431f54f47cb578f495c8992749` at `cod/lang_checked.hh`, `tests/cod/language/selected_slot_tests.py` and `design/RegionalOwnership.md`. H-270/H-302/P-241/P-260 confirms destination target identity/access and chaining; it does not clearly qualify the builtin result's retained lhs slot. The current design now exposes target-only versus builtin-slot alternatives and their consequences. This specific capability is pending-human, not qualified by existing code/tests or earlier agent continuity. Other assignment, alias, swap, query, owner/access and cleanup contracts continue; no human decision or compiler change is claimed.

## 2026-11-15 — Storage-exchange type inspection

<a id="k-711"></a>
**K-711 — Share destination contracts with non-executing queries.** Agent continuity under H-295/H-306/H-407/H-420/H-423 and charter §§3.3/4.3/6.3/8/12: source reference328→329, registry718→719 and D-14r104→105 preserve C-69/P-268–P-269 and bounded type inspection. Baseline: `04c58e4594f5e8afd00e6d8ff1d2b9c52b53a3f1` at affected paths. Declaration evidence retains fixed-slot and physical-access facts through aliases and nested contexts; selected swap signatures reuse destination/type/access and owner checks. Queries add no payload reads, effects, initialization guarantees, executable-body demands or runtime transport. Existing invocation/cleanup obligations and D-16 exact-bits constraints remain. No new language choice, ABI or document qualification; all pending design scopes retain their status. Current acceptance belongs to the source reference; process evidence stays outside the repository.

## 2026-11-15 — Argument design checkpoint integration

<a id="k-710"></a>
**K-710 — Preserve bounds coverage and integrate argument-design decisions.** Agent reconciliation under H-420/H-423/H-451–H-457 and charter §§3.1/3.3/4.3/6.3/8/12: merge design checkpoint `bfcd95cd76c59fdf446959b08b752f96bbe0bba5` with compiler checkpoint `40854b4157739c85d5c1681017c02a703bcaf671`. Registry r718 and source reference328 retain D-17r65/D-26r19 bounds capability while integrating D-10r156/D-18r96. Named arrows, default environments and expression-colon direction retain their recorded scope; default phase order, complete grammar, declaration/view integration, conversion/assertion semantics and candidate value paths remain pending. Both histories, owner/access/cleanup constraints and D-16 deferral remain intact. This merge changes documentation only; it adds no source capability or qualification.

## 2026-11-15 — Catchable source bounds failures

<a id="k-709"></a>
**K-709 — Bounds policy through shared native completion.** Agent implementation/continuity under H-448/H-267/H-356/H-407/H-420/H-423 and charter §§2/4.3/6.3/8/12: source reference326→327, registry716→717, D-17r64→65 and D-26r18→19 preserve C-63/C-65/P-401–P-402 and the qualified native-coverage contract. Baseline: `973b381bc00e3b5cef6d087d59d7464a77c01da5` at affected paths. cause_at reports logical bounds failure as unchecked IndexOutOfBoundsException, preserving ordered preparation, shared cleanup and exception fields. --no-bounds-checks is scoped to the current whole-source artifact; generated calls retain the policy while internal integrity and unrelated checks remain. Requested index/available count preserve the existing exception payload layout without adding allocation. These bounded engineering choices continue H-448; broader indexing, mixed-artifact transport, packs, regional protocols and D-16 remain separate. No new qualification or pending language choice is inferred. Acceptance and current limitations are in the source reference; process evidence remains outside the repository.

## 2026-11-15 — Variadic design checkpoint integration

<a id="k-708"></a>
**K-708 — Preserve eager-alias coverage and integrate variadic decisions.** Agent reconciliation under H-420/H-423/H-449/H-450 and charter §§2/3.1/6.3/8/12: merge design checkpoint `8e999e58f72d1801f75ae0af621f0f48de12c84a` with compiler checkpoint `7c12d7ad133f44d4140dd40e0f28815db27326c7`. Registry r716 retains source reference326/D-26r18 and the checked operation/preparation capability; D-10r154 retains the scoped array-expansion and ranking decisions. Both decision histories, value/dependency relationships and exclusions remain intact. D-10-VARARGS/P-403 still requires value-path and operational qualification; P-404 grants no authority. Executable packs, broader composition and physical transport are not claimed implemented. Source/native evidence remains the compiler checkpoint's evidence; this merge changes documentation only.

## 2026-11-15 — Checked eager operation identity

<a id="k-707"></a>
**K-707 — Eager aliases through one checked operation path.** Agent implementation/continuity under H-148/H-154/H-155/H-226/H-230/H-232/H-261/H-356/H-407/H-420/H-423 and charter §§2/6.3/8/12: source reference325→326, registry714→715 and D-26r17→18 preserve selected meaning through ordered preparation, type evidence, null/constant proofs and native emission. Baseline: `0ca04137856a593c6a87f9f92eac13c1dd18f395` at affected paths. C-89/C-90/P-371–P-372/P-374–P-375 retains the qualified native-coverage/value contract; D-10/D-11/D-17 local operation/numeric/cleanup decisions remain scoped dependencies. Aliases do not create a hidden builtin for an ordinary family or change an eager family's operation. The registry governance row follows its current revision. No new language choice, conversion, failure policy, runtime ABI, regional protocol, fraction implementation or document qualification. Other descendants and open mechanisms retain their status. Current acceptance is in the source reference; process and validation records remain outside the repository.

## 2026-11-15 — Bounds design checkpoint integration

<a id="k-706"></a>
**K-706 — Preserve alias coverage and integrate the bounds policy.** Agent reconciliation under H-420/H-423/H-448 and charter §§2/4.3/6.3/8/12: merge design checkpoint `3965f1815cde29350959ba6dfa8d2c9275fe3772` with compiler checkpoint `e14cd85a953dcf79a432054a11922280ecadda09`. Registry r714 retains source reference325 and its verified alias capability. Concurrent D-14r103/D-17r63 revisions become D-14r104/D-17r64; D-10r152 retains the side checkpoint. H-448/P-401–P-402 qualifies only default bounds checking, unchecked failure and explicit opt-out, preserving other checks and existing cleanup when an exception is delivered. Both decision histories, qualified scopes and pending mechanisms remain intact. Current compiler bounds paths are not migrated, and broader executable packs remain under review. This merge changes documentation only; compiler evidence remains the alias checkpoint's evidence.

## 2026-11-15 — Selected operation aliases

<a id="k-705"></a>
**K-705 — Operation identity through shared destination preparation.** Agent implementation/continuity under H-155/H-226/H-230/H-232/H-306/H-356/H-420/H-423 and charter §§2/6.3/8/12: source reference325/registry713 admits selected swap aliases and imported extensions through the existing caller-slot/target-storage and cleanup paths. D-14r102→103/D-17r62→63 align current capability pointers; C-69/P-268–P-269 and C-63/P-321–P-324 retain their scoped contracts. Baseline: `57d108f116e54ce5b673452cb0aa646cc74396a6` at affected paths. Renaming preserves operation identity and selected interpretation; extension signatures retain exact destinations, explicit owners and void results. This changes no confirmed value, failure policy or parent relationship. Callable-value slot ABI, independent-family composition, eager-arithmetic fallback, regional allocation, fractions and purity remain outside this slice. Other document qualifications and exclusions remain unchanged. Acceptance and current bounds are in `tests/cod/language/README.md`; process and validation reports remain outside the repository.

## 2026-11-15 — Argument arrows and default environments

<a id="h-453"></a>
**H-453 — Reserve expression colon for explicit typing.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. Says “`:` 语法给默认值不妥，CoD应支持 "explicit type conversion"/"type assertion"，使用 `:` 语法。 named arg用 `=` 或者 `<-` 你看下有没有quirks”. Selects colon's conversion/assertion direction and excludes it as a named separator; asks for comparison of assignment-like alternatives, not their adoption. Detailed conversion/assertion behavior remains open.

<a id="h-454"></a>
**H-454 — Distinct definition-site and caller-context default blocks.** Same human/source/date as H-453. After proposing caller-context default blocks, refines the proposal: “哦， `{ ... }` 是在 callee 定义位置 eval，`{{ ... }}` 是在 caller context 开个scope eval吧。” Selects the two environment forms and an explicit caller scope. H-452's parameter-initialization preamble remains; the earlier agent inference that all defaults necessarily execute after callee entry does not settle the new caller form's phase boundary.

<a id="h-455"></a>
**H-455 — Per-omission evaluation and caller-local lookup.** Same human/source/date; asynchronous answers to `call_69f4afef10db4689b51593c49adcfa4a`, items 0/1. Chooses “Per omitted call, in the definition environment (Recommended)” for `{ tick() }`, and “Caller’s local x, yielding 2” when `{{ x }}` sees definition-site x=1 and caller-local x=2. No definition-time default cache is selected. These answers fix frequency and lookup; mixed default sequencing, entry/cleanup placement and other name categories remain separate.

<a id="h-456"></a>
**H-456 — Prefer fixed symbolic identity with caller-current values.** Same human/source/date. Says “但是推荐定义时在 {{ ... }} 内部只引用symbolic vars，symbol 身份在 callee definition site 解析”. Recommends symbolic-variable dependencies in caller-context defaults; resolves symbol identity at definition and reads that identity's caller-current binding. This is guidance rather than a ban on H-455's caller-local references; caller homonyms do not retarget an established symbol identity.

<a id="h-457"></a>
**H-457 — Expression-first named arrows with grouped owner values.** Same human/source/date. Proposes “那 named arg passing 考虑 `<expr> -> <arg-name>` 语法看看” and, after the existing owner-projection collision is identified, supplies “可以 `(<callable-var> -> <owner-name>)` 形式显式写参数值”. Selects that local argument direction with explicit grouping for the projected owner value. D-10/D-18 integrates outer named-arrow syntax without overload-dependent reinterpretation; borrowed region identity/access/lifetime remain. This does not authorize new scalar grouping behavior, ownership transfer or complete parser support.

<a id="k-argument-contexts-20261115"></a>
**K-ARGUMENT-CONTEXTS-20261115 — Integrate syntax and environment decisions without assuming phase order.** Agent integration of H-453–H-457: D-10r155→156/D-18r95→96 and registry r716→717 update D-10-ARGUMENTS/P-405–P-406, add D-10-TYPED-EXPR/P-407–P-408, and preserve confirmed owner projection through grouped argument values. Charter §§3.1/3.3/4.3/6.3/8/12 motivates readable binding, stable identity and preserved effects. Current docs replace colon-label illustrations and the blanket post-entry-default inference in K-NAMED-ARGUMENTS-20261115; the current mixed/default/deferred phase proposal remains pending. H-451/H-452's completeness and preamble, H-449/H-450, and D-16 remain unchanged. Candidate value paths and full grammar/operational scope are not self-qualified. The publication review distinguishes conversion from assertion and preserves once-only evaluation without selecting fallback, narrowing, failure or value/view mechanisms. Published baseline: `8e999e58f72d1801f75ae0af621f0f48de12c84a` at affected paths. Documentation only; no compiler/build/tests or native support claim.

## 2026-11-15 — Named arguments and callable parameter preamble

<a id="h-451"></a>
**H-451 — Complete declaration-directed named/default argument binding.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. After requesting the H-450 checkpoint, the user says: “然后继续讨论 arg default，named arg passing，omitting for default。当前好像还没有吧，加这个设计： 第一个命名传递的参数之前可以有位置参数，之后不允许；以声明签名为匹配结果，不全的reject，只要能给全，调用方可以乱序给参数。” Selects the named/default feature direction, positional prefix only, declaration-directed labels, arbitrary named order and complete required binding. Missing default-bearing slots use their declared defaults; missing required slots reject. Exact punctuation, default-count overload ranking and declaration/view integration are not selected by this statement.

<a id="h-452"></a>
**H-452 — Parameter initialization belongs to a callable preamble.** Same human/source/date as H-451. The user adds: “参考 initialization，心智模型是一个 callable 在 preamble block 中初始化所有参数”. Selects ordinary initialization in a callable preamble as the semantic model. D-10-ARGUMENTS integrates existing source-ordered argument preparation, initializer lookup, entry ownership transfer, callee effect context and exceptional cleanup; this is not approval of a new ABI, source block keyword or all remaining default/overload mechanisms.

<a id="k-named-arguments-20261115"></a>
**K-NAMED-ARGUMENTS-20261115 — Reuse initialization and preserve the call boundary.** Agent integration under H-451/H-452 and H-423's routine-decision delegation: D-10r154→155 and registry r715→716 add D-10-ARGUMENTS, candidate value edge P-405 and dependency P-406. Charter §§3.1/3.3/6.3/8/12 motivates readable typed slot binding with observable order preserved. Defaults run during parameter initialization after entry; explicit supplied expressions/conversions retain source order before entry. All supplied owned inputs transfer at entry and remain subject to exactly-once callee cleanup even if an earlier default fails before their named binding initializes. Ordinary initializer lookup, earlier-parameter visibility, function/procedure effects, nontransactional allocation and init obligations continue; plain saved-call defaults run on invocation. Duplicate/unknown/missing required bindings reject; no trailing-default restriction is added when named binding can complete the signature. Illustrative punctuation and remaining integration are pending, and the value path is not self-qualified. H-449/H-450 and D-16 remain unchanged. Baseline: `8e999e58f72d1801f75ae0af621f0f48de12c84a` at the affected design paths. Documentation only; no compiler/build/tests or implementation claim.

## 2026-11-15 — Fixed and variadic overload ranking

<a id="h-450"></a>
**H-450 — Type matching precedes the fixed-form tie-breaker.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact timestamp/personal name unavailable. The user replies “同意。提交一版” to the concrete proposal: fixed u64 beats an otherwise equally specific singleton u64 pack, while an exact u32 pack beats widening into fixed u64. Confirms type matching first and fixed-over-implicit-pack preference only on an otherwise equal match, within D-10-VARARGS; explicit expansion remains one pack. Does not choose differing-prefix variadic ranking or broader pack operations. The same message's new named/default-argument request is a separate design scope.

<a id="k-variadic-ranking-20261115"></a>
**K-VARIADIC-RANKING-20261115 — Integrate the confirmed overload tie-breaker.** Agent integration under H-450: D-10r153→154 and registry r714→715 update D-10-VARARGS/P-403 and acceptance while retaining H-449's element/owner/access/init, count-independent selection and D-16 boundaries. Candidate value path through C-57/C-89 and charter §§3.1/6.3/8/12 remains pending; no whole-file or operational qualification is inferred. Incomparable constraints remain ambiguous. Published baseline: `3965f1815cde29350959ba6dfa8d2c9275fe3772` at the affected design paths; H-449's decision is included in this checkpoint. Documentation only; no compiler/build/test changes.

## 2026-11-15 — Array expansion and one variadic parameter

<a id="h-449"></a>
**H-449 — Ordinary-array expansion with one logical overload input.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. In response to support for runtime-array expansion with ordinary value/borrow semantics and conservative sound implementation coverage, the user says: “同意。 不过 overload selection 应该把 vararg 当一个参数看待吧？” Selects that expansion direction and proposes the single-parameter model, adopted in D-10-VARARGS: element contracts participate in matching, while count is runtime data and does not select fixed arities. Fixed-versus-implicit-pack priority, differing prefixes, full element/preparation/escape contracts and physical representation are not settled by this statement.

<a id="k-variadic-pack-20261115"></a>
**K-VARIADIC-PACK-20261115 — Scoped variadic direction and contract boundaries.** Agent integration of H-449 with H-258/P-229: D-10r152→153 and registry r713→714 add the scoped section, candidate value path P-403 and dependency edge P-404. Charter §§3.1/6.3/8/12 motivates inspectable typed calls and count-independent native semantics; the candidate path remains pending, without borrowing qualification from C-89/D-26 or qualifying all of D-10. Preserve exact-before-lossless numeric selection, actual owners/access/init, value independence, exception cleanup, regional no-copy/move and D-16 constraints. Distinguish bodyless variadic signature checking from executable pack calls; no existing prototype coverage proves array expansion. Baseline: `3965f1815cde29350959ba6dfa8d2c9275fe3772:design/SymbolicDynamicScope.md` and `design/DesignAuthorityRegistry.md`. Documentation only; no implementation, build or tests.

## 2026-11-15 — Unchecked bounds exceptions and explicit check opt-out

<a id="h-448"></a>
**H-448 — Default runtime checking without a checked-exception obligation.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. In response to the proposed common builtin array/varargs bounds policy, the user says: “感觉 IndexOutOfBoundsException 还是 unchecked 比较好吧；运行时默认检测，但允许编译开关 opt-out，后果自负”. Selects unchecked IndexOutOfBoundsException, default runtime detection, and explicit compiler opt-out at the program's risk. D-17-BOUNDS/P-401–P-402 records this scope; no checked throws requirement, implicit disablement in optimized builds, general safety-check opt-out or broader pack semantics are selected.

<a id="k-bounds-policy-20261115"></a>
**K-BOUNDS-POLICY-20261115 — Failure-policy integration and continuity.** Agent integration under H-448, existing C-63/C-65 and charter §§4.3/6.3/8/12: D-17r62→63/D-10r151→152/D-14r102→103 and registry r712→713 distinguish unchecked exception classification from disabled detection. Enabled checks preserve ordinary exception cleanup and operation preparation; opt-out transfers the bounds precondition to the program, interpreted as no defined-result/exception/cleanup guarantee on invalid unchecked execution. Preserve all owner/access/initialization, checked NPE and D-16 requirements; P-400's no-unsound-fallback rule remains because opt-out is an explicit separate bounds policy, not an initialization-proof escape. Add scoped default/read/write/view acceptance and retain CLI/mixed-compilation/payload engineering work; broader pack contracts remain under review. Published baseline: `57d108f116e54ce5b673452cb0aa646cc74396a6` at affected paths. Documentation only; existing native bounds paths are not claimed migrated, and no code/build/tests or compiler configuration changed.

## 2026-11-15 — Initialization design checkpoint integration

<a id="k-704"></a>
**K-704 — Preserve implementation evidence and integrate scoped planning decisions.** Agent reconciliation under H-420/H-423/H-443–H-447 and charter §§2/3.3/4.3/6/8/12: merge design checkpoint `3a44444e9c40ceed635bd1b8524001d603e97e9e` with compiler checkpoint `80eeab6ac61e29e1dcb2738e91b926de3958689b`. Registry r712 retains source reference324, D-17r62, D-18r95, D-23r14 and D-26r17. D-14's two r101 branches become r102; D-28r19→20 and D-22r18→19 align current coverage pointers. D-19r4 retains the side checkpoint's scoped research records. H-443/P-393–P-394 preserves CoD semantics at foreign boundaries; H-444/P-395–P-397 records deferred pure knot-tying; H-445–H-447/P-398–P-400 preserves identity/readiness separation, mandatory-write obligations and staged soundness. Their recorded value paths, mechanism exclusions and descendant constraints remain authoritative. No aggregate/publication syntax, pure evaluation model, runtime initializer flag or physical protocol is inferred. Both histories remain intact; current capability pointers retain the verified compiler behavior rather than reverting to older pending-implementation text. This merge changes documentation only; source/native evidence remains the compiler checkpoint's evidence.

## 2026-11-15 — Prepared caller-binding methods

<a id="k-703"></a>
**K-703 — Shared deferred method and binding proofs.** Agent implementation/continuity under H-306/H-356–H-362/H-407/H-420/H-423 and charter §§2–3/6/8/12: source reference324/registry711 admits prepared reference-swap methods for scalar and intrinsic exception targets. D-14r100→101/D-17r61→62 update implementation alignment; D-17's confirmed value path through C-63/P-235–P-236/P-321–P-324 governs prepared cleanup, with D-14's independently confirmed C-69/P-268–P-269 requirements supplying ordered destination selection, actual caller bindings and owner/access constraints. This does not qualify D-14 as a whole. Baseline: `8914d0f3914d4586789f0461f187bb428fa54446` at affected paths. Invocation substitutes current targets into shared cleanup equations; entry checks, per-exit writes and pending completion preserve the existing contracts. No callable slot ABI, broader permission proof, regional allocator, moving backing, fraction or purity implementation follows. The registry authority summary links to retained historical evidence instead of repeating implementation history; no human record or document-scope qualification is removed. Other design scopes and descendants retain their qualification. Current acceptance and limits are in `tests/cod/language/README.md`; progress and validation reports remain outside the repository.

## 2026-11-15 — Reference identity and shared owner inputs

<a id="k-702"></a>
**K-702 — Bounded reference observation and source migration.** Agent implementation/continuity under H-437/H-439–H-442/H-420/H-423 and charter §§2/4.3/6.3/8/12: source reference323/registry710 implements stable lexical is/is not, contextual null testing and canonical not/! through shared argument, cleanup and native-call paths. D-10r150→151/D-14r99→100/D-17r60→61/D-18r94→95/D-23r13→14/D-26r16→17/D-28r13→14 align implementation pointers without changing their confirmed scope. Baseline: `d4b4e4493a9a3ca54147b9e137f6c71138308a9b` at the affected paths. Actual-owner inputs are shared per input-bound role; no permanent ABI, per-reference owner record, moving backing, graph key, regional/subobject layout, allocation-qualifier implementation or fraction change follows. Current acceptance and limits are in `tests/cod/language/README.md`; process and reports remain outside the repository.

## 2026-11-15 — Confirmed reference-design integration

<a id="k-701"></a>
**K-701 — Preserve compiler coverage and integrate scoped reference decisions.** Agent reconciliation under H-420/H-423/H-433–H-442 and charter §§2/4.3/6.3/8/12: registry r709 combines source reference322/K-700 from `a67e1abaaec6ce5151298ae0a65b1ec9e3e7a8ca` with the independently selected scopes at `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6:design/RegionalAndLexicalStorageRefactor.md` and its dependent designs. D-14r99 preserves K-684 type-query evidence; D-18r94 preserves K-682 parameter access; D-10r150 preserves K-681 shadow diagnostics; D-21r12 remains current. H-437–H-442 owner sharing, allocation qualifiers, identity/null operators and unary spellings are design contracts with implementation gaps, not new native evidence. Exact layouts, key carriers, allocation transport and whole-document qualifications remain separate. D-16 constraints and all unrelated qualifications continue.

## 2026-11-15 — Caller-binding methods

<a id="k-700"></a>
**K-700 — Caller-binding method continuity.** Agent assessment under H-305/H-306/H-327/H-342/H-344/H-356/H-398/H-407/H-420/H-423, D-14r92/D-17r57/D-18r92/D-21r12/D-26r13 and charter §§2/3.3/6.3/12: registry r707→708 and source reference321→322 implement direct reference-swap methods with actual caller slots through ordinary dispatch, native frames and per-completion binding summaries. Acceptance preserves ordered evaluation, current-target entry checks, exact type/owner/access, ref validity, conservative alias mutation, separate entry-payload and output-selection initialization, exception/cleanup effects, ordinary helper isolation and shared generic bodies. Prepared method calls, callable payload flow, callable-value slot contracts and broader permission/ref-preservation proofs remain unproved; builtin coverage remains. No semantic revision, physical regional protocol, persistent ABI, fraction implementation or new design qualification. Prior capability: `f6bb01009d93c46740b063b66fedfa02046143eb:tests/cod/language/README.md`.

## 2026-11-14 — Builtin operand completion

<a id="k-699"></a>
**K-699 — Builtin completion continuity.** Agent assessment under H-261/H-272/H-304/H-356/H-361/H-362/H-398/H-407/H-420/H-423, D-14r92/D-17r57/D-18r92/D-21r12/D-26r13 and charter §§2/3.3/6.3/12: registry r706→707 and source reference320→321 propagate non-producing operands through definitely builtin eager operations. Acceptance preserves arity, known-input validity, source-order preparation, earlier failures, return/throw replacement, no aborted invocation/registration, complete-image cleanup and shared recursive native bodies. Missing input evidence for user-method/declaration selection stays unproved. No new payload type, numeric conversion, lifetime, ABI, fraction implementation or design qualification. Prior capability: `913fd456d3c692a08e195a2bf01351f1780eb283:tests/cod/language/README.md`.

## 2026-11-14 — Deferred reference exchange

<a id="k-698"></a>
**K-698 — Deferred binding-exchange continuity.** Agent assessment under H-305/H-306/H-342/H-344/H-356/H-398/H-407/H-420, D-14r92/D-17r57/D-18r92/D-21r12/D-26r13 and charter §§2/3.3/6.3/12: registry r705→706 and source reference319→320 support prepared reference swaps through shared slot selection and native invocation. Acceptance preserves invocation-time targets, exact type/owner/access, ref initialization and non-null validity, conditional registration, composed queues, failed preparation, pending completion and exact image release. Prepared rebind summaries cannot restore unrelated selection facts. Reference-swap methods, broader target correlations, regional storage and fractions retain their boundaries. No semantic revision or new qualification. Prior capability: `5a000c50e527e50dc39605d0e4e4cea4012713f3:tests/cod/language/README.md`.

## 2026-11-14 — Prepared assignment cleanup

<a id="k-697"></a>
**K-697 — Prepared assignment continuity.** Agent assessment under H-270/H-272/H-293/H-302/H-304/H-342/H-344/H-356/H-398/H-407/H-417/H-420, D-10r147/D-14r92/D-17r57/D-18r92/D-21r12/D-26r13 and charter §§2/3.3/6.3/12: registry r704→705 and source reference318→319 share assignment preparation and mutation with deferred cleanup. Acceptance preserves selected targets/slots, source snapshots, once-only preparation, fixed owners/access, successful-registration initialization, pending completion and exact image release. Borrowed result views retain no source-call invocation metadata. Assignment methods, required assignment queries, regional storage and fraction implementation retain their boundaries. No semantic revision or new qualification. Prior capability: `8ee5156192a524350ea4dff60dde4963fb8fac68:tests/cod/language/README.md`.

## 2026-11-14 — Explicit assignment forms

<a id="k-696"></a>
**K-696 — Assignment normalization continuity.** Agent assessment under H-270/H-272/H-293/H-302/H-304/H-342/H-344/H-398/H-407/H-417/H-420, D-10r147/D-14r92/D-17r57/D-18r92/D-21r12/D-26r13 and charter §§2/3.3/5.3/6.3/12: registry r703→704 and source reference317→318 normalize explicit/intrinsic `=` and `<-` calls into the infix assignment nodes, retaining their builtin marker. Acceptance preserves exact owners/access, initialization, ordered preparation, destination-view results, completion, complete callable cleanup, identical emitted code and shared recursive bodies. Standalone deferred invocations, assignment methods and required assignment queries remain explicit gaps. No new operator semantics, construction, ownership, ABI, fraction rule or design qualification. Prior capability reference: `06fabc24f03aca4df75bb389ea04cbe7a6e47e85:tests/cod/language/README.md`.

## 2026-11-14 — Symbolic query signature validation

<a id="k-695"></a>
**K-695 — Shared invocation-contract continuity.** Agent assessment under H-184/H-185/H-295/H-381/H-382/H-398/H-407/H-420–H-424, D-10r147/D-14r92/D-18r92/D-24r7/D-26r13 and charter §§2/5.3/6.3/12: registry r702→703 and source reference316→317 share procedure-symbol and print-signature validation between required queries and executable checking. Acceptance rejects invalid kind/arity/argument types while preserving unevaluated operands, uninstalled symbols, lexical/source identity, independent body evidence and identical native code for valid queries. The governance row follows the current registry revision. No new construction form, effect permission, lifetime, ABI, fraction rule or design qualification. Prior capability reference: `bc6971fe6518842a9fb59db9a97f3793e0c30012:tests/cod/language/README.md`.

## 2026-11-14 — Partially implemented family queries

<a id="k-694"></a>
**K-694 — Combined query-evidence continuity.** Agent assessment under H-230/H-232/H-245/H-247–H-249/H-258/H-295/H-398/H-407/H-420–H-424, D-10r147/D-14r92/D-15r22/D-18r92/D-21r12/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r701→702 and source reference315→316 use declared contracts for inspected inputs with no applicable method, preserving selection and validation where a method applies. Methods and declarations share the selected family identity, including receiver contracts in imported helper queries. Acceptance covers contextual preparation, partial admitted domains, independent results, ambiguity, owners/access/throws, source isolation, identical native code and exact cleanup. Real calls/materialization still require executable coverage; declarations cannot discharge unresolved method proofs. No new dispatch priority, result inference policy, world-composition mechanism, lifetime, ABI, fraction rule or design qualification. Prior capability reference: `360d5cc1192932b73ce88013875226e6ad616c7b:tests/cod/language/README.md`.

## 2026-11-14 — Callable signature inspection

<a id="k-693"></a>
**K-693 — Declared-signature continuity.** Agent assessment under H-258/H-295/H-398/H-407/H-420–H-424, D-10r147/D-14r92/D-18r92/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r700→701 and source reference314→315 share signature construction, result agreement and symbolic owner evidence between declared-call inspection and callable type-only operands. Unique concrete or independently contextual signatures retain all admitted-input, same-type, owner/access, result and checked-failure obligations. Acceptance covers generic/variadic/overloaded declarations, selected value calls, imports, retained contexts, permitted value-result conversions, identical native code and allocation-free queries. Inspection proves no executable coverage and creates no runtime value. Open signature inference, borrowed target adaptation and raw callable construction remain unproved. No new source inference policy, lifetime, carrier, ABI, fraction mechanism or design qualification. Prior capability reference: `95cc87e97cc08beb9c86c263ed29922019a6e988:tests/cod/language/README.md`.

## 2026-11-14 — Contextual declaration queries

<a id="k-692"></a>
**K-692 — Signature-evidence continuity.** Agent assessment under H-258/H-295/H-398/H-407/H-420–H-424, D-10r147/D-14r92/D-18r92/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r699→700 and source reference313→314 share partial argument matching between executable methods and declaration-only type queries. All applicable declaration contracts remain obligations; established inputs can supply generic callable context without executing, materializing or demanding a body. Acceptance covers variadic tails, conflicting signatures, owner/access/throws, imported and retained contexts, no native query work and exact cleanup. Ordinary execution still requires implementations. No parameter-inference policy, dispatch tiebreaker, conversion, lifetime, ABI, fraction mechanism or design qualification is added. The registry heading is synchronized with its current authority revision. Prior capability reference: `865ae7d9cc8162a53694fcbda58d639a9f38d040:tests/cod/language/README.md`.

## 2026-11-14 — Exact common-exception borrowing

<a id="k-691"></a>
**K-691 — Borrowed-target continuity.** Agent assessment under H-258/H-269/H-342/H-344/H-375/H-378/H-398/H-407/H-420–H-424, D-14r92/D-17r57/D-18r92/D-21r12/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r698→699 and source reference312→313 preserve exact common-exception storage types across declaration coverage, specificity, callable arguments and native invocation. Copied value categories retain their distinct selection path. Acceptance covers mutable/readonly/nullable targets, independent owners, declared/copied results, mixed dispatch, captured/deferred calls, identical native code, shared recursion and exact failure cleanup. Borrowed target adaptation and unresolved value-category relationships remain unproved. No new covariance, owner representation, lifetime, region protocol, ABI, fraction mechanism or qualification. Prior capability reference: `37bddc8e5bcf4a1569f2e5505ccaa3a529872b3b:tests/cod/language/README.md`.

## 2026-11-14 — Dependent exception result obligations

<a id="k-690"></a>
**K-690 — Dependent-result proof continuity.** Agent assessment under H-258/H-269/H-398/H-399/H-407/H-420–H-424, D-14r92/D-17r57/D-18r92/D-24r7/D-26r13 and charter §§2/5.3/6.3/12: registry r697→698 and source reference311→312 separate input coverage from dependent-result category evidence. A covering declaration cannot replace an unknown concrete result category with SystemException, including through an instantiated method binder. Such unproved promises receive an implementation-gap diagnostic; concrete categories, fixed results and exact borrowed target types retain their contracts. Acceptance covers broad/generic/recursive/nested/deferred/imported obligations and both native modes. No language prohibition, new inference mechanism, runtime guard, carrier, conversion, ABI, fraction rule or qualification is selected. Prior capability reference: `03a1402c890035a8d0af570a9e9916dea5b9bf41:tests/cod/language/README.md`.

## 2026-11-14 — Exception declaration overlap

<a id="k-689"></a>
**K-689 — Exception-domain continuity.** Agent assessment under H-258/H-269/H-398/H-407/H-417/H-420–H-424, D-14r92/D-17r57/D-18r92/D-24r7/D-26r13 and charter §§2/3.3/5.3/6.3/12: registry r696→697 and source reference310→311 distinguish possible exception-value overlap from whole-method coverage. Symbolic category intersections retain repeated method binders, independent results and original-input failure applicability through preparation. Partial dependent results and borrowed/nested common-type relationships retain explicit proof boundaries. Acceptance covers known/distinct/unknown categories, queries, complete values, deferred/import/operator paths, equivalent native code, shared recursion and retained-cause cleanup. No finite catalog defines the overlap proof; no new source policy, runtime carrier, ingress, lifetime, ABI, fraction mechanism or design qualification. Prior capability reference: `7a977eacca0db44040284473dc5544b0f53a47ac:tests/cod/language/README.md`.

## 2026-11-14 — Numeric declaration overlap

<a id="k-688"></a>
**K-688 — Numeric declaration continuity.** Agent assessment under H-258/H-261/H-398/H-407/H-417/H-420–H-424, D-14r92/D-18r92/D-24r7/D-26r13 and charter §§2/3.3/5.3/6.3/12: registry r695→696 and source reference309→310 preserve narrower numeric declaration obligations alongside independently established method results. Original and prepared inputs retain result and checked-failure promises; complete callable signatures reuse admitted-input coverage. Acceptance covers recursive anchors, generic relations, retained queries, owner/variadic/operator/import paths, direct/deferred failures, equivalent native code and exact capture cleanup. Common-exception type-domain proofs and differing preparation modes remain unproved. No new conversion, inference policy, owner union, lifetime, exception category, ABI, fraction mechanism or design qualification. Prior capability reference: `b1261421dfda8b77962daeb998ec0202e65a2318:tests/cod/language/README.md`.

## 2026-11-14 — Overlapping declaration contracts

<a id="k-687"></a>
**K-687 — Declaration-obligation continuity.** Agent assessment under H-258/H-398/H-407/H-417/H-420–H-424, D-14r92/D-18r92/D-24r7/D-26r13 and charter §§2/3.3/5.3/6.3/12: registry r694→695 and source reference308→309 distinguish covering result contracts from overlapping owner/access obligations. Validate independent results using the declaration’s owner equalities without changing method roles or permissions. Direct calls preserve actual applicability; complete callable views retain all admitted checked failures. Acceptance covers recursive anchors, transitive/variadic owners, readonly/nullable results, deferred/operator/import paths, identical native forms and failed-copy cleanup. No new conversion, owner union, lifetime, exception, ABI, fraction mechanism or design qualification. Prior capability reference: `8fe09dc853f5536c6311c74513a0179f1985694e:tests/cod/language/README.md`.

## 2026-11-14 — Caught-value owner evidence

<a id="k-686"></a>
**K-686 — Handler-evidence continuity.** Agent assessment under H-267/H-269/H-356–H-362/H-398/H-407/H-417/H-420–H-424, D-14r92/D-17r57/D-18r92/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r693→694 and source reference307→308 preserve caught-value owners in shared operand/result evidence. Independent handler returns, retained query contexts and inferred deferred recovery preserve storage permissions, actual owners, capture/borrow boundaries, ordered preparation and cleanup. Acceptance covers nested/shadowed catches, snapshots versus borrows, recursive native reuse, missing returns and allocation-failure release. No new exception category, lifetime rule, query form, runtime representation, fraction rule, ABI or design qualification. Prior capability reference: `69d5451647536b15349e1e035a8c1f7342cc8c08:tests/cod/language/README.md`.

## 2026-11-14 — Loop-control owner evidence

<a id="k-685"></a>
**K-685 — Loop-evidence continuity.** Agent assessment under H-365/H-398/H-407/H-417–H-424, D-14r92/D-17r57/D-18r92/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r692→693 and source reference306→307 extend shared owner evidence to readonly loop indices and fold accumulators, enabling independent recursive results and retained type-only contexts. Full-scope ownership for here remains separate from loop-control storage. Preserve ordered checking, actual owners, declaration/capture restrictions, initialization, lifetime, effects, deferred queues and shared native bodies. Acceptance covers nested controls/locals, aliases/copies, returned query contexts, invalid escapes and native resource accounting. No new loop abstraction, representation, ABI, query form, fraction rule or design qualification. Prior capability reference: `27acacf7873756aee75ad886db3965e78508c22f:tests/cod/language/README.md`.

## 2026-11-14 — Scoped owner evidence

<a id="k-684"></a>
**K-684 — Scoped-evidence continuity.** Agent assessment under H-273/H-295/H-342/H-344/H-398/H-407/H-417/H-420–H-424 and charter §§2/3.2/5.3/6.3/12: registry r691→692 and source reference305→306 preserve fresh full-scope owner identity through ordered operand evidence, retained declaration contexts and independent recursive result inference. Ordinary groups and existing aliases keep their owners. Checker identities distinguish checked, retained, formal and fresh-scope facts; native storage, access/lifetime/initialization, complete captures and cleanup remain unchanged. D-14r91→92’s bounded typeof paragraph aligns omitted-result evidence with D-24r7/H-421–H-424 while preserving H-295’s no-runtime-evaluation rule. Acceptance covers snapshots versus borrows, nested scopes, inferred results, captured/query contexts, native failures and shared recursive code. Broader query forms, owner-dependent family dispatch, permanent proof/ABI choices and pending designs remain separate; no new qualification. Prior contracts: `2d8b1e00675899b6aeef4a5f46957aea0fe37660:design/RegionalOwnership.md` and `2d8b1e00675899b6aeef4a5f46957aea0fe37660:tests/cod/language/README.md`.

## 2026-11-14 — Shared symbolic owner proof

<a id="k-683"></a>
**K-683 — Callable-owner continuity.** Agent assessment under H-273/H-302/H-342/H-344/H-375–H-378/H-398/H-407/H-417/H-420, D-14r91/D-18r92/D-21r12/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r690→691 and source reference304→305 record callable coverage through the ordinary selector’s symbolic owner proof. Repeated formal roles establish equality; distinct roles retain both equal and different possibilities. Preserve full admitted-type obligations, exact method/view contracts, declared access, unique result owners, complete captures, initialization and cleanup. Acceptance covers independent/repeated roles, recursive forwarding, query/capture/deferred paths, native failures and shared method inventory. Unresolved owner-dependent selection remains unproved; no runtime owner carrier, permanent proof interface, ABI or new qualification. Prior capability reference: `2a2fa2a874f3851eba75ce7887a315ddcee18829:tests/cod/language/README.md`.

## 2026-11-14 — Callable-view target access

<a id="k-682"></a>
**K-682 — Callable-access continuity.** Agent assessment under H-272/H-366–H-369/P-327/H-375/H-378/H-398/H-407/H-420 and charter §§3.2/5.3/6.3/8/12: registry r689→690, D-18r91→92 runtime-callable contract, D-21r11→12 callable hierarchy and source reference303→304 preserve the view's declared target access alongside actual argument type identity. H-369 delegates direct qualifier consequences; this clarification applies existing readonly propagation and independent local-value storage to family coverage. Preserve complete captures, ordered preparation, owner/throws/result checks and native reuse. Acceptance covers direct-versus-view access, actual input types, higher-order/borrowed/deferred/symbolic calls, selected dependencies and cleanup in both native modes. Owner-dependent family selection, recursive self-value capture policy, broader representations and pending designs retain their separate status; no ABI, owner representation or new qualification. Prior contracts: `2326246352eb6377ae996f9f42ac556ae442156f:design/RegionalClosures.md`, `2326246352eb6377ae996f9f42ac556ae442156f:design/AheadOfTimeExecutionPreparation.md` and `2326246352eb6377ae996f9f42ac556ae442156f:tests/cod/language/README.md`.

## 2026-11-14 — Shared lexical shadow diagnostics

<a id="k-681"></a>
**K-681 — Shadow-diagnostic continuity.** Agent assessment under H-257/P-229/H-381/H-382/H-398/H-420, D-10r147/D-14r91/D-18r91 and charter §§3.2/5.3/8/12: registry r688→689 and source reference302→303 use one checked source-declaration path for shadow warnings. Preserve nearest-binding lookup, enclosing source visibility, initializer order, duplicate errors and warning promotion before execution. Diagnostic metadata remains checker-local; synthetic captures do not warn, and repeated native contexts do not duplicate source diagnostics. Acceptance covers checked parameters/owners, local definitions/values/aliases, loop/catch bindings, generic/import reuse and both native modes. Type-only or otherwise undemanded bodies retain their existing checking boundary. Existing actual-type family dispatch and the unproved access/owner-dependent family-selection boundary retain their scope. Capture-time semantics, native representations and pending designs remain separate; no new qualification. Prior capability reference: `5f2553e377b65e325e19fd1e4cd3afab6eefa7c9:tests/cod/language/README.md`.

## 2026-11-14 — Lexical callable name resolution

<a id="k-680"></a>
**K-680 — Lexical-name continuity.** Agent assessment under H-257/H-295/H-329/H-331/H-342/H-343/H-381/H-398/H-420, D-10r147/D-14r91/D-18r91/D-21r11/D-26r13 and charter §§3.2/5.3/6.3/12: registry r687→688 and source reference301→302 share nearest-name resolution for value and capture operands. Recursive lexical identities cannot fall through to same-spelled module methods; unsupported self-value materialization reports an implementation gap. Nested type-only queries retain established lexical signatures without runtime construction. Preserve source context, shadowing, complete captures, direct recursion, owners and cleanup. Recursive self-value capture semantics remain under review; no capture-time, lifetime, ABI or new design qualification is selected. Prior capability reference: `3eaabe8fbe34e412bf49071964017568cf456214:tests/cod/language/README.md`.

## 2026-11-14 — Lexical callable family coverage

<a id="k-679"></a>
**K-679 — Lexical-family continuity.** Agent assessment under H-261/H-269/H-329/H-331/H-342/H-344/H-375–H-378/H-398/H-407/H-420, D-10r147/D-14r91/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r686→687 and source reference300→301 extend shared admitted-domain coverage and dispatch to lexical callables. Preserve actual generic inputs, fixed enclosing type bindings, independent results, definition-site contextual contracts, complete captures, owner/access/throws checks, cleanup and shared native bodies. The single-instance callable representation is removed. Acceptance covers unused inputs, nested snapshots, dynamic dependencies, native failure cleanup and runtime-depth independence. Broader family/representation proofs and fraction boundaries remain separate; no new conversion, allocation mechanism, ABI or qualification. Prior capability reference: `bd3fababda74cb60acbf063134d930095e728a01:tests/cod/language/README.md`.

## 2026-11-14 — Ordinary module-family values

<a id="k-678"></a>
**K-678 — Callable-family continuity.** Agent assessment under H-261/H-269/H-342/H-344/H-375–H-378/H-398/H-407/H-420, D-10r147/D-14r91/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r685→686 and source reference299→300 unify ordinary and symbolic module-family coverage/materialization. Contextual type-only checking uses the same admitted-domain obligations without runtime construction; concrete methods can collectively cover common views. Preserve actual-input semantic selection, source/import interpretations, independent method results, complete captures, owner/access/throws constraints, cleanup and shared native bodies. Lexical full-domain coverage, wider compatibility/representation and fraction boundaries remain separate; no new conversion, allocation mechanism, ABI or design qualification. Prior capability reference: `62e77edee116abf1132ecae5a8b5118cf671543c:tests/cod/language/README.md`.

## 2026-11-14 — Family value-result adaptation

<a id="k-677"></a>
**K-677 — Family-result continuity.** Agent assessment under H-261/H-269/H-372–H-378/H-398/H-407/H-420, D-10r147/D-14r91/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r684→685 and source reference298→299 record family value results using existing lossless numeric and concrete-to-common exception transfers after method completion. Preserve full-domain checks, actual exception identity, selected interpretations, captures, failure outcomes, cleanup and native reuse. Borrowed targets, callable signatures, access/owner rules and ordinary method materialization retain their contracts. Correct the stale exact-argument limitation for established callable views. No general covariance, new conversion policy, fraction mechanism, permanent ABI or additional design qualification. Prior capability reference: `f49186c628f2faff93f9dbd274cae7798fa7e59e:tests/cod/language/README.md`.

## 2026-11-14 — Independent return-expression alternatives

<a id="k-676"></a>
**K-676 — Result-alternative continuity.** Agent assessment under H-398/H-407/H-421–H-424, D-14r91/D-17r57/D-18r91/D-23r10/D-24r7/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r683→684 and source reference297→298 record independent return evidence through conditional, block and try/catch result alternatives. Preserve separate lexical/module interpretations, generic/capture context, recursive-component anchors, completed expression checks, owner/access constraints and cleanup. Operand guesses, unreachable/always-abrupt results and unanchored cycles supply no contract; no conditional conversion, result-mode, fraction, lifetime or ABI policy changes. Whole D-24 value-path qualification and broader proof boundaries remain separate. Prior capability reference: `20a10f9f4b10f7bc64f5751632a2395adb134bbb:tests/cod/language/README.md`.

## 2026-11-14 — Checked module initialization planning

<a id="k-675"></a>
**K-675 — Initialization-phase continuity.** Agent assessment under H-106/H-109/H-140/H-166/H-245/H-398, D-10r147/D-15r22/D-21r11 and charter §§2/3.2/5.3/6.3/12: registry r682→683 and source reference296→297 place ordered initialization and module-origin records in the checked program, independently of source discovery. Preserve source isolation, once-only header encounter order, effect/cleanup behavior and native execution. Conditional first-reference policy, interleaved incorporation, retained dependency scopes/shutdown and symbolic installation cancellation remain open or unproved; no new language, lifetime, ABI, regional or fraction policy is chosen. Prior capability reference: `5fae80b57abb865c2a81251f274aa37b85365cee:tests/cod/language/README.md`.

## 2026-11-14 — Ordinary callable import interpretation

<a id="k-674"></a>
**K-674 — Import-identity continuity.** Agent assessment under H-230/H-232/H-245/H-247–H-249/H-252/H-253/H-258/H-375–H-378/H-398, D-10r147/D-15r22/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/4/5.3/6.3/12: registry r681→682 and source reference295→296 record ordinary receiver import through shared identity records and selected interpretation chains. Preserve source definitions, non-callable lookup, resolved signatures/throws, nested contexts, complete captures, actual owners and cleanup; retain inherited overloads and reject conflicts without alias-order arbitration. Bodyless publication and export-import preserve explicit identity. Symbolic import/cancellation, first-reference incorporation, retained module scopes/shutdown, same-alias unions, mixin, broader context equivalence and persistent ABI remain unproved; whole D-15 qualification and fraction/regional mechanisms remain unchanged. Prior capability reference: `c032c34f9e0bea3632404e1a17e54746d1307755:tests/cod/language/README.md`.


## 2026-11-14 — Source-preserving module contexts

<a id="k-673"></a>
**K-673 — Module-context continuity.** Agent assessment under H-106/H-109/H-166/H-212/H-231/H-233/H-245/H-247–H-249/H-372–H-378/H-398, D-10r147/D-15r22/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/4/5.3/6.3/12: registry r680→681 and source reference294→295 record source-preserving file selection, explicit alias publication, separate lookup/symbol/family records and once-only encounter-ordered initialization. Preserve private definition context, full family coverage/capture, actual owners, explicit initialization-scope cleanup and shared native methods. Interleaved selection/first-reference incorporation, retained dependency scopes/shutdown, import adaptation, mixin, broader composition, cyclic construction, module-local callable captures, entry-installation cancellation and persistent ABI remain unproved; D-15’s whole operational/value-path qualification remains pending. No source-policy, regional allocation or fraction mechanism is selected. Prior capability reference: `8689f8465da4c964cb700802d3690b71431a4e94:tests/cod/language/README.md`.

## 2026-11-14 — Family input contracts

<a id="k-672"></a>
**K-672 — Family-input continuity.** Agent assessment under H-269/H-273/H-302/H-342/H-344/H-356/H-372–H-378/H-398/H-407/H-409/H-417/H-420, D-10r147/D-14r91/D-17r57/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r679→680 and source reference293→294 record concrete-category dispatch through common exception values, complete callable input coverage and exact borrowed family contracts. Preserve actual type/owner identity, access, initialization, result targets, prepared snapshots, complete captures and exact cleanup through shared checked/native invocation. Common-exception borrowing, broader adapters, permanent ABI, external input, regional storage and fraction mechanisms retain their boundaries. Prior capability reference: `7919ec0afb9956f3ee4755d375ea5d26317db754:tests/cod/language/README.md`.


## 2026-11-14 — Symbolic family source execution

<a id="k-671"></a>
**K-671 — Family-value continuity.** Agent assessment under H-372–H-378/H-381/H-382/H-398/H-407/H-409, D-10r147/D-18r91/D-21r11/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r678→679 and source reference292→293 record module-local symbolic families with primitive value-input views. Preserve actual-type semantic selection before parameter preparation, source identity across installation, full admitted-domain checking/capture, complete independent values and ordered cleanup. Reuse shared general bodies and existing image/TLS ownership; registration, materialization and installation remain separate. Broader family proofs, module composition/cancellation, permanent ABI, regional storage and fraction mechanisms retain their boundaries. Prior capability reference: `e7aae20e8bba8125dc13bb5156f3ce0172fd0c4e:tests/cod/language/README.md`.

## 2026-11-14 — Exception-valued dynamic bindings

<a id="k-670"></a>
**K-670 — Exception-binding continuity.** Agent assessment under H-269/H-277–H-281/H-286/H-356/H-365/H-420, P-236/P-248/P-252, D-10r147/D-17r57/D-18r91 and charter §§2/3.2/5.3/6.3/12: registry r677→678 and source reference291→292 record intrinsic-exception TLS values through the existing bits classification, complete captures and scope restoration. Preserve immutable payloads/causes, actual categories, checked throws, current-versus-captured calls, deferred timing and execution-local cause lifetime. Acceptance covers checked native delivery, nested/returned captures, failed chaining/copy/registration, allocation-free reads/replay and thread isolation. No eager cause collection, cross-execution identity, regional storage/copy, new result defaults or fraction mechanism. Prior capability reference: `0de1df90a94e6c58d245499bc0d0c149cab3a538:tests/cod/language/README.md`.

## 2026-11-14 — Complete dynamic captures and callable replay

<a id="k-669"></a>
**K-669 — Dynamic-callable continuity.** Agent assessment under H-98/H-185/P-89, H-286/P-252, H-288–H-291/P-254, H-342/H-344/P-301–P-303, H-356/H-365/H-373/H-420, D-10r147/D-17r57/D-18r91/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r676→677 and source reference290→291 record complete primitive/callable dynamic captures, compatible concrete procedure-value installation, current-binding invocation and captured replay. Preserve definition/materialization separation, existing concrete-view preparation, lexical frame roles, independent nested images, function exclusion, deferred timing and caller restoration on every completion. Acceptance covers native/shared execution, missing-binding rejection, zero-allocation replay, injected preparation failures and thread isolation. Whole symbolic families, actual-type family dispatch, regional/exception bindings, persistent ABI and fractions remain outside coverage; no new source policy, owner extension or whole-design qualification. Prior capability reference: `6d119dd20f2a9e6a67f59a16a4c7bc80c92f4416:tests/cod/language/README.md`.

## 2026-11-14 — Primitive dynamic binding and restoration

<a id="k-668"></a>
**K-668 — Dynamic-binding continuity.** Agent assessment under H-98/P-89, H-277–H-281/P-248, H-286/P-252, H-319/H-356/H-365/H-371/P-326, H-420, D-10r147/D-14r91/D-17r57/D-18/D-26r13 and charter §§2/3.2/5.3/6.3/12: registry r675→676 and source reference289→290 record primitive TLS installation/read/touch, current-binding requirements through calls and cleanup, and full-scope/iteration/else restoration. Preserve identity, function exclusion, lossless preparation, RHS effects, completion, deferred timing, allocation-free restoration and thread isolation. Declaration-only queries allocate no TLS storage. Incoming dynamic captures, non-primitive bindings, symbolic family installation/composition, regional resources and public/persistent ABI remain outside coverage. No new language permission, owner rule, fraction mechanism or whole-design qualification. Prior capability reference: `ba07cb6da10963a66dfd76c3e79b02d9821bbe0d:tests/cod/language/README.md`.

## 2026-11-14 — Typed symbol declaration evidence

<a id="k-667"></a>
**K-667 — Symbol-contract continuity.** Agent assessment under H-184/P-160, H-381/P-342, H-382/P-343, H-420, D-10r147/D-14r91/D-18/D-21r11/D-26r13 and charter §§2/5.3/6.3/12: registry r674→675 and source reference288→289 record typed symbol identities, declaration-only type inspection and lexical lookup evidence independent of runtime captures. Preserve alias identity, nearest binding, callable definition context, unevaluated explicit signatures, owners, completion and native execution. Own-family installation syntax retains an explicit runtime gap. General symbol reads/installation, symbolic methods, family composition, unresolved same-scope categories and regional owner contracts remain outside coverage; no runtime symbol ABI, default binding, capture permission or fraction mechanism is selected. Prior capability reference: `51c1c4c96c1850027bca67db5d943c87a055c169:tests/cod/language/README.md`.

## 2026-11-14 — Shared reference-intrinsic invocation

<a id="k-666"></a>
**K-666 — Reference-invocation continuity.** Agent assessment under C-65/P-258/P-268/P-269, H-306/H-313/H-315/H-356/H-365/H-420, D-14r91/D-17r57/D-18/D-26r13 and charter §§5.3/6.3/12: registry r673→674 and source reference287→288 record checked target-versus-binding-slot preparation, shared native invocation for null tests/reference swaps and prepared deferred null tests. Preserve evaluation order, non-dereferencing tests, actual owners, initialization, cleanup, complete-image ownership and shared native interpretation. Untyped absence carries no referent identity. The capability reference retains K-665 lossless fold support consistently. Deferred binding swaps, captured-payload mutation and executable packs remain separate gaps; no new language permission, layout, public ABI or fraction mechanism. Prior capability reference: `6fb1f4970153c860a28ad1867030897b463aa06d:tests/cod/language/README.md`.

## 2026-11-14 — Lossless fold updates

<a id="k-665"></a>
**K-665 — Fold-conversion continuity.** Agent assessment under H-261/P-231/H-365/H-420, D-10/D-14r91/D-17r57/D-26r13 and charter §§5.3/6.3/12: registry r672→673 and source reference286→287 record whole-type lossless primitive conversion from each completed fold body into its established accumulator type. Reuse the checked/native value-conversion path; preserve readonly loop bindings, once-only bounds, empty results, cleanup/completion, generic identity and shared native coverage. No new widening, mixed-endpoint rule, quantity/fraction support, pack semantics, layout or ABI is selected. D-11/D-27 whole-document qualification and executable-pack design boundaries remain separate. Prior capability reference: `f1b3a63a948ada815f50edb4414b39578f065bf6:tests/cod/language/README.md`.

## 2026-11-14 — Call-summary input roles

<a id="k-664"></a>
**K-664 — Call-effect continuity.** Agent assessment under C-65/P-258, H-382/H-398–H-410/H-420, D-14r91/D-17r57/D-18/D-21r11/D-26r13 and charter §§5.3/6.3/12: registry r671→672 and source reference285→286 record argument-indexed read/write/return/null-provenance summaries derived from checked frame roles. Captured environment storage and parameter storage remain independent; native layout preserves the checked correspondence. Preserve alias-sensitive initialization, exact return selection, owners/access, completion, preparation, cleanup and native reuse. No new language permission, allocation, public ABI or design qualification; executable packs and broader proof coverage remain work. Prior capability reference: `7ef3e6cba11e016ea50948e4a8ee1bdaa28e9eca:tests/cod/language/README.md`.

## 2026-11-14 — Checked callable frames

<a id="k-663"></a>
**K-663 — Checked-frame continuity.** Agent assessment under H-382/H-398–H-410/H-420, D-14r91/D-17r57/D-18/D-21r11/D-26r13 and charter §§5.3/6.3/12: registry r670→671 and source reference284→285 record explicit input storage/passing and capture roles shared by callable propagation and native emission. Owner-name bindings remain distinct from payload storage. Preserve source signatures, owner/access proofs, completion, ordered preparation, cleanup and native reuse. Executable packs, broader proof coverage and permanent representations remain unresolved implementation/design scopes; no new language rule, allocation, ABI or design qualification. Prior capability reference: `942390d17aa8040827c49ba1072e9784ea33a881:tests/cod/language/README.md`.

## 2026-11-14 — Variadic declaration contracts

<a id="k-662"></a>
**K-662 — Variadic-signature continuity.** Agent assessment under H-258/H-261/H-420–H-424, D-10/D-14r91/D-17r57/D-18/D-24r7/D-26r13 and charter §§5.3/6.3/12: registry r669→670 and source reference283→284 record variadic signature obligations on fixed-arity implementations and type queries. Every actual tail argument retains type/access/owner obligations; empty tails invent no type, target or owner. Preserve original-input result promises, checked failures, preparation, cleanup and native identity. Executable packs, bounds/forwarding, variadic callable views, broader declaration coverage, regional layout and fractions remain outside this increment; no new allocation, public ABI or design approval. Prior capability reference: `73fb0baebedb03860db4d4c881d7593d6abf965f:tests/cod/language/README.md`.
## 2026-11-15 — Confirm staged initialization plan and synchronize development

<a id="h-447"></a>
**H-447 — Record and publish the two-stage decision.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. After the two-stage proposal, the user says: “按此记录决策，提交推送。 然后跟开发主线同步一次。” Confirms D-28-INIT/P-400's stated plan: common provable places/fields/aliases, branch joins, checked init helpers and finite tracked components when aggregate prerequisites exist; expand recognition on demand while preserving soundness. Unknown proofs reject distinctly from demonstrated invalid behavior; failure/cleanup, identity/readiness and mandatory-write requirements remain. Authorizes this documentation checkpoint and mainline notification. Exact incomplete-aggregate reservation, modifier combinations, broader publication and deferred pure mechanisms are not implicitly resolved.

<a id="k-init-confirm-20261115"></a>
**K-INIT-CONFIRM-20261115 — Confirmed planning scope and continuity.** Agent integration under H-447 and existing C-65/C-72/P-398/P-400, charter §§6.3/8/12: D-28r18→19/D-14r100→101/D-22r17→18 and registry r682→683 marks the stated coverage and staged soundness policy confirmed, retaining detailed mechanism exclusions and all unrelated qualifications. Route ordinary ownership/construction readers to the confirmed plan without changing current initialization/group rules. Remove a stale example comment about is integration; it remains a semantic-flow illustration. Baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6` at affected paths and preceding K-INIT-STAGES-20261115. Documentation only; no source/build/test changes.

## 2026-11-15 — Staged acceptance with invariant soundness

<a id="h-446"></a>
**H-446 — Basic acceptance first; improve recognition as needed.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. The user asks: “能不能有效分两阶段，先最小effort支持基本、常见场景的accept，偏门写法暂时reject；今后按需投入更多实现特别需要的场景的识别。 关键是soundness始终有保障。” D-28-INIT/P-400 records the requested staged direction and explicit soundness requirement. Exact coverage below is an engineering proposal; pending init syntax/publication rules and source implementation are not approved by this request.

<a id="k-init-stages-20261115"></a>
**K-INIT-STAGES-20261115 — Bounded proof first, precision later.** Agent integration under H-444–H-446, P-398–P-400 and charter §§6.3/8/12: D-28r17→18/registry r681→682 defines a phase-1 target using common place/field facts, branch joins and checked init helper summaries, including finite cyclic completion when aggregate prerequisites are ready. Unknown alias, loop, mutation and publication proofs reject; every accepted case still accounts for failures, cleanup, actual owners, reads and mandatory writes. Phase 2 improves recognition for demonstrated needs within the same semantics/analyzer. Distinguish unproved from proven-invalid diagnostics; no unchecked fallback, runtime init registry, qualifier erasure or deferred correctness repair. Earlier qualified scopes and D-16 remain unchanged. Baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6` at affected paths, preceding K-INIT-COST-20261115 and its immutable source evidence. Documentation only; no source/build/test changes or measured cost claim.

## 2026-11-15 — Combined initialization-flow cost review

<a id="k-init-cost-20261115"></a>
**K-INIT-COST-20261115 — Integrated feasibility and debt assessment.** Agent review requested by the current project user, under H-444/H-445, P-398–P-399 and charter §§3.3/4.3/6.3/8/12: D-28r16→17/registry r680→681 recommends shared local/field/call initialization analysis with finite cyclic completion, assessed as moderate incremental compiler work and controllable debt. Complete delivery still includes unfinished aggregate/layout prerequisites. Separate reference-slot contents, payload readiness and mandatory writes; preserve dependency checks at ordinary publication, failure/defer ordering, fixed owners and no rollback. Both identities may be reserved before linking, so forward-name groups are not inherently required. Arbitrary graph precision remains costly; no new runtime pointer flag or lazy evaluator is required by the bounded approach. This updates K-KNOT-COST-20261115's general-feature assessment using H-445's narrower model, without approving init syntax or publication rules. Current source evidence: `8914d0f3914d4586789f0461f187bb428fa54446:cod/lang_checked.hh:66–184,1143–1200,1361–1390,4480–4484,5319–5355`, `cod/lang_completion.hh:27–48`, `cod/lang_syntax.hh:160`. Published design baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6` at affected paths; preceding draft H-445/K-INIT-FLOW-20261115. Existing qualifications, D-16 validity and D-19-KNOT goals continue. Documentation/source inspection only; no code, build/tests, schedule estimate or measured cost claim.

## 2026-11-15 — Initialization obligations throughout control flow

<a id="h-445"></a>
**H-445 — Initialization flow and identity-only use.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamps/personal name unavailable. The user proposes init-qualified ref parameters with write-before-read and delegated initialization, clarifies that a.next = b and is can use b's identity without reading its payload, and states: “第一性原则触发，init 责任不仅是 `init` 参数显式传递，代码块内部流也传递并跟踪消除位置，之前之后有所区分，才对吧？” D-28-INIT/P-398 records the flow/identity principle. Complete parameter syntax, aggregate publication, exceptional guarantees and supported analysis scope are not thereby confirmed.

<a id="k-init-flow-20261115"></a>
**K-INIT-FLOW-20261115 — Flow model and bounded reassessment.** Agent integration under H-445/H-439, charter §§3.3/4.3/6.3/8 and existing initialization/cleanup contracts: D-28r15→16/registry r679→680 distinguishes initialized reference slots, selected target identity, payload readiness and mandatory-write obligations. Local flow, aliases and call summaries track discharge without copied permanent init flags. Correct the discussion's overly restrictive inference that storing an incomplete target's identity itself dereferences it; ordinary readable publication still needs proof. H-444's cost limit remains, with bounded flow support under review rather than a selected cyclic-construction mechanism. K-KNOT-COST-20261115 retains the general-feature assessment; existing qualified scopes and D-16 continue. Published baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6` at affected paths; preceding draft evidence H-444/K-KNOT-COST-20261115. Documentation only; no code, build or tests.

## 2026-11-15 — Cyclic construction cost and future pure knot-tying

<a id="h-444"></a>
**H-444 — Cost-bounded ordinary construction; pursue pure knot-tying.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. About required-ref cyclic construction: “这个现在评估看看实现难度和技术债务代价，如果不大就支持，费劲就算了。 这是说当前的 proc/func 范围。 将来的 pure (const func）需要认真考虑，追求支持类似 Haskell knot-tying 技巧。” This delegates a cost-based disposition for ordinary proc/func and separately confirms D-19-KNOT/P-395's future research goal. It does not approve a construction syntax, lazy evaluation model, publication protocol or implementation schedule.

<a id="k-knot-cost-20261115"></a>
**K-KNOT-COST-20261115 — Assessment and scoped continuity.** Agent assessment under H-444, H-317/P-284 and charter §§3.3/6.3/8/12: finite edge patching has low runtime complexity, but provisional-target checking, forward bindings, completion/escape proof and unfinished aggregate integration make current general proc/func support exceed the low-cost threshold. Retain nullable ptr cycles and defer staged required-ref publication; this is the agent's delegated conclusion, not a new human mechanism approval. D-28r14→15/D-14r99→100/D-22r16→17 preserve eager initialization and H-428 effects without rollback; D-19r3→4 and registry r678→679 record the separate pure goal, evaluation/lifetime questions and P-395–P-397. Prior design baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6` at affected paths, with preceding H-443 integration. Source evidence: `f6bb01009d93c46740b063b66fedfa02046143eb:cod/lang_checked.hh:1168–1174,1280–1300,4078–4166,4343–4345` and `cod/lang_llvm.hh:1332–1343`. H-317 original evidence: `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md:4722–4731`. Existing purity/TCO, owner/storage and D-16 contracts continue; detailed mechanisms remain unqualified. Documentation/source review only; no implementation, build/test execution, timing or performance claim.

## 2026-11-15 — Foreign interfaces adapt to CoD semantics

<a id="h-443"></a>
**H-443 — Prioritize CoD semantic completeness without extra foreign promises.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-15 America/Los_Angeles; exact message timestamp/personal name unavailable. Asked to choose a stable-storage restriction or lexical pinning for foreign address retention, the user says: “以CoD自身机制的语义系统完备为主，foreign接口不做额外承诺，把限制状况说清楚就行，外部语言系统机制跟CoD对接要适配CoD语义。” D-28-INTEROP/P-393–P-394 records the selected value/boundary: CoD semantics govern; external systems adapt and limitations are explicit. No language pinning, stable-address guarantee, altered allocation failure or general foreign ABI is selected. Generated native CoD execution retains its full correctness obligation.

<a id="k-foreign-adapters-20261115"></a>
**K-FOREIGN-ADAPTERS-20261115 — Boundary closure and continuity.** Agent integration under H-443/H-439 and charter §§2/4.3/6.3/8: D-28r13→14 and registry r677→678 remove foreign pinning/raw-address policy as an open core-language requirement. State adapter obligations across callback growth, owner replacement, access, layout and cleanup without modifying CoD behavior or granting hidden lifetime/storage changes. Existing D-28-SCOPE/OWNERS/ALLOC/IDENTITY and downstream qualifications continue; D-16 remains unchanged. Record required-ref cyclic construction as a separate pending CoD question, without selecting syntax or provisional-reference rules. Baseline: `8fb2bfd5120fb271af4fb12f38e7e9d498f3b2b6:design/RegionalAndLexicalStorageRefactor.md` and the same commit’s registry/changelog. Documentation only; compiler/build/tests untouched.

## 2026-11-14 — Unary negation spellings

<a id="h-442"></a>
**H-442 — Support not and ! with the same meaning.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact message timestamp/personal name unavailable. The user says: “`not` 和 `!` 都支持如何？ 语义相同”. Selected D-10 unary-negation scope/P-136: not and ! are spellings of the same unary operation. This replaces K-IS-NOT-20261114’s contextual-only not restriction, while H-441 is not remains the compound identity comparison.

<a id="k-unary-not-20261114"></a>
**K-UNARY-NOT-20261114 — Spelling equivalence and continuity.** Agent integration under H-442/H-439 and charter §§6.3/8: D-10r149→150/D-28r12→13 and registry r676→677 preserve unary precedence, operand typing, once-only evaluation, method identity, failures and refinement across not/!. Explicit/operator/intrinsic spellings share one operation; is not retains comparison precedence. Parenthesized negation of is null agrees with is not null. No truthiness, independent overload family, owner/lifetime or fraction change follows. Published baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2` at affected paths; preceding draft K-IS-NOT-20261114. Documentation only; compiler/build/tests untouched.

## 2026-11-14 — Identity negation

<a id="h-441"></a>
**H-441 — Add is not.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact message timestamp/personal name unavailable. The user says: “加入`is not` 操作符吧”. This extends D-28-IDENTITY/C-95/P-391–P-392 with is not, including is not null, as the exact negation of is. It changes no owner/locator, lifetime or payload-equality rule.

<a id="k-is-not-20261114"></a>
**K-IS-NOT-20261114 — Negation syntax and continuity.** Agent integration under H-441/H-439 and charter §§4.3/6.3/8: D-28r11→12/D-14r98→99/D-10r148→149/D-17r59→60 and registry r675→676 add the two-word comparison at is precedence, preserving once-only left-to-right reference selection, contextual null ownership and checked non-null refinement. not is contextual within this operator, without a new standalone negation form. Existing qualifications, graph-key/ABI exclusions and D-16 numeric/fraction constraints continue. Published baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2` at affected paths; preceding draft K-REFERENCE-IDENTITY-20261114. Documentation only; compiler/build/tests untouched.

## 2026-11-14 — Reference identity and unified null tests

<a id="h-439"></a>
**H-439 — Owner-plus-locator correctness and is.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact message timestamp/personal name unavailable. In response to language-provided reference identity and key support for graph libraries, the user says: “当然应该CoD提供机制，正确性就要求CoD引用每次解释都是当时的 owner + locator 决定，允许编译器优化但保持此唯一正确性判断语义。 类似的衍生决策你直接定，没有疑义吧。 另外 same_target 就不要了，引入 `is` 操作符”. Selected D-28-IDENTITY/C-95/P-391: actual owner plus target locator is the reference correctness criterion, optimization must preserve it, CoD provides the identity mechanism, and is is the comparison spelling. The user delegates clear derived decisions. This does not select a persistent ID, raw-address exposure, precise key carrier or complete lexical ADT layout.

<a id="h-440"></a>
**H-440 — Use is null as the sole null-test form.** Human/source: same chat/user/date as H-439. The user adds: “对了，以前的 is_null() 可以取消了，`is null` 就行了”. The selected source contract replaces the separate null-test intrinsic (spelled isnull in the existing prototype) with is null. H-439/P-392 retains non-dereferencing reference observation and established owner/lifetime/NPE constraints; no compatibility alias is selected. This records design, not a completed compiler migration.

<a id="k-reference-identity-20261114"></a>
**K-REFERENCE-IDENTITY-20261114 — Identity/null integration and continuity.** Agent derivation under H-439/H-440/H-423 and charter §§2/4.3/6.2–6.3/8/12: D-28r10→11/D-14r97→98/D-22r15→16, D-10r147→148/D-17r58→59/D-18r93→94/D-23r12→13/D-26r15→16 and registry r674→675 integrate is, contextual is null, selected operand order, actual-owner/null distinctions, growth, access, graph keys and unchanged payload/type semantics. Remove the open question of whether CoD provides identity. The remaining key/native/layout transport is separate; no graph copier, generation registry, hidden allocation, lifetime extension or fraction change follows. Clarify K-OWNER-ALLOCATION-20261114's val-example rationale: existing val storage can be borrowed and compared; only independent values lack a target. Its ref-based example remains valid and does not revoke borrowing val storage. Acceptance covers aliases/equal bits, selected targets across mutation/growth, subobjects, null, owner replacement, keys and unknown-payload references. Published baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2` at affected paths; preceding draft assessment K-OWNER-ALLOCATION-20261114. Documentation/source inspection only; compiler, builds and tests untouched.

## 2026-11-14 — Owner qualifiers govern allocation

<a id="h-438"></a>
**H-438 — Construction access from qualified owners.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamps/personal name unavailable. The user states: “我倾向于：参数里有至少一个非 const 引用 in O 的话，就能允许 where { owner O }，所有 in O 引用都是 const 的话，只能允许 where { const owner O }； const owner 不可分配，非const owner都可分配。” When the assistant identifies deliberate const owner with writable targets as a boundary to discuss, the user responds: ““a callable may deliberately choose `const owner O` while still accepting writable targets” ? why not?” Accepted local direction: writable-target inputs permit a non-const owner; all-readonly inputs require a const owner; const owner forbids allocation and non-const owner permits it. Explicitly choosing const owner with writable targets is allowed. D-28-ALLOC/C-94/P-389–P-390 records this source rule, replacing the open generic allocation-capability delivery choice while preserving ordinary target access, placement and lifetime rules. Exact allocator ABI, callable-qualifier matching, graph identity and lexical ADT layouts are not selected.

<a id="k-owner-allocation-20261114"></a>
**K-OWNER-ALLOCATION-20261114 — Qualified allocation and continuity.** Agent integration under H-438/H-423 and charter §§2/4.3/6.2–6.3/8/12: D-28r9→10/D-14r96→97/D-22r14→15 and registry r673→674 distinguish fixed owner identity, target access and allocation permission. Remove the open builder/signature alternative; generic owner construction no longer requires an extra builder solely for permission. Preserve const-owner restrictions through forwarding/null/aliases and shared owner contexts; independently writable paths remain usable. Update D-14's owner example to materialized ref targets rather than owner-free val inputs. Selected rules add no root-policy exposure, capture permission, region growth, graph copier, rollback or cleanup transfer. D-16 and unrelated qualifications continue; D-28-SCOPE/D-28-OWNERS remain scoped and exact native/callable integration stays open. Published baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2` at affected paths; preceding draft assessment K-SHARED-OWNER-20261114. Documentation only; no implementation/build/test claim.

## 2026-11-14 — Shared actual-owner runtime information

<a id="h-437"></a>
**H-437 — Track owner relations and share dereference information.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact message timestamp/personal name unavailable. The user says: “编译器应该跟踪所有的 `in o` 声明，让运行时代码合并跟踪实际 owner 的身份和地址，保证所有 deref 都有充分且正确的运行时信息，合理避免为每个指针单独跟踪、存储owner的运行时开销。” They request documentation updates and one-by-one discussion of unresolved questions with CoD examples. Accepted D-28-OWNERS/C-93/P-387 local requirement and rationale: compiler-tracked owner relations support sufficient dereference information with shared actual-owner identity/address state, avoiding redundant per-pointer owner storage. P-388 preserves existing unique-owner, slot/content, access and lifetime constraints. Exact descriptor/ABI, allocation capability, graph identity API and native-address exposure are not selected.

<a id="k-shared-owner-20261114"></a>
**K-SHARED-OWNER-20261114 — Owner-sharing integration and contract continuity.** Agent assessment under H-437/H-423 and charter §§2/4.3/6.2–6.3/12: D-28r8→9 adds the selected sharing requirement and ordered source questions. D-14r95→96/D-18r92→93/D-22r13→14/D-23r11→12/D-26r14→15 preserves qualified owner, capture, construction, result and coverage behavior while carrying the requirement into their integration obligations. Registry r672→673 records independently scoped D-28-OWNERS/C-93/P-387–P-388; D-28-SCOPE continues unchanged. Acceptance covers inferred/inherited owners, distinct recursive instances, forwarding, null, shared fields, calls/results/defer and current backing after growth. Static elision remains available; sharing grants no allocation/lifetime permission and fixes no physical ABI. D-16 validity and unrelated qualifications remain. Published baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2` at affected paths; preceding integrated draft assessment: K-POINTER-GLOBAL-20261114. Documentation only; no implementation or measured-overhead claim.

## 2026-11-14 — Integrated pointer and lexical ADT design review

<a id="k-pointer-global-20261114"></a>
**K-POINTER-GLOBAL-20261114 — Global impact and contract continuity.** Agent assessment under the current user's documentation-only integrated-review request, H-423/H-429/H-433–H-436 and charter §§2/3.3/4.3/6.2–6.3/7.1–7.2/8/12: D-28r7→8 combines lexical pointer-bearing ADT feasibility with slot/target operations, calls/results, graph identity/transfer, capabilities, cleanup, captures/TLS, native exposure, persistence and migration. D-22r12→13/D-14r94→95 links construction and assignment integration. D-17r57→58/D-18r91→92/D-23r10→11/D-26r13→14 continues qualified behavior with targeted proof obligations; it selects no new placement, capture, allocator, graph algorithm or ABI. Registry r671→672 preserves independently confirmed D-28-SCOPE and all unrelated qualifications. Acceptance distinguishes backing growth from object movement/owner replacement and covers optimized/materialized forms, all transfer pairs, null/self-edge encoding, sharing/cycles and resource failures; D-16 constraints remain. Baseline: `cd84f93f852b98e6ea9edd828125b00deca24bb2:design/RegionalAndLexicalStorageRefactor.md` and the same commit's affected paths. Source inspection and documentation checks only; no code/build/test or performance evidence.

## 2026-11-14 — Lexical pointer checkpoint integration

<a id="k-lexical-checkpoint-sync-20261114"></a>
**K-LEXICAL-CHECKPOINT-SYNC-20261114 — Preserve numeric mainline continuity.** Agent reconciliation of the H-433–H-436/D-28r7/D-22r12/D-14r94 documentation checkpoint with `73fb0baebedb03860db4d4c881d7593d6abf965f:design/DesignAuthorityRegistry.md`: registry r671 combines the design draft's r670 scope with mainline r669/K-660/K-661 and source reference283. All selected versus pending pointer/layout boundaries and independent numeric contracts retain their scope. Documentation overlap only; no source/build/test change or new design approval.

## 2026-11-14 — Lexical slot location and reference contents

<a id="h-436"></a>
**H-436 — Separate slot addressing, payload interpretation and optimization.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user says: “lexical allocation 和 offset ptr 语义还要明确： 不排斥 register/stack allocation 优化可能性； offset 仅限找到 lexical 变量存储位置，其实际内容是 ref/ptr 时，只有同 local scope 为 owner 时按相同 offset 语义存储和deref，指向其它 owner 归属内容时，另外按不同的语义解释。” This clarifies H-435: physical register/stack optimization remains available; a lexical storage locator does not determine the meaning of its contents; only same-local-owner ref/ptr contents use the same offset convention. Other-owner carriers, native ABI and optimization algorithms are not selected, and same-owner ADT-field constraints are not relaxed.

<a id="k-lexical-slot-payload-20261114"></a>
**K-LEXICAL-SLOT-PAYLOAD-20261114 — Representation-boundary clarification.** Agent integration under H-436, H-435/H-423 and charter §§2/3.3/4.3/6.2–6.3/7.1/8/12: D-28r6→7/D-22r11→12/D-14r93→94 and registry r669→670 distinguish storage locators, typed bits, same-scope offset payloads and other-owner references. Record caller/callee representation adaptation, aliases of slots versus new bindings, independent slot/target growth and optimization-equivalence acceptance. Preserve dynamic capacity, unique owners, H-428 effects, region resize, D-16 validity and unchanged capture/TLS/field-owner restrictions; no mandatory heap frame or inter-owner ABI follows. Published baseline: `733826b30b9880d930443a6545b249af0c977f48` at the affected paths; preceding scoped draft decision: K-SCOPE-OFFSET-ADOPT-20261114. Documentation only; no implementation/build/test claim.

## 2026-11-14 — Adopt owner-base lexical references

<a id="h-435"></a>
**H-435 — Independent lexical-capacity and offset decision.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user says: “这个方案几乎是可以独立确认采纳的，lexical scope 容量动态分配是已确认语言特性，我看目前 offset ptr 是最优实现方式。” This reaffirms dynamic lexical capacity and selects the H-434 owner-base-offset direction independently of the wider ADT refactoring. Its local rationale is preserving lexical variables/targets while replacing backing; it does not select anchor ABI, exact allocator/capability syntax, native interop, growth policy or a default graph-transfer algorithm. The preference is a design judgment, not benchmark evidence.

<a id="k-scope-offset-adopt-20261114"></a>
**K-SCOPE-OFFSET-ADOPT-20261114 — Selected scope and mainline continuity.** Agent integration under H-435/H-423 and charter §§2/3.3/4.3/6.3/7.1/8/12: D-28r5→6/D-22r10→11/D-14r92→93 and registry r668→669 record D-28-SCOPE/C-92/P-386 independently selected lexical requirements. Retain actual-owner base resolution, semantic identity across growth, selected-place preservation and existing-target validity until replacement is ready as direct consequences; physical anchor/allocation/native protocols and broader qualification remain open. H-433 lexical ADTs remain the target; H-206/H-208/H-351 library graph policy, H-428 effects, region resize and D-16 validity remain separate. Reconcile drafts with `733826b30b9880d930443a6545b249af0c977f48:design/DesignAuthorityRegistry.md` and its K-659/source reference281, preserving all evidence; dedicated branch fast-forwarded to that published main commit. Documentation only; no implementation/build/test claim.

## 2026-11-14 — Lexical owner-base offsets

<a id="h-434"></a>
**H-434 — Evaluate relocatable lexical storage fragments.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user proposes: “有一个技术处理方案是： lexical scope owner 的指针变量，其指针值也不是global/direct，而是相对于当前 lexical scope storage 的 heap frag base addr 的 offset，这样 heap frag ，可以随着分配需要重新从heap分配，而不影响 lexical 变量”. This places the lexical direct/global physical candidate under renewed review in favor of considering owner-base offsets and reallocatable lexical payload fragments. Stable owner anchors, cross-call address invalidation, allocation/failure protocol and raw native-address exposure are not yet selected. Region-only relative graphs and explicit consuming region resize remain separate.

<a id="k-scope-offsets-20261114"></a>
**K-SCOPE-OFFSETS-20261114 — Relocation feasibility and authority boundary.** Agent assessment under H-433/H-434 and charter §§2/3.3/4.3/6.3/7.1/8/12: D-28r4→5/D-22r9→10/D-14r91→92 and registry r667→668 review target-owner base resolution, stable anchors, retained selected places across allocation/calls, native-address exposure, all four transfer pairs and scope cleanup. P-382's lexical direct encoding needs reconfirmation; storage separation, unique owners, regional resize/cleanup, D-16 validity and unrelated qualified scopes remain. Candidate lexical growth retains semantic owner identity and may preserve old backing on pre-publication allocation failure; it is not consuming region resize, graph copying or H-428 rollback. Baseline: `8209c5ff4ff22c6275180114e86e40bc88b62dc2` at affected design/registry paths; preceding evaluation: K-POINTER-LAYOUT-20261114. No source/build/test or measured-performance claim.

## 2026-11-14 — Lexical pointer-bearing ADT feasibility

<a id="h-433"></a>
**H-433 — Target lexical ADTs with automatic pointer addressing.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. Asked about one nominal ADT in both storage domains, the user requests: “评估技术可行性及UX友好性： ADT 在 lexical scope 中分配时，使用 direct ptr ，而在 region 中分配时，使用 relative ptr，需要 CoD 自动识别和处理 bits/graph transfer 过程中的指针寻址方式差异”. They clarify: “保守兜底是 lexical scope 只允许分配 bits ADT，但是理想目标是 lexical scope 也能支持含指针的 ADT”. This establishes the design target/fallback and authorizes feasibility/UX evaluation. It does not select a physical layout carrier, scope allocation API, cross-owner fields or a default graph-cloning algorithm; H-206/H-208/H-351 library transfer remains the baseline.

<a id="k-pointer-layout-20261114"></a>
**K-POINTER-LAYOUT-20261114 — Layout and transfer feasibility review.** Agent assessment under H-429/H-433 and charter §§2/3.3/4.3/5.3/6.3/7.1/8/12: D-28r3→4/D-22r8→9 and registry r666→667 distinguish target layout, selected slot encoding and semantic owner/access evidence. Evaluate one nominal ADT, automatic field/transfer addressing and all four lexical/regional transfer pairs; retain H-206/H-208/H-351 library topology/allocation policy and H-428 failure effects. Caller-scope construction capability and native layout evidence are material open choices; no lexical heap lifetime, capture/TLS extension, graph copier or pointer ABI is qualified. D-16 bits validity and unrelated qualified contracts continue. Baseline: `8209c5ff4ff22c6275180114e86e40bc88b62dc2:design/RegionalAndLexicalStorageRefactor.md` and the same commit's affected paths. Source inspection only; no implementation/build/test or performance claim.

## 2026-11-14 — Lossless method selection

<a id="k-661"></a>
**K-661 — Numeric-dispatch continuity.** Agent assessment under H-258/H-261/P-231/H-420–H-424, D-10/D-14r91/D-17r57/D-18/D-24r7/D-26r13 and charter §§5.3/6.3/12: registry r668→669 and source reference282→283 record shared exact-before-lossless candidate selection, normalized formal inputs and preserved original-input declaration promises. Preserve same-T, fixed callable signatures, owners/access, ordered preparation, result independence, checked failures, cleanup and finite native sharing; partial declaration domains remain unproved. No new conversion policy, quantity/fraction mechanism, regional representation, callable adapter or permanent ABI. Prior capability reference: `19289bb8b62de12538b26f5704da02e66ecaafb0:tests/cod/language/README.md`.

## 2026-11-14 — Lossless numeric value preparation

<a id="k-660"></a>
**K-660 — Numeric-value continuity.** Agent assessment under H-261/P-231/H-424, D-10/D-14r91/D-17r57/D-18/D-24r7/D-26r13 and charter §§5.3/6.3/12: registry r667→668 and source reference281→282 record lossless primitive arguments through established callable views and one least-common-value calculation for builtin numeric operands and inferred results. Preserve preparation order, exact borrowed targets, semantic identity, exception/cleanup behavior and native sharing. Named-method widening stays unproved and cannot silently permit builtin fallback. D-11/D-27 whole-document qualification, quantities/fractions, D-28 layout/resize mechanisms and permanent ABI remain separate; no new language conversion or design approval. Prior capability reference: `733826b30b9880d930443a6545b249af0c977f48:tests/cod/language/README.md`.

## 2026-11-14 — Declaration input-domain proofs

<a id="k-659"></a>
**K-659 — Declaration-coverage continuity.** Agent assessment under H-258/H-420–H-424, C-84/P-356, D-10/D-14r91/D-17r57/D-18/D-24/D-26 and charter §§5.3/6.3/12: registry r666→667 and source reference280→281 record contained owner/access domains, disjoint destination contracts and independent broad exception inputs. Reuse existing argument-admission rules; preserve method permissions, actual owners, result contracts, checked failures, dispatch and native coverage. Partial applicability remains unproved; a common exception view does not establish same-T. H-430–H-432 and D-28r3 retain the integrated growth/consumption decisions and pending resize/layout mechanisms. No new source permission, owner union, conversion, capture mechanism, regional representation, fraction behavior or permanent ABI. Prior capability reference: `c246909498a9abd3c8bf1248f54e82951621624e:tests/cod/language/README.md`; integrated design: `8209c5ff4ff22c6275180114e86e40bc88b62dc2`.

## 2026-11-14 — Resize design and mainline synchronization

<a id="k-resize-sync-20261114"></a>
**K-RESIZE-SYNC-20261114 — Preserve independent mainline continuity.** Agent reconciliation of K-EXPLICIT-GROW-20261114/K-RESIZE-20261114 with K-658 at `c246909498a9abd3c8bf1248f54e82951621624e:design/DesignAuthorityRegistry.md`: registry r666 combines the design branch's r665 scope with mainline r664/source reference280. D-28r3's capability pointer is refreshed; D-14r91/D-17r57/D-22r8 and all pending representation/capacity boundaries remain unchanged. This resolves documentation overlap only and adds no source, build, test or implementation authority.

## 2026-11-14 — Lossless consuming region resize

<a id="h-432"></a>
**H-432 — Unify capacity growth and shrinkage.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user says: “grow(r) 签名不全，或者考虑一个统一的 resize(r, new_cap) 兼顾扩容和缩减容量，当然需要无损，否则该抛异常抛异常”. This proposes replacing the growth-only shape in H-430 with one explicit-capacity resize covering lossless growth and shrinkage, with exceptions when it cannot succeed. It retains H-431 ordinary consuming cleanup; exact capacity units, shrink algorithm, equal-capacity treatment, error mapping and backing protocols are not separately selected by this statement.

<a id="k-resize-20261114"></a>
**K-RESIZE-20261114 — Resize integration review.** Agent assessment under H-423/H-430–H-432 and charter §§3.3/4.3/6.3/7.1/8/12: D-28r2→3/D-22r7→8 proposes exact payload capacity using D-17 E5 units, offset-preserving shrink down to the retained allocated extent, no implicit compaction, uniform consuming ownership for equal capacity and reuse of existing failure categories. D-14r90→91/D-17r56→57 and registry r664→665 reconcile names/references while continuing existing entry, cleanup and owner rules; new detailed resize/backend scope remains under review. Successful losslessness does not restore consumed input on failure, including rejected shrink. Acceptance covers extent/capacity/alignment/overflow boundaries, graph/value preservation, stale views and exact resource cleanup. D-16 validity and unrelated qualified contracts remain unchanged. Committed baseline: `798867ecc2fe8656598b3a3709a5dd7213e26576:design/RegionalAndLexicalStorageRefactor.md` and the same commit's affected design/registry paths; preceding draft review: K-EXPLICIT-GROW-20261114. No implementation, build or test claim.

## 2026-11-14 — Explicit consuming region growth

<a id="h-430"></a>
**H-430 — Growth replaces an owned region.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user says: “"automatic growth with live references" 这个问题我倾向于是禁止，CoD语言层面不允许 region 的 inplace growth ，只能将 `owned region` ref 传给一个 intrinsic `func grow(r:owned region:R) -> owned region:R where {R}` 得到一个新的， grown region。” This selects the explicit consuming-replacement direction rather than automatic/in-place language growth. Growth size, exact grammar, no-op requests, allocator/backing-file protocol and physical address behavior are not specified.

<a id="h-431"></a>
**H-431 — Keep ordinary owned-parameter failure cleanup.** Same human/chat/date as H-430. Asked whether failure after grow takes ownership should release the input under the existing consuming-call rule or require recovery of the original region, the user selects “Use ordinary owned-parameter cleanup (Recommended)”. This accepts callee cleanup with no implicit return of input ownership on failure. The subsequent question asking to verify irreversible call transfer and owning return only on success is checked against D-17 E4; it does not supply a new argument-evaluation rule.

<a id="k-explicit-grow-20261114"></a>
**K-EXPLICIT-GROW-20261114 — Growth and ownership-boundary review.** Agent assessment under H-430/H-431, H-423 routine-consequence delegation and charter §§3.3/4.3/6.3/7.1/8/12: D-22r6→7/D-28r1→2 and P-385 replace automatic/live-borrow growth alternatives with consuming replacement and ordinary failure cleanup. D-14r89→90 distinguishes replacement from H-263 same-region owner forwarding; D-17r55→56 clarifies existing pre-entry preparation, consuming entry and result delivery after successful cleanup. Registry r663→664 preserves unrelated qualifications. Fresh output owner/views and no use of old views after replacement are recorded as direct replacement consequences, not new blanket approval of carriers/ABI. Acceptance covers pre-entry and post-entry failure, abandoned owning results, exact release, graph preservation and rejected old-view uses. Current source reference279 excludes regional values; no grow implementation or test claim. Baseline: `798867ecc2fe8656598b3a3709a5dd7213e26576` at the affected design paths and registry. No source/build/test change.

## 2026-11-14 — Operator signature obligations

<a id="k-658"></a>
**K-658 — Operator-declaration continuity.** Agent assessment under H-258/H-420–H-424, C-84/P-356, D-10/D-14/D-17/D-18/D-24/D-26 constraints and charter §§5.3/6.3/12: registry r663→664 and source reference279→280 extend shared signature obligations to eager operators and bits swaps. User methods reuse declared results; builtin signatures remain established and satisfy applicable contracts, including declared checked failures. Explicit intrinsic bypass, dispatch, argument sequencing, result conversion, owners/access, cleanup and native coverage remain unchanged. Broader declaration applicability, abrupt-operand builtin evidence, local declarations, D-24 whole-document qualification and D-28 representation decisions retain their separate boundaries. No new conversion, capture permission, regional mechanism, fraction behavior or permanent ABI. Prior capability reference: `798867ecc2fe8656598b3a3709a5dd7213e26576:tests/cod/language/README.md`.

## 2026-11-14 — Independent expected callable results

<a id="k-657"></a>
**K-657 — Contextual-result continuity.** Agent assessment under H-420–H-424, C-84/P-356 and charter §§5.3/6.3/12: registry r662→663 and source reference278→279 cover missing result contracts supplied at fresh callable definitions, retained generic context and unchanged existing callable signatures. Preserve parameter/kind/effect contracts, unique owners, access/initialization, ordered evaluation, cleanup and finite shared native coverage; fixed contracts need no added native context input. H-428/H-429 and D-14r89/D-17r55/D-23r10/D-28r1 retain their qualified versus pending boundaries. This increment chooses no storage layout, live-growth mechanism, C++ rename, RHS normalization, fraction conversion or permanent ABI. Prior capability reference: `1b43a1737de90f2791936e2e78135c49e3fe3d2c:tests/cod/language/README.md`; integrated design scope: `1379158e0397b3da00272e320b522702ff0a8e1e`.

## 2026-11-14 — Distinct regional and lexical storage

<a id="h-429"></a>
**H-429 — Separate storage domains and track the refactoring.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user says “这个问题让我觉得要撤回之前说的让 region 去 back lexical scope 存储的想法了，两者还是有本质区别，region 只能是 relative/local ptr，lexical scope 要改为都是 direct/global ptr，这是个概念重构，影响面和改动面会比较大，值得专门开一个重构文档跟踪吧，你看看呢”. They then propose “C++ `global_ptr` 可以考虑改名为 external_ptr，让整体概念不冲突，你考虑上”. This withdraws H-426's scope-region backing proposal, directs distinct pointer/storage concepts and authorizes a dedicated draft; external_ptr is a naming proposal. Exact carrier/layout rules, lexical non-bits ADTs and the rename implementation are not yet selected. A subsequent instruction authorizes committing/pushing confirmed decisions, requesting mainline merge and asking the main chat to assess pausing affected development until the conceptual refactoring is settled.

<a id="k-storage-split-20261114"></a>
**K-STORAGE-SPLIT-20261114 — Refactoring draft and current-state reconciliation.** Agent assessment under H-429 and charter §§2/3.3/4.3/5.3/6.3/7.1/8/12: add D-28r1/C-91/P-381–P-384 with scoped direction versus pending value/ABI qualification; update D-22r5→6 and registry r661→662. Remove implicit scope-region/capacity-hint proposals from current construction text, retain explicit-region growth as a separate open topic, and preserve H-428 allocation effects, no move/graph copy, TLS boundaries and D-16 validity. D-28 tracks slot representation versus target ownership, C++ naming, affected designs/native paths, migration and acceptance; unrelated qualifications continue. D-14r88→89/D-17r54→55/D-23r9→10 carry H-428 allocation effects and explicit D-22/D-28 review boundaries into the applicable contracts without selecting the unfinished RHS or representation changes. Historical scope-region proposal: H-426/H-427/K-SCOPE-REGION-20261114. Cumulative committed baseline: `6aa46f0f5e0dc8bb02b0bdab15fd81e9c0b42209:design/ConstructionAndAcquisition.md` and the same commit's registry. Rebased on main `1b43a1737de90f2791936e2e78135c49e3fe3d2c`, retaining K-655/K-656 and source reference278. Documentation only; no compiler/allocator/build/test change.

## 2026-11-14 — Initialization allocation is an allowed effect

<a id="h-428"></a>
**H-428 — No language-level allocation checkpoint.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. After the older-field publication counterexample, the user decides: “确实，那就不引入 alloc checkpoint 语义了，CoD语言层面，分配作为允许的初始化副作用，有需要的话让用户或者第三方库自己去处理撤销分配的框架。” This rejects the H-426 checkpoint proposal and settles allocation as an allowed initialization effect, leaving undo frameworks to users/libraries. It does not withdraw RHS normalization, scope regions or capacity-growth discussion, or waive ordinary lifetime/initialization validity and whole-region acquisition cleanup.

<a id="k-init-effects-20261114"></a>
**K-INIT-EFFECTS-20261114 — Effect-boundary integration.** Agent assessment under H-428 and charter §§3.3/4.3/6.3/7.2/8/12: D-22r4→5 and registry r660→661 remove checkpoint/publication-rollback alternatives from current contracts and pending questions. Acceptance now preserves earlier valid publication/allocation in an existing live region after a later initializer fails, while incomplete bindings stay unavailable and D-17 whole-region cleanup remains exact. D-14/D-23 RHS, D-01 growth and D-10/D-18 scope/TLS/capture proposals retain their pending boundaries; no move or graph-copy semantics follows. This resolves the affected failure proposal without invalidating unrelated qualifications. Historical proposal: H-426/H-427/K-SCOPE-REGION-20261114; committed design baseline remains `6aa46f0f5e0dc8bb02b0bdab15fd81e9c0b42209:design/ConstructionAndAcquisition.md`. No source/build/test change.

## 2026-11-14 — Value normalization, group checkpoints and scope regions

<a id="h-426"></a>
**H-426 — Broaden the unified-owner proposal.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact timestamp and personal name unavailable. The user proposes “Automatically copying only when rhs is `val` or `const ref`”, `(local)` as the copy expression, an allocation checkpoint with whole-group rollback, and scalar conditional normalization toward const-ref/val. They explicitly identify storage/processing lifetime separation as an intentional “innovation token” for libraries/frameworks. They further propose every lexical scope visibly using a region, non-bits local ADTs, default automatic capacity growth with fixed-capacity opt-out, compiler/user capacity hints, and retaining TLS bits/callable categories plus explicit `in <some-owner>` for symbolic references. These statements authorize design exploration and establish the intended value; they do not select publication/rollback mechanisms, a live-growth implementation, exact grammar/ABI, broader TLS backing or graph-copy semantics.

<a id="h-427"></a>
**H-427 — Grouped values, no move semantics and joint initialization.** Same human/chat/date as H-426. In response to whether `(local)` should yield a readonly view or merely group, the user says “grouping 和 val 不冲突吧，结果类似 C++ 的 "rvalue" 语义，有没有解决问题？” and then explicitly corrects “注意CoD不像C++那样有 moving 意义”. In response to pre-commit publication restrictions, they say “initialization group 的意思就是一起生效，你能举出 early publication 的例子吗？” This clarifies the intended grouped-value/no-move model and joint effectiveness; it does not yet choose between preventing an initializer's writes from publishing new targets and staging/undoing such writes. D-22 supplies the requested older-object-field counterexample and remaining integration boundaries.

<a id="k-scope-region-20261114"></a>
**K-SCOPE-REGION-20261114 — Current proposal and dependency review.** Agent assessment under H-426/H-427 and charter §§2/3.3/4.3/5.3/6.3/7.2/8/12: D-22r3→4 and registry r659→660 replace superseded draft alternatives with RHS normalization, group checkpoints and exposed scope-region proposals. The cumulative D-22 change since its committed r2 baseline preserves unique owners, access, explicit cleanup, regional graph/no-copy/no-move rules and D-16 validity. It records candidate P-348/P-350 scope, D-14/D-23 receiving changes, D-17 rollback integration, D-01 live-growth constraints and D-10/D-18 TLS/capture boundaries as under review; unrelated qualifications continue. Acceptance includes early publication, cleanup, nested/shared allocation, value/alias identity and live views across growth. No compiler, allocator, build or test change. Committed baseline: `6aa46f0f5e0dc8bb02b0bdab15fd81e9c0b42209:design/ConstructionAndAcquisition.md` and the same commit's registry; the preceding draft review is K-UNIFIED-INIT-20261114.

## 2026-11-14 — Unified initialization review

<a id="h-425"></a>
**H-425 — Prefer one lexical/regional initialization model.** Human/source: current project user in [this design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), recorded 2026-11-14 America/Los_Angeles; exact message timestamp and personal name unavailable. After automatic same-region scalar-target materialization versus explicit target construction was presented, the user says: “我倾向于前一个选择，在变量初始化相关的内存分配语法、语义上， region与lexical scope尽量合在一个口径下表述，使用相同的心智模型，你推演各种细节看看有没有quirks”. This records the preferred direction and requests analysis; it does not confirm the subsequently drafted ordering, conditional-join policy, allocator protocol, aggregate ABI or whole document.

<a id="k-unified-init-20261114"></a>
**K-UNIFIED-INIT-20261114 — Draft analysis and preserved boundaries.** Agent review under H-425 and charter §§3.3/4.3/5.3/6.3/7.2/8/12: D-22r2→3 and registry r658→659 replace the explicit-only field recommendation with a pending unified initialization proposal on candidate P-348/P-350; P-351 adds D-23 as a dependency. It distinguishes existing-target aliasing from fresh scalar placement, inline bits from reference targets, owner identity from allocation access, and initial construction from later assignment. Review covers explicit copies, name/order/access rules, allocation-before-RHS effects, retained capacity after failure and mixed conditional alternatives. D-14/D-23's confirmed scope, D-17 cleanup, D-01 relocation/region lifetime and D-16 validity remain unchanged; dependent construction extensions remain pending. Acceptance is documented; no implementation, build or test evidence is claimed. Prior text: `6aa46f0f5e0dc8bb02b0bdab15fd81e9c0b42209:design/ConstructionAndAcquisition.md` and the same commit's `design/DesignAuthorityRegistry.md`.

## 2026-11-14 — Declared callable result contracts

<a id="k-656"></a>
**K-656 — Declaration-result continuity.** Agent assessment under H-258/H-420–H-424, C-84/P-356, existing D-10/D-14/D-17/D-18/D-26 constraints and charter §§5.3/6.3/12: registry r659→660 and source reference277→278 record module-declaration result establishment, per-instance value/view/owner contracts and preserved checked-failure obligations. Declarations remain constraints rather than executable methods; native cleanup restoration has a deterministic field order. Preserve ordinary dispatch, explicit returns, existing conversions, owner/access/initialization, preparation, cleanup and native coverage. Broader declaration applicability, independent expected contracts, non-scalar classification, recursive proof coverage and D-24's whole-document value path retain their separate status. No new conversion, owner union, capture permission, fraction mechanism or permanent ABI. Prior capability reference: `f4b427b8ee7d305d82f8ac4f87ef28fad94e52cd:tests/cod/language/README.md`.

## 2026-11-14 — Independent callable result evidence

<a id="k-655"></a>
**K-655 — Scoped inference continuity.** Agent assessment under H-420–H-424, C-84/P-356, existing D-14/D-17/D-18/D-26 constraints and charter §§3.3/5.3/6.3/12: registry r658→659 and source reference276→277 record bounded body-result inference across callable forms, independent recursive anchors and existing-conversion numeric joins. Preserve explicit returns, owner/access/initialization, ordered preparation, cleanup and finite native coverage. Separate declaration/expected-contract establishment, broader recursive proofs, non-scalar classification and D-24's whole-document value path remain outside this implementation increment. No new conversion, owner union, capture permission, fraction mechanism or permanent ABI. Prior capability reference: `d18e21c18613142f78fa756738d5148e19ec70d5:tests/cod/language/README.md`.

## 2026-11-14 — Common-result compiler-reference alignment

<a id="k-654"></a>
**K-654 — Current-contract continuity.** Agent assessment under H-424 and charter §§3.3/5.3/6.3/8/12: source reference275→276 and registry r657→658 align the compiler boundary with D-24r7/D-26r13's confirmed unique least common result rule. Inference remains implementation work; existing scalar/owner behavior and scoped authority remain unchanged. Preserve independent recursive anchors, existing conversions, return-boundary sequencing, exact owners/access and native coverage. No design alternative is reopened or broader mechanism qualified. Integrated design source: `97bf56dfd0e1799bd37aa7fba95a7c42d58c1393`.

## 2026-11-14 — Unique least common callable results

<a id="h-424"></a>
**H-424 — Infer the unique least common result.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable message URL unavailable. The user replies “同意” to inferring a unique least common result using already-permitted conversions, with an explicit contract required when no unique candidate exists. The preceding question contrasts u32/u64 returns inferring val u64 with requiring `-> u64`, and explains that adding a wider return may change downstream overload selection even for the original branch. This confirms that policy and trade-off, not new conversions, union types, owner alternatives or broader representation choices.

<a id="k-common-result-20261114"></a>
**K-COMMON-RESULT-20261114 — Common-result integration.** Agent assessment under H-424 and charter §§3.3/5.3/6.3/8/12: registry r656→657 and D-24r6→7 replace the alternatives with unique least common results of independent anchors under existing compatibility. Acceptance covers branch order, numeric/quantity widening, absent/incomparable candidates, explicit contracts, recursive-anchor preservation, return-boundary conversion and downstream dispatch. D-26r12→13/P-373 aligns the dependency without changing qualified native coverage. Owners/access, cleanup, nominal identity and deferred D-16 fraction rules remain; no source/build/test change. Prior text: `93ae4ec60b3a7efeaaedb8aeeeaeb64ed77af5ad` at `design/CallableResultInference.md`, `design/GenericAndRecursiveClosure.md` and `design/DesignAuthorityRegistry.md`.

## 2026-11-14 — Callable result-contract design integration

<a id="k-653"></a>
**K-653 — Result-contract integration continuity.** Agent assessment under H-420/H-421–H-423 and charter §§3.3/5.3/6.3/8/12: registry r653/r655→656, source reference274→275 and D-23's two r8 revisions→r9 reconcile the confirmed result-inference contract with K-652's implemented scalar/owner capability. D-24r6/D-26r12/D-07r155 retains the delivered scope, including independent recursive anchors, declared/expected contracts and the delegated no-value-completion consequence. Different-type common-result synthesis remains open; no whole-document qualification, inference implementation or permanent ABI is added by this merge. Source/compiler fixtures are unchanged. H-413–H-422's immutable evidence locators, H-423, all prior K records and scoped exclusions remain. Design input: `825f7a2a0f8b81800b7beb146b014c6c585b8e00`; validated compiler input: `5301e754e009c73d60e52fee029f27a4a40210d1`.

## 2026-11-14 — Scalar results, receiving storage and null owners

<a id="k-652"></a>
**K-652 — Scoped compiler and capability continuity.** Agent assessment under H-413–H-420 and charter §§3.3/4.3/5.3/6.3/8/12: source reference273→274, registry r652→653, D-14r87→88 and D-23r7→8 record primitive scalar value/borrowed result modes, val/ref initialization operators and statically unique/null-owner checking. Callable value-passing mode is independent of target access; prepared operand evidence preserves qualified null owners. Source fixtures use the confirmed syntax without a compatibility parser. Focused source/native, failure-injection, sanitizer and staging acceptance distinguish saved bits from saved targets, preserve cleanup and reject invalid owners. Design edits update capability references only: existing value/dependency paths, scoped human baselines, invariants and acceptance remain. D-23's whole-file/P-354 value path and unrelated generic/aggregate defaults, quantities, construction, regional representation, fractions, purity and permanent ABI retain their separate boundaries. No bootstrap/toolchain change.

## 2026-11-14 — Result contracts and focused design review

<a id="h-423"></a>
**H-423 — Reuse declared results and resolve routine consequences.** Human/source: current project user in this design-discussion chat, recorded 2026-11-14 America/Los_Angeles; exact timestamp, personal name and durable message URL unavailable. Responding to the separate-declaration example, the user says: “这个问题感觉没有疑义啊，必然是这样啊。 类似的无疑义的你直接决定就行，以前的无脑记录清理一波，然后回来跟我探讨真正有有疑义的设计决策。” This accepts the explicit declaration as the result contract, delegates analogous unambiguous consequences and requests cleanup before discussing material alternatives. It is not blanket approval of unresolved choices or whole-document value paths.

<a id="k-inference-cleanup-20261114"></a>
**K-INFERENCE-CLEANUP-20261114 — Consolidate contracts and evidence.** Agent assessment under H-423 and charter §§3.3/5.3/6.3/8/12: registry r654→655; D-24r5→6 reuses independently known declaration/expected callable contracts, rejects conflicting/circular constraints and preserves receiving-storage independence. Routine completion resolution infers void from normal no-value completion when no value return is required; missing required values retain H-361, and absent result/completion evidence requires an explicit contract. These consequences are agent resolutions under the user's delegation, not separately quoted human decisions. Automatic common-result synthesis remains open. D-23r7→8 compacts authority/review boilerplate without changing scalar semantics; D-26r11→12 aligns dependencies while preserving H-407 native coverage. D-07r154→155 removes duplicate declaration alternatives against D-10/H-258: signatures constrain applicable implementations and add no executable/negative override. Historical H-413–H-422 and their linked K payloads move to immutable source D below with all IDs retained. Prior text: `1c91962fe5bc525c833c92f589d654528f69d24b` at `CHANGELOG.md`, `design/CallableResultInference.md`, `design/ScalarCallResults.md`, `design/DomainLanguageDesign.md`, `design/GenericAndRecursiveClosure.md` and `design/DesignAuthorityRegistry.md`. Owner/access/cleanup, value-path qualification and D-16 fraction constraints remain; no source/build/test change.

## 2026-11-14 — Confirmed scalar and owner design integration

<a id="k-651"></a>
**K-651 — Design integration continuity.** Agent assessment under H-420 and charter §§3.3/4.3/5.3/6.3/8/12: registry r651 on both branches→652 and source reference272/268→273 reconcile H-413–H-420's confirmed scalar results, declaration operators and unique/null-owner rules with K-646–K-650's compiler capability record. D-14r87/D-10r147/D-18r91/D-23r7/D-24r3 and their recorded scoped qualification remain; the integration supplies no whole-file approval or implementation evidence for pending migration. Acceptance requires the receiving-storage matrices, return snapshots versus retained targets, statically unique owners and explicit null-owner forms while preserving cleanup/access constraints. All human evidence and concurrent compiler continuity remain. Unrelated aggregate, inference, abstract-evidence, regional-representation, fraction and permanent-ABI questions retain their status. Integrated design source: `b644351aa9521f0dbcabe6460e3e5ded71d827ae`; prior compiler capability reference: `df8bd38618eab3dd4cd446598fd10ea0019ca169:tests/cod/language/README.md`.

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
| D | `1c91962fe5bc525c833c92f589d654528f69d24b:CHANGELOG.md` | H-413–H-422 and the linked scalar/owner/inference discussion assessments |

Retrieve an event with `git show <commit>:<path>` and its source line range. Line numbers below refer to the immutable source, not this file.

### Scalar, owner and callable-inference discussion evidence

| Event | Source: lines |
| --- | --- |
| <a id="h-422"></a>H-422 | D:5–10 |
| <a id="k-callable-result-20261114"></a>K-CALLABLE-RESULT-20261114 | D:12–13 |
| <a id="h-421"></a>H-421 | D:17–22 |
| <a id="k-result-anchor-20261114"></a>K-RESULT-ANCHOR-20261114 | D:24–25 |
| <a id="h-420"></a>H-420 | D:34–35 |
| <a id="k-current-design-20261114"></a>K-CURRENT-DESIGN-20261114 | D:37–38 |
| <a id="h-418"></a>H-418 | D:42–47 |
| <a id="h-419"></a>H-419 | D:49–54 |
| <a id="k-null-owner-20261114"></a>K-NULL-OWNER-20261114 | D:56–57 |
| <a id="h-417"></a>H-417 | D:61–68 |
| <a id="k-unique-owner-20261114"></a>K-UNIQUE-OWNER-20261114 | D:70–71 |
| <a id="h-416"></a>H-416 | D:75–80 |
| <a id="k-scalar-init-20261114"></a>K-SCALAR-INIT-20261114 | D:82–83 |
| <a id="h-413"></a>H-413 | D:87–92 |
| <a id="h-414"></a>H-414 | D:94–99 |
| <a id="h-415"></a>H-415 | D:101–106 |
| <a id="k-scalar-val-20261114"></a>K-SCALAR-VAL-20261114 | D:108–109 |

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
