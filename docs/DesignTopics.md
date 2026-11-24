# CoD 远期后备讨论主题

这里记录留待具体用例检验的远期设计问题。各项列出已有契约、待决定的行为，以及可以区分候选方案的例子。

## 讨论依据

- 以当前设计为准。[设计权威登记表](../design/DesignAuthorityRegistry.md)记录了各项决定的确认范围；本索引中的候选写法和验收问题不构成实现授权。
- 开题时给出具体程序、前提和可观察结果。形成新决定后同步更新对应设计、登记表和简短 changelog，移除已解决的后备问题。
- Idiomatic CoD 沿用[写作与验收规范](IdiomaticsGuideline.md)。示例遵循已确认的语言与源码风格。

## 1. 通用 collections API

状态：远期后备

### 已敲定的范围

集合层面需要区分观察、取走成员与消费遍历。消费遍历每成功交接一次才移除一个成员；提前退出时，未交接的成员按原顺序留在集合中。循环头持有的旧对象在下次交接覆盖赋值时释放，循环整体退出时释放仍持有的最后一个对象。

`UnhandledDeferError.exceptions` 这一具体实例已经有独立成员所有权、
只读枚举及单成员转移契约。该实例的确认不自动批准完整的通用集合接口。

### 待决策问题

1. 首批通用协议是否限定为有序、可索引集合？
   类型如何声明或满足该协议？map/set 是否明确留在其他范围？
2. 观察、取出单个成员、消费遍历的最终名称与完整签名是什么？
   `visit`、`take_at`、`drain` 当前是提议名称，需要明确确认或替换。
3. 各操作的 element mode、访问权限、实际 owner、返回值和 owning 接收形式如何表达？
4. 哪些遍历操作允许通过其他引用修改集合结构？移除成员后，各种集合如何定义剩余成员的索引？

### 完成标准

- 确定一张操作签名、访问权限与所有权交接表。
- 给出空集合、普通 bits 集合和成员分别属于不同 owner 的集合示例。
- 明确说明取走成员 B 后，A/C 的访问、索引变化及 owner 身份。
- 明确剩余集合与取走的 B 各自承担的释放责任。
- 将最终接口与仍未纳入的扩展范围分别记录。

参考：

