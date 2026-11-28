# Changelog

## 2026-11-28 — Set the bootstrap boundary for mixin and type-level programming

<a id="h-static-mixin-bootstrap-scope-20261128"></a>
**H-STATIC-MIXIN-BOOTSTRAP-SCOPE-20261128.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirms that booter source uses neither `mixin` nor type-level programming and the seed need not support them. The user proposes single-apostrophe expressions, triple-apostrophe blocks, `t:K` within them and possible Haskell-style signatures and equations. [D-15 r92→93](design/ModuleWorlds.md#quoted-type-level-expressions-and-blocks) continues the [receiving-context and vocabulary identity rules](https://github.com/complyue/cod-project/blob/4e0817757e95a295a4b065b7a09e6a5df52864af/design/ModuleWorlds.md#static-module-parameters-for-mixin) under P-102/P-208. [D-20-BOOTSTRAP r54→55](applications/cod-booter/README.md#trusted-source-and-the-seed) retains ordinary type family applications under P-642. Quotation grammar, static evaluation and result publication remain pending-human for `e-wrks/cod`. Registry r1383→1384.

## 2026-11-28 — Distinguish DBMR storage from a nominal region interface

<a id="h-dbmr-storage-technique-20261128"></a>
**H-DBMR-STORAGE-TECHNIQUE-20261128.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) clarified that DBMR denotes a technique which different products and region providers can use. A custom nominal region may equally use other backing. [D-13 r42→43](design/GraphGrainedMemoryManagement.md#dbmr-as-a-storage-technique) and the charter §7 terminology continue C-05/P-786/P-787 and the [existing nominal contract](https://github.com/complyue/cod-project/blob/845ed7c23f09cb3548a1917e72e3f93f17e7acb6/design/GraphGrainedMemoryManagement.md#opaque-nominal-region-types). [D-22 r77→78](design/ConstructionAndAcquisition.md#dbmr-modes) retains the intrinsic publication guarantees. DSDS draft 4→5 and guide draft 3→4 clarify stock provider choices under [D-DBMR-SERVICE-RESEARCH](design/DesignAuthorityRegistry.md#d-dbmr-service-research), preserving the service-level acceptance under P-752/P-753. Registry r1382→1383.

## 2026-11-27 — Separate field and receiver entries with commas

<a id="h-comma-field-and-receive-lists-20261127"></a>
**H-COMMA-FIELD-AND-RECEIVE-LISTS-20261127.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirmed the list-versus-block distinction, the affected forms, one optional trailing comma, and a single internal cutover without a compatibility period. [D-07 r238→239](design/DomainLanguageDesign.md#comma-separated-fields-and-receivers), [D-10 r280→281](design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux) and [D-22 r76→77](design/ConstructionAndAcquisition.md#receiving-specifications) supersede the [semicolon-terminated receiving item](https://github.com/complyue/cod-project/blob/1990037033799e3a98b4c1b8afd16d3f21761bdb/design/ConstructionAndAcquisition.md#receiving-specifications) and the corresponding field and member separators. Typed enums r8→9, D-13 r41→42 and the idiomatics guideline r5→6 and guide r21→22 apply the rule. Other affected design, paper and booter examples carry the spelling change. Field order, selector meaning, executable initialization, layout and evaluation are continued under P-515–P-517/P-570–P-572/P-658–P-659. Registry r1381→1382.

## 2026-11-27 — Apply vocabulary identity to static mixins

<a id="k-mixin-vocabulary-type-identity-20261127"></a>
**K-MIXIN-VOCABULARY-TYPE-IDENTITY-20261127.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) pointed to the existing receiving-context mixin and shared vocabulary principles. [D-15 r91→92](design/ModuleWorlds.md#static-module-parameters-for-mixin) closes the [cross-module result identity question](https://github.com/complyue/cod-project/blob/0259d7410d153385bc27c3c770de99a3e03e95cb/design/ModuleWorlds.md#static-module-parameters-for-mixin) by applying [body template semantics](#h-mixin-template-deferral-20261124) and [H-242](#h-242). Independently mixed-in declarations define distinct nominal types. Selecting a type defined once in a vocabulary preserves its identity across clients. Static parameter syntax and evaluation stay deferred. P-102/P-208 continued, registry r1380→1381.

## 2026-11-27 — Name the type family identity and locate future type-level programming in mixin

<a id="h-type-families-and-static-mixin-params-20261127"></a>
**H-TYPE-FAMILIES-AND-STATIC-MIXIN-PARAMS-20261127.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) chose *type family* for the identity introduced by `data deque[T]`, following the method-family analogy, and expressed a preference for type- and kind-level static module parameters supplied at the mixin site. [D-07 r237→238](design/DomainLanguageDesign.md#type-families-and-member-types) supersedes the [earlier reservation of the term](https://github.com/complyue/cod-project/blob/43665dd1a125ac4e6ffba155f79ccebed9b0b484/design/DomainLanguageDesign.md#generic-adts-and-member-types). Member identities, canonical argument order and re-exports continue. [D-15 r90→91](design/ModuleWorlds.md#static-module-parameters-for-mixin) records the future direction under the [existing template contract](#h-mixin-template-deferral-20261124). Syntax, full evaluation semantics and identity of fresh template declarations remain open. P-18/P-102/P-208/P-229, registry r1379→1380.

## 2026-11-27 — Reserve the term type family for type-indexed definitions

<a id="h-type-family-term-reservation-20261127"></a>
**H-TYPE-FAMILY-TERM-RESERVATION-20261127.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) asked to keep the Haskell sense of *type family* available if CoD could support an analogous mechanism. [D-07 r236→237](design/DomainLanguageDesign.md#generic-adts-and-member-types) uses *generic ADT* for `deque` and *nominal type constructor* for its role in type formation. This supersedes the generic ADT name in the [preceding decision](https://github.com/complyue/cod-project/blob/b75681294647fbbebb1da9e2c96ca4cff541a106/design/DomainLanguageDesign.md#nominal-type-families-and-member-types). *Member type*, nominal identity, canonical arguments and the [D-15 r89→90 import rules](design/ModuleWorlds.md#identity-records-aliases-and-publication) continue. The possible language feature is uncommitted. P-18/P-208/P-229, registry r1378→1379.

## 2026-11-27 — Name nominal type families and their member types

<a id="h-nominal-type-family-terms-20261127"></a>
**H-NOMINAL-TYPE-FAMILY-TERMS-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user proposed “`deque` 叫作 "type family"，`deque[UInt64]` 看作它的一个 member type”, specified independent family identities for declarations in different modules and delegated the terminology choice. [D-07 r235→236](design/DomainLanguageDesign.md#nominal-type-families-and-member-types) adopts *nominal type family* and *member type*. [D-15 r88→89](design/ModuleWorlds.md#identity-records-aliases-and-publication) applies that identity through aliases and re-exports. The [prior generic bracket and ctor contract](https://github.com/complyue/cod-project/blob/0a41b7df7ae9351098ca4f0da0eef7b666e88719/design/DomainLanguageDesign.md#brackets-in-generic-type-heads), ordered canonical arguments, value-only container items and world-local method selection continue. P-18/P-208/P-229, registry r1377→1378.

## 2026-11-27 — Distinguish make by ctor from make by init

<a id="h-make-by-ctor-and-init-20261127"></a>
**H-MAKE-BY-CTOR-AND-INIT-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user named `make deque[UInt64]()` “make by ctor” and the braced form “make by init”. [D-22 r75→76](design/ConstructionAndAcquisition.md#make-by-ctor-and-make-by-init) defines the terms for ordinary ADT construction. D-07 r234→235, D-30 r22→23 and the idiomatic guide r20→21 apply them to sum alternatives and the deque. The [prior behavior](https://github.com/complyue/cod-project/blob/d22d127aa3bbfd30ee8fdd3310a7c04dbaf6039f/design/ConstructionAndAcquisition.md#typed-reservation-and-completed-construction) is continued, including bare reservation, the fieldless shorthand and public deque construction through its ctor. P-502/P-590/P-601/P-602/P-694/P-695, registry r1376→1377.

## 2026-11-27 — Construct an empty deque through its initializer and distinguish sum payload forms

<a id="h-deque-ctor-sum-braces-20261127"></a>
**H-DEQUE-CTOR-SUM-BRACES-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user requested deque construction through a ctor, the shorthand `make Nothing in r`, and confirmed “positional 和 labeled 语法禁止互通” and “Maybe[T] 也是容器，它的 T 也应该仅 val 语义”. [D-30 r21→22](design/CollectionsAndBuiltinContainers.md#bootstrap-linked-deque) replaces caller-written deque initialization with its public zero-argument ctor and applies value-item storage to Maybe. [D-07 r233→234](design/DomainLanguageDesign.md#positional-and-labeled-sum-payloads) requires direct initialization and case reception to follow the alternative’s declared payload form. [D-22 r74→75](design/ConstructionAndAcquisition.md#typed-reservation-and-completed-construction) completes a bare fieldless alternative when it has zero visible initializer overloads. Constructor-backed reservation, ordinary owner/transfer rules and the [prior type-application grammar](https://github.com/complyue/cod-project/blob/6a9042c228f237a4720f227ba7353766e2d86ede/design/DomainLanguageDesign.md#brackets-in-generic-type-heads) continue. P-502/P-590/P-601/P-602/P-625/P-626, registry r1375→1376.

## 2026-11-27 — Use brackets in generic product and type-application heads

<a id="h-generic-type-brackets-20261127"></a>
**H-GENERIC-TYPE-BRACKETS-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user selected the proposed bracket grammar subject to identifying a material conflict: “选项1如果是唯一合理语法就这样定，如果还有quirks需要考虑不同的语法，你再跟我讨论。” [D-07 r232→233](design/DomainLanguageDesign.md#brackets-in-generic-type-heads) confirms `data Link[T]`, applied nominal types in declarations and explicit applied construction heads. Named arrays, value indexing, method owner binders and the [value-only item rule](https://github.com/complyue/cod-project/blob/d1cb3373a3bb6ebdbe354a197f47d5a04b94f2d8/design/CollectionsAndBuiltinContainers.md#container-pointer-results) continue. Expression-level `x:T[i]` and positional sum payload syntax are outside the decision. P-18/P-229/P-590, registry r1374→1375.

## 2026-11-27 — Generic containers store items by value

<a id="h-value-only-container-items-20261127"></a>
**H-VALUE-ONLY-CONTAINER-ITEMS-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user proposed “容器始终实现 `val` 语义的item，pop/lookup 都返回ptr” and confirmed the proposal with “按此决定”. [D-30 r20→21](design/CollectionsAndBuiltinContainers.md#container-pointer-results) binds type parameters to types and chooses value or reference mode at each declaration. The deque and map store value items and return nullable pointers to item storage. This retires the [direct reference-element rule](https://github.com/complyue/cod-project/blob/fc5ff84cad54ceb5d4ac06b45fd6ccf721c90088/design/CollectionsAndBuiltinContainers.md#container-pointer-results) and drops the proposed parallel Maybe-returning methods. Live lookup, shallow detached pop, owner lifetime, hash width and ordinary reference fields are continued. The value-sum deque probe remains a bootstrap witness. P-589/P-590, registry r1373→1374.

## 2026-11-27 — Lookup and pop of a ref element select its current target

<a id="h-container-selected-ref-target-20261127"></a>
**H-CONTAINER-SELECTED-REF-TARGET-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user confirmed “ref 元素的 lookup 和 pop 都返回当时选中的 target”. [D-30 r19→20](design/CollectionsAndBuiltinContainers.md#container-pointer-results) specifies that after a map entry rebinds from A to B, the earlier result still selects A and a new lookup selects B. The [prior shallow pop rule](https://github.com/complyue/cod-project/blob/5650020e153a89d52e9eabf2a70c10d70c303b9b/design/CollectionsAndBuiltinContainers.md#container-pointer-results), nullable result, actual owner and access continue under P-590/P-162. Registry r1372→1373.

## 2026-11-27 — Deque pop preserves the detached slot’s immediate value

<a id="h-detached-pop-slot-20261127"></a>
**H-DETACHED-POP-SLOT-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user specified “只承诺该 ptr 的slot值不会再变，不要承诺深度对象不变哈”. [D-30 r18→19](design/CollectionsAndBuiltinContainers.md#container-pointer-results) confirms the scope of the [r18 proposal](https://github.com/complyue/cod-project/blob/798e07f56ba93b1dd6e55aef764c61bce76d307d/design/CollectionsAndBuiltinContainers.md#container-pointer-results). Later deque operations preserve the immediate slot value. A write through an alias to a referenced node remains observable. The map lookup contract, owner lifetime and deferred reuse policy continue under P-589/P-590/P-162. Registry r1371→1372.

## 2026-11-27 — Fix the public hash width and distinguish lookup from removal

<a id="h-hash-width-and-pop-20261127"></a>
**H-HASH-WIDTH-AND-POP-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user decided “那就决定用 UInt64 hash 值” and clarified “我前面说的不承诺不会变主要针对lookup语义哈”. [D-30 r17→18](design/CollectionsAndBuiltinContainers.md#public-hash-result) fixes the public hash result independently of a table’s storage width. The [container result contract](design/CollectionsAndBuiltinContainers.md#container-pointer-results) narrows the [previous reuse example](https://github.com/complyue/cod-project/blob/f12c7e14b9ca69737a0158033c8b06039bc00442/design/CollectionsAndBuiltinContainers.md#container-pointer-results) to lookup and records the user’s proposal for a detached pop. Actual owner, access and deferred regional allocation policy continue under P-162/P-589/P-590. Registry r1370→1371.

## 2026-11-27 — Copy a container result when its observed value must be retained

<a id="h-container-pointer-results-20261127"></a>
**H-CONTAINER-POINTER-RESULTS-20261127.** The project user proposed pointer results for pop and lookup, then clarified in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “容器返回的ptr不要承诺后续不会变，让caller形成想长期用就要复制的心智模型” and assigned reuse optimization criteria to the later regional memory management design. [D-30 r16→17](design/CollectionsAndBuiltinContainers.md#container-pointer-results) selects the nullable pointer result under the [existing owner rule](https://github.com/complyue/cod-project/blob/c391cc0d94416eb5348895a9c2487cc5aed0161e/design/CollectionsAndBuiltinContainers.md#bootstrap-linked-deque). A saved independent payload survives subsequent reuse. The first implementation can also retain its storage. P-590/P-162, registry r1369→1370.

## 2026-11-27 — A reference may never outlive its actual owner

<a id="h-ref-outlives-owner-20261127"></a>
**H-REF-OUTLIVES-OWNER-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user clarified “CoD 只考虑 ref 是否 outlive  owner ，没有其它outlive要检查阻止。” This continues [H-190](#h-190) and the [previous typed-slot rules](https://github.com/complyue/cod-project/blob/5d33e907b2e28f814df7c76dab5e3e2ba1d91671/design/RegionalOwnership.md#reuse-is-not-whole-region-release). A retained reference permits reuse of live typed storage. A library’s by-value result retains the value delivered at removal. [D-14 r140→141](design/RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy), [D-30 r15→16](design/CollectionsAndBuiltinContainers.md#bootstrap-linked-deque), P-162, registry r1368→1369.

## 2026-11-27 — Prioritize source ergonomics in the bootstrap

<a id="h-booter-source-ergonomics-20261127"></a>
**H-BOOTER-SOURCE-ERGONOMICS-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user directed “应该先落实seed支持” for the independent nominal bits result and placed the generic CoD collection and its seed support ahead of the CoD-written regional storage manager and vector. The [previous resolver](https://github.com/complyue/cod-project/blob/eadfbd47806db457105204e49fba62e43ead830d/applications/cod-booter/src/PackageGraphPaths.cod#L200) receives its four-way result in a one-element array. The user identified ordinary products for related same-owner operands. Asked whether the first public linked collection should be a deque with private nodes and no separate list, the user confirmed “Yes, deque only for bootstrap”. [D-30 r15](design/CollectionsAndBuiltinContainers.md#bootstrap-linked-deque) records that choice. The [roadmap r53→54](applications/cod-booter/README.md#booter-source-ergonomics) continues the [booter-driven seed scope](#h-booter-driven-seed-20261119), the existing result-destination contract and whole-region ownership. D-20-BOOTSTRAP, P-642/P-643, registry r1367→1368.

## 2026-11-27 — Bootstrap recovery acceptance after seed retirement

<a id="k-bootstrap-recovery-acceptance-20261127"></a>
**K-BOOTSTRAP-RECOVERY-ACCEPTANCE-20261127.** Editorial continuity for [D-ARCH-REVIEW r228→r229](design/DesignAuthorityRegistry.md#d-arch-review), P-682/P-683, registry r1366→1367. The [prior review](https://github.com/complyue/cod-project/blob/70e456f507b69cf2e9a034ea87238e2d42b177c7/design/ImplementationArchitecture.md#whole-file-recovery-in-the-bootstrap-compilers) still prescribed paired runs. The [project user’s switch retirement](#h-seed-shared-switch-retirement-20261126), [single-build scope](#k-bootstrap-verification-scope-20261125) and [terminal verifier removal](https://github.com/complyue/cod-project/commit/42ca800c8f212340dd687e4b871678f78db385b5) govern the current commands. Recovery still requires valid suffixes, located diagnostics, safe handoff and preserved outputs. The independent-body recipe is archived.

## 2026-11-27 — Build native package providers with the CoD toolchain

<a id="h-native-package-toolchain-20261127"></a>
**H-NATIVE-PACKAGE-TOOLCHAIN-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user requested “支持 CoD pkg 内含 C++ 代码（只要跟CoD的接口始终是C）” and confirmed the proposed toolset and provider model with “好的，按此决定，更新相关文档： 论文、产品指南、特性规划，包括codup的”. The [production distribution plan](applications/cod-booter/docs/CompilerPipeline.md#codup-toolchain-for-native-packages) covers C, C++ and target assembly, host execution via `cod`, and cross-builds via `cfe`. Source builds for Rust, Zig and Fortran are deferred. [Nominal region constructors and methods](applications/cod-booter/docs/LLVMInterop.md#native-package-providers-for-nominal-regions) use generated C headers and adapters, separate authored sources and the target C ABI. A stable provider binary interface across releases requires a separate consumer. The [r40 nominal type boundary](https://github.com/complyue/cod-project/blob/3627a2d0b1a8fa6f91b775479763c4b484f69e77/design/GraphGrainedMemoryManagement.md#opaque-nominal-region-types) and [prior host/target contract](https://github.com/complyue/cod-project/blob/3627a2d0b1a8fa6f91b775479763c4b484f69e77/applications/cod-booter/docs/CompilerPipeline.md#production-cross-builds-and-target-runtimes) continue. DSDS draft 4 and guide draft 3 apply the decision to the reservation provider. Registry r1365→1366, P-753–P-755 and P-786/P-787. Toolchain and provider implementation remain planned.

## 2026-11-27 — Nominal regions have exact types and method-only interfaces

<a id="h-nominal-region-types-20261127"></a>
**H-NOMINAL-REGION-TYPES-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user proposed `region type` with C-implemented constructors and methods and specified “nominal typed region 没有roots，只能通过methods接口与CoD程序交互” and “要求 nominal typed region 总是精确匹配类型，而不得有roots要求”. [D-13-NOMINAL-REGIONS](design/GraphGrainedMemoryManagement.md#opaque-nominal-region-types) excludes the suggested structural projection, including one under a transaction loan. D-14r140 and D-22r74 continue the [prior structural root contract](https://github.com/complyue/cod-project/blob/3b9cf62b591d63b31175fbf364c04e76efd5ba7c/design/GraphGrainedMemoryManagement.md#named-root-access-and-receiving-specifications). DSDS draft 3 applies the boundary to the specialized stock database. Provider ABI, commit and recovery protocols and compiler support remain open. P-786/P-787, registry r1364→1365.

## 2026-11-27 — Dispatch with numeric literals still untyped

<a id="h-dispatch-before-literal-typing-20261127"></a>
**H-DISPATCH-BEFORE-LITERAL-TYPING-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user specified “untyped literal 先参与 dispatch，确定唯一候选后，再按参数签名推断类型” and “其它类型推断穷尽仍无果以后，才参考 `default` 规则”. [D-11-LITERAL-DEFAULTS r4](design/NumericLiterals.md#dispatch-before-literal-typing) makes that order explicit for numeric arguments, including operator calls. [D-10r280](design/SymbolicDynamicScope.md#named-arguments-defaults-and-complete-binding) incorporates it in public signature selection. The [r3 baseline](https://github.com/complyue/cod-project/blob/8fc2731781370a0186ac592dfdd011985ddc9041/design/NumericLiterals.md) continues for integer representability, floating applicability and standalone defaults. P-782/P-783, registry r1363→1364.

## 2026-11-27 — Keep floating-point rounding out of overload selection

<a id="h-floating-literal-signature-matching-20261127"></a>
**H-FLOATING-LITERAL-SIGNATURE-MATCHING-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user specified “浮点literal值是否有损，完全不该影响dispatch签名匹配，匹配上了就是备选，多个备选匹配就reject，同样不涉及default规则”. [D-11-LITERAL-DEFAULTS r3](design/NumericLiterals.md#floating-literals-in-overloaded-calls) rejects an ambiguous call to `Float32` and `Float64` methods whether both, neither or only one represents the literal exactly. The [r2 integer and standalone rules](https://github.com/complyue/cod-project/blob/f6c1c726f4a2ed2131c8baca6a6d3040a7afd3ba/design/NumericLiterals.md#numeric-defaults-in-source-order) and typed-value conversion rules continue. P-782/P-783, registry r1362→1363.

## 2026-11-27 — Use the written call form to order OO and free overloads

<a id="h-dispatch-by-call-form-20261127"></a>
**H-DISPATCH-BY-CALL-FORM-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user specified “dot access 时，最大匹配分组是是否OO，variadic与否是内部小分组”. [Named dotted calls](design/DomainLanguageDesign.md#how-receiver-dispatch-works) preserve OO priority. Ordinary [indexing](design/CollectionsAndBuiltinContainers.md#abstract-defaults-and-free-indexing-operators) groups fixed methods across families before packs. The user’s default-completion rule keeps a fixed method eligible when its trailing argument defaults. This revises the [earlier scoped order](https://github.com/complyue/cod-project/blob/0fcb9cb2180dbde224e567550c04a847e50d492f/design/CollectionsAndBuiltinContainers.md#fixed-arity-oo-indexed-reads). Explicit pack dispatch, module visibility, once-only preparation and the production named-fallback diagnostic continue under D-07r232, D-10r279, D-15r88, D-30r14, P-403/P-404 and P-778–P-781. The user deferred the concrete Julia array interface and operator-declaration discussions until the `e-wrks/cod` repository exists. Registry r1361→1362.

## 2026-11-27 — Reject competing perfect numeric literal matches at the call

<a id="h-perfect-numeric-overload-matches-20261127"></a>
**H-PERFECT-NUMERIC-OVERLOAD-MATCHES-20261127.** For the [two fixed overloads](design/NumericLiterals.md#numeric-defaults-in-source-order) whose parameter types occupy positions (1, 4) and (2, 2) in the caller’s ordered list, the project user chose “static reject，因为两个method都完美匹配上了” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). For the signed/unsigned single-argument pair the user clarified “无类型literal数值可以完美匹配任意无损转换数值类型，这种场景完全不触发default规则吧”. Both calls to `tag(7)` are ambiguous, including when only `Int64` is listed. This supersedes the single-listed-type `send(7)` tie-break in [r1](https://github.com/complyue/cod-project/blob/d891763e5d90d83a0dabdeae7fd941eaa4ba9877/design/NumericLiterals.md#numeric-defaults-in-source-order). D-11-LITERAL-DEFAULTS r2 retains contextual typing, exact representability and explicit suffixes under P-782/P-783.

## 2026-11-27 — Reject duplicate fixed positional signatures exposed by defaults

<a id="h-defaulted-method-conflict-20261127"></a>
**H-DEFAULTED-METHOD-CONFLICT-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user corrected the proposed `pick(7)` outcome: “你这个例子应该在method定义角度reject”. For `choose`, where *both* fixed methods omit their distinct second parameters, the project user selected “Reject at family formation — both claim the same one-argument signature”. The [r277 example](https://github.com/complyue/cod-project/blob/0fcb9cb2180dbde224e567550c04a847e50d492f/design/SymbolicDynamicScope.md#L1685) used a call-time preference for `pick` and a call-time ambiguity for the two defaulted heads. [D-10r278](design/SymbolicDynamicScope.md#fixed-method-conflicts-from-trailing-defaults) rejects both pairs at their definitions, including when the common signature occurs only through omission. Verify both declaration orders and first composition without a call. [H-474](#h-474) continues for valid families. Ordinary type specialization, explicit default evaluation and fixed-plus-variadic coexistence keep their prior scope. Registry r1360→1361, P-405/P-406.

## 2026-11-27 — Prefer the matching fixed OO method for an ordinary indexed read

<a id="h-fixed-arity-oo-indexed-reads-20261127"></a>
**H-FIXED-ARITY-OO-INDEXED-READS-20261127.** The project user [confirmed](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “确认A=2,B=2。 C的代码写出来看”. In the [two programs](design/CollectionsAndBuiltinContainers.md#fixed-arity-oo-indexed-reads), the fixed one-index abstract default wins whether the variadic indexed read belongs to the concrete child or its storage parent. This qualifies the [prior family priority](https://github.com/complyue/cod-project/blob/41dd59d7491cfbf9708e63518b5bc504805dd1e8/design/CollectionsAndBuiltinContainers.md#abstract-defaults-and-free-indexing-operators) for this operation. Explicit expansion still selects variadic methods. C compares a concrete pack with a free fixed method and awaits its own decision. D-30r13, D-07r231, P-778/P-779. Registry r1359→1360.

## 2026-11-27 — Interpret numeric literals in context and group before recognizing their sign

<a id="h-numeric-literals-and-signs-20261127"></a>
**H-NUMERIC-LITERALS-AND-SIGNS-20261127.** In the [numeric-literals side discussion](codex://threads/01a21c7c-229d-7fb0-8c33-ade7ab502d7b), the project user directed “确实应该是类型推断不能唯一决定类型后才参考default指令” and “不按参数顺序短路决定匹配程度，而是看所有参数匹配程度的总得分去判断”. The user confirmed `UInt8` addition with future certain-overflow advice, floating defaults and written-site scope. After distinguishing `-1` from `- 1`, the user supplied Julia’s `e=2; -2^e` result −4 and approved grouping first, with that example in parser comments. [D-11-LITERAL-DEFAULTS](design/DesignAuthorityRegistry.md#d-11-literal-defaults) and [D-10-LITERAL-SIGN](design/DesignAuthorityRegistry.md#d-10-literal-sign), P-782–P-785, govern the [contract and acceptance](design/NumericLiterals.md). D-11r44 and idioms guide r20 continue the [baseline typed conversion and unit rules](https://github.com/complyue/cod-project/blob/586dacf8ce29658534c0026ca2a8e849bff9eefb/design/ScalarUnitsAndIndexTypes.md#quantity-types-and-lossless-widening). Parser comments record the model for the bootstrap subsets. Runtime and diagnostic delivery are separate. Registry r1358→1359.

## 2026-11-27 — Allow a dotted named call to reach the visible free callable

<a id="h-named-member-free-fallback-20261127"></a>
**H-NAMED-MEMBER-FREE-FALLBACK-20261127.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user chose the option where `x.quote()` returns 9 through the free `quote(x)` and added “选2但是正式编译器的诊断要给出warning，编译时通过开关可降为info”. [D-07-NAMED-FALLBACK](design/DesignAuthorityRegistry.md#d-07-named-fallback), P-780/P-781, extends the [indexing priority precedent](#h-index-defaults-and-free-methods-20261126) to named dotted calls. The [scope and acceptance](design/DomainLanguageDesign.md#when-a-dotted-call-uses-a-free-callable) cover the explicit free call, concrete and abstract methods, the selected module view and production diagnostic. Indexing and data call-operator diagnostics keep their previous scope.

## 2026-11-27 — Explain receiver dispatch across CoD operations

<a id="k-oo-dispatch-model-20261127"></a>
**K-OO-DISPATCH-MODEL-20261127.** **Continued** from the [r229 language scope](https://github.com/complyue/cod-project/blob/c2a1edec79a45c7c692eba2d1b8433983b74f45b/design/DomainLanguageDesign.md#receivers-in-field-accessors-and-member-methods). The project user requested a review of OO family priority and its interaction with receiver notation. [The consolidated model](design/DomainLanguageDesign.md#how-receiver-dispatch-works) applies [member and owner rules](#h-member-names-and-owner-20261124), [super-calls](#h-product-extends-super-20261125), the [OO call operator](#h-oo-call-operator-head-20261125) and [indexing priority](#h-index-defaults-and-free-methods-20261126). D-07r230, D-15r87, D-30r12, portability study r6 and idioms guide r19 distinguish type-carried publication from ordinary callable names. At the user’s request, “下标访问应该确立专属沟通词汇”, the [indexing terminology](design/CollectionsAndBuiltinContainers.md#indexing-terminology) names all three bracket operations. The related D-10r277, D-11r43, D-14r139 and D-ARCH-REVIEWr228 edits carry forward their preceding operation and acceptance rules. The named-call fallback is recorded in the separate human decision above. The interaction of storage-parent and abstract indexing defaults is tracked with the receiver design. Registry r1357→1358.

## 2026-11-26 — Check inline record construction at its indexed destination

<a id="k-booter-indexed-inline-record-20261126"></a>
**K-BOOTER-INDEXED-INLINE-RECORD-20261126.** **Continued** under [D-20-MODULE-GRAPH](design/DesignAuthorityRegistry.md#d-20-module-graph), P-680/P-681. The [actual interface row writer](https://github.com/complyue/cod-project/blob/420b187ead84ade35f457248affc58689f6fd065/applications/cod-booter/src/ModuleExportStorage.cod#L205-L227) supplies an existing readonly sum, a fresh sum alternative and a fresh product. The [r100 source-checking boundary](applications/cod-booter/docs/ModuleGraph.md#checking-an-indexed-record-with-inline-fields) applies the r99 completed storage facts and the existing indexed transfer semantics to this write on declaration and provider routes. Object lowering has separate acceptance. Registry r1356→1357.

## 2026-11-26 — Separate aggregate binding from storage classification

<a id="k-booter-bound-aggregate-schemas-20261126"></a>
**K-BOOTER-BOUND-AGGREGATE-SCHEMAS-20261126.** **Continued** under [D-20-MODULE-GRAPH](design/DesignAuthorityRegistry.md#d-20-module-graph) and the [booter rewrite](applications/cod-booter/docs/CompilerPipeline.md#replacing-the-booter-pipeline). The [previous walker](https://github.com/complyue/cod-project/blob/18e5658cca52e6dd054013d674092ba778e6040b/applications/cod-booter/src/AggregateSchemas.cod#L213-L235) returned to the parent field’s source after completing its inline child. The reviewed [r99 phase boundary](applications/cod-booter/docs/ModuleGraph.md#checking-aggregate-fields-before-publication) keeps traversal over frozen dependency and terminal records. The r98 storage distinctions, diagnostic order and publication requirements continue under P-680/P-681. Mainline owns implementation and linked acceptance. Registry r1355→1356.

## 2026-11-26 — Reuse abstract indexing defaults before visible free operators

<a id="h-index-defaults-and-free-methods-20261126"></a>
**H-INDEX-DEFAULTS-AND-FREE-METHODS-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user selected automatic use of an abstract default (“那肯定是选项1”) and directed “认为OO method比全局free method更具体”. [D-30-INDEX-FAMILIES](design/DesignAuthorityRegistry.md#d-30-index-families) extends the [r10 receiver decision](https://github.com/complyue/cod-project/blob/2d8c18d2968fa0370243a0a6c3cd9ebfda7d32b2/design/CollectionsAndBuiltinContainers.md#receiver-families-for-indexing) to a visible unprefixed indexing family contributed through ordinary callable declarations. D-30r11 / D-15r86 preserve module-world isolation, explicit expansion, once-only evaluation and covered runtime dispatch. This scope covers the three indexing operations. [Examples and acceptance](design/CollectionsAndBuiltinContainers.md#abstract-defaults-and-free-indexing-operators). Registry r1354→1355.

## 2026-11-26 — Give each actual receiver its own indexing families

<a id="h-receiver-index-families-20261126"></a>
**H-RECEIVER-INDEX-FAMILIES-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user endorsed per-type indexing families and concrete ADT dispatch “即使caller用的抽象类型声明变量”. [D-30-INDEX-FAMILIES](design/DesignAuthorityRegistry.md#d-30-index-families) adds the receiver identity and module-view rule to the [prior indexed call contract](https://github.com/complyue/cod-project/blob/ac3cbf00ef69e08b506169c01e4adb2db2216424/design/CollectionsAndBuiltinContainers.md#multidimensional-subscript-overloads). D-30r10 / D-07r229 / D-15r85 carry forward receiver permissions, distinct operations, pack selection and demand-based AOT coverage. How a concrete type acquires an abstract default for a missing shape is open. [Generic writer and adapter acceptance](design/CollectionsAndBuiltinContainers.md#receiver-families-for-indexing). Registry r1353→1354.

## 2026-11-26 — Use the default scalar names in selector examples

<a id="k-selector-scalar-spellings-20261126"></a>
**K-SELECTOR-SCALAR-SPELLINGS-20261126.** **Continued** under [D-07-SCALAR-NAMES](design/DesignAuthorityRegistry.md#d-07-scalar-names). The [published naming decision](https://github.com/complyue/cod-project/blob/421d26afcd8392b122b5198b4c40f40889a61f5f/design/JuliaPackagePortability.md#scalar-names-and-transparent-type-synonyms) applies to source annotations in the [range and slice schemas](design/RangedForAndGenerators.md#concrete-types-of-range-and-slice-literals), [method heads](design/SymbolicDynamicScope.md#slice-patterns-in-method-parameters) and expression conversion example. D-29r28→29 and D-10r275→276 denote the same types and preserve the written numeric suffixes. Registry r1352→1353.

## 2026-11-26 — Use Julia’s fixed-width numeric type names in CoD source

<a id="h-builtin-numeric-names-20261126"></a>
**H-BUILTIN-NUMERIC-NAMES-20261126.** In the [side discussion](codex://threads/01a218df-119e-78f0-8cd7-c920e9c7778e), the project user cited variables such as `i31` and `i32` and approved the proposed fixed-width names, package-selected word aliases and separate literal suffixes (“同意，落repo然后知会你的主对话”). [D-07-SCALAR-NAMES](design/DesignAuthorityRegistry.md#d-07-scalar-names) supersedes the source naming portion of the [previous contract](https://github.com/complyue/cod-project/blob/486327493e0e4e1e8a0aa1e399d985490a0cf50b/design/JuliaPackagePortability.md#scalar-names-and-transparent-type-synonyms). Transparent synonyms, signed and unsigned identities, conversion and native layout continue. The [acceptance and booter migration](design/JuliaPackagePortability.md#scalar-names-and-transparent-type-synonyms) distinguish this design decision from compiler delivery. Registry r1351→1352.

## 2026-11-26 — Give explicit scalar conversions Julia-compatible results and failures

<a id="h-julia-numeric-conversions-20261126"></a>
**H-JULIA-NUMERIC-CONVERSIONS-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user instructed “2 按往Julia靠的原则能决定的现在就决定” after reviewing the rounding and inexact-integer cases. Under that delegation, [D-11-NUMERIC-CONVERSION](design/DesignAuthorityRegistry.md#d-11-numeric-conversion) replaces the [r40 exactness precondition, warning and undefined behavior](https://github.com/complyue/cod-project/blob/ae53eee87569c91964d8d8eea5e9d1ee7d64bb6e/design/ScalarUnitsAndIndexTypes.md#explicit-numeric-conversion-and-loss-warnings) of H-463–H-466 for explicit dimensionless builtin scalar conversions. D-11r41 / D-10r275 / D-17r153 and the Julia study r4 specify direct destination rounding, optional precision advice, exact integer conversion or catchable InexactError, and ordinary ordering through cleanup. H-261 implicit conversion and H-470 nonnumeric assertions continue. Compiler delivery and indexing-family identity are separate. [Acceptance](design/ScalarUnitsAndIndexTypes.md#explicit-scalar-numeric-conversions). Registry r1350→1351.

## 2026-11-26 — Match a slice directly in a method parameter

<a id="h-slice-method-patterns-20261126"></a>
**H-SLICE-METHOD-PATTERNS-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user proposed extending the mirrored slice pattern to method signatures and chose “Allow direct and grouped heads” for `func stride((.. by val d:i64)) -> i64 { d }`. [D-10-SLICE-METHODS](design/DesignAuthorityRegistry.md#d-10-slice-methods) composes the [confirmed slice case shape](https://github.com/complyue/cod-project/blob/234f06019c63f72bb8570fdc708f8ca8caf68d07/design/RangedForAndGenerators.md#matching-a-slice-in-a-case-arm) with [existing method selection](https://github.com/complyue/cod-project/blob/234f06019c63f72bb8570fdc708f8ca8caf68d07/design/SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch). D-10r274/D-07r228/D-29r28 preserve field identity, fixed arity, whole-tuple specificity and explicit variadic dispatch. [Examples and acceptance](design/SymbolicDynamicScope.md#slice-patterns-in-method-parameters). Compiler delivery remains separate. Registry r1349→1350.

## 2026-11-26 — Match the written slice shape in a library case arm

<a id="h-mirrored-slice-case-20261126"></a>
**H-MIRRORED-SLICE-CASE-20261126.** Asked how a library receiving a slice should express `case some_slice of`, the project user chose “Mirror the literal — `(.. by val d) -> use(d)`” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). [D-07-SLICE-CASE](design/DesignAuthorityRegistry.md#d-07-slice-case) extends the [previous case grammar](https://github.com/complyue/cod-project/blob/ad1f2e29e028e2aaa5e7865778cb860c9c01f534/design/DomainLanguageDesign.md#case-expressions-and-branch-local-variables) with a consumer of the [canonical slice ADT](https://github.com/complyue/cod-project/blob/ad1f2e29e028e2aaa5e7865778cb860c9c01f534/design/RangedForAndGenerators.md#concrete-types-of-range-and-slice-literals). D-07r227/D-29r27 preserve actual field types, default versus explicit step, existing case control and library axis policy. See the [example and acceptance cases](design/RangedForAndGenerators.md#matching-a-slice-in-a-case-arm). Frontend support remains a delivery item. Registry r1348→1349.

## 2026-11-26 — Select the pack body on explicit expansion and bind selector literals to concrete ADTs

<a id="h-selector-literals-and-pack-dispatch-20261126"></a>
**H-SELECTOR-LITERALS-AND-PACK-DISPATCH-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user supported fixed specializations plus a variadic implementation and answered the two-index witness “9 — writing an expansion explicitly selects the variadic body”. [D-10-VARARGS r273](design/SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter) supersedes the [fixed-plus-variadic formation ban](#h-variadic-method-family-20261123). Distinct variadic implementations still conflict. Element reception, failures and forwarded-pack rules continue. The requested setter order leaves the indices at the tail and preserves bracket evaluation order. The user asked for “一个公共的 abs 父类型” and delegated how each literal obtains a concrete ADT. [D-29r26](design/RangedForAndGenerators.md#concrete-types-of-range-and-slice-literals) names AxisSelector, Range and Slice and defines the builtin literal instantiations. The [previous selector contract](https://github.com/complyue/cod-project/blob/cb2d45c4c4dc95848e7a14837c201ae6bbf8c204/design/CollectionsAndBuiltinContainers.md#multidimensional-subscript-overloads) supplies the unit, progression and preparation baseline. D-30r9/D-14r138, D-07r226/D-11r40, D-ARCH r227 and D-20-MODULE-GRAPH r98 trace the affected consumers. Implementation and generalized type-application syntax remain separate. Registry r1347→1348.

## 2026-11-26 — Give stepped ranges and typed axis arguments to library subscripts

<a id="h-ranges-and-typed-subscripts-20261126"></a>
**H-RANGES-AND-TYPED-SUBSCRIPTS-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user confirmed the recommended half-open `..`, inclusive `..=` and contextual `by` design, requested “要给lib的overload拿到下标值，包括多维下标” and specified “普通的unitless下标类型我们肯定要支持”. [D-29-STEPPED-RANGES](design/DesignAuthorityRegistry.md#d-29-stepped-ranges) covers range values in loops and subscripts. [D-30-TYPED-SUBSCRIPTS](design/DesignAuthorityRegistry.md#d-30-typed-subscripts) carries each scalar or selector to a library call with its actual type, together with the existing UoM, evaluation order and ownership contracts. This continues the [r2 array-library and colon decisions](https://github.com/complyue/cod-project/blob/6f64f8a19b90d4e3d37a4f8dcd7b8d5b9786fa9b/design/JuliaPackagePortability.md#stepped-subscripts) and the existing one-index call forms. Implementation coverage is separate. Registry r1346→1347.

## 2026-11-26 — Give array conventions and expression grouping to libraries and prioritize type synonyms

<a id="h-library-arrays-fixity-and-type-names-20261126"></a>
**H-LIBRARY-ARRAYS-FIXITY-AND-TYPE-NAMES-20261126.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user directed “不自动using，不占 `Array` 名头，留给 julia.cod 去用”, delegated concrete choices and requested early type synonyms. The user confirmed “Keep signed i64 and unsigned u64. Both lower to LLVM i64. Julia names are transparent synonyms.” For the range conflict the user decided “把range写法转换作为porting jl to cod 的手工步骤要求吧，CoD坚持自己的原味语法”. The [r2 portability design](design/JuliaPackagePortability.md#library-defined-arrays-and-the-native-c-boundary) qualifies library array policy, selected operator fixity and first-order synonyms under P-758–P-763. It continues the [r1 port workloads](https://github.com/complyue/cod-project/blob/5b0c4c5fb88d8857c16d82b12fd79a83f51cf238/design/JuliaPackagePortability.md) and the [tokenization contract](#h-maximal-operator-runs-20261123). Breaking updates to the booter source are permitted where needed for the intended library design. Registry r1345→1346.

## 2026-11-26 — Select an ordinary enum overload before preparing the call

<a id="k-booter-enum-overloads-20261126"></a>
**K-BOOTER-ENUM-OVERLOADS-20261126.** The [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirmed [read-only selection over completed same-revision family signatures](applications/cod-booter/docs/CompilerPipeline.md#ordinary-overloads-on-an-enum-and-its-backing-type). The [baseline object checker](https://github.com/complyue/cod-project/blob/116e98ea7900dbdf7ca16e2cae54519d2aef22a5/applications/cod-booter/src/OperandChecking.cod#L4161-L4188) uses transfer-family checks on a linked list of methods. The new ordinary-call boundary implements the existing [exact enum overload preference](design/TypedEnums.md#using-an-enum-as-its-backing-value) while preserving argument preparation, nominal identity and ambiguity. D-07-ENUM and D-20-STAGE-CONTROLS continue at pipeline r16, registry r1344→1345.

## 2026-11-26 — Schedule booter syntax delivery alongside the pipeline rewrite

<a id="h-booter-syntax-delivery-order-20261126"></a>
**H-BOOTER-SYNTAX-DELIVERY-ORDER-20261126.** The [project user](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirmed the proposed delivery order, “同意，按你的建议安排下去。” Mainline inventories confirmed booter syntax now and identifies the first missing phase for each form. Features that use stable interfaces can ship during the rewrite. Where both changes need the same phase interface, complete that boundary first and implement the feature against it. The [enum assertion decision](https://github.com/complyue/cod-project/blob/0d132ecbf394daa768824dab76fa59a2d89085fe/applications/cod-booter/docs/CompilerPipeline.md#enum-assertions-in-source-and-object-commands) illustrates this order. Start the [final cleanup](#h-post-rewrite-cleanup-20261126) when the rewritten declaration, provider and object commands are delivered, even if independent syntax work remains. Extend the seed for a concrete bootstrap need, including a capability that makes the intended booter source materially clearer. [Bootstrap delivery contract](applications/cod-booter/README.md#initial-seed-delivery)

## 2026-11-26 — Share enum assertion selection across the booter commands

<a id="k-booter-enum-assertion-selection-20261126"></a>
**K-BOOTER-ENUM-ASSERTION-SELECTION-20261126.** At [the linked baseline](https://github.com/complyue/cod-project/blob/403316ef220743db3961b867c38b3ff4b0af9db0/applications/cod-booter/src/ModuleGraph.cod#L573-L594), object binding returns before declaration-expression checking. The [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirmed [one revision-bound selection](applications/cod-booter/docs/CompilerPipeline.md#enum-assertions-in-source-and-object-commands) for the booter declaration, provider and object commands. The user’s [enum coercion and decoding contract](#h-enum-backing-coercion-20261125) governs membership, operand effects and failure. This architecture decision continues D-20-STAGE-CONTROLS and D-07-ENUM. Pipeline r14→15, registry r1343→1344.

## 2026-11-26 — Investigate agent-assisted Julia package ports

<a id="h-julia-package-portability-20261126"></a>
**H-JULIA-PACKAGE-PORTABILITY-20261126.** In the CoD project conversation on 2026-11-26, the project user requested native C access to Julia's Rmath library and a Julia-like array interface so agents could port important packages. The user stated “CoD 的 `x : f64`可以往julia靠，类似的语义特性，其它没冲突的也完全值得向Julia靠。” and requested a repository WIP. [Research r1](design/JuliaPackagePortability.md) compares explicit integer-to-binary64 rounding with the [exact-conversion baseline](https://github.com/complyue/cod-project/blob/d3a2381f9ae1709447659633234a28530ae27e52/design/ScalarUnitsAndIndexTypes.md#explicit-numeric-conversion-and-loss-warnings). The commission covers package porting and evaluating Julia semantics in CoD. Detailed rounding and diagnostics, the package API and implementation are research candidates. [Registry scope](design/DesignAuthorityRegistry.md#d-julia-pkg-research), P-756/P-757, r1342→1343.

## 2026-11-26 — Complete booter readiness at the initialization sites that use it

<a id="k-booter-demand-readiness-20261126"></a>
**K-BOOTER-DEMAND-READINESS-20261126.** The [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) confirmed mainline’s algorithm within the [existing booter phase contract](https://github.com/complyue/cod-project/blob/d3a2381f9ae1709447659633234a28530ae27e52/applications/cod-booter/docs/CompilerPipeline.md#replacing-the-booter-pipeline). The reference compiled 384 successive reserve/write/read triples. The held rewrite exhausted workspace retaining facts beyond the reads they could influence. [Pipeline r14](applications/cod-booter/docs/CompilerPipeline.md#replacing-the-booter-pipeline) limits readiness propagation to the predecessors that can affect a demanded site and retains a single result bit per obligation. The [deferred writer acceptance case](applications/cod-booter/workloads/deferred-initialization.cod), diagnostic order, client check scopes and structural restart prohibition carry forward. D-20-STAGE-CONTROLS continued, registry r1341→1342.

## 2026-11-26 — Assign cross-builds to cfe and distribute the production LLVM toolchain with codup

<a id="h-production-cross-build-20261126"></a>
**H-PRODUCTION-CROSS-BUILD-20261126.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user directed “cross-build 能力只规划给 cfe 入口，cod还是直接宿主运行就行” and “通过 codup 管理CoD安装，不要依赖系统的llvm实例”. The user endorsed static musl for hosted Linux, permitted CRT linkage for packages with C components, and identified SoC development as the primary motivation for a CRT-free path. D-20-COMPILER-MODES and D-20-REPOSITORIES continue the [prior command contract](https://github.com/complyue/cod-project/blob/b7fd6327984f537e2256ca14383bbeedd7e833bd/applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds). [D-20-TARGET-TOOLCHAIN](design/DesignAuthorityRegistry.md#d-20-target-toolchain), P-754/P-755, records the new distribution and target runtime scope at pipeline r13 and roadmap r52. The user raised `brk` as an example for discussion. No allocator interface, system call or board was selected. Registry r1340→1341.

## 2026-11-26 — Give the booter one practical code-generation mode

<a id="h-booter-single-codegen-mode-20261126"></a>
**H-BOOTER-SINGLE-CODEGEN-MODE-20261126.** The project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “booter也只支持一种实用模式就够了，到 cfe 再折腾各种性能选项”. [D-20-COMPILER-MODES](design/DesignAuthorityRegistry.md#d-20-compiler-modes), pipeline r12 and roadmap r51 assign user-selectable performance tuning to `cfe` in `e-wrks/cod`. The [earlier mode contract](#h-cfe-output-directory-20261126) supplies the execution and output behavior and release milestones. The booter keeps its fast default and its independent checking and output operations. C-80/P-746/P-747 continue. Registry r1339→1340.

## 2026-11-26 — Retire the rewritten seed’s inert shared-emission switch

<a id="h-seed-shared-switch-retirement-20261126"></a>
**H-SEED-SHARED-SWITCH-RETIREMENT-20261126.** Asked whether to retain shared emission as a seed requirement if the rewritten translator has no supported input on which the switch changes emission, the project user selected “Drop the inert seed switch. Keep the old implementation in Git.” The [post-rewrite cleanup](#h-post-rewrite-cleanup-20261126) covers the option, its forwarding and its callers. The [old generic relay comparison](#k-booter-one-build-20261124) remains evidence for the earlier implementation. The [build rule](AGENTS.md#build-and-command-execution) continues single-build acceptance for the booter. The decision applies to the rewritten seed.

## 2026-11-26 — Clean up both bootstrap compilers after their rewrites

<a id="h-post-rewrite-cleanup-20261126"></a>
**H-POST-REWRITE-CLEANUP-20261126.** The project user directed the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) to sweep the seed now and the booter after its rewrite, saying “旧的死代码和误导参数、命名再迭代扫描清扫一波，直到自后一次扫描没有残留” and “多余的参数和变量也在清扫之列”. The [retirement contract](design/ImplementationArchitecture.md#retiring-a-replaced-compiler-pipeline) extends the [structural prohibition](#h-compiler-restart-mechanism-20261126) to the command surface and material consumers. D-ARCH-REVIEW r225→226, bootstrap roadmap r49→50 and booter pipeline r10→11 carry the same final-scan condition. The [seed dispatch at the audited baseline](https://github.com/complyue/cod-project/blob/e9cdcaf7e4f4756401d76875f19a7cf6c791843f/cod/main.cc#L213) still exposes the old checker. Registry r1338→1339.

## 2026-11-26 — Scope verification to the compiler and workload that own the result

<a id="k-bootstrap-audit-acceptance-20261126"></a>
**K-BOOTSTRAP-AUDIT-ACCEPTANCE-20261126.** Continued from the [verification scope correction](#k-bootstrap-verification-scope-20261125) and the [terminal diagnostic retirement](https://github.com/complyue/cod-project/commit/42ca800c8f212340dd687e4b871678f78db385b5). Validation reference r488→489 locates historical terminal and deferred diagnostics at the checked seed. The surviving runtime cases retain ordered failure, payload and cleanup observations. D-20-MODULE-GRAPH r96→97 sizes routine acceptance to the affected commands and imports. Broad comparisons remain available for release decisions, including the documented package cutover. P-680/P-681 and P-682/P-683 retain their earlier scope. [Validation reference](tests/cod/language/README.md) · [Module graph](applications/cod-booter/docs/ModuleGraph.md)

## 2026-11-26 — Make pass restarts inexpressible and apply the rule to the booter

<a id="h-compiler-restart-mechanism-20261126"></a>
**H-COMPILER-RESTART-MECHANISM-20261126.** The project user clarified in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “不允许存在程序机制能允许写出导致这种结果的程序。 正确解法是分阶段收集信息再使用信息。 真实技术困难找我讨论。” This clarifies [H-RESTART-FROM-ENTRY](#h-restart-from-entry-20261124), [H-WORKQUEUE-VALUE](#h-workqueue-value-20261125) and the [project-wide replacement decision](#h-compiler-pipeline-replacement-20261125). D-ARCH-REVIEW r224→225 and P-682/P-683 cover the absence of a restart operation in the reachable interfaces. Pipeline r9→10 and roadmap r48→49 apply the rule to the [CoD booter rewrite](applications/cod-booter/docs/CompilerPipeline.md#replacing-the-booter-pipeline), using the [standalone build](https://github.com/complyue/cod-project/commit/b801a6d98744260873745f55d2fa91164eade04a) as the behavior reference. D-20-STAGE-CONTROLS and P-744/P-745 retain the distinct stage-specific goals for client switches. Validation reference r487→488 identifies the legacy scheduler as historical. Registry r1337→1338. Design confirmed the first CoD initialization boundary: extract operations, complete correlated control states, then propagate readiness before checking uses. Mainline owns implementation and cutover evidence.

## 2026-11-26 — Describe the callback from native cleanup

<a id="k-native-cleanup-callback-wording-20261126"></a>
**K-NATIVE-CLEANUP-CALLBACK-WORDING-20261126.** The project user pointed out in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) that “reentry” could misdescribe a further callback. D-ARCH-REVIEW r223→224 names the same-thread call from the test cleanup hook to a separate CoD procedure. Registry r1336→1337 continues P-682/P-683 and the [existing replay behavior](https://github.com/complyue/cod-project/blob/c5d979e9/design/ImplementationArchitecture.md#emission-boundary-for-live-handler-installations). [Callback example](design/ImplementationArchitecture.md#callbacks-from-c-and-protected-path-acquisition)

## 2026-11-26 — Apply the necessity rule to design review’s own gates

<a id="k-design-review-delivery-gates-20261126"></a>
**K-DESIGN-REVIEW-DELIVERY-GATES-20261126.** The project user identified a first-principles failure by design review in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). Review had made proof that handler replay could be omitted a delivery prerequisite. The [case correction](https://github.com/complyue/cod-project/commit/c5d979e9) restored the existing language contract. D-ARCH-REVIEW r222→223 applies the [confirmed necessity rule](#h-compiler-work-necessity-20261125) to the reviewer’s own decisions to hold delivery. Mainline retains its implementation authority. Registry r1335→1336 continues P-682/P-683. [Review responsibilities](design/ImplementationArchitecture.md#design-review-and-delivery-responsibilities)

## 2026-11-26 — Prioritize correct handler replay in the replacement seed

<a id="k-seed-replay-delivery-20261126"></a>
**K-SEED-REPLAY-DELIVERY-20261126.** The project user challenged treating replay omission as a first principle in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). [D-ARCH-REVIEW r221→222](design/ImplementationArchitecture.md#emission-boundary-for-live-handler-installations) continues [H-PERFORM-FAMILY-CAPTURE-20261125](#h-perform-family-capture-20261125). The [previous seed profile](https://github.com/complyue/cod-project/blob/6ca6183a/applications/cod-booter/README.md#trusted-source-and-the-seed) requires proof of an empty set for its supported live handlers. Ordinary family capture and replay remain the delivery target. Registry r1334→1335 continues P-682/P-683.

## 2026-11-26 — Clarify component sourcing for domain-specific databases

<a id="h-dsds-package-sourcing-20261126"></a>
**H-DSDS-PACKAGE-SOURCING-20261126.** The project user clarified that index maintenance, compatibility, concurrency protocols and recovery may be provided as modular CoD packages and outsourced. The [paper](papers/DSDS.md#who-maintains-the-database-components) and [guide](docs/BuildingADatabaseServiceInCoD.md#choose-the-packages-and-their-maintainers) apply that choice to the last-foldout reservation. [D-DBMR-SERVICE-RESEARCH](design/DesignAuthorityRegistry.md#d-dbmr-service-research) continues from the [first-draft scope](https://github.com/complyue/cod-project/blob/fc03ae9d6c83cc09c4eaa1863afe01902d5ba51e/papers/DSDS.md) with distinct deployment and shared-provider costs. The database service's ACID and convergence obligations still follow its domain contract.

## 2026-11-26 — Open the domain-specific database paper and application guide

<a id="h-domain-specific-dbmr-services-20261126"></a>
**H-DOMAIN-SPECIFIC-DBMR-SERVICES-20261126.** In the CoD project discussion the project user commissioned the [DSDS working paper](papers/DSDS.md) and [application guide](docs/BuildingADatabaseServiceInCoD.md). The direction combines direct access to indexed regional graphs with native CoD DML programs and results carried in separate Actor packets. Domain services may combine multiple read-only ontology DBMRs, writable OLTP databases and memory-only regions. The user called for an advanced DBMR interface for server authors responsible for multiprocess transaction coordination, explicit ACID guarantees and the option of eventual consistency. The [research scope](design/DesignAuthorityRegistry.md#d-dbmr-service-research) builds on the [existing immutable-image contract](https://github.com/complyue/cod-project/blob/6ca6183a1dd72405b8eac235acfd3defb5ae1fdf/design/ConstructionAndAcquisition.md#dbmr-modes). The interface, recovery protocol and lower lifetime cost require design and evidence.

## 2026-11-26 — Study the marginal cost of composition and introduce the scoped-effects guide

<a id="k-marginal-cost-and-scoped-effects-guide-20261126"></a>
**K-MARGINAL-COST-AND-SCOPED-EFFECTS-GUIDE-20261126.** The project user requested initial paper and product-guide drafts on the engineering cost and local reasoning required for complex composition. The [paper](papers/MarginalCostOfComposition.md) retains the routing and Store arguments from the [prior revision](https://github.com/complyue/cod-project/blob/a229aca12306e93f83f99f2bc76de0add207151b/papers/ComposingEffectsAndFocusedContexts.md) and applies them to independently adding a detector and a shadow deployment. The [guide](docs/ScopedEffectComposition.md) exercises a shared vocabulary, lexical direct calls, current dynamic invocation and installation-time replay. The audit distinguishes rejection requests from issued commands. [D-DOC-IDIOMATICS](design/DesignAuthorityRegistry.md#d-doc-idiomatics) continues the guide scope. Comparative implementation and observed reader effort are future work.

## 2026-11-26 — Prepare a current symbol’s selected target and owner together

<a id="k-seed-current-symbol-argument-20261126"></a>
**K-SEED-CURRENT-SYMBOL-ARGUMENT-20261126.** [D-ARCH-REVIEW r220→221](design/ImplementationArchitecture.md#preparing-a-current-reference-symbol-argument) applies [D-10-OWNER-SYMBOL](design/DesignAuthorityRegistry.md#d-10-owner-symbol) to [`writeUtf8(@sout, piece)`](https://github.com/complyue/cod-project/blob/b85956e7/cod/library/string.cod#L83). The [entry-owner lookup](https://github.com/complyue/cod-project/blob/b85956e7/cod/lang_llvm.hh#L1130) and [current target load](https://github.com/complyue/cod-project/blob/b85956e7/cod/lang_llvm.hh#L3074) identify different preparation times. The reviewed path takes both words from the descriptor at the operand site. Registry r1330→1331 continues P-682/P-683. Linked acceptance remains with mainline.

## 2026-11-26 — Complete the case result before applying the enclosing return

<a id="k-seed-case-arm-result-20261126"></a>
**K-SEED-CASE-ARM-RESULT-20261126.** [D-ARCH-REVIEW r219→220](design/ImplementationArchitecture.md#returning-a-case-with-a-full-scope-arm) applies [D-07-CASE](design/DesignAuthorityRegistry.md#d-07-case), [D-17-FULL-SCOPE](design/DesignAuthorityRegistry.md#d-17-full-scope) and [D-24-TAIL](design/DesignAuthorityRegistry.md#d-24-tail) to the [two arms in `report_symbol_binder`](https://github.com/complyue/cod-project/blob/6b797d28/applications/cod-booter/src/ModuleGraph.cod#L820). The replacement seed stops at 821:81. The review establishes the order from scope result to case result to enclosing return. Registry r1329→1330 continues P-682/P-683. Implementation and linked evidence remain with mainline.

## 2026-11-26 — Separate omitted-result inference from statement-call use in the seed

<a id="k-seed-omitted-statement-result-20261126"></a>
**K-SEED-OMITTED-STATEMENT-RESULT-20261126.** [D-ARCH-REVIEW r218→219](design/ImplementationArchitecture.md#statement-calls-with-an-omitted-result) applies [D-24-TAIL](design/DesignAuthorityRegistry.md#d-24-tail) at the [intermediate booter call](https://github.com/complyue/cod-project/blob/da068d66/applications/cod-booter/src/ModuleGraph.cod#L988). The [selected function](https://github.com/complyue/cod-project/blob/da068d66/applications/cod-booter/src/DeclarationTypes.cod#L582) supplies the independent evidence for its result. The caller consumes the sealed contract for emission. Registry r1328→1329 continues P-682/P-683. Executable acceptance remains with mainline.

## 2026-11-26 — Name the composition papers and connect domain rules through open sinks

<a id="k-three-composition-papers-20261126"></a>
**K-THREE-COMPOSITION-PAPERS-20261126.** The project user approved the titles and filenames for the [lifetime](papers/ComposingAcrossLifetimes.md), [closed-pipeline](papers/ClosedEventPipelines.md) and [event-effects](papers/EventEffectPropagation.md) papers. The open-network paper develops the approved focus on a domain vocabulary for openly composable business rules. Its [pump alarm and inspection request](papers/EventEffectPropagation.md#from-a-pump-alarm-to-an-inspection-request) extend the [previous monitor study](https://github.com/complyue/cod-project/blob/3cec32a0a23a67f232209bad245cd1d0cc466e94/papers/OpenEventPipelines.md#a-worker-that-can-distinguish-the-histories). [D-EPS-RESEARCH](design/DesignAuthorityRegistry.md#d-eps-research) continues the library research scope at WIP r2. The trace is a paper derivation under an explicit handler selection.

## 2026-11-26 — Complete recursive allocator requirements over recorded owner substitutions

<a id="k-seed-recursive-allocator-20261126"></a>
**K-SEED-RECURSIVE-ALLOCATOR-20261126.** [D-ARCH-REVIEW r217→218](design/ImplementationArchitecture.md#completing-allocator-roles-across-recursive-calls) applies the [replacement pipeline decision](#h-compiler-pipeline-replacement-20261125). At `139acf2b` the [completion pass](https://github.com/complyue/cod-project/blob/139acf2b/cod/lang_seed_bodies.hh#L1542) rejects the recursive `descend` call at 6:9. The reference links the real `reserve_visits` and returns 22. The approved algorithm propagates each required owner role once over recorded substitutions and checks replay closure separately before emission. Registry r1326→1327 continues P-682/P-683. Implementation acceptance is pending.

## 2026-11-26 — Reframe the ownership paper around domain lifetimes

<a id="k-ownership-paper-domain-lifetimes-20261126"></a>
**K-OWNERSHIP-PAPER-DOMAIN-LIFETIMES-20261126.** The project user asked the [ownership paper](papers/CoarseGrainedOwnership.md) to foreground multi-owner tracking, domain choices about which allocations retire together, and the long-running process that can have 1 GB free yet fail a 10 MB request. The [previous paper](https://github.com/complyue/cod-project/blob/ce57f249170c181d3e666da706af5127d2260efe/papers/CoarseGrainedOwnership.md) supplies the revision-replacement example. The revision adds a [three-owner fare helper](papers/CoarseGrainedOwnership.md#one-call-can-use-three-owners-and-return-a-view-into-one), a [fixed-window allocation comparison](papers/CoarseGrainedOwnership.md#how-1-gb-free-can-fail-a-10-mb-request), the [audit result's owner](papers/CoarseGrainedOwnership.md#which-owner-should-carry-an-audit-result) and a workload evaluation plan under the [existing regional contract](design/RegionalOwnership.md#intrinsic-region-ownership-versus-intra-region-library-policy). The project user clarified that a `ref` can never outlive its actual owner and that the compiler must reject such an escape. Both placements in the bounded-allocator example release only whole regions. Production memory and latency remain empirical questions.

## 2026-11-26 — Explain Compile on Demand alongside Compile for Ever

<a id="k-cod-cli-name-20261126"></a>
**K-COD-CLI-NAME-20261126.** The project user described the CLI meaning as “命令行上 cod 的 Compile on Demand 语义” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). Registry r1325→1326 continues [D-20-COMPILER-MODES](design/DesignAuthorityRegistry.md#d-20-compiler-modes). The [compiler mode table](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds) pairs the two names with the run and save behavior in the [previous contract](https://github.com/complyue/cod-project/blob/b3e97f50b15a39f107db3c25b4a689a326ac6ac9/applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds).

## 2026-11-26 — Plan Event Processing Systems as one CoD library

<a id="h-eps-library-direction-20261126"></a>
**H-EPS-LIBRARY-DIRECTION-20261126.** In the CoD project conversation on 2026-11-26, the project user directed the two event-processing papers to build on the conclusions of the retention working paper, investigate closed event pipelines and open event networks, and plan a joint CoD library repository named `e-wrks/eps.cod`, “Event Processing Systems” (English rendering of the request). The [retention paper at the baseline](https://github.com/complyue/cod-project/blob/b3e97f50b15a39f107db3c25b4a689a326ac6ac9/papers/WhatShouldAComposedOperationRetain.md) and [previous pairing](https://github.com/complyue/cod-project/blob/b3e97f50b15a39f107db3c25b4a689a326ac6ac9/papers/README.md#read-the-two-pipeline-papers-together) identify the inputs. The [closed-drive](papers/ClosedEventPipelines.md#where-a-closed-event-pipeline-ends) and [open-network](papers/EventEffectPropagation.md#connecting-two-closed-drives) studies now share a delayed-audit case and a [WIP library plan](design/EventProcessingSystems.md). [D-EPS-RESEARCH](design/DesignAuthorityRegistry.md#d-eps-research) records the direction separately from approval of public interfaces or executable support.

## 2026-11-26 — Name the `cfe` entry point throughout the compiler design

<a id="k-cfe-entry-point-20261126"></a>
**K-CFE-ENTRY-POINT-20261126.** Registry r1323→1324 continues [D-20-COMPILER-MODES](design/DesignAuthorityRegistry.md#d-20-compiler-modes) from the [confirmed `cfe` contract](https://github.com/complyue/cod-project/blob/b2fbf663/applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds). The project rules, charter, architecture and roadmap now use the same production command name. The [pipeline](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds) retains the output, acceptance and release terms of [H-CFE-OUTPUT-DIRECTORY-20261126](#h-cfe-output-directory-20261126).

## 2026-11-26 — Open a paper and developer guide on retaining composed operations

<a id="k-composed-operation-retention-20261126"></a>
**K-COMPOSED-OPERATION-RETENTION-20261126.** The project user requested an English working paper and a companion product guide applying its best practices to domain development. The [opening paper](papers/ComposingAcrossLifetimes.md) develops the mutation, lifetime and control cases from the [existing parser study](https://github.com/complyue/cod-project/blob/358d8e2568ec9cf55799269faf8d73c5c011bcf8/design/ParserCombinatorLibSupport.md#lexical-composition-or-an-independently-owned-plan). The [developer guide](docs/BuildingReusableDomainOperations.md) applies them to adding a terminal, replaying a source and retaining a rule. [D-DOC-IDIOMATICS](design/DesignAuthorityRegistry.md#d-doc-idiomatics) continues the [previous guide](https://github.com/complyue/cod-project/blob/358d8e2568ec9cf55799269faf8d73c5c011bcf8/docs/IdiomaticCoD.md#name-reusable-pipeline-connections) through r16. The commission covers the investigation and companion documentation. Language extensions, executable acceptance and productivity measurements remain open.

## 2026-11-26 — Put review routing first in the project rules

<a id="k-review-routing-20261126"></a>
**K-REVIEW-ROUTING-20261126.** D-GOV r1321→1322 continues the [confirmed delegation](#h-mainline-design-review-boundary-20261125). The user reiterated “你只负责影响足够重大的事项上把关” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). Compared with [the previous guide](https://github.com/complyue/cod-project/blob/813eab5d/AGENTS.md), [the decision table](AGENTS.md#design-discussion-and-development-mainline) gives each review trigger and its owner once. The [registry](design/DesignAuthorityRegistry.md#recording-a-design-revision) now carries the detailed design-record instructions. Existing approval thresholds, phase prohibitions, verification and operational permissions continue.

## 2026-11-26 — Compare lexical connections and independently owned parser plans

<a id="k-parser-owned-plan-study-20261126"></a>
**K-PARSER-OWNED-PLAN-STUDY-20261126.** The project user requested a feasibility comparison and identified ADT functors in an independent owner as the promising storage direction. The [r4 WIP](design/ParserCombinatorLibSupport.md#lexical-composition-or-an-independently-owned-plan) extends the [r3 storage alternatives](https://github.com/complyue/cod-project/blob/cac10f6204e5d2665a914088e93a533ca43c8cf2/design/ParserCombinatorLibSupport.md#reusable-rules-and-the-scope-of-language-support) with a lifetime counterexample, a mutation witness, a construction-cost comparison and an acceptance workload. The [research scope](design/DesignAuthorityRegistry.md#d-parser-combinator-research) distinguishes the candidate representation from the existing owner and synchronous invocation contracts.

## 2026-11-26 — Compose fare and event stages before attaching endpoints

<a id="k-papers-open-operator-composition-20261126"></a>
**K-PAPERS-OPEN-OPERATOR-COMPOSITION-20261126.** The project user proposed studying the language mechanisms explored for `pcl.cod` in both pipeline drafts. The [fare](papers/ClosedEventPipelines.md#an-itinerary-plan-with-both-ends-open) and [event](papers/EventEffectPropagation.md#one-ingress-plan-for-live-and-replay) examples each compose a reusable middle and drive it with two different endpoint pairs. They extend the [previous paper comparison](https://github.com/complyue/cod-project/blob/7dc810e04133d74e06f485daec478e94a521eb11/papers/README.md#read-the-two-pipeline-papers-together) with derived observations under the [existing lexical connection contract](design/ProducersAndTransformers.md#lexical-variables-for-open-pipelines). The shared mechanisms and domain-specific feedback, completion and transport laws remain distinguishable in the [comparison](papers/README.md#read-the-two-pipeline-papers-together).

## 2026-11-26 — Forward borrowed CLI inputs through the catalog callback

<a id="k-seed-cli-input-loans-20261126"></a>
**K-SEED-CLI-INPUT-LOANS-20261126.** [D-ARCH-REVIEW r216→217](design/ImplementationArchitecture.md#borrowing-cli-entry-inputs-in-a-catalog-callback) continues P-682/P-683. At `7dc810e0`, [`compile_cli`](https://github.com/complyue/cod-project/blob/7dc810e0/applications/cod-booter/src/__main__.cod#L7) forwards borrowed input, output text and a writable diagnostic destination through its synchronous catalog callback. The [existing entry adapter](https://github.com/complyue/cod-project/blob/7dc810e0/cod/native/provider_call.h#L36) establishes the bounded UTF-8 snapshot. This review clears use of the existing place and owner records through the callback and ordinary forward. Full CLI translation remains an implementation milestone.

## 2026-11-26 — Identify the CoD parser as an application of the pcl.cod HLVM

<a id="h-pcl-hlvm-parser-application-20261126"></a>
**H-PCL-HLVM-PARSER-APPLICATION-20261126.** The project user clarified “pcl.cod是HLVM，**CoD parser是这个hlvm的一个app**”. This corrects the [earlier paper heading](https://github.com/complyue/cod-project/blob/2b2dc9d5d5c9ee61cbb398d3dbe562d978ea44c4/papers/AlgebraicEffectsIsAllYouNeed.md#the-production-cod-parser-as-an-hlvm) and its attribution in P-748. The [production bundle](design/DesignAuthorityRegistry.md#d-20-parser-bundle) supplies the parsing HLVM as `pcl.cod` and the CoD parser application’s DBMR grammar program as `cod.grammar`. The [revised paper example](papers/AlgebraicEffectsIsAllYouNeed.md#the-cod-parser-as-an-application-of-the-parsing-hlvm), WIP r3 and roadmap r48 carry that distinction through to the installed-toolchain workload.

## 2026-11-26 — Name the optimizing compiler CLI and its output directory

<a id="h-cfe-output-directory-20261126"></a>
**H-CFE-OUTPUT-DIRECTORY-20261126.** In the CoD project design conversation the project user named the optimizing CLI `cfe` (Compile For Ever) and specified “默认往 ~/.local/bin/ 下保存，目标目录可以通过 ~/.cod/config.yaml 修改。” [Compiler pipeline r8→9](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds) and [roadmap r46→47](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) apply this to executables generated by `e-wrks/cod`. The [prior mode contract](https://github.com/complyue/cod-project/blob/b18bbc063d5a4bf36ab6012e3e64e4aac693a438/applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds) established the compile-and-save behavior and release milestones. Registry r1317→1318, [C-80/P-746/P-747](design/DesignAuthorityRegistry.md#d-20-compiler-modes).

## 2026-11-26 — Use ordered array views for a single-owner catalog callback

<a id="k-seed-pipeline-ordered-catalog-loans-20261126"></a>
**K-SEED-PIPELINE-ORDERED-CATALOG-LOANS-20261126.** [D-ARCH-REVIEW r215→216](design/ImplementationArchitecture.md#ordered-array-loans-from-one-native-owner) continues P-682/P-683 and the [r214 synchronous callback review](#k-seed-pipeline-path-key-callback-20261125). At `33362b65` the [catalog declaration](https://github.com/complyue/cod-project/blob/33362b65/applications/cod-booter/src/NativeCatalog.cod#L7) offers two nominal arrays under one owner. The [existing LLVM bridge](https://github.com/complyue/cod-project/blob/33362b65/cod/lang_llvm.hh#L1272) already passes each typed array view and their common owner. The replacement producer may use that ordered interface for direct and forwarded synchronous calls. Execution coverage is reported separately.

## 2026-11-26 — Ship the production parser library and its DBMR grammar

<a id="h-production-parser-bundle-20261126"></a>
**H-PRODUCTION-PARSER-BUNDLE-20261126.** In the CoD project design conversation the user specified that `e-wrks/cod` will ship bundled `pcl.cod`, that its parser uses DBMR `cod.grammar` shipped with the toolchain, and that the paper should present it as a typical HLVM use case. The [production parser delivery contract](design/DesignAuthorityRegistry.md#d-20-parser-bundle) extends the [r1 research baseline](https://github.com/complyue/cod-project/blob/b18bbc063d5a4bf36ab6012e3e64e4aac693a438/design/ParserCombinatorLibSupport.md) at r2 and the production roadmap at r46. The [paper case](papers/AlgebraicEffectsIsAllYouNeed.md#the-production-cod-parser-as-an-hlvm) gives the planned workload. Parser interface research and the [later booter backport decision](#h-production-parser-direction-20261124) retain their prior scope.

## 2026-11-26 — Reuse lexical native cleanup in the replacement seed

<a id="k-seed-pipeline-lexical-native-handles-20261126"></a>
**K-SEED-PIPELINE-LEXICAL-NATIVE-HANDLES-20261126.** [D-ARCH-REVIEW r214→215](design/ImplementationArchitecture.md#direct-native-handle-acquisition-in-the-replacement-seed) continues P-682/P-683 under the [replacement pipeline decision](#h-compiler-pipeline-replacement-20261125). At `3fa5ab0c` the [real root-opening prefix](https://github.com/complyue/cod-project/blob/3fa5ab0c/applications/cod-booter/src/ModuleGraph.cod#L1617) uses both owning construction and an attached path. The [existing LLVM release registration](https://github.com/complyue/cod-project/blob/3fa5ab0c/cod/lang_llvm.hh#L2567) consumes success at the owning place. The review authorizes new producers for those direct lexical operations and preserves the existing native failure ABI. Replacement execution remains to be verified.

## 2026-11-25 — Separate supplier and callback extraction at the native path loan

<a id="k-seed-pipeline-path-key-callback-20261125"></a>
**K-SEED-PIPELINE-PATH-KEY-CALLBACK-20261125.** [D-ARCH-REVIEW r213→214](design/ImplementationArchitecture.md#extracting-the-path-key-callback) continues P-682/P-683 under the [replacement pipeline decision](#h-compiler-pipeline-replacement-20261125). At `a0d304e3` real [`fill_path`](https://github.com/complyue/cod-project/blob/a0d304e3/applications/cod-booter/src/ModuleIndex.cod#L138) supplies a literal to [`path_key`](https://github.com/complyue/cod-project/blob/a0d304e3/applications/cod-booter/src/NativeSource.cod#L26). The [existing bridge](https://github.com/complyue/cod-project/blob/a0d304e3/cod/lang_llvm.hh#L1316) consumes a descriptor and distinguishes protected completion, value feedback and provider failure. The review clears separate literal extraction and subsequent capture binding against that ABI. Executable acceptance remains a mainline milestone.

## 2026-11-25 — Make fast compile-and-run the default and schedule optimizing builds

<a id="h-default-run-and-optimized-build-20261125"></a>
**H-DEFAULT-RUN-AND-OPTIMIZED-BUILD-20261125.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user confirmed “cod 默认模式是 `go run` 语义，不产出binary” and clarified “seed 默认是产出 cod-booter binary 没毛病”. Seed, booter and the first production release target Go-like fast compilation with moderate runtime performance. The user specified “高质量机器码的优化编译模式，默认是产出binary，而不是立即运行，规划在 e-wrks/cod 首版实验，2.0正式提供。” [D-20-COMPILER-MODES](design/DesignAuthorityRegistry.md#d-20-compiler-modes), C-80/P-746/P-747, adds the [charter principle](design/RegionalSemanticPlatformCharter.md#核心原则) and [mode acceptance](applications/cod-booter/docs/CompilerPipeline.md#default-execution-and-optimized-builds). Against [the prior pipeline](https://github.com/complyue/cod-project/blob/3e10ecec/applications/cod-booter/docs/CompilerPipeline.md), pipeline r7→8, roadmap r44→45, D-ARCH-REVIEW r212→213 and D-21 r38→39 retain independent stage selection and native execution semantics.

## 2026-11-25 — Accept completed seed results at the existing LLVM interface

<a id="k-replacement-seed-llvm-handoff-20261125"></a>
**K-REPLACEMENT-SEED-LLVM-HANDOFF-20261125.** D-ARCH-REVIEW r211→212 continues P-682/P-683 under the [project user’s pipeline decision](#h-compiler-pipeline-replacement-20261125). At `d46a2bbf` the [LLVM emitter](https://github.com/complyue/cod-project/blob/d46a2bbf44e2d4cf153bd4064eb5d2fecdbb6333/cod/lang_llvm.hh#L5533) and [C provider generator](https://github.com/complyue/cod-project/blob/d46a2bbf44e2d4cf153bd4064eb5d2fecdbb6333/cod/lang_c_provider.hh#L178) take a const program. The [first adapter](design/ImplementationArchitecture.md#replacement-seed-pipeline) may supply that shape from the new producers, with the matching parsed source alive through emission. The first two-reserve workload and cutover gate continue. This approves the interface for implementation.

## 2026-11-25 — Replace the seed pipeline and make compiler stages selectable

<a id="h-compiler-pipeline-replacement-20261125"></a>
**H-COMPILER-PIPELINE-REPLACEMENT-20261125.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user made restarting a project-wide engineering prohibition, explicitly stating “即使未来的 e-wrks/cod 也不允许走这条路，确实遇到技术困难跟我讨论。” They authorized “按你的建议开新的编译管线开发”. They requested independent parse, type, lexical-access, ownership and emission controls where feasible in the seed and booter and required them in production for LSP/IDE and build users. [D-ARCH-REVIEW r210→211](design/ImplementationArchitecture.md#replacement-seed-pipeline) extends the [phase](#h-completed-callee-behavior-20261125) and [workqueue](#h-workqueue-value-20261125) decisions. The [legacy scheduler at the decision baseline](https://github.com/complyue/cod-project/blob/d46a2bbf44e2d4cf153bd4064eb5d2fecdbb6333/cod/lang_checked.hh#L24536) remains a reference. [D-20-STAGE-CONTROLS](design/DesignAuthorityRegistry.md#d-20-stage-controls), pipeline r6→7 and roadmap r43→44, records P-744/P-745 and stage-specific acceptance. The [trusted seed’s delivery and timing gates](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance) continue. A concrete obstacle to the restart prohibition returns to the user for decision.

## 2026-11-25 — Record delegated PR authorization and commission the ASCII scanner path

<a id="h-pr-route-and-ascii-delivery-20261125"></a>
**H-PR-ROUTE-AND-ASCII-DELIVERY-20261125.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user stated “PR#1 我授权另一智能体合入的” and “原则上 PR 是正规途径”, and requested the complete ASCII vocabulary, both compiler paths and the scanner migration. [PR #1](https://github.com/complyue/cod-project/pull/1) merged at [9a95bc01](https://github.com/complyue/cod-project/commit/9a95bc01c72380d67fdf10868d9556d32e1eb160). Its design propositions retain their original [named constant](#h-named-method-constants-20261125), [enum conversion](#h-enum-backing-coercion-20261125) and [ASCII vocabulary](#k-ascii-vocabulary-20261125) records. This confirmation covers the delegated merge and the stated delivery. [Architecture r209→210](design/ImplementationArchitecture.md#pull-request-authorization-and-design-evidence) clarifies the publication route. [ASCII acceptance](design/ImplementationArchitecture.md#delivering-ascii-to-the-booter-scanner) connects D-15-DEFAULT-EXPORTS r74→84 coverage to [P-692/P-693](design/DesignAuthorityRegistry.md#d-15-default-exports) and D-07-ENUM r8. Registry r1309→1310. Compiler delivery is tracked by mainline.

## 2026-11-25 — Show a full ASCII vocabulary with a small default selection

<a id="k-ascii-vocabulary-20261125"></a>
**K-ASCII-VOCABULARY-20261125.** The project user suggested uppercase constant names and “通过 `default { ... }` export 常用值” in the CoD design conversation. [Idiomatic CoD r14→15](docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner) applies this style to the [earlier scanner example](https://github.com/complyue/cod-project/blob/17779f20c16cc7ef016265e3f833d62113d7d65c/docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner). [Module worlds r83→84](design/ModuleWorlds.md#default-and-complete-public-selections) illustrates selector expansion with an enum member subset. [Enum r7→8](design/TypedEnums.md#names-and-case-matching) carries forward lexical lookup and code coercion. Registry r1308→1309. Compiler and library delivery follow the development mainline.

## 2026-11-25 — Name method constants and use enum backing codes

<a id="h-named-method-constants-20261125"></a>
**H-NAMED-METHOD-CONSTANTS-20261125.** In the CoD project design conversation the project user confirmed “我喜欢你建议的 = 前缀方案，就这样定吧。” [D-10 r269→270](design/SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch) closes the [named-constant roadmap item](https://github.com/complyue/cod-project/blob/9af5fcaf91bfe426f6ca9d0a23619048814b58d6/docs/DesignTopics.md#按实际用例重启的小议题). A method head resolves `=name` outside its own formals and matches the typed singleton.

<a id="h-enum-backing-coercion-20261125"></a>
**H-ENUM-BACKING-COERCION-20261125.** The project user proposed an ASCII enum for the lexer and clarified “typed enum 值应该能被自动 coarse 成底层类型值用” and “反过来就必须要求 `10:ASCII` 这种写法才行了”. [Typed enums r6→7](design/TypedEnums.md#using-an-enum-as-its-backing-value) supersedes the outbound conversion restriction and the source integer assertion restriction in the [r6 contract](https://github.com/complyue/cod-project/blob/9af5fcaf91bfe426f6ca9d0a23619048814b58d6/design/TypedEnums.md#declaration-and-value-identity). A declared backing-typed value position can consume the enum's code. An explicit `code:E` checks incoming member codes under the existing assertion failure rules. [Module graph r96](applications/cod-booter/docs/ModuleGraph.md#checking-the-native-package-probe) applies the outbound rule to a `u64` bound. [Idiomatic CoD r14](docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner) applies both decisions to the booter scanner. Registry r1307→1308, [P-658/P-659](design/DesignAuthorityRegistry.md#d-07-enum) and [P-740/P-741](design/DesignAuthorityRegistry.md#d-10-method-groups). Compiler delivery requires implementation evidence.

## 2026-11-25 — Investigate parser combinator library support

<a id="h-parser-combinator-wip-20261125"></a>
**H-PARSER-COMBINATOR-WIP-20261125.** In the CoD project design conversation the project user required “grammar和parse结果肯定要分不同owner”, expressed the preference “接口类型包含多个here-callback实现完整Megaparsec语义”, and requested a WIP. [Research r1](design/ParserCombinatorLibSupport.md) records that invariant and preference under the [production parser direction](#h-production-parser-direction-20261124). The four-arm contract, bound procedure value, open graph representation and comparator details are proposals. Registry r1306→1307, [P-742/P-743](design/DesignAuthorityRegistry.md#d-parser-combinator-research). The prior [roadmap §7](https://github.com/complyue/cod-project/blob/4a4df1338a893433d2e95512511495824f169380/docs/DesignTopics.md#production-parser-combinators) remains the backport boundary.

## 2026-11-25 — Define callable methods by types and constant values

<a id="h-grouped-method-dispatch-20261125"></a>
**H-GROUPED-METHOD-DISPATCH-20261125.** In the CoD project design conversation the project user asked for “常量参数值参与method dispatch” and “公共头声明更泛类型的参数，内部方法声明更具体类型的参数”. The user confirmed the `of` method list, `=>` arms and the requirement to brace the whole method body for a direct-level parameterized trailing callback, and requested support in the planned zero-configuration formatter. [D-10 r268→269](design/SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch) and [D-07 r223→224](design/DomainLanguageDesign.md#concrete-data-dispatch-through-abstract-parameters) broaden the [grouped installation](#h-grouped-local-proc-install-20261124) documented at `9f9ef18bfebe025d799e0b3c35a097ba45cd58cd:design/SymbolicDynamicScope.md`. The new method arrow supersedes that installation’s `->` arm spelling. Open named families can specialize actual values and types across authorized module extensions. Each installed family must cover its symbol. [P-740/P-741 and the confirmed boundary](design/DesignAuthorityRegistry.md#d-10-method-groups) also cover enum r6, D-15r83 and D-21r38. The idiomatics guideline r5, guide r13 and syntax showcase document the [method-arm boundaries](design/SymbolicDynamicScope.md#method-arm-boundaries). Compiler and formatter delivery require separate evidence.

## 2026-11-25 — Qualify protected-name diagnostics by compiler stage

<a id="k-seed-protected-binder-stage-20261125"></a>
**K-SEED-PROTECTED-BINDER-STAGE-20261125.** The [architecture review](design/ImplementationArchitecture.md#protecting-a-declared-symbol-name) continues D-ARCH-REVIEW r208→209. The [r103 lexical check](#k-runtime-symbol-declaration-check-20261124) is part of the checked seed baseline at `a0922741:cod/lang_checked.hh`. The [trusted seed](#h-trusted-seed-translation-20261125) may omit this diagnostic. The booter enforces the [source rule](#h-runtime-symbol-identity-20261124) at the conflicting declaration.

## 2026-11-25 — Require live handler evidence at installation

<a id="k-seed-scoped-activation-emission-20261125"></a>
**K-SEED-SCOPED-ACTIVATION-EMISSION-20261125.** The [architecture review](design/ImplementationArchitecture.md#emission-boundary-for-live-handler-installations) continues D-ARCH-REVIEW r207→208 and applies the [r188 omission rule](design/ImplementationArchitecture.md#live-handlers-without-scalar-snapshot-fields). At held revision `3884f7fa:cod/lang_checked.hh`, the perform-only finalization misses an emitted installation in the owning loader's current-context twin. The held seed must establish its representation at the installation before emitting output. Native disposal permissions remain a separate design question.

## 2026-11-25 — Complete an ordered byte transfer before its callers

<a id="k-seed-overlapping-byte-transfer-20261125"></a>
**K-SEED-OVERLAPPING-BYTE-TRANSFER-20261125.** The [architecture review](design/ImplementationArchitecture.md#completing-an-overlapping-byte-transfer) continues D-ARCH-REVIEW r206→207. At `4d0158a7:applications/cod-booter/src/LoadPackageInputs.cod`, `copy_path` has a writable output loan and no ordinary callees. Its callers can consume a completed result and exit contract while the emitted loop retains the source-visible order of reads and writes to overlapping storage.

## 2026-11-25 — Bind product fill fields in writable array growth

<a id="k-seed-array-growth-product-fill-20261125"></a>
**K-SEED-ARRAY-GROWTH-PRODUCT-FILL-20261125.** The [architecture review](design/ImplementationArchitecture.md#product-fills-in-writable-array-growth) continues D-ARCH-REVIEW r205→206. At `f7be2b4b:applications/cod-booter/src/ModuleGraph.cod`, five reserve procedures use explicit product fills. The [writable growth schedule](#k-seed-array-growth-contract-20261125) can consume their ordinary checked field operations in written order, including each call to a completed literal function.

## 2026-11-25 — Complete writable array growth before its callers

<a id="k-seed-array-growth-contract-20261125"></a>
**K-SEED-ARRAY-GROWTH-CONTRACT-20261125.** The [architecture review](design/ImplementationArchitecture.md#completing-the-writable-array-growth-procedures) continues D-ARCH-REVIEW r204→205 under the [completed-callee rule](#h-completed-callee-behavior-20261125). At `78adf259:applications/cod-booter/src/ModuleIndex.cod`, two reserve procedures allocate in the same owner through distinct caller slots. The reviewed seed schedule preserves discovery of writes, settled slot outcomes and the allocator role consumed by LLVM. The batch publishes each procedure's contract only after its observations are complete.

## 2026-11-25 — Apply the completed module key comparison to a visit lookup

<a id="k-seed-module-visit-contract-20261125"></a>
**K-SEED-MODULE-VISIT-CONTRACT-20261125.** The [architecture review](design/ImplementationArchitecture.md#applying-the-completed-key-comparison-in-a-visit-lookup) continues D-ARCH-REVIEW r203→204 under the [completed-callee rule](#h-completed-callee-behavior-20261125) and [case reception contract](#h-case-contract-20261119). At `2eeadd72:applications/cod-booter/src/ModuleIndex.cod`, the four nonempty visit alternatives supply independent endpoints to the same completed comparator. The existing second batch can establish the lookup's contract while preserving the runtime index result, actual loans and failure origins.

## 2026-11-25 — Complete the seed contract for the module key comparison

<a id="k-seed-module-key-range-20261125"></a>
**K-SEED-MODULE-KEY-RANGE-20261125.** The [architecture review](design/ImplementationArchitecture.md#completing-the-module-key-comparison) continues D-ARCH-REVIEW r202→203 under the [completed-callee rule](#h-completed-callee-behavior-20261125). At `0b01b2d1:applications/cod-booter/src/ModuleIndex.cod`, the read-only range in `same_key` has two independent endpoints and two explicit intrinsic length reads. The seed may seal its contract from the existing read-only application batch before `find_visit` needs it. Argument preparation and separate final validation retain their ordinary roles.

## 2026-11-25 — Skip the seed source-form walk when the relevant syntax is absent

<a id="k-seed-source-form-walk-20261125"></a>
**K-SEED-SOURCE-FORM-WALK-20261125.** The [architecture review](design/ImplementationArchitecture.md#skipping-the-seed-source-form-walk) continues D-ARCH-REVIEW r201→202 under the [material-consumer rule](#h-compiler-work-necessity-20261125). At `b1af7314:cod/lang_syntax.hh` the ordinary parser constructs every syntax form that can affect the pre-registration visitor. A completed file can carry that fact to the checker. Marked modules and inputs of unknown provenance keep the existing visitor and diagnostic order.

## 2026-11-25 — Prove the lexer's read-only range once per phase

<a id="k-seed-readonly-range-phase-20261125"></a>
**K-SEED-READONLY-RANGE-PHASE-20261125.** The [architecture review](design/ImplementationArchitecture.md#proving-the-read-only-byte-comparison-range) continues D-ARCH-REVIEW r200→201 under the [phase rule](#h-completed-callee-behavior-20261125) and [material-consumer rule](#h-compiler-work-necessity-20261125). In `e52519a7:applications/cod-booter/src/Lexer.cod`, the bound byte comparison and its completed callees establish that the next iteration inherits the same selections. The seed may omit discovery and loop settlement for that profile while preserving the runtime loop, cleanup and independent final comparison.

## 2026-11-25 — Omit target discovery in completed direct lexer wrappers

<a id="k-seed-wrapper-target-preparation-20261125"></a>
**K-SEED-WRAPPER-TARGET-PREPARATION-20261125.** The [architecture review](design/ImplementationArchitecture.md#target-preparation-in-direct-lexer-wrappers) continues D-ARCH-REVIEW r199→200 under the [material-consumer rule](#h-compiler-work-necessity-20261125). At `6827e2b2:cod/lang_checked.hh`, the proving frame already supplies all places in the admitted read-only wrapper. The reviewed change keeps separate proving visits for application and final comparison, with ordinary validation on a late written-name collision.

## 2026-11-25 — Apply the completed lexer predicate to two loans and a string literal

<a id="k-seed-lexer-literal-wrappers-20261125"></a>
**K-SEED-LEXER-LITERAL-WRAPPERS-20261125.** The [architecture review](design/ImplementationArchitecture.md#forwarding-two-loans-and-a-string-literal) continues D-ARCH-REVIEW r198→199 under the [completed-callee decision](#h-completed-callee-behavior-20261125) and [UTF-8 text rule](#h-utf8-native-text-20261121). At `9f714ef8:applications/cod-booter/src/Lexer.cod`, direct wrappers can consume the finished `word_is` contract in the existing second batch. Ordinary preparation supplies the literal's owner, and an entered callee's failure retains its runtime origin.

## 2026-11-25 — Complete read-only lexer calls from settled exits

<a id="k-seed-readonly-lexer-contracts-20261125"></a>
**K-SEED-READONLY-LEXER-CONTRACTS-20261125.** The [architecture review](design/ImplementationArchitecture.md#completing-read-only-calls-in-the-lexer) continues D-ARCH-REVIEW r197→198 under the [completed-callee decision](#h-completed-callee-behavior-20261125). At `dd92531f:applications/cod-booter/src/Lexer.cod`, `word_is` calls `SourceAccess.len` before its byte comparison loop. The reviewed seed producer completes the length leaf first and seals the caller from settled exits expressed in its formal arguments. The existing loan, bounds, loop and final validation rules continue.

## 2026-11-25 — Apply a completed predicate in the next group of scalar callers

<a id="k-seed-second-scalar-application-20261125"></a>
**K-SEED-SECOND-SCALAR-APPLICATION-20261125.** The [architecture review](design/ImplementationArchitecture.md#applying-a-completed-scalar-predicate) continues D-ARCH-REVIEW r196→197 under the [completed-callee decision](#h-completed-callee-behavior-20261125). In `b38eb47f:applications/cod-booter/src/NativeCommands.cod`, the seed can complete `local_binding_fact` against literal leaves before applying it at the guarded call in `initialization_access`. The two batches consume only earlier completed inputs and retain ordinary validation and output publication.

## 2026-11-25 — Omit target discovery for already bound independent scalar frames

<a id="k-seed-independent-scalar-proving-20261125"></a>
**K-SEED-INDEPENDENT-SCALAR-PROVING-20261125.** The [architecture review](design/ImplementationArchitecture.md#target-discovery-in-independent-scalar-frames) continues D-ARCH-REVIEW r195→196 under [H-COMPILER-WORK-NECESSITY](#h-compiler-work-necessity-20261125) and the [completed-callee rule](#h-completed-callee-behavior-20261125). At `d4f2dae8:cod/lang_checked.hh`, eligible scalar producers and applications still enter discovery before proving. The reviewed schedule retains the proving frame, arithmetic exits and established diagnostics. Its eligibility depends on all places being available in that frame and all selected callee inputs already being complete.

## 2026-11-25 — Review local handler selection during recursive visits

<a id="k-seed-recursive-local-selection-20261125"></a>
**K-SEED-RECURSIVE-LOCAL-SELECTION-20261125.** The [implementation review](design/ImplementationArchitecture.md#recursive-local-handler-selection) continues D-ARCH-REVIEW r194→195 under the user's [family capture decision](#h-perform-family-capture-20261125). The private seed at `e8f005c6:cod/lang_checked.hh` resolves the local handler before arguments. The replay collector can consume that selection from the completed call frame. Each nested activation needs its own complete empty inventory on the late route, including when its parent supplies a symbol. The production graph's native disposal obligation remains open.

## 2026-11-25 — Review replay through the package resolver’s result writes and root scan

<a id="k-seed-package-resolver-replay-20261125"></a>
**K-SEED-PACKAGE-RESOLVER-REPLAY-20261125.** The [implementation review](design/ImplementationArchitecture.md#collecting-replay-requirements-in-the-package-resolver) continues D-ARCH-REVIEW r193→194. The trusted seed can consume the checked lexical constructor and indexed destination for each finite `PackageResolution` result and inspect the installation-free numeric procedure loop once. The actual workload at `a960f7f3:applications/cod-booter/src/PackageModuleResolution.cod` distinguishes search failure, missing, one match and two matches in source order.

## 2026-11-25 — Review empty replay through the real module key lookup

<a id="k-seed-key-lookup-replay-20261125"></a>
**K-SEED-KEY-LOOKUP-REPLAY-20261125.** The [implementation review](design/ImplementationArchitecture.md#collecting-replay-requirements-in-the-key-lookup) continues D-ARCH-REVIEW r192→193. After removing the scheduling shortcut, the private seed at `0bf6c449:cod/lang_checked.hh` stops at the numeric range in `ModuleIndex.find_visit`. The reviewed collector can use the already checked half-open ranges, indexed tag and by-value key fields to establish an empty inventory. The native-disposal case continues to require independent replay evidence.

## 2026-11-25 — Bound native reentry at the protected relative-path acquisition

<a id="k-seed-protected-path-reentry-20261125"></a>
**K-SEED-PROTECTED-PATH-REENTRY-20261125.** The [implementation review](design/ImplementationArchitecture.md#native-reentry-and-protected-path-acquisition) continues D-ARCH-REVIEW r191→192 under the [family replay](#h-perform-family-capture-20261125) and [native callable-kind](#h-native-callable-kind-20261123) decisions. A linked native procedure with only a scalar operand reenters a CoD procedure and reads its dynamic binding. The next bounded collector scope follows the real `PathOwner` acquisition, relative path preparation and cleanup in `a9804e30:applications/cod-booter/src/ModuleGraph.cod`. Opaque native reentry retains a located translation limit.

## 2026-11-25 — Apply native function permissions to the exported source revision loan

<a id="k-seed-source-use-function-20261125"></a>
**K-SEED-SOURCE-USE-FUNCTION-20261125.** The [implementation review](design/ImplementationArchitecture.md#callable-kind-of-the-exported-source-revision-loan) continues D-ARCH-REVIEW r190→191 under [H-NATIVE-CALLABLE-KIND](#h-native-callable-kind-20261123). The private path-key review is followed by the exported `source_use` declaration at `88784b85:applications/cod-booter/src/NativeSourceUse.cod`. Its provider meets the documented native function contract. The approved correction makes this operation directly usable from an eligible CoD function while preserving its callback, borrowed revision and C interface.

## 2026-11-25 — Review the native path-key handoff in handler replay

<a id="k-seed-native-path-key-replay-edge-20261125"></a>
**K-SEED-NATIVE-PATH-KEY-REPLAY-EDGE-20261125.** The [implementation review](design/ImplementationArchitecture.md#symbol-uses-across-the-native-path-key-loan) continues D-ARCH-REVIEW r189→190. The native provider at `96fda399:applications/cod-booter/native/path_provider.c` uses its explicit borrowed input and synchronous callback. A private function declaration can express that promise. The checked relay must still identify the actual CoD body and account for its symbol uses. The wider recursive handler and other native procedures retain their existing acceptance boundaries.

## 2026-11-25 — Account for a local installation across a synchronous here call

<a id="k-seed-live-handler-local-here-supply-20261125"></a>
**K-SEED-LIVE-HANDLER-LOCAL-HERE-SUPPLY-20261125.** The [implementation review](design/ImplementationArchitecture.md#local-symbol-supply-through-a-direct-here-call) continues D-ARCH-REVIEW r188→189 under the [family replay decision](#h-perform-family-capture-20261125). At `bba9a1d0:cod/lang_checked.hh` the completed-frame fallback stops at a live handler’s `perform` even when its helper supplies the callback’s only read. Reuse the bound receiver and outlined callback to account for that local supply. The paired unmasked use still requires an activation-time snapshot. The recursive module-graph handler remains under the r188 compatibility gate.

## 2026-11-25 — Review empty replay and unused snapshot storage in live handlers

<a id="k-seed-live-handler-compatibility-20261125"></a>
**K-SEED-LIVE-HANDLER-COMPATIBILITY-20261125.** The [compatibility review](design/ImplementationArchitecture.md#live-handlers-without-scalar-snapshot-fields) continues D-ARCH-REVIEW r187→188 under [H-PERFORM-FAMILY-CAPTURE](#h-perform-family-capture-20261125). In the saved `LiveTable` workload an actual `perform` requires evidence for the entire helper family. The `ModuleGraph` handler at `96fda399:applications/cod-booter/src/ModuleGraph.cod` exercises current-context invocation. Omitting unused snapshot storage there requires both closed consumer evidence and preserved activation behavior. The assertion and failed-argument controls exercise restoration and non-entry, respectively.

## 2026-11-25 — Review scalar family replay in a live scoped handler

<a id="k-seed-live-scalar-handler-replay-20261125"></a>
**K-SEED-LIVE-SCALAR-HANDLER-REPLAY-20261125.** The delegated [architecture review](design/ImplementationArchitecture.md#replaying-scalar-bindings-in-a-live-scoped-handler) continues D-ARCH-REVIEW r186→187 under the [confirmed family replay rule](#h-perform-family-capture-20261125). The live record at `96fda399:cod/lang_llvm.hh` carries lexical inputs. The reviewed extension stores each activation's outstanding scalar bindings after those inputs and restores the caller's bindings after handler cleanup. An empty selected method summary leaves open what its family's other members need. Acceptance includes an unselected symbol-using overload, recursive activation and unwind.

## 2026-11-25 — Use current scalar bindings in a directly supplied here body

<a id="k-seed-here-current-bindings-20261125"></a>
**K-SEED-HERE-CURRENT-BINDINGS-20261125.** The delegated [architecture review](design/ImplementationArchitecture.md#scalar-symbol-reads-in-a-direct-here-body) continues D-ARCH-REVIEW r185→186 from `5d9531b2`. The ordinary synchronous here handoff already has a lexical frame and the independent scalar symbol load observes the active binding. [D-18](design/CallableStorageAndHereAccess.md#an-effect-in-a-here-callback) owns the literal’s lexical permission and [D-10](design/SymbolicDynamicScope.md#captured-values-and-call-behavior) owns handler replay. Acceptance compares argument-scope restoration, repeated callback reads and unwind.

## 2026-11-25 — Omit symbol-only work for eligible seed function instances

<a id="k-seed-function-symbol-work-20261125"></a>
**K-SEED-FUNCTION-SYMBOL-WORK-20261125.** The delegated [architecture review](design/ImplementationArchitecture.md#functions-with-an-empty-outgoing-symbol-set) continues D-ARCH-REVIEW r184→185 from `c111e60a` under the [trusted-seed cost decision](#h-effect-checking-cost-and-compiler-stages-20261125). A selected ordinary function with no effect-capable handoff has an empty outgoing dynamic-symbol set even in a mixed world. Keep preparation for separately demanded callbacks, actual calls, final frames and handler replay. This changes work-item admission in the seed’s symbol phase.

## 2026-11-25 — Use the installed procedure family’s union for perform capture and restoration

<a id="h-perform-family-capture-20261125"></a>
**H-PERFORM-FAMILY-CAPTURE-20261125.** The project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “proc 的 perform 准备，也用 family 并集去 capture 和 restore 吧，从简。” [D-10r268](design/SymbolicDynamicScope.md#captured-values-and-call-behavior) extends r267’s [outstanding-requirement union](#h-effect-checking-cost-and-compiler-stages-20261125) to runtime replay, including unselected methods of the installed family. D-18r137, D-ARCH-REVIEW r184 and roadmap r43 carry the rule through handler activation, lowering and exceptional restoration. The [r183 entry evidence](#k-seed-module-entry-symbol-consumers-20261125) at `9961d657` used name-only handler activation in both source orders. Calling this storage of a proc value misstated D-18’s existing restriction. The trusted-seed exemption from static availability checking continues.

## 2026-11-25 — Distinguish direct invocation from capture at seed module entry

<a id="k-seed-module-entry-symbol-consumers-20261125"></a>
**K-SEED-MODULE-ENTRY-SYMBOL-CONSUMERS-20261125.** Under the [trusted-seed removal decision](#h-effect-checking-cost-and-compiler-stages-20261125) at `07868aae`, D-ARCH-REVIEW r182→183 clears removal of entry-only ordinary availability facts and the rechecks they induce. The [reviewed boundary](design/ImplementationArchitecture.md#symbol-requirements-at-seed-module-entry) preserves full reads for actual capture and layout, and the narrower inputs used for native loans and scoped handlers. The implementation reference is `e67910be:cod/lang_checked.hh`. Language and performance gates continue.

## 2026-11-25 — Retire selected-path binding verification from the trusted seed

<a id="h-effect-checking-cost-and-compiler-stages-20261125"></a>
**H-EFFECT-CHECKING-COST-AND-COMPILER-STAGES-20261125.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user identified selected-path missing-binding checks as “耗时长的一个主要怀疑点” and specified “之前的 booter 和 e-wrks/cod 只实现极简保守版。” Remove the seed’s availability proof and its exclusive work. Keep symbol facts used to generate correct code. Before 2.0 both semantic checkers use the [family union after branch-local supply](design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20). Finer path-sensitive admission is deferred for consideration after 2.0. D-10r267, D-ARCH-REVIEW r182 and roadmap r42 extend [the r266 scope](#h-booter-branch-local-effect-requirements-20261125) at `d956ce7d`. The existing whole-cutover performance gate continues.

## 2026-11-25 — Keep family effect admission in the booter and discharge branch-local uses

<a id="h-booter-branch-local-effect-requirements-20261125"></a>
**H-BOOTER-BRANCH-LOCAL-EFFECT-REQUIREMENTS-20261125.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user clarified “seed 已经不检查了吧？ booter 适用这个简化决策” and “任何分支被本地 `effect` install 满足的需求，不进入 family 并集。” [D-10r266](design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20), D-ARCH-REVIEW r181 and bootstrap roadmap r41 supersede the [r265 assignment to the seed](#h-family-effect-requirements-20261125). During collection the booter discharges each locally supplied use before combining the outstanding demands. An unsupplied sibling still contributes. The family-wide admission rule, 2.0 reconsideration point and invocation semantics continue for the checker. The [trusted seed contract](#h-trusted-seed-translation-20261125) governs seed translation.

## 2026-11-25 — Require the whole method family’s incoming effect symbols before 2.0

<a id="h-family-effect-requirements-20261125"></a>
**H-FAMILY-EFFECT-REQUIREMENTS-20261125.** The project user directed “在 method family 口径上，汇总所有路径可能要求的 symbols 并集，要求caller上下文完整提供” for all current CoD compilers and deferred branch precision until at least 2.0. [D-10r265](design/SymbolicDynamicScope.md#family-wide-effect-requirements-before-20), D-ARCH-REVIEW r180 and bootstrap roadmap r40 record the first-release trade-off under P-738/P-739. A miss on the family-wide union now rejects a call even when its selected overload or branch is safe. This supersedes the precise fallback in the [earlier seed bound](#h-seed-symbol-union-admission-20261124) for this question. The [trusted seed scope](#h-trusted-seed-translation-20261125) is narrowed by this specific default check. Dynamic lookup, captured/current values, restoration and ownership retain their existing meaning.

## 2026-11-25 — Finalize a scalar caller after its module names are available

<a id="k-seed-scalar-caller-name-finalization-20261125"></a>
**K-SEED-SCALAR-CALLER-NAME-FINALIZATION-20261125.** D-ARCH-REVIEW r178→179 continues [scalar frame reuse](#k-seed-completed-scalar-frames-20261125) for the already admitted direct calls to completed byte literal functions. At `71cf4246:cod/lang_checked.hh`, module entry can introduce a value after the caller frame has been prepared. The [reviewed finalizer](design/ImplementationArchitecture.md#reusing-a-completed-scalar-frame) checks the caller’s written call names in their original lookup context and preserves ordinary diagnostic ordering. Implementation and acceptance belong to mainline.

## 2026-11-25 — Reuse completed call-free scalar frames at final validation

<a id="k-seed-completed-scalar-frames-20261125"></a>
**K-SEED-COMPLETED-SCALAR-FRAMES-20261125.** At `069c377a:cod/lang_checked.hh`, the producer retains the selected scalar method’s frame and effects, and final validation revisits its body. D-ARCH-REVIEW r177→178 continues P-682/P-683 and the [phase rule](#h-completed-callee-behavior-20261125). The [reviewed architecture](design/ImplementationArchitecture.md#reusing-a-completed-scalar-frame) lets LLVM use that original frame when emission facts are complete. The finalizer observes parameter shadows after entry names become available and preserves warning and error order. This records a design decision for mainline implementation.

## 2026-11-25 — Make invalid algorithms inexpressible and justify workqueues by project value

<a id="h-workqueue-value-20261125"></a>
**H-WORKQUEUE-VALUE-20261125.** The project user directed “我倾向于让错误算法无路可走的工程手段，必须要有绝对价值相关的理由，才能使用workqueue什么的机制” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-ARCH-REVIEW r176→177 extends P-682/P-683 and the [material-consumer rule](#h-compiler-work-necessity-20261125) to rescheduling. The [standing rule](AGENTS.md#passes-and-dependency-changes) requires an identifiable outcome and a reason a simpler algorithm falls short. The [seed example](design/ImplementationArchitecture.md#when-a-workqueue-is-justified) applies the existing [phase decision](#h-completed-callee-behavior-20261125) to the solver’s interface. Mainline owns implementation within the agreed architecture.

## 2026-11-25 — Produce the seed’s final body effect summary after loop settlement

<a id="k-seed-final-body-effect-production-20261125"></a>
**K-SEED-FINAL-BODY-EFFECT-PRODUCTION-20261125.** D-ARCH-REVIEW r175→176 continues the [per-kind phase rule](#h-completed-callee-behavior-20261125) and the [trusted-source contract](#h-trusted-seed-translation-20261125). At `efb06925:cod/lang_checked.hh`, `check_flow` always runs a final proving traversal when validating, and `begin_check` resets the ordinary effect accumulator before it. The [reviewed boundary](design/ImplementationArchitecture.md#producing-effects-during-final-body-validation) assigns the outgoing effect summary to the traversal with a consumer while preserving loop settlement, late body materialization and the ordinary effect solver. This records architecture approval for mainline implementation.

## 2026-11-25 — Confirm major program structure and algorithms and delegate routine delivery

<a id="h-mainline-design-review-boundary-20261125"></a>
**H-MAINLINE-DESIGN-REVIEW-BOUNDARY-20261125.** The project user told the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “主线也不应该事无巨细地跟你确认吧，不是结对编程” and clarified “程序大结构和算法还是要找你确认，这种时机应该不会很多”. D-ARCH-REVIEW r174→175 supersedes the per-addition approval requirement of [the earlier structural review](#h-project-architecture-review-20261122). Under [the revised division of responsibilities](design/ImplementationArchitecture.md#design-review-and-delivery-responsibilities), mainline obtains confirmation for major structural and algorithmic choices before implementation and owns routine choices, verification and publication within the agreed design. Human confirmation still governs changes to language contracts and user principles. P-682/P-683 retain the material-consumer and evidence requirements.

## 2026-11-25 — Retire test fixtures and restore them on demand

<a id="h-test-fixtures-on-demand-20261125"></a>
**H-TEST-FIXTURES-ON-DEMAND-20261125.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), after an inventory of 72 seed cases was proposed for migration, the project user directed “测试fixture可以都砍了，有需要时再加回来”. The [working rule](AGENTS.md#tests) permits removal of all existing test fixtures, including complete groups. A concrete need determines when to add a case back. This supersedes fixture retention and case-by-case migration expectations. [D-ARCH-REVIEW r174](design/ImplementationArchitecture.md#justifying-compiler-work-before-coding) carries the decision into implementation review. Language contracts and the seed’s delivery objective continue.

## 2026-11-25 — Keep declaration results that supply a translation contract

<a id="k-seed-declared-result-consumers-20261125"></a>
**K-SEED-DECLARED-RESULT-CONSUMERS-20261125.** D-ARCH-REVIEW r173→174 continues P-682/P-683 under the trusted-source decision. The [review](design/ImplementationArchitecture.md#selecting-a-declared-result-for-translation) traces the comparison-only uses of `check_declared_results` at `8bab8677:cod/lang_checked.hh` and retains covering declarations that determine an omitted result or answer a type query. D-10/H-258 and D-24 retain their language contracts. The [fixture reference](tests/cod/language/README.md#current-implementation-boundary) r486→487 qualifies archived result-mismatch expectations separately from translation and declared failure transport.

## 2026-11-25 — Move checked operations and prepared defer effects to their consumers

<a id="k-seed-operation-publication-20261125"></a>
**K-SEED-OPERATION-PUBLICATION-20261125.** D-ARCH-REVIEW r172→173 continues P-682/P-683. The [review](design/ImplementationArchitecture.md#publishing-a-checked-operation-and-registering-a-defer) traces two final copies of local operations and the copy of each prepared effect at `fefd18fa:cod/lang_checked.hh`. Moving these objects preserves expression-table replacement, defer preparation, exception routing and the metadata used by LLVM. The [earlier call-effect review](#k-seed-call-effect-lifetime-20261125) continues through the handoff to the staged action.

## 2026-11-25 — Retire two seed reports about entry family installation

<a id="k-seed-entry-installation-reports-20261125"></a>
**K-SEED-ENTRY-INSTALLATION-REPORTS-20261125.** D-ARCH-REVIEW r171→172 continues P-682/P-683 under the trusted-source decision. The [review](design/ImplementationArchitecture.md#translating-an-entry-that-extends-a-dependency-family) traces the candidate walk and the end-of-entry branch proof at `b00fdc9d:cod/lang_checked.hh` to their diagnostic sinks. The entry extension identities still control cancellation, dependency lifetime and emitted installation. D-21 and H-379/H-380 retain their existing scope. The [fixture reference](tests/cod/language/README.md#current-implementation-boundary) r485→486 assigns the two missing-installation expectations to the checked archive.

## 2026-11-25 — Apply seed call and cleanup effects to their private records

<a id="k-seed-apply-private-effects-20261125"></a>
**K-SEED-APPLY-PRIVATE-EFFECTS-20261125.** D-ARCH-REVIEW r170→171 continues P-682/P-683. At `284894ce:cod/lang_checked.hh`, the [review](design/ImplementationArchitecture.md#applying-a-private-effect-record) clears the redundant copy inside `apply_effects` for its two callers and the unused pending-exception initialization for immediate invocation rows. The cleanup cache, per-origin substitution, post-application selection invalidation and actual exception routes retain their established behavior.

## 2026-11-25 — Build builtin effect records only for deferred invocation

<a id="k-seed-immediate-builtin-effects-20261125"></a>
**K-SEED-IMMEDIATE-BUILTIN-EFFECTS-20261125.** D-ARCH-REVIEW r169→170 continues P-682/P-683 after the [call-effect lifetime review](#k-seed-call-effect-lifetime-20261125). At `284894ce:cod/lang_checked.hh`, the [two builtin producers](design/ImplementationArchitecture.md#recording-builtin-effects-for-a-prepared-call) record immediate failure flow independently of the map used for defer registration. The review clears omission of that map’s entries for immediate occurrences and preserves declaration checks, ordered failure events and prepared-call outcomes.

## 2026-11-25 — Retain seed call effects through their last use

<a id="k-seed-call-effect-lifetime-20261125"></a>
**K-SEED-CALL-EFFECT-LIFETIME-20261125.** D-ARCH-REVIEW r168→169 continues P-682/P-683. Following the [material-consumer rule](#h-compiler-work-necessity-20261125), the [review](design/ImplementationArchitecture.md#retaining-call-effects-through-their-last-use) traces the per-exit effect map at `9a299525:cod/lang_checked.hh` from `finish_call` to its immediate application or a prepared child's defer registration. Immediate operations can release the map once applied. Prepared calls retain it through registration. Call behavior, result evidence and the existing callee solver keep their prior contracts.

## 2026-11-25 — Retire two static predicate assertion reports from trusted translation

<a id="k-seed-predicate-assertion-reports-20261125"></a>
**K-SEED-PREDICATE-ASSERTION-REPORTS-20261125.** D-ARCH-REVIEW r167→168 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). The [review](design/ImplementationArchitecture.md#translating-a-predicate-assertion) traces the ordinary and type-query reports at `9a299525:cod/lang_checked.hh` and the runtime guard in `9a299525:cod/lang_llvm.hh`. D-10-ASSERT and H-470 retain the language rejection. The [fixture reference](tests/cod/language/README.md) r484→485 distinguishes the compile-only negative from the executable operand, failure and cleanup controls.

## 2026-11-25 — Permit a supplied here body to initialize a live scalar slot

<a id="k-seed-live-here-slot-20261125"></a>
**K-SEED-LIVE-HERE-SLOT-20261125.** D-ARCH-REVIEW r166→167 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). The [review](design/ImplementationArchitecture.md#supplying-a-live-scalar-slot-to-a-here-body) traces the original value-slot address through seed lowering at `fde288d7:cod/lang_checked.hh` and `fde288d7:cod/lang_llvm.hh`. It clears removal of the supply-time payload report for admitted numeric and enum value slots. D-18-HERE-STORAGE and D-28-INIT continue to govern lexical access and each executed read.

## 2026-11-25 — Keep one full view for a projected case break

<a id="k-seed-case-projection-view-20261125"></a>
**K-SEED-CASE-PROJECTION-VIEW-20261125.** D-ARCH-REVIEW r165→166 continues the [case projection review](#k-seed-case-projection-read-20261125). At `3144d6e5:cod/lang_checked.hh`, the presence check reads only the nullable-view predicate and target presence. The enclosing break record already compares the same full view. The [revised scope](design/ImplementationArchitecture.md#projecting-a-scalar-from-a-case-arm) clears removal of the duplicate view and preserves a readiness distinction observed before the outer finalizer in the R164 nested case. The existing seed acceptance inputs and language rules carry forward.

## 2026-11-25 — Retire the trusted seed’s case projection read report

<a id="k-seed-case-projection-read-20261125"></a>
**K-SEED-CASE-PROJECTION-READ-20261125.** D-ARCH-REVIEW r164→165 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). The [review](design/ImplementationArchitecture.md#projecting-a-scalar-from-a-case-arm) traces the ordinary arm’s initialization snapshot to a single report at `3144d6e5:cod/lang_checked.hh` and clears both for removal. A value-carrying break uses the same bit in pending-exit equality before cleanup. This review preserves that comparison and the independent target-presence check. D-07-CASE and D-22-CONDITIONAL keep their existing meaning.

## 2026-11-25 — Align bootstrap verification with its current consumers

<a id="k-bootstrap-verification-scope-20261125"></a>
**K-BOOTSTRAP-VERIFICATION-SCOPE-20261125.** Two documentation findings from the [compiler-work review](#h-compiler-work-necessity-20261125) concerned expectations still written as current at `c4d07562`. The [language fixture reference](tests/cod/language/README.md#deferred-execution-and-initialization) r483→484 assigns whole-drain coverage and physical read-proof diagnostics to the archived checked seed and keeps valid FIFO and cleanup behavior observable. [D-20-MODULE-GRAPH](applications/cod-booter/docs/ModuleGraph.md) r94→95 continues P-680/P-681. Its acceptance workloads apply the existing [single full-build policy](#k-booter-one-build-20261124), including the exception for a concrete whole-application mode question. Previously observed paired results retain their scope.

## 2026-11-25 — Retire two direct String place reports from trusted translation

<a id="k-seed-direct-string-init-reports-20261125"></a>
**K-SEED-DIRECT-STRING-INIT-REPORTS-20261125.** D-ARCH-REVIEW r163→164 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). The [review](design/ImplementationArchitecture.md#translating-a-direct-string-place) traces the two diagnostic sinks at `303af432:cod/lang_checked.hh` separately from the actual field address, transfer method and completion facts consumed by translation. D-22’s source initialization obligation and D-07’s String transfer policy continue.

## 2026-11-25 — Require a material consumer for proposed compiler work

<a id="h-compiler-work-necessity-20261125"></a>
**H-COMPILER-WORK-NECESSITY-20261125.** After the [ordinary reference write clarification](#k-ordinary-ref-write-facts-20261125), the project user asked the [design side discussion](codex://threads/01a212f2-09c5-7951-994a-e2663ec9a088) to guard against needless compiler work at review and track a repository-wide scan. D-ARCH-REVIEW r162→163 extends the [pre-code rule](AGENTS.md#before-coding) at `c4d07562:AGENTS.md` under P-682/P-683. Review traces the work to the decision it serves and clears only the precision that decision needs. The [application](design/ImplementationArchitecture.md#justifying-compiler-work-before-coding) preserves the applicable language and delivery obligations.

## 2026-11-25 — Separate payload-read diagnostics from seed type evidence

<a id="k-seed-type-evidence-payload-reads-20261125"></a>
**K-SEED-TYPE-EVIDENCE-PAYLOAD-READS-20261125.** D-ARCH-REVIEW r161→162 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). At `79140a20:cod/lang_checked.hh`, the [review](design/ImplementationArchitecture.md#removing-payload-read-diagnostics-from-type-evidence) clears the physical-read diagnostic and its propagation through product call summaries. Structural selection, result and completion evidence still serve translation. The [fixture reference](tests/cod/language/README.md) r482→483 identifies historical checked-seed expectations for these reads.

## 2026-11-25 — Clarify which writes a caller must prove

<a id="k-ordinary-ref-write-facts-20261125"></a>
**K-ORDINARY-REF-WRITE-FACTS-20261125.** D-ARCH-REVIEW r160→161 continues P-682/P-683. The project user questioned the value of tracking completed ordinary `ref` writes in the [design side discussion](codex://threads/01a212f2-09c5-7951-994a-e2663ec9a088). The [clarification](design/ImplementationArchitecture.md#consuming-completed-callee-behavior) qualifies the general instruction to record ordered writes in `79140a20:design/ImplementationArchitecture.md`. It distinguishes runtime visibility, invalidation of a caller fact by possible mutation, and the successful-completion obligation of [an `init` parameter](design/ConstructionAndAcquisition.md#bounded-init-obligations-and-nullability). The bounded seed proof remains an example of a caller that uses stronger evidence under the [existing phase rule](#h-completed-callee-behavior-20261125).

## 2026-11-25 — Remove local payload-read proof work from trusted seed translation

<a id="k-seed-local-payload-read-obligations-20261125"></a>
**K-SEED-LOCAL-PAYLOAD-READ-OBLIGATIONS-20261125.** D-ARCH-REVIEW r159→160 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125) and D-28 initialization rules. The [review](design/ImplementationArchitecture.md#removing-local-payload-read-obligations) traces physical read requirements in `55c5a602:cod/lang_checked.hh` and `cod/lang_completion.hh` to their diagnostic consumers. It clears their removal and identifies the remaining consumers of conditional read sites, reference selections and actual initialization facts.

## 2026-11-25 — Keep declared root presence in seed cleanup analysis

<a id="k-seed-declared-root-presence-20261125"></a>
**K-SEED-DECLARED-ROOT-PRESENCE-20261125.** D-ARCH-REVIEW r158→159 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). After [removal of hypothetical deferred failures](#k-seed-deferred-coverage-removal-20261125), the remaining consumers at `e1b41cfd:cod/lang_checked.hh` and `cod/lang_completion.hh` use the presence of a root declaration. The [review](design/ImplementationArchitecture.md#tracking-presence-of-a-root-exception-declaration) clears removal of the per-expression IDs while retaining action reachability, scope summaries and actual queued occurrences.

## 2026-11-25 — Retire cross-call borrowed payload read proofs from trusted translation

<a id="k-seed-borrowed-read-obligations-20261125"></a>
**K-SEED-BORROWED-READ-OBLIGATIONS-20261125.** Following the [trusted-source decision](#h-trusted-seed-translation-20261125), D-ARCH-REVIEW r157→158 continues P-682/P-683 and D-28 initialization semantics. At `d973346d:cod/lang_checked.hh`, a callee’s borrowed read is exported by parameter ordinal and consumed as a caller-side proof obligation. The [removal review](design/ImplementationArchitecture.md#removing-interprocedural-borrowed-payload-read-obligations) retains the Boolean and exception input observations needed by actual results and writes, runtime guards, prepared slots and conditional representation limits.

## 2026-11-25 — Apply incremental build approval to CMake regeneration

<a id="h-incremental-cmake-regeneration-20261125"></a>
**H-INCREMENTAL-CMAKE-REGENERATION-20261125.** The project user clarified “CMake regeneration 不触发llvm大面积rebuild的情况下也不需要审批” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-ARCH-REVIEW r156→157 supersedes the blanket approval for regeneration in `24572783:AGENTS.md` and its Cursor counterpart. The [standing rule](AGENTS.md#bootstrap-and-build-configuration-safety) permits routine existing-tree regeneration and requires inspecting the refreshed plan before executing any uncertain or large-scale LLVM build. The seed executable rename is the first application.

## 2026-11-25 — Remove unused pending-read propagation from seed leaf classification

<a id="k-seed-pending-leaf-reads-20261125"></a>
**K-SEED-PENDING-LEAF-READS-20261125.** The [trusted-source cutover](#h-trusted-seed-translation-20261125) supersedes collecting pending read origins for the archived [R132 leaf boundary](#k-seed-closed-leaf-behavior-20261125). At `d973346d:cod/lang_checked.hh`, the presence of an ordinary callee already determines the borrowed procedure’s fallback before any propagated read could affect admission. D-ARCH-REVIEW r155→156 continues P-682/P-683 and clears the [classification change](design/ImplementationArchitecture.md#removing-unused-read-propagation-from-leaf-classification). The checked byte operation, completed exit facts and separate scalar caller path retain their existing scope.

## 2026-11-25 — Name the seed executable and the booter entry

<a id="h-bootstrap-tool-and-entry-names-20261125"></a>
**H-BOOTSTRAP-TOOL-AND-ENTRY-NAMES-20261125.** In the [entry naming discussion](codex://threads/01a212d2-a272-7611-bc6b-775c6d307521) the project user selected `src/__main__.cod` for the booter and `cod-seed` for the seed executable, and requested implementation. At `23796b73:applications/cod-booter/Makefile` the inputs still use `bin/cod` and `src/CompilerCLI.cod`. D-20-BOOTSTRAP r38→39 applies the reserved local entry name from [H-499](#h-499) to the single executable’s [three operations](applications/cod-booter/README.md#bootstrap-executable-and-entry-names). D-ARCH-REVIEW r154→155 records [build and lookup migration](design/ImplementationArchitecture.md#naming-the-bootstrap-executables-and-source-entry). Explicit source-path invocation continues during bootstrap.

## 2026-11-25 — Separate the receiver result query from callback execution

<a id="k-seed-here-query-20261125"></a>
**K-SEED-HERE-QUERY-20261125.** At `f40d4db8:cod/lang_checked.hh`, native providers return from their own type-evidence branch before `query_here_call`. The remaining uncovered-exception report reads the temporary exits of a separately invoked callback during an ordinary receiver query. This corrects the earlier attribution to native feedback. D-ARCH-REVIEW r153→154 continues P-682/P-683 and the [trusted-source decision](#h-trusted-seed-translation-20261125). The [review](design/ImplementationArchitecture.md#querying-the-result-of-a-call-with-a-supplied-callback) clears removal of that body traversal while preserving the result of the receiver, same-scope feedback, standalone full-scope inference and actual callback emission.

## 2026-11-25 — Remove hypothetical deferred failures from trusted translation

<a id="k-seed-deferred-coverage-removal-20261125"></a>
**K-SEED-DEFERRED-COVERAGE-REMOVAL-20261125.** Following the ordinary terminal coverage removal at `42ca800c:cod/lang_checked.hh`, D-ARCH-REVIEW r152→153 continues P-682/P-683 under the [trusted-source decision](#h-trusted-seed-translation-20261125). The [cleanup review](design/ImplementationArchitecture.md#removing-hypothetical-failures-from-seed-cleanup-analysis) clears removal of synthetic declared-root arrivals and their source-coverage flags in `42ca800c:cod/lang_completion.hh`. The ordered reasons and payloads for actual failures, the cyclic model limit and D-17’s completed-pass language rule retain their existing scope. The [seed-first work order](#h-seed-performance-first-20261125) continues.

## 2026-11-25 — Finish the seed performance task before verifying booter behavior

<a id="h-seed-performance-first-20261125"></a>
**H-SEED-PERFORMANCE-FIRST-20261125.** The project user asked “seed性能任务完成前，先不需要关注booter行为吧？” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-ARCH-REVIEW r151→152 applies the [work-order rule](AGENTS.md#order-work-by-the-decision-it-serves) to the [seed cutover](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance). The seed uses real booter source to check translation and, once verifier-only work is gone, to assess compilation time. Verification of the booter’s own behavior follows the seed performance task. The [terminal exception diagnostic review](design/ImplementationArchitecture.md#removing-terminal-checked-exception-diagnostics) clears the next seed removal against the consumers in `e8225a21:cod/lang_checked.hh`. The booter retains its static checking responsibility under [the stage decision](#h-trusted-seed-translation-20261125).

## 2026-11-25 — Order evaluation by its decision and prerequisites

<a id="h-evaluate-the-intended-state-20261125"></a>
**H-EVALUATE-THE-INTENDED-STATE-20261125.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user specified “所有非代码生成所依赖的checks都砍掉以后，才测试性能” and clarified “不是这件具体事的直接原则，是元工作原则”. D-ARCH-REVIEW r150→151 records the [general work-order rule](AGENTS.md#order-work-by-the-decision-it-serves). It extends the measurement guidance at `e8225a21:AGENTS.md` to evaluation decisions generally. The [seed cutover](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance) applies it to the default translator. Correctness and compatibility checks whose results carry forward remain appropriate during implementation.

## 2026-11-25 — Archive the checked seed and clear removal of its second definition graph

<a id="k-seed-remove-definition-graph-20261125"></a>
**K-SEED-REMOVE-DEFINITION-GRAPH-20261125.** The annotated tag `archive/checked-cpp-seed` (tag object `54a52351`) resolves to `a0922741befb276e8b5225c2eaaae84a8923d3e3`. On the booter’s unmodified `ProviderNames.cod`, the archived seed enters both the translating and definition-only graphs. At `a0922741:cod/lang_checked.hh:25887` only warnings from the second graph reach the first on success. D-ARCH-REVIEW r149→150 continues P-682/P-683 under [H-TRUSTED-SEED-TRANSLATION-20261125](#h-trusted-seed-translation-20261125). The [first removal](design/ImplementationArchitecture.md#removing-the-second-definition-graph) preserves the translating graph’s concrete dispatch and output.

## 2026-11-25 — Archive the checked C++ seed and focus the seed on LLVM translation

<a id="h-trusted-seed-translation-20261125"></a>
**H-TRUSTED-SEED-TRANSLATION-20261125.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user confirmed “阶段性收尾后打tag归档作为历史参考，然后砍掉所有不需要的checks，仅服务于理想CoD源码的 ll bitcode 生成；遇到实际工程方便性需要时，通过 opt-in 开关加有用的check。” [D-20-BOOTSTRAP and stage diagnostics](applications/cod-booter/README.md#trusted-source-and-the-seed), roadmap r37→38, assign the seed faithful translation of valid booter source and the CoD booter its semantic verification. The decision supersedes the seed’s default verification duty under [r17](#h-booter-exceptions-diagnostics-20261120) and the [r141 full-checker migration prerequisites](#h-seed-migration-completion-20261125) at `abd1d4b0:design/ImplementationArchitecture.md`. D-ARCH-REVIEW r148→149 records the [archive, removal and acceptance sequence](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance). Valid-source runtime semantics, booter verification and the general per-kind phase rule continue.

## 2026-11-25 — Review sharing of reference target bounds

<a id="k-seed-reference-target-layers-20261125"></a>
**K-SEED-REFERENCE-TARGET-LAYERS-20261125.** D-ARCH-REVIEW r147→148 continues P-682/P-683 and the [r141 scope](#h-seed-migration-completion-20261125). At `95082dcf:cod/lang_checked.hh:6251` a surviving binding may keep its target row after its owner expires. At `95082dcf:cod/lang_llvm.hh:2956` the emitted null check depends on the checked operand’s possible targets. The [review](design/ImplementationArchitecture.md#sharing-reference-target-bounds) preserves the distinction between expiration and erasure, versioned branch conditions and the existing refinement stop.

## 2026-11-25 — Review sharing of stored Boolean predicates

<a id="k-seed-boolean-predicate-layers-20261125"></a>
**K-SEED-BOOLEAN-PREDICATE-LAYERS-20261125.** D-ARCH-REVIEW r146→147 continues P-682/P-683 under the [seed-wide migration decision](#h-seed-migration-completion-20261125). At `e157ce8f:cod/lang_checked.hh:3869` a multi-target read tests for an absent key. At `e157ce8f:cod/lang_checked.hh:6070` the join takes the union of keys before comparing the two proof pointers. The [review](design/ImplementationArchitecture.md#sharing-stored-boolean-predicates) preserves both behaviors and the numeric order used to assign observation IDs.

## 2026-11-25 — Review separate application and sealing for the lexer classifier

<a id="k-seed-lexer-classifier-caller-20261125"></a>
**K-SEED-LEXER-CLASSIFIER-CALLER-20261125.** D-ARCH-REVIEW r145→146 continues P-682/P-683 under the [per-kind phase rule](#h-completed-callee-behavior-20261125). At `08201d9b:applications/cod-booter/src/Lexer.cod:23` the short-circuit caller uses two completed literal definitions. The [reviewed structure](design/ImplementationArchitecture.md#checking-the-lexers-short-circuit-caller) separates applying their contracts, deriving the caller contract from completed exit observations, and comparing final validation. The [whole-seed completion conditions](#h-seed-migration-completion-20261125) remain open.

## 2026-11-25 — Review completed behavior for the lexer’s literal token functions

<a id="k-seed-lexer-literal-contracts-20261125"></a>
**K-SEED-LEXER-LITERAL-CONTRACTS-20261125.** D-ARCH-REVIEW r144→145 continues P-682/P-683. At `b24c2df1:applications/cod-booter/src/Lexer.cod:23` the real error classifier calls two zero-argument functions that return exact byte literals. The [review](design/ImplementationArchitecture.md#completing-literal-returns-used-by-the-lexer) reuses the checked leaf producer, keeps normal source diagnostics and leaves ordinary caller composition open under the [full migration](#h-seed-migration-completion-20261125).

## 2026-11-25 — Review sharing of per-origin completion inputs

<a id="k-seed-completion-input-layers-20261125"></a>
**K-SEED-COMPLETION-INPUT-LAYERS-20261125.** D-ARCH-REVIEW r143→144 continues P-682/P-683. At `dfb050fc:cod/lang_checked.hh:5962`, the cleanup join consults the incoming branches as well as their per-origin rows. The [reviewed representation](design/ImplementationArchitecture.md#sharing-per-origin-completion-inputs) shares snapshots while preserving that join and the original remap and scope-drain order. The [whole-seed migration](#h-seed-migration-completion-20261125) remains open.

## 2026-11-25 — Preserve the micro-dispatch experiment at its historical revision

<a id="k-shilos-dispatch-experiment-archive-20261125"></a>
**K-SHILOS-DISPATCH-EXPERIMENT-ARCHIVE-20261125.** D-21 r36→37 and D-25 r17→18 retain the [case descriptions](https://github.com/complyue/cod-project/blob/2c1d987fcffb60bbf537b5922127555740e6c746/experiments/micro-dispatch/README.md) and [recorded observations](https://github.com/complyue/cod-project/blob/2c1d987fcffb60bbf537b5922127555740e6c746/experiments/micro-dispatch/observations.json) at `2c1d987f` under the [shilos retirement decision](#h-retire-shilos-20261125). Both cited blobs match the tree at `dfb050fc`. D-ARCH-REVIEW r142→143 records the [subtree disposition and the optional language report’s header inputs](design/ImplementationArchitecture.md#retiring-shilos). Current dispatch rules and the scope of the experimental evidence continue.

## 2026-11-25 — Review shared lexical declaration maps

<a id="k-seed-lexical-declaration-layers-20261125"></a>
**K-SEED-LEXICAL-DECLARATION-LAYERS-20261125.** D-ARCH-REVIEW r141→142 continues P-682/P-683. At `9c368da4:cod/lang_checked.hh:5840` the name merge checks the entire ordered right map against the left, even on a departing arm. The [reviewed representation](design/ImplementationArchitecture.md#sharing-lexical-declaration-maps) preserves that check with shared append-only ancestry and private additions. The [full migration and measurement conditions](#h-seed-migration-completion-20261125) continue.

## 2026-11-25 — Carry the seed migration scope through review

<a id="h-seed-migration-completion-20261125"></a>
**H-SEED-MIGRATION-COMPLETION-20261125.** In the [seed performance discussion](codex://threads/01a21256-bf46-7801-ade0-9f396b479fa1) the project user specified “layering改造应该把能覆盖掉的地方都覆盖掉” and “phase rule必须全面应用了再测性能，跟layering改造一样”. Compile time takes priority. An initial RAM increase is acceptable and effective heap reuse follows separately. D-ARCH-REVIEW r140→141 applies P-682/P-683 to the [complete seed scope](design/ImplementationArchitecture.md#seed-checker-migration-and-performance-acceptance). It supersedes the six-table limit for that objective at `fbede9ed:design/ImplementationArchitecture.md`, preserves the [per-kind phase rule](#h-completed-callee-behavior-20261125), and makes the [review responsibility](AGENTS.md#review-scope-and-completion) explicit.

## 2026-11-25 — Fix the implicit OO receiver binding at entry

<a id="h-fixed-self-binding-20261125"></a>
**H-FIXED-SELF-BINDING-20261125.** The project user specified “`self` 参数内部应该声明为 `const ref self:Type`” to prevent accidental `=` rebinding. [D-07 r222→223](design/DomainLanguageDesign.md#receivers-in-field-accessors-and-member-methods) corrects the receiver table at `27323666:design/DomainLanguageDesign.md`. P-724/P-725 continue for visible target access and receiver identity. Apply leading `const ref` to writable and readonly targets alike in accessors, named members and call operators. The [OO source decision](#h-oo-call-operator-head-20261125) and the [queued implementation order](design/ImplementationArchitecture.md#oo-call-operator-implementation-plan) continue.

## 2026-11-25 — Bound the first regional fixture migration

<a id="k-shilos-native-fixture-boundary-20261125"></a>
**K-SHILOS-NATIVE-FIXTURE-BOUNDARY-20261125.** D-ARCH-REVIEW r138→139 continues the [shilos retirement](#h-retire-shilos-20261125) at `47b60a0b:design/ImplementationArchitecture.md`. The [fixture review](design/ImplementationArchitecture.md#first-native-regional-fixture-checkpoint) pairs a relocated keyed cycle with an abrupt initializer and assigns the remaining legacy observations to separate evidence. The seed with a matching C runtime reproduced both source outputs. The instrumented observations remain pending.

## 2026-11-25 — Declare data call operators on their OO receiver

<a id="h-oo-call-operator-head-20261125"></a>
**H-OO-CALL-OPERATOR-HEAD-20261125.** The project user confirmed “数据接收者的调用运算符 `(())` 只支持 OO 接收者声明语法” for `func` and `proc`, signatures and definitions, in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). [D-07 r221→222](design/DomainLanguageDesign.md#functor-receivers-and-callback-arguments) supersedes the two source heads in [H-FUNCTOR-RECEIVER-SUGAR-20261124](#h-functor-receiver-sugar-20261124) at `41f82794:design/DomainLanguageDesign.md`. The owner-preserving diagnostic and the OO-only first implementation are confirmed. P-724/P-725 and P-636/P-637 continue for the first dispatch argument, target access, ownership and type-carried family. Ordinary operator syntax continues. D-15 r81→82, D-31 r13→14 and D-10 r263→264 migrate the module and generator examples. Guide r11 and the syntax showcase adopt the confirmed form. [D-ARCH-REVIEW r138](design/ImplementationArchitecture.md#oo-call-operator-implementation-plan) records the ordered future work. [Source reference r482](tests/cod/language/README.md#data-call-operator-coverage) records the separate frontend observations.

## 2026-11-25 — Correct the future package repository name

<a id="h-package-repository-name-20261125"></a>
**H-PACKAGE-REPOSITORY-NAME-20261125.** The project user corrected a typo in the [manual bootstrap package decision](#h-manual-bootstrap-packages-20261125). Future package management belongs in `e-wrks/cod`. D-03 r3→4 and the booter roadmap r36→37 continue the hand-prepared project layout and lookup rules recorded at `41f82794:design/CodProjectStructureAndPkgMgmt.md`.

## 2026-11-25 — Check the manually staged project example with the seed

<a id="k-manual-project-example-20261125"></a>
**K-MANUAL-PROJECT-EXAMPLE-20261125.** D-03r3 continues the [manual input decision](#h-manual-bootstrap-packages-20261125). The seed built from `2bd8b365:cod/lang_modules.hh` printed 42 for the [two-file project](design/CodProjectStructureAndPkgMgmt.md#directory-layout) after adding the bootstrap reader’s top-level statement terminator to the example at `fed05fd3:design/CodProjectStructureAndPkgMgmt.md`. A second staged provider located both competing paths and preserved prior IR. These are seed observations.

## 2026-11-25 — Retire codp and stage bootstrap dependencies in each project

<a id="h-manual-bootstrap-packages-20261125"></a>
**H-MANUAL-BOOTSTRAP-PACKAGES-20261125.** The project user directed “codp也砍掉，规划到未来 e-wrks/code 去实现，当前 seed/booter 手工制作符合预期的目录结构。” On the proposed `PROJECT/.cod/packages/<dependency>/` convention, including the project itself as a source root, they selected “Use the project-local layout”. [D-03r3](design/CodProjectStructureAndPkgMgmt.md) supersedes the CLI and lockfile draft at `3545f504:design/CodProjectStructureAndPkgMgmt.md` and the producer/transport at `3545f504:applications/cod-booter/docs/ModuleGraph.md:380`. [D-15r81](design/ModuleWorlds.md#resolving-source-module-paths) continues relative lookup, ambiguity rejection, bundle fallback and the shadow warning under P-690/P-691. D-ARCH-REVIEW r137 and D-20-MODULE-GRAPH r94 continue P-680–P-683 for filesystem inputs and the normal compiler entry points.

## 2026-11-25 — Retire the C++ REPL and its workspace commands

<a id="h-retire-cpp-repl-20261125"></a>
**H-RETIRE-CPP-REPL-20261125.** Asked about the user-visible C++ eval and interactive commands, the project user decided “C++ repl也整体砍掉吧，e-wrks/cod 再重新支持repl” and authorized cleanup of live build definitions and regeneration with no LLVM rebuild. This retires [D-02](design/DesignAuthorityRegistry.md#d-02), including its submission cache and workspace commands, and supersedes their migration disposition at `24e8db100b00c3e33e89312b83481a0f1f81ade7:design/ImplementationArchitecture.md:352`. [D-ARCH-REVIEW r136](design/ImplementationArchitecture.md#retiring-shilos) retains the CoD source driver and its native link/run path. The [production compiler](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) owns future REPL design.

## 2026-11-25 — Retire shilos and assign its remaining work by component

<a id="h-retire-shilos-20261125"></a>
**H-RETIRE-SHILOS-20261125.** The project user in the CoD design discussion directed “`shilos` 相关的认定为历史遗留作废吧，尽快删除清理，新的支持性功能都转移到 C native components，高层功能优先使用CoD实现。 seed 内部用 C++ 实现高层部分。” This supersedes the maintained-legacy disposition at `0d8591512e18c2f2c3c2cc26904c5f8ca82aeed4:design/ImplementationArchitecture.md:382`. [D-ARCH-REVIEW r135](design/ImplementationArchitecture.md#retiring-shilos) and [the native roadmap](applications/cod-booter/README.md#next-stage-repositories-and-native-runtime) apply the decision to consumers, build wiring and library removal. [D-01](design/DesignAuthorityRegistry.md#d-01), D-04 and D-05 archive the former C++ specifications at their last revision. D-28r50 withdraws its C++ rename proposal. Regional ownership, DBMR publication and package metadata retain their independent contracts. P-593/P-594 and P-682/P-683 continue.

## 2026-11-25 — Review completed scalar behavior for the booter lexer

<a id="k-seed-lexer-predicate-behavior-20261125"></a>
**K-SEED-LEXER-PREDICATE-BEHAVIOR-20261125.** D-ARCH-REVIEW r133→134 continues P-682/P-683 under the [phase rule](#h-completed-callee-behavior-20261125). The saved IR of the actual lexer contains five direct calls to the predicate at `957e286a:applications/cod-booter/src/Lexer.cod:415`. The [seed review](design/ImplementationArchitecture.md#completing-the-lexers-scalar-predicates) extends the frozen instance contract to checked by-value predicates with one normal exit. It preserves ordinary operator selection and short-circuit behavior. The existing solver receives its frozen-instance exclusion at `957e286a:cod/lang_checked.hh:23478`. Implementation evidence and any full-build timing remain separate.

## 2026-11-25 — Teach product inheritance and receiver interfaces in the idiom guide

<a id="k-idiomatic-oo-showcase-20261125"></a>
**K-IDIOMATIC-OO-SHOWCASE-20261125.** [Guide r10](docs/IdiomaticCoD.md#choose-a-storage-parent-and-a-receiver-interface) applies the confirmed parent-construction, field-shadowing, receiver and dispatch contracts to worked examples. The [syntax showcase](toolchain/syntax/showcase.cod) collects the corresponding source forms and other single-file syntax in short excerpts. The [preview index](toolchain/syntax/README.md) describes the gallery. [D-DOC-IDIOMATICS](design/DesignAuthorityRegistry.md#d-doc-idiomatics), P-694/P-695, continues the existing writing and source-style requirements. Previous guide and showcase: `ff6b68ac70ef139ef26bb7507fbe4ca959a41f76:docs/IdiomaticCoD.md` and `ff6b68ac70ef139ef26bb7507fbe4ca959a41f76:toolchain/syntax/showcase.cod`.

## 2026-11-25 — Review imported functions with one literal return

<a id="k-booter-imported-literal-return-20261125"></a>
**K-BOOTER-IMPORTED-LITERAL-RETURN-20261125.** D-ARCH-REVIEW r132→133 and D-20-MODULE-GRAPH r92→93 continue P-680–P-683. The candidate at `7d4e28a0:applications/cod-booter/src/NativeDeclarations.cod:3411` already stages interfaces for ordinary functions without declared failures. At `7d4e28a0:applications/cod-booter/src/OperandChecking.cod:2176` source-call preparation skips the frozen parser’s `call_node()` and `no_node()`. The [reviewed continuation](applications/cod-booter/docs/ModuleGraph.md#checking-an-imported-function-that-returns-a-scalar-literal) requires positive defining-body evidence on the existing header and checks the selected view and argument list on use. Indexed-field demand and the frozen release gate continue.

## 2026-11-25 — Warn when a derived product declares a shadowing field

<a id="h-product-field-shadow-warning-20261125"></a>
**H-PRODUCT-FIELD-SHADOW-WARNING-20261125.** In the [OO discussion](codex://threads/01a211e6-8b5f-7532-8e5c-c1337ce40796), the project user required a default warning from `e-wrks/cod` for a child shadowing an ancestor’s stored field and explicit options for info or error severity. D-07 r220→221 specifies the [production diagnostic and acceptance cases](design/DomainLanguageDesign.md#production-diagnostic-for-a-shadowed-stored-field) under P-736/P-737. Field selection continues from `582b80ce:design/DomainLanguageDesign.md:279`. The seed and booter gain no new delivery requirement.

## 2026-11-25 — Allow a child to shadow a parent’s stored field

<a id="h-product-field-shadow-20261125"></a>
**H-PRODUCT-FIELD-SHADOW-20261125.** The project user asked “子类shadow父类字段是自然允许的吧，有没有明确讲出来？” in the [OO discussion](codex://threads/01a211e6-8b5f-7532-8e5c-c1337ce40796). D-07 r219→220 corrects the contrary sentence at `d44c1e2a:design/DomainLanguageDesign.md:277`. [The worked example](design/DomainLanguageDesign.md#product-storage-inheritance) preserves both declarations and both initialization places under P-726–P-729. H-MEMBER-NAMES-AND-OWNER-20261124 continues to govern collisions across member categories. Physical layout and compiler implementation remain separate.

## 2026-11-25 — Require explicit parent construction before inherited field access

<a id="h-product-parent-init-20261125"></a>
**H-PRODUCT-PARENT-INIT-20261125.** In the [OO side discussion](codex://threads/01a211e6-8b5f-7532-8e5c-c1337ce40796) the project user decided “子类读写父类字段之前，要求必须显式调用过一次 `super(...)` ctor，否则认为其内嵌父类对象尚未初始化”. D-22 r72→73 qualifies [direct-parent initialization](design/ConstructionAndAcquisition.md#initializing-the-storage-parent-of-a-product) under P-734/P-735. The user also recalled that the ctor has only field initialization places and no `self`, consistent with `b9b426d6:design/DomainLanguageDesign.md:295` and line 297. A normal child result requires the initialized parent. OO member lookup and the runtime exception owner retain their existing contracts.

## 2026-11-25 — Give products a storage parent and OO bodies explicit super-calls

<a id="h-product-extends-super-20261125"></a>
**H-PRODUCT-EXTENDS-SUPER-20261125.** The project user proposed single product storage inheritance and per-exception backtrace storage, accepted direct and named ancestor calls, and concluded “就只在 OO body允许 super-call 好了，不再需要 `(subobj:SupStruct).method()` 语法” in the [OO side discussion](codex://threads/01a211e6-8b5f-7532-8e5c-c1337ce40796). D-07 r218→219 records [storage inheritance](design/DomainLanguageDesign.md#product-storage-inheritance) and [lexically selected super-calls](design/DomainLanguageDesign.md#super-calls-in-oo-bodies). D-17 r151→152 records the abstract root’s [per-object runtime state](design/ExceptionControl.md#runtime-base-of-an-exception). Ordinary actual-type dispatch and construction-trace timing continue from `b9b426d6:design/DomainLanguageDesign.md:714` and `b9b426d6:design/ExceptionControl.md:704`. P-728–P-733 qualify the source and object-identity scope. Compiler support and physical layout are separate.

## 2026-11-25 — Review the first seed caller of completed callee behavior

<a id="k-seed-closed-leaf-behavior-20261125"></a>
**K-SEED-CLOSED-LEAF-BEHAVIOR-20261125.** D-ARCH-REVIEW r131→132 continues P-682/P-683 under [the user’s phase rule](#h-completed-callee-behavior-20261125). At `48011c90:cod/lang_checked.hh:7575` the named call reads the evolving effects, and line 22995 validates against the instance’s effects directly. The [reviewed seed scope](design/ImplementationArchitecture.md#first-seed-call-with-completed-behavior) separates the bound operations, guarded equations, frozen procedure contract and call-site checks for a scalar leaf with a possible bounds failure. Acceptance includes the checker’s account of a write visible to the handler and the behavior of the emitted program. Implementation and verification remain with development mainline.

## 2026-11-25 — Separate collection and use of each kind of information

<a id="h-completed-callee-behavior-20261125"></a>
**H-COMPLETED-CALLEE-BEHAVIOR-20261125.** The project user directed “不允许某种信息在一个phase中既收集又使用。 如果有合理场景必须打破这个规则，拿给我讨论” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). Collection propagates discoveries. The use phase reads the completed input and cannot change it. D-ARCH-REVIEW r130→131 supersedes the proposed intra-caller rechecks in `598a2d13:design/ImplementationArchitecture.md`, “Rechecking an ordinary call in one conditional arm”, and refines the [pass rule](AGENTS.md#passes-and-dependency-changes). The [new contract](design/ImplementationArchitecture.md#consuming-completed-callee-behavior) keeps provisional solving private and preserves the existing owner, alias and completion guarantees under P-682/P-683. Seed commit `932dc122` remains evidence of a limited improvement to its current solver.

## 2026-11-25 — Review source checking of visit-table growth

<a id="k-booter-visit-array-growth-20261125"></a>
**K-BOOTER-VISIT-ARRAY-GROWTH-20261125.** D-ARCH-REVIEW r129→130 and D-20-MODULE-GRAPH r91→92 continue P-680–P-683. Both linked source commands on candidate `af87c3b4` rejected the caller-slot formal in `932dc122:applications/cod-booter/src/ModuleIndex.cod:94`. The [review](applications/cod-booter/docs/ModuleGraph.md#checking-growth-of-the-module-visit-array) extends the byte-array proof to the checked `VisitSlot` sum, its `Vacant` fill and the ordered whole-element transfer. The C++ seed and booter runtime layouts are outside this source-checking change.

## 2026-11-25 — Review indexed byte fields in a demanded booter body

<a id="k-indexed-byte-field-compatibility-20261125"></a>
**K-INDEXED-BYTE-FIELD-COMPATIBILITY-20261125.** D-ARCH-REVIEW r128→129 and D-20-MODULE-GRAPH r90→91 continue P-680–P-683. Candidate `af87c3b4:applications/cod-booter/src/OperandChecking.cod:3626` accepts only a `u64` field. Its new demand for the frozen `ParserStorage.selected_resource_name` exposes a valid `u8` read in the same body. The [repair scope](applications/cod-booter/docs/ModuleGraph.md#deciding-which-bodies-need-indexed-field-checks) preserves the demand trigger and the exact type of each accepted field. Release requires the frozen source and provider comparisons to pass.

## 2026-11-24 — Reject restarting a pass from entry on a local change

<a id="h-restart-from-entry-20261124"></a>
**H-RESTART-FROM-ENTRY-20261124.** The project user directed “该多分几遍处理的就要分，从头重来这种pattern记为工程反例，今后要杜绝” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-ARCH-REVIEW r127→128 records the [project rule](AGENTS.md#passes-and-dependency-changes) under P-682/P-683. At `598a2d13:cod/lang_checked.hh:23192` a callee change can enqueue the whole caller. The [counterexample](design/ImplementationArchitecture.md#restarting-a-pass-from-entry) identifies that same-pass restart as a pattern to eliminate. Existing retries outside the R127 slice are tracked migration work. Deliberate passes and fixed-point evaluation of affected dependencies retain their proper roles.

## 2026-11-24 — Review selective rechecking at an ordinary seed call

<a id="k-seed-ordinary-call-edge-replay-20261124"></a>
**K-SEED-ORDINARY-CALL-EDGE-REPLAY-20261124.** D-ARCH-REVIEW r126→127 continues P-682/P-683. At `039b9c7f:cod/lang_checked.hh:216` an effect read subscribes the caller instance, and a notification in `close_effects` re-enters its body checker. The [scoped implementation review](design/ImplementationArchitecture.md#rechecking-an-ordinary-call-in-one-conditional-arm) admits reuse in the proving pass for one conditional, including its ordinary continuation and typed failure handler. The compiler’s full validation and fallback still govern acceptance.

## 2026-11-24 — Extend receiver declaration sugar to functors

<a id="h-functor-receiver-sugar-20261124"></a>
**H-FUNCTOR-RECEIVER-SUGAR-20261124.** The project user requested “OO 语法糖应该扩展到 functor 语法” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) and authorized a separate design update. D-07 r217→218 applies P-724/P-725 to the call operator. The design uses the existing `(())` name after the receiver dot. The baseline at `2d205cc5:design/DomainLanguageDesign.md` gives the receiver head and explicit first parameter in adjacent sections. The [new paired definitions](design/DomainLanguageDesign.md#functor-receivers-and-callback-arguments) preserve the selected family, access and actual owner under P-636/P-637.

## 2026-11-24 — Review initialized fixed byte-array references in the booter

<a id="k-booter-fixed-byte-array-reference-20261124"></a>
**K-BOOTER-FIXED-BYTE-ARRAY-REFERENCE-20261124.** D-ARCH-REVIEW r125→126 and D-20-MODULE-GRAPH r89→90 continue P-680–P-683 under the binding and target qualifier rule H-366–H-369/P-327. The existing bare reservation at `0d6abff9:applications/cod-booter/src/ExpressionParser.cod:645` has no fixed-binding field. The [source review](applications/cod-booter/docs/ModuleGraph.md#checking-an-initialized-fixed-byte-array-reference) covers an initialized local and requires a checked target before publication. Both seed modes distinguished writing through a fixed slot from rebinding a slot with a read-only target. Linked booter acceptance remains outstanding.

## 2026-11-24 — Review how a slot body proves which failures can escape

<a id="k-booter-array-slot-escape-bound-20261124"></a>
**K-BOOTER-ARRAY-SLOT-ESCAPE-BOUND-20261124.** D-ARCH-REVIEW r124→125 and D-20-MODULE-GRAPH r88→89 continue P-680–P-683. The text-byte operation already routes a possible bounds error at `fb938b59:applications/cod-booter/src/CompletionChecking.cod:829`. The proposed header could omit that error for a direct or forwarded call. The [source-checking review](applications/cod-booter/docs/ModuleGraph.md#checking-growth-of-the-module-key-array) requires the actual routed events and a conservative four-category bound for ordinary imports whose interface supplies none. It retains the existing proof requirement for a slot rebind across modules.

## 2026-11-24 — Review failure facts on a private array-slot header

<a id="k-booter-array-slot-failure-row-20261124"></a>
**K-BOOTER-ARRAY-SLOT-FAILURE-ROW-20261124.** D-ARCH-REVIEW r123→124 and D-20-MODULE-GRAPH r87→88 continue P-680–P-683. Seed `cad15cf2` accepted the prior `015af50c:applications/cod-booter/src/ModuleExportStorage.cod` and rejected the draft whole-sum field transfer. The [R121 refinement](applications/cod-booter/docs/ModuleGraph.md#checking-growth-of-the-module-key-array) retains the typed body proof in the visit frame and clears ordinary named-field writes to the private graph header. The acceptance distinguishes an absent proof from a checked empty failure set. Imported slot calls still require their own completion and rebinding proof.

## 2026-11-24 — Bound the body checks triggered by indexed record fields

<a id="k-booter-indexed-field-demand-20261124"></a>
**K-BOOTER-INDEXED-FIELD-DEMAND-20261124.** D-ARCH-REVIEW r122→123 and D-20-MODULE-GRAPH r86→87 continue P-680–P-683. At `015af50c:applications/cod-booter/src/CaseSchemas.cod` a file-wide scan could reject an unrelated valid field. A prepared field could also demand an enclosing body outside the new source profile in `015af50c:applications/cod-booter/src/DeclarationDemand.cod`. The [corrected review](applications/cod-booter/docs/ModuleGraph.md#deciding-which-bodies-need-indexed-field-checks) covers both causes and the corresponding indexed-destination test for local sums. The linked candidate is held for compatibility.

## 2026-11-24 — Skip an unused caller Boolean snapshot in the seed

<a id="k-seed-boolean-entry-release-20261124"></a>
**K-SEED-BOOLEAN-ENTRY-RELEASE-20261124.** The [reviewed scalar-call change](design/ImplementationArchitecture.md#boolean-snapshots-at-immediate-scalar-calls) shipped at `cad15cf2:cod/lang_checked.hh`. The built seed skipped the snapshot at the lexer’s byte predicate and retained it for a borrowed Boolean. Focused comparisons covered both seed modes, diagnostics, artifacts and runtime behavior. The build-time impact on the full booter was not measured.

## 2026-11-24 — Distinguish the two slots a parameter can forward

<a id="k-booter-forward-parameter-slot-20261124"></a>
**K-BOOTER-FORWARD-PARAMETER-SLOT-20261124.** D-20-MODULE-GRAPH r85→86 continues P-680/P-681 and clarifies the [R121 acceptance workload](applications/cod-booter/docs/ModuleGraph.md#checking-growth-of-the-module-key-array) at `0deaf9c1:applications/cod-booter/docs/ModuleGraph.md`. On seed `75e196aa`, forwarding an ordinary parameter’s writable local reference slot to `reserve_keys` produced lengths 2 inside the forwarding call and 1 in its caller. Passing a freshly made array directly rejected at that operand. These controls pin the existing [parameter binding rule](design/RegionalOwnership.md#independent-binding-and-target-qualifiers).

## 2026-11-24 — Review an unused caller snapshot during seed checking

<a id="k-seed-boolean-entry-snapshot-20261124"></a>
**K-SEED-BOOLEAN-ENTRY-SNAPSHOT-20261124.** D-ARCH-REVIEW r121→122 continues P-682/P-683. During a full booter build, an observed stack reached `current_booleans` from ordinary call checking. At `75e196aa:cod/lang_checked.hh:5136`, a Boolean result alone can trigger a snapshot of every available Boolean in the caller. The [implementation review](design/ImplementationArchitecture.md#boolean-snapshots-at-immediate-scalar-calls) clears omitting that map for a direct independent scalar function call once its inputs and summary establish that the map has no consumer. By-value result predicates and borrowed-argument checks retain their proofs.

## 2026-11-24 — Review source checking for growth of the module key array

<a id="k-booter-reserve-keys-20261124"></a>
**K-BOOTER-RESERVE-KEYS-20261124.** D-ARCH-REVIEW r120→121 and D-20-MODULE-GRAPH r84→85 continue P-680–P-683. The [review](applications/cod-booter/docs/ModuleGraph.md#checking-growth-of-the-module-key-array) covers the scalar leaf at `a934f56b:applications/cod-booter/src/ModuleIndex.cod:103`. The released booter rejects the slot formal. Demanding private bodies with a supplied replacement separately exposes missing byte transfer and slot rebinding checks. The reviewed source path verifies the real allocation, copy and final rebind under the existing owner and completion contracts.

## 2026-11-24 — Share seed binding snapshots and bound symbol requirements on first demand

<a id="k-seed-symbol-bound-release-20261124"></a>
**K-SEED-SYMBOL-BOUND-RELEASE-20261124.** The specialized seed using shared binding snapshots at `54c40e3e:cod/lang_checked.hh` reproduced the saved provider files, LLVM IR and linked booter for `d007ba5c:applications/cod-booter`. The [first-demand rule](design/ImplementationArchitecture.md#first-demand-union-of-dynamic-symbol-requirements) shipped separately at `75e196aa:cod/lang_checked.hh`. Sixteen focused comparisons matched execution, diagnostics and emitted artifacts. Traces showed that a closed call could skip a caller recheck triggered solely by symbol availability. An open call used precise checking from its first visit. A caller with both kinds of use still received the required notification, as did one affected by a borrowed read.

## 2026-11-24 — Reserve a single slot for memory-heavy workloads

<a id="h-workload-based-build-concurrency-20261124"></a>
**H-WORKLOAD-BASED-BUILD-CONCURRENCY-20261124.** The [side discussion](codex://threads/01a210f9-d663-7283-b208-54020badf06c) conveyed the project user’s direction to limit concurrency by workload. A full booter build shares one host slot with comparably heavy jobs. Routine invocations may overlap each other and that build when their combined footprint leaves sufficient memory. The [standing rule](AGENTS.md#concurrency-on-the-build-host) applies this distinction at launch and during execution. D-20-BOOTSTRAP roadmap r33→34 continues P-642/P-643 and the memory acceptance in `7dce4b07:applications/cod-booter/README.md`, “Compiler memory acceptance”.

## 2026-11-24 — Reserve member names across fields and methods and allow an explicit receiver owner

<a id="h-member-names-and-owner-20261124"></a>
**H-MEMBER-NAMES-AND-OWNER-20261124.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user confirmed “不允许field和method重名，包括结构数据字段” and accepted the proposed parenthesized receiver-owner syntax. D-07 r216→217 adds [member names and explicit receiver owners](design/DomainLanguageDesign.md#receivers-in-field-accessors-and-member-methods) under P-724–P-727. The prior questions are at `e2ad134c:design/DomainLanguageDesign.md`, “Receivers in field accessors and member methods”. The user leaves the choice of a stateful accessor or a named method to program authors under the ordinary language rules.

## 2026-11-24 — Place member receiver const before the receiver type

<a id="h-member-receiver-const-20261124"></a>
**H-MEMBER-RECEIVER-CONST-20261124.** The project user confirmed “`const` 放在 receiver 类型前面，field accessor 和 member method 都一样，省略时 `self` 可写” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-07 r215→216 qualifies [the four receiver forms](design/DomainLanguageDesign.md#receivers-in-field-accessors-and-member-methods) under P-724/P-725. Prior receiver and owner rules are at `92853ab5:design/DomainLanguageDesign.md`, “Functor receivers and callback arguments”, and `92853ab5:design/RegionalOwnership.md`, “Owner-qualified reference arguments”. The decision covers the accessor marker, implicit receiver and target qualifier. Lookup collisions and the spelling that exposes the receiver owner are still open.

## 2026-11-24 — Decide symbol-union eligibility on first demand

<a id="h-seed-symbol-union-admission-20261124"></a>
**H-SEED-SYMBOL-UNION-ADMISSION-20261124.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the project user directed the seed to use precise checking from the outset whenever a later discovery could exceed the proposed union. D-ARCH-REVIEW r119→120 continues P-682/P-683 and clarifies [cache admission](design/ImplementationArchitecture.md#first-demand-union-of-dynamic-symbol-requirements) in `0c873cbf:design/ImplementationArchitecture.md`. This supersedes the proposed late invalidation and caller requeue for this optimization.

## 2026-11-24 — Collect precise gains only when needed

<a id="h-measure-gains-when-needed-20261124"></a>
**H-MEASURE-GAINS-WHEN-NEEDED-20261124.** The project user directed agents to proceed with well-founded improvements and avoid spending delivery time collecting precise gains without a practical use. The [project rule](AGENTS.md#when-to-measure-an-improvement) applies across tasks. D-ARCH-REVIEW r118→119 carries forward the behavioral acceptance for seed proof reuse and symbol snapshots under P-682/P-683 and supersedes routine profiling and paired cold builds in `b391f311:design/ImplementationArchitecture.md`. Established memory limits retain their acceptance checks. [Design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50)

## 2026-11-24 — Review the seed’s first-demand union of symbol requirements

<a id="k-seed-first-demand-symbol-union-20261124"></a>
**K-SEED-FIRST-DEMAND-SYMBOL-UNION-20261124.** D-ARCH-REVIEW r117→118 continues P-682/P-683. The project user clarified in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) that the seed should compute the union when it first touches a procedure. The [review](design/ImplementationArchitecture.md#first-demand-union-of-dynamic-symbol-requirements) separates that opportunity from reusing an already validated body. At `6a054526:cod/lang_checked.hh`, `merge_effects` reports one change flag for several kinds of evidence and `close_effects` uses it to reschedule callers. Acceptance measures which actual caller rechecks a closed symbolic bound can eliminate.

## 2026-11-24 — Review seed bootstrap time in dynamic-symbol checking

<a id="k-seed-bootstrap-symbol-checks-20261124"></a>
**K-SEED-BOOTSTRAP-SYMBOL-CHECKS-20261124.** D-ARCH-REVIEW r116→117 continues P-682/P-683. The project user prioritized reducing bootstrap time in the [side discussion](codex://threads/01a210a7-c445-74e2-8dce-eeb3ddc65a21). The [review](design/ImplementationArchitecture.md#reducing-seed-bootstrap-time-in-dynamic-symbol-checks) clears instrumentation, bounded reuse of completed proof and independent sharing of checker scope snapshots at `0167b89a:cod/lang_checked.hh`. The prior completed-path experiment on a fixed compiler input saved 940 seconds and increased peak RSS by 353,716 KiB. Its timing does not establish a benefit for either new proposal.

## 2026-11-24 — Use one seed mode to build the booter at milestones

<a id="k-booter-one-build-20261124"></a>
**K-BOOTER-ONE-BUILD-20261124.** Following the project user’s question about identical booter IR in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50), the [build rule](AGENTS.md#build-and-command-execution) supersedes the automatic milestone comparison in [H-BOOTER-BUILD-CADENCE](#h-booter-build-cadence-20261124). In the two builds of `d007ba5c:applications/cod-booter`, all 1,939 emitted CoD methods used their single native body. The same seed produced four bodies for `relay` in default mode and one in shared mode on `b6464e46:tests/cod/language/numeric_dispatch_staging_tests.py`. Both executions produced identical output.

## 2026-11-24 — Review the native package probe’s array and value arguments

<a id="k-package-probe-value-arguments-20261124"></a>
**K-PACKAGE-PROBE-VALUE-ARGUMENTS-20261124.** D-ARCH-REVIEW r115→116 and D-20-MODULE-GRAPH r83→84 continue P-680–P-683. The [source-checking review](applications/cod-booter/docs/ModuleGraph.md#checking-the-native-package-probe) covers the call at `b6464e46:applications/cod-booter/src/PackageModuleResolution.cod:38`. The released specialized booter rejects the isolated call in declaration checking and provider generation. Replacing just the call with the scalar `start` passes both commands. The two array loans and four value inputs follow the existing native call and H-261 widening rules.

## 2026-11-24 — Focus routine booter builds on one seed mode

<a id="h-booter-build-cadence-20261124"></a>
**H-BOOTER-BUILD-CADENCE-20261124.** The project user proposed concentrating routine builds on one fast mode and comparing both at major milestones in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). The [build rule](AGENTS.md#build-and-command-execution) selects the default specialized mode. At `d007ba5c:applications/cod-booter`, full specialized and shared builds took 57 min 3 s and 59 min 19 s. Their provider files, LLVM IR and linked compiler matched byte for byte. A mode-specific issue gets an immediate focused comparison.

## 2026-11-24 — Review the package resolver’s indexed row fields

<a id="k-package-row-field-read-20261124"></a>
**K-PACKAGE-ROW-FIELD-READ-20261124.** D-ARCH-REVIEW r114→115 and D-20-MODULE-GRAPH r82→83, continued under P-680–P-683. The [row-field review](applications/cod-booter/docs/ModuleGraph.md#reading-a-field-of-an-indexed-package-row) combines the [array loan](#k-package-classifier-source-call-20261124) and [inline field](#k-booter-nested-product-loan-20261124) checks for `2060a40a:applications/cod-booter/src/PackageModuleResolution.cod:33`. With the classifier call and result writes removed in a private control, both released build modes reject the first row projection through declaration checking and provider generation. Acceptance requires the unchanged resolver once its earlier prerequisites are available.

## 2026-11-24 — Review source checking the package path classifier

<a id="k-package-classifier-source-call-20261124"></a>
**K-PACKAGE-CLASSIFIER-SOURCE-CALL-20261124.** D-ARCH-REVIEW r113→114 and D-20-MODULE-GRAPH r81→82, continued under P-680–P-683. The [classifier review](applications/cod-booter/docs/ModuleGraph.md#checking-the-package-path-classifier) applies ordinary loan, nominal result and completion checks to `2060a40a:applications/cod-booter/src/ModulePaths.cod:12`. Both released booter modes reject a demanded private guard with the imported classifier and accept the control that omits the call. A direct byte-read control exposes the producer-side prerequisite. The [indexed sum write](#k-booter-local-sum-transfer-20261124) remains a separate check.

## 2026-11-24 — Verify booter diagnostic traversal and peak memory on the compiler entry

<a id="k-booter-entry-world-release-20261124"></a>
**K-BOOTER-ENTRY-WORLD-RELEASE-20261124.** D-20-BOOTSTRAP r32→33 and D-20-WORLD-DIAGNOSTICS r30, continued under P-642/P-643 and P-713/P-714. Both builds at `2bc0e290:applications/cod-booter/src/ModuleGraph.cod` passed the [fixture memory budgets](applications/cod-booter/README.md#compiler-memory-acceptance), 22 workloads and 38 declaration/provider comparisons. Both report the five [entry-world acceptance findings](applications/cod-booter/README.md#diagnostic-coverage-of-the-entry-modules-fused-world) and distinguish exact from incomplete coverage at the cap. On the actual compiler entry, the three independent source-command findings retain 501, one and one import notes respectively. This release covers failing self-input through all 63 files. Successful self-compilation remains open.

## 2026-11-24 — Review local sum construction in the package resolver

<a id="k-booter-local-sum-transfer-20261124"></a>
**K-BOOTER-LOCAL-SUM-TRANSFER-20261124.** D-ARCH-REVIEW r112→113 and D-20-MODULE-GRAPH r80→81, continued under P-680–P-683. The [source-checking review](applications/cod-booter/docs/ModuleGraph.md#checking-a-local-sum-constructor-in-an-indexed-transfer) applies the [named constructor decision](#h-dctor-source-20261118) and D-22-TRANSFER to `acd9e453:applications/cod-booter/src/PackageModuleResolution.cod:21`. The linked source commands reject the local sum constructor at name lookup and accept the corresponding record control. Implementation and executable lowering have separate evidence requirements.

## 2026-11-24 — Review one failed dependency shared by grouped selectors

<a id="k-grouped-dependency-failure-20261124"></a>
**K-GROUPED-DEPENDENCY-FAILURE-20261124.** D-ARCH-REVIEW r111→112, continued under D-20-WORLD-DIAGNOSTICS and P-682/P-683. The [grouped-path review](design/ImplementationArchitecture.md#a-failed-path-in-a-grouped-import) applies the [entry-world coverage decision](#h-entry-world-diagnostics-20261124) to a single written dependency shared by multiple selectors. The path node is created in `acd9e453:applications/cod-booter/src/ModuleParser.cod` by `parse_selection_items`. The linked candidate produced two identical parent notes and, at limit one, reported incomplete coverage with only the second blocked selector remaining. This record clears the correction and its acceptance cases.

## 2026-11-24 — Measure linked booter peak memory on release workloads

<a id="k-booter-memory-baseline-20261124"></a>
**K-BOOTER-MEMORY-BASELINE-20261124.** D-20-BOOTSTRAP roadmap r31→32, continued under P-642/P-643 and the [user’s memory decision](#h-bootstrap-compiler-memory-20261124). The [per-command budgets](applications/cod-booter/README.md#compiler-memory-acceptance) use the existing released application at `acd9e453:applications/cod-booter`, linked image SHA-256 `dff984aaf6d46363d07aa0d8e38794aa0843bdde7ccc3cf814831783ff282d02`. The 49-module nested graph peaked at 43.8/44.7/55.7 MiB for checking declarations, emitting a provider and emitting an object, respectively. The 512-function file peaked at 58.9/57.9/76.7 MiB. A damaged last child in a 49-file graph retained diagnostics and previous outputs. The compiler entry opened seven of 63 reachable files on its source commands and four on its object command before rejection. Successful full self-compilation remains unmeasured.

## 2026-11-24 — Qualify bootstrap compiler memory acceptance

<a id="h-bootstrap-compiler-memory-20261124"></a>
**H-BOOTSTRAP-COMPILER-MEMORY-20261124.** The project user in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) accepts a higher seed footprint if replacing `std::variant` would materially harm readability and maintainability, requires bounded concurrency, and states “务必保证 booter 的内存消耗合理”. [Roadmap r31](applications/cod-booter/README.md#compiler-memory-acceptance) extends D-20-BOOTSTRAP under P-642/P-643. This qualifies the earlier [seed RAM priority](#k-seed-drained-path-memory-20261124). Mainline measures the linked booter’s own workloads. No universal numeric limit or sum representation was selected. Previous roadmap at `b47abceb:applications/cod-booter/README.md`.

## 2026-11-24 — Review seed memory at checked scope exits

<a id="k-seed-drained-path-memory-20261124"></a>
**K-SEED-DRAINED-PATH-MEMORY-20261124.** D-ARCH-REVIEW r110→111 continues [normal completion](#h-normal-completion-20261124) and [terminal diagnostic validation](#k-seed-terminal-body-coverage-20261124) under P-682/P-683. The project user gave seed RAM reduction priority over new full-booter-dependent work. The [checker review](design/ImplementationArchitecture.md#consuming-completed-paths-in-the-seed-checker) covers reuse of a completed path’s state after its last consumer. The existing drain is at `2bde4ff6065af3db07ccd4edb2a7839f62517443:cod/lang_checked.hh:6145`. The memory comparison uses the same complete compiler input on both revisions.

## 2026-11-24 — Review final validation of independent checked throws

<a id="k-seed-terminal-body-coverage-20261124"></a>
**K-SEED-TERMINAL-BODY-COVERAGE-20261124.** D-ARCH-REVIEW r109→110 continues [entry-world coverage](#h-entry-world-diagnostics-20261124) under P-682/P-683. The [review](design/ImplementationArchitecture.md#reporting-uncovered-checked-exceptions-from-independent-bodies) clears independent final-replay findings for uncovered concrete checked exceptions. A failed method supplies no checked body to its caller. The seed’s final replay begins at `bd77ebf419276a46336f3045948f4e33c1f6c98d:cod/lang_checked.hh:24551`. That revision ships the [independent layout-cycle correction](#k-seed-inline-layout-cycles-20261124). Earlier return-type failures in effect solving remain open.

## 2026-11-24 — Review independent inline layout diagnostics

<a id="k-seed-inline-layout-cycles-20261124"></a>
**K-SEED-INLINE-LAYOUT-CYCLES-20261124.** D-ARCH-REVIEW r108→109 continues [seed schema recovery](#k-seed-payload-diagnostic-recovery-20261124) under [entry-world coverage](#h-entry-world-diagnostics-20261124) and P-682/P-683. The [seed review](design/ImplementationArchitecture.md#reporting-independent-inline-layout-cycles) preserves a completed field-shape proof across transitive storage failure. The previous residual walk is at `d954a7f12c6ff8ba02fc343106e503ec3cbec9c9:cod/lang_checked.hh:23587`. Callable solver recovery remains separate.

## 2026-11-24 — Review declared void fallthrough in booter objects

<a id="k-booter-declared-void-completion-20261124"></a>
**K-BOOTER-DECLARED-VOID-COMPLETION-20261124.** D-ARCH-REVIEW r107→108 continues [normal completion](#h-normal-completion-20261124) under P-682/P-683. The [booter object review](design/ImplementationArchitecture.md#completing-a-declared-void-procedure-in-the-booter) clears a correction to the shared predicate at `fb27c7ae7374828eb7ab6e82d032d02afe0b799a:applications/cod-booter/src/CompletionChecking.cod:509` for validated void procedures. Omitted-result object signatures and deferred return inspection keep separate proof requirements.

## 2026-11-24 — Review the bootstrap normal-completion paths

<a id="k-normal-completion-review-20261124"></a>
**K-NORMAL-COMPLETION-REVIEW-20261124.** D-ARCH-REVIEW r106→107 applies [the normal-completion decision](#h-normal-completion-20261124) to the [seed invocation exit and the booter empty-pattern reader](design/ImplementationArchitecture.md#entering-cleanup-on-natural-void-completion) under P-682/P-683. Seed inference, checking and emission share the resolved invocation boundary. The staged booter reader retains a located prepublication stop pending checked destination evidence. [Compiler coverage](tests/cod/language/README.md#normal-completion-coverage)

## 2026-11-24 — Natural void completion and unbound return inspection

<a id="h-normal-completion-20261124"></a>
**H-NORMAL-COMPLETION-20261124.** The project user decided in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-24 (America/Los_Angeles) that “`if returning ()` 检查执行到该清理点时，本次调用是否正在正常完成” and “允许具名和匿名 void proc 自然走到过程体末尾”. [D-24r13](design/CallableResultInference.md#results-of-proc-func-and-here-bodies) and [D-17r151](design/ExceptionControl.md#return-state-inspection-and-replacement) supersede the explicit-proc-return and no-payload marker clauses of [H-CALLABLE-TAIL-20261119](#h-callable-tail-20261119) and the prior inspection profile at `6cc53adf6088a58be1436ecf8068ba4ef1d8a71b:design/ExceptionControl.md:1172`. Defer-only placement, typed patterns, value tails, missing required results and cleanup settlement retain their contracts. D-29r24 applies natural exhaustion without another yield. Guide r9 and formatter guideline r4 preserve return destinations and tail results. P-324/P-482–P-484/P-631–P-632/P-723 record the scoped relationships. Compiler coverage is tracked separately.

## 2026-11-24 — Frame effect composition and focused computation as language questions

<a id="k-composition-presentation-20261124"></a>
**K-COMPOSITION-PRESENTATION-20261124.** The [paper](papers/MarginalCostOfComposition.md) states its conditional bound on authored wiring and its Store reference semantics. The [applications](papers/examples/ComposedTileReport.md#application-end-in-cod-and-haskell) name both staging policies and show ordinary void procedures completing naturally under the user's clarified rule. The [engineering checks](papers/examples/ComposedTileReport.md#engineering-checks) compare the same observations separately. Previous text: `da12f768a12bd6dec6794820273feda0d3d9fc21:papers/examples/ComposedTileReport.md`.

## 2026-11-24 — Recover independent seed schema payload checks

<a id="k-seed-payload-diagnostic-recovery-20261124"></a>
**K-SEED-PAYLOAD-DIAGNOSTIC-RECOVERY-20261124.** D-ARCH-REVIEW r105→106 continues [the product registration review](#k-seed-product-diagnostic-recovery-20261124) and [entry-world coverage](#h-entry-world-diagnostics-20261124) under P-682/P-683. The [review](design/ImplementationArchitecture.md#recovering-independent-schema-payload-checks) covers independent product and sum fields and named-array index and element checks. The registrar and nominal layout walk are at `41c4f2232a29fb52398e09e4b8217070501207d8:cod/lang_checked.hh:23235`. Mainline owns implementation and compatibility verification.

## 2026-11-24 — Distinguish function values and functor values

<a id="h-functor-value-distinction-20261124"></a>
**H-FUNCTOR-VALUE-DISTINCTION-20261124.** The project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-24 (America/Los_Angeles) “`func ... in O` 叫作 functor value，`func ...` 叫作 function value，两者不能交叉赋值” and emphasized “术语要明确区分”. [D-18-FUNCTOR-VALUE](design/CallableStorageAndHereAccess.md#function-value-and-functor-value), P-721/P-722, and [D-10r262](design/SymbolicDynamicScope.md#installing-a-functor-value) cover owner-qualified reception and installation. This extends the stored-value inventory of [H-CALLABLE-MODEL-20261118](#h-callable-model-20261118) and preserves the unqualified slot of [D-10r261](#h-function-valued-symbol-20261124), previously recorded at `da12f768a12bd6dec6794820273feda0d3d9fc21:design/SymbolicDynamicScope.md:195`. D-18r135, D-15r79, guide r8 and the repository terminology rule apply the distinction. Compiler implementation has separate acceptance.

## 2026-11-24 — Compare ordinary tile operations over scoped state

<a id="k-tile-scoped-state-and-traversal-20261124"></a>
**K-TILE-SCOPED-STATE-AND-TRAVERSAL-20261124.** The [tile study](papers/examples/ComposedTileReport.md#cod) implements the eight business operations as ordinary CoD procedures over scoped reference bindings. Its [case traversal](papers/examples/ComposedTileReport.md#cod-installation-and-check) matches the two inner Haskell `forM_` loops. The [paper](papers/MarginalCostOfComposition.md#compare-the-operations-the-inspector-can-request) distinguishes state selection from replacing an operation. The prior comparison is at `154e2634ac0870f40756999c7b70e854d4e96adc:papers/examples/ComposedTileReport.md`.

## 2026-11-24 — Retain the defining symbol name on a binder collision

<a id="k-booter-protected-symbol-diagnostic-20261124"></a>
**K-BOOTER-PROTECTED-SYMBOL-DIAGNOSTIC-20261124.** D-ARCH-REVIEW r104→105 continues [the declaration check](#k-runtime-symbol-declaration-check-20261124) under [H-DIAGNOSTIC-NAMES-20261122](#h-diagnostic-names-20261122). The [review](design/ImplementationArchitecture.md#reporting-the-symbol-behind-a-conflicting-binder) covers origin-preserving symbol publication and names on an invocation-owned finding under P-682/P-683. The existing export representation is at `0d7cd74f2d7ea08dfee5aa1119d60748d23cb11c:applications/cod-booter/src/ModuleExportStorage.cod`. Mainline owns compiler integration and linked verification.

## 2026-11-24 — Check an inline field loan at the receiving case arm

<a id="k-booter-arm-local-inline-loan-20261124"></a>
**K-BOOTER-ARM-LOCAL-INLINE-LOAN-20261124.** D-ARCH-REVIEW r104 and D-20-MODULE-GRAPH r80 continue [the readonly product-loan review](#k-booter-nested-product-loan-20261124) for [the two direct arm-local fields](applications/cod-booter/docs/ModuleGraph.md#lending-an-inline-field-directly-from-a-case-receipt). P-680–P-683 govern pre-code review. The source at `7a9ba7b38ac97d390da45ddfbc0f2f6c1b22dfcd:applications/cod-booter/src/ParserStorage.cod:1117` checks both IDs before writing its result. Mainline owns implementation and verification.

## 2026-11-24 — Call a symbol according to its declared callable kind

<a id="h-function-valued-symbol-20261124"></a>
**H-FUNCTION-VALUED-SYMBOL-20261124.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-24 (America/Los_Angeles), the project user accepted the function-symbol proposal, recommended `return @score()` and required direct calls for function-valued symbols. The subsequent clarification “这个保留吧，我撤回 ‘`proc symbol` 要求必须通过 perform/behave 调用’ 这条” expressly retains [H-LEXICAL-SYMBOLIC-PROC-20261123](#h-lexical-symbolic-proc-20261123). [D-10r261](design/SymbolicDynamicScope.md#calling-a-function-valued-symbol), P-719/P-720, covers both function-symbol declaration spellings, scoped installation of an independent function selection and calls by declared kind. D-15r78, guide r7 and source reference r480 apply that distinction. The earlier procedure call table is at `154e2634ac0870f40756999c7b70e854d4e96adc:design/SymbolicDynamicScope.md:601`.

## 2026-11-24 — Show the tile inspector's application code in both languages

<a id="k-tile-application-comparison-20261124"></a>
**K-TILE-APPLICATION-COMPARISON-20261124.** The [paper](papers/MarginalCostOfComposition.md#compare-the-operations-the-inspector-can-request) shows the CoD, `mtl` and labeled `effectful` expressions that compose retry and staging. The [worked example](papers/examples/ComposedTileReport.md#application-end-in-cod-and-haskell) checks the same twelve inputs and nine observations with each assembly. The previous presentation is at `bc720b26c06d4389f16cad7eb4f1975f094b81f1:papers/examples/ComposedTileReport.md`.

## 2026-11-24 — Check protected symbol names at declaration

<a id="k-runtime-symbol-declaration-check-20261124"></a>
**K-RUNTIME-SYMBOL-DECLARATION-CHECK-20261124.** D-ARCH-REVIEW r102→103 continues [H-PROJECT-ARCHITECTURE-REVIEW-20261122](#h-project-architecture-review-20261122) under P-682/P-683 for the [runtime identity decision](#h-runtime-symbol-identity-20261124). The [review](design/ImplementationArchitecture.md#protecting-a-declared-symbol-name) covers both frontends’ module and lexical binding checks, including unused declarations and selected spellings. The earlier gates are in `00c971ba7c94685a8dfb8e81620e8c662d4f02fa:cod/lang_checked.hh` (declaration_scope) and `00c971ba7c94685a8dfb8e81620e8c662d4f02fa:applications/cod-booter/src/NameResolution.cod` (declare_name). Mainline owns implementation. Runtime value transport retains its own review.

## 2026-11-24 — Preserve the symbol identity at its bare name during execution

<a id="h-runtime-symbol-identity-20261124"></a>
**H-RUNTIME-SYMBOL-IDENTITY-20261124.** On 2026-11-24 (America/Los_Angeles) the project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “运行时 score 是提供 @score symbol 的 identity 的语义，所以不仅不许shadow，同scope更不许赋予其它值，因为类型不匹配。” [D-10r260](design/SymbolicDynamicScope.md#the-bare-symbol-name-at-runtime), P-718, carries [H-373](#h-373)’s bare symbol identity into runtime use. It supersedes the compilation-only limit associated with [H-114](#h-114) and [H-381](#h-381)’s use-site-only rejection of a shadowing local. The prior text is at lines 710 and 977 of `bc720b26c06d4389f16cad7eb4f1975f094b81f1:design/SymbolicDynamicScope.md`. D-07r215, D-14r136, D-15r77, D-21r35, booter graph r79 and source reference r479 apply the same rule. Dynamic installations continue to shadow bindings. Runtime identity representation and the proposed callable-prefix spellings remain separate.

## 2026-11-24 — Compare named business operations in Haskell and CoD

<a id="k-ports-symbolic-procs-20261124"></a>
**K-PORTS-SYMBOLIC-PROCS-20261124.** The [tile study](papers/examples/ComposedTileReport.md#which-report-is-correct) pairs Haskell `Ports m` over the actual transformer stack and labeled handlers with eight installed CoD symbolic procedures. Only the two provisional writer operations are reinterpreted under staging. A nested rejection tests that an accepted child forwards to its enclosing stage. The [paper](papers/MarginalCostOfComposition.md#compare-the-operations-the-inspector-can-request) distinguishes an operation interface from access to writable storage and includes the installation work on both sides.

## 2026-11-24 — Publish mixed value and reference effects together

<a id="h-mixed-effect-group-20261124"></a>
**H-MIXED-EFFECT-GROUP-20261124.** On 2026-11-24 (America/Los_Angeles) the project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) “同意你的建议，用户需要顺序执行时可以分两条写。” [D-10r259](design/SymbolicDynamicScope.md#mixed-value-and-reference-effects), P-716/P-717, extends the preparation order of [H-285](#h-285) to one directive containing a bits installation and a borrowed reference rebinding. Grouped publication yields 10 in the distinguishing example, separate statements yield 20, and a failing operand leaves all proposed bindings unpublished. This resolves the mixed-form exclusion at `e597150269901e0536a7bcb8c55ebaea55acbac2:design/SymbolicDynamicScope.md:456`. The module name-collision question remains separate.

## 2026-11-24 — Reconcile design exclusions with their confirmed scopes

<a id="k-design-scope-audit-20261124"></a>
**K-DESIGN-SCOPE-AUDIT-20261124.** Editorial continuity from `bc28b3af4d4154bf9fff6788c41d6fbdc9e6396d`: D-10 r257→258 applies the [grouped selector decision](#h-using-symbolic-variable-identity-20261123) to its selection summary. D-22 r70→71 and the source reference r477→478 apply [D-18’s callable kinds](#h-callable-model-20261118) to receiving and handler capture. D-14 r134→135 and D-16 r7→8 distinguish [scalar target initialization](#h-regional-ref-field-initialization-20261123), [nullable target initialization](#h-regional-ptr-field-initialization-20261123) and [finite cyclic placement](#h-placement-init-20261123) from deferred pure knot-tying. D-13 r38→39, D-08’s language notes r220→221 and P-95/P-148 distinguish [explicit interior](#h-482) and [owner-parameter](#h-561) reference symbols from the open recursive-carrier question. Original examples and exclusions are at that revision in `design/ConstructionAndAcquisition.md:415`, `design/SymbolicDynamicScope.md:759`, `design/RegionalOwnership.md:230` and `applications/steady-snn/research/cod-language-scope-cases.json:282`. Each whole-document qualification and existing value relationship retains its scope.

## 2026-11-24 — Model shared read-only access to a large, rarely revised ontology

<a id="k-facsimile-ontology-20261124"></a>
**K-FACSIMILE-ONTOLOGY-20261124.** The [HLVM paper](papers/AlgebraicEffectsIsAllYouNeed.md#shared-ontology-packages-and-dbmr-databases) now uses an edition-level historical facsimile catalog. Independent fulfillment and editorial machines repeatedly query the same published DBMR while stock and proofs change in their own databases. The paper follows the [shared readonly acquisition contract](design/ConstructionAndAcquisition.md#dbmr-modes) and gives separate controls for an ordinary operational change, a rare bibliographic correction and a reader still pinned to the prior image.

## 2026-11-24 — Declare bits value symbols explicitly or by shorthand

<a id="h-bits-value-symbol-spelling-20261124"></a>
**H-BITS-VALUE-SYMBOL-SPELLING-20261124.** On 2026-11-24 (America/Los_Angeles) the project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) that “当解析后的 `T` 是 bits 类型时，`symbol @x:T` 是 `val symbol @x:T` 的简写。” [D-10r257](design/SymbolicDynamicScope.md#declaring-a-bits-value-symbol) and P-715/P-160/P-248 qualify both spellings for the same independent bits storage, reads, capture and restoration. This supersedes the general exclusion at `485451164acaed56231557b17bf85e6889ef3e9a:design/SymbolicDynamicScope.md:194` and the open question in `docs/DesignTopics.md:218` at that revision. [H-482](#h-482) continues to govern its regional reference scope. [Source reference r477](tests/cod/language/README.md#bits-value-symbol-coverage) distinguishes seed and booter evidence for both spellings, a renamed bits type and a writable ref.

<a id="k-bits-value-symbol-check-20261124"></a>
**K-BITS-VALUE-SYMBOL-CHECK-20261124.** [D-ARCH-REVIEW r102](design/ImplementationArchitecture.md#checking-bits-value-symbol-declarations) clears the seed’s existing scalar/enum TLS path and the CoD booter’s paired source reader with checked profile gates. The booter parser and seed parser are at `485451164acaed56231557b17bf85e6889ef3e9a:applications/cod-booter/src/ModuleParser.cod:512` and `485451164acaed56231557b17bf85e6889ef3e9a:cod/lang_syntax.hh:2893`. Wider bits storage and linked booter execution have separate acceptance.

## 2026-11-24 — Expose effect-layer selection and exercise shared ontology data across HLVMs

<a id="k-direct-effects-shared-ontology-20261124"></a>
**K-DIRECT-EFFECTS-SHARED-ONTOLOGY-20261124.** The [tile comparison](papers/examples/ComposedTileReport.md#haskell-with-mtl) now shows selection at each operation in two eight-layer transformer layouts and in a native labeled effect row. The papers use the user's confirmed bits value-symbol abbreviation. The [HLVM study](papers/AlgebraicEffectsIsAllYouNeed.md#shared-ontology-packages-and-dbmr-databases) follows two independently authored machines sharing one CoD ontology package and the same catalog DBMR while retaining their own packages, programs and fact databases. All traces in these papers are derived observations.

## 2026-11-24 — Compare same-typed effect instances and independent domain HLVM authors

<a id="k-paper-eight-roles-domain-hlvms-20261124"></a>
**K-PAPER-EIGHT-ROLES-DOMAIN-HLVMS-20261124.** The project user requested scalar value symbols for simple examples, a concrete three-State/five-Writer stack, and [Software Defined HLVMs](papers/AlgebraicEffectsIsAllYouNeed.md) authored independently by domain application teams for their business users. The [comparison](papers/examples/ComposedTileReport.md) stages two journals and observes all eight roles in CoD, `mtl` and labeled `effectful`. The [single-handler control](papers/examples/ScopedTileAudit.md) uses a scoped value and lexical audit cells. [Design topic 8](docs/DesignTopics.md#8-symbolic-effects-的可组合性与带焦点的上下文计算) records the explicit value-symbol spelling used in the papers. The HLVM study compares AI-assisted graphical business programming with a textual DSL and a graphical workbench on the same authoring and execution requirements.

## 2026-11-24 — Review recovery between plain products in the CoD name resolver

<a id="k-booter-product-name-recovery-20261124"></a>
**K-BOOTER-PRODUCT-NAME-RECOVERY-20261124.** Under [the entry-world decision](#h-entry-world-diagnostics-20261124) and [delegated review](#h-project-architecture-review-20261122), D-ARCH-REVIEW r100→r101 and [module graph r77→r78](applications/cod-booter/docs/ModuleGraph.md#recovering-field-type-names-between-plain-products) clear independent field type-name diagnostics after successful module registration, under P-680–P-683 and P-713/P-714. The booter must also check deferred selected-name kinds at the actual field, preserve the registered module prefix and count each retained finding once. The split lookup and selection checks are at `e0bde669e3e96c710f6cd8bf0b604e6d1046ea7e:applications/cod-booter/src/NameResolution.cod:292`. Broader semantic recovery and release evidence have separate scope.

## 2026-11-24 — Check readonly loans of inline parser fields

<a id="k-booter-nested-product-loan-20261124"></a>
**K-BOOTER-NESTED-PRODUCT-LOAN-20261124.** Under [the delegated review](#h-project-architecture-review-20261122), D-ARCH-REVIEW r99→r100 and [module graph r76→r77](applications/cod-booter/docs/ModuleGraph.md#lending-inline-fields-of-a-readonly-product) clear the source proof for the two distinct `NodeId` fields lent by `checked_assignment_sites`. P-680–P-683 and D-14-REF-ARGUMENT preserve nominal identity, field selection, owner and loan duration under both source commands. The actual caller is at `434f0cfc31ebd9b3207e27d5cb2948befab6d65f:applications/cod-booter/src/ParserStorage.cod:1132`. Object layout and execution have separate acceptance.

## 2026-11-24 — Install a local method family with a shared contract

<a id="h-grouped-local-proc-install-20261124"></a>
**H-GROUPED-LOCAL-PROC-INSTALL-20261124.** On 2026-11-24 (America/Los_Angeles), the project user proposed moving `-> u64 where {const owner S}` to the header and confirmed the resulting design with “满意，按此决定落实。” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). [D-10r256](design/SymbolicDynamicScope.md#installing-a-local-family-with-several-methods) adds a nonempty list of local methods to the [single-method contract](#h-local-proc-handler-body-20261124) at `50febe89cfc61a17647d1f7b5a9502f71ed4d392:design/SymbolicDynamicScope.md`. P-330/P-613/P-614 cover one scoped installation, per-call owner substitution, full symbol coverage and ordinary most-specific dispatch. D-07r214, D-14r134, D-15r76, D-17r150, D-18-HERE-STORAGE r18 and D-21r34 continue their existing module, lifetime, restoration and dispatch rules for this form. The [r99 engineering clearance](#k-local-proc-handler-review-20261124) covers the single-method implementation.

## 2026-11-24 — Make the regional HLVM and observable effect composition the paper subjects

<a id="k-software-defined-hlvm-composition-20261124"></a>
**K-SOFTWARE-DEFINED-HLVM-COMPOSITION-20261124.** The project user identified the central concept of [*Algebraic Effects Is All You Need*](papers/AlgebraicEffectsIsAllYouNeed.md) as a Software Defined HLVM whose instructions are relocatable functor objects. The revision develops the native executor, handler, reopen and cost hypotheses under the [regional platform charter](design/RegionalSemanticPlatformCharter.md), with graph editing as an authoring case. The user also distinguished invocation from composition in the [raster paper](papers/MarginalCostOfComposition.md#composing-retry-and-publication). [Three paper programs](papers/examples/ComposedTileReport.md) now compare the same retry and publication scopes in opposite orders. The traces are derived expectations. Both revisions concern research examples.

## 2026-11-24 — Install a local single-method family when an effect supplies a body

<a id="h-local-proc-handler-body-20261124"></a>
**H-LOCAL-PROC-HANDLER-BODY-20261124.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) the project user specified diagnostics naming `@verb` as a local proc and confirmed “保留短写法，心智模型就是可以省略 `proc` 关键词就好，是同一种语法。 只要是 effect installation 语义带body的，就是新匿名family，而不是扩展已有family”. [D-10r255 / D-15r75](design/SymbolicDynamicScope.md#installing-a-local-procedure-body) supersedes [H-372](#h-372) and the [r244 application of the extension check](#h-review-backlog-duty-20261123) for body-bearing effects under P-330/P-613/P-614. Ordinary extension permissions, name-only family activation, contract coverage, owner validity and restoration remain in force. The previous contract is at `e803e4cc6e5020c502c80c5e7c31fd5133ecb9ec:design/SymbolicDynamicScope.md`.

<a id="k-local-proc-handler-review-20261124"></a>
**K-LOCAL-PROC-HANDLER-REVIEW-20261124.** [D-ARCH-REVIEW r99](design/ImplementationArchitecture.md#checking-an-anonymous-handler-installation) and [module graph r76](applications/cod-booter/docs/ModuleGraph.md#dependency-handler-activation) clear the shared source form, private method, capture-free seed execution and migration of the existing live graph handler. Multiple possible live-frame layouts require a separate transport proof. The seed registration path is at `e803e4cc6e5020c502c80c5e7c31fd5133ecb9ec:cod/lang_checked.hh`.

## 2026-11-24 — Record the seed’s independent product diagnostics

<a id="k-seed-world-product-status-20261124"></a>
**K-SEED-WORLD-PRODUCT-STATUS-20261124.** [Roadmap r29→r30](applications/cod-booter/README.md#diagnostic-coverage-of-the-entry-modules-fused-world) continues the [entry-world decision](#h-entry-world-diagnostics-20261124) under P-713/P-714. The [reviewed direct product scope](#k-seed-product-diagnostic-recovery-20261124) shipped at `6645621c4fc7ebc1d9e924d6a0fc46543428c1f9:cod/lang_checked.hh`. Both seed modes report all five entry findings, or four with incomplete coverage at a limit of four.

## 2026-11-24 — Test the additive source-extension claim on matched effect programs

<a id="k-additive-effect-sources-20261124"></a>
**K-ADDITIVE-EFFECT-SOURCES-20261124.** At the user's request the [raster paper](papers/MarginalCostOfComposition.md#additive-source-extension) sharpens the M × N to M + N conjecture at `eaadae7073f1d0285ae7ce19a749da189b706413:papers/ComposingEffectsAndFocusedContexts.md`. It bounds authored source for a transparent fragment, identifies independent policy and wiring costs, and compares prior lifting and handler mechanisms. [Matched source](papers/examples/ScopedTileAudit.md) contrasts CoD, mtl and effectful on threshold selection and audited tile failure. These are worked language-design comparisons.

## 2026-11-24 — Record the seed’s delivered module diagnostic coverage

<a id="k-seed-world-loader-status-20261124"></a>
**K-SEED-WORLD-LOADER-STATUS-20261124.** [Roadmap r28→r29](applications/cod-booter/README.md#diagnostic-coverage-of-the-entry-modules-fused-world) continues [D-20-WORLD-DIAGNOSTICS](#h-entry-world-diagnostics-20261124) under P-713/P-714. The seed ships traversal through recoverable module errors and the shared diagnostic cap at `1d9b292e24a76b9dc702fd2c51cb86de985dea72:cod/lang_modules.hh`. The status distinguishes loader findings from independent semantic checks and CoD booter coverage.

## 2026-11-24 — Review recovery between independent product schemas in the seed

<a id="k-seed-product-diagnostic-recovery-20261124"></a>
**K-SEED-PRODUCT-DIAGNOSTIC-RECOVERY-20261124.** [D-ARCH-REVIEW r97→r98](design/ImplementationArchitecture.md#recovering-between-product-declarations-in-the-seed) continues the [entry-world decision](#h-entry-world-diagnostics-20261124) and [collection review](#k-world-diagnostic-collection-20261124) under P-682/P-683. The cleared unit stages direct field checks for one reserved nominal product and tracks unavailable inline layouts separately from cycles. The inspected registration and layout path is at `eaadae7073f1d0285ae7ce19a749da189b706413:cod/lang_checked.hh`.

## 2026-11-24 — Review diagnostic collection across bootstrap module visits

<a id="k-world-diagnostic-collection-20261124"></a>
**K-WORLD-DIAGNOSTIC-COLLECTION-20261124.** [D-ARCH-REVIEW r96→r97](design/ImplementationArchitecture.md#collecting-diagnostics-across-the-entry-world) continues the [entry-world decision](#h-entry-world-diagnostics-20261124) under P-682/P-683 and P-713/P-714. The design review clears loader continuation, invocation-owned findings, the shared cap and terminal gating in both bootstrap compilers. Independent semantic recovery still requires review at its mutable proof boundaries. The prior architecture is at `0504ef3db56140d46800ce30ab595dda5c43771e:design/ImplementationArchitecture.md`.

## 2026-11-24 — Cover the entry module’s fused world in all compilers

<a id="h-entry-world-diagnostics-20261124"></a>
**H-ENTRY-WORLD-DIAGNOSTICS-20261124.** The project user in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) stated “parser是覆盖全文件，compiler要覆盖基于入口模块去准备 fused world 的全范围呀，这个从 seed/booter 应该就开始是这个模式，支持 opt-in 设一个最大 error/warning 数，提前中止”. [Roadmap r28](applications/cod-booter/README.md#diagnostic-coverage-of-the-entry-modules-fused-world) qualifies D-20-WORLD-DIAGNOSTICS under C-06/C-80, P-713/P-714. Graph r75 and architecture r96 apply it to discovery and publication. The invocation-wide combined count is the review’s reading of the requested single maximum. The user clarified that the reviewer should derive such consequences directly. This extends [per-file coverage](#h-whole-file-parser-recovery-20261123) while preserving the existing bootstrap diagnostic jobs. The prior loader gate is in `20871f88ba0ca17cbefd38a43662cf2087bdc339:design/ImplementationArchitecture.md`.

## 2026-11-24 — Study effect composition and focused raster computations

<a id="k-effects-focused-contexts-20261124"></a>
**K-EFFECTS-FOCUSED-CONTEXTS-20261124.** The user asked for a workload that exposes the M × N lifting problem and tests a possible CoD answer to comonadic programming. The [raster paper](papers/MarginalCostOfComposition.md) extends the portfolio at `6a397519f591b0a2d6bde7736545c0b81d667504:papers/README.md`. It derives the small mtl instance matrix, tests independently added flagging and tile handling, and compares scoped CoD reads with a focused Store on a stable image. [Open questions](docs/DesignTopics.md#effect-composition-and-focused-contexts) cover forwarding, reusable refocusing and spatial dependencies. The study adds no language decision or implementation authorization.

## 2026-11-24 — Check a case on a sum received into an arm local

<a id="k-booter-direct-received-sum-20261124"></a>
**K-BOOTER-DIRECT-RECEIVED-SUM-20261124.** The delegated review continues D-ARCH-REVIEW r94→r95 and [D-20-MODULE-GRAPH r73→r74](applications/cod-booter/docs/ModuleGraph.md#matching-a-sum-received-into-an-arm-local) under P-680–P-683 and D-07-CASE. The source checker may select the independently initialized `NodeLink` in `braced_reference_group`, with revision, arm membership, nominal identity and lexical lifetime checked at the use. The workload is `149d2b00afb7c6af9237837b1b8f3daaf8487094:applications/cod-booter/src/ParserStorage.cod:617`. [Prior receipt review](#k-booter-projected-case-helper-20261124)

## 2026-11-24 — Explore CoD language and library choices in the working papers

**K-COD-DESIGN-PAPERS-20261124.** The [four papers](papers/README.md) develop CoD design cases on paper. A regional DCG can itself be the source edited by an AI-assisted CoD program. Replacing its actual owner bars subsequent uses of old views. The pipeline papers explore higher-kinded producer, transformer and consumer signatures for closed traversals and independent chan ends. A [Reflex-style fuel sale](papers/EventEffectPropagation.md#a-reflex-style-network-for-a-fuel-sale) compares timeline-indexed sampling and switching policies. Proposed notation and library contracts remain research choices under the [higher-kinded type synonym topic](docs/DesignTopics.md#higher-kinded-type-synonyms). This revision supersedes the working-paper framing at `5f8937a5dee62ab7458789f9b2af2501f3c86986`.

## 2026-11-24 — Reframe the four CoD working papers

<a id="k-four-working-papers-20261124"></a>
**K-FOUR-WORKING-PAPERS-20261124.** At the project user's request, the [working papers](papers/README.md) revisit the drafts at `c7396294fb9d16e892cbd96599b82bc4c83a42fd:papers/`. The research questions concern correspondence under successive program edits, owner and target identity under relocation, feedback and cleanup during synchronous enumeration, and preservation of routing and progress decisions across asynchronous submission. Each paper gives a competing account or negative control and the observations that distinguish it. The two Python appendices report bounded model comparisons. The other figures are derived examples or planned measurements. As the user requested, compiler observations are version-specific evidence checked against the declared contracts and independent expectations. These are research revisions and confer no language design or implementation clearance.

## 2026-11-24 — Make the bootstrap grammar readable in CoD

<a id="h-booter-readable-grammar-20261124"></a>
**H-BOOTER-READABLE-GRAMMAR-20261124.** On 2026-11-24 (America/Los_Angeles) the project user in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) requested “保留现有递归下降、固定操作符优先级和整文件错误恢复，让维护者能直接从 CoD 源码读出词法拼写、产生式选择和表达式分组规则”. [D-ARCH-REVIEW r93→r94](design/ImplementationArchitecture.md#reading-the-bootstrap-grammar-in-cod) continues P-682/P-683 and [D-20-PARSE-RECOVERY](design/DesignAuthorityRegistry.md#d-20-parse-recovery). It covers a bounded String word matcher, named block-item choice with nested control contexts, and readonly fixed-infix classification. The baseline is `ad43e2666f8c8b013de048ccbbb685883cb25d73:applications/cod-booter/src/ExpressionParser.cod`. The [production parser direction](#h-production-parser-direction-20261124) keeps its separate scope.

## 2026-11-24 — Plan CoD parser combinators for the production compiler

<a id="h-production-parser-direction-20261124"></a>
**H-PRODUCTION-PARSER-DIRECTION-20261124.** In the CoD project discussion the project user proposed “类 Megaparsec 技术路径留给规划中的 e-wrks/cod 编译器去推进设计开发更合适，等完成以后再视情况决定是否 backport 回 booter” and confirmed the proposed division with “同意，未来规划你落repo文档”. The [production parser plan](docs/DesignTopics.md#production-parser-combinators) places library and grammar development in `e-wrks/cod` and leaves the backport decision until that implementation is complete. Typed dynamically scoped effects should make the sequential grammar and its value dependencies clear in CoD. The library API, state representation and operator visibility rules remain open. Registry r1178→r1179 carries forward [repository roles](design/DesignAuthorityRegistry.md#d-20-repositories), [bootstrap operator scanning](design/DesignAuthorityRegistry.md#d-10-operator-runs) and [whole-file recovery](design/DesignAuthorityRegistry.md#d-20-parse-recovery) from `6748793ad600df30234b88a032f7eba11530620c:docs/DesignTopics.md` §4 and the registry at that revision.

## 2026-11-24 — Review booter module-root checking and borrowed source signatures

<a id="k-booter-module-root-and-borrowed-signatures-20261124"></a>
**K-BOOTER-MODULE-ROOT-AND-BORROWED-SIGNATURES-20261124.** [D-20-MODULE-GRAPH r72→r73](applications/cod-booter/docs/ModuleGraph.md#checking-module-initialization-and-publishing-borrowed-scalar-formals) and D-ARCH-REVIEW r92→r93 continue P-680–P-683, the [declared-failure check](#k-booter-imported-checked-call-20261123) and ordinary owner substitution. The six regressed inputs include `4d6f1d35fd14832566f52fdcd5cf790d626be093:applications/cod-booter/workloads/modules/fallible-count.cod`. The source repair covers module-level demand, private transfer policies and public scalar loans. Object lowering retains separate acceptance.

## 2026-11-24 — Open a design topic for higher-kinded type synonyms

<a id="k-higher-kinded-type-synonym-topic-20261124"></a>
**K-HIGHER-KINDED-TYPE-SYNONYM-TOPIC-20261124.** In the current CoD project conversation the project user requested a separate [higher-kinded type synonym topic](docs/DesignTopics.md#higher-kinded-type-synonyms) grounded in the research planned in `papers/`. The topic gives a candidate constructor argument, an invalid application and observable cases from closed traversals and open event pipelines. Its type rules and declarations await discussion. The [named-signature discussion order](#h-named-signatures-before-pipeline-protocols-20261123) and [independent endpoint prerequisite](#h-actor-region-transport-and-chan-ends-20261124) continue from `52f74d7ae302cb1d3244c20cd5149c55e850417c:docs/DesignTopics.md`. Registry r1177 links D-31 and D-32-CHAN-ENDS to the topic.

## 2026-11-24 — Distinguish dependency initialization from installation cancellation in seed fixtures

<a id="k-module-context-fixture-scope-20261124"></a>
**K-MODULE-CONTEXT-FIXTURE-SCOPE-20261124.** [D-ARCH-REVIEW r91→r92](design/ImplementationArchitecture.md#module-context-and-import-fixtures) continues P-682/P-683 under [H-379](#h-379) and the [procedure installation contract](#h-effectful-proc-install-20261118). The seven fixture migrations preserve the applicable module identity, scope and query checks. The two ordinary dependency inputs cover initialization order and failure. The obsolete RHS cases are at `9b7ac52277c553e8c1c45e6839271ea47e2c47d4:tests/cod/language/module_context_tests.py`.

## 2026-11-24 — Plan atomic region transport and independent chan endpoints

<a id="h-actor-region-transport-and-chan-ends-20261124"></a>
**H-ACTOR-REGION-TRANSPORT-AND-CHAN-ENDS-20261124.** The project user specified both planning scopes in the current CoD project conversation on 2026-11-24 (America/Los_Angeles). Their words are reproduced below. An exact message timestamp and durable conversation URL were unavailable.

> 规划actor间通信，使用 udp 基础上的 akcp （[https://github.com/daowei2026/vnp](https://github.com/daowei2026/vnp)）流（允许漏洞）实现半可靠的任意长度region传输，region粒度原子成功或失败，支持队列和背压特性。
>
> chan 肯定要允许分开发送端与接收端独立建模，这个等 CoD 支持了 higher-kinded type synonym 后，通过形式化的 producer/transformer/consumer 类型语言去描述表达。

[D-32-REGION-TRANSPORT](design/ActorChannels.md#regional-transport-over-akcp), r2/P-711, selects the inter-Actor network path, whole-region publication, queues and backpressure. [D-32-CHAN-ENDS](design/ActorChannels.md#independent-chan-endpoints), r2/P-712, selects independent endpoint modeling and the prerequisite for its formal type language. D-10-ACTOR-CHAN/P-702 continues to govern ordinary fire-and-forget and systemic-fault reporting. Admission waits, wire framing, closure, cardinality, fault ownership and type declarations await their respective decisions. D-31's general named-signature discussion and its synchronous composite contract retain their separate scopes.

From the baseline `bc2f01bfe15f7db6423d967726519bbf6cbb6c6a:design/ActorChannels.md`, D-32 r1→2 adds whole-region and pressure acceptance cases using [VNP's AKCP contract](https://github.com/daowei2026/vnp/blob/315634cf92cfb322f91788e295cffefb0c65825b/docs/protocol/akcp-v1.md). D-07 r211→212, D-10 r253→254, D-12 r36→37 and D-13 r37→38 apply those boundaries to packet admission, runtime retention and disposal. D-31 r12→13 records the endpoint workload under the [earlier pipeline sequencing decision](#h-named-signatures-before-pipeline-protocols-20261123). Other scopes are continued from that baseline. The multi-record framing and private assembly approach are proposals for satisfying the decision.

## 2026-11-24 — Defer body-mixin templates

<a id="h-mixin-template-deferral-20261124"></a>
**H-MIXIN-TEMPLATE-DEFERRAL-20261124.** The project user in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-24 (America/Los_Angeles) stated “mixin 是 C++ template 语义，相当于代码模板，mixin 就是抄一遍代码进来” and deferred detailed discussion and implementation, permitting removal of pending tests. [D-10-MIXIN r253](design/SymbolicDynamicScope.md#import-time-alias-translation) continues the receiving-context meaning of H-105/H-115/H-212 under C-43/C-46 and P-102/P-111. Mapping distinct source identities to one destination stays open. [D-ARCH-REVIEW r91](design/ImplementationArchitecture.md#registering-and-importing-an-empty-procedure-family-in-the-seed) supersedes the separate body-mixin fixture requirement at `a8ab84e8cbf10d3e0f1cbdcd37af23819c8201f3:design/ImplementationArchitecture.md`. Ordinary import and its runner checks retain their scope.

## 2026-11-24 — Review the seed registrar and importer for empty procedure families

<a id="k-seed-empty-procedure-import-20261124"></a>
**K-SEED-EMPTY-PROCEDURE-IMPORT-20261124.** [D-ARCH-REVIEW r89→r90](design/ImplementationArchitecture.md#registering-and-importing-an-empty-procedure-family-in-the-seed) continues P-682/P-683 under the [declaration](#h-empty-procedure-family-20261124) and [import](#h-proc-import-identity-20261124) decisions. The registrar and resource binder share program-owned family indices. [D-15r73→r74](design/ModuleWorlds.md#selecting-a-symbolic-procedure-family) uses the seed-supported signed literal `7` in the provider example. The prior `ded2f1590c8336639843d22acdbd3ab6f11fd7ba:tests/cod/language/import-mapping.cod` exercises different-symbol body mapping. Preserve its separate acceptance while verifying same-symbol import, extension and explicit installation through the runner.

## 2026-11-24 — Let a procedure import bind the published symbol

<a id="h-proc-import-identity-20261124"></a>
**H-PROC-IMPORT-IDENTITY-20261124.** On 2026-11-24 (America/Los_Angeles) the project user in the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) specified “`import "./vocabulary.cod" proc @p;` 就够了，不要求写出 `symbol from "./vocabulary.cod" @p;` 当然也允许写出来。” [D-15-DEFAULT-EXPORTS r73 / D-10r252](design/ModuleWorlds.md#selecting-a-symbolic-procedure-family) extends the free-name rule to imports, including [empty families](#h-empty-procedure-family-20261124). Reusing the same identity succeeds and a conflicting one rejects. P-692/P-693 preserve receiver extension and selection rules. Guideline r3 and guide r6 align the contributor example under P-694/P-695. The prior two-directive example is at `b109ea54af33a9f2e2f3603404bb6c4281588b6f:design/ModuleWorlds.md`.

## 2026-11-24 — Establish the procedure family at declaration

<a id="h-empty-procedure-family-20261124"></a>
**H-EMPTY-PROCEDURE-FAMILY-20261124.** The project user in the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-24 (America/Los_Angeles) specified “定义一个 proc 性质的 symbol 时，就创建 empty family，之后的 export/import/using 都接受 empty family，具体情况下的规则不变” and requested the same model for a bodyless `proc` header. [D-10r251 / D-15r72](design/SymbolicDynamicScope.md#procedure-families-before-the-first-method), P-709/P-710, extend [H-257](#h-257), [H-258](#h-258) and [H-374](#h-374) to this declaration point. Signature, extension and installation contracts retain their scopes. At `bd2ebfb02b300d51b40b007dc1485f58ed3cfdf0:cod/lang_checked.hh`, `publish_module` can still publish a procedure symbol without its family.

## 2026-11-24 — Review the booter’s nested case and its two helper loans

<a id="k-booter-projected-case-helper-20261124"></a>
**K-BOOTER-PROJECTED-CASE-HELPER-20261124.** [D-20-MODULE-GRAPH r71→r72](applications/cod-booter/docs/ModuleGraph.md#matching-a-sum-field-of-an-arm-local-product) and D-ARCH-REVIEW r88→r89 continue [constructor field reception](#k-booter-case-field-source-20261123) and [direct sum loans](#k-booter-direct-sum-subject-20261123) under P-680–P-683. In `6ca1050404b3ca8841599aeb4e4b64b178315875:applications/cod-booter/src/ParserStorage.cod`, `inspect_here_supply` matches an embedded sum and lends two arm-local values to ordinary helpers. Scope covers both declaration commands.

## 2026-11-24 — Review custody through a direct booter source call

<a id="k-booter-direct-holder-flow-20261124"></a>
**K-BOOTER-DIRECT-HOLDER-FLOW-20261124.** [D-20-MODULE-GRAPH r70→r71](applications/cod-booter/docs/ModuleGraph.md#checking-direct-exception-custody-through-a-source-call) and D-ARCH-REVIEW r87→r88 continue [direct holder reception](#k-direct-exception-holder-review-20261123) under P-680–P-683. At `ffdfd94b29d7aef7eab4376c1def3f24a4748917:applications/cod-booter/src/CompletionChecking.cod`, `record_exit` keeps one routed category per site. The declaration and provider proof must distinguish failures before and after call entry even when both reach one handler with the same category.

## 2026-11-23 — Review direct owning exception reception and return

<a id="k-direct-exception-holder-review-20261123"></a>
**K-DIRECT-EXCEPTION-HOLDER-REVIEW-20261123.** [D-ARCH-REVIEW r86→r87](design/ImplementationArchitecture.md#transferring-a-directly-caught-exception-through-an-ordinary-call) continues P-682/P-683 for the direct arrival in [D-17r149](#h-typed-catch-owner-handoff-20261123). The caught-owner cleanup in `055264454f115cfa922e7739f4cbe801f3da5c96:cod/lang_llvm.hh` supplies the seed starting point. Scope covers caller custody until entry, independent normal-result delivery after cleanup, and the booter’s source proof. Distinct wrapper and matched targets retain their separate acceptance.

## 2026-11-23 — Review a directly borrowed sum in the booter case checker

<a id="k-booter-direct-sum-subject-20261123"></a>
**K-BOOTER-DIRECT-SUM-SUBJECT-20261123.** [D-20-MODULE-GRAPH r69→r70](applications/cod-booter/docs/ModuleGraph.md#checking-a-directly-borrowed-sum-subject) and D-ARCH-REVIEW r85→r86 continue [constructor field reception](#k-booter-case-field-source-20261123) under P-680–P-683, [ordinary ref preparation](#h-owner-ref-argument-20261121) and [case matching](#h-case-contract-20261119). At `1db226ea703dddc82a3c4e08c1d2c68beb2b4593:applications/cod-booter/src/ParserStorage.cod`, `inspect_here_receivers` tests a borrowed `NodeLink` and receives a `NodeId`. Scope covers source checking in declarations and providers.

## 2026-11-23 — Select the incoming exception for a compatible owning destination

<a id="h-typed-catch-owner-handoff-20261123"></a>
**H-TYPED-CATCH-OWNER-HANDOFF-20261123.** The project user in the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles) specified “让 `exception own_e:Exception <- e` 时，对方接收到的是 `e.tip`” and an ordinary user helper with an owning `Exception` parameter. The user delegated the incompatible-destination rule. [D-17r149 / P-707–P-708](design/ExceptionControl.md#choosing-the-target-of-an-owning-catch-handoff) admits the compatible matched target as fallback and rejects an uncovered arrival. This revises [K-EXCEPTION-SCENARIOS-20261118](#k-exception-scenarios-20261118) for consuming reception. The readonly projection, checked obligations and single owner retain their contracts. The previous wording is at `912b06ade8d855c8f7903b22fe7b0a19f5982b9c:design/ExceptionControl.md`, “Matched binding and hidden tip”.

## 2026-11-23 — Review scalar object calls through a receiver import

<a id="k-booter-facade-scalar-object-20261123"></a>
**K-BOOTER-FACADE-SCALAR-OBJECT-20261123.** [D-20-MODULE-GRAPH r68→r69](applications/cod-booter/docs/ModuleGraph.md#reusing-a-scalar-method-body-across-a-receiver-import) and D-ARCH-REVIEW r84→r85 continue P-680–P-683 under [H-232](#h-232) and [H-253](#h-253). At `ae7358adee232232137dc46f0f605d95f958f989:applications/cod-booter/src/OperandChecking.cod`, `prepare_imported_calls` finds a defining header and function number. The scalar facade slice also requires the checked receiving interpretation, eligible body and exact ordinal before reuse. Transitive adaptation retains the 2/14 acceptance.

## 2026-11-23 — Review onward publication of expanded imports

<a id="k-booter-facade-publication-20261123"></a>
**K-BOOTER-FACADE-PUBLICATION-20261123.** [D-20-MODULE-GRAPH r67→r68](applications/cod-booter/docs/ModuleGraph.md#publishing-expanded-imports-from-a-facade) and D-ARCH-REVIEW r83→r84 continue P-680–P-683 under [selector substitution](#h-module-default-selector-substitution-20261123). The written-item publisher at `e513d4acdbd22c7b688a925ee6a5b9bed483bffd:applications/cod-booter/src/NativeDeclarations.cod` establishes the comparison path. Expanded re-exports need checked receiver contributions, owner-bounded name comparisons and facade-local default references.

## 2026-11-23 — Review constructor field reception in the booter source checker

<a id="k-booter-case-field-source-20261123"></a>
**K-BOOTER-CASE-FIELD-SOURCE-20261123.** [D-20-MODULE-GRAPH r66→r67](applications/cod-booter/docs/ModuleGraph.md#checking-a-constructor-field-received-by-value) and D-ARCH-REVIEW r82→r83 continue P-680–P-683 under the [case contract](#h-case-contract-20261119) and [coverage rule](#h-case-coverage-20261119). At `831caad4ff65618201f763d1dfcce3f520912518:applications/cod-booter/src/ParserStorage.cod`, `syntax_boundary` requires the active constructor, a finite bits transfer into a fresh local and two ordinary nominal projections. Scope covers declaration and provider checking.

## 2026-11-23 — Review borrowing the graph owner at the ordinary writer call

<a id="k-booter-owning-local-call-loan-20261123"></a>
**K-BOOTER-OWNING-LOCAL-CALL-LOAN-20261123.** [D-20-MODULE-GRAPH r65→r66](applications/cod-booter/docs/ModuleGraph.md#borrowing-an-owning-local-for-an-ordinary-call) and D-ARCH-REVIEW r81→r82 continue P-680–P-683 under [ordinary borrowing of an owning handle](#h-booter-handle-owner-ctype-20261120) and [nominal enum identity](#h-typed-enum-20261121). The object-writer workload at `d89c1e58711ee1328d214da6e444178a62e4ace4:applications/cod-booter/docs/ModuleGraph.md` requires both halves of the enum identity on an inferred local and a live acquisition proof at the direct CoD call.

## 2026-11-23 — Review the graph collection call in the object-writer compatibility input

<a id="k-booter-mixed-native-loans-20261123"></a>
**K-BOOTER-MIXED-NATIVE-LOANS-20261123.** [D-20-MODULE-GRAPH r64→r65](applications/cod-booter/docs/ModuleGraph.md#checking-handles-and-readonly-array-loans-in-one-native-call) and D-ARCH-REVIEW r80→r81 continue P-680–P-683. The [native loan](#h-booter-c-handles-20261120) and [typed enum](#h-typed-enum-20261121) contracts establish per-argument owner and type checks. The source-checking structure at `b55749b6b9fd8d20507264ca6db13f6b548d3f05:applications/cod-booter/docs/ModuleGraph.md` extends to the two array arguments sharing B and the imported enum result. The C transport and graph implementation retain their existing boundaries.

## 2026-11-23 — Review direct named-region checking in the CoD booter

<a id="k-booter-direct-region-check-20261123"></a>
**K-BOOTER-DIRECT-REGION-CHECK-20261123.** [D-20-MODULE-GRAPH r63→r64](applications/cod-booter/docs/ModuleGraph.md#checking-a-direct-named-region-acquisition) and D-ARCH-REVIEW r79→r80 continue P-680–P-683 for the [confirmed ref group](#h-placement-init-20261123) under [H-596–H-599](#h-596). F keeps schema, fresh owner and successful holder evidence distinct. The existing ordered root contract at `faf9d572cfa826a1cf7e470f56afbf7e682ebd3d:design/ConstructionAndAcquisition.md` permits repeated ordinary writes. The current factory supplies thirteen roots as the second source-checking workload.

## 2026-11-23 — Review the object writer’s lexical helper and filename argument

<a id="k-booter-object-writer-helper-and-text-20261123"></a>
**K-BOOTER-OBJECT-WRITER-HELPER-AND-TEXT-20261123.** [D-20-MODULE-GRAPH r62→r63](applications/cod-booter/docs/ModuleGraph.md#checking-the-object-writers-lexical-helper) and D-ARCH-REVIEW r78→r79 continue P-680–P-683. The [callable](#h-callable-model-20261118), [owning effect](#h-native-handle-effect-20261121) and [UTF-8](#h-utf8-native-text-20261121) contracts govern the two source demands. Preserve the helper’s lexical diagnostic owner and nominal return, and the [filename assertion’s completion before native entry](applications/cod-booter/docs/ModuleGraph.md#checking-the-object-filename-argument). The prior source-checking boundary is at `eb8fd0d218c2770f51fa0f15b797631aa8e21443:applications/cod-booter/docs/ModuleGraph.md`.

## 2026-11-23 — Review effect scope in the booter’s session proof

<a id="k-booter-session-installation-scope-20261123"></a>
**K-BOOTER-SESSION-INSTALLATION-SCOPE-20261123.** [D-20-MODULE-GRAPH r61→r62](applications/cod-booter/docs/ModuleGraph.md#tracing-the-installed-session-to-its-operand) and D-ARCH-REVIEW r77→r78 continue P-680–P-683. [H-319](#h-319) and the [owning effect contract](#h-native-handle-effect-20261121) govern the use-site proof for the call reviewed at `142d400ecdbdcb5421fc409842392b6022469f14:applications/cod-booter/docs/ModuleGraph.md`. An ordinary group carries a successful installation forward. A full scope restores the preceding binding on exit. A demanded join must account for every reaching installation.

## 2026-11-23 — Review source checking of two native handle loans

<a id="k-booter-two-handle-source-call-20261123"></a>
**K-BOOTER-TWO-HANDLE-SOURCE-CALL-20261123.** [D-20-MODULE-GRAPH r60→r61](applications/cod-booter/docs/ModuleGraph.md#checking-an-imported-native-operation-with-two-handle-loans) and D-ARCH-REVIEW r76→r77 continue P-680–P-683 under the [native loan](#h-booter-c-handles-20261120) and [owning effect](#h-native-handle-effect-20261121) contracts. This extends the source checker reviewed at `baf4af88b5f33e2776d7faceda90b768d86c7143:applications/cod-booter/docs/ModuleGraph.md` to the imported graph/session operation. Published-interface bounds, both actual-owner substitutions, caught `u32` field evidence and existing provider outputs are acceptance gates.

## 2026-11-23 — Review the booter reader for reference identity

<a id="k-booter-identity-reader-20261123"></a>
**K-BOOTER-IDENTITY-READER-20261123.** [D-ARCH-REVIEW r75→r76](design/ImplementationArchitecture.md#reading-reference-identity-in-the-booter), P-682/P-683. The [confirmed ref group](#h-placement-init-20261123) uses [H-439](#h-439), [H-440](#h-440) and [H-441](#h-441) for its reciprocal identity observations. Preserve the equality precedence, two-word operator and owner-qualified null from `3eede442a282f5976b0714ab18c4e4f90970928b:design/RegionalAndLexicalStorageRefactor.md`. Acceptance covers both operand links and the located checking boundary.

## 2026-11-23 — Review the Boolean assertion in the regional constructor

<a id="k-constructor-assert-review-20261123"></a>
**K-CONSTRUCTOR-ASSERT-REVIEW-20261123.** [D-ARCH-REVIEW r74→r75](design/ImplementationArchitecture.md#checking-a-constructor-predicate-assertion), P-682/P-683. The [confirmed ref group](#h-placement-init-20261123) exercises the predicate syntax, static resolution and unchecked failure named in [H-469](#h-469), [H-470](#h-470) and [H-472](#h-472). Both source readers retain the predicate and optional literal. Linked acceptance preserves ordinary operand effects and constructor cleanup. The broader D-10-ASSERT qualification at `635847177549436aecd7196b5c2b1b8964cdc1ad:design/DesignAuthorityRegistry.md` stays pending.

## 2026-11-23 — Audit registry navigation and historical evidence

<a id="h-hygiene-coordination-20261123"></a>
**H-HYGIENE-COORDINATION-20261123.** The project user requested coordination with the affected Codex chats before this hygiene pass took action. Recorded Nov 23, America/Los_Angeles. [Hygiene chat](codex://threads/01a1d485-00a9-7022-96a1-6d7b5faecfd5)

<a id="k-hygiene-index-20261123"></a>
**K-HYGIENE-INDEX-20261123.** D-GOV r1153→r1154 continues the H-05/H-388/H-391/H-411/H-412 authority under charter §§3.3/8/11. The scope index, relationship register and external review queue each have one location. D-22 retains its current semantic revision with a corrected link to the native output contract. Source E preserves the Nov 13–22 changelog entries. The local archive tag `archive/local-dispatch-exploration` retains the experiment cited at `25ccc75121140d5b901a5a9d590c47f1bb2b3cd8:experiments/local-dispatch/`.

## 2026-11-23 — Review borrowed-handle replacement and its publication check

<a id="k-lexical-handle-rebind-check-20261123"></a>
**K-LEXICAL-HANDLE-REBIND-CHECK-20261123.** [D-ARCH-REVIEW r73→r74](design/ImplementationArchitecture.md#checking-rebinding-of-a-borrowed-handle-local), P-682/P-683, clears seed checking and pair installation plus the CoD booter’s source gate under the [same-owner rebinding decision](#h-native-handle-local-rebinding-20261123). The two-session negative distinguishes actual owner from CType at `566c16c9:applications/cod-booter/docs/LLVMInterop.md`, “Borrowed handle locals and results”. Native booter object emission retains separate review.

## 2026-11-23 — Review frontend support for in-place constructor calls

<a id="k-placement-init-frontends-20261123"></a>
**K-PLACEMENT-INIT-FRONTENDS-20261123.** [D-ARCH-REVIEW r72→r73](design/ImplementationArchitecture.md#checking-in-place-construction-of-a-regional-cycle), P-682/P-683, clears both source readers and the direct regional seed workload under [D-22-PLACEMENT-INIT](#h-placement-init-20261123). At `1afc9471b42709e70cab774bdf2c8338c4c770a8:design/ConstructionAndAcquisition.md` the two-node ref group supplies the acceptance source. Keep selected target, constructor completion and readable component distinct through operand effects and failure. CoD booter checked-state and backend additions retain separate structural review.

## 2026-11-23 — Initialize a reserved target in a ref group

<a id="h-placement-init-20261123"></a>
**H-PLACEMENT-INIT-20261123.** On 2026-11-23 (America/Los_Angeles) the project user confirmed explicit reservation followed by in-place constructor invocation in the [design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50): “只能传递给明确 `not init ref` 的参数；分配失败后续 side effects 都因不执行而不生效。” [D-22-PLACEMENT-INIT](design/ConstructionAndAcquisition.md#in-place-construction-in-a-ref-group), D-22r70/D-07r211, P-705/P-706, selects `init a(args)` on the reserved identity, restricted ctor peer access, readiness of the linked pair and ordinary ordered failure/cleanup. This resolves the deferred in-place question at `24be1843fdd364393881ca604a2f7cb563540ea8:design/DomainLanguageDesign.md`, “Sum object construction and reference storage”, under [H-562](#h-562), [constructor selection](#h-dctor-source-20261118) and [H-428](#h-428). Compiler structure, ABI and live-object replacement are outside the confirmation.

## 2026-11-23 — Review the seed guard in the opening-bid workload

<a id="k-seed-effect-only-guard-20261123"></a>
**K-SEED-EFFECT-ONLY-GUARD-20261123.** [D-ARCH-REVIEW r71→r72](design/ImplementationArchitecture.md#braced-control-only-guards-in-the-seed), P-682/P-683, applies the [ordinary guard](#h-guards-and-loop-discard-20261121) and [semicolon](#h-idiomatic-semicolons-20261122) decisions to the non-final braced guard at `a1e72853f0d70e6dd08be66d8f104e2eeae4494c:design/SymbolicDynamicScope.md`, “Indexed by-value elements”. The scalar control also reaches the conditional join before the fix. Verify token-based delimiting, empty-alternative equivalence, discarded transfer and normal or exceptional effects in both seed modes.

## 2026-11-23 — Apply ordinary target selection to variadic elements

<a id="k-variadic-element-selection-20261123"></a>
**K-VARIADIC-ELEMENT-SELECTION-20261123.** [D-10-VARARGS r249→r250](design/SymbolicDynamicScope.md#indexed-borrowed-elements), P-403/P-404, applies H-272/H-449 and [ordered expansion](#h-ordered-pack-expansions-20261123) to repeated targets, source rebinding, indexing and forwarding. The previous open indexing label is at `03e596004dc7e8179e8aba30b35530b024b3b3db:design/SymbolicDynamicScope.md`. [DesignTopics](docs/DesignTopics.md) narrows the ownership and construction questions to reflect the existing same-owner handoff and active-variant rules. The three deferred language topics keep their existing scope.

## 2026-11-23 — Review private preparation and cleanup of seed value packs

<a id="k-seed-value-pack-20261123"></a>
**K-SEED-VALUE-PACK-20261123.** [D-ARCH-REVIEW r71](design/ImplementationArchitecture.md#preparing-a-by-value-variadic-call-in-the-seed), P-682/P-683, clears direct concrete `u64` value packs under the [ordered expansion](#h-ordered-pack-expansions-20261123) and [ordinary forwarding](#k-value-pack-forwarding-20261123) rules. The seed gate at `933241fbffe9653c4947d0aad157e8d457cf3935:cod/lang_checked.hh` rejects both opening-bid workloads. The scope covers one checked operand plan, call-duration private storage and validation of three independent values and exact cleanup. CoD-written booter execution, general pack storage and C ABI retain separate scope.

## 2026-11-23 — Clarify the named signature prerequisite

<a id="k-parameterized-named-signature-examples-20261123"></a>
**K-PARAMETERIZED-NAMED-SIGNATURE-EXAMPLES-20261123.** The project user identified `Producer<A>`, `Consumer<B>` and `Transformer<A, B>` as examples of the [named signatures prerequisite](#h-named-signatures-before-pipeline-protocols-20261123). [DesignTopics](docs/DesignTopics.md#named-signatures-and-pipelines) clarifies that use of type parameters in the deferred topic recorded at `f0ae9a40fafd9d9f9f5d91d9ae2e3fb393bddac4:docs/DesignTopics.md`. Declaration syntax and type rules remain open. [Design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50)

## 2026-11-23 — Revisit pipeline protocols after named type signatures

<a id="h-named-signatures-before-pipeline-protocols-20261123"></a>
**H-NAMED-SIGNATURES-BEFORE-PIPELINE-PROTOCOLS-20261123.** On 2026-11-23 (America/Los_Angeles) the project user deferred general producer, consumer and transformer design until CoD has named type signatures, directing “放到 DesignTopics去推后讨论”. [D-31r12](design/ProducersAndTransformers.md#design-authority) records the discussion order and leaves the general signatures and nested call semantics open in [DesignTopics](docs/DesignTopics.md#named-signatures-and-pipelines). The explicit composite workload retains its [r11 qualification](#h-pipeline-composite-contract-20261119) at `852c0f8622dce07b3b84e49ba5fc22c77f2f6913:design/ProducersAndTransformers.md`. [Design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50)

## 2026-11-23 — Clarify disposal in the first native provider profile

<a id="k-booter-disposal-profile-20261123"></a>
**K-BOOTER-DISPOSAL-PROFILE-20261123.** [D-20-C-HANDLES, interop r31→r32](applications/cod-booter/docs/LLVMInterop.md#native-handles-and-c-records), P-646/P-647, continues the [first LLVM lifecycle](#h-booter-llvm-handles-20261120) and [host declaration scope](#h-booter-c-declarations-20261120). The overview now identifies the existing nonthrowing hook and explicit failure reporting specified at `5cfc4f3a629a3937f85c1fd9032b23e8d103a6c0:applications/cod-booter/docs/LLVMInterop.md`, “Declarations and the first host call”. The general policy remains open.

## 2026-11-23 — Drain one member at each handoff and release on overwrite

<a id="h-drain-handoff-20261123"></a>
**H-DRAIN-HANDOFF-20261123.** On 2026-11-23 (America/Los_Angeles), the project user confirmed that a consuming traversal leaves the unvisited suffix in the collection on early exit. For an owning loop control the user specified “下一个迭代覆盖赋值时释放旧 own 对象” and release at whole-loop exit. [D-30r6](design/CollectionsAndBuiltinContainers.md#removal-order-and-failure) and [D-29-HEADERS r23](design/RangedForAndGenerators.md#owning-loop-controls) record that scope under P-589/P-590 and P-540–P-542. The broader interface and owning-yield signatures remain open. Prior draft at `8e7f7af37dcee53c3b9bc0730796321fe6533f70:design/CollectionsAndBuiltinContainers.md`, “Removal, order and failure”. [Design chat](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50)

## 2026-11-23 — Check the diagnostic helper's independent u32 inputs

<a id="k-booter-checked-u32-helper-20261123"></a>
**K-BOOTER-CHECKED-U32-HELPER-20261123.** [D-20-MODULE-GRAPH r60](applications/cod-booter/docs/ModuleGraph.md#checking-u32-in-the-diagnostic-helper) and D-ARCH-REVIEW r70, P-680–P-683, continue the [imported helper review](#k-booter-source-handle-helper-20261123). The helper at `5a0c93f9b94c9d3714e65169b280f342dcf7f224:applications/cod-booter/src/NativeDiagnostics.cod` takes an exact u32 and receives a backend u8 under H-261. The clearance covers source checking and published source facts for an independent u32 and preserves separate executable and C transport admission.

## 2026-11-23 — Apply ordinary argument preparation when forwarding a value pack

<a id="k-value-pack-forwarding-20261123"></a>
**K-VALUE-PACK-FORWARDING-20261123.** [D-10-VARARGS r249](design/SymbolicDynamicScope.md#indexed-by-value-elements), P-403/P-404, continues [callee-local value semantics](#k-variadic-local-values-20261123) and [ordered expansion](#h-ordered-pack-expansions-20261123). The second call observes writes made before forwarding and receives independent storage. At `b0b8648a4e99a593b77e3f277b3c16c9031a0078:design/SymbolicDynamicScope.md` the open-scope wording still included pack forwarding generally. Ordinary `len` lookup follows H-ORDINARY-NAMES-20261119. Wider pack storage and collection traversal retain their own contracts.

## 2026-11-23 — Review seed placement for products with regional scalar fields

<a id="k-seed-regional-scalar-product-20261123"></a>
**K-SEED-REGIONAL-SCALAR-PRODUCT-20261123.** [D-ARCH-REVIEW r69](design/ImplementationArchitecture.md#constructing-a-regional-product-with-scalar-edges), P-682/P-683, continues the delegated architecture review for D-22-REGIONAL-REF-INIT. At `e2e52a47162e658f70641c9159d5093638a32bc9:cod/lang_checked.hh`, both seed modes stop at the enclosing explicit regional placement. The reviewed increment shares field classification, preserves owner-relative selections and applies H-416 reservation order before each fresh scalar's RHS. The two-mode workload distinguishes failure, absence and a valid zero-offset target. CoD-written frontend coverage remains separate.

## 2026-11-23 — Apply ordinary value parameters to variadic element writes

<a id="k-variadic-local-values-20261123"></a>
**K-VARIADIC-LOCAL-VALUES-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user clarified “按值传的参数在 callee 的 lexical scope 分配空间，只要没声明 const ，当然允许修改本地值。 这种可以衍生决定，不需要跟我确认。” [D-10-VARARGS r248](design/SymbolicDynamicScope.md#indexed-by-value-elements), P-403/P-404, applies H-272 and H-366–H-369 to H-449's by-value elements. This closes the by-value portion of the indexing question at `00c32d5fa787151e685b9d609fd921f78a9929e4:design/SymbolicDynamicScope.md`. Pack transport and borrowed-element operations keep their separate scope.

## 2026-11-23 — Let the outer brace terminate a grouped module selection

<a id="k-grouped-selector-boundary-20261123"></a>
**K-GROUPED-SELECTOR-BOUNDARY-20261123.** Design-review continuity for [D-15-PUBLIC-SOURCE r71](design/ModuleWorlds.md#public-definitions-and-grouped-selections), P-662/P-663. At `7e1230a3ce6832ce373fa6e5f5fe93a410bdc232:design/ModuleWorlds.md` the grouped selection rule left termination implicit while the published variable example omitted its semicolon. The outer brace determines the boundary under [H-MODULE-SOURCE-STYLE-20261121](#h-module-source-style-20261121), [H-IDIOMATIC-SEMICOLONS-20261122](#h-idiomatic-semicolons-20261122) and [H-NEWLINE-AND-LONG-TEXT-20261123](#h-newline-and-long-text-20261123). The review covers admitted outer selection groups and preserves item syntax and selection permissions.

## 2026-11-23 — Distinguish a present zero from an absent regional ptr field

<a id="h-regional-ptr-field-initialization-20261123"></a>
**H-REGIONAL-PTR-FIELD-INITIALIZATION-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user answered “同意” to allowing `limit = 0u64` to create a fresh present target in the containing region for a nullable scalar field. In the paired example `limit = null` establishes absence. [D-22r69](design/ConstructionAndAcquisition.md#initializing-a-regional-scalar-ptr-field), D-22-REGIONAL-REF-INIT / P-351–P-352, extends the scoped ref-field decision at `5ac88ddf5880bc0d2b28a84677dcc890f93046f6:design/ConstructionAndAcquisition.md`. Existing borrowing, initialization-failure and later assignment contracts govern both fields.

## 2026-11-23 — Initialize a regional scalar ref field from independent bits

<a id="h-regional-ref-field-initialization-20261123"></a>
**H-REGIONAL-REF-FIELD-INITIALIZATION-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user replied “同意” to `remaining = 12u64` creating a fresh target in the counter's region during construction without a second `make`. In the paired example `remaining = first.remaining` binds the existing target. [D-22r68](design/ConstructionAndAcquisition.md#initializing-a-regional-scalar-ref-field), D-22-REGIONAL-REF-INIT / P-351–P-352, settles this part of the H-425 proposal at `98a24cd5a547509131620e67a7d45e3a333a880a:design/ConstructionAndAcquisition.md`. H-438 owner access, H-428 initialization effects and the readonly-reception rules still apply.

## 2026-11-23 — Drop branch-specific defaults from an inferred function signature

<a id="h-inferred-function-defaults-20261123"></a>
**H-INFERRED-FUNCTION-DEFAULTS-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user selected “把默认值清除， `delay(...)` 只留共同的签名内容” for the inferred conditional between urgent and normal delays. [D-10-ARGUMENTS r247](design/SymbolicDynamicScope.md#inferred-function-values-with-different-defaults), P-405/P-406, applies the common-result principle to two compatible function entries with different default origins. H-459–H-461 at `f7bcceab6bc78733665e31ca939a3fb353ced497:design/SymbolicDynamicScope.md` still governs explicit origin-bearing contracts. General overload intersection and physical callable representation were outside the question.

## 2026-11-23 — Preserve checked family membership through booter dependency visits

<a id="k-booter-variadic-dependency-visit-20261123"></a>
**K-BOOTER-VARIADIC-DEPENDENCY-VISIT-20261123.** D-20-MODULE-GRAPH r58→59 and D-ARCH-REVIEW r67→68, P-680–P-683 continue the delegated [family-registration review](applications/cod-booter/docs/ModuleGraph.md#variadic-membership-during-a-dependency-visit). At `eb425957cc4c0498c1bdfa0b305b03068a16113a:applications/cod-booter/src/ModuleGraph.cod`, the child failure prevents receiver validation. The clearance covers a bounded structural visit and one gate before `traverse_graph` calls its terminal. The D-10 family contract and runtime pack scope retain independent authority.

## 2026-11-23 — Review local and imported variadic family registration

<a id="k-variadic-family-registration-20261123"></a>
**K-VARIADIC-FAMILY-REGISTRATION-20261123.** D-ARCH-REVIEW r66→67 and D-20-MODULE-GRAPH r57→58, P-680–P-683 continue the delegated [frontend review](design/ImplementationArchitecture.md#registering-a-variadic-method-family) for D-10’s single-method rule. At `84d5f51203dd1cb1832c5424bc088a53c58bcbda:cod/lang_checked.hh` the seed's executable-profile check precedes registration. The review separates unused family conflicts, original method identity and bodyless promises from executable coverage. Pack execution has a separate acceptance boundary.

## 2026-11-23 — Several arrays can supply one variadic tail in written order

<a id="h-ordered-pack-expansions-20261123"></a>
**H-ORDERED-PACK-EXPANSIONS-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user replied “同意，允许。” to allowing `invoice_total(base, fees..., taxes...)` with left-to-right expansion, increasing element index and failure before later operands or entry. [D-10-VARARGS r246](design/SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter), P-403/P-404 extends the single-tail contract at `84d5f51203dd1cb1832c5424bc088a53c58bcbda:design/SymbolicDynamicScope.md`. Existing value/borrow modes and cleanup apply. Pack storage, escape and executable lowering remain separately scoped.

## 2026-11-23 — A variadic method stands alone in its family

<a id="h-variadic-method-family-20261123"></a>
**H-VARIADIC-METHOD-FAMILY-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user proposed “杜绝 variadic overloads 形成，发现迹象就reject” and answered “同意。” to rejecting even a fixed-plus-variadic pair when the family forms. [D-10r245](design/SymbolicDynamicScope.md#runtime-array-expansion-and-one-variadic-parameter), D-10-VARARGS/D-10-ARGUMENTS, P-403–P-406 replace H-450 and the crossed-preference portion of H-475 at `5da3e593d42f3dd0f9564e840fbd94732fb1ef8b:design/SymbolicDynamicScope.md`. Matching declaration/definition and repeated paths to one method count once. H-449 expansion, explicit default/empty-pack requests and fixed-only default ranking retain their scopes. Multiple expansions and physical pack transport are separate.

## 2026-11-23 — Check a borrowed diagnostic handle across an ordinary module call

<a id="k-booter-source-handle-helper-20261123"></a>
**K-BOOTER-SOURCE-HANDLE-HELPER-20261123.** Delegated architecture review continues D-20-MODULE-GRAPH r56→57 and D-ARCH-REVIEW r65→66 under P-680–P-683. The [checked interface and acceptance](applications/cod-booter/docs/ModuleGraph.md#checking-an-imported-helper-that-borrows-a-handle) apply the declaration-proof contract at `a90aa348759cba9cfa2d05f8a41f203ffd9373e7:applications/cod-booter/docs/ModuleGraph.md` to an ordinary imported diagnostic helper reached inside a catch. Source applicability, checked failures and executable availability retain separate evidence.

## 2026-11-23 — Document when to select a symbolic procedure family

<a id="h-symbolic-selector-guideline-20261123"></a>
**H-SYMBOLIC-SELECTOR-GUIDELINE-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user directed “using/import symbolic proc 应该落到 guidelines 文档，语言支持也要加注释说明适用场景。” D-DOC-IDIOMATICS guideline r2 and D-15r70 distinguish the dynamic consumer, policy installer and family contributor. P-692/P-693 and P-694/P-695 retain the permissions and idiom recorded at `499da9275e93ec78959ab3070120f324d1ef0a51:design/ModuleWorlds.md` and `docs/IdiomaticCoD.md` in that commit.

## 2026-11-23 — Prefer ordinary calls and identity-only use in application code

<a id="h-symbolic-family-reuse-idiom-20261123"></a>
**H-SYMBOLIC-FAMILY-REUSE-IDIOM-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user stated “using/import symbolic proc families 行为应该被积极劝退” and requested discussion of reasonable exceptions. D-15r69 and Idiomatic CoD guide r5 apply the preference under P-692/P-693/P-694/P-695. The tariff installation and extension are illustrative integration cases. Source permissions remain as recorded at `82603c380eb813497f21ba20f43b656f62f7d867:design/ModuleWorlds.md`. No compiler warning is specified.

## 2026-11-23 — Track outstanding source choices and apply existing extension permission

<a id="h-review-backlog-duty-20261123"></a>
**H-REVIEW-BACKLOG-DUTY-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user required open design choices to be maintained in the backlog and brought forward for timely discussion. AGENTS.md assigns that duty to the design discussion. The user also reiterated “using 不授权 extension” for a nested same-signature handler body. At D-10r244 the existing permission check settles the example before overload comparison. D-07r210 and D-15r68 reconcile stale review labels for callback owner counts and ordered postponed initialization against D-18-HERE-PARAM and D-15-POSTPON. Baseline: `8715f6d5:design/DesignAuthorityRegistry.md`, D-10-HANDLER-INSTALL, D-07-CALLBACK-BOUND and D-15-POSTPON. Wider proofs and representation retain their own status.

## 2026-11-23 — Select, call and install a lexical symbolic procedure family

<a id="h-lexical-symbolic-proc-20261123"></a>
**H-LEXICAL-SYMBOLIC-PROC-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user proposed “允许 `effect proc @score;`” and “直接调用静态绑定的 `@score(...)`”, then confirmed “那就按我上面说的明确下来。” The user separately answered “Yes, introduce the symbol when the name is free” to the three-case `using proc` selector question. D-10r243, D-15r67 and P-161/P-613/P-614/P-692/P-693 distinguish source selection, static-family calls with current bindings, and scoped activation with snapshots. This updates H-373 and the invocation boundary of H-EFFECTFUL-PROC-INSTALL-20261118. It supersedes the missing-destination stop in K-QUALIFIED-PROCEDURE-USING-20261123 at `abecd35c618aad811f114c23c43153a648553471:applications/cod-booter/docs/ModuleGraph.md`. D-07r209, D-14r133, D-18 here-storage r16, D-21r32 and module graph r56 apply the same source decision. Other verbs, callable value kinds and native ABIs keep their separate scope.

<a id="k-lexical-symbolic-proc-review-20261123"></a>
**K-LEXICAL-SYMBOLIC-PROC-REVIEW-20261123.** Delegated pre-code review under H-PROJECT-ARCHITECTURE-REVIEW-20261122 clears D-ARCH-REVIEW r65 and the [module graph r56 paths](applications/cod-booter/docs/ModuleGraph.md#checking-a-symbolic-procedure-selector) for the seed and booter. Release of free-name selection can precede executable family invocation and activation. See the former selector review at `abecd35c618aad811f114c23c43153a648553471:applications/cod-booter/docs/ModuleGraph.md`.

## 2026-11-23 — Carry a borrowed package edge through a synchronous directory loan

<a id="k-seed-readonly-here-product-20261123"></a>
**K-SEED-READONLY-HERE-PRODUCT-20261123.** D-ARCH-REVIEW r63→64 and D-20-MODULE-GRAPH r54→55 clear [readonly product access in an ordinary supplied body](design/ImplementationArchitecture.md#readonly-product-references-in-an-ordinary-here-body) under H-CALLABLE-MODEL-20261118, H-OWNER-REF-ARGUMENT-20261121 and P-680–P-683. The existing writable path at `ff472369f7468d0f576a90328955d4b3b35203ab:cod/lang_checked.hh`, `demand_supplied_here`, supplies the live binding projection. The readonly extension includes checked borrowed formals and preserves actual target and slot lifetimes, access and existing native loan boundaries.

## 2026-11-23 — Report the namespace entries that determined a package lookup

<a id="k-booter-package-claim-origins-20261123"></a>
**K-BOOTER-PACKAGE-CLAIM-ORIGINS-20261123.** D-20-MODULE-GRAPH r53→54 and D-ARCH-REVIEW r62→63 correct the canonical-path requirement for shadow and ambiguity reporting at `ab4e279abd92789b4253cbffb6b7dc4e87ec8c2b:applications/cod-booter/docs/ModuleGraph.md`, “Package diagnostics on the normal graph route”. Under H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123 and K-BOOTER-PACKAGE-DIAGNOSTIC-SINK-20261123, the [reviewed boundary](applications/cod-booter/docs/ModuleGraph.md#package-diagnostics-on-the-normal-graph-route) uses observed namespace spellings for claims and canonical identity for opening and graph reuse. P-680–P-683 and P-690/P-691 retain their scope.

## 2026-11-23 — Preserve the procedure destination check in the selector migration

<a id="k-qualified-procedure-using-20261123"></a>
**K-QUALIFIED-PROCEDURE-USING-20261123.** D-20-MODULE-GRAPH r52→53 clarifies [bootstrap checking of a qualified procedure selector](applications/cod-booter/docs/ModuleGraph.md#checking-a-symbolic-procedure-selector) under P-680/P-681. H-116–H-121 distinguish destination selection from combined identity introduction. H-372 separates selection from installation. H-MODULE-DEFAULT-SELECTOR-SUBSTITUTION-20261123 applies the same entry check to written and expanded lists. The missing-destination stop is a bootstrap scope limit. The migration case is `94ccc5f01b125f9dcc7bacac988c679222a10c34:tests/cod/language/module_context_tests.py`, `symbol_destination_boundary`.

## 2026-11-23 — Separate strict path cutover from bundled default selection

<a id="k-package-and-default-cutovers-20261123"></a>
**K-PACKAGE-AND-DEFAULT-CUTOVERS-20261123.** D-20-MODULE-GRAPH r51→52 clarifies the [two delivery checks](applications/cod-booter/docs/ModuleGraph.md#package-and-default-selection-delivery) under H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123 and K-MODULE-INPUTS-AND-DEFAULT-TABLES-20261123. The explicit public interface at `da08a24e4a840ab184fe70117afafa171de82160:cod/library/string.cod` permits a compatible path cutover with migrated consumers. The bundled default list retains its own two-frontend external-use acceptance. P-690–P-693 and the language contracts are unchanged.

## 2026-11-23 — Review lookup of names introduced by module selections

<a id="k-booter-imported-name-lookup-20261123"></a>
**K-BOOTER-IMPORTED-NAME-LOOKUP-20261123.** D-ARCH-REVIEW r61→62 and D-20-MODULE-GRAPH r50→51 continue K-BOOTER-CHECKED-SELECTION-INTEGRATION-20261123 under P-680–P-683. The [review](applications/cod-booter/docs/ModuleGraph.md#lookup-of-imported-names) separates complete module registration from per-occurrence lookup at `40ad5848d314284d7782c503a5c5f39f8241ef27:applications/cod-booter/src/NameResolution.cod`. H-381 and H-MODULE-DEFAULT-SELECTOR-SUBSTITUTION-20261123 retain nearest-binding and receiving-verb semantics.

## 2026-11-23 — Define the Actor and typed chan boundary

<a id="h-actor-chan-boundary-20261123"></a>
**H-ACTOR-CHAN-BOUNDARY-20261123.** In the current cod project conversation on 2026-11-23 (America/Los_Angeles), the project user directed research into future Actor communication using complyue/nedh's networking and complyue/sedh's cluster scheduling as business context. They ruled out a separate language-level thread concurrency model, initially described Erlang-like communicating virtual processes with isolated TLS, then selected a 1:1 relation with OS threads for a simple, effective structure. They requested Actor and chan as the language terminology and required CoD chan to be statically strongly typed.

The user then specified:

> 消息基本采用 udp 语义，不保证接收，不提供回执，应用自己处理流和其它可靠协议

The subsequent refinement was:

> 发送者在后续发送大概率失败的场景下，应收到系统故障报错，除此之外，应获得 “发后不管” 的发送语义。

D-10-ACTOR-CHAN/P-702 covers the r242 Actor section: one dedicated OS thread per Actor, its own execution stack and isolated dynamic context; static message types; ordinary fire-and-forget sending; and system-fault feedback when subsequent sending is likely to fail. It continues H-151/H-168 while replacing the shared-worker execution candidate and narrowing the former generic local-send-error discussion. The UDP comparison selects a semantic boundary, not a physical transport, and does not prohibit a chosen transport's internal retransmission. Endpoint identities/forms, receiver count, source grammar, queue/close behavior, fault classification/reporting, exact ownership consumption and fault recovery, Actor/module lifetimes, native failure containment and wire compatibility remain open. P-131–P-135 retain their qualification. This is an attributed decision record with excerpts; no durable message URL or exact timestamp is available.

## 2026-11-23 — Ground Actor and chan research in network and cluster workloads

<a id="k-actor-chan-research-20261123"></a>
**K-ACTOR-CHAN-RESEARCH-20261123.** [D-32](design/ActorChannels.md), research r1, inspects nedh at `04f5e0f62aabe656c21c3181edd9ba72c3f0c5b7` and sedh at `2567b52ed58985d0edfa6312a3b883c5cf8b79ab`. P-703 records the existing design dependencies; P-704 is a candidate value path. The work traces network routing, explicit reply endpoints, shared collectors, resource recruitment, job settlement and cancellation, and separates source facts, design constraints and operational candidates. Reference examples were inspected, not executed.

Under H-ACTOR-CHAN-BOUNDARY-20261123, D-10 r241→242, D-12 r35→36, D-13 r36→37, D-14 r131→132 and D-07 r207→208 integrate Actor-local terminology, dedicated-thread execution and the systemic-fault/fire-and-forget boundary from the baseline `ec9ac9091925f97ec8caf0cd329f6aaa6e515a4c:design/SymbolicDynamicScope.md` and its companion files. The earlier one-worker/two-Actor milestone and generic per-message Accepted/LocalError recommendation are replaced. Existing H-262/H-263 consumed-name and view-validity rules, intrinsic owner carriers, callable restrictions and cleanup remain; ownership when a systemic fault is reported is not selected. The design-topic index links the research and its remaining decisions. The rest of each document is **continued** with its prior qualification. This records research and contract alignment, not new code or implementation approval.

## 2026-11-23 — Review the standalone booter’s prepared library dependency

<a id="k-booter-standalone-bundle-inputs-20261123"></a>
**K-BOOTER-STANDALONE-BUNDLE-INPUTS-20261123.** D-ARCH-REVIEW r60→61 and D-20-MODULE-GRAPH r49→50 apply P-680–P-683 and H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123 to the input tracking in `3cd933362ff0b3fa3e883de0c1cad830a2c4da51:applications/cod-booter/Makefile`. The [review](applications/cod-booter/docs/ModuleGraph.md#prepared-library-inputs-to-standalone-generation) continues the independent application build established at `86f894407cf51ffe4dc773c1db3dff505b6b8584:applications/cod-booter/Makefile` after the [November 21 standalone build handoff](codex://threads/01a1fd61-077d-75f3-b90d-c1b5e1266148). Its scope is consuming and tracking prepared inputs. Packaging, installation and bootstrap retain their separate authorization.

## 2026-11-23 — Review the seed’s indexed declared-place representation

<a id="k-seed-indexed-declared-place-20261123"></a>
**K-SEED-INDEXED-DECLARED-PLACE-20261123.** D-ARCH-REVIEW r59→60 continues H-PROJECT-ARCHITECTURE-REVIEW-20261122 and H-AUGMENTED-TRANSFER-PLACE-20261123 under P-682/P-683. The [review](design/ImplementationArchitecture.md#declared-places-on-lexical-array-elements) qualifies the shared checked path for explicit and implicit declared places on stable lexical named-array elements. The prior checker gate is at `ec9ac9091925f97ec8caf0cd329f6aaa6e515a4c:cod/lang_checked.hh`, with index preparation in `cod/lang_llvm.hh` at the same commit. Mainline owns implementation and verification of this scope.

## 2026-11-23 — Record booter support for native function declarations and Boolean results

<a id="k-native-callable-booter-status-20261123"></a>
**K-NATIVE-CALLABLE-BOOTER-STATUS-20261123.** D-20-C-DECLARATIONS interop r31 updates the implementation evidence from K-NATIVE-CALLABLE-SEED-STATUS-20261123 under the same r27/r28 contracts. The ordinary Boolean predicate is at `0ce3403e7ba2f56cfcc09d3b4634042c54a667e0:applications/cod-booter/src/NativeSource.cod`, with booter provider-output handling in `ProviderCalls.cod` in the same directory. [Current coverage and remaining feedback limit](applications/cod-booter/docs/LLVMInterop.md#native-callable-kind-and-c-linkage)

## 2026-11-23 — Read maximal symbolic operator runs and assign future infix parsing

<a id="h-maximal-operator-runs-20261123"></a>
**H-MAXIMAL-OPERATOR-RUNS-20261123.** On Nov 23 (America/Los_Angeles) the project user chose “Use maximal runs and migrate adjacent operators” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-10 r240→241 supersedes the joined `n<--1` guarantee at `ba669065025466feb5b761c8cbbe22b1d2a93806:design/SymbolicDynamicScope.md`. P-700/P-701 cover a whole token and source span independent of known operators. The five arithmetic updates retain their P-698/P-699 expansion. D-ARCH-REVIEW r59 and guide r4 apply the rule to frontend migration and authored examples. D-11 r37 and D-07 r207 separate the two existing tokens in starred conversion routes.

<a id="k-infix-fixity-roadmap-20261123"></a>
**K-INFIX-FIXITY-ROADMAP-20261123.** The same user specified “自定义操作符留给 e-wrks/cod 编译器去实现，seed/booter 不需要支持” and identified Edh’s parser as implementation precedent. [DesignTopics](docs/DesignTopics.md#4-用户定义中缀操作符的优先级与结合性) records the future compiler scope and reference. Declaration syntax, precedence scale and module rules remain open.

## 2026-11-23 — Adopt the five colon-marked arithmetic updates

<a id="h-augmented-transfer-tokens-20261123"></a>
**H-AUGMENTED-TRANSFER-TOKENS-20261123.** On Nov 23 (America/Los_Angeles), the project user approved the five spellings `<-:+`, `<-:-`, `<-:*`, `<-:/` and `<-:%` with “好的，按此决定落实” in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). D-10 r239→240 closes the spelling question at `5a30c19e330e78387f5d037607a4c187aa1a64fa:design/SymbolicDynamicScope.md` under H-AUGMENTED-TRANSFER-PLACE-20261123 and P-698/P-699. Guide r3 uses spaces around each complete token. Joined `n<--1` keeps its prior meaning. D-ARCH-REVIEW r58 clears both frontend paths and equivalence checks against the declared-place expansion.

## 2026-11-23 — Review checked selection records in booter name resolution

<a id="k-booter-checked-selection-integration-20261123"></a>
**K-BOOTER-CHECKED-SELECTION-INTEGRATION-20261123.** D-ARCH-REVIEW r56→57 and D-20-MODULE-GRAPH r48→49 continue K-MODULE-INPUTS-AND-DEFAULT-TABLES-20261123 and H-MODULE-DEFAULT-SELECTOR-SUBSTITUTION-20261123. The [review](applications/cod-booter/docs/ModuleGraph.md#checked-selections-in-name-resolution) covers the consuming boundary at `5a30c19e330e78387f5d037607a4c187aa1a64fa:applications/cod-booter/src/NameResolution.cod`, `same_selection` and `register_selection`. Frame entries preserve independent syntax and selection identities and source, graph and catalog spelling provenance. Mainline owns implementation and compatibility.

## 2026-11-23 — Define augmented transfer through one declared place

<a id="h-augmented-transfer-place-20261123"></a>
**H-AUGMENTED-TRANSFER-PLACE-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on Nov 23 (America/Los_Angeles), the project user specified “展开语义准确定义为 `var the_n -> n; the_n <- the_n + 1`”. D-10 r238→239 and P-698/P-699 qualify the expansion under the declared-place and ordinary transfer contracts at `d090ccd4a4d0570d650800e4b72e051af9fdd47f:design/CallableViewsAndCaptures.md` and `design/RegionalOwnership.md` in the same commit. This record covers semantics. The subtraction spelling would reinterpret an existing assignment of a negative value and remains open with the operator inventory.

## 2026-11-23 — Word spellings for Boolean conjunction and disjunction

<a id="h-boolean-word-operators-20261123"></a>
**H-BOOLEAN-WORD-OPERATORS-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on Nov 23 (America/Los_Angeles), the project user requested “加上 and/or 等效于 &&/||” to give authors a choice of source spelling. D-10 r237→238 adds the two infix spellings to the operator contract at `a4d2e60d18aa50ee7d597e931cfb605d9b49cbf9:design/SymbolicDynamicScope.md`. The design review applies H-442 and P-130/P-136 to shared precedence and operation identity, retaining ordinary identifier positions and Boolean short-circuit execution. D-ARCH-REVIEW r56 clears both frontend infix readers and the corresponding source and runtime checks.

## 2026-11-23 — Short case heads with field reception and guards

<a id="h-guarded-short-heads-20261123"></a>
**H-GUARDED-SHORT-HEADS-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on Nov 23 (America/Los_Angeles), the project user requested `Dispatched { val count <- cartons; } -> count;` and added “含 guard 的也最好不要求必须 () 吧”. D-07-CASE r206 extends H-SHORT-CASE-ARMS-20261123 from `a50ef654db52f37d192defaac7edfbc7d0e55e7f:design/DomainLanguageDesign.md`. P-625/P-626 and enum r5/P-659 preserve the pattern identity, reception, guard effects and completion of the full spelling. Guide r2 applies it to Shipment. The scoped D-ARCH-REVIEW r55 assessment clears both direct arm readers to share the existing pattern, guard and body representations.

## 2026-11-23 — Rebind a borrowed native handle within its live owner

<a id="h-native-handle-local-rebinding-20261123"></a>
**H-NATIVE-HANDLE-LOCAL-REBINDING-20261123.** The project user confirmed in the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles) that later `view = other` is allowed: “只要能证明 `O` 还活着，所有曾经的 `make handle ... in O` 就都还有效。” This settles the question left by K-BOOTER-HANDLE-BINDINGS-20261120 at `86631c0731a41c4cb3df9d455898ef9db6d6ecd1:applications/cod-booter/docs/LLVMInterop.md`. D-20-C-HANDLES interop r29→r30 applies P-646/P-647 to a mutable lexical borrow of the same CType and actual owner. Independent borrows, access limits, shorter foreign loans and owner cleanup retain their contracts.

## 2026-11-23 — Review package diagnostics on the normal graph route

<a id="k-booter-package-diagnostic-sink-20261123"></a>
**K-BOOTER-PACKAGE-DIAGNOSTIC-SINK-20261123.** D-ARCH-REVIEW r53→r54 and D-20-MODULE-GRAPH r47→r48 continue the package crossing under H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123 and H-PROJECT-ARCHITECTURE-REVIEW-20261122. The [diagnostic sink design](applications/cod-booter/docs/ModuleGraph.md#package-diagnostics-on-the-normal-graph-route) extends the invocation-owned sink at `a1beef94a5de52e6765d13f88f19077a259e8ab4:applications/cod-booter/native/compile_diagnostic.h`. Mainline owns normal CLI integration and compatibility verification.

## 2026-11-23 — Rewrite Idiomatic CoD against settled language contracts

<a id="k-idiomatic-cod-rewrite-20261123"></a>
**K-IDIOMATIC-COD-REWRITE-20261123.** Editorial continuity from `9d17ad62b9499826aaaee21c4a80c7773ac1f5e6:docs/IdiomaticCoD.md` under the confirmed guideline r1 at `d99a307f388bb680f12393ed0ff60e3680c19317:docs/IdiomaticsGuideline.md`. In the current cod project conversation, the project user directed the rewrite to proceed within settled design after reserving the first three [DesignTopics](docs/DesignTopics.md) for longer-term discussion.

[Guide r1](docs/IdiomaticCoD.md) replaces syntax-led excerpts with explanations of programming choices: domain modeling; storage, access and ownership; callable and dynamic context; absence; control and cleanup; traversal and bounded pipeline composition; modules; domain display; and canonical source form. Examples identify their context and derive observable results, owner obligations and control destinations. General collection APIs, general producer binding and mutable here holders, and pure-related admission remain outside this rewrite; the already confirmed pipeline partials remain included.

D-DOC-IDIOMATICS is **continued** from the approved authoring standards to their guide r1 application under H-IDIOMATICS-GUIDELINE-20261123 and P-694/P-695. Each language operation retains its existing confirmation boundary. The guide preserves identities, permissions, actual owners, effects and cleanup, introduces no language mechanism and follows settled source behavior independently of implementation progress. Acceptance applied the guideline's semantic, example, coverage and hard style checks, including the 100-column maximum; review checked the design references, links and code widths. No compiler-execution claim is made.

## 2026-11-23 — Published bootstrap compiler milestones

<a id="k-bootstrap-delivery-20261123"></a>
**K-BOOTSTRAP-DELIVERY-20261123.** Mainline verified the following changes in both bootstrap modes.

- **Booter source migration.** The booter adopted grouped selections, exported definitions, single-return guards, loop-tail discard and owner-qualified reference formals across 31 CoD files. Both seed-built modes preserved all 280 previously emitted object byte sequences. [Migrated compiler source](https://github.com/complyue/cod-project/blob/5cfb8b408715cf2465232cb84c30e491dd03749b/applications/cod-booter/src/Compiler.cod)
- **Provider compatibility after the typed graph cutover.** Separating source-only sum kinds from builtin catalog kinds restored selection of `prepare_display_operands`. In both modes, all 37 previously accepted modules plus direct `string.cod` matched the prior declaration results and both generated provider files. [Published export-kind fix](https://github.com/complyue/cod-project/blob/a0689d8fc9c91b44a6a6182191cc19e0208d7ca4/applications/cod-booter/src/ModuleExports.cod)
- **Parser recovery through the launcher.** The check, object and provider commands reported two missing operands in source order, retained the primary diagnostic on incomplete recovery, and preserved existing artifacts. Recovery also retained later declarations after a damaged binding name or annotation. [Launcher reporting](https://github.com/complyue/cod-project/blob/8426bf9c091559320ff8d8d1b4c55a1f315bd60e/applications/cod-booter/native/launcher.c) · [Binding recovery](https://github.com/complyue/cod-project/blob/3ea609b17586b25d19e26dacf06994d7168b5aef/applications/cod-booter/src/ParserStorage.cod)

## 2026-11-23 — Review writable product access in the package loader

<a id="k-booter-package-loader-here-place-20261123"></a>
**K-BOOTER-PACKAGE-LOADER-HERE-PLACE-20261123.** D-ARCH-REVIEW r52→r53 and D-20-MODULE-GRAPH r46→r47 continue the crossing at `7c36178e:applications/cod-booter/docs/ModuleGraph.md` under H-PROJECT-ARCHITECTURE-REVIEW-20261122 and H-CALLABLE-MODEL-20261118. The approved seed projection preserves the supplier’s actual lexical place, target access and completion effects for the two-phase loader. Linked acceptance and delivery belong to mainline.

## 2026-11-23 — Align lexical call examples and record deferred discussion topics

<a id="k-lexical-callee-examples-20261123"></a>
**K-LEXICAL-CALLEE-EXAMPLES-20261123.** Editorial continuity from `32a2278768b6460bc68c705c53ea4bbbf7830d4e:design/SymbolicDynamicScope.md#lexical-calls-and-shadowing`. H-257/P-229 and H-CALLABLE-MODEL-20261118 govern the nearest binding and the two callable value kinds. D-10 r236→237, D-07 r204→205, D-12 r34→35 and D-15 r65→66 align examples and references with definition-installed symbolic handlers. Existing qualification boundaries and open choices remain. At the project user’s request, [DesignTopics.md](docs/DesignTopics.md) records the longer-term discussion reserve. Mainline delivery and newly discovered blockers take priority.

## 2026-11-23 — Review the standalone booter’s prepared package inputs

<a id="k-booter-prepared-package-crossing-20261123"></a>
**K-BOOTER-PREPARED-PACKAGE-CROSSING-20261123.** Delegated structural review continues D-ARCH-REVIEW r51→r52 and D-20-MODULE-GRAPH r45→r46 under H-PROJECT-ARCHITECTURE-REVIEW-20261122 and K-MODULE-INPUTS-AND-DEFAULT-TABLES-20261123. The producer at `29190058:include/codp_module_inputs.hh` supplies the frame. CoD validates it once per compilation on demand. The OS boundary drains, closes and reaps before graph traversal resumes. Linked implementation, remote preparation and installation retain separate scope.

## 2026-11-23 — Distinguish published seed support from booter verification

<a id="k-native-callable-seed-status-20261123"></a>
**K-NATIVE-CALLABLE-SEED-STATUS-20261123.** D-20-C-DECLARATIONS interop r28 to r29 updates implementation status under H-NATIVE-CALLABLE-KIND-20261123 and H-NATIVE-BOOLEAN-RESULT-20261123. The published seed admits function linkage at `4540a4bdadf93bc2f81705e06d43a076fd1c4855:cod/lang_syntax.hh` and validates ordinary Boolean provider results at that commit’s `cod/native/provider_call.h`. The booter’s own checker, generators and bundled predicate have a separate delivery checkpoint. P-644/P-645 and the source contract retain their scope.

## 2026-11-23 — Confirm idiomatic guidance standards and canonical source style

<a id="h-idiomatics-guideline-20261123"></a>
**H-IDIOMATICS-GUIDELINE-20261123.** In the current cod project conversation on 2026-11-23 (America/Los_Angeles), the project user required all Idiomatic CoD content to follow settled language design independently of implementation progress, with genuine open design questions listed separately for discussion. They approved the proposed quality standards, requested an independent `docs/IdiomaticsGuideline.md` committed on the latest main, and kept the IdiomaticCoD rewrite for after the design discussions. The same approval made code style a hard requirement, set an uncompromising, almost configuration-free formatter direction, and recommended automatic formatting in commit hooks and formatting check gates. The subsequent message specified: “风格方面，最大列宽初步定 100”.

D-DOC-IDIOMATICS and D-10-FORMAT-POLICY qualify guideline r1 under P-694–P-697/C-06. The initial 100-column maximum applies to CoD source; remaining unconfirmed formatting details and tool implementation/deployment retain their own scope. Existing language decisions are preserved. The predecessor language and authoring baseline is `4a7b6bab67845ced1427888d6c32420d0a37d5fc:AGENTS.md` and the same commit's `design/SymbolicDynamicScope.md`. This record is an attributed decision paraphrase with the exact width statement; no stable locator or exact timestamp for this conversation is available.

## 2026-11-23 — Check exception obligations under the declaration command’s source contracts

<a id="k-booter-declaration-proof-scope-20261123"></a>
**K-BOOTER-DECLARATION-PROOF-SCOPE-20261123.** Delegated pre-code review continues D-20-MODULE-GRAPH r44 to r45 and D-ARCH-REVIEW r50 to r51, P-680–P-683. The [declaration proof scope](applications/cod-booter/docs/ModuleGraph.md#selecting-checked-exception-evidence-for-declaration-commands) applies the checked-call, caught-field and callback contracts at `cb80d92ea54158fee37857b9b34f7f0a1aa95732:applications/cod-booter/docs/ModuleGraph.md` to bound per-revision checking. The review verified the two callback edges in the frozen graph input and the three declaration regressions. Both-mode compatibility and same-candidate wrapper acceptance remain mainline delivery obligations.

## 2026-11-23 — Return a Boolean from a native predicate

<a id="h-native-boolean-result-20261123"></a>
**H-NATIVE-BOOLEAN-RESULT-20261123.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user requested `path_ready(...) -> bool` for use directly in a condition and suggested a named byte representation in C. D-20-C-DECLARATIONS interop r28 spells it `typedef uint8_t COD_BOOL;` and applies existing canonical Boolean and provider failure checks to ordinary results under P-644/P-645. D-ARCH-REVIEW r50 extends the reviewed path provider work at `36c6b9fd40909d19d3dece8bd346bef5d291d755:design/ImplementationArchitecture.md`, “Checking callable kind at C linkage”. This scope covers the two result adapters and the actual predicate use.

## 2026-11-23 — Review native callable-kind checking and the path provider migration

<a id="k-native-callable-kind-architecture-20261123"></a>
**K-NATIVE-CALLABLE-KIND-ARCHITECTURE-20261123.** Delegated pre-code review under H-PROJECT-ARCHITECTURE-REVIEW-20261122 continues [D-ARCH-REVIEW r48 to r49](design/ImplementationArchitecture.md#checking-callable-kind-at-c-linkage), P-682/P-683. The scope applies [H-NATIVE-CALLABLE-KIND-20261123](#h-native-callable-kind-20261123) at `56c3389e9bdbe6fea3cbf26c1a3dcf77584e26e5:applications/cod-booter/docs/LLVMInterop.md`, “Native callable kind and C linkage”, to the two frontends and the real `path_ready` provider. The two-session proof uses established owning bindings for attachment and forwards the resulting borrows for inspection. Mainline owns implementation, compatibility verification and publication.

## 2026-11-23 — Use the least required CoD callable permissions at C linkage

<a id="h-native-callable-kind-20261123"></a>
**H-NATIVE-CALLABLE-KIND-20261123.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user specified that C code with no need for symbolic objects should declare `func "C"` under the “最小必要” principle, citing `ModuleView` and `inspect_module`. D-20-C-DECLARATIONS interop r27 applies the ordinary transitive function restrictions and the existing body-presence direction rule to this spelling. P-644/P-645 and C-48/P-117 preserve explicit owner selection, checked outcomes and synchronous callbacks. The preceding procedure-only profile is at `3db02f06c5cdd21c69bf57fcc5689effaaaa29ae:applications/cod-booter/docs/LLVMInterop.md`, “Declarations and the first host call”. The review found no qualified C API for direct dynamic-symbol lookup or invocation.

## 2026-11-23 — Selecting a symbolic variable names its identity before installation

<a id="h-using-symbolic-variable-identity-20261123"></a>
**H-USING-SYMBOLIC-VARIABLE-IDENTITY-20261123.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user proposed that reading `@sout` is dynamically scoped and effectful, and that at declaration `using "string.cod" { var @sout }` and `symbol from "string.cod" { @sout }` are fully equivalent. The design review accepts that rule and closes the variable-selection question at `118f87074307c947181529aa2878bd2afb149577:design/ModuleWorlds.md`, “Default and complete public selections”. D-15r65 (P-662/P-663 and P-692/P-693), D-10r236, D-07-STRING-BUILD r9 and D-20-MODULE-GRAPH r44 separate static selection from the actual read. H-502 continues to govern visibility when the read executes.

## 2026-11-23 — Substitute kind-qualified module selectors under the receiving verb

<a id="h-module-default-selector-substitution-20261123"></a>
**H-MODULE-DEFAULT-SELECTOR-SUBSTITUTION-20261123.** The project user in the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles) specified “相当于给使用者模块 using/import/mixin 省略的花括号内容补上” and required a symbolic object's nature as a selector prefix, giving `var @sout` and `proc @xxx` to distinguish resource selection from `symbol from`. This supersedes the special bulk-symbol interpretation in K-BULK-SYMBOL-SELECTION-AND-CHECKED-OBJECT-20261123 at `f7dda3349583680463ef296e93c2c06bfbf988f2:design/ModuleWorlds.md`. D-15r64 (P-662/P-663, P-692/P-693) governs the language clarification. D-10r235, D-07-STRING-BUILD r8, interop r26, D-20-MODULE-GRAPH r43 and D-ARCH-REVIEW r48 apply it. Package lookup, overload delegation and the independent exception object review keep their recorded status.

## 2026-11-23 — Clarify bulk symbol rights and review checked throw object integration

<a id="k-bulk-symbol-selection-and-checked-object-20261123"></a>
**K-BULK-SYMBOL-SELECTION-AND-CHECKED-OBJECT-20261123.** Delegated continuity from `d8d165eb1776a828197aa6438f6885c368bc9376:design/ModuleWorlds.md`, “Default and complete public selections”, clarifies bulk symbol identity and existing explicit callable-family grants at D-15r63. Under the same baseline’s `applications/cod-booter/docs/ModuleGraph.md`, “Lowering an owning exception through cleanup”, D-20-MODULE-GRAPH r41 to r42 and D-ARCH-REVIEW r46 to r47 clear shared frame evidence through ordinary source-to-object emission. Native operations and protected callback lowering for the path wrapper remain separate.

## 2026-11-23 — Review bootstrap package inputs and default selection tables

<a id="k-module-inputs-and-default-tables-20261123"></a>
**K-MODULE-INPUTS-AND-DEFAULT-TABLES-20261123.** Delegated structural review of H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123 continues D-ARCH-REVIEW r45 to r46 and D-20-MODULE-GRAPH r40 to r41 from `72aa6a24473c0d1dd168d800c8a2e157d64c23d2:design/ModuleWorlds.md`, “Resolving source module paths” and “Default and complete public selections”. It clears the typed codp input producer, compiler-owned lookup, checked selection storage and source cutover. A prepared remote requires verified Git source content for the selected pin. The project and local-override increment can proceed separately. Dependency acquisition and floating lock policy remain outside this review.

## 2026-11-23 — Resolve package modules and select a library's default exports

<a id="h-module-paths-and-default-exports-20261123"></a>
**H-MODULE-PATHS-AND-DEFAULT-EXPORTS-20261123.** In the [CoD design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on 2026-11-23 (America/Los_Angeles), the project user specified “不是 "./" 开头的目标模块路径，一律按 project deps pkgs 路径解析，即使同pkg的模块代码，也需要完整namespace路径； 以 compiler builtin libs 兜底，用户pkg允许shadow builtin libs，诊断信息报警告；注意明确不引入类似Java classpath那种机制。” They confirmed the other module declaration proposals, including default and complete public selections, and deferred overload `default` to the backlog. D-15r62 qualifies D-15-MODULE-PATH and D-15-DEFAULT-EXPORTS, P-690–P-693. D-07-STRING-BUILD r7 applies the rule to the bundled library and D-20-MODULE-GRAPH r40 separates source selection from native path resolution. Prior contracts are at `339621421b053239bde0e951e20c02495d9b378e:design/ModuleWorlds.md`, “Public definitions and grouped selections”, and the same commit’s `design/StringBuilding.md`. Compiler and package installation changes require their own implementation evidence.

## 2026-11-23 — Check scalar payload reads on a caught exception

<a id="k-booter-caught-field-source-check-20261123"></a>
**K-BOOTER-CAUGHT-FIELD-SOURCE-CHECK-20261123.** Delegated review continues D-20-MODULE-GRAPH r38 to r39 and D-ARCH-REVIEW r44 to r45 from `5e26bb6a727da0892a60fba2fc2cdaaa4e36f1dd:applications/cod-booter/docs/ModuleGraph.md`, “Checking an imported call without object code”. Construction and direct catch-field projection use the same nominal schema. The caller retains distinct field-access and failure-coverage proofs. Executable nominal projection retains its existing acceptance.

## 2026-11-23 — Carry a declared checked failure through module publication

<a id="k-booter-imported-checked-call-20261123"></a>
**K-BOOTER-IMPORTED-CHECKED-CALL-20261123.** Delegated review continues D-20-MODULE-GRAPH r37 to r38 and D-ARCH-REVIEW r43 to r44 from `d902a970f3a460613a1609da7183378c7fbe1120:applications/cod-booter/docs/ModuleGraph.md`, “Publishing a checked nominal exception”. The callable’s published source signature carries its declared failure through re-export and forwarding even when the caller constructs no exception and the declaration has no native ordinal. H-524/H-525 govern the caller obligation. The clearance covers the CoD graph and source checks before declaration and provider publication.

## 2026-11-23 — Check the forwarded path callback and exception re-exports

<a id="k-booter-here-and-exception-selection-20261123"></a>
**K-BOOTER-HERE-AND-EXCEPTION-SELECTION-20261123.** Delegated pre-code review continues the path callback, nominal exception and module selection contracts at `75e8d6482e9a7fd1da3dce2e60cc2c8f4f99322d:applications/cod-booter/docs/ModuleGraph.md`, “Checking the forwarded path callback”, and `75e8d6482e9a7fd1da3dce2e60cc2c8f4f99322d:design/ModuleWorlds.md`, “Publication-only re-export”. D-20-MODULE-GRAPH r36 to r37 and D-ARCH-REVIEW r42 to r43 clear shared checked-call evidence and both exception selection paths with separate receiver-world acceptance. Executable native-here lowering retains its own review.

## 2026-11-23 — Review the bootstrap recovery path through diagnostic delivery

<a id="k-parser-recovery-architecture-20261123"></a>
**K-PARSER-RECOVERY-ARCHITECTURE-20261123.** Delegated pre-code review under H-PROJECT-ARCHITECTURE-REVIEW-20261122 continues the recovery contract at `a53ca1f9368c6182594197eacf4f53733fea228f:applications/cod-booter/README.md`, “Whole-file parser recovery”. D-ARCH-REVIEW r41 to r42 covers stable report references, lexer and scratch preflight, complete-file publication gates and the launcher-owned ordered snapshot. The two-operand workload and subsequent production-by-production migration share that structure. D-20-PARSE-RECOVERY/P-688–P-689 retains its confirmed scope.

## 2026-11-23 — Definition doclets and whole-file syntax recovery

<a id="h-definition-doclets-20261123"></a>
**H-DEFINITION-DOCLETS-20261123.** In the [design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50) on Nov 23 (America/Los_Angeles), the project user requested that `""" ... """` also associate documentation with a nearby definition, specifying “这在 seed/booter 都无须处理，未来的 LSP 分析和 IDE 集成等组件要映射为 ref-site tooltip ，且支持 doclet 内部放导航链接等能力。” D-07-DOCLETS/P-686–P-687 qualifies this tooling scope under charter §§8–9, extending the literal at `bf994e54449f7d4834555cfb5632eae3017a128d:design/Utf8Strings.md`. Language r204 specifies leading attachment and definition-context links; UTF-8 r6 distinguishes declaration and expression positions.

<a id="h-whole-file-parser-recovery-20261123"></a>
**H-WHOLE-FILE-PARSER-RECOVERY-20261123.** In the same discussion, the project user directed “所有 parser 都不要遇到第一个错误即停止，而是容忍错误确保全文件覆盖，把后续错误都能报出来” and proposed AST “错误报告” nodes that can occupy different syntactic roles. D-20-PARSE-RECOVERY/P-688–P-689 qualifies full-file recovery and later error delivery for all parsers. Roadmap r27 extends the stage policy at `0290f99e8588acfb0842314b58ac701c9169c7d3:applications/cod-booter/README.md`; interop r25 extends its private single-report boundary at the same commit in `applications/cod-booter/docs/LLVMInterop.md`. A recovered tree can be used by tooling. Source acceptance, ownership checks and artifact publication retain their requirements.

## 2026-11-23 — Identify the written type in an uncovered checked throw

<a id="k-seed-checked-throw-names-20261123"></a>
**K-SEED-CHECKED-THROW-NAMES-20261123.** The delegated architecture review continues D-20-DIAGNOSTIC-NAMES at `499638e08258f02269b361e1248ac2b7a8f5d75d:applications/cod-booter/README.md`, “Names in diagnostics”. D-ARCH-REVIEW r40 to r41 clears retaining occurrence evidence outside solver state and sharing the checked defining-name formatter with the emitter. Handling obligations and cleanup follow the established exception contract.

## 2026-11-23 — Short case arms and layout-independent CoD syntax

<a id="h-short-case-arms-20261123"></a>
**H-SHORT-CASE-ARMS-20261123.** Recorded Nov 23 (America/Los_Angeles) from the [project user’s design discussion](codex://threads/01a1d9b7-6ec6-7331-b986-bbc05a8feb50). Following the `Vacant -> false;` assessment, the user added “还有 `enum` 也跟 sum type 类似” and “使用 `_ -> ...` 是CoD程序作者自担后果，CoD语言设计仅提供此便利性。按此决定落实执行。” D-07-CASE at language r203 and D-07-ENUM r4 extend the case and enum contracts at `499638e08258f02269b361e1248ac2b7a8f5d75d:design/DomainLanguageDesign.md` and `499638e08258f02269b361e1248ac2b7a8f5d75d:design/TypedEnums.md`. P-625/P-626 and P-658/P-659 apply existing identity, coverage and control rules to the short spelling. The wildcard applies to every subject.

<a id="h-newline-and-long-text-20261123"></a>
**H-NEWLINE-AND-LONG-TEXT-20261123.** In the same discussion the project user directed “newline 在 CoD 中总是承担 whitespace 职责，不做特殊语义设计；相应的长字符串literal要允许 `""" ... """` 这种包含内部换行的语法，其它有要求同行写的场景也通过语法设计消除限制。” D-10-SEMICOLON-UX at r234 supersedes the open newline choice at `499638e08258f02269b361e1248ac2b7a8f5d75d:design/SymbolicDynamicScope.md`. D-07-UTF8 r5 extends the literal forms in `499638e08258f02269b361e1248ac2b7a8f5d75d:design/Utf8Strings.md`. P-570–P-572 and P-674/P-675 preserve deterministic parsing and exact text bytes. Escape and indentation handling follow the existing text contract.

<a id="k-compact-arms-and-literals-20261123"></a>
**K-COMPACT-ARMS-AND-LITERALS-20261123.** The delegated pre-code review under H-PROJECT-ARCHITECTURE-REVIEW-20261122 continues D-ARCH-REVIEW r39 to r40 for both source frontends, the existing text emission path and direct acceptance. See `499638e08258f02269b361e1248ac2b7a8f5d75d:design/ImplementationArchitecture.md` for the preceding scope. Implementation and compatibility verification belong to mainline.

## Historical evidence for November 13–22

Historical entries below use source E, `cb7e10f86058054b3709cd437d87ac443ec4cd50:CHANGELOG.md`. Each line range identifies the original record, including its evidence and exclusions. [Source E on GitHub](https://github.com/complyue/cod-project/blob/cb7e10f86058054b3709cd437d87ac443ec4cd50/CHANGELOG.md)

## 2026-11-22 — Review seed exception naming and booter publication of diagnostic names

<a id="k-seed-nominal-diagnostic-names-20261122"></a>K-SEED-NOMINAL-DIAGNOSTIC-NAMES-20261122 · E:382–383

<a id="k-booter-publication-plan-20261122"></a>K-BOOTER-PUBLICATION-PLAN-20261122 · E:385–386

## 2026-11-22 — Preserve the source name at each constructing throw

<a id="k-booter-throw-site-names-20261122"></a>K-BOOTER-THROW-SITE-NAMES-20261122 · E:390–391

## 2026-11-22 — Show resolved diagnostic names and prefer semicolon omission

<a id="h-diagnostic-names-20261122"></a>H-DIAGNOSTIC-NAMES-20261122 · E:395–396

<a id="h-idiomatic-semicolons-20261122"></a>H-IDIOMATIC-SEMICOLONS-20261122 · E:398–399

## 2026-11-22 — Carry the defining exception name and field layout across object units

<a id="k-booter-exception-artifact-20261122"></a>K-BOOTER-EXCEPTION-ARTIFACT-20261122 · E:403–404

## 2026-11-22 — Assign design review and production cutover to their respective owners

<a id="h-design-review-delivery-roles-20261122"></a>H-DESIGN-REVIEW-DELIVERY-ROLES-20261122 · E:408–409

## 2026-11-22 — Separate builtin and source-only export kind codes

<a id="k-booter-export-kinds-20261122"></a>K-BOOTER-EXPORT-KINDS-20261122 · E:413–414

## 2026-11-22 — Build the standalone booter through the CoD graph

<a id="k-booter-cod-cutover-20261122"></a>K-BOOTER-COD-CUTOVER-20261122 · E:418–419

## 2026-11-22 — Review the path wrapper’s owned throw and forwarded callback

<a id="k-booter-owned-throw-lowering-20261122"></a>K-BOOTER-OWNED-THROW-LOWERING-20261122 · E:423–424

## 2026-11-22 — Move booter production membership to the CoD driver

<a id="k-booter-production-cutover-checks-20261122"></a>K-BOOTER-PRODUCTION-CUTOVER-CHECKS-20261122 · E:428–429

## 2026-11-22 — Read a symbolic dependency procedure and its scoped definition

<a id="k-booter-symbolic-procedure-syntax-20261122"></a>K-BOOTER-SYMBOLIC-PROCEDURE-SYNTAX-20261122 · E:433–434

## 2026-11-22 — Read the module index’s slot and fixed value parameters

<a id="k-booter-slot-formal-syntax-20261122"></a>K-BOOTER-SLOT-FORMAL-SYNTAX-20261122 · E:438–439

## 2026-11-22 — Check finite aggregate storage in the booter

<a id="k-booter-aggregate-schemas-20261122"></a>K-BOOTER-AGGREGATE-SCHEMAS-20261122 · E:443–444

## 2026-11-22 — Parse named constructor fields in booter case arms

<a id="k-booter-constructor-case-syntax-20261122"></a>K-BOOTER-CONSTRUCTOR-CASE-SYNTAX-20261122 · E:448–449

## 2026-11-22 — Share the provider’s reserved member spellings

<a id="k-booter-provider-member-names-20261122"></a>K-BOOTER-PROVIDER-MEMBER-NAMES-20261122 · E:453–454

## 2026-11-22 — Read the graph factory’s owning result in CoD

<a id="k-booter-own-region-result-syntax-20261122"></a>K-BOOTER-OWN-REGION-RESULT-SYNTAX-20261122 · E:458–459

## 2026-11-22 — Return a fresh graph owner to each typed command

<a id="k-booter-fresh-graph-result-20261122"></a>K-BOOTER-FRESH-GRAPH-RESULT-20261122 · E:463–464

## 2026-11-22 — Preserve the graph budget’s readonly declaration in the booter parser

<a id="k-booter-const-val-parser-20261122"></a>K-BOOTER-CONST-VAL-PARSER-20261122 · E:468–469

## 2026-11-22 — Forward the graph allocator through the named terminal receiver

<a id="k-booter-terminal-array-allocator-20261122"></a>K-BOOTER-TERMINAL-ARRAY-ALLOCATOR-20261122 · E:473–474

## 2026-11-22 — Publish and select the booter’s nominal sum constructors

<a id="k-booter-sum-interfaces-20261122"></a>K-BOOTER-SUM-INTERFACES-20261122 · E:478–479

## 2026-11-22 — Parse the graph region and bind CLI destinations at their entries

<a id="k-booter-region-parser-terminal-inputs-20261122"></a>K-BOOTER-REGION-PARSER-TERMINAL-INPUTS-20261122 · E:483–484

## 2026-11-22 — Read nominal sums and consume the completed booter graph

<a id="k-booter-sum-parser-graph-output-20261122"></a>K-BOOTER-SUM-PARSER-GRAPH-OUTPUT-20261122 · E:488–489

## 2026-11-22 — Native target layout facts for provider rows

<a id="k-booter-provider-target-layout-20261122"></a>K-BOOTER-PROVIDER-TARGET-LAYOUT-20261122 · E:493–494

## 2026-11-22 — Generate and publish the booter’s provider files through CoD

<a id="k-booter-provider-publication-20261122"></a>K-BOOTER-PROVIDER-PUBLICATION-20261122 · E:498–499

## 2026-11-22 — Writable root field in a scoped dependency handler

<a id="k-booter-handler-scalar-loan-20261122"></a>K-BOOTER-HANDLER-SCALAR-LOAN-20261122 · E:503–504

## 2026-11-22 — CoD visit publication through the object compiler

<a id="k-booter-visit-publication-20261122"></a>K-BOOTER-VISIT-PUBLICATION-20261122 · E:508–509

## 2026-11-22 — LLVM construction from published graph units

<a id="k-booter-llvm-unit-projection-20261122"></a>K-BOOTER-LLVM-UNIT-PROJECTION-20261122 · E:513–514

## 2026-11-22 — Synchronous graph root-field loans

<a id="k-booter-root-field-loan-20261122"></a>K-BOOTER-ROOT-FIELD-LOAN-20261122 · E:518–519

## 2026-11-22 — Checked table-holder updates through a local dependency call

<a id="k-booter-handler-holder-20261122"></a>K-BOOTER-HANDLER-HOLDER-20261122 · E:523–524

## 2026-11-22 — CoD module index for the first linked dependency edge

<a id="k-booter-module-index-20261122"></a>K-BOOTER-MODULE-INDEX-20261122 · E:528–529

## 2026-11-22 — Live activation for the dependency handler

<a id="k-booter-visit-activation-20261122"></a>K-BOOTER-VISIT-ACTIVATION-20261122 · E:533–534

## 2026-11-22 — Writable supplier array in the path-key consumer

<a id="k-booter-path-key-consumer-20261122"></a>K-BOOTER-PATH-KEY-CONSUMER-20261122 · E:538–539

## 2026-11-22 — Named-array transport for the first dependency edge

<a id="k-booter-array-transport-20261122"></a>K-BOOTER-ARRAY-TRANSPORT-20261122 · E:543–544

## 2026-11-22 — Ordinary native path results and key-loan feedback

<a id="k-booter-native-u64-result-20261122"></a>K-BOOTER-NATIVE-U64-RESULT-20261122 · E:548–549

## 2026-11-22 — First CoD-owned dependency edge

<a id="k-booter-first-edge-20261122"></a>K-BOOTER-FIRST-EDGE-20261122 · E:553–554

## 2026-11-22 — Dependency path owners and graph storage

<a id="k-booter-graph-edge-prerequisites-20261122"></a>K-BOOTER-GRAPH-EDGE-PREREQUISITES-20261122 · E:558–559

## 2026-11-22 — Read-only Boolean observation in the seed checker

<a id="k-seed-boolean-observation-20261122"></a>K-SEED-BOOLEAN-OBSERVATION-20261122 · E:563–564

## 2026-11-22 — Typed syntax across the booter compiler passes

<a id="k-booter-syntax-consumers-20261122"></a>K-BOOTER-SYNTAX-CONSUMERS-20261122 · E:568–569

## 2026-11-22 — Isolated typed booter file-to-object route

<a id="k-booter-typed-object-20261122"></a>K-BOOTER-TYPED-OBJECT-20261122 · E:573–574

## 2026-11-22 — Node identity in the booter's typed AST

<a id="k-booter-ast-membership-20261122"></a>K-BOOTER-AST-MEMBERSHIP-20261122 · E:578–579

## 2026-11-22 — Lexical backing for the indexed AST array

<a id="k-booter-lexical-extent-20261122"></a>K-BOOTER-LEXICAL-EXTENT-20261122 · E:583–584

## 2026-11-22 — First indexed booter AST path

<a id="k-booter-ast-table-20261122"></a>K-BOOTER-AST-TABLE-20261122 · E:588–589

## 2026-11-22 — Work-method counterexample for the booter AST

<a id="h-booter-work-method-20261122"></a>H-BOOTER-WORK-METHOD-20261122 · E:593–594

## 2026-11-22 — Implementation architecture review

<a id="h-compiler-layers-20261122"></a>H-COMPILER-LAYERS-20261122 · E:598–599

<a id="h-project-architecture-review-20261122"></a>H-PROJECT-ARCHITECTURE-REVIEW-20261122 · E:601–602

## 2026-11-22 — Seed availability of residual deferred aggregates

<a id="k-defer-aggregate-availability-20261122"></a>K-DEFER-AGGREGATE-AVAILABILITY-20261122 · E:606–607

## 2026-11-21 — Preparing operands for display concatenation

<a id="k-string-concat-operands-20261121"></a>K-STRING-CONCAT-OPERANDS-20261121 · E:611–612

## 2026-11-21 — Concatenation with displayable values

<a id="h-string-concat-display-20261121"></a>H-STRING-CONCAT-DISPLAY-20261121 · E:616–617

## 2026-11-21 — Unchecked UTF-8 output failure

<a id="h-utf8-write-error-20261121"></a>H-UTF8-WRITE-ERROR-20261121 · E:621–622

## 2026-11-21 — Scoped output for public type formatters

<a id="h-string-sout-20261121"></a>H-STRING-SOUT-20261121 · E:626–627

## 2026-11-21 — Successful completion in text examples

<a id="k-text-proc-return"></a>K-TEXT-PROC-RETURN · E:631–632

## 2026-11-21 — Direct text places and lexical string building

<a id="h-string-build-20261121"></a>H-STRING-BUILD-20261121 · E:636–637

## 2026-11-21 — Library transfer policies for text

<a id="h-string-transfer-policy-20261121"></a>H-STRING-TRANSFER-POLICY-20261121 · E:641–642

## 2026-11-21 — UTF-8 text at the C boundary

<a id="h-utf8-native-text-20261121"></a>H-UTF8-NATIVE-TEXT-20261121 · E:646–647

## 2026-11-21 — Scoped native handle effects

<a id="h-native-handle-effect-20261121"></a>H-NATIVE-HANDLE-EFFECT-20261121 · E:651–652

## 2026-11-21 — Native handle type declarations and nested resource ownership

<a id="h-handle-type-within-20261121"></a>H-HANDLE-TYPE-WITHIN-20261121 · E:656–657

## 2026-11-21 — Owner-qualified ref formals and checked selection at the call site

<a id="h-owner-ref-argument-20261121"></a>H-OWNER-REF-ARGUMENT-20261121 · E:661–662

## 2026-11-21 — Single-return guards and discarded loop results

<a id="h-guards-and-loop-discard-20261121"></a>H-GUARDS-AND-LOOP-DISCARD-20261121 · E:666–667

## 2026-11-21 — Publish at the definition and group same-module selections

<a id="h-module-source-style-20261121"></a>H-MODULE-SOURCE-STYLE-20261121 · E:671–672

## 2026-11-21 — Typed enums and checked by-value C exchange

<a id="h-typed-enum-20261121"></a>H-TYPED-ENUM-20261121 · E:676–677

<a id="k-typed-enum-review-20261121"></a>K-TYPED-ENUM-REVIEW-20261121 · E:679–680

<a id="k-enum-status-wiring"></a>K-ENUM-STATUS-WIRING · E:682–683

## 2026-11-21 — Name the static type of a catch binding

<a id="k-catch-root-spelling-20261121"></a>K-CATCH-ROOT-SPELLING-20261121 · E:687–688

## 2026-11-21 — Share the C language runtime and build native components independently

<a id="h-shared-c-runtime-20261121"></a>H-SHARED-C-RUNTIME-20261121 · E:692–693

<a id="h-native-build-boundary-20261121"></a>H-NATIVE-BUILD-BOUNDARY-20261121 · E:695–696

## 2026-11-21 — Catch the holder and release contextual variable names

<a id="h-catch-holder-only-20261121"></a>H-CATCH-HOLDER-ONLY-20261121 · E:700–701

<a id="h-lexical-name-release-20261121"></a>H-LEXICAL-NAME-RELEASE-20261121 · E:703–704

## 2026-11-21 — Exception ownership at the source and parser boundaries

<a id="h-exception-bindings-20261121"></a>H-EXCEPTION-BINDINGS-20261121 · E:708–709

## 2026-11-21 — Use contextual `own` for ownership

<a id="h-contextual-own-20261121"></a>H-CONTEXTUAL-OWN-20261121 · E:713–714

## 2026-11-20 — Spell lexical borrows and owner-bound CoD results

<a id="k-booter-handle-bindings-20261120"></a>K-BOOTER-HANDLE-BINDINGS-20261120 · E:718–719

## 2026-11-20 — Declare the owner CType of a borrowed native handle

<a id="h-booter-handle-owner-ctype-20261120"></a>H-BOOTER-HANDLE-OWNER-CTYPE-20261120 · E:723–724

## 2026-11-20 — Connect builtin lookup and rejection reporting to the first launcher

<a id="k-booter-catalog-diagnostic-20261120"></a>K-BOOTER-CATALOG-DIAGNOSTIC-20261120 · E:728–729

## 2026-11-20 — Generate the typed SourceRevision failure interface

<a id="h-source-revision-failure-20261120"></a>H-SOURCE-REVISION-FAILURE-20261120 · E:733–734

## 2026-11-20 — Confirm C declaration direction and failed acquisition

<a id="h-booter-c-declarations-20261120"></a>H-BOOTER-C-DECLARATIONS-20261120 · E:738–739

## 2026-11-20 — Acquire and release the first LLVM binding through typed handles

<a id="h-booter-llvm-handles-20261120"></a>H-BOOTER-LLVM-HANDLES-20261120 · E:743–744

## 2026-11-20 — Use scoped native handles in the first booter

<a id="h-booter-c-handles-20261120"></a>H-BOOTER-C-HANDLES-20261120 · E:748–749

## 2026-11-20 — Put the first generated C provider profile up for review

<a id="k-booter-c-profile-20261120"></a>K-BOOTER-C-PROFILE-20261120 · E:753–754

## 2026-11-20 — Generate the booter’s C providers from CoD declarations

<a id="h-booter-generated-c-20261120"></a>H-BOOTER-GENERATED-C-20261120 · E:758–759

## 2026-11-20 — Keep the booter and its exception-handling work in cod-seed

<a id="h-booter-exceptions-diagnostics-20261120"></a>H-BOOTER-EXCEPTIONS-DIAGNOSTICS-20261120 · E:763–764

## 2026-11-20 — Defer HLVM applications until the basic CoD frontend works

<a id="h-hlvm-candidate-20261120"></a>H-HLVM-CANDIDATE-20261120 · E:768–769

## 2026-11-19 — Require the booter’s C-native execution path

<a id="h-booter-native-minimum-20261119"></a>H-BOOTER-NATIVE-MINIMUM-20261119 · E:773–774

## 2026-11-19 — Make the initial booter the seed delivery scope

<a id="h-booter-driven-seed-20261119"></a>H-BOOTER-DRIVEN-SEED-20261119 · E:778–779

## 2026-11-19 — Carry conditional field readiness into queried helpers

<a id="k-query-input-guards-20261119"></a>K-QUERY-INPUT-GUARDS-20261119 · E:783–784

## 2026-11-19 — Preserve prepared Boolean conditions in queried helpers

<a id="k-query-prepared-boolean-20261119"></a>K-QUERY-PREPARED-BOOLEAN-20261119 · E:788–789

## 2026-11-19 — Keep guarded readiness on a saved executable receiver

<a id="k-saved-embedded-receiver-20261119"></a>K-SAVED-EMBEDDED-RECEIVER-20261119 · E:793–794

## 2026-11-19 — Check guarded inline writes in executable code

<a id="k-executable-embedded-selection-20261119"></a>K-EXECUTABLE-EMBEDDED-SELECTION-20261119 · E:798–799

## 2026-11-19 — Retain guarded inline field readiness in queries

<a id="k-queried-guarded-inline-fields-20261119"></a>K-QUERIED-GUARDED-INLINE-FIELDS-20261119 · E:803–804

## 2026-11-19 — Preserve guarded readiness through query calls

<a id="k-queried-guarded-chains-20261119"></a>K-QUERIED-GUARDED-CHAINS-20261119 · E:808–809

## 2026-11-19 — Preserve proofs through embedded reference chains in queries

<a id="k-queried-embedded-chains-20261119"></a>K-QUERIED-EMBEDDED-CHAINS-20261119 · E:813–814

## 2026-11-19 — Preserve returned embedded referents in queries

<a id="k-queried-returned-referents-20261119"></a>K-QUERIED-RETURNED-REFERENTS-20261119 · E:818–819

## 2026-11-19 — Track selected embedded reference slots in queries

<a id="k-queried-embedded-slots-20261119"></a>K-QUERIED-EMBEDDED-SLOTS-20261119 · E:823–824

## 2026-11-19 — Preserve selected field readiness through borrows in queries

<a id="k-queried-borrowed-fields-20261119"></a>K-QUERIED-BORROWED-FIELDS-20261119 · E:828–829

## 2026-11-19 — Prove readiness of selected product fields in queries

<a id="k-queried-selected-fields-20261119"></a>K-QUERIED-SELECTED-FIELDS-20261119 · E:833–834

## 2026-11-19 — Check product effects through queried helper calls

<a id="k-queried-product-calls-20261119"></a>K-QUERIED-PRODUCT-CALLS-20261119 · E:838–839

## 2026-11-19 — Check local product readiness inside type queries

<a id="k-queried-product-readiness-20261119"></a>K-QUERIED-PRODUCT-READINESS-20261119 · E:843–844

## 2026-11-19 — Check live lexical product slots in full scopes

<a id="k-full-scope-product-slots-20261119"></a>K-FULL-SCOPE-PRODUCT-SLOTS-20261119 · E:848–849

## 2026-11-19 — Prove shared saved receiver choices during cleanup

<a id="k-deferred-saved-receiver-20261119"></a>K-DEFERRED-SAVED-RECEIVER-20261119 · E:853–854

## 2026-11-19 — Update deferred field initialization coverage

<a id="k-deferred-path-initialization-20261119"></a>K-DEFERRED-PATH-INITIALIZATION-20261119 · E:858–859

## 2026-11-19 — Update deferred field rebinding coverage

<a id="k-deferred-slot-destinations-20261119"></a>K-DEFERRED-SLOT-DESTINATIONS-20261119 · E:863–864

## 2026-11-19 — Update deferred field lifetime coverage

<a id="k-deferred-live-ancestors-20261119"></a>K-DEFERRED-LIVE-ANCESTORS-20261119 · E:868–869

## 2026-11-19 — Update deferred product projection coverage

<a id="k-deferred-product-projections-20261119"></a>K-DEFERRED-PRODUCT-PROJECTIONS-20261119 · E:873–874

## 2026-11-19 — Update deferred product field coverage

<a id="k-deferred-embedded-slot-effects-20261119"></a>K-DEFERRED-EMBEDDED-SLOT-EFFECTS-20261119 · E:878–879

## 2026-11-19 — Update borrowed slot rebinding coverage

<a id="k-borrowed-embedded-slot-effects-20261119"></a>K-BORROWED-EMBEDDED-SLOT-EFFECTS-20261119 · E:883–884

## 2026-11-19 — Update borrowed reference-field coverage

<a id="k-borrowed-reference-fields-20261119"></a>K-BORROWED-REFERENCE-FIELDS-20261119 · E:888–889

## 2026-11-19 — Update finite reference component coverage

<a id="k-required-reference-components-20261119"></a>K-REQUIRED-REFERENCE-COMPONENTS-20261119 · E:893–894

## 2026-11-19 — Update the lexical product coverage reference

<a id="k-lexical-reference-field-coverage-20261119"></a>K-LEXICAL-REFERENCE-FIELD-COVERAGE-20261119 · E:898–899

## 2026-11-19 — Confirm the storage-helper migration

<a id="h-storage-helper-confirmation-20261119"></a>H-STORAGE-HELPER-CONFIRMATION-20261119 · E:903–904

## 2026-11-19 — Return traversal and storage names to ordinary lookup

<a id="h-ordinary-names-20261119"></a>H-ORDINARY-NAMES-20261119 · E:908–909

## 2026-11-19 — Use the lexical owner by default for ordinary construction

<a id="h-make-lexical-default-20261119"></a>H-MAKE-LEXICAL-DEFAULT-20261119 · E:913–914

## 2026-11-19 — Write the warehouse paper's appendix in CoD

<a id="k-warehouse-cod-source-20261119"></a>K-WAREHOUSE-COD-SOURCE-20261119 · E:918–919

## 2026-11-19 — Document the typed DBMR program cycle

<a id="k-typed-dbmr-program-overview-20261119"></a>K-TYPED-DBMR-PROGRAM-OVERVIEW-20261119 · E:923–924

## 2026-11-19 — Qualify the composite synchronous pipeline for implementation

<a id="h-pipeline-composite-contract-20261119"></a>H-PIPELINE-COMPOSITE-CONTRACT-20261119 · E:928–929

## 2026-11-19 — Design the complete pipeline before implementation and tests

<a id="h-pipeline-composite-first-20261119"></a>H-PIPELINE-COMPOSITE-FIRST-20261119 · E:933–934

## 2026-11-19 — Assign pipeline semantics and implementation-led syntax work

<a id="h-pipeline-design-remit-20261119"></a>H-PIPELINE-DESIGN-REMIT-20261119 · E:938–939

## 2026-11-19 — Name partial pipelines locally and store domain functors in regions

<a id="k-open-pipeline-storage-20261119"></a>K-OPEN-PIPELINE-STORAGE-20261119 · E:943–944

## 2026-11-19 — Connect both ends of a composed processor chain

<a id="k-pipeline-named-terminal-20261119"></a>K-PIPELINE-NAMED-TERMINAL-20261119 · E:948–949

## 2026-11-19 — Compose processors before attaching a source

<a id="k-processor-composition-20261119"></a>K-PROCESSOR-COMPOSITION-20261119 · E:953–954

## 2026-11-19 — Compare forward and reverse pipelines over processor functors

<a id="k-processor-functor-pipes-20261119"></a>K-PROCESSOR-FUNCTOR-PIPES-20261119 · E:958–959

## 2026-11-19 — Assess closed pipelines against streaming library use cases

<a id="k-pipeline-library-examples-20261119"></a>K-PIPELINE-LIBRARY-EXAMPLES-20261119 · E:963–964

## 2026-11-19 — Compare closed pipeline declarations and calls on one fare search

<a id="k-closed-pipeline-idioms-20261119"></a>K-CLOSED-PIPELINE-IDIOMS-20261119 · E:968–969

## 2026-11-19 — Merge data constructors as methods of their type’s constructor family

<a id="h-dctor-family-merge-20261119"></a>H-DCTOR-FAMILY-MERGE-20261119 · E:973–974

## 2026-11-19 — Publish and merge call operators with the receiver type

<a id="h-functor-publication-20261119"></a>H-FUNCTOR-PUBLICATION-20261119 · E:978–979

## 2026-11-19 — Review short decisions before pipelines and the Actor model

<a id="k-quick-review-and-actor-agenda-20261119"></a>K-QUICK-REVIEW-AND-ACTOR-AGENDA-20261119 · E:983–984

## 2026-11-19 — Apply ordinary method dispatch to sum receivers

<a id="k-sum-receiver-dispatch-20261119"></a>K-SUM-RECEIVER-DISPATCH-20261119 · E:988–989

## 2026-11-19 — Put the sum alternative in the receiver signature

<a id="h-sum-receiver-parameter-20261119"></a>H-SUM-RECEIVER-PARAMETER-20261119 · E:993–994

## 2026-11-19 — Consolidate constructor, functor and module selection rules

<a id="k-ctor-call-organization-20261119"></a>K-CTOR-CALL-ORGANIZATION-20261119 · E:998–999

## 2026-11-19 — Name the opposing constructor and call-operator method families

<a id="h-ctor-call-families-20261119"></a>H-CTOR-CALL-FAMILIES-20261119 · E:1003–1004

## 2026-11-19 — Distinguish constructor destinations from call-operator receivers

<a id="h-ctor-call-separation-20261119"></a>H-CTOR-CALL-SEPARATION-20261119 · E:1008–1009

## 2026-11-19 — Consider carrying call operators with their nominal receiver type

<a id="k-type-call-family-20261119"></a>K-TYPE-CALL-FAMILY-20261119 · E:1013–1014

## 2026-11-19 — Focus the comprehension research outline on enumeration

<a id="k-comprehension-research-scope-20261119"></a>K-COMPREHENSION-RESEARCH-SCOPE-20261119 · E:1018–1019

## 2026-11-19 — Express ambiguous call intent with the distinguishing argument

<a id="h-call-intent-20261119"></a>H-CALL-INTENT-20261119 · E:1023–1024

## 2026-11-19 — Scope structural producers and transformers

<a id="k-producer-transformers-20261119"></a>K-PRODUCER-TRANSFORMERS-20261119 · E:1028–1029

## 2026-11-19 — Document callable containers as choice producers

<a id="k-enumerative-choice-20261119"></a>K-ENUMERATIVE-CHOICE-20261119 · E:1033–1034

## 2026-11-19 — Apply ordinary argument preparation to a directly supplied functor

<a id="h-functor-argument-20261119"></a>H-FUNCTOR-ARGUMENT-20261119 · E:1038–1039

## 2026-11-19 — Plan a shared language specification repository

<a id="h-lang-repository-20261119"></a>H-LANG-REPOSITORY-20261119 · E:1043–1044

## 2026-11-19 — Callable results and deferred return inspection

<a id="h-callable-tail-20261119"></a>H-CALLABLE-TAIL-20261119 · E:1048–1049

<a id="h-defer-returning-20261119"></a>H-DEFER-RETURNING-20261119 · E:1051–1052

## 2026-11-19 — Review ordinary function and here-body tail results

<a id="k-callable-tail-20261119"></a>K-CALLABLE-TAIL-20261119 · E:1056–1057

## 2026-11-19 — Confirm explicit discard of a typed result

<a id="h-void-discard-20261119"></a>H-VOID-DISCARD-20261119 · E:1061–1062

## 2026-11-19 — Preserve cleanup responsibility when discarding a result

<a id="h-discard-cleanup-20261119"></a>H-DISCARD-CLEANUP-20261119 · E:1066–1067

## 2026-11-19 — Confirm the explicit throwing case fallback

<a id="h-case-throw-20261119"></a>H-CASE-THROW-20261119 · E:1071–1072

## 2026-11-19 — Permit statically exhaustive cases and delimit arm headers

<a id="h-case-coverage-20261119"></a>H-CASE-COVERAGE-20261119 · E:1076–1077

## 2026-11-19 — Require a wildcard fallback for value-producing cases

<a id="h-case-fallback-20261119"></a>H-CASE-FALLBACK-20261119 · E:1081–1082

## 2026-11-19 — Review static value-case completion

<a id="k-case-completion-20261119"></a>K-CASE-COMPLETION-20261119 · E:1086–1087

## 2026-11-19 — Identify the consumer of a case result

<a id="k-case-consumer-20261119"></a>K-CASE-CONSUMER-20261119 · E:1091–1092

## 2026-11-19 — Confirm case expressions and lexical control

<a id="h-case-contract-20261119"></a>H-CASE-CONTRACT-20261119 · E:1096–1097

## 2026-11-19 — Confirm cleanup on fallthrough from a full-scope arm

<a id="h-case-exits-20261119"></a>H-CASE-EXITS-20261119 · E:1101–1102

## 2026-11-19 — Define source-ordered case execution

<a id="h-case-flow-20261119"></a>H-CASE-FLOW-20261119 · E:1106–1107

## 2026-11-19 — Set case coverage and arm-scope boundaries

<a id="h-case-arms-20261119"></a>H-CASE-ARMS-20261119 · E:1111–1112

## 2026-11-19 — Explore case matching with branch-local variables

<a id="h-case-lexical-20261119"></a>H-CASE-LEXICAL-20261119 · E:1116–1117

## 2026-11-19 — Assign diagnostic work across compiler stages

<a id="h-compiler-diagnostics-20261119"></a>H-COMPILER-DIAGNOSTICS-20261119 · E:1121–1122

## 2026-11-19 — Confirm the DBMR redesign

<a id="h-dbmr-approved-20261119"></a>H-DBMR-APPROVED-20261119 · E:1126–1127

## 2026-11-19 — Build privately and publish immutable DBMR generations

<a id="h-dbmr-immutable-20261119"></a>H-DBMR-IMMUTABLE-20261119 · E:1131–1132

## 2026-11-19 — Align the trailing-callback introduction with its examples

<a id="k-header-prose-20261119"></a>K-HEADER-PROSE-20261119 · E:1136–1137

## 2026-11-19 — Adopt parenthesized receiving headers

<a id="h-here-header-20261119"></a>H-HERE-HEADER-20261119 · E:1141–1142

## 2026-11-19 — Receive each generator handoff with the same loop controls

<a id="h-generator-reception-20261119"></a>H-GENERATOR-RECEPTION-20261119 · E:1146–1147

## 2026-11-18 — Explore generator declarations as a named here parameter

<a id="k-generator-yield-formal-20261118"></a>K-GENERATOR-YIELD-FORMAL-20261118 · E:1151–1152

## 2026-11-18 — Count the receiver-bound functor as a here callable

<a id="h-here-functor-20261118"></a>H-HERE-FUNCTOR-20261118 · E:1156–1157

## 2026-11-18 — Generalized for-loops and collection visits

<a id="k-generalized-for-20261118"></a>K-GENERALIZED-FOR-20261118 · E:1161–1162

## 2026-11-18 — Give here-body feedback to continue and the original call result to break

<a id="h-here-break-continue-20261118"></a>H-HERE-BREAK-CONTINUE-20261118 · E:1166–1167

## 2026-11-18 — Confirm capture-free functions and activation-bounded here callables

<a id="h-callable-model-20261118"></a>H-CALLABLE-MODEL-20261118 · E:1171–1172

<a id="k-callable-contract-alignment-20261118"></a>K-CALLABLE-CONTRACT-ALIGNMENT-20261118 · E:1174–1175

## 2026-11-18 — Return break feedback to the body invocation and name the callable value kinds

<a id="h-here-body-result-20261118"></a>H-HERE-BODY-RESULT-20261118 · E:1179–1180

<a id="h-callable-value-kinds-20261118"></a>H-CALLABLE-VALUE-KINDS-20261118 · E:1182–1183

## 2026-11-18 — Assess break with a result for a here-body call

<a id="k-here-break-20261118"></a>K-HERE-BREAK-20261118 · E:1187–1188

## 2026-11-18 — Assess single-brace here arguments with caller-targeted exits

<a id="k-here-group-20261118"></a>K-HERE-GROUP-20261118 · E:1192–1193

## 2026-11-18 — Forward one here callable through synchronous function calls

<a id="k-here-callable-parameter-20261118"></a>K-HERE-CALLABLE-PARAMETER-20261118 · E:1197–1198

<a id="h-here-callable-forward-20261118"></a>H-HERE-CALLABLE-FORWARD-20261118 · E:1200–1201

<a id="h-here-origin-handoff-20261118"></a>H-HERE-ORIGIN-HANDOFF-20261118 · E:1203–1204

## 2026-11-18 — Assess function-only callable values

<a id="k-function-only-callable-values-20261118"></a>K-FUNCTION-ONLY-CALLABLE-VALUES-20261118 · E:1208–1209

## 2026-11-18 — Call symbolic-effect consumers by definition

<a id="h-effect-consumer-call-20261118"></a>H-EFFECT-CONSUMER-CALL-20261118 · E:1213–1214

## 2026-11-18 — Restrict installation and invocation of effectful procedures

<a id="h-effectful-proc-install-20261118"></a>H-EFFECTFUL-PROC-INSTALL-20261118 · E:1218–1219

## 2026-11-18 — Assess callable storage, lexical slot access and indirect replay

<a id="k-callable-storage-review-20261118"></a>K-CALLABLE-STORAGE-REVIEW-20261118 · E:1223–1224

## 2026-11-18 — Include executable module top levels as here callables

<a id="h-here-module-toplevel-20261118"></a>H-HERE-MODULE-TOPLEVEL-20261118 · E:1228–1229

## 2026-11-18 — Fix procedure replay snapshots at definition activation

<a id="h-symbolic-snapshot-activation-20261118"></a>H-SYMBOLIC-SNAPSHOT-ACTIVATION-20261118 · E:1233–1234

## 2026-11-18 — Review here callables and direct lexical access

<a id="k-here-callables-20261118"></a>K-HERE-CALLABLES-20261118 · E:1238–1239

## 2026-11-18 — Assess callback parameters with different owner counts at different call sites

<a id="k-callback-owner-arity-20261118"></a>K-CALLBACK-OWNER-ARITY-20261118 · E:1243–1244

## 2026-11-18 — Review functor receiver access and callback escape

<a id="k-functor-receiver-20261118"></a>K-FUNCTOR-RECEIVER-20261118 · E:1248–1249

## 2026-11-18 — Close the ordinary sum constructor source design

<a id="h-dctor-source-20261118"></a>H-DCTOR-SOURCE-20261118 · E:1253–1254

## 2026-11-18 — Distinguish constructing a sum alternative from allocating a reference slot

<a id="k-dctor-slot-review-20261118"></a>K-DCTOR-SLOT-REVIEW-20261118 · E:1258–1259

## 2026-11-18 — Publish and select whole data constructors in their module world

<a id="h-dctor-world-publication-20261118"></a>H-DCTOR-WORLD-PUBLICATION-20261118 · E:1263–1264

## 2026-11-18 — Draft named sum constructor bodies

<a id="k-sum-constructor-draft-20261118"></a>K-SUM-CONSTRUCTOR-DRAFT-20261118 · E:1268–1269

## 2026-11-18 — Region holder reuse and completion of persistent graph versions

<a id="h-owner-reuse-20261118"></a>H-OWNER-REUSE-20261118 · E:1273–1274

<a id="h-dbmr-image-completion-20261118"></a>H-DBMR-IMAGE-COMPLETION-20261118 · E:1276–1277

<a id="h-dbmr-payload-checksum-20261118"></a>H-DBMR-PAYLOAD-CHECKSUM-20261118 · E:1279–1280

## 2026-11-18 — Distinguish parked questions from the active discussion queue

<a id="k-discussion-parked-20261118"></a>K-DISCUSSION-PARKED-20261118 · E:1284–1285

## 2026-11-18 — Prioritize constructor and functor discussions

<a id="k-discussion-priorities-20261118"></a>K-DISCUSSION-PRIORITIES-20261118 · E:1289–1290

## 2026-11-18 — Assign DBMR contention handling to the native provider

<a id="h-dbmr-contention-work-20261118"></a>H-DBMR-CONTENTION-WORK-20261118 · E:1294–1295

## 2026-11-18 — Scope DBMR coordination to cooperating CoD processes

<a id="h-dbmr-cooperating-processes-20261118"></a>H-DBMR-COOPERATING-PROCESSES-20261118 · E:1299–1300

## 2026-11-18 — Coordinate DBMR file access across processes

<a id="h-dbmr-access-modes-20261118"></a>H-DBMR-ACCESS-MODES-20261118 · E:1304–1305

<a id="k-dbmr-marker-20261118"></a>K-DBMR-MARKER-20261118 · E:1307–1308

## 2026-11-18 — Correct implementation defects under the confirmed design

<a id="h-implementation-correction-20261118"></a>H-IMPLEMENTATION-CORRECTION-20261118 · E:1312–1313

<a id="k-scalar-correction-20261118"></a>K-SCALAR-CORRECTION-20261118 · E:1315–1316

## 2026-11-18 — Publish initialized DBMRs exclusively and define the native C boundary

<a id="h-dbmr-new-publication-20261118"></a>H-DBMR-NEW-PUBLICATION-20261118 · E:1320–1321

<a id="h-native-c-interface-20261118"></a>H-NATIVE-C-INTERFACE-20261118 · E:1323–1324

## 2026-11-18 — Clarify collection visitor proof cases

<a id="k-collection-visitor-proof-20261118"></a>K-COLLECTION-VISITOR-PROOF-20261118 · E:1328–1329

## 2026-11-18 — Give collections and builtin containers a dedicated design

<a id="k-collection-interfaces-20261118"></a>K-COLLECTION-INTERFACES-20261118 · E:1333–1334

## 2026-11-18 — Take one independently owned deferred failure

<a id="h-defer-member-take-20261118"></a>H-DEFER-MEMBER-TAKE-20261118 · E:1338–1339

## 2026-11-18 — Select the cause before braced exception fields

<a id="h-braced-cause-20261118"></a>H-BRACED-CAUSE-20261118 · E:1343–1344

## 2026-11-18 — Type membership guards for abstract targets

<a id="h-isa-20261118"></a>H-ISA-20261118 · E:1348–1349

## 2026-11-18 — Best-effort reporting on memory exhaustion

<a id="h-oom-best-effort-20261118"></a>H-OOM-BEST-EFFORT-20261118 · E:1353–1354

## 2026-11-18 — Defer typed borrow support across exception wrapping

<a id="k-cause-borrow-cost-20261118"></a>K-CAUSE-BORROW-COST-20261118 · E:1358–1359

## 2026-11-18 — Preserve typed inspection across a cause-owner handoff

<a id="h-exception-cause-handoff-20261118"></a>H-EXCEPTION-CAUSE-HANDOFF-20261118 · E:1363–1364

## 2026-11-18 — Review mutually exclusive exception construction clauses

<a id="k-make-cause-placement-20261118"></a>K-MAKE-CAUSE-PLACEMENT-20261118 · E:1368–1369

## 2026-11-18 — Settle normal exception construction

<a id="h-make-exception-20261118"></a>H-MAKE-EXCEPTION-20261118 · E:1373–1374

## 2026-11-18 — Exception capacity tuning and next-stage toolchain planning

<a id="h-exception-capacity-20261118"></a>H-EXCEPTION-CAPACITY-20261118 · E:1378–1379

<a id="h-toolchain-repositories-20261118"></a>H-TOOLCHAIN-REPOSITORIES-20261118 · E:1381–1382

## 2026-11-18 — Give a deferred aggregate its own region and exception array

<a id="h-defer-aggregate-region-20261118"></a>H-DEFER-AGGREGATE-REGION-20261118 · E:1386–1387

## 2026-11-18 — Evaluate exception values backed by owned regions

<a id="k-exception-region-model-20261118"></a>K-EXCEPTION-REGION-MODEL-20261118 · E:1391–1392

## 2026-11-18 — Trace exception ownership across control flow

<a id="k-exception-scenarios-20261118"></a>K-EXCEPTION-SCENARIOS-20261118 · E:1396–1397

## 2026-11-18 — Give exception declarations one ownership mode

<a id="h-exception-decl-owned-20261118"></a>H-EXCEPTION-DECL-OWNED-20261118 · E:1401–1402

## 2026-11-18 — Name the exception object and its owner together

<a id="h-exception-holder-20261118"></a>H-EXCEPTION-HOLDER-20261118 · E:1406–1407

## 2026-11-18 — Compare spellings for one exception-scope handoff

<a id="h-exception-whole-owner-20261118"></a>H-EXCEPTION-WHOLE-OWNER-20261118 · E:1411–1412

## 2026-11-18 — Keep each cause chain in one owner

<a id="h-same-owner-cause-20261118"></a>H-SAME-OWNER-CAUSE-20261118 · E:1416–1417

<a id="k-exception-scope-release-20261118"></a>K-EXCEPTION-SCOPE-RELEASE-20261118 · E:1419–1420

## 2026-11-18 — Account for exception owners at catch and return

<a id="k-explicit-exception-owners-20261118"></a>K-EXPLICIT-EXCEPTION-OWNERS-20261118 · E:1424–1425

## 2026-11-18 — Integrate the exception factory and full-scope records

<a id="k-exception-factory-mainline-20261118"></a>K-EXCEPTION-FACTORY-MAINLINE-20261118 · E:1429–1430

## 2026-11-18 — Return a locally caught exception from a factory

<a id="h-exception-factory-20261118"></a>H-EXCEPTION-FACTORY-20261118 · E:1434–1435

## 2026-11-18 — Align the full-scope exception input contract

<a id="k-full-scope-exception-references-20261118"></a>K-FULL-SCOPE-EXCEPTION-REFERENCES-20261118 · E:1439–1440

## 2026-11-18 — Construct and initialize at a throw site

<a id="h-throw-initializer-20261118"></a>H-THROW-INITIALIZER-20261118 · E:1444–1445

## 2026-11-18 — Select exception storage at construction

<a id="h-exception-make-scope-20261118"></a>H-EXCEPTION-MAKE-SCOPE-20261118 · E:1449–1450

## 2026-11-18 — Make exception objects owner-bound

<a id="h-exception-object-owner-20261118"></a>H-EXCEPTION-OBJECT-OWNER-20261118 · E:1454–1455

<a id="h-catch-owner-transfer-20261118"></a>H-CATCH-OWNER-TRANSFER-20261118 · E:1457–1458

<a id="h-exception-field-owners-20261118"></a>H-EXCEPTION-FIELD-OWNERS-20261118 · E:1460–1461

<a id="k-exception-owner-migration-20261118"></a>K-EXCEPTION-OWNER-MIGRATION-20261118 · E:1463–1464

## 2026-11-18 — Pass immediate scope inputs through ordinary call roles

<a id="k-immediate-scope-inputs-20261118"></a>K-IMMEDIATE-SCOPE-INPUTS-20261118 · E:1468–1469

## 2026-11-18 — Integrate exception inspection and ownership review

<a id="k-exception-inspection-integration-20261118"></a>K-EXCEPTION-INSPECTION-INTEGRATION-20261118 · E:1473–1474

## 2026-11-18 — Assess application lifetime for exception objects

<a id="k-exception-owner-review-20261118"></a>K-EXCEPTION-OWNER-REVIEW-20261118 · E:1478–1479

## 2026-11-18 — Use a nullable pointer for cause and a non-null reference for tip

<a id="h-cause-ptr-tip-ref-20261118"></a>H-CAUSE-PTR-TIP-REF-20261118 · E:1483–1484

<a id="h-cause-link-only-20261118"></a>H-CAUSE-LINK-ONLY-20261118 · E:1486–1487

<a id="k-cause-ptr-tip-ref-20261118"></a>K-CAUSE-PTR-TIP-REF-20261118 · E:1489–1490

## 2026-11-18 — Give cause and tip the direct Exception type

<a id="h-exception-field-types-20261118"></a>H-EXCEPTION-FIELD-TYPES-20261118 · E:1494–1495

<a id="k-exception-field-types-20261118"></a>K-EXCEPTION-FIELD-TYPES-20261118 · E:1497–1498

## 2026-11-18 — Report the incoming exception from its catch

<a id="h-catch-tip-20261118"></a>H-CATCH-TIP-20261118 · E:1502–1503

<a id="k-catch-tip-20261118"></a>K-CATCH-TIP-20261118 · E:1505–1506

## 2026-11-18 — Infer checked failures of standalone full scopes

<a id="k-full-scope-checked-interface-20261118"></a>K-FULL-SCOPE-CHECKED-INTERFACE-20261118 · E:1510–1511

## 2026-11-18 — Integrate standalone scope decisions

<a id="k-standalone-scope-integration-20261118"></a>K-STANDALONE-SCOPE-INTEGRATION-20261118 · E:1515–1516

## 2026-11-18 — Set the throw-only scope default and application rethrow guidance

<a id="h-standalone-void-20261118"></a>H-STANDALONE-VOID-20261118 · E:1520–1521

<a id="h-wrapper-usage-20261118"></a>H-WRAPPER-USAGE-20261118 · E:1523–1524

<a id="k-standalone-void-wrapper-20261118"></a>K-STANDALONE-VOID-WRAPPER-20261118 · E:1526–1527

## 2026-11-18 — Share full-scope value and callable completion

<a id="k-full-scope-value-boundary-20261118"></a>K-FULL-SCOPE-VALUE-BOUNDARY-20261118 · E:1531–1532

## 2026-11-18 — Clarify source validity in a full-scope type query

<a id="k-full-scope-query-20261118"></a>K-FULL-SCOPE-QUERY-20261118 · E:1536–1537

## 2026-11-18 — Specify checked failures for standalone and callback double braces

<a id="h-double-brace-throws-20261118"></a>H-DOUBLE-BRACE-THROWS-20261118 · E:1541–1542

<a id="k-double-brace-throws-20261118"></a>K-DOUBLE-BRACE-THROWS-20261118 · E:1544–1545

## 2026-11-18 — Release the displaced region on successful owning assignment

<a id="h-occupied-owner-20261118"></a>H-OCCUPIED-OWNER-20261118 · E:1549–1550

<a id="k-occupied-owner-20261118"></a>K-OCCUPIED-OWNER-20261118 · E:1552–1553

## 2026-11-18 — Integrate conditional source decisions and capture evidence

<a id="k-conditional-capture-integration-20261118"></a>K-CONDITIONAL-CAPTURE-INTEGRATION-20261118 · E:1557–1558

## 2026-11-18 — Infer one type and category for conditional results

<a id="h-conditional-result-20261118"></a>H-CONDITIONAL-RESULT-20261118 · E:1562–1563

<a id="k-conditional-result-20261118"></a>K-CONDITIONAL-RESULT-20261118 · E:1565–1566

## 2026-11-18 — Preserve readonly capture ownership and measure native cost

<a id="k-readonly-capture-cost-20261118"></a>K-READONLY-CAPTURE-COST-20261118 · E:1570–1571

## 2026-11-18 — Select parenthesized value results and unparenthesized operator idioms

<a id="h-parenthesized-values-20261118"></a>H-PARENTHESIZED-VALUES-20261118 · E:1575–1576

<a id="k-parenthesized-values-20261118"></a>K-PARENTHESIZED-VALUES-20261118 · E:1578–1579

## 2026-11-18 — Qualify the bounded full-scope implementation profile

<a id="h-full-scope-implementation-20261118"></a>H-FULL-SCOPE-IMPLEMENTATION-20261118 · E:1583–1584

<a id="k-full-scope-implementation-20261118"></a>K-FULL-SCOPE-IMPLEMENTATION-20261118 · E:1586–1587

## 2026-11-17 — Implement bounded capture inputs and defer source contexts

<a id="k-bounded-capture-defer-20261117"></a>K-BOUNDED-CAPTURE-DEFER-20261117 · E:1591–1592

## 2026-11-17 — Include the immediate-borrow proof in full-scope review

<a id="k-full-scope-immediate-borrow-20261117"></a>K-FULL-SCOPE-IMMEDIATE-BORROW-20261117 · E:1596–1597

## 2026-11-17 — Review ordinary-callable lowering for full scopes

<a id="k-full-scope-lowering-review-20261117"></a>K-FULL-SCOPE-LOWERING-REVIEW-20261117 · E:1601–1602

## 2026-11-17 — Preserve readonly reference reception

<a id="h-readonly-ref-binding-20261117"></a>H-READONLY-REF-BINDING-20261117 · E:1606–1607

<a id="k-readonly-ref-binding-20261117"></a>K-READONLY-REF-BINDING-20261117 · E:1609–1610

## 2026-11-17 — Isolate the readonly receiving choice

<a id="k-rhs-review-20261117"></a>K-RHS-REVIEW-20261117 · E:1614–1615

## 2026-11-17 — Adopt bounded implementation contracts and require semantic fidelity

<a id="h-bounded-implementation-adoption-20261117"></a>H-BOUNDED-IMPLEMENTATION-ADOPTION-20261117 · E:1619–1620

<a id="h-implementation-fidelity-20261117"></a>H-IMPLEMENTATION-FIDELITY-20261117 · E:1622–1623

<a id="k-bounded-implementation-adoption-20261117"></a>K-BOUNDED-IMPLEMENTATION-ADOPTION-20261117 · E:1625–1626

## 2026-11-17 — Clarify replacement failure without rolling back RHS effects

<a id="k-replacement-effects-20261117"></a>K-REPLACEMENT-EFFECTS-20261117 · E:1630–1631

## 2026-11-17 — Prefer optional semicolons with actionable syntax warnings

<a id="h-semicolon-ux-20261117"></a>H-SEMICOLON-UX-20261117 · E:1635–1636

<a id="k-semicolon-ux-20261117"></a>K-SEMICOLON-UX-20261117 · E:1638–1639

## 2026-11-17 — Keep the registry preface focused on current authority

<a id="k-current-registry-20261117"></a>K-CURRENT-REGISTRY-20261117 · E:1643–1644

## 2026-11-17 — Unify full scopes with ordinary callable execution

<a id="h-full-scope-callable-20261117"></a>H-FULL-SCOPE-CALLABLE-20261117 · E:1648–1649

<a id="h-trailing-positional-20261117"></a>H-TRAILING-POSITIONAL-20261117 · E:1651–1652

<a id="h-trailing-callback-20261117"></a>H-TRAILING-CALLBACK-20261117 · E:1654–1655

<a id="k-full-scope-callable-20261117"></a>K-FULL-SCOPE-CALLABLE-20261117 · E:1657–1658

## 2026-11-17 — Keep review routing and registry records current

<a id="k-review-routing-20261117"></a>K-REVIEW-ROUTING-20261117 · E:1662–1663

## 2026-11-17 — Give explicit full scopes independent deferred cleanup

<a id="h-full-scope-defer-20261117"></a>H-FULL-SCOPE-DEFER-20261117 · E:1667–1668

<a id="k-full-scope-defer-20261117"></a>K-FULL-SCOPE-DEFER-20261117 · E:1670–1671

## 2026-11-17 — Propose bounded deferred-execution source checks

<a id="k-798"></a>K-798 · E:1675–1676

## 2026-11-17 — Share native invocation parameter roles

<a id="k-797"></a>K-797 · E:1680–1681

## 2026-11-17 — Integrate current source and discussion contracts

<a id="k-discussion-integration-20261117"></a>K-DISCUSSION-INTEGRATION-20261117 · E:1685–1686

## 2026-11-17 — Factor independent scalar input checks

<a id="k-796"></a>K-796 · E:1690–1691

## 2026-11-17 — Refresh the current design discussion index

<a id="k-discussion-scan-20261117"></a>K-DISCUSSION-SCAN-20261117 · E:1695–1696

## 2026-11-17 — Separate source decisions from remaining implementation work

<a id="k-tracker-boundaries-20261117"></a>K-TRACKER-BOUNDARIES-20261117 · E:1700–1701

## 2026-11-17 — Bind yielded here references to the generator invocation

<a id="h-604"></a>H-604 · E:1705–1706

<a id="k-generator-here-20261117"></a>K-GENERATOR-HERE-20261117 · E:1708–1709

## 2026-11-17 — Close bounded init cases and review stable generator-local borrowing

<a id="k-init-flow-owner-review-20261117"></a>K-INIT-FLOW-OWNER-REVIEW-20261117 · E:1713–1714

## 2026-11-17 — Integrate caller-owned generator effect boundaries

<a id="k-generator-effect-integration-20261117"></a>K-GENERATOR-EFFECT-INTEGRATION-20261117 · E:1718–1719

## 2026-11-17 — Keep generator bodies under caller permissions

<a id="h-603"></a>H-603 · E:1723–1724

<a id="k-generator-caller-effects-20261117"></a>K-GENERATOR-CALLER-EFFECTS-20261117 · E:1726–1727

## 2026-11-17 — Delimit generator callable types and isolate effect eligibility

<a id="k-generator-type-context-20261117"></a>K-GENERATOR-TYPE-CONTEXT-20261117 · E:1731–1732

## 2026-11-17 — Retire the source root intrinsic

<a id="k-795"></a>K-795 · E:1736–1737

## 2026-11-17 — Integrate init-place clarification with ready-ref proofs

<a id="k-init-source-integration-20261117"></a>K-INIT-SOURCE-INTEGRATION-20261117 · E:1741–1742

## 2026-11-17 — Clarify init references and explicit caller places

<a id="h-601"></a>H-601 · E:1746–1747

<a id="h-602"></a>H-602 · E:1749–1750

<a id="k-init-place-correction-20261117"></a>K-INIT-PLACE-CORRECTION-20261117 · E:1752–1753

## 2026-11-17 — Isolate the target-initialization helper interface

<a id="k-init-target-review-20261117"></a>K-INIT-TARGET-REVIEW-20261117 · E:1757–1758

## 2026-11-17 — Prove caller ref invariants through pointer methods

<a id="k-794"></a>K-794 · E:1762–1763

## 2026-11-17 — Keep generator loop-control owners fixed

<a id="h-600"></a>H-600 · E:1767–1768

<a id="k-generator-fixed-owners-20261117"></a>K-GENERATOR-FIXED-OWNERS-20261117 · E:1770–1771

## 2026-11-17 — Bound borrowed yields by ordinary owners

<a id="k-generator-borrows-20261117"></a>K-GENERATOR-BORROWS-20261117 · E:1775–1776

## 2026-11-17 — Select named region access and compositional initialization

<a id="h-595"></a>H-595 · E:1780–1781

<a id="h-596"></a>H-596 · E:1783–1784

<a id="h-597"></a>H-597 · E:1786–1787

<a id="h-598"></a>H-598 · E:1789–1790

<a id="h-599"></a>H-599 · E:1792–1793

<a id="k-named-roots-20261117"></a>K-NAMED-ROOTS-20261117 · E:1795–1796

<a id="k-generator-protocol-20261117"></a>K-GENERATOR-PROTOCOL-20261117 · E:1798–1799

## 2026-11-17 — Apply ordinary liveness to result-bound owners

<a id="h-594"></a>H-594 · E:1803–1804

<a id="k-result-owners-20261117"></a>K-RESULT-OWNERS-20261117 · E:1806–1807

## 2026-11-17 — Prepare checked body storage independently of LLVM

<a id="k-793"></a>K-793 · E:1811–1812

## 2026-11-17 — Derive generator header reception from existing scope rules

<a id="k-generator-headers-20261117"></a>K-GENERATOR-HEADERS-20261117 · E:1816–1817

## 2026-11-17 — Settle derived receiving and result rules

<a id="h-592"></a>H-592 · E:1821–1822

<a id="h-593"></a>H-593 · E:1824–1825

<a id="k-receiving-core-20261117"></a>K-RECEIVING-CORE-20261117 · E:1827–1828

## 2026-11-17 — Allow structural callable-result projection

<a id="h-591"></a>H-591 · E:1832–1833

<a id="k-result-projection-20261117"></a>K-RESULT-PROJECTION-20261117 · E:1835–1836

## 2026-11-17 — Stabilize contextual callable coverage

<a id="k-792"></a>K-792 · E:1840–1841

## 2026-11-17 — Preserve lexical returns across generator cleanup

<a id="h-589"></a>H-589 · E:1845–1846

<a id="h-590"></a>H-590 · E:1848–1849

<a id="k-generator-cleanup-20261117"></a>K-GENERATOR-CLEANUP-20261117 · E:1851–1852

## 2026-11-17 — Accept generator signatures and checked body forwarding

<a id="h-584"></a>H-584 · E:1856–1857

<a id="h-585"></a>H-585 · E:1859–1860

<a id="h-586"></a>H-586 · E:1862–1863

<a id="h-587"></a>H-587 · E:1865–1866

<a id="h-588"></a>H-588 · E:1868–1869

<a id="k-generator-failures-20261117"></a>K-GENERATOR-FAILURES-20261117 · E:1871–1872

## 2026-11-17 — Bound source preparation depth

<a id="k-791"></a>K-791 · E:1876–1877

## 2026-11-17 — Specify generator feedback checking and propose signatures

<a id="h-580"></a>H-580 · E:1881–1882

<a id="h-581"></a>H-581 · E:1884–1885

<a id="h-582"></a>H-582 · E:1887–1888

<a id="h-583"></a>H-583 · E:1890–1891

<a id="k-generator-feedback-20261117"></a>K-GENERATOR-FEEDBACK-20261117 · E:1893–1894

## 2026-11-17 — Separate checked programs from source analysis

<a id="k-790"></a>K-790 · E:1898–1899

## 2026-11-17 — Consolidate ranged-for and generator design

<a id="h-577"></a>H-577 · E:1903–1904

<a id="h-578"></a>H-578 · E:1906–1907

<a id="h-579"></a>H-579 · E:1909–1910

<a id="k-generator-draft-20261117"></a>K-GENERATOR-DRAFT-20261117 · E:1912–1913

## 2026-11-17 — Prove uniform scalar callable coverage

<a id="k-789"></a>K-789 · E:1917–1918

## 2026-11-17 — Define receiving borrowed-read conflicts as undefined behavior

<a id="h-576"></a>H-576 · E:1922–1923

<a id="k-receive-overlap-20261117"></a>K-RECEIVE-OVERLAP-20261117 · E:1925–1926

## 2026-11-17 — Require a declarative receiving specification

<a id="h-575"></a>H-575 · E:1930–1931

<a id="k-receiving-spec-20261117"></a>K-RECEIVING-SPEC-20261117 · E:1933–1934

## 2026-11-17 — Assess explicit receiving items and same-name omission

<a id="h-574"></a>H-574 · E:1938–1939

<a id="k-receiver-items-20261117"></a>K-RECEIVER-ITEMS-20261117 · E:1941–1942

## 2026-11-17 — Assess temporary mappings into existing result receivers

<a id="h-573"></a>H-573 · E:1946–1947

<a id="k-receiver-scope-20261117"></a>K-RECEIVER-SCOPE-20261117 · E:1949–1950

## 2026-11-17 — Identify callable-family coverage exhaustion

<a id="k-788"></a>K-788 · E:1954–1955

## 2026-11-17 — Assess return-only structural result signatures

<a id="h-572"></a>H-572 · E:1959–1960

<a id="k-structural-results-20261117"></a>K-STRUCTURAL-RESULTS-20261117 · E:1962–1963

## 2026-11-17 — Keep pending completion fields together

<a id="k-787"></a>K-787 · E:1967–1968

## 2026-11-17 — Select logical output destinations and named reception

<a id="h-571"></a>H-571 · E:1972–1973

<a id="k-output-accept-20261117"></a>K-OUTPUT-ACCEPT-20261117 · E:1975–1976

## 2026-11-17 — Reconcile result assessment with declaration completion

<a id="k-786"></a>K-786 · E:1980–1981

## 2026-11-17 — Preserve abrupt declaration initialization

<a id="k-785"></a>K-785 · E:1985–1986

## 2026-11-17 — Assess caller-provided outputs and named result construction

<a id="h-570"></a>H-570 · E:1990–1991

<a id="k-output-assessment-20261117"></a>K-OUTPUT-ASSESSMENT-20261117 · E:1993–1994

## 2026-11-17 — Keep native binding roles with their slots

<a id="k-784"></a>K-784 · E:1998–1999

## 2026-11-17 — Consolidated ordinary value-input review

<a id="k-783"></a>K-783 · E:2003–2004

## 2026-11-17 — Preserve declared places at the capture boundary

<a id="k-782"></a>K-782 · E:2008–2009

## 2026-11-17 — Partial named receiving and anonymous structural typing

<a id="h-569"></a>H-569 · E:2013–2014

## 2026-11-17 — Consolidate mainline evidence and the design checkpoint

<a id="k-design-mainline-20261117"></a>K-DESIGN-MAINLINE-20261117 · E:2019–2020

## 2026-11-17 — Callable storage, transfer semantics and anonymous-data direction

<a id="h-563"></a>H-563 · E:2027–2028

<a id="h-564"></a>H-564 · E:2030–2031

<a id="h-565"></a>H-565 · E:2033–2034

<a id="h-566"></a>H-566 · E:2036–2037

<a id="h-567"></a>H-567 · E:2039–2040

<a id="h-568"></a>H-568 · E:2042–2043

<a id="k-lexical-products-20261117"></a>K-LEXICAL-PRODUCTS-20261117 · E:2045–2046

## 2026-11-17 — Typed reservation uses the existing block distinction

<a id="h-562"></a>H-562 · E:2051–2052

<a id="k-typed-reservation-20261117"></a>K-TYPED-RESERVATION-20261117 · E:2054–2055

## 2026-11-17 — Accept owner-parameter reference symbols

<a id="h-561"></a>H-561 · E:2060–2061

<a id="k-owner-symbol-accept-20261117"></a>K-OWNER-SYMBOL-ACCEPT-20261117 · E:2063–2064

## 2026-11-17 — Owner-parameter reference-symbol feasibility

<a id="h-560"></a>H-560 · E:2069–2070

<a id="k-owner-symbol-cost-20261117"></a>K-OWNER-SYMBOL-COST-20261117 · E:2072–2073

## 2026-11-17 — Prepared-readiness and terminology integration

<a id="k-781"></a>K-781 · E:2078–2079

## 2026-11-17 — Canonical alias glossary and current design documents

<a id="h-558"></a>H-558 · E:2083–2084

<a id="k-alias-glossary-20261117"></a>K-ALIAS-GLOSSARY-20261117 · E:2086–2087

<a id="h-559"></a>H-559 · E:2089–2090

<a id="k-aot-contract-consolidation-20261117"></a>K-AOT-CONTRACT-CONSOLIDATION-20261117 · E:2092–2093

## 2026-11-17 — Declared place aliases versus runtime target identity

<a id="k-alias-layers-20261117"></a>K-ALIAS-LAYERS-20261117 · E:2097–2098

## 2026-11-17 — Explicit formal aliasing terminology

<a id="k-formal-aliasing-20261117"></a>K-FORMAL-ALIASING-20261117 · E:2102–2103

## 2026-11-17 — Preserve prepared source-target initialization

<a id="k-780"></a>K-780 · E:2107–2108

## 2026-11-17 — Explicit captures, aliases and lifetimes

<a id="h-553"></a>H-553 · E:2112–2113

<a id="h-554"></a>H-554 · E:2115–2116

<a id="h-555"></a>H-555 · E:2118–2119

<a id="h-556"></a>H-556 · E:2121–2122

<a id="h-557"></a>H-557 · E:2124–2125

<a id="k-capture-lifetimes-20261117"></a>K-CAPTURE-LIFETIMES-20261117 · E:2127–2128

## 2026-11-17 — Preserve checked native emission contexts

<a id="k-779"></a>K-779 · E:2132–2133

## 2026-11-17 — Swap-method signature inspection

<a id="k-778"></a>K-778 · E:2137–2138

## 2026-11-16 — Caller-access and defer-nesting integration

<a id="k-777"></a>K-777 · E:2142–2143

## 2026-11-16 — Caller-binding target-access preservation

<a id="k-776"></a>K-776 · E:2147–2148

## 2026-11-16 — Reject nested defer declarations

<a id="h-552"></a>H-552 · E:2152–2153

<a id="k-defer-nesting-20261116"></a>K-DEFER-NESTING-20261116 · E:2155–2156

## 2026-11-16 — Native-validation and deferred-coverage integration

<a id="k-775"></a>K-775 · E:2160–2161

## 2026-11-16 — Independent native validation fixtures

<a id="k-774"></a>K-774 · E:2165–2166

## 2026-11-16 — Require checked handling before defer-pass completion

<a id="h-550"></a>H-550 · E:2170–2171

<a id="h-551"></a>H-551 · E:2173–2174

<a id="k-defer-checked-coverage-20261116"></a>K-DEFER-CHECKED-COVERAGE-20261116 · E:2176–2177

## 2026-11-16 — Return-pattern typing and materialization

<a id="h-549"></a>H-549 · E:2181–2182

<a id="k-returning-binding-20261116"></a>K-RETURNING-BINDING-20261116 · E:2184–2185

## 2026-11-16 — Bounded return-inspection profile

<a id="h-548"></a>H-548 · E:2189–2190

<a id="k-returning-profile-20261116"></a>K-RETURNING-PROFILE-20261116 · E:2192–2193

## 2026-11-16 — Borrowed-result initialization

<a id="k-773"></a>K-773 · E:2197–2198

## 2026-11-16 — Prepared initialization provenance

<a id="k-772"></a>K-772 · E:2202–2203

## 2026-11-16 — Inductive selected-target initialization

<a id="k-771"></a>K-771 · E:2207–2208

<a id="k-770"></a>K-770 · E:2210–2211

## 2026-11-16 — Selected identities and concrete targets

<a id="k-769"></a>K-769 · E:2215–2216

## 2026-11-16 — Complete reference-comparison evidence

<a id="k-768"></a>K-768 · E:2220–2221

## 2026-11-16 — Borrowed selected initialization

<a id="k-767"></a>K-767 · E:2225–2226

## 2026-11-16 — Conditional selected initialization

<a id="k-766"></a>K-766 · E:2230–2231

## 2026-11-16 — Nullable reference joins

<a id="k-765"></a>K-765 · E:2235–2236

## 2026-11-16 — Selected-reference equality proofs

<a id="k-764"></a>K-764 · E:2240–2241

## 2026-11-16 — Returned selection initialization

<a id="k-763"></a>K-763 · E:2245–2246

## 2026-11-16 — Nullable result conditions

<a id="k-762"></a>K-762 · E:2250–2251

## 2026-11-16 — Return-state design and compiler evidence integration

<a id="k-returning-sync-20261116"></a>K-RETURNING-SYNC-20261116 · E:2255–2256

## 2026-11-16 — Return inspection, explicit recovery and scoped cancellation

<a id="h-542"></a>H-542 · E:2260–2261

<a id="h-543"></a>H-543 · E:2263–2264

<a id="h-544"></a>H-544 · E:2266–2267

<a id="h-545"></a>H-545 · E:2269–2270

<a id="h-546"></a>H-546 · E:2272–2273

<a id="h-547"></a>H-547 · E:2275–2276

<a id="k-returning-20261116"></a>K-RETURNING-20261116 · E:2278–2279

## 2026-11-16 — Copied nullable reference evidence

<a id="k-761"></a>K-761 · E:2283–2284

## 2026-11-16 — Nullable input predicate evidence

<a id="k-760"></a>K-760 · E:2288–2289

## 2026-11-16 — Exception-derived predicate evidence

<a id="k-759"></a>K-759 · E:2293–2294

## 2026-11-16 — Consistent initialization conditions

<a id="k-758"></a>K-758 · E:2298–2299

## 2026-11-16 — Conditional initialization and deferred FIFO integration

<a id="k-defer-fifo-sync-20261116"></a>K-DEFER-FIFO-SYNC-20261116 · E:2303–2304

## 2026-11-16 — Conditional initialization across completion boundaries

<a id="k-757"></a>K-757 · E:2308–2309

## 2026-11-16 — FIFO pending deferred failures

<a id="h-541"></a>H-541 · E:2312–2313

<a id="k-defer-fifo-20261116"></a>K-DEFER-FIFO-20261116 · E:2315–2316

## 2026-11-16 — Retained return results with queued deferred handling

<a id="h-539"></a>H-539 · E:2320–2321

<a id="h-540"></a>H-540 · E:2323–2324

<a id="k-defer-return-order-20261116"></a>K-DEFER-RETURN-ORDER-20261116 · E:2326–2327

## 2026-11-16 — Integrate retained body-failure design with conditional-read checking

<a id="k-body-exception-sync-20261116"></a>K-BODY-EXCEPTION-SYNC-20261116 · E:2331–2332

## 2026-11-16 — Preserve the ordinary body exception beside deferred failures

<a id="h-538"></a>H-538 · E:2336–2337

<a id="k-defer-body-exception-20261116"></a>K-DEFER-BODY-EXCEPTION-20261116 · E:2339–2340

## 2026-11-16 — Conditional initialization in deferred actions

<a id="k-756"></a>K-756 · E:2344–2345

## 2026-11-16 — Integrate declared-defer design with compiler enforcement

<a id="k-declared-defer-sync-20261116"></a>K-DECLARED-DEFER-SYNC-20261116 · E:2349–2350

## 2026-11-16 — Declared checked failures from deferred work

<a id="h-536"></a>H-536 · E:2354–2355

<a id="h-537"></a>H-537 · E:2357–2358

<a id="k-defer-throws-20261116"></a>K-DEFER-THROWS-20261116 · E:2360–2361

## 2026-11-16 — Selected payload proofs and declared failure obligations

<a id="k-755"></a>K-755 · E:2365–2366

## 2026-11-16 — Preserve declared exception obligations during proof improvements

<a id="k-checked-declaration-boundary-20261116"></a>K-CHECKED-DECLARATION-BOUNDARY-20261116 · E:2370–2371

## 2026-11-16 — Integrate endpoint constraints and boolean output evidence

<a id="k-disjoint-endpoint-sync-20261116"></a>K-DISJOINT-ENDPOINT-SYNC-20261116 · E:2375–2376

## 2026-11-16 — Boolean output evidence through calls and cleanup

<a id="k-754"></a>K-754 · E:2380–2381

## 2026-11-16 — Endpoint ancestry, independent parents and shared constraint information

<a id="h-533"></a>H-533 · E:2385–2386

<a id="h-534"></a>H-534 · E:2388–2389

<a id="h-535"></a>H-535 · E:2391–2392

<a id="k-disjoint-endpoints-20261116"></a>K-DISJOINT-ENDPOINTS-20261116 · E:2394–2395

## 2026-11-16 — Per-pair authority for disjoint declarations

<a id="h-532"></a>H-532 · E:2399–2400

<a id="k-disjoint-authority-20261116"></a>K-DISJOINT-AUTHORITY-20261116 · E:2402–2403

## 2026-11-16 — Integrate disjoint-path design and boolean input proofs

<a id="k-disjoint-path-sync-20261116"></a>K-DISJOINT-PATH-SYNC-20261116 · E:2407–2408

## 2026-11-16 — Preserve boolean input evidence across call results

<a id="k-753"></a>K-753 · E:2412–2413

## 2026-11-16 — N-ary disjoint constraints, exempt paths and method-aware warnings

<a id="h-527"></a>H-527 · E:2417–2418

<a id="h-528"></a>H-528 · E:2420–2421

<a id="h-529"></a>H-529 · E:2423–2424

<a id="h-530"></a>H-530 · E:2426–2427

<a id="h-531"></a>H-531 · E:2429–2430

<a id="k-disjoint-paths-20261116"></a>K-DISJOINT-PATHS-20261116 · E:2432–2433

## 2026-11-16 — Unchecked Error root and explicit checked declaration contracts

<a id="h-524"></a>H-524 · E:2437–2438

<a id="h-525"></a>H-525 · E:2440–2441

<a id="h-526"></a>H-526 · E:2443–2444

<a id="k-error-roots-20261116"></a>K-ERROR-ROOTS-20261116 · E:2446–2447

## 2026-11-16 — Preserve wrapper checkedness and suggest explicit automatic rethrow

<a id="h-523"></a>H-523 · E:2451–2452

<a id="k-rethrow-from-20261116"></a>K-RETHROW-FROM-20261116 · E:2454–2455

## 2026-11-16 — Integrate exception identity and boolean-result evidence

<a id="k-exception-identity-sync-20261116"></a>K-EXCEPTION-IDENTITY-SYNC-20261116 · E:2459–2460

## 2026-11-16 — Preserve non-null proofs through boolean helper results

<a id="k-752"></a>K-752 · E:2464–2465

## 2026-11-16 — Transparent rethrow matching and catch-local exception identity

<a id="h-518"></a>H-518 · E:2468–2469

<a id="h-519"></a>H-519 · E:2471–2472

<a id="h-520"></a>H-520 · E:2474–2475

<a id="h-521"></a>H-521 · E:2477–2478

<a id="h-522"></a>H-522 · E:2480–2481

<a id="k-wrapper-catch-20261116"></a>K-WRAPPER-CATCH-20261116 · E:2483–2484

## 2026-11-16 — Explicit exception construction and bounded OOM diagnostics

<a id="h-510"></a>H-510 · E:2488–2489

<a id="h-511"></a>H-511 · E:2491–2492

<a id="h-512"></a>H-512 · E:2494–2495

<a id="h-513"></a>H-513 · E:2497–2498

<a id="h-514"></a>H-514 · E:2500–2501

<a id="h-515"></a>H-515 · E:2503–2504

<a id="h-516"></a>H-516 · E:2506–2507

<a id="h-517"></a>H-517 · E:2509–2510

<a id="k-exception-construction-20261116"></a>K-EXCEPTION-CONSTRUCTION-20261116 · E:2512–2513

## 2026-11-16 — Retain incoming-target presence across calls

<a id="k-751"></a>K-751 · E:2517–2518

## 2026-11-16 — Integrate deferred-stack decisions with identity guards

<a id="k-defer-stack-sync-20261116"></a>K-DEFER-STACK-SYNC-20261116 · E:2522–2523

## 2026-11-16 — Preserve identity guard evidence

<a id="k-750"></a>K-750 · E:2527–2528

## 2026-11-16 — Separate deferred errors and scan them at handler entries

<a id="h-503"></a>H-503 · E:2532–2533

<a id="h-504"></a>H-504 · E:2535–2536

<a id="h-505"></a>H-505 · E:2538–2539

<a id="h-506"></a>H-506 · E:2541–2542

<a id="h-507"></a>H-507 · E:2544–2545

<a id="h-508"></a>H-508 · E:2547–2548

<a id="h-509"></a>H-509 · E:2550–2551

<a id="k-defer-stack-20261116"></a>K-DEFER-STACK-20261116 · E:2553–2554

## 2026-11-16 — Mark the defer handler contract under revision

<a id="k-defer-contract-notice-20261116"></a>K-DEFER-CONTRACT-NOTICE-20261116 · E:2558–2559

## 2026-11-16 — Integrate module visibility and cleanup decisions

<a id="k-module-drain-sync-20261116"></a>K-MODULE-DRAIN-SYNC-20261116 · E:2563–2564

## 2026-11-16 — Close call effects through their dependencies

<a id="k-749"></a>K-749 · E:2568–2569

## 2026-11-16 — Integrate the startup visibility and cleanup batch

<a id="k-startup-batch-integration-20261116"></a>K-STARTUP-BATCH-INTEGRATION-20261116 · E:2573–2574

## 2026-11-16 — Initialization blocks determine binding visibility

<a id="h-502"></a>H-502 · E:2578–2579

<a id="k-block-binding-visibility-20261116"></a>K-BLOCK-BINDING-VISIBILITY-20261116 · E:2581–2582

## 2026-11-16 — Global order for module-level deferred cleanup

<a id="h-501"></a>H-501 · E:2586–2587

<a id="k-module-shutdown-20261116"></a>K-MODULE-SHUTDOWN-20261116 · E:2589–2590

## 2026-11-16 — Developer review of initialization publication and shutdown

<a id="k-startup-peer-review-20261116"></a>K-STARTUP-PEER-REVIEW-20261116 · E:2594–2595

## 2026-11-16 — Retain stored null-test evidence

<a id="k-748"></a>K-748 · E:2599–2600

## 2026-11-16 — Reconsider result evidence through its dependencies

<a id="k-747"></a>K-747 · E:2604–2605

## 2026-11-16 — Retain definition evidence by dependency

<a id="k-746"></a>K-746 · E:2609–2610

## 2026-11-16 — Rebase startup decisions onto current compiler work

<a id="k-design-startup-sync-20261116"></a>K-DESIGN-STARTUP-SYNC-20261116 · E:2614–2615

## 2026-11-16 — Static startup and namespace-selected entries

<a id="h-495"></a>H-495 · E:2619–2620

<a id="h-496"></a>H-496 · E:2622–2623

<a id="h-497"></a>H-497 · E:2625–2626

<a id="h-498"></a>H-498 · E:2628–2629

<a id="h-499"></a>H-499 · E:2631–2632

<a id="h-500"></a>H-500 · E:2634–2635

<a id="k-static-module-startup-20261116"></a>K-STATIC-MODULE-STARTUP-20261116 · E:2637–2638

## 2026-11-16 — Declaration cycles and ordered postponed initialization

<a id="h-492"></a>H-492 · E:2642–2643

<a id="h-493"></a>H-493 · E:2645–2646

<a id="h-494"></a>H-494 · E:2648–2649

<a id="k-module-init-review-20261116"></a>K-MODULE-INIT-REVIEW-20261116 · E:2651–2652

## 2026-11-16 — Construct callable views from definition context

<a id="k-745"></a>K-745 · E:2656–2657

## 2026-11-16 — Integrate reference-symbol and module-selection decisions

<a id="k-744"></a>K-744 · E:2661–2662

## 2026-11-16 — Reconcile callable construction with the synchronized decision

<a id="k-743"></a>K-743 · E:2666–2667

## 2026-11-16 — Resolve cleanup failure categories

<a id="k-742"></a>K-742 · E:2671–2672

## 2026-11-16 — Loop exception payload invariants

<a id="k-741"></a>K-741 · E:2676–2677

## 2026-11-16 — Callable mention conformance boundary

<a id="k-740"></a>K-740 · E:2681–2682

## 2026-11-16 — Callable mention and compiler-scope integration

<a id="k-739"></a>K-739 · E:2686–2687

## 2026-11-16 — Preserve invariant loop targets

<a id="k-738"></a>K-738 · E:2691–2692

## 2026-11-16 — Explicit source-module traversal

<a id="k-737"></a>K-737 · E:2696–2697

## 2026-11-16 — Explicit callable bundles and independent re-export

<a id="h-488"></a>H-488 · E:2700–2701

<a id="h-489"></a>H-489 · E:2703–2704

<a id="h-490"></a>H-490 · E:2706–2707

<a id="h-491"></a>H-491 · E:2709–2710

<a id="k-reexport-methods-20261116"></a>K-REEXPORT-METHODS-20261116 · E:2712–2713

## 2026-11-16 — Source fallback, private barriers and deferred diagnostics

<a id="h-484"></a>H-484 · E:2717–2718

<a id="h-485"></a>H-485 · E:2720–2721

<a id="h-486"></a>H-486 · E:2723–2724

<a id="h-487"></a>H-487 · E:2726–2727

<a id="k-source-fallback-20261116"></a>K-SOURCE-FALLBACK-20261116 · E:2729–2730

## 2026-11-16 — Narrow the type-selection question queue

<a id="k-type-selection-review-20261116"></a>K-TYPE-SELECTION-REVIEW-20261116 · E:2734–2735

## 2026-11-16 — Type selections carry shadowable method names

<a id="h-483"></a>H-483 · E:2739–2740

<a id="k-type-method-names-20261116"></a>K-TYPE-METHOD-NAMES-20261116 · E:2742–2743

## 2026-11-16 — Explicit regional reference symbols

<a id="h-482"></a>H-482 · E:2747–2748

<a id="k-reference-symbols-20261116"></a>K-REFERENCE-SYMBOLS-20261116 · E:2750–2751

## 2026-11-16 — Definition mentions construct callable views

<a id="h-481"></a>H-481 · E:2755–2756

<a id="k-callable-mentions-20261116"></a>K-CALLABLE-MENTIONS-20261116 · E:2758–2759

## 2026-11-16 — ADT placement and mainline integration

<a id="k-736"></a>K-736 · E:2763–2764

## 2026-11-15 — Keep only the quirk note and user workaround

<a id="h-480"></a>H-480 · E:2768–2769

<a id="k-quirk-workaround-20261115"></a>K-QUIRK-WORKAROUND-20261115 · E:2771–2772

## 2026-11-15 — Keep the first-field self-reference quirk nonblocking

<a id="h-479"></a>H-479 · E:2776–2777

<a id="k-self-ref-quirk-20261115"></a>K-SELF-REF-QUIRK-20261115 · E:2779–2780

## 2026-11-15 — One nominal ADT across storage domains

<a id="h-478"></a>H-478 · E:2784–2785

<a id="k-single-adt-20261115"></a>K-SINGLE-ADT-20261115 · E:2787–2788

## 2026-11-15 — Handler queues and finally composition

<a id="k-735"></a>K-735 · E:2792–2793

## 2026-11-15 — Deferred handler payload state

<a id="k-734"></a>K-734 · E:2797–2798

## 2026-11-15 — Cleanup payload transformations

<a id="k-733"></a>K-733 · E:2802–2803

## 2026-11-15 — Borrowed exception call postconditions

<a id="k-732"></a>K-732 · E:2807–2808

## 2026-11-15 — Caught exception-value evidence

<a id="k-731"></a>K-731 · E:2812–2813

## 2026-11-15 — Callable exception-result evidence

<a id="k-730"></a>K-730 · E:2817–2818

## 2026-11-15 — Exception result read timing

<a id="k-729"></a>K-729 · E:2822–2823

## 2026-11-15 — Borrowed exception payload evidence

<a id="k-728"></a>K-728 · E:2827–2828

## 2026-11-15 — Alias selector completion

<a id="k-727"></a>K-727 · E:2832–2833

## 2026-11-15 — Declared builtin input evidence

<a id="k-726"></a>K-726 · E:2837–2838

## 2026-11-15 — Composed null-test evidence

<a id="k-725"></a>K-725 · E:2842–2843

## 2026-11-15 — Recursive type-only signatures

<a id="k-724"></a>K-724 · E:2847–2848

## 2026-11-15 — Recursive declaration evidence

<a id="k-723"></a>K-723 · E:2852–2853

## 2026-11-15 — Default selection and inference integration

<a id="k-722"></a>K-722 · E:2857–2858

## 2026-11-15 — Select the overload before analyzing defaults

<a id="h-477"></a>H-477 · E:2862–2863

<a id="k-post-selection-defaults-20261115"></a>K-POST-SELECTION-DEFAULTS-20261115 · E:2865–2866

## 2026-11-15 — Generic inference from established default types

<a id="h-476"></a>H-476 · E:2870–2871

<a id="k-default-type-evidence-20261115"></a>K-DEFAULT-TYPE-EVIDENCE-20261115 · E:2873–2874

## 2026-11-15 — Explicit choice between defaults and variadic packs

<a id="h-475"></a>H-475 · E:2878–2879

<a id="k-explicit-default-selection-20261115"></a>K-EXPLICIT-DEFAULT-SELECTION-20261115 · E:2881–2882

## 2026-11-15 — Default-ranking contract integration

<a id="k-721"></a>K-721 · E:2885–2886

## 2026-11-15 — Default-count overload priority

<a id="h-474"></a>H-474 · E:2890–2891

<a id="k-default-ranking-20261115"></a>K-DEFAULT-RANKING-20261115 · E:2893–2894

## 2026-11-15 — Canonical failures and common-root catches

<a id="k-720"></a>K-720 · E:2898–2899

## 2026-11-15 — Assertion and failure-name contract integration

<a id="k-719"></a>K-719 · E:2903–2904

## 2026-11-15 — Callable target/value receiving

<a id="k-718"></a>K-718 · E:2908–2909

## 2026-11-15 — Numeric conversion contract integration

<a id="k-717"></a>K-717 · E:2913–2914

## 2026-11-15 — Callable payloads through caller slots

<a id="k-716"></a>K-716 · E:2918–2919

## 2026-11-15 — Callable contract integration

<a id="k-715"></a>K-715 · E:2923–2924

## 2026-11-15 — Built-in failure names and assertion responsibility

<a id="h-472"></a>H-472 · E:2927–2928

<a id="h-473"></a>H-473 · E:2930–2931

<a id="k-failure-names-20261115"></a>K-FAILURE-NAMES-20261115 · E:2933–2934

## 2026-11-15 — Concrete-type specialization and explicit predicate assertions

<a id="h-468"></a>H-468 · E:2938–2939

<a id="h-469"></a>H-469 · E:2941–2942

<a id="h-470"></a>H-470 · E:2944–2945

<a id="h-471"></a>H-471 · E:2947–2948

<a id="k-assert-specialization-20261115"></a>K-ASSERT-SPECIALIZATION-20261115 · E:2950–2951

## 2026-11-15 — Unchecked nonnumeric type assertions

<a id="h-467"></a>H-467 · E:2955–2956

<a id="k-type-assertion-review-20261115"></a>K-TYPE-ASSERTION-REVIEW-20261115 · E:2958–2959

## 2026-11-15 — Undefined behavior for invalid explicit numeric conversion

<a id="h-466"></a>H-466 · E:2963–2964

<a id="k-numeric-conversion-ub-20261115"></a>K-NUMERIC-CONVERSION-UB-20261115 · E:2966–2967

## 2026-11-15 — No runtime error after numeric conversion warnings

<a id="h-465"></a>H-465 · E:2971–2972

<a id="k-unchecked-conversion-20261115"></a>K-UNCHECKED-CONVERSION-20261115 · E:2974–2975

## 2026-11-15 — Explicit lossy numeric conversion

<a id="h-463"></a>H-463 · E:2979–2980

<a id="h-464"></a>H-464 · E:2982–2983

<a id="k-lossy-conversion-20261115"></a>K-LOSSY-CONVERSION-20261115 · E:2985–2986

## 2026-11-15 — Complete multi-method callable contracts

<a id="h-462"></a>H-462 · E:2990–2991

<a id="k-callable-family-contract-20261115"></a>K-CALLABLE-FAMILY-CONTRACT-20261115 · E:2993–2994

## 2026-11-15 — Type-bound default expressions

<a id="h-461"></a>H-461 · E:2998–2999

<a id="k-type-default-expression-20261115"></a>K-TYPE-DEFAULT-EXPRESSION-20261115 · E:3001–3002

## 2026-11-15 — Compatible callable targets and default-provider review

<a id="h-460"></a>H-460 · E:3006–3007

<a id="k-compatible-default-targets-20261115"></a>K-COMPATIBLE-DEFAULT-TARGETS-20261115 · E:3009–3010

## 2026-11-15 — Named callable origin for default expressions

<a id="h-459"></a>H-459 · E:3014–3015

<a id="k-named-default-origin-20261115"></a>K-NAMED-DEFAULT-ORIGIN-20261115 · E:3017–3018

## 2026-11-15 — Default evaluation phases

<a id="h-458"></a>H-458 · E:3022–3023

<a id="k-default-phase-20261115"></a>K-DEFAULT-PHASE-20261115 · E:3025–3026

## 2026-11-15 — Scalar target/value receiving

<a id="k-714"></a>K-714 · E:3029–3030

## 2026-11-15 — Assignment queries and target-only results

<a id="k-713"></a>K-713 · E:3034–3035

## 2026-11-15 — Assignment results retain only target views

<a id="h-assignment-result-20261115"></a>H-ASSIGNMENT-RESULT-20261115 · E:3039–3040

## 2026-11-15 — Assignment-result capability review

<a id="k-712"></a>K-712 · E:3044–3045

## 2026-11-15 — Storage-exchange type inspection

<a id="k-711"></a>K-711 · E:3049–3050

## 2026-11-15 — Argument design checkpoint integration

<a id="k-710"></a>K-710 · E:3054–3055

## 2026-11-15 — Catchable source bounds failures

<a id="k-709"></a>K-709 · E:3059–3060

## 2026-11-15 — Variadic design checkpoint integration

<a id="k-708"></a>K-708 · E:3064–3065

## 2026-11-15 — Checked eager operation identity

<a id="k-707"></a>K-707 · E:3069–3070

## 2026-11-15 — Bounds design checkpoint integration

<a id="k-706"></a>K-706 · E:3074–3075

## 2026-11-15 — Selected operation aliases

<a id="k-705"></a>K-705 · E:3079–3080

## 2026-11-15 — Argument arrows and default environments

<a id="h-453"></a>H-453 · E:3084–3085

<a id="h-454"></a>H-454 · E:3087–3088

<a id="h-455"></a>H-455 · E:3090–3091

<a id="h-456"></a>H-456 · E:3093–3094

<a id="h-457"></a>H-457 · E:3096–3097

<a id="k-argument-contexts-20261115"></a>K-ARGUMENT-CONTEXTS-20261115 · E:3099–3100

## 2026-11-15 — Named arguments and callable parameter preamble

<a id="h-451"></a>H-451 · E:3104–3105

<a id="h-452"></a>H-452 · E:3107–3108

<a id="k-named-arguments-20261115"></a>K-NAMED-ARGUMENTS-20261115 · E:3110–3111

## 2026-11-15 — Fixed and variadic overload ranking

<a id="h-450"></a>H-450 · E:3115–3116

<a id="k-variadic-ranking-20261115"></a>K-VARIADIC-RANKING-20261115 · E:3118–3119

## 2026-11-15 — Array expansion and one variadic parameter

<a id="h-449"></a>H-449 · E:3123–3124

<a id="k-variadic-pack-20261115"></a>K-VARIADIC-PACK-20261115 · E:3126–3127

## 2026-11-15 — Unchecked bounds exceptions and explicit check opt-out

<a id="h-448"></a>H-448 · E:3131–3132

<a id="k-bounds-policy-20261115"></a>K-BOUNDS-POLICY-20261115 · E:3134–3135

## 2026-11-15 — Initialization design checkpoint integration

<a id="k-704"></a>K-704 · E:3139–3140

## 2026-11-15 — Prepared caller-binding methods

<a id="k-703"></a>K-703 · E:3144–3145

## 2026-11-15 — Reference identity and shared owner inputs

<a id="k-702"></a>K-702 · E:3149–3150

## 2026-11-15 — Confirmed reference-design integration

<a id="k-701"></a>K-701 · E:3154–3155

## 2026-11-15 — Caller-binding methods

<a id="k-700"></a>K-700 · E:3159–3160

## 2026-11-14 — Builtin operand completion

<a id="k-699"></a>K-699 · E:3164–3165

## 2026-11-14 — Deferred reference exchange

<a id="k-698"></a>K-698 · E:3169–3170

## 2026-11-14 — Prepared assignment cleanup

<a id="k-697"></a>K-697 · E:3174–3175

## 2026-11-14 — Explicit assignment forms

<a id="k-696"></a>K-696 · E:3179–3180

## 2026-11-14 — Symbolic query signature validation

<a id="k-695"></a>K-695 · E:3184–3185

## 2026-11-14 — Partially implemented family queries

<a id="k-694"></a>K-694 · E:3189–3190

## 2026-11-14 — Callable signature inspection

<a id="k-693"></a>K-693 · E:3194–3195

## 2026-11-14 — Contextual declaration queries

<a id="k-692"></a>K-692 · E:3199–3200

## 2026-11-14 — Exact common-exception borrowing

<a id="k-691"></a>K-691 · E:3204–3205

## 2026-11-14 — Dependent exception result obligations

<a id="k-690"></a>K-690 · E:3209–3210

## 2026-11-14 — Exception declaration overlap

<a id="k-689"></a>K-689 · E:3214–3215

## 2026-11-14 — Numeric declaration overlap

<a id="k-688"></a>K-688 · E:3219–3220

## 2026-11-14 — Overlapping declaration contracts

<a id="k-687"></a>K-687 · E:3224–3225

## 2026-11-14 — Caught-value owner evidence

<a id="k-686"></a>K-686 · E:3229–3230

## 2026-11-14 — Loop-control owner evidence

<a id="k-685"></a>K-685 · E:3234–3235

## 2026-11-14 — Scoped owner evidence

<a id="k-684"></a>K-684 · E:3239–3240

## 2026-11-14 — Shared symbolic owner proof

<a id="k-683"></a>K-683 · E:3244–3245

## 2026-11-14 — Callable-view target access

<a id="k-682"></a>K-682 · E:3249–3250

## 2026-11-14 — Shared lexical shadow diagnostics

<a id="k-681"></a>K-681 · E:3254–3255

## 2026-11-14 — Lexical callable name resolution

<a id="k-680"></a>K-680 · E:3259–3260

## 2026-11-14 — Lexical callable family coverage

<a id="k-679"></a>K-679 · E:3264–3265

## 2026-11-14 — Ordinary module-family values

<a id="k-678"></a>K-678 · E:3269–3270

## 2026-11-14 — Family value-result adaptation

<a id="k-677"></a>K-677 · E:3274–3275

## 2026-11-14 — Independent return-expression alternatives

<a id="k-676"></a>K-676 · E:3279–3280

## 2026-11-14 — Checked module initialization planning

<a id="k-675"></a>K-675 · E:3284–3285

## 2026-11-14 — Ordinary callable import interpretation

<a id="k-674"></a>K-674 · E:3289–3290

## 2026-11-14 — Source-preserving module contexts

<a id="k-673"></a>K-673 · E:3295–3296

## 2026-11-14 — Family input contracts

<a id="k-672"></a>K-672 · E:3300–3301

## 2026-11-14 — Symbolic family source execution

<a id="k-671"></a>K-671 · E:3306–3307

## 2026-11-14 — Exception-valued dynamic bindings

<a id="k-670"></a>K-670 · E:3311–3312

## 2026-11-14 — Complete dynamic captures and callable replay

<a id="k-669"></a>K-669 · E:3316–3317

## 2026-11-14 — Primitive dynamic binding and restoration

<a id="k-668"></a>K-668 · E:3321–3322

## 2026-11-14 — Typed symbol declaration evidence

<a id="k-667"></a>K-667 · E:3326–3327

## 2026-11-14 — Shared reference-intrinsic invocation

<a id="k-666"></a>K-666 · E:3331–3332

## 2026-11-14 — Lossless fold updates

<a id="k-665"></a>K-665 · E:3336–3337

## 2026-11-14 — Call-summary input roles

<a id="k-664"></a>K-664 · E:3341–3342

## 2026-11-14 — Checked callable frames

<a id="k-663"></a>K-663 · E:3346–3347

## 2026-11-14 — Variadic declaration contracts

<a id="k-662"></a>K-662 · E:3351–3352

## 2026-11-15 — Confirm staged initialization plan and synchronize development

<a id="h-447"></a>H-447 · E:3355–3356

<a id="k-init-confirm-20261115"></a>K-INIT-CONFIRM-20261115 · E:3358–3359

## 2026-11-15 — Staged acceptance with invariant soundness

<a id="h-446"></a>H-446 · E:3363–3364

<a id="k-init-stages-20261115"></a>K-INIT-STAGES-20261115 · E:3366–3367

## 2026-11-15 — Combined initialization-flow cost review

<a id="k-init-cost-20261115"></a>K-INIT-COST-20261115 · E:3371–3372

## 2026-11-15 — Initialization obligations throughout control flow

<a id="h-445"></a>H-445 · E:3376–3377

<a id="k-init-flow-20261115"></a>K-INIT-FLOW-20261115 · E:3379–3380

## 2026-11-15 — Cyclic construction cost and future pure knot-tying

<a id="h-444"></a>H-444 · E:3384–3385

<a id="k-knot-cost-20261115"></a>K-KNOT-COST-20261115 · E:3387–3388

## 2026-11-15 — Foreign interfaces adapt to CoD semantics

<a id="h-443"></a>H-443 · E:3392–3393

<a id="k-foreign-adapters-20261115"></a>K-FOREIGN-ADAPTERS-20261115 · E:3395–3396

## 2026-11-14 — Unary negation spellings

<a id="h-442"></a>H-442 · E:3400–3401

<a id="k-unary-not-20261114"></a>K-UNARY-NOT-20261114 · E:3403–3404

## 2026-11-14 — Identity negation

<a id="h-441"></a>H-441 · E:3408–3409

<a id="k-is-not-20261114"></a>K-IS-NOT-20261114 · E:3411–3412

## 2026-11-14 — Reference identity and unified null tests

<a id="h-439"></a>H-439 · E:3416–3417

<a id="h-440"></a>H-440 · E:3419–3420

<a id="k-reference-identity-20261114"></a>K-REFERENCE-IDENTITY-20261114 · E:3422–3423

## 2026-11-14 — Owner qualifiers govern allocation

<a id="h-438"></a>H-438 · E:3427–3428

<a id="k-owner-allocation-20261114"></a>K-OWNER-ALLOCATION-20261114 · E:3430–3431

## 2026-11-14 — Shared actual-owner runtime information

<a id="h-437"></a>H-437 · E:3435–3436

<a id="k-shared-owner-20261114"></a>K-SHARED-OWNER-20261114 · E:3438–3439

## 2026-11-14 — Integrated pointer and lexical ADT design review

<a id="k-pointer-global-20261114"></a>K-POINTER-GLOBAL-20261114 · E:3443–3444

## 2026-11-14 — Lexical pointer checkpoint integration

<a id="k-lexical-checkpoint-sync-20261114"></a>K-LEXICAL-CHECKPOINT-SYNC-20261114 · E:3448–3449

## 2026-11-14 — Lexical slot location and reference contents

<a id="h-436"></a>H-436 · E:3453–3454

<a id="k-lexical-slot-payload-20261114"></a>K-LEXICAL-SLOT-PAYLOAD-20261114 · E:3456–3457

## 2026-11-14 — Adopt owner-base lexical references

<a id="h-435"></a>H-435 · E:3461–3462

<a id="k-scope-offset-adopt-20261114"></a>K-SCOPE-OFFSET-ADOPT-20261114 · E:3464–3465

## 2026-11-14 — Lexical owner-base offsets

<a id="h-434"></a>H-434 · E:3469–3470

<a id="k-scope-offsets-20261114"></a>K-SCOPE-OFFSETS-20261114 · E:3472–3473

## 2026-11-14 — Lexical pointer-bearing ADT feasibility

<a id="h-433"></a>H-433 · E:3477–3478

<a id="k-pointer-layout-20261114"></a>K-POINTER-LAYOUT-20261114 · E:3480–3481

## 2026-11-14 — Lossless method selection

<a id="k-661"></a>K-661 · E:3485–3486

## 2026-11-14 — Lossless numeric value preparation

<a id="k-660"></a>K-660 · E:3490–3491

## 2026-11-14 — Declaration input-domain proofs

<a id="k-659"></a>K-659 · E:3495–3496

## 2026-11-14 — Resize design and mainline synchronization

<a id="k-resize-sync-20261114"></a>K-RESIZE-SYNC-20261114 · E:3500–3501

## 2026-11-14 — Lossless consuming region resize

<a id="h-432"></a>H-432 · E:3505–3506

<a id="k-resize-20261114"></a>K-RESIZE-20261114 · E:3508–3509

## 2026-11-14 — Explicit consuming region growth

<a id="h-430"></a>H-430 · E:3513–3514

<a id="h-431"></a>H-431 · E:3516–3517

<a id="k-explicit-grow-20261114"></a>K-EXPLICIT-GROW-20261114 · E:3519–3520

## 2026-11-14 — Operator signature obligations

<a id="k-658"></a>K-658 · E:3524–3525

## 2026-11-14 — Independent expected callable results

<a id="k-657"></a>K-657 · E:3529–3530

## 2026-11-14 — Distinct regional and lexical storage

<a id="h-429"></a>H-429 · E:3534–3535

<a id="k-storage-split-20261114"></a>K-STORAGE-SPLIT-20261114 · E:3537–3538

## 2026-11-14 — Initialization allocation is an allowed effect

<a id="h-428"></a>H-428 · E:3542–3543

<a id="k-init-effects-20261114"></a>K-INIT-EFFECTS-20261114 · E:3545–3546

## 2026-11-14 — Value normalization, group checkpoints and scope regions

<a id="h-426"></a>H-426 · E:3550–3551

<a id="h-427"></a>H-427 · E:3553–3554

<a id="k-scope-region-20261114"></a>K-SCOPE-REGION-20261114 · E:3556–3557

## 2026-11-14 — Unified initialization review

<a id="h-425"></a>H-425 · E:3561–3562

<a id="k-unified-init-20261114"></a>K-UNIFIED-INIT-20261114 · E:3564–3565

## 2026-11-14 — Declared callable result contracts

<a id="k-656"></a>K-656 · E:3569–3570

## 2026-11-14 — Independent callable result evidence

<a id="k-655"></a>K-655 · E:3574–3575

## 2026-11-14 — Common-result compiler-reference alignment

<a id="k-654"></a>K-654 · E:3579–3580

## 2026-11-14 — Unique least common callable results

<a id="h-424"></a>H-424 · E:3584–3585

<a id="k-common-result-20261114"></a>K-COMMON-RESULT-20261114 · E:3587–3588

## 2026-11-14 — Callable result-contract design integration

<a id="k-653"></a>K-653 · E:3592–3593

## 2026-11-14 — Scalar results, receiving storage and null owners

<a id="k-652"></a>K-652 · E:3597–3598

## 2026-11-14 — Result contracts and focused design review

<a id="h-423"></a>H-423 · E:3602–3603

<a id="k-inference-cleanup-20261114"></a>K-INFERENCE-CLEANUP-20261114 · E:3605–3606

## 2026-11-14 — Confirmed scalar and owner design integration

<a id="k-651"></a>K-651 · E:3610–3611

## 2026-11-14 — Prepared builtin bits swaps

<a id="k-650"></a>K-650 · E:3615–3616

## 2026-11-14 — Bits-swap method dispatch

<a id="k-649"></a>K-649 · E:3620–3621

## 2026-11-14 — Cleanup target flow

<a id="k-648"></a>K-648 · E:3625–3626

## 2026-11-14 — Cleanup entry-target initialization

<a id="k-647"></a>K-647 · E:3630–3631

## 2026-11-14 — Cleanup target relationships

<a id="k-646"></a>K-646 · E:3635–3636

## 2026-11-14 — Selected-target cleanup initialization

<a id="k-645"></a>K-645 · E:3640–3641

## 2026-11-14 — Runtime-selected ordinary aliases

<a id="k-644"></a>K-644 · E:3645–3646

## 2026-11-14 — Outer lexical declaration evidence

<a id="k-643"></a>K-643 · E:3650–3651

## 2026-11-14 — Anonymous callable signature queries

<a id="k-642"></a>K-642 · E:3655–3656

## 2026-11-14 — Contextual callable type evidence

<a id="k-641"></a>K-641 · E:3660–3661

## 2026-11-14 — Compile-time type queries

<a id="k-640"></a>K-640 · E:3665–3666

## 2026-11-14 — Selected lexical reference slots

<a id="k-639"></a>K-639 · E:3670–3671

## 2026-11-14 — Contextual deferred capture storage

<a id="k-638"></a>K-638 · E:3675–3676

## 2026-11-13 — Loop setup and completion evidence

<a id="k-637"></a>K-637 · E:3680–3681

## 2026-11-13 — Storage operand completion

<a id="k-636"></a>K-636 · E:3685–3686

## 2026-11-13 — Named-call preparation completion

<a id="k-635"></a>K-635 · E:3690–3691

## 2026-11-13 — Shared callable signature preparation

<a id="k-634"></a>K-634 · E:3695–3696

## 2026-11-13 — Checked unknown-type exception contracts

<a id="k-633"></a>K-633 · E:3700–3701

## 2026-11-13 — Current-state repository and authority audit

<a id="h-411"></a>H-411 · E:3707–3708

<a id="h-412"></a>H-412 · E:3710–3711

<a id="k-632"></a>K-632 · E:3713–3714

<a id="source-compiler-validation-history"></a>SOURCE-COMPILER-VALIDATION-HISTORY · E:3716–3717

<a id="design-discussion-history"></a>DESIGN-DISCUSSION-HISTORY · E:3719–3720

<a id="micro-dispatch-experiment-history"></a>MICRO-DISPATCH-EXPERIMENT-HISTORY · E:3722–3725

## Authority evidence index

H records identify human decisions; K records identify agent assessments. Read the original event when its exact scope matters. Locators preserve original statements, provenance, time uncertainty, exclusions and supersession; an index is not additional approval. Corrections and withdrawals receive new IDs.

| Source | Immutable commit + path | Events |
| --- | --- | --- |
| A | `0c0efbfde4dccb3f3397b6f961bd82bd9a12c848:design/DesignAuthorityRegistry.md` | H-01–H-387, K-00–K-577 |
| B | `061a4a684770447ba674e54079f9a157aa75e43e:design/DesignAuthorityRegistry.md` | H-388/H-389, K-578 |
| C | `55667c37cd67131a50bdca6264478ab03084ae4f:design/DesignAuthorityRegistry.md` | H-390–H-410, K-579–K-631 |
| D | `1c91962fe5bc525c833c92f589d654528f69d24b:CHANGELOG.md` | H-413–H-422 and the linked scalar/owner/inference discussion assessments |
| E | `cb7e10f86058054b3709cd437d87ac443ec4cd50:CHANGELOG.md` | Dated entries for Nov 13–22, including their stable H/K IDs and secondary evidence |

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

Pinned unmodified LLVM 23.1.3 for Linux and native macOS. The three-stage bootstrap and host-specific validation, including debugger and relocation limits, are recorded at `cb7e10f86058054b3709cd437d87ac443ec4cd50:CHANGELOG.md`, lines 4814–4849.
