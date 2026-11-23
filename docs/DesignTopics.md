# CoD 远期后备讨论主题

这里记录留待具体用例检验的远期设计问题。各项列出已有契约、待决定的行为，以及可以区分候选方案的例子。

## 讨论依据

- 以当前设计为准。[设计权威登记表](../design/DesignAuthorityRegistry.md)记录了各项决定的确认范围；本索引中的候选写法和验收问题不构成实现授权。
- 开题时给出具体程序、前提和可观察结果。形成新决定后同步更新对应设计、登记表和简短 changelog，移除已解决的后备问题。
- Idiomatic CoD 沿用[写作与验收规范](IdiomaticsGuideline.md)。示例遵循已确认的语言与源码风格。

## 1. 通用 collections API

状态：远期后备

### 已敲定的范围

集合层面需要区分观察、取走成员与消费遍历。

`UnhandledDeferError.exceptions` 这一具体实例已经有独立成员所有权、
只读枚举及单成员转移契约。该实例的确认不自动批准完整的通用集合接口。

### 待决策问题

1. 首批通用协议是否限定为有序、可索引集合？
   类型如何声明或满足该协议？map/set 是否明确留在其他范围？
2. 观察、取出单个成员、消费遍历的最终名称与完整签名是什么？
   `visit`、`take_at`、`drain` 当前是提议名称，需要明确确认或替换。
3. 各操作的 element mode、访问权限、实际 owner、返回值和 owning 接收形式如何表达？
4. 遍历期间的结构修改、移除后的顺序、提前退出及异常路径，
   对已移除成员和剩余成员分别有什么保证？

### 完成标准

- 确定一张操作签名、访问权限与所有权交接表。
- 给出空集合、普通 bits 集合和成员分别属于不同 owner 的集合示例。
- 明确说明取走成员 B 后，A/C 的访问、索引变化及 owner 身份。
- 明确剩余集合与取走的 B 各自承担的释放责任。
- 将最终接口与仍未纳入的扩展范围分别记录。

参考：

- [设计依据与已确认范围](../design/CollectionsAndBuiltinContainers.md#design-authority)
- [候选集合操作](../design/CollectionsAndBuiltinContainers.md#proposed-common-operations)

## 2. Pipeline 的一般绑定与可变 here 绑定

状态：远期后备；以下两个子问题可以分别处理

### 已敲定的范围

当前 composite 契约已经覆盖操作数角色、三种初始化后的 lexical partial、
有效生命周期内的重复调用，以及库重载返回普通 data-only regional functor。

这些既定形式可以直接作为惯用法依据，不应整体重新置为待定。

### 待决策问题 A：一般 producer 绑定

1. 在已经确认的 pipe 上下文之外，普通嵌套表达式何时表示尚未执行的 producer，
   何时表示普通调用？
2. 例如 `budget(half_price(src))`，是否允许表达组合？若允许，依据什么明确规则？
3. named/defaulted arguments 与 overload 如何参与判断？
   必须拒绝哪些歧义？

### 待决策问题 B：可变 here 绑定

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

## 按实际用例重启的小议题

| 议题 | 何时值得讨论、需要确定什么 |
| --- | --- |
| overload 中的 `default` | 有更具体的方法需要调用本族更通用的实现时，比较显式调用与委托的选取规则，涵盖多个候选及递归。模块的 `default { ... }` 与默认导出已经有独立决定。[模块选择](../design/ModuleWorlds.md#default-and-complete-public-selections) |
| 异常所有权交接后的引用 | 用具体消费方检验两个独立需求：把完整 owner 连同另一个选定的异常目标交给 helper；包装成功后保留此前单独声明、仍指向同一 region 的借用。现有 `from original` 消费原 holder，`e.tip` 可以只读观察。[异常控制](../design/ExceptionControl.md) |
| 初始化、借用与 sum 构造 | 用真实调用检验非 bits 或产生所有权的分支如何合流，或要求 initializer 的 sum 如何在已预留的位置完成构造、切换 variant 后如何处理原字段引用。另有无 owner 的纯 bits 构造表达式候选；目前新建实例写 `make`。[构造与分配](../design/ConstructionAndAcquisition.md) · [owner 和初始化](../design/RegionalOwnership.md) |
| 仅凭名称激活 symbolic handler | 多个定义经模块组合后，如入口确需单写 `effect @p`，需明确选中哪些定义以及何处获取快照。现行定义安装和 `perform` / `behave` 已可直接使用。[定义安装](../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures) |
| 结构化结果与 generator 的扩展 | 当现有 receiving spec、`yields`、反馈和同步 `here` 无法表达某个调用时，以该调用界定需增加的行为。[generator 与循环](../design/RangedForAndGenerators.md) |
| 尾调用、分数和单位 | 有实际递归深度要求时讨论尾调用的可保证范围及 ABI 条件。固定宽度分数依已有草案保留兼容性；更广的单位与索引运算需要各自的用例及取舍。[尾调用](../design/ConstFunctionsAndTailCalls.md#tco-and-the-callable-abi) · [分数](../design/FixedWidthFractions.md) · [单位与索引](../design/ScalarUnitsAndIndexTypes.md) |

## 开放式异步管道

用两路传感器共同报告一台泵的状态比较不同写法。让两路以不同速率到达，并加入迟到事件：谁负责缓冲，何时关闭窗口，消费者跟不上时生产者观察到什么？以同一输入的可观察结果对照已有复杂事件处理系统。[异步管道研究](../papers/OpenEventPipelines.md)

## Actor model 与 mailbox

最后用一次本地发送串起 Actor 身份、动态上下文、排队、接收和关停。程序需要能判断发送成功或失败后谁拥有消息图、队列满时谁负责处理，以及接收方结束时谁释放未消费消息。把 mailbox 的类型与上述行为一起决定。[Actor 与 region](../design/GlobalVariablesDoneRight.md#actor-direction-and-region-holding)