- [设计依据与已确认范围](../design/CollectionsAndBuiltinContainers.md#design-authority)
- [候选集合操作](../design/CollectionsAndBuiltinContainers.md#proposed-common-operations)

<a id="named-signatures-and-pipelines"></a>

## 2. 命名的类型签名与 producer、consumer、transformer

先在 CoD 语言层面支持**可带类型参数的命名签名**。设想库作者可以定义一次 `Producer<A>`，使用者在另一处写 `Producer<Invoice>` 来引用提供 `Invoice` 的签名。同类例子有 `Consumer<B>` 和 `Transformer<A, B>`。这里的尖括号只作示意，声明写法和类型规则留待设计。

已有的 `proc emit(x:T) yields T -> bool where {T}` 给操作 `emit` 命名。这里需要的是给签名本身命名，使多种实现和调用者可以引用它。等语言支持这种声明和引用后，再讨论 producer、consumer、transformer 的通用接口、绑定与组合。[现有 generator 签名](../design/RangedForAndGenerators.md#independent-protocol-establishment)

### 已敲定的范围

当前 composite 契约已经覆盖操作数角色、三种初始化后的 lexical partial、
有效生命周期内的重复调用，以及库重载返回普通 data-only regional functor。

这些既定形式可以直接作为惯用法依据，不应整体重新置为待定。

### 命名的类型签名就绪后再讨论

1. 用命名签名分别表达 producer 提供的数据、consumer 的反馈，以及 transformer 的上下游要求。用带借用的输入检验签名如何表达实际 owner 和访问权限。
2. 比较 `invoice_ids(select_day()) !> cap` 与候选写法 `cap(invoice_ids(select_day()))`。当处理器 `cap` 配置为取零项时，显式管道只求值一次 `select_day()`，不会打开游标。普通调用能否根据参数签名获得同样的行为，届时再定。
3. 检查命名实参、默认参数和 overload 对绑定的影响，以及何时需要调用者显式消歧义。

### 相关的独立问题：可变 here 绑定

1. 是否允许替换已有 `here` 绑定？若允许，源代码形式是什么？
2. 分支产生不同候选绑定时，如何保留各自的 owner 与生命周期要求？
3. 替换对已经准备的参数、活动调用和已登记的延期调用有什么影响？

### 完成标准

- 给出“普通调用／producer 绑定／应拒绝歧义”的明确对照。
- 对可变 here 绑定形成明确的准入或排除决定。
- 若允许替换，给出分支替换、旧依赖失效、重复调用及延期调用的行为示例。
- 保留参数准备一次、书写顺序、反馈和受保护控制转移等既定契约。

参考：

- [设计依据与已确认范围](../design/ProducersAndTransformers.md#design-authority)
- [未闭合管道的词法绑定](../design/ProducersAndTransformers.md#lexical-variables-for-open-pipelines)
- [待决项与验收](../design/ProducersAndTransformers.md#open-decisions-and-acceptance)

## 3. pure 的调用准入、输出边界与 knot-tying

状态：远期后备；先明确调用与输出边界，再讨论递归构造

### 已敲定的范围

`pure` 声明拼写、外层调用期间外部状态只读、允许私有可变构造，
以及即时返回的观察边界已有决定。

支持 Haskell-like knot-tying 是已经确认的研究目标；
它尚未确定具体的递归绑定与求值语言。

### 待决策问题 A：helper/callback 准入

1. 普通 helper、here callback、选中的 overload 和 native service，
   如何被准入 pure 调用树？
2. 采用显式 pure callable 类型、调用上下文约束，还是两者组合？
3. 对调用期间外部输入稳定性的要求，如何成为可检查的契约？

### 待决策问题 B：私有构造的输出

1. 私有构造结果允许通过哪些输出与 owner 路径离开调用？
2. 在 caller-owned region 中分配是否允许？若允许，需要什么明确权限？
3. 如何防止把对调用者分配状态的修改直接当作“私有写”？

### 待决策问题 C：knot-tying

1. 首批范围覆盖有限循环图、依赖自身结果的递归计算，还是也包括延迟结果？
2. 分别采用什么递归绑定、求值、共享、完成与 publication 规则？
3. 若引入延迟求值，后续计算的 owner、失败与观察边界如何定义？

### 完成标准

- 形成 pure 调用准入表与明确的输出边界。
- 给出外部只读、私有可写、非法外部修改和非法悬垂输出的对照示例。
- 分别用有限双向环和 tree-minimum 重建说明所选 knot-tying 能力。
- 不将有限引用环等同于递归计算，也不从 pure 自动推出一般 lazy semantics。
- 明确本次决定与后续扩展的边界。

参考：

- [pure 的调用边界](../design/ConstFunctionsAndTailCalls.md#boundary-of-a-const-function)
- [私有构造与结果输出](../design/ConstFunctionsAndTailCalls.md#private-construction-and-escape)
- [knot-tying 目标](../design/ConstFunctionsAndTailCalls.md#knot-tying-as-a-pure-design-goal)

## 4. 用户定义中缀操作符的优先级与结合性

规划目标是让库声明新的中缀操作符，并在声明处指定优先级、左结合、右结合或不可连续结合。
此功能由未来的 `e-wrks/cod` 编译器实现。seed 和 booter 负责按已确认的字符集合读取完整 token，
并报告当前不支持的拼写。新操作符的声明写法、可供用户定义的拼写范围和跨模块规则留待讨论。
[CoD 的操作符 token](../design/SymbolicDynamicScope.md#symbolic-operator-tokens)

例如，假定某个库定义 `a <+> b` 的结果为 `10 * a + b`。对于整数表达式
`1 <+> 2 <+> 3`，左结合得 123，右结合得 33，不可结合则要求作者明确分组。
再用 `1 <+> 2 * 3` 检验该声明与内置乘法的相对优先级。
比较操作符经重导出、同名 family 合并或声明不同 overload 后如何确定同一个表达式的分组，
以及两个同优先级、不同结合性的操作符相邻时如何报错。

Edh 的声明解析器会把操作符的优先级和结合性记入表中，并检查重复声明是否一致；
表达式解析器在读取中缀操作符时查表，用这些信息决定分组。可以用同一组例子比较 Edh 和
Haskell，再按 CoD 的模块解释规则决定声明的可见性、优先级区间与缺省行为。
[Edh 声明处理](https://github.com/e-wrks/edh/blob/a2e9b95ef1f84cc5677499512c69d4b03a12cbdc/elr/src/Language/Edh/Parser.hs#L1046-L1138) ·
[Edh 中缀分组](https://github.com/e-wrks/edh/blob/a2e9b95ef1f84cc5677499512c69d4b03a12cbdc/elr/src/Language/Edh/Parser.hs#L2216-L2276) ·
[Haskell 2010 §4.4.2](https://www.haskell.org/onlinereport/haskell2010/haskellch4.html) · [CoD 的模块解释](../design/ModuleWorlds.md)

<a id="body-mixin-templates"></a>

## 5. `mixin` 代码模板

`mixin` 按 C++ template 的心智模型理解：把选中的代码体当作在接收模块现场重新写了一遍，按该处的绑定检查。将来有具体用例时，再细化并实现这个特性。[接收环境中的定义](../design/SymbolicDynamicScope.md#import-time-alias-translation)

一个待讨论的例子是，模板原本声明两个独立的 symbol，先向 `@output` 写入 9，再读取 `@input`。若接收方显式写 `with { @cell as @output; @cell as @input; }`，是否应接受并读到 9，还是要求这两个角色在接收方也保持独立？这个选择留待用例检验。

## 按实际用例重启的小议题

| 议题 | 何时值得讨论、需要确定什么 |
| --- | --- |
| overload 中的 `default` | 有更具体的方法需要调用本族更通用的实现时，比较显式调用与委托的选取规则，涵盖多个候选及递归。模块的 `default { ... }` 与默认导出已经有独立决定。[模块选择](../design/ModuleWorlds.md#default-and-complete-public-selections) |
| 初始化与分支结果 | 用真实调用检验非 bits 或产生所有权的分支如何合流。另有无 owner 的纯 bits 构造表达式候选；目前新建实例写 `make`。[构造与分配](../design/ConstructionAndAcquisition.md) · [owner 和初始化](../design/RegionalOwnership.md) |
| 结构化结果与 generator 的扩展 | 当现有 receiving spec、`yields`、反馈和同步 `here` 无法表达某个调用时，以该调用界定需增加的行为。[generator 与循环](../design/RangedForAndGenerators.md) |
| 尾调用、分数和单位 | 有实际递归深度要求时讨论尾调用的可保证范围及 ABI 条件。固定宽度分数依已有草案保留兼容性；更广的单位与索引运算需要各自的用例及取舍。[尾调用](../design/ConstFunctionsAndTailCalls.md#tco-and-the-callable-abi) · [分数](../design/FixedWidthFractions.md) · [单位与索引](../design/ScalarUnitsAndIndexTypes.md) |

## 开放式异步管道

用两路传感器共同报告一台泵的状态比较不同写法。让两路以不同速率到达，并加入迟到事件：谁负责缓冲，何时关闭窗口，消费者跟不上时生产者观察到什么？以同一输入的可观察结果对照已有复杂事件处理系统。[异步管道研究](../papers/OpenEventPipelines.md)

## Actor 与 chan

状态：已启动预研；基础执行与消息语义已确定，具体接口和生命周期继续讨论。

语言概念统一为 Actor 和静态强类型的 `chan`。每个 Actor 对应一条专属 OS 线程，
拥有独立执行栈和隔离的 TLS；语言不提供另一套线程并发模式。
消息采用 UDP 式语义，不保证接收、不提供回执，应用自行建立流及其他可靠协议。
当系统有依据判断后续发送大概率失败时，应向发送者报系统故障；
其余情况提供“发后不管”的发送语义。具体物理传输不因此被限定为 UDP。
[已确认边界](../design/SymbolicDynamicScope.md#actor-concurrency-direction)

业务预研参考 [nedh 的网络通信](https://github.com/complyue/nedh) 和
[sedh 的集群调度](https://github.com/complyue/sedh)，重点检验请求携带回信端点、
多个 worker 汇流、资源征召、任务结算与取消。
[预研与源码依据](../design/ActorChannels.md)

接下来分别确定 chan 身份与端点权限、接收方数量、消息图的转移点、
系统故障时的 owner 去向、队列资源与关闭、Actor 启动/退出及模块生命周期，
以及网络类型/版本验证。不能把单条消息未收、业务拒绝或任务失败作为基础 send 的回执。
本轮不重新讨论已经确定的术语、1:1 映射、静态类型和基础发送契约。
