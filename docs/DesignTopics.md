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

`chan` 两端是这套类型语言的具体用例。发送端和接收端已经确定可以独立建模。等 CoD 支持 higher-kinded type synonym 后，再正式表达发送侧的 consumer、接收侧的 producer，以及各自可组合的 transformer。[高阶类型别名议题](#higher-kinded-type-synonyms) · [端点设计](../design/ActorChannels.md#independent-chan-endpoints)

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

等 `e-wrks/cod` 仓库建立后，再讨论库作者的声明语法与跨模块细节。

规划目标是让库声明新的中缀操作符，并在声明处指定优先级、左结合、右结合或不可连续结合。
此功能由未来的 `e-wrks/cod` 编译器实现。seed 和 booter 负责按已确认的字符集合读取完整 token，
并报告当前不支持的拼写。新操作符的声明写法、可供用户定义的拼写范围和跨模块规则留待讨论。
[CoD 的操作符 token](../design/SymbolicDynamicScope.md#symbolic-operator-tokens) · [生产解析器的技术路线](#production-parser-combinators)

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

`mixin` 按 C++ template 的心智模型理解：把选中的代码体当作在接收模块现场重新写了一遍，按该处的绑定检查。booter 源码不使用 `mixin` 或类型级编程，seed 无须支持两者。后续语法和求值设计面向 `e-wrks/cod`。[接收环境中的定义](../design/SymbolicDynamicScope.md#import-time-alias-translation)

类型级函数以及更完整的类型级编程优先从 `mixin` 扩展。模块声明静态参数，使用者在 mixin 处提供具体的类型级或 kind 级实参。例如给模板传入 `deque` 和 `UInt64`，可以得到已有的成员类型 `deque[UInt64]`。若模板声明新的名义类型，两个接收模块各自混入，各有自己的类型身份。需要互传同一类型的值时，在共享 vocabulary 模块定义一次并导出，双方选入即可。静态参数语法、计算结果的发布形式及求值规则留待设计。[静态模块参数](../design/ModuleWorlds.md#static-module-parameters-for-mixin)

类型级不使用 OO 写法。声明 `'type Container [elem: Type]'` 和 `'deque[T] : Container { T -> elem }'` 后，前缀投影 `'elem deque[UInt64]'` 求值得到类型 `UInt64`。投影的导出与选择、谁能提供映射，以及冲突时如何诊断留待具体用例决定。[字段映射与前缀投影](../design/ModuleWorlds.md#product-kinds-and-prefix-projections)

候选的 `''' … '''` 类型级块可以分别写函数签名和分支方程，例如按输入类型计算更宽的类型。将来若加入兜底方程，再决定重叠如何处理。[函数方程例子](../design/ModuleWorlds.md#type-level-functions-with-equations)

另一个待讨论的例子是，模板原本声明两个独立的 symbol，先向 `@output` 写入 9，再读取 `@input`。若接收方显式写 `with { @cell as @output; @cell as @input; }`，是否应接受并读到 9，还是要求这两个角色在接收方也保持独立？这个选择留待用例检验。

<a id="higher-kinded-type-synonyms"></a>

## 6. Higher-kinded type synonym

已确定的用途是用形式化的 producer、transformer、consumer 类型语言分别描述 `chan` 的发送端和接收端。这个议题讨论类型构造器如何进入别名的参数和结果，以及编译器怎样检查每次应用。通用管道的绑定与组合另有讨论入口。[命名签名与管道](#named-signatures-and-pipelines) · [chan 端点契约](../design/ActorChannels.md#independent-chan-endpoints)

### 用一个例子区分两种能力

在 `Producer<Invoice>` 中，库作者写定了 `Producer`，传入的是完整的消息类型 `Invoice`。现在反过来固定消息类型，让使用者选择签名族本身。下面只用数学记法描述候选行为，`≔` 表示示例定义，`≡` 表示预期展开结果。

```text
ForInvoice<F>        ≔ F<Invoice>
ForInvoice<Producer> ≡ Producer<Invoice>
ForInvoice<Consumer> ≡ Consumer<Invoice>
```

要使这三行成立，`F` 必须能接受一个消息类型，并产生可用于该位置的签名。若传入完整类型 `Invoice`，展开会要求 `Invoice<Invoice>`，应在类型应用处指出 kind 不匹配。两个合法实参仍会产生方向不同的签名。持有发送端的 worker 可以向其提交发票，持有接收端的 collector 可以从中取得发票。两端分别做静态检查。

### 用论文中的程序检验候选规则

| 研究用例 | 展开别名后应当能够观察到什么 |
| --- | --- |
| 闭合遍历。票价依次为 120、80、80，筛选器跳过 120，调用方取首个可接受票价。 | 拒绝 120 时，筛选器仍须向要求布尔反馈的上游回答是否继续。在第一个 80 向外层返回时，已进入的遍历依次清理。比较命名签名与显式签名时，应对照反馈归属、退出目标和清理顺序。[枚举选择论文](../papers/ClosedEventPipelines.md#what-an-equivalence-must-preserve) |
| 开放管道。传感器提交读数后退出，worker 在稍后的调用中处理已入队事件。 | 签名要保留读取或转移载荷所需的 owner。把借用了 ingress 栈帧的 `here` 或局部引用排队应被拒绝。worker 的反馈只属于其本地调用。以回放 sink 和队列 sink 检验同一签名可采用的不同实现。[开放事件论文](../papers/EventEffectPropagation.md#a-dynamically-scoped-event-sink) |
| 函数响应式编程（FRP）。提枪与改价同帧发生，挂枪与最后一升计量也在同一帧。 | 尝试固定一条本地时间线，再把事件、可采样值和带更新的状态用作不同的类型构造器。旧价启动、新量结账应得到 510 分。若换一个采样规则会收到 540 或 340 分，别名展开和库操作各自需要说明什么？[加油销售与 Reflex 风格的网络](../papers/EventEffectPropagation.md#a-reflex-style-network-for-a-fuel-sale) |

仓储分配策略编辑器提供了一个管道以外的对照。论文以 `Program<Orders → Decisions>` 描述可编辑的程序图。给这个完整类型起简称时，普通别名就够用。若要让同一个编辑签名在多种程序表示之间复用，再给出必须把 `Program` 所在位置参数化的实际用例，并检查编辑程序图和执行它各自需要的权限。[HLVM 论文中的图编辑案例](../papers/AlgebraicEffectsIsAllYouNeed.md#1-a-program-that-takes-a-program-as-its-input)

### 待决定的问题与验收

1. **允许传入什么。** 首批是否接受未填入参数的具名别名？例如用 `FromReading<B> ≔ Transformer<Reading, B>` 固定输入，`ForInvoice<FromReading>` 能否展开为 `Transformer<Reading, Invoice>`？再决定是否还需要直接传入部分应用的二元构造器。
2. **怎样表达和检查 kind。** 哪些参数位置接受完整类型、哪些接受构造器，结果位置能否接收签名？哪些信息可由使用推断，哪些须在声明处写出？要求错误能定位到传错构造器或实参的那一处。
3. **展开后保留什么。** 确定别名与展开式在类型匹配和模块重导出时的相等规则。用一对带 owner 的输入检查访问权限、目标身份及生命周期，用发送端和接收端互换检查错误方向。[owner 与目标身份](../papers/CoarseGrainedOwnership.md#owner-identity-target-identity-and-representation)

形成可评审提案时，给出示例的完整声明、两种合法实例化和 kind 不匹配诊断，再用上述同步与异步用例列出应接受和应拒绝的调用。声明拼写、kind 推断、部分应用及展开规则都在本议题中待定。

<a id="production-parser-combinators"></a>

## 7. 生产编译器的 CoD 解析组合子

**已确认的分工。** 规划中的 `e-wrks/cod` 负责设计和开发类 Megaparsec 的 CoD 解析库，在生产编译器的 lexer 和 parser 中使用，并参照 Edh 组织产生式。booter 继续用递归下降处理 bootstrap 子集和固定优先级。生产实现完成后，另行决定是否将其中一部分回移到 booter。[项目用户的决定](../CHANGELOG.md#h-production-parser-direction-20261124) · [编译器分工](../design/DesignAuthorityRegistry.md#d-20-repositories) · [Edh 解析器](https://github.com/e-wrks/edh/blob/a2e9b95ef1f84cc5677499512c69d4b03a12cbdc/elr/src/Language/Edh/Parser.hs)

**发布内容。** `pcl.cod` 是以解析组合子库形式提供的 HLVM，**生产 CoD parser 是这台 HLVM 的一个应用**。`e-wrks/cod` 工具链随附 `pcl.cod` 和这个应用的 DBMR 文法程序 `cod.grammar`。解析 CoD 源文件时，应用在 `pcl.cod` 上运行这份文法。[安装版验收用例](../design/ParserCombinatorLibSupport.md#packaged-production-parser) · [论文中的 CoD parser 应用](../papers/AlgebraicEffectsIsAllYouNeed.md#the-cod-parser-as-an-application-of-the-parsing-hlvm)

[Parser combinator lib support 预研 WIP](../design/ParserCombinatorLibSupport.md) 要求可复用 grammar 与解析结果属于不同的实际 owner。首选研究接口用四个同步 `here` 回调区分消费与否、成功与失败，以 Megaparsec 9.8.3 对照分支路由、状态恢复、诊断和整文件恢复。可绑定的 `proc` 接收者视图及区域内异构子规则仍待验证。

### 怎样检验 CoD 的表达力

选一条右操作数依赖已读运算符的真实产生式。目标是让作者按文法顺序写出读取左侧、查询结合强度、再解析右侧的过程。检验命名 `proc`、有类型的动态作用域 effects 和同步 `here` 正文怎样承担 Edh 中 monadic sequencing 的作用，让读者能从产生式看出值依赖、局部语法环境和失败处理位置。操作符声明的可见性和结合规则仍由前面的操作符议题决定。库接口、解析状态的表示和失败的传递方式留给原型比较。[动态调用与绑定](../design/SymbolicDynamicScope.md#lexical-selection-installation-and-invocation) · [同步正文的控制语义](../design/CallableStorageAndHereAccess.md#an-effect-in-a-here-callback)

- **分支试探。** 让两个候选共享一个前缀。比较左支未消费即失败、消费后失败、显式允许回溯这三种情形。观察右支从哪里开始、失败位置是否保留、左支暂建的 AST 节点或候选诊断能否泄漏。恢复动态绑定与撤销对其引用目标的写入需要分别说明。[Megaparsec 的回溯与前瞻](https://github.com/mrkkrp/megaparsec/blob/0cae4396a870b7fa5058e957efbec188a224dda5/Text/Megaparsec/Internal.hs#L428-L449)
- **整文件恢复。** 用同一过程体中两条缺初始化器的声明及后续合法 `return`，区分尚待选择的分支错误与已经发布的独立报告。核对两个零宽源码范围、错误节点的角色、保留下来的返回语句，以及物理资源耗尽时的未完成结果。[整文件恢复契约和样例](../applications/cod-booter/README.md#whole-file-parser-recovery)

### 何时考虑回移

生产 parser 完成并在正常编译入口使用后，选一条双方都支持且带局部恢复的产生式作对照，例如 `val` 初始化器。用相同输入比较 token 与源码范围、AST 分组、坏输入后的有效语法和诊断次序，同时记录源码可读性、对 seed 能力的要求、运行时间及峰值工作空间。这些结果用于决定保留现有实现，或回移有独立收益的词法、恢复或文法组件。

<a id="effect-composition-and-focused-contexts"></a>

## 8. Symbolic effects 的可组合性与带焦点的上下文计算

[组合的边际成本论文](../papers/MarginalCostOfComposition.md)以质检候选模型的影子上线为例，研究独立扩展时应用作者需要改动哪些代码、掌握哪些上下文。下列路由成本与局部采样问题为这一研究提供可分别检验的条件。普通动态绑定、`perform` 与 `behave` 的选择以及实际 owner 的存活条件沿用[已有契约](../design/SymbolicDynamicScope.md#captured-values-and-call-behavior)。下列问题留给语言与库的后续设计。

论文中的标量阈值和坐标遵循已确认的[值符号声明规则](../design/SymbolicDynamicScope.md#declaring-a-bits-value-symbol)。当 `T` 解析为 bits 类型时，`symbol @x:T` 是 `val symbol @x:T` 的简写。两种写法使用同一套安装、读取和作用域恢复语义。

| 待讨论问题 | 用什么程序区分候选设计 |
| --- | --- |
| 新能力穿过无关作用域时，哪些配对可共用查找或转发，哪些需要单独的解释？ | 让两位库作者分别增加阈值标记和图块层。把兼容性检查、需要修改的声明与必须定制的行为分别列出。图块验证失败时，尝试审计与正式缺陷报告应有各自可检查的结果。 |
| 能否把透明组合中作者需要提供的实现规模从逐对适配降为 M + N，并保住已承诺的行为？ | 用[三个 State、五个 Writer 的源码对照](../papers/examples/ComposedTileReport.md)检验同型实例的身份、两个暂存输出的联合提交及三份状态在拒绝后的更新。交换重试和发布作用域，再独立加入第九个日志。[调用模式对照](../papers/examples/ScopedTileAudit.md)单独检查回放与恢复。单列交互策略、中央分发器和生成器输入的成本；与已有模块化效果系统比较，并用 owner 替换检验拒绝条件。[命题与适用前提](../papers/MarginalCostOfComposition.md#additive-source-extension) |
| 在既有 callable 和 owner 规则下，能否给任意合格的只读局部计算提供可复用的重新聚焦接口？ | 固定输入图像和边界策略，比较按需北移、物化中间栅格、错误回放中心坐标。中心得分应分别为 15、15、0；换实际 owner 后旧视图的使用必须拒绝。 |
| 读取能力、空间依赖范围和可见副作用应怎样分别表达？ | 改东北像素会改变中心结果，改西南像素不会。请求同一位置两次时，缓存与重算可以得到相同分数，但可能发出不同数量的标记。分别规定预取、诊断及提交所需的承诺。 |

<a id="julia-package-portability"></a>

## 9. 用 julia.cod 降低 Julia 包的移植成本

`julia.cod` 的具体数组接口与步进列视图的移植讨论，等 `e-wrks/cod` 仓库建立后再展开。

用一个单样本 t 检验打通 CoD 调用 Rmath 的浮点结果，再让另一个智能体移植按列计算的距离矩阵，观察数组 API 还需要多少适配和人工纠正。对于 Julia 和 CoD 都用得上的操作，优先研究在 CoD 本身采用一致语义。首例是显式 `x : f64`：用不能精确表示的整数区分舍入结果，并单独决定精度警告。矩阵按列遍历、视图写回和底层存储的实际 owner 则决定数组包能复用到什么程度。[预研 WIP 与对照用例](../design/JuliaPackagePortability.md) · [现有数值转换契约](../design/ScalarUnitsAndIndexTypes.md#explicit-numeric-conversion-and-loss-warnings)

## 生产阶段的包管理

生产包中的 C、C++ 和目标汇编（`.s`、`.S`）由 `cod` 或 `cfe` 使用 [`codup` 配套工具链](../applications/cod-booter/docs/CompilerPipeline.md#codup-toolchain-for-native-packages)构建。包管理留给未来的 `e-wrks/cod`。重启讨论时，从一个需要获取、选择版本和复现依赖的实际项目出发。旧 `codp` 的命令和 YAML 格式仅作历史参考。当前 seed 与 booter 使用[手工准备的项目目录](../design/CodProjectStructureAndPkgMgmt.md#directory-layout)。

## 按实际用例重启的小议题

| 议题 | 何时值得讨论、需要确定什么 |
| --- | --- |
| overload 中的 `default` | 有更具体的方法需要调用本族更通用的实现时，比较显式调用与委托的选取规则，涵盖多个候选及递归。模块的 `default { ... }` 与默认导出已经有独立决定。[模块选择](../design/ModuleWorlds.md#default-and-complete-public-selections) |
| 初始化与分支结果 | 用真实调用检验非 bits 或产生所有权的分支如何合流。另有无 owner 的纯 bits 构造表达式候选；目前新建实例写 `make`。[构造与分配](../design/ConstructionAndAcquisition.md) · [owner 和初始化](../design/RegionalOwnership.md) |
| 结构化结果与 generator 的扩展 | 当现有 receiving spec、`yields`、反馈和同步 `here` 无法表达某个调用时，以该调用界定需增加的行为。[generator 与循环](../design/RangedForAndGenerators.md) |
| 尾调用、分数和单位 | 有实际递归深度要求时讨论尾调用的可保证范围及 ABI 条件。固定宽度分数依已有草案保留兼容性；更广的单位与索引运算需要各自的用例及取舍。[尾调用](../design/ConstFunctionsAndTailCalls.md#tco-and-the-callable-abi) · [分数](../design/FixedWidthFractions.md) · [单位与索引](../design/ScalarUnitsAndIndexTypes.md) |

## 开放式异步管道

用两路传感器共同报告一台泵的状态比较不同写法。让两路以不同速率到达，并加入迟到事件：谁负责缓冲，何时关闭窗口，消费者跟不上时生产者观察到什么？以同一输入的可观察结果对照已有复杂事件处理系统。[异步管道研究](../papers/EventEffectPropagation.md)

## Actor 与 chan

状态：已启动预研；基础执行与消息语义已确定，具体接口和生命周期继续讨论。

CoD 面向一项任务调动多个线程、进程或集群机器的重任务场景。每次 `go` 都必须有目标资源池，即使入口不带参数。不指定目标池的启动不成立。swarm 的实现形式包括线程池、本机进程池和动态招募的跨机进程池。每个运行中的 go process 占用 worker 上的专属 OS 线程。两种入口都能移交所有权：以候选拼法 `go on(p) entry_proc(...)` 为例，按入口过程签名中的 `own region`、`own handle` 形参接收实参的所有权；双花括号正文则用显式列表接收。两种写法都从新的初始 TLS 开始，调用者已安装的动态绑定不会传入。Actor 间通过静态强类型的 `chan` 通信。[设计原则](../design/RegionalSemanticPlatformCharter.md#核心原则) · [启动写法](../design/SymbolicDynamicScope.md#go-process-launch)

基础发送采用 UDP 式语义，不保证接收、不提供回执。应用按需建立可靠流、重试和业务结算协议。
当系统有依据判断后续发送大概率失败时，应向发送者报系统故障；
其余情况提供“发后不管”的发送语义。
[已确认边界](../design/SymbolicDynamicScope.md#actor-concurrency-direction)

业务预研参考 [nedh 的网络通信](https://github.com/complyue/nedh) 和
[sedh 的集群调度](https://github.com/complyue/sedh)，重点检验请求携带回信端点、
多个 worker 汇流、资源征召、任务结算与取消。
[预研与源码依据](../design/ActorChannels.md)

跨网络的 Actor 通过 UDP 上的 AKCP 流传输任意长度的 packet region。packet region 始终不关联资源池，经类型化 `chan` 传递，两端可以属于不同的池。流中允许有缺口，传输以整个 region 为单位原子地成功或失败。接收 Actor 只能看到完整且通过校验的 region。传输支持队列和背压，容量恢复时可以继续推进。[区域传输](../design/ActorChannels.md#regional-transport-over-akcp)

普通 region 在 `make` 时无须指定 swarm，可作为本地工作空间，也可在满足消息契约时作为 packet 经 `chan` 发送。swarm region 的所有权可随启动参数在同一 swarm 内移交。即使同一 swarm 的两个线程共享地址空间，向 `chan` 发送 swarm region 也不合法。同机进程可用共享内存支持启动时的移交。待检验的选择包括成员身份怎样进入静态签名、如何验证动态招募的 worker，以及队列何时接下释放责任。[资源池与共享内存示例](../design/ActorChannels.md#resource-pools-and-ownership-handoff)

swarm 内 worker 之间的所有权移交由 swarm provider 提供操作，并显式声明支持哪些资源和传输路径。动态招募时，一个入口的所有 owning 实参都要能交给同一个 worker。声明形式及选定 worker 后的故障处理仍待设计。[provider 声明示例](../design/ActorChannels.md#declaring-a-swarm-providers-handoff-operations)

发送端和接收端可独立建模。其正式类型表达等待 higher-kinded type synonym，再用 producer、transformer、consumer 描述。[高阶类型别名议题](#higher-kinded-type-synonyms) · [管道类型语言讨论](#named-signatures-and-pipelines)

| 待讨论问题 | 区分方案的用例 |
| --- | --- |
| 如何跨记录划分 region、识别缺口并回收残片？ | A 的首块、中间块或末块分别永久丢失，B 完整到达。A 的根不可见，B 可独立交付，A 的迟到片不能复活它。 |
| 如何对组装和就绪队列计量、施加及解除背压？ | 接收 Actor 暂停后恢复。一个远大于传输窗口的 region 在资源充足时仍可完成。明确等待期间的 owner 及关闭后的释放责任。 |
| 两端各有哪些权限，如何建连、关闭和传递回信端点？ | worker 只持发送端，collector 只持接收端。检验不相容的消息、方向和多接收者情形。 |
| 系统故障如何报告，消息 owner 在哪里？ | 比较入队前、入队后和交接后发生故障。区分已消耗的发送者名字、接收端丢弃与应用结算。 |

若 OS 线程创建失败，需确定错误如何报告，以及谁仍负责释放移交列表中的 region 与 handle。Actor 退出、模块生命周期及外部表示的兼容性也有待细化。[讨论清单与验收场景](../design/ActorChannels.md#decisions-to-discuss-next)
