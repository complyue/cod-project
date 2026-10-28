# Steady recurrent SNN: decisions and open questions

Document-set revision: 32 · Recorded: 2026-10-27.

## Design authority

- **Registry / coverage:** [D-08](../../../design/DesignAuthorityRegistry.md#d-08), revision 32, this application decision history and status index; mechanisms remain `pending-human`.
- **Value lineage:** candidate P-25 → C-12 and P-22 → C-01 → V-01/V-02, with P-26 for C-13 adaptive capacity; explicit decisions preserve the meaning of the application while resource and execution choices evolve.
- **Invariants / non-goals:** distinguish human constraints from assistant proposals and calculations. Source quotes are evidence for their stated scope, not approval of every generated document or downstream mechanism. Multi-field data defaults to SoA; AoS requires an access-pattern-specific cache rationale and empirical evidence (H-29). Standard field widths are 1, 2, 4, 8, 16, 32, ... bits (`8 * 2^n`, integer n with a positive whole-number bit count); FP4 is standard. Other widths require separate packing/alignment discussion (H-30–H-32). Consider auxiliary structures only after clear business requirements lead to an implementation-algorithm discussion; no preset auxiliary storage (H-34). Pool sizing targets M slightly above N with simple static indexing; J and final quotas remain open (H-38/H-39).
- **Acceptance:** each selected mechanism needs its proposition, trade-off, scope, evidence and remaining dependencies recorded before implementation. The registry remains authoritative for qualification.
- **Human baseline / continuity:** H-08–H-23 below. K-03–K-13 record the document set and preserve the prior decision history; no qualified application implementation baseline exists. H-20 requests biological evidence. H-21 provisionally retains the alpha-near-3 template and asks to evaluate target-owned incoming sets; execution details remain proposals under K-11. H-22 requests anatomical and physiological evidence about incoming degree and excitability; K-12 records this assessment without selecting an integration or activity-control mechanism. H-23 accepts compensating incoming weight scale as a reasonable direction and requests separate incoming-degree parameters; K-13 records the scoped assessment without choosing a weight formula or replacing the provisional allocation. H-24 assigns weight-scale policy to training strategy; K-14 records that boundary and clarifies calculated versus measured population evidence. H-25 keeps capacity allocation provisional and requests 100+100 population analysis; H-26 asks for measured human column counts. K-16 records these scoped assessments. H-27 requests the Flash 60+60 calculation; K-17 records verified width and conditional pool sizing without selecting new neural dynamics. H-28 places the embedding table in the shared node budget; K-18 records element-aligned array sizing and the per-CC/node accounting boundary. H-29 establishes that SoA default and the evidence requirement for exceptions; K-19 records this scope. H-30 requires field-specific discussion for nonstandard widths. H-31/H-32 define standard widths as powers of two in bits, including FP4; K-22 supersedes the nonnegative-exponent restriction recorded in K-21. Packing/access details remain separate from width classification. H-33 requests the scaling document’s current memory calculation; K-23 records synchronized budgets and indexing arithmetic. H-34 establishes requirements before auxiliary structures; K-24 records that principle and the removal of speculative structures and budgets. H-35 limits current planning to Flash 60+60; K-25 clarifies A and aligns the active examples and budgets with that scope. H-36 asks about planning-time sizing; H-37 asks about 15/16 classes. H-38 sets M slightly above N as the objective; H-39 excludes preserving older connection capacity. K-27 records the worked candidates without selecting final J or quotas.
- **Current discussion:** H-50 / AD-044 clarifies that neurons already have their semantic-vector arrangement and no natural physical positions. [K-37](../../../design/DesignAuthorityRegistry.md#k-37) withdraws the premature soft-distance recommendation and the proposed choice between positions and a contact abstraction. H-51 / AD-045 requests corrected documents followed by at least 12 hours of deeper research into the computational benefit and applicability of biological locality. H-41–H-43 and H-46–H-48 remain; no new topology or learning mechanism is adopted.

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
| AD-033 | Use coactivity as the basic connection-learning direction, favoring nearer eligible partners outside a small very-near exclusion. Research 1D versus 2D; no dimension, metric, placement, kernel, exclusion radius or detailed weight update is selected. | [H-40](../../../design/DesignAuthorityRegistry.md#h-40); quote below; recorded 2026-10-27 |
| AD-034 | Keep semantic-dimension/minicolumn labels free of distance preference. Evaluate equal cross-minicolumn connection opportunity with a preference for corresponding internal cell positions; research semantic representations versus biological locations. Shared-role meaning, exact equality semantics, geometry, polarity and learning mechanisms remain open. | [H-41](../../../design/DesignAuthorityRegistry.md#h-41); quote below; recorded 2026-10-27 |
| AD-035 | Reject cross-minicolumn distance preference derived from internal minicolumn positions. The shared-position/aligned-index proposal is not an active design direction. | [H-42](../../../design/DesignAuthorityRegistry.md#h-42); first sentence below; recorded 2026-10-27 |
| AD-036 | Deepen research into semantic-vector mappings and 2D topology within a human CC, seeking design inspiration. This selects a research scope, not an alternative encoding, biological equivalence or implementation. | H-42; second sentence below |
| AD-037 | Treat CC as the basic semantic presentation unit, with approximate/non-exact replicas across CCs. Minicolumn/neuron semantic response roles are assigned statically; learning those roles is not required. Reassess topology research within that premise. Exact replication, response curves, learned dynamics and spatial layout are not selected. | [H-43](../../../design/DesignAuthorityRegistry.md#h-43); quote below; recorded 2026-10-27 |
| AD-038 | Prioritize reconciling neuron positions implied by the fixed semantic-vector assignment with biological anatomical topology. Continue deeper research; 1D versus 2D is secondary. This does not select interleaving, anatomical compactness, an opportunity model or a relaxation of dimension neutrality. | [H-44](../../../design/DesignAuthorityRegistry.md#h-44); quote below; recorded 2026-10-27 |
| AD-039 | Investigate whether actual ordinary-token embeddings use many dimensions, using `deepseek-ai/DeepSeek-V4.1-Flash-1017`; check local SSD capacity for its semantic table. The checkpoint is identified for this investigation, without selecting the final SNN encoder or deployment. | [H-45](../../../design/DesignAuthorityRegistry.md#h-45); quotes below; recorded 2026-10-27 |
| AD-040 | Biological features need not correspond one-to-one with minicolumns. Train for bounded recurrent activity with three semantic regimes: noise-supported DMN with a slight negative-semantic activation advantage and gradual negative drift; sequential token-associated flow; and immersion through autonomous multidimensional semantic loops with varying bounded lengths. Exact single-coordinate activation is not the goal. | [H-46](../../../design/DesignAuthorityRegistry.md#h-46); quote below; recorded 2026-10-27 |
| AD-041 | Apply the steady active-neuron fraction bound within each CC. | [H-47](../../../design/DesignAuthorityRegistry.md#h-47); scoped reply below; recorded 2026-10-27 |
| AD-042 | Keep the per-CC fraction allowance broad; different mental states may have different activity levels. Do not require equal fractions for every phase/regime. Numerical bounds and counting rules remain open. | [H-48](../../../design/DesignAuthorityRegistry.md#h-48); quote below; recorded 2026-10-27 |
| AD-043 | Continue deeper research to find scientific support for how the model should handle neuronal physical-distance preference. This requests an evidence-based assessment, without adopting a distance kernel, physical layout or learning mechanism. | [H-49](../../../design/DesignAuthorityRegistry.md#h-49); quote below; recorded 2026-10-27 |
| AD-044 | The neurons already have a semantic-vector arrangement and no natural physical positions. Correct the premise of the physical-distance assessment; this does not select another topology. | [H-50](../../../design/DesignAuthorityRegistry.md#h-50); quote below; recorded 2026-10-27 |
| AD-045 | Correct the documents, then continue substantive deeper research for at least 12 hours. Track evidence and actual research time; implementation remains deferred. | [H-51](../../../design/DesignAuthorityRegistry.md#h-51); quote below; recorded 2026-10-27 |

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


AD-033 / H-40:

> 接下来讨论神经元连接学习算法中的邻近优先原则，其拓扑采用一维还是二维有没有本质区别。 基本的学习还是 fire-together->wire-together，只是对方节点选择上，肯定要按（排除一小片极近的豁免区之后）越近则选择的概率越高的原则。 你从拓扑学相关研究的调研入手，看看简单的一维邻近拓扑就够，还是用更接近生物现实的二维拓扑有本质性优势。

AD-034 / H-41:

> 对，我们的模型首先会把神经元下标域安排给 minicolumn 布局使用，而我们刻意让 minicolumn 代表语义向量的一维，鉴于语义向量的维度之间不该有距离远近概念，我们不能让距离偏好影响各 维度/minicolumn 的正交性。 也许可以考虑 “距离” 概念映射到 minicolumn 内部神经元之间的拓扑关系？ 也就是一个神经元与其它 minicolumn 的连接概率相等，但是选择对方 minicolumn 内部个体神经元时，偏好等位距离更近的。 这个设计方向有多大程度上成立，值得看看有没有语义向量与人脑神经元物理位置对应关系的研究成果。

AD-035 / AD-036 / H-42:

> 那 minicolumn 内部位置去产生跨 minicolumn 的距离偏好确实没有说得通的理由，这个方向不该走。 接下来值得深挖语义向量与人脑的单个CC范围内二维拓扑的映射关系的相关研究成果，找找启发思路。

AD-037 / H-43:

> 我们的设计，有一个源自 HTM 的前提假定是 CC 是语义呈现的基本单元，CC与CC之间是非精确replica关系，minicolumn代表一定的 概念/语义，所以目前为止神经元的 “**response properties across semantic space**” 我们是静态分配的而不是要求网络去学出来。 基于这个澄清再看看

AD-038 / H-44:

> 是啊，现在 1D vs 2D 已经不是主要问题了，首先要面对的是神经元之间的相对位置，我们建模基于语义向量分配的物理位置，跟生物解剖现实中的的神经元物理位置拓扑，如何消除明显冲突。 继续深挖相关研究成果吧

AD-039 / H-45:

> LLM的语义向量，应该没有单维激活倾向吧？ 普通token应该是不同程度激活多个维度的吧，这个有统计数据可以查的吧，看看 DS  V4.1F 的真实情况

> 看 deepseek-ai/DeepSeek-V4.1-Flash-1017 ，本地ssd是否能存下语义表？

AD-040 / H-46:

> 所以人脑测量 “feature” 不跟 minicolumn 一一对应并不跟我们的建模冲突。 我们要训练的 SNN，并非精确语义单维激活，而是： 无任务 DMN 状态 —— 白噪音维持一定比例范围的稳态循环激活，语义上负面信息比正面信息后活跃稍强，所以稳态会慢慢滑向综合负面状态的极端；任务心流状态 —— token序列式轮番激活，整体仍维持一定比例范围的稳态激活；沉浸状态 —— 多维度语义向量循环式自我激活，循环链条长短不一，在一个特定范围内，整体神经元激活比例也维持一定的比例范围。

AD-041 / H-47, answer to whether the activity-fraction bound applies within each CC, across the whole network, or both:

> Within each CC

AD-042 / H-48:

> 每个CC内部激活比例是稳的，整体就不会失稳吧？ 另外这个比例不要求每个相态都一样，也不要太窄，应该允许不同心智状态在激活比例上也有所区别。

The whole-network weighted-average bound in [ActivationRegimes](ActivationRegimes.md#whole-network-activity-follows-from-the-per-cc-bounds) is an agent mathematical consequence under consistent counting/window definitions. H-48 asks about that consequence; it does not approve an unrestricted dynamical-stability theorem. Semantic valence, coordinate sign, neuronal phases and mental regimes remain separate meanings.

AD-043 / H-49:

> 继续深挖科研成果，找到我们的建模应该如何处理神经元物理距离偏好的支持依据

AD-044 / H-50:

> 我们已经按语义向量安排神经元位置了，没有天然物理位置，你自己晕了？

AD-045 / H-51:

> 修正文档以后继续深挖，花足够成本，至少12小时以上

## Proposal history and current status

| Proposal | Status / reason |
| --- | --- |
| Physical distance as an opportunity prior | Withdrawn agent recommendation after AD-044 / H-50, as recorded in K-37. The existing semantic arrangement supplies no physical metric. Biological findings remain references in [PhysicalDistancePreferenceResearch](PhysicalDistancePreferenceResearch.md); no choice between explicit positions and a contact abstraction is currently required. |
| Steady activity and three mental regimes | AD-040–AD-042 specify broad per-CC occupancy bounds, with different levels allowed across mental states and distinct semantic trajectories. No exact set point, equal phase populations, global controller, valence estimator or transition/learning formula is selected. |
| Biological feature / minicolumn correspondence | AD-040 permits non-bijective correspondence. Distributed semantic features are compatible with fixed coordinate roles; feature heterogeneity alone does not require interleaving or re-encoding. |
| Proximity-biased coactivity learning | AD-033 remains the direction, with AD-034's dimension-index neutrality. AD-035 rejects the internal-position mechanism; no geometry or distance kernel is selected. [Semantic cortical mapping research](SemanticCorticalMappingResearch.md) assesses AD-036's research scope. |
| Shared internal positions across minicolumns | Rejected by AD-035 / H-42. Equal cell index or an imposed shared internal coordinate supplies no accepted cross-minicolumn distance preference. |
| Spatial interleaving of fixed semantic roles / selective neurite reach | Alternatives examined under AD-038, not adopted. Interleaving relaxes anatomical compactness and alone provides only expected label symmetry, not exact equal proposals for every source; H-41 did not select that full probability contract. Selective reach requires a justified opportunity model; neither adds preset structures or changes capacity budgets. |
| Spatially organized neuronal response functions | AD-037 fixes semantic assignments and approximate-replica CCs; AD-038 prioritizes anatomical compatibility. Research distinguishes semantic group, compact anatomical population, neurite opportunity and storage order. Learning replacement roles remains outside the baseline. |
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
| [SNN-02/SNN-03](ActivationRegimes.md) | What counts as active within a CC, over what time window, with what broad envelope and transient allowance? | Makes the selected local bound measurable while allowing different mental states to occupy different activity levels. |
| [SNN-01/SNN-03/SNN-05](ActivationRegimes.md) | What defines negative semantic valence and its slight activation advantage, task-sequence success and an immersion loop's length? | Specifies the three semantic trajectories without substituting coordinate signs for meaning or choosing a learning rule prematurely. |
| [SNN-01/SNN-03/SNN-05/SNN-07](PhysicalDistancePreferenceResearch.md) | Which computational effect of biological locality is useful within the existing semantic arrangement, and what evidence establishes its transfer conditions? | Research precedes any proposed metric or mechanism. There is no natural physical distance to parameterize, and no required choice between positions and a contact abstraction. A result may support retaining the current organization without adding geometry. |
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
| [SNN-06](Model.md#snn-06) | Which state is durable and how much history is retained? | Determines DBMR growth and compaction headroom. |

Detailed choices and candidate value relationships remain open. Implementation is deferred under the user's language-design instruction H-07; this document set supports the ongoing discussion.
