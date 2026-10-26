# Steady recurrent SNN: decisions and open questions

Document-set revision: 24 · Recorded: 2026-10-26.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 24, this application decision history and status index; mechanisms remain `pending-human`.
- **Value lineage:** candidate P-25 → C-12 and P-22 → C-01 → V-01/V-02, with P-26 for C-13 adaptive capacity; explicit decisions preserve the meaning of the application while resource and execution choices evolve.
- **Invariants / non-goals:** distinguish human constraints from assistant proposals and calculations. Source quotes are evidence for their stated scope, not approval of every generated document or downstream mechanism. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** each selected mechanism needs its proposition, trade-off, scope, evidence and remaining dependencies recorded before implementation. The registry remains authoritative for qualification.
- **Human baseline / continuity:** H-08–H-23 below. K-03–K-13 record the document set and preserve the prior decision history; no qualified application implementation baseline exists. H-20 requests biological evidence. H-21 provisionally retains the alpha-near-3 template and asks to evaluate target-owned incoming sets; execution details remain proposals under K-11. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.

## Human decisions

The speaker is the user in the [CoD project conversation](codex://threads/01a14005-1edf-70d1-adcf-c091e8652c75); no independently verified name is available. Exact message timestamps are unavailable. Dates here are recording dates, not invented message times.

| Entry | Direction and scope | Evidence |
| --- | --- | --- |
| AD-001 | First application: steady recurrent SNN, one CC per region, semantic-vector exchange, hundreds to tens of thousands of CCs, intended DeepSeek V4 Pro input embeddings. The initial 100-cell/FP4/FP8 dense model starts the discussion; sign encoding and dynamics are open. | [H-08](../../../design/DesignAuthorityRegistry.md#h-08), original statement preserved there; recorded 2026-10-23 |
| AD-002 | Prioritize parameter and memory feasibility; each CC must fit a Raspberry Pi-sized budget. Reopen the 100-cell population size. No specific board, throughput or topology was chosen. | H-09; quotes below |
| AD-003 | Within-minicolumn connections are less necessary; consult HTM. This redirects connectivity analysis but does not select exact sparsity, segment limits or a complete Temporal Memory algorithm. | H-10; quote below |
| AD-004 | Maintain a separate document set for this first business application. CoD's language tracker retains the example link; the application model and capacity decisions live here. | H-11; quote below |
| AD-005 | Set the per-node RAM ceiling to 16 GB. Preserve the separate per-CC Raspberry Pi constraint; CCs per node and update rate remain open. | H-12; quote below |
| AD-006 | Consider unequal sparse connection capacities: highly connected neurons receive more weight slots, lightly connected neurons fewer, with training-driven migration between preallocated connection sets. A degree scaling law is a consideration, not a specified exponent or distribution. | H-13; quote below; SNN-07 |
| AD-007 | Statically provision the connection-weight slot sets according to the scale-law plan. Each neuron has an out-of-line pointer to its current set. Connection management uses no heap allocation. Exact class counts, pointer layout and reassignment protocol remain open. | H-14; quote below; SNN-07 |
| AD-008 | Evaluate a sufficiently large outgoing-degree allowance: around 10,000 connections for a very small minority among 716,800 neurons. The assistant’s 4,096-slot example is not a design ceiling. Exact maximum, class sizes, high-degree population and rate remain open. | H-15; quote below; SNN-04/SNN-07 |
| AD-009 | Count only a target ID and one-byte weight per slot; total set count equals neuron count. Add one array mapping each set back to its source neuron. Recalculate exact static memory. No per-edge reverse index or spare sets are implied. | H-16; quote below |
| AD-010 | Store targets and weights in parallel arrays of four-byte and one-byte elements for CPU alignment. Five bytes is the aggregate per-slot payload, not a packed struct requirement. | H-17; quote below |
| AD-011 | Permit more sets than neurons to assist migration. Prioritize class counts that make scale-based indexes easy to calculate; keep pool topology static rather than maintaining it through heap allocations. This supersedes only AD-009’s exact-N count requirement. | H-18; quote below |
| AD-012 | Set the current minimum set capacity to 100; omit smaller classes and permit ignoring insufficiently connected outgoing sources. Revisit the minimum if underfilled 100-slot sets prove both common and useful. Exact admission/re-entry rules, population limits and larger class counts remain open. | H-19; quote below |
| AD-013 | Investigate whether human-brain research supports a better scaling exponent than the doubling proposal. This requests research and comparison; it does not approve a particular exponent, class ratio or pool size. | H-20; quote below |
| AD-014 | Provisionally retain the preceding alpha-near-3 template for planning: 100/144 times powers of two, halving counts, 786,408 sets and 134,516,736 slots. This is a provisional choice, not measured suitability or a full implementation contract. | H-21; first sentence below; revision 8 template carried into revision 9 |
| AD-015 | Evaluate target-owned incoming sets for simulation convenience and compatibility with the scale law. Ownership and execution details remain under discussion; no automatic reinterpretation of the low-outgoing-degree omission rule. | H-21; second sentence below |
| AD-016 | Investigate whether neurons with many incoming connections activate too easily, using neuroscience and anatomical evidence. This is a request to assess the model, not adoption of a new degree cap, normalization rule, dendritic model or experiment. | H-22; quote below; recorded 2026-10-26 |
| AD-017 | Regard lower typical incoming weights at greater incoming degree as a reasonable compensation for excessive activation. This accepts a modeling direction; it does not establish a biological law or choose inverse-degree normalization, a fixed weight budget or a learning rule. | H-23; first sentence below |
| AD-018 | Assess incoming-degree allocation parameters separately from outgoing-degree parameters. The question does not select alpha=3.15 or replace AD-014’s provisional integer table. | H-23; second sentence below |
| AD-019 | Weight scale belongs to training strategy, as a possible hyperparameter or dynamically planned/adapted quantity; it is not a capacity-allocation design parameter. No particular training algorithm or scale rule is selected. | H-24; first sentence below |
| AD-020 | Retain the current capacity allocation provisionally and move discussion to semantic-dimension encoding. The two-class capacity doubling and per-class count halving remain the planning geometry; resizing for a different neuron population is a separate calculation. | H-25; first sentence below |
| AD-021 | Analyze 100 cells for positive values plus 100 additional cells for negative values per semantic dimension. This proposes 200 cells per dimension, not a 50+50 division of the old population; it does not yet select the decoder, training rule or resized pool. | H-25; second sentence below |
| AD-022 | Check measured human cortical-column neuron counts and compare the unit of measurement with the modeled CC. This requests anatomical evidence, without selecting biological equivalence or a smaller CC. | H-26; quote below |
| AD-023 | Calculate a smaller planning case using DeepSeek V4.1 Flash token vectors and 60 positive + 60 negative cells per semantic dimension. This makes that case the current sizing discussion; it does not select a final checkpoint, resized pool quota or neural decoder. | H-27; quote below |
| AD-024 | Account for a semantic-vector table once per compute node, shared by its CCs, and exclude it from per-CC storage. The user also questions the padding entry; K-18 removes the assistant-imposed eight-byte start requirement for byte arrays. No complete physical ABI is selected. | H-28; quote below |
| AD-025 | Default to SoA for multi-field data. An AoS exception needs a specific cache-friendly access-pattern justification and empirical evidence. This extends the explicit slot layout to the general layout policy without approving an implementation contract. | H-29; quote below |
| AD-026 | Discuss packing and alignment separately for each field with a nonstandard bit width. SoA does not automatically select tight packing, widening or padding; FP4 byte packing remains a sizing assumption pending that discussion. | H-30; quote below |
| AD-027 | Define standard field widths as exactly `8 * 2^n` bits for integer n >= 0. All other widths require AD-026’s separate packing/alignment discussion, including whole-byte widths such as 24 and 48 bits. | H-31; quote below |
| AD-028 | Include FP4 as standard: `4 = 8 * 2^-1`. Negative integer exponents are allowed where the resulting bit count is a positive integer, giving 1, 2, 4, 8, 16, ... standard widths. This supersedes AD-027’s n >= 0 restriction and excludes FP4 from AD-026’s nonstandard-width trigger. | H-32; quote below |
| AD-029 | Establish business requirements before discussing implementation algorithms; consider auxiliary data structures only within that discussion. Remove preset auxiliary structures and their hypothetical memory allowances from current planning. | H-34; quote below |
| AD-030 | Set aside the V4 Pro 7,168-dimensional case for now; keep current planning focused on Flash 5,120 dimensions and 60+60 cells. Clarify A as the set count in the 100-slot class; explaining the existing value does not approve a final quota. | H-35; quote below |
| AD-031 | Size the static pool so total slot sets M are slightly greater than total neurons N, while retaining convenient arithmetic indexing. No exact surplus percentage or final class count is selected. | [H-38](../../../design/DesignAuthorityRegistry.md#h-38); quote below |
| AD-032 | Do not preserve connection capacity from an older sizing scenario as a constraint. Derive reserved slots from the current population, class geometry and small-surplus objective. | [H-39](../../../design/DesignAuthorityRegistry.md#h-39); quote below |

AD-002 / H-09:

> 参数量和内存需求要先讨论，确定一个目前分布式CPU集群可承载的规划，每minicolumn 100 神经元也许太大了，目前不现实。

In response to a question about cluster capacity:

> 每个CC的参数量，控制在一个树莓派可以承载的范围

AD-003 / H-10:

> minicolumn内部反而不很需要连接，参考 HTM 的理论阐述

AD-004 / H-11:

> 第一个业务应用单独编写文档集，不要与cod文档混

AD-005 / H-12:

> 单节点ram上限放宽到16GB

AD-006 / H-13:

> 代替dense matrix的sparse方案，考虑scale law，为多链神经元分配更多连接权重槽位，少连神经元分配少槽位，随着神经元连接权重的训练，特定神经元可以迁移到更多或更少槽位的预分配连接集上去

AD-007 / H-14:

> 基于scale law的连接权重槽位集合采用静态分配，神经元通过一个外链指针指向它的当前槽位集，不做堆内存分配。

AD-008 / H-15:

> 允许的最大外连数不能太小，总共 716800 个神经元，允许极少数外连10000个的量级不过分吧？

AD-009 / H-16:

> 你仔细检查连接槽集分配，每槽只需目标神经元编号加一个权重，5字节够了吧？ 然后槽集总数应该等于总神经元数，另有一个槽集指回源神经元的数组。 总的静态内存需求算准确

AD-010 / H-17:

> 5字节元素这种应该分成两个数组，一个4字节元素的，另一个1字节元素，这样CPU内存对齐友好。

AD-011 / H-18:

> 槽集个数比总神经元个数多些问题不大，迁移时可能节省内存搬运复制吧。槽集个数按方便基于scale law直接算各种下标的方便优先，避免依靠堆内存分配维护所有槽的拓扑形状。

AD-012 / H-19:

> 外连少于100的槽集可以直接不分配，现实中对外影响太小的神经元可以忽略其作用。将来发现100槽用不满的情况普遍且合理的话再调整这个最小槽集容量。

AD-013 / H-20:

> 翻倍这个scale可能不是最好的，调研看有没有人脑相关研究可以现在就给出一个更好的指数值的

AD-014 / AD-015 / H-21:

> 这个就这样暂定。然后从模拟计算方便的角度，槽集承载一个目标神经元的连入权重比较好吧，看看跟scale law会不会产生冲突？

AD-016 / H-22:

> 输入太多的神经元，太容易激活吧，现实意义感觉不合理，这方面有神经学、生物解剖方面的研究数据吗？

AD-017 / AD-018 / H-23:

> 所以连入多的神经元，其入连权重普遍偏低，也就可以不表现为过于容易激活，这个合理。 然后根据入连数分配槽集，适用的 scale law 参数可能还是跟连出的建模参数有所区别吧

AD-019 / H-24:

> weight scale 不应该是容量分配的设计参数，应该是训练策略考虑采用的超参数，甚至是动态规划的。你说的 “Next-class population” 这个是来自实验测量结果还是根据我们当前设计反推的？

The provenance question requests clarification. The fitted exponents are study results; adjacent-bin population ratios are our calculations using those exponents and selected spacing. The provisional halving rule is chosen geometry, with alpha near 3 inferred from it. These distinctions are documented in [the provenance analysis](ConnectivityScalingResearch.md#provenance-of-adjacent-class-population-ratios); they are an agent clarification, not a human adoption of the calculated quotas.

AD-020 / AD-021 / H-25:

> 容量分配就先这样暂定。 接下来看minicolumn对语义维的建模，我想到说用 100 个神经元表示正值，另外再分配100 个神经元表示负值，你分析一下这个方向会产生哪些设计变化？

AD-022 / H-26:

> 人脑每CC的神经元个数，有研究实测的是多少？

AD-023 / H-27:

> 这样看，我们考虑用更少维度的 Deepseek V4.1 Flash 语义向量，然后每 MiniColumn 60 positive + 60 negative 神经元这样，你按这个计算一下

AD-024 / H-28:

> 怎么还出 “Alignment padding” 了？ 一个计算节点负责多个CC时，语义向量表可以共享，不应该往单CC的内存需求上加。

AD-025 / H-29:

> 对于多字段，除非有特殊理由（访问模式决定的缓存友好性）且实证，否则默认做 SoA 而不是 AoS。

AD-026 / H-30:

> 一个字段如果bit数不整齐，要单独讨论对齐问题。

AD-027 / H-31:

> nonstandard bit width 应该只要不是 8 乘以 1,2,4,8… 就算

AD-028 / H-32:

> FP4 是 8*2^-1 ，也算 standard bit width

AD-029 / H-34:

> 这些辅助数据结构，先不引入预设，只有明确了业务需求，讨论实现算法时才考虑。 更新文档去掉这些内容，并增加原则说明段落。

AD-030 / H-35:

> 这个 Pool A是啥？ 可以先不考虑 V4Pro 的7168维语义向量了。

Class-count question / H-37:

> 目前总共15个class是硬性的吗？ 其它个数，比如16个，行不行？

This requests assessment; it does not select 16 classes.

AD-031 / H-38:

> 目标是总槽集数略微大于总神经元数吧，这个作为明确目标没有？

AD-032 / H-39:

> 没必要 preserve “connection capacity from the older sizing case,” 吧？


## Proposal history and current status

| Proposal | Status / reason |
| --- | --- |
| Dense local minicolumn blocks plus sparse or low-rank global coupling | Earlier assistant capacity sketch, not approved. AD-003 redirects the active proposal to contextual segments without assumed dense local blocks. Low-rank coupling is not equivalent to arbitrary sparse context matching. |
| Pi 5 with 4 GB as a provisional reference | Earlier assistant assumption, replaced for current planning by the explicit AD-005 16 GB node ceiling. Pi 5 remains a reference CPU, not a confirmed fleet inventory. |
| Flash pool size | AD-031 makes M slightly above N the objective; AD-032 excludes preserving earlier connection capacity. The [comparison](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options) uses the smallest exact-halving pools above N: 15 classes give M=622573, 1.330241% surplus and 515.15 MiB; 16 give M=655350, 6.665039% surplus and 542.88 MiB. Final J and quotas remain open; execution uses a static pool. |
| Equal positive/negative cell populations | AD-023 makes Flash 5,120 × (60+60) the current sizing case: 614,400 neurons. The worked 15-class A=311296 candidate gives about 515.15 MiB of per-CC named arrays. Current planning uses the Flash 60+60 case under AD-030. Signed readout, magnitude/context separation, coactivation, prediction and training remain open. |
| Field width classification | AD-028 supersedes AD-027’s exponent restriction: standard widths are 1, 2, 4, 8, 16, 32, ... bits. FP4 is standard; 12/24/48-bit fields remain nonstandard and require AD-026’s separate discussion. Current FP4 storage figures retain two values per byte; classification adds no padding and does not select an access/update protocol. |
| Multi-field data layout | AD-025 establishes SoA as the default. AoS requires an explicit access-pattern/cache rationale supported by measurements; no exception is currently established. |
| Shared embedding storage | AD-024 excludes the embedding table from per-CC storage. The current two-byte table uses 1,323,827,200 bytes once per node; node totals add k CC payloads, the shared table and separately budgeted runtime/state. |
| Biological column size | AD-022 requests measured evidence. Human V1 tissue measurements support a study-specific ocular-dominance module estimate of about 159,000 neurons. The current 614,400-neuron case is about 3.86 times that population. This does not establish a universal human CC count or functional equivalence. |
| Sparse contextual segments, `m=8/16`, `s=4`, `k=32` | Uniform arithmetic reference. AD-006 makes variable capacity the active direction; four segments and 32 synapses do not constrain every neuron. |
| 512 MiB private planning allowance for the 16-cell candidate | Proposal with explicit storage arithmetic; no measured peak or throughput guarantee. Thirty-two and 100 cells remain comparison points. |
| Retaining every old live set until DBMR compaction | Earlier assistant interpretation applied history retention too broadly. AD-007 fixes a static live pool; assignment changes need not create history. Reuse of unchanged typed set objects is the current proposal, with its release boundary still open. |
| Extra sets and per-edge reverse indexes | AD-009 excluded both from its exact-N calculation. AD-011 subsequently permits extra sets for indexing/migration; per-edge reverse indexes remain absent from the base budget. AD-031 explicitly targets a small surplus; the worked 15-class candidate provides 8,173 sets (1.330241%) above N. |
| Auxiliary data structures | AD-029 defers consideration until business requirements are clear and implementation algorithms are discussed. Current planning prescribes no auxiliary representation or speculative memory allowance. |
| Geometric classes and occupancy thresholds | Assistant proposals. AD-009/AD-010 fix outgoing slot contents and layout; AD-014 provisionally retains the alpha-near-3 class counts; migration publication remains open; auxiliary representations are deferred under AD-029 until requirements and algorithm discussion justify them. |
| Maximum connection-set capacity | The 15-class candidate reaches 12,800 slots with 19 largest sets; the 16-class candidate reaches 18,432 with 10. Class count is open. H-15’s approximately 10,000 outgoing-degree case remains distinct from the incoming-capacity limit. |
| Equal-slot geometric classes | Earlier comparison, outside the current Flash-only planning scope. The worked candidate uses 15 alpha-near-3 classes; 16 is also feasible. |
| Exponent near 3 with finer capacity classes | Assistant evaluation candidate under AD-013. Adult-fly evidence reports outgoing exponent 2.96; human evidence does not establish a universal value. Capacities alternate 100/144 times powers of two, while counts halve. The current Flash case has A=311296 sets in the 100-slot class, 622,573 total sets and 515.150806427001953125 MiB. AD-014 provisionally retains the class geometry; lognormal remains a comparison, not the chosen planning layout. |
| Target-owned incoming sets | AD-015 requests assessment. Recommended for synchronous gather: source IDs per slot, target ID per set, unchanged base storage. Incoming degree now determines required capacity; outgoing distribution, minimum-input admission and sparse-event routing are separate questions. |
| Input count versus excitability | AD-016 requests biological evidence. A flat positive sum with unchanged weight scale/threshold has degree bias; anatomy, human/animal recordings and constrained models motivate explicit timing, dendritic integration, inhibition and adaptation. Point-neuron control and branch-based integration remain alternatives; no numerical firing threshold or biological degree law is adopted. |
| Conditional incoming weight scale | AD-017 accepts the compensating direction as reasonable. AD-019 assigns weight-scale policy to training strategy, including possible hyperparameters or dynamic planning/adaptation. A fixed total or specific normalization rule remains unselected. |
| Separate incoming population exponent | AD-018 requests assessment. Adult-fly fitted point estimates are alpha_in=3.15 and alpha_out=2.96; adjacent-class ratios are calculated projections using our geometry. At the current Flash N=614400 and M=622573, the same-capacity 3.15 sensitivity calculation gives 10 largest sets and about 484.07 MiB, not an experimentally measured requirement. The existing alpha=3 allocation remains provisional. |
| Omitted outgoing sources | AD-012 permits excluding low-connectivity contributions. A null current-set reference with retained neuron identity/state is a source-owned representation proposal. Under incoming ownership, a null input set does not suppress that source’s outputs in other sets. Actual admission, suppression, discovery and re-entry rules remain open; automatic permanent removal below 100 live edges is not selected. |
| Four-bit phase/intensity, eight-bit synapse storage | User starting direction, encoding unresolved. Standard floats, custom codes and quantized permanence have different semantics. |

## Next decisions

| Topic | Exact open decision | Consequence |
| --- | --- | --- |
| [SNN-02/SNN-03/SNN-07](SynapticIntegrationResearch.md) | Which integration, input-timing and activity-control contract prevents unwanted degree bias while retaining useful selectivity? | Determines the meaning of activation, any branch or adaptation state, and acceptance beyond storage arithmetic. |
| [SNN-01/SNN-04](ConnectivityScalingResearch.md#sizing-objective-and-class-count-options) | Which class count and quotas best satisfy the small-surplus objective for Flash 60+60? | With N=614400 and exact halving, 15 classes give 1.330241% surplus and a 12,800-slot maximum; 16 give 6.665039% and 18,432. No old-capacity preservation requirement applies. |
| [SNN-05](SynapticIntegrationResearch.md#training-strategy-incoming-weight-scale) | Which training strategy, hyperparameters or dynamic planning/adaptation should govern weight scale? | Determines learned effective weights and any adaptation state; pure allocation migration preserves those weights and their effect. |
| [SNN-07](IncomingConnectivity.md) | Adopt incoming ownership and define gather/event-driven timing, incoming admission and the relationship to outgoing influence. | Determines update work, any routing index and the meaning of the 100-slot minimum. The alpha-near-3 layout is provisional. |
| [SNN-07](AdaptiveConnectivity.md) | What determines useful growth/pruning, incoming class shortages, migration/availability and source participation/re-entry? | Determines learned topology, fixed pool utilization and migration work; useful underfilling and output influence need separate evidence. History retention is separate. |
| [SNN-04](CapacityPlanning.md) | What context capacity, population and segment/synapse caps should the first model explore? What update/emission rate matters? | Determines memory, CPU demand and useful placement. |
| [SNN-03](Model.md#snn-03) | How do dense signed coordinates generate sparse column/cell activity while retaining required magnitude and context? | Determines both representational fidelity and activity-driven CPU cost. |
| [SNN-01](SignedPopulationEncoding.md#magnitude-and-contextual-cell-selection) | In the proposed positive/negative populations, do cells encode magnitude samples or select context with graded magnitude, and how is each population decoded? | Prevents contextual bursting from silently changing the represented coordinate. |
| [SNN-01/SNN-02](SignedPopulationEncoding.md#coactivation-prediction-and-transitions-through-zero) | Can both polarities be active or predictive together, and what does cancellation mean? | Determines zero/sign transitions, hidden activity and information preserved in signed-vector messages. |
| [SNN-02](Model.md#snn-02) | How do current activation and future prediction coexist, and what does the four-bit code mean? | Determines state transitions, extra fields and arithmetic. |
| [SNN-03](Model.md#snn-03), [SNN-05](Model.md#snn-05) | Which recurrent invariant and learning task define success? | Supplies observable acceptance beyond a memory estimate. |
| [SNN-06](Model.md#snn-06) | Which state is durable and how much history is retained? | Determines DBMR growth and compaction headroom. |

Detailed choices and candidate value relationships remain open. Implementation is deferred under the user's language-design instruction H-07; this document set supports the ongoing discussion.
