# Idiomatic CoD

CoD lets an operation state which storage it reads or changes, which owner keeps that storage alive,
and whether its behavior depends on explicit inputs or installed dynamic context. Idiomatic code
makes those choices visible at the interface and uses local structure to make their consequences
easy to follow.

This guide is for programmers learning how to make those choices in CoD. Its small shipment, stock
and billing examples are independent unless a section explicitly reuses a definition. Statements
shown outside a named callable run in a full lexical scope; procedural contexts and application
dependencies are identified where needed. For a compact reference to the source forms used here, see the
[syntax showcase](../toolchain/syntax/showcase.cod).

## Design authority

Guide revision 21 applies the [construction terminology](../CHANGELOG.md#h-make-by-ctor-and-init-20261127) to the shipment example. Its result and owner follow the existing guide r20 contract.

Guide revision 20 adds [contextual numeric literals](#write-numeric-literals-in-context) under [D-11-LITERAL-DEFAULTS](../design/DesignAuthorityRegistry.md#d-11-literal-defaults). Guide revision 19 distinguishes [receiver-family lookup](../design/DomainLanguageDesign.md#how-receiver-dispatch-works) from [associated callable names](#make-module-boundaries-express-intent) under [D-DOC-IDIOMATICS](../design/DesignAuthorityRegistry.md#d-doc-idiomatics). Guide revision 18 links the [DBMR database service guide](BuildingADatabaseServiceInCoD.md) from the stock ownership example. Revision 17 links the [scoped effect composition guide](ScopedEffectComposition.md) at the integration-boundary example. Revision 16 links the [companion guide to reusable domain operations](BuildingReusableDomainOperations.md) from the pipeline example. Revision 15 illustrates the [default ASCII vocabulary](../CHANGELOG.md#k-ascii-vocabulary-20261125) and uppercase named constants. Revision 14 applies [enum backing coercion](../design/TypedEnums.md#using-an-enum-as-its-backing-value)
to [ASCII members in a byte scanner](#ascii-members-in-a-byte-scanner). Revision 13 applies the [grouped method decision](../CHANGELOG.md#h-grouped-method-dispatch-20261125)
to [tariff dispatch](#put-type-and-value-specializations-in-one-method-group) and [method arm boundaries](#use-canonical-source-form).
It retains revision 12's [fixed receiver binding](../CHANGELOG.md#h-fixed-self-binding-20261125) in the member examples. Revision 11 applies the [OO call-operator decision](../CHANGELOG.md#h-oo-call-operator-head-20261125) to direct calls and borrowed results. It continues the revision 10 guidance on the [confirmed OO source rules](../design/DomainLanguageDesign.md#product-storage-inheritance)
and [direct-parent construction](../design/ConstructionAndAcquisition.md#initializing-the-storage-parent-of-a-product). It applies the [normal-completion rule](../CHANGELOG.md#h-normal-completion-20261124), the [single-directive import](../CHANGELOG.md#h-proc-import-identity-20261124) and the confirmed [writing and review guideline](IdiomaticsGuideline.md) under
[D-DOC-IDIOMATICS](../design/DesignAuthorityRegistry.md#d-doc-idiomatics), P-694/P-695, and the
[canonical source style](IdiomaticsGuideline.md#canonical-code-style). The
[guide baseline](../CHANGELOG.md#k-idiomatic-cod-rewrite-20261123),
[case spelling](../CHANGELOG.md#h-guarded-short-heads-20261123) and
[arithmetic updates](../CHANGELOG.md#h-augmented-transfer-tokens-20261123) identify its scope. The
[operator-token decision](../CHANGELOG.md#h-maximal-operator-runs-20261123) covers spacing between symbolic operators.
The design sections linked throughout own the language rules, including identity, permissions,
owners, effects, control and cleanup. This guide explains their use without extending those rules.
The [symbolic family selection guideline](../CHANGELOG.md#h-symbolic-family-reuse-idiom-20261123)
covers the distinction between consuming a symbol, installing a family and extending it.
The [function-valued symbol decision](../CHANGELOG.md#h-function-valued-symbol-20261124) adds the direct call idiom under [D-10](../design/SymbolicDynamicScope.md#calling-a-function-valued-symbol).
The [callable terminology](../design/CallableStorageAndHereAccess.md#function-value-and-functor-value) governs function and functor values under the [owner-qualified binding decision](../CHANGELOG.md#h-functor-value-distinction-20261124).
Acceptance applies the guideline's semantic, example, coverage and mandatory style checks.
[Long-term design topics](DesignTopics.md) retain their separate scope.

## Contents

- [Model the states your program can observe](#model-the-states-your-program-can-observe)
- [Make storage and access visible](#make-storage-and-access-visible)
- [Choose a storage parent and a receiver interface](#choose-a-storage-parent-and-a-receiver-interface)
- [Choose a callable by its dependencies](#choose-a-callable-by-its-dependencies)
- [Treat absence as part of the interface](#treat-absence-as-part-of-the-interface)
- [Choose control and cleanup boundaries](#choose-control-and-cleanup-boundaries)
- [Traverse and compose through declared protocols](#traverse-and-compose-through-declared-protocols)
- [Make module boundaries express intent](#make-module-boundaries-express-intent)
- [Give domain values a display vocabulary](#give-domain-values-a-display-vocabulary)
- [Use canonical source form](#use-canonical-source-form)

## Model the states your program can observe

Choose a sum when alternatives carry different information. A shipment waiting for dispatch has no
dispatched carton count; a dispatched shipment does. Keeping the count inside its alternative makes
that relationship part of the type.

```cod
export data Shipment =
    Queued {}
  | Dispatched { cartons:u64; }

"""Returns the dispatched carton count for [Shipment](Shipment)."""
export func cartons_sent(shipment:const Shipment in O) -> u64 where {const owner O} {
    case shipment of {
        Dispatched { val count <- cartons; } -> count;
        Queued -> 0u64
    }
}

func sample_count() -> u64 {
    ref sent:Shipment = make Dispatched { cartons <- 3u64 }
    cartons_sent(sent)
}
```

The result of `sample_count()` is `3u64`. This is **make by init**. The braces establish the payload directly, which is allowed because `Dispatched` has no ctor overload. With no contextual owner, this `make`
uses the current full lexical scope. The query borrows that live target with readonly access and
returns independent bits. The object can therefore finish its lifetime when `sample_count` exits
without invalidating the returned count.

In the case, the constructor test precedes field reception. Only a `Dispatched` target supplies
`cartons`; `count` is local to that arm. The `Queued` arm needs only a tag test, so its short form
has no field receiver. Both alternatives complete with `u64`, and the parameter requires a present
target. These arms cover the whole subject domain without a catch-all.

Use this form when the payload belongs to the state. Separate flags and a universally present count
would instead require callers to maintain their relationship. If every alternative has the same
independent information, a product with an appropriate discriminant can express that different
intent. Do not introduce a sum merely to rename a Boolean.

Construction names the selected alternative, here `Dispatched`. If its author supplies a ctor taking a carton count, the call becomes `make Dispatched(3u64)` (**make by ctor**). The ctor then validates and initializes the shipment before publication. [Construction terminology](../design/ConstructionAndAcquisition.md#make-by-ctor-and-make-by-init)

A guard runs after its pattern's receiving operations. Receive into arm-local variables when the
guard may decline; move writes that should occur only on acceptance into the body. A false guard
does not roll back reception or its own effects.

[Sum construction](../design/DomainLanguageDesign.md#named-sum-constructors) ·
[Lexical construction](../design/ConstructionAndAcquisition.md#owner-directed-construction) ·
[Case reception and completion](../design/DomainLanguageDesign.md#case-expressions-and-branch-local-variables)

### Use an enum for a closed set of payload-free choices

A service priority is a choice with no per-alternative payload:

```cod
export enum Priority:u8 {
    Standard;
    Urgent;
}

func handling_order(priority:Priority) -> u8 {
    case priority of {
        Standard -> 1u8;
        Urgent -> 0u8
    }
}
```

`handling_order(Urgent:Priority)` returns `0u8`, assuming no lexical binding shadows the expression
name `Urgent`. The case heads resolve members through the subject's enum, so a same-spelled lexical
variable does not change their patterns.

A `Priority` value can supply its code to a `u8` result or parameter. `handling_order` gives
`Urgent` the first handling position independently of its declared code. Use a sum when a
priority needs distinct payloads, such as a deadline belonging only to one alternative.

Member aliases retain one value identity, and coverage counts distinct members of the complete
enum. Selecting only some member names does not reduce its domain. An exhaustive case makes each
current choice explicit. Use a wildcard when treating all remaining choices alike is the intended
policy, accepting that it also covers members added later.

[Enum identity](../design/TypedEnums.md#declaration-and-value-identity) ·
[Member names and case coverage](../design/TypedEnums.md#names-and-case-matching)

### ASCII members in a byte scanner

Use `UPPER_SNAKE_CASE` for published named compile-time constants. Control-code abbreviations
such as `LF` and `CR` already fit that convention. Give both letter cases distinct readable
names, for example `UPPER_A` and `LOWER_A`. Name style has no effect on lookup or dispatch.

The full ASCII vocabulary assigns a member to each code from 0 through 127. This excerpt
from a package-root `ASCII.cod` includes the scanner's proposed defaults and two less
frequently used members. The module chooses the defaults. Every member belongs to `ASCII`.

```cod
export enum ASCII:UInt8 {
    HT = 9;
    LF = 10;
    CR = 13;
    ESC = 27;
    SPACE = 32;
    HASH = 35;
    DEL = 127;
}
default { ASCII { HT, LF, CR, SPACE, HASH } }
```

The [booter's trivia scanner](../applications/cod-booter/src/Lexer.cod) reads raw bytes.
In a consumer, `using "ASCII.cod";` selects the type and the five named defaults. Where
the scanner already has `byte:UInt8` and the current `comment:bool`, its update reads:

```cod
comment <- byte != LF && (comment || byte == HASH)
```

After `#`, CR or a byte of value 255 keeps the comment open. LF closes it. In a different
rule, `byte == (ESC:ASCII)` can recognize escape. With no lexical `ESC` binding, the type
supplies the member table. A local binding with that spelling takes precedence in this
expression. Code that needs an enum value for an integer uses `10:ASCII`.

The same vocabulary works in a [method group](../design/SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch).
A `UInt8` slot can match `(=LF)` through its selected constant. An `ASCII` slot can match
`(ESC)` through the enum's member table even when `ESC` is outside the defaults.
See [default module selections](../design/ModuleWorlds.md#default-and-complete-public-selections).

## Make storage and access visible

An interface should show whether an operation needs a number, a continuing view of a target, or
responsibility for an entire resource. Keep those choices distinct as the program grows.

### Choose independent state, a borrowed target, or an existing place

When a stock count is saved for later comparison, the saved number should survive later updates to
the count. When an operation must update the count itself, it needs a reference to that storage.
Choose between those intentions before choosing a declaration.

The following statements run in one full lexical scope. Both counts belong to that scope.

```cod
val available <- 10u64
val reserved <- 3u64
val before <- available
ref current = available

current <- 8u64
ref retained = current
ref var selected -> current
selected = reserved
selected <- 4u64
```

`before` remains 10. `available` is 8 and `reserved` is 4. `retained` still selects `available`;
`current` now selects `reserved`. These observations follow from three different operations:

- `val before <- available` transfers bits into independent storage.
- `ref retained = current` creates an independent reference binding to the selected target.
- `ref var selected -> current` declares another name for `current`'s existing binding slot.

The last declaration is **declared place aliasing**. Rebinding through `selected` changes `current`
because they name the same slot. It leaves `retained`'s independent binding alone. The two references
initially selecting `available` illustrate **runtime target aliasing**. That sharing requires no
pairwise declaration and does not merge their slots.

Use a place alias when several operations should address the same selected field, array element or
binding slot. Its selection happens once. Use a new reference when the new name should retain its
own target selection. Neither choice duplicates an object graph, transfers ownership or prolongs
the selected storage's lifetime. A reference rebind must also preserve its fixed actual owner; the
two local counts above satisfy that condition because they have the same lexical owner.

See [operation terminology](../design/CallableViewsAndCaptures.md#copying-moving-and-graph-transfer)
and [declared place aliases](../design/CallableViewsAndCaptures.md#declared-place-aliases).

### Ask an operation for the access it needs

A stock reservation changes existing storage. Express that capability directly in its parameter.
It needs neither authority to allocate in the stock's owner nor access to dynamic context.

```cod
func take_one(stock:u64 in O) -> bool where {const owner O} {
    if (stock == 0u64) return false
    stock <-:- 1u64
    return true
}
```

The ordinary `stock:u64 in O` formal is a non-null borrowed reference. A successful reservation
decrements its target once; an empty stock leaves it unchanged and returns false. The `const owner`
qualifier withholds allocation authority through `O`. It leaves the writable target access intact.
This is an ordinary `func`: its inputs identify the storage it reads and changes. Mutation alone
does not call for a procedure.

A comparison only needs readonly views. Independent counts should normally have independent owner
parameters:

```cod
func same_count(left:const u64 in L, right:const u64 in R) -> bool
    where {const owner L, const owner R} {
    left == right
}
```

`L` and `R` may denote the same owner or different owners. Distinct names allow either situation;
they do not prove disjointness. Repeating one owner parameter would require the arguments to have
the same actual owner. Use that relationship when the operation needs it, such as installing a
reference into a same-owner field. A comparison gains no such benefit from coupling its inputs.

By contrast, an owner-free bits parameter such as `amount:u64` supplies independent callee-local
bits. Adding `in O` changes the contract to external storage. Choose it for a borrow whose identity
or updates matter, rather than adding owner annotations to ordinary numeric inputs indiscriminately.

See [owner-qualified formals](../design/RegionalOwnership.md#owner-qualified-reference-arguments),
[owner constraints](../design/RegionalOwnership.md#confirmed-owner-syntax) and
[allocation authority](../design/RegionalOwnership.md#owner-constness-and-allocation).

### Distinguish a fixed binding from readonly access

A fixed selection and a readonly target solve different problems. A cashier may need a reference
that stays attached to one stock count while still updating that count. An observer may need to
switch between counts while retaining readonly access.

```cod
val shelf <- 8u64
val reserve <- 3u64
const ref fixed = shelf
ref observed:const u64 = shelf

fixed <- 7u64
observed = reserve
reserve <- 4u64
```

`fixed` still selects `shelf`, whose value is now 7. `observed` selects `reserve` and reads 4.
Declaration-leading `const` prevents rebinding `fixed`; it permits the shown target write.
Type-leading `const` prevents writes through `observed`; it permits the shown rebind. Combining both
qualifiers fixes the binding and restricts its target access.

`fixed = reserve` is rejected because the binding is fixed. `observed <- 5u64` is rejected because
that view is readonly. The legitimate write through `reserve` shows why readonly access is not a
promise that the underlying value never changes. Readonly access also propagates through reference
fields and derived views; another spelling cannot recover write permission from that path.

See [independent qualifiers](../design/RegionalOwnership.md#independent-binding-and-target-qualifiers).

### Choose the result contract before choosing the receiver

A scalar result without an owner, such as `-> u64`, supplies independent bits. A result such as
`-> ref const u64 in O` supplies a readonly view of existing storage in O. Choose the result mode
according to what the caller is meant to retain.

```cod
func read_count(count:const u64 in O) -> u64 where {const owner O} {
    count
}

func count_view(count:const u64 in O) -> ref const u64 in O
    where {const owner O} {
    count
}
```

These two operations select the same input target, but make different promises. In one scope:

```cod
val count <- 3u64
val saved <- read_count(count)
ref live = count_view(count)
count <- 9u64
```

`saved` remains 3; reading `live` observes 9. The readonly view limits writes through that view.
It does not prevent the owner from using another writable path to the same target.

The receiver cannot change the operation's contract. `ref local = read_count(count)` materializes
the returned bits in new local storage; it does not turn `read_count` into a borrow. Conversely,
`val snapshot <- count_view(count)` reads the returned target into independent bits. State the
borrow in the result signature when target identity matters, and use a value result when the caller
needs a measurement or decision independent of later mutation.

For a fresh scalar ref declaration, an independent initializer can materialize local storage.
Later reference assignment only selects an existing compatible target. Thus updating the existing
target and selecting a different target remain separate operations. A returned reference also needs
an owner that survives the call; a function cannot export its local scalar's lifetime by returning
a view of it.

These distinctions become especially useful in interfaces: a query that returns a count can expose
`u64` without making every caller track the queried storage, while an editing or observing interface
can deliberately return a borrowed view. Do not choose a borrow merely to avoid the small bits result.
Choose it when the continuing connection to that target is part of the operation.

[Scalar result modes](../design/ScalarCallResults.md#result-mode-belongs-to-the-callable) ·
[Receiving new storage](../design/ScalarCallResults.md#call-site-matrix-new-declarations) ·
[Updating existing storage](../design/ScalarCallResults.md#call-site-matrix-existing-variables)

### Keep borrowing separate from responsibility for a region

A query needs access to stock. An ownership handoff gives another operation responsibility for the
whole region. Declare the latter only when that transfer belongs to the operation's purpose.

```cod
region Stock {
    remaining:u64
}

func stock_count(region stock:const Stock) -> u64 {
    stock.remaining
}

func accept_stock(own region stock:Stock) -> own region :Stock {
    return stock
}

own region incoming:Stock <- make region Stock(4096u64 -> capacity) {
    remaining <- 12u64
}
val before <- stock_count(incoming)
own region inventory:Stock <- accept_stock(incoming)
val after <- stock_count(inventory)
```

After successful construction, both observations are 12. `stock_count` borrows the region and
acquires no release responsibility. `accept_stock` consumes its input at successful call entry and
returns that same region through an owning result. The caller receives responsibility in
`inventory`; no stock graph has been copied. `incoming` is consumed: reading or transferring it
again before reinitializing it is rejected. It does not silently become a nullable region.

The still-held `inventory` is released at its owning scope's exit. Construction must initialize
every required root before delivering the new owner; a failure releases the unfinished region.
Argument preparation precedes consumption at call entry. A failure while preparing arguments does
not, by itself, transfer the owner to a callee that was never entered.

An interior `ref` or an ordinary whole-region borrow grants neither release responsibility nor a
longer lifetime. Return a borrowed view only under an owner that remains valid after the call. When
the result must carry the resource's lifetime responsibility, use the owning result contract.

The [DBMR database service guide](BuildingADatabaseServiceInCoD.md) extends this choice to a shared catalog, mutable stock and a receipt that belongs to the caller after the request ends.

See [region construction](../design/ConstructionAndAcquisition.md#region-construction-and-named-root-initializers),
[region declarations](../design/RegionalOwnership.md#region-as-a-declaration-keyword-throughout-the-language),
[consumed names](../design/RegionalOwnership.md#named-owner-consumption) and
[call entry and owning results](../design/ExceptionControl.md#e4--consuming-calls-and-results).

## Choose a storage parent and a receiver interface

### Share storage when an ancestor must see the same object

Use `extends` when code borrowing a parent must operate on the parent part of the very same
object. A product has at most one direct storage parent, fixed by its definition, and the chain
must be acyclic. Use `<:` to express nominal conformance to an abstract type. If a shipment
merely *has* a rate, give it a rate field. If every pickup rate *is usable as* a parcel rate and
must carry its established storage, give it that storage parent.

Consider three rates in a single module. A parcel adds a handling charge to the base fee.
Customer pickup carries the parcel data, but its quote applies a pickup fee directly to the
base fee.

```cod
data ShippingRate { base_fee:u64 }
data ParcelRate extends ShippingRate { handling:u64 }
data PickupRate extends ParcelRate { pickup_fee:u64 }

func ShippingRate(base:u64) { base_fee <- base }

func ParcelRate(base:u64, extra:u64) {
    super(base)
    handling <- extra
}

func PickupRate(base:u64, extra:u64, pickup:u64) {
    super(base, extra)
    pickup_fee <- pickup
}
```

A constructor has field initialization places. `super(base, extra)` initializes the embedded
**direct** parent in place. That parent initializes its own parent. On every normally completing
construction path, exactly one direct-parent call must succeed, including its cleanup. An
independent child field may be initialized first. An inherited field may be read or written only
after the parent is ready. Catching a parent failure does not establish readiness. Even after
success, the constructor works with field places and has no whole-object `self`. Write `super()`
only when the direct parent actually supplies an applicable niladic constructor.

[Product storage inheritance](../design/DomainLanguageDesign.md#product-storage-inheritance) ·
[Parent initialization and failure](../design/ConstructionAndAcquisition.md#initializing-the-storage-parent-of-a-product)

### Choose the ancestor implementation deliberately

Choose the call form that expresses the dependency. `as_shipping.quote()` asks the actual receiver for its member. A free call `quote(as_shipping)` looks up the callable name in the current scope. When the dotted form reaches that free callable, the production compiler warns by default. Write the free call explicitly if that dependency is intended, or lower the finding to info at compile time. For an indexed collection, a library can supply a default through a declared abstraction and a caller can select an adapter by choosing its module view. [Which family does each operation use?](../design/DomainLanguageDesign.md#how-receiver-dispatch-works)

```cod
func const ShippingRate.quote() -> u64 { self.base_fee }
func const ParcelRate.quote() -> u64 { super.quote() + self.handling }
func const PickupRate.quote() -> u64 { super(ShippingRate).quote() + self.pickup_fee }

ref pickup = make PickupRate(10u64, 3u64, 1u64)
ref as_shipping:ShippingRate = pickup
val price <- as_shipping.quote()
```

`price` is **11**. The reference selects the original pickup object under the same owner.
The ordinary call chooses the most specific applicable method for its actual type in the selected
method family. In `PickupRate.quote`, `super(ShippingRate).quote()` starts lookup at
`ShippingRate`, so the result is 10 + 1. If the business rule included parcel handling too,
`super.quote()` at that same site would start at `ParcelRate` and give **14**. Inside
`ParcelRate.quote`, its own `super` still starts at `ShippingRate`.

A super-call selects the implementation for **that call**. The selected body receives the
original `self`. A subsequent ordinary call on `self` again dispatches on the actual type.
Write a super-call inside a member method, computed accessor or call-operator body that has
an implicit receiver. A helper taking a parent reference can make an ordinary call.
An assertion such as `(pickup:ShippingRate).quote()` also makes an ordinary call and gives 11.
The named ancestor must be a strict storage ancestor and its method must accept the receiver's
access. A parent borrow permits no slicing, replacement of a child-typed slot with a plain parent,
or writable array covariance.

[Super-calls in OO bodies](../design/DomainLanguageDesign.md#super-calls-in-oo-bodies) ·
[Concrete dispatch](../design/DomainLanguageDesign.md#concrete-data-dispatch-through-abstract-parameters)

### Put type and value specializations in one method group

A carrier publishes a surcharge for any `ShippingRate` and `Priority`, using the rate types above
and the earlier priority enum. Parcels incur their handling charge and urgent shipments incur a
flat charge of 10. Express the two rules and their intersection in one `func` definition.

```cod
func surcharge(rate:const ShippingRate in S, priority:Priority) -> u64 where {const owner S} of {
    (parcel:const ParcelRate in S, Urgent) => parcel.handling + 10u64;
    (parcel:const ParcelRate in S, _) => parcel.handling;
    (_, Urgent) => 10u64;
    (_, _) => 0u64
}
```

For a parcel with handling charge 3, the standard surcharge is 3 and the urgent surcharge is 13.
An ordinary shipping rate incurs 0 and 10 respectively. A caller can pass the parcel through a
`ShippingRate` reference and still get 13. `parcel` names the same readonly object in its actual
owner `S`. The public labels remain `rate` and `priority`. `Urgent` matches a member of `Priority`.

Each `=>` supplies a method. Reordering these arms preserves the result. Omitting the first
leaves two equally applicable, incomparable specializations for an urgent parcel. An integrating
module with extension permission can supply that intersection when it brings the rules together.
Use `case … of` inside a method when a guard or the order of tests carries business meaning.
[Grouped methods and value dispatch](../design/SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch)

### Give distinct stored facts distinct names

When designing both types, names such as `carrier_code` and `label_code` make it clear which
one a caller is reading. When an established interface calls both fields `code`, a child may
shadow the inherited stored field. Each declaration still needs its own initialization.

```cod
data Label { code:u64 }
data CarrierLabel extends Label { code:u64 }

func Label(value:u64) { code <- value }
func CarrierLabel(label_code:u64, carrier_code:u64) {
    code <- carrier_code
    super(label_code)
}

ref label = make CarrierLabel(7u64, 9u64)
ref original:Label = label
val carrier <- label.code
val printed <- original.code
original.code <- 11u64
```

`carrier` is 9 and `printed` is 7. After the write, `original.code` is 11 and `label.code`
is still 9. A stored-field read starts at the product **view** of its receiver. Even a parent
method called on a child selects its own stored fields through the parent view of `self`.
When both views select the same inherited field declaration, writes are visible through both.

The production compiler reports a legal shadow at the child declaration by default, with a
warning pointing to the nearest ancestor field it hides. Projects may select info or error
severity. Merely inheriting the child field adds no further warning. A repeated field in the
same product, or a collision among a stored field, computed accessor and named method, is a
declaration error when those members become visible together.

[Stored-field selection and shadowing](../design/DomainLanguageDesign.md#product-storage-inheritance) ·
[Production shadow diagnostic](../design/DomainLanguageDesign.md#production-diagnostic-for-a-shadowed-stored-field)

### Choose a field read, a named action or a call operator

A stock card exposes the number of units available and lets a caller reserve some of them.
The card starts with at most as many reserved units as it has on hand.

```cod
data StockCard { on_hand:u64; reserved:u64 }

func const StockCard.@available() -> u64 { self.on_hand - self.reserved }

func StockCard.reserve(units:u64) -> bool {
    if (units > self.available) return false
    self.reserved <-:+ units
    true
}

func (const StockCard in O).held_units() -> ref const u64 in O where {const owner O} {
    return self.reserved
}

ref card = make StockCard { on_hand <- 9u64; reserved <- 2u64 }
val before <- card.available
ref held:const u64 = card.held_units()
val booked <- card.reserve(3u64)
val after <- card.available
```

`before` is 7, `booked` is true, `after` is 4 and the borrowed `held` now reads 5. The
accessor marker `@` occurs in its declaration. The caller writes `card.available`.
It returns independent bits. The named action `reserve` signals a state change and needs a
writable receiver. `held_units` names the receiver owner because its result borrows a field.
The result is readonly and its lifetime remains bounded by that actual owner.

Put `const` before the receiver type for readonly access. Omitting it requires writable access,
including for an accessor. The implicit `self` slot is fixed in both cases. In a writable
method, `self.reserved <-:+ units` can update the field, while `self = other` is an error.
A writable accessor can change its receiver on every read. Give
that behavior an action name when callers should count invocations. A writable receiver alone
grants no property setter, and a readonly receiver alone promises no purity. The target is
selected once before the written arguments. Later rebinding of the source reference slot does
not redirect a call already being prepared.

A weight limit reads naturally as a predicate. Give the product a call operator when clients
should apply the configured instance directly or pass it to a synchronous `here` parameter.

```cod
data WeightLimit { maximum:u64 }
func const WeightLimit.(())(weight:u64) -> bool { weight <= self.maximum }
func fits(here allowed(n:u64) -> bool, weight:u64) -> bool { allowed(weight) }

ref limit = make WeightLimit { maximum <- 12u64 }
val at_limit <- limit(12u64)
val overweight <- fits(limit, 13u64)
```

`at_limit` is true and `overweight` is false. `make` constructs the instance. Both later calls
use its initialized receiver. Write the short head when the interface only needs the receiver’s
access. To expose a borrow from the same object, name its owner in the head and in the result:

```cod
func (const WeightLimit in O).(())() -> ref const u64 in O where {const owner O} {
    self.maximum
}
ref maximum:const u64 = limit()
```

The zero-argument overload lends the field while its actual owner is live. The same head lets
another parameter require that owner. The leading `const` governs receiver access, the result lends
readonly access, and `const owner O` withholds allocation authority. To keep a selected `func`
for later use, give the functor value its actual owner and keep the receiver live through every call.

[Accessors, member names and receiver owners](../design/DomainLanguageDesign.md#receivers-in-field-accessors-and-member-methods) ·
[Functor receivers and synchronous use](../design/DomainLanguageDesign.md#functor-receivers-and-callback-arguments) ·
[Callable values and owner dependencies](../design/CallableStorageAndHereAccess.md#function-value-and-functor-value)

## Choose a callable by its dependencies

### Pass state explicitly, or borrow the caller's live work

Use **function value** for a capture-free selection (`func …`). Use **functor value** for a
call operator bound to a receiver in its owner (`func … in O`). Choose the latter when a
pricing rule needs that particular receiver on every call. A function-valued holder or
symbol accepts function values. A functor-valued one accepts functor values.
[Function value and functor value](../design/CallableStorageAndHereAccess.md#function-value-and-functor-value)
The [weight-limit example](#choose-a-field-read-a-named-action-or-a-call-operator) supplies a
receiver when the selected rule carries its own configuration. A capture-free selection takes
its runtime state as explicit inputs. For example, billing can select this calculation and
supply the fee with each call:

```cod
func add_fee(total:u64, fee:u64) -> u64 {
    total + fee
}

func quote = add_fee
val quoted <- quote(100u64, 2u64)
```

`quoted` is 102. A storable function has no enclosing runtime capture. Referring to a factory's local
fee from such a function is rejected even when that local is readonly. Store the fee as data and
pass it explicitly when the calculation must remain available later.

Use `here` for work that runs while its supplier's activation and referenced owners are live. It can
operate on the supplier's actual slots:

```cod
func twice(here action() -> void) -> void {
    action()
    action()
}

func count_quotes() -> u64 {
    val count <- 0u64
    twice() {{
        count <-:+ 1u64
        return
    }}
    return count
}
```

`count_quotes()` returns 2. Each invocation updates the same `count`; it does not update a saved
copy. A receiver may forward the work synchronously to another compatible `here` parameter. It
cannot store or return that supplied work for execution after the supplier ends.

A function's own statements cannot call named or symbolic procedures. Use a procedure when the
operation itself needs that authority.

A `here` body written in a procedure keeps its lexical permissions when a function invokes it.
That permits a general function library to drive caller-supplied work without giving the library's
own statements permission to access dynamic bindings. The choice of braces also controls return
and exception destinations; choose those boundaries deliberately.

Procedures remain directly callable definitions or symbol-selected handlers, not ordinary callable
values. Saving a named procedure in a local, ordinary parameter or returned callable is rejected.
For delayed business work, keep the operation's data and dispatch to a named procedure in the
delivery context. A synchronous callback uses `here` while that context is still live.

See [callable kinds](../design/CallableStorageAndHereAccess.md#callable-value-kinds),
[function inputs](../design/CallableViewsAndCaptures.md#callable-owners-and-captures) and
[direct lexical access](../design/CallableViewsAndCaptures.md#here-callables-and-direct-lexical-access).

### Choose which dynamic context an operation should observe

A procedure is appropriate when an operation depends on installed dynamic context. A direct named
procedure call uses current bindings. Symbolic calls also select the operation through a symbol:
`behave` uses the current dynamic values, while `perform` temporarily installs the selected
handler's saved dynamic values.

This module-scope example distinguishes those values from a live lexical fee:

```cod
symbol @rate:u64
symbol @quote:proc() -> u64

val fee <- 1u64
effect @rate <- 10u64
effect @quote() -> u64 { return @rate + fee }

fee <- 2u64
effect @rate <- 20u64
val saved <- perform @quote()
val current <- behave @quote()
val scoped <- {{
    effect @rate <- 30u64
    behave @quote()
}}
val restored <- @rate
```

`saved` is 12: the installation saved rate 10, and the live lexical fee is now 2. `current` is 22.
`scoped` is 32, and `restored` is 20 after the inner full scope ends. `effect` installs a scoped
binding. `perform` and `behave` invoke an installed handler; neither is another installation syntax.

Choose `behave` when work should adapt to the active caller context. Choose `perform` when it should
use the symbolic values retained when that handler was installed. Both preserve the handler's
lexical accesses and restore bindings on scope exit, including unwind. Restoration does not undo
writes through valid references, copy a regional graph or extend its owner's lifetime. A quoted
rate retained as a symbolic value is consequently different from freezing an entire invoice.

See [call forms](../design/SymbolicDynamicScope.md#functions-and-procedures),
[capture and restoration](../design/SymbolicDynamicScope.md#captured-values-and-call-behavior) and
[symbolic snapshots](../design/CallableViewsAndCaptures.md#symbolic-snapshots-at-definition-activation).

For a function-valued symbol declared as `func symbol @score()->u64`, prefer
`return @score()` inside a procedure. This reads the installed function and calls it.
With a procedure symbol, the same spelling calls the lexically selected family.
Use `perform` or `behave` when the caller wants the installed procedure handler.
[Call by symbol kind](../design/SymbolicDynamicScope.md#calling-a-function-valued-symbol)

### Keep symbolic procedure families at integration boundaries

Prefer an ordinary named function or procedure when the program wants one implementation. A caller
that should obey the active policy can select the vocabulary identity with `symbol from` and call
`behave` or `perform`. Ordinary callers rarely need `using proc` or `import proc`. A direct call to
a lexically selected family would bypass an active replacement for that operation.

The companion guide puts this decision into practice by routing two detectors' requests and tracing a missing or misplaced handler. [Facing Complexity with Scoped Effect Composition](ScopedEffectComposition.md)

One reason to select the family is to install a complete policy for downstream code. Suppose a
rating engine calls `behave @quote(shipment)`. The carrier package supplies methods for several
shipment types under that same published symbol. The application entry can run a batch with all
of the carrier's methods:

```cod
using "./carrier-tariff.cod" proc @quote
using "./billing.cod" rate_pending_shipments

proc run_carrier_batch() -> void {
    effect proc @quote;
    rate_pending_shipments()
    return
}
```

Use `import proc` only when a module needs to **contribute a method to that same operation**. For
example, an integration package may add a rate for its own refrigerated shipment type and publish
the extended family for the entry to install. Write `import "./carrier-tariff.cod" proc @quote;`
to select the carrier’s symbol and gain extension permission. If the package already selected
that vocabulary identity with `symbol from`, the import reuses it. A different identity conflicts. Downstream billing code can keep using the
vocabulary identity. When one call simply needs a different calculation, give that calculation an
ordinary name. [Module selection](../design/ModuleWorlds.md#default-and-complete-public-selections)

To replace a policy just for one batch, install the complete set of local methods together. Suppose
`@credit` has the public signature `proc(Priority) -> u64`. Inside the following scope each urgent
shipment receives one credit. On scope exit the previous policy is restored.

```cod
{{
    effect @credit -> u64 {
        (Urgent) => 1u64;
        (_) => 0u64
    }
    rate_pending_shipments()
}}
```

Here the batch requests the installed policy with `behave @credit(priority)`. The symbol signature
supplies the enum domain and the wildcard covers every remaining member. Installing this group
changes one dynamic binding. Publishing a reusable extension uses the module's procedure family.
[Grouped handler installation](../design/SymbolicDynamicScope.md#installing-a-local-family-with-several-methods)

## Treat absence as part of the interface

When a missing stock entry means the item is unavailable, return that ordinary business outcome.
Reusing `take_one` above, a nullable wrapper can handle absence before requesting a required borrow:

```cod
func take_if_present(ptr stock:u64 in O) -> bool where {const owner O} {
    if (stock is null) return false
    take_one(stock)
}
```

The remaining path proves presence for the argument selection. A missing entry and a present entry
with zero stock both return false under this interface's stated policy. If callers need to tell
those outcomes apart, give them distinct domain results rather than relying on a null exception.

When absence is a failure that the caller must handle, make that checked selection explicit:

```cod
using "std:exceptions" NullPointerException

func take_required(ptr stock:u64 in O) -> bool
    throws NullPointerException where {const owner O} {
    take_one(not null stock)
}
```

`not null` selects the target once at that argument's evaluation position. Absence throws before
entering `take_one`; presence supplies the same target and access. Declaring `throws` alone would
not make an unproved plain pointer argument valid. Accept a `ptr` directly when handling absence
belongs to the callee; use a ref formal when every call must supply a target.

Presence is only one obligation. The target must also be initialized, suitably accessible and owned
by a live owner. Rebinding a tested slot, or invoking work that may change it, can invalidate the
presence fact before a later selection. A null check never repairs an expired owner.

See [reference argument selection](../design/RegionalOwnership.md#owner-qualified-reference-arguments)
and [separate storage obligations](../design/RegionalOwnership.md#separate-initialization-absence-and-access).

## Choose control and cleanup boundaries

### Preserve the intended control destination

Choose a body form by where its work should finish. A single-brace supplied `here` body keeps the
supplier's lexical control destinations. Double braces give the body its own callable return.
Adding a scope therefore changes what an early return means.

A function can use its final expression as its result, with explicit returns for early completion.
Let a void procedure finish after its last operation. It executes the operation and discards its
result. Use a value return to deliver a non-void procedure result. A full-scope here invocation
has its own final-expression result. Keep that result separate from the supplier's
result and from feedback requested by a receiving operation.

```cod
func plus_one(here body() -> u64) -> u64 {
    body() + 1u64
}

proc demo() -> u64 {
    val n <- plus_one() { continue 7u64 }
    return n + 100u64
}
```

With this receiver and supplier, the following alternative bodies have different results:

| Supplied body | Destination | Result of `demo()` |
| --- | --- | --- |
| `{ 7u64 }` or `{ continue 7u64 }` | One invocation of `body`; both additions run. | 108 |
| `{{ return 7u64 }}` | The body's own invocation; both additions run. | 108 |
| `{ break 7u64 }` | The original `plus_one` call; only the supplier's addition runs. | 107 |
| `{ return 7u64 }` | The supplying `demo` invocation; neither addition runs. | 7 |

Use `continue` to supply feedback, `break` to finish the receiving call, and `return` to leave the
supplier. A forwarding helper preserves these destinations. The value of a `continue` must satisfy
the body signature; a `break` value must satisfy the original call's result contract. They need not
have the same type. A generator's final result is void, so its traversal accepts bare `break`.

For example, with a writable credit cell installed and an application operation `appended` for the
samples sink, both procedures infer void:

```cod
proc charge() { @credit_cell <- @credit_cell - 2u64 }
proc samples(n:u64) { @samples_sink <- appended(@samples_sink, n) }
```

The writes take effect and the readonly views produced by the transfers are discarded. An
`if returning ()` in either procedure’s defer matches this natural exit just as it matches an
explicit empty return. An escaping ordinary exception takes the other branch.

A successful early exit still owes the cleanup and initialization guarantees of its destination.
For example, a `break` cannot report successful completion if it skips an initialization that the
receiver promised to perform. Cleanup must finish before an outgoing result is delivered.

[Here control destinations](../design/CallableStorageAndHereAccess.md#break-and-continue-in-a-single-brace-here-body)
and [body results](../design/CallableResultInference.md#results-of-proc-func-and-here-bodies).
The example returns independent scalar bits. Replacing them with references also requires proving
that the selected owners remain live through cleanup and delivery.

### Give temporary work an actual scope

Ordinary braces group expressions. Use `{{ ... }}` when work needs its own local declarations,
deferred actions or local return. Its return ends that immediate invocation, even inside a loop.
To deliver its result as feedback, put the `continue` outside it.

Here `source()` is an externally supplied generator with contract `yields u64 -> u64`; it has no
other inputs or checked failures, and the offered values can be doubled within `u64`. Its application
contract determines how it uses the feedback.

```cod
for val item <- source() {
    continue {{
        val doubled <- item * 2u64
        return doubled
    }}
}
```

The local return supplies the number; the outer continue sends it to the active yield. Writing
`{{ continue item }}` cannot continue the outer loop: that full scope has introduced a callable
boundary. Similarly, a return inside an iteration-local full scope does not finish the surrounding
search.

A direct defer in an ordinary loop body registers with the nearest enclosing full scope. It can
accumulate work across iterations. Put the registration inside an explicit full scope when cleanup
must finish for that iteration. Effect restoration at a handoff remains part of the loop contract;
it does not give direct defers an implicit per-iteration queue.

[Full scopes inside a loop body](../design/RangedForAndGenerators.md#full-scopes-inside-a-loop-body)
and [loop scoping](../design/ExceptionControl.md#loop-scoping-rules). The example sends bits out of
the inner scope. A reference to its local storage could not survive that exit.

### Register cleanup at the time you mean

Use a deferred call to save a particular invocation's inputs. Use a deferred block when its work
must read the state present at cleanup. Assume `record(value:u64)` is a procedure that records its
bits argument and completes normally.

```cod
proc record_timing() -> void {
    val n <- 1u64
    defer record(n)
    defer { record(n) }
    n <- 2u64
    return
}
```

This records 2, then 1. Registrations run in reverse order. The block reads `n` during cleanup; the
plain call prepared its bits argument when registered. Its callable selection and argument
preparation also happen then. A failure during preparation registers no partial call.

Preparing a reference saves its selected target under its access and owner contract, rather than
copying that target's contents. Keep that owner alive until the deferred invocation. Preparing a
direct procedure call does not freeze unrelated dynamic bindings: its execution still follows
ordinary current-binding rules.

A deferred block does not provide an implicit scope for new variables or effect installation.
Introduce an explicit nested full scope if that cleanup needs either. The nested scope owns and
drains its own deferred work; an action cannot append another defer directly to the scope already
being drained.

[Deferred calls and blocks](../design/ExceptionControl.md#deferred-work-and-explicit-scopes)
and [nested defer](../design/ExceptionControl.md#nested-defer-declarations). The example's outer
scalar remains live throughout its cleanup. A borrowed loop-control slot cannot be kept alive
merely by registering a deferred call.

### Handle failure where cleanup occurs

Keep ordinary failure propagation and deferred failure handling explicit. A checked operation in
ordinary execution needs a covering catch or an appropriate declared `throws` contract. Checked
failures from deferred work must instead be covered before their owning defer pass finishes.

In this fragment, the buffer API supplies an already acquired buffer whose owner survives the
procedure. `flush_buffer()` declares `throws FlushException`, its library's checked failure type.
`report_flush_failure` borrows that exception synchronously and handles its own failures.
`write_buffer()` completes normally.

```cod
proc update_buffer() -> void {
    defer catch (failure:const FlushException) {
        report_flush_failure(failure)
    }
    defer { flush_buffer() }
    write_buffer()
    return
}
```

The handler is registered first, so it is visited after the flush. It can handle a checked flush
failure before the pass completes. Registering it after the flush would visit and retire it too
early. Adding `throws FlushException` to `update_buffer` would not repair missing deferred
coverage. An attached catch on the flushing action is another way to handle that action's failure.

A deferred catch handles deferred failures offered at its position. It does not handle an ordinary
body failure just because cleanup follows that failure. Ordinary deferred actions still run when
another action has failed. Remaining unchecked deferred failures produce `UnhandledDeferError`
after the pass; an outstanding body failure is retained separately from those deferred members.

A catch binding owns its exception-handling scope even when its target annotation is `const`.
Borrow it for synchronous inspection; preserve or transfer the actual owning responsibility when
propagating it. Inside a catch, `throw;` forwards the incoming tip unchanged; `rethrow;` adds a
trace-bearing standard wrapper. Public failure contracts should name the application failure that
callers can handle. Transparent rethrow does not require exposing wrapper names as API outcomes.

A storage failure can declare its checked exception kind once. A short write adds a count to
that same kind of failure.

```cod
data StorageException <: Exception { code:u64 }
data ShortWriteException extends StorageException { written:u64 }
```

When a `ShortWriteException` arrives directly, a catch for `StorageException` sees its inherited
`code`. The same object retains `written`, available through its child view. Each concrete
exception object has one runtime base, with its own creation
trace and nullable readonly immediate cause. Initializing its domain parent creates no additional
exception or trace. A wrapper is a new exception object with its own creation trace and cause,
even in the same region. When the catch matches a cause, `failure.tip` still selects the
incoming tip for this handling scope.
[Exception runtime base](../design/ExceptionControl.md#runtime-base-of-an-exception)

Honor declared checked obligations. An explicit `throws Exception` remains checked even if a
particular implementation currently produces only unchecked errors. A runtime-type observation
cannot silently narrow the public contract.

[Deferred failure handling](../design/ExceptionControl.md#exceptions-between-deferred-actions),
[catch ownership](../design/ExceptionControl.md#exception-declarations-and-catch-bindings),
[application-facing failures](../design/ExceptionControl.md#application-facing-failure-contracts)
and [declared obligations](../design/ExceptionControl.md#declared-handling-obligations).
The reporting helper must not retain a borrow after the catch releases its holder.

## Traverse and compose through declared protocols

### Drive traversal through its declared feedback

A generator controls when it offers an item and what it does with the body's response. Boolean
false is not an automatic stop instruction. The generator has to act on it. Here
`finish_attempt()` is an externally supplied procedure that records completion and returns
normally.

```cod
proc candidates() yields u64 -> bool {
    try {
        if (not yield(7u64)) { return }
        yield(9u64) : void
    } finally {
        finish_attempt()
    }
    return
}

proc first_large() -> u64 {
    for val n <- candidates() {
        if (n >= 8u64) return n
        continue true
    }
    return 0u64
}
```

The first handoff returns true to `candidates`, which then offers 9. The body's return exits
`first_large`. The generator's finally runs before 9 is delivered to its caller. Its try encloses
both yields, so the finally runs when that protected construct exits, not after every handoff.

Supply the declared feedback on every normally completing body path. A path that returns from the
supplier owes no feedback because it never resumes the yield. For void feedback, use bare
`continue`; for non-void feedback, supply the required result. A normal result in a void-feedback
body is discarded with its effects and cleanup intact.

Use `break` when the traversal should finish and the supplier should continue after it. Use
`return` when the surrounding operation is complete. Put resource cleanup in finally or defer,
rather than only after a yield: an outward exit may never reach that later statement.

A failure escaping a single-brace consumer reaches its supplying loop or call site. A producer
catch around the handoff cannot intercept it; the producer can catch failures from its own work.
A double-brace supplied body instead has ordinary callback exception matching and a local return.
In both cases, entered cleanup remains required. A new escaping cleanup failure can prevent a
pending result from being delivered.

[Generalized loops](../design/RangedForAndGenerators.md#generalized-for-loops),
[explicit feedback](../design/RangedForAndGenerators.md#explicit-continue-feedback),
[finally](../design/RangedForAndGenerators.md#finally-and-defer-are-independent-of-the-transport)
and [supplied body forms](../design/CallableStorageAndHereAccess.md#single-brace-bodies-for-a-here-callable).
This example yields bits. A borrowed yield component needs one fixed, proven actual owner for the
traversal, and its consumer cannot extend that owner's lifetime.

### Name reusable pipeline connections

Use composition when stages have compatible traversal protocols and the intermediate connection
deserves a name. The invoice library in this example supplies three readonly processor functors,
each taking a `here` source yielding `u64` with void feedback and yielding `u64` downstream:

| Processor | Contract |
| --- | --- |
| `InvoiceFloor(base, discount)` | Compute the minimum whole-cent total once, rounded up; filter lower totals. |
| `WholeUnits(places)` | Divide each amount by the decimal divisor established at construction. |
| `FirstN(limit)` | Keep a fresh per-invocation count; stop upstream after the limit; skip input at zero. |

Select these processor definitions and methods. Assume no applicable library composition or
attachment overload and a procedure `report(u64)` that completes normally. These are the
[documented invoice processor contracts](../design/ProducersAndTransformers.md#processor-functors-in-either-direction).

```cod
proc report_invoices(here src() yields u64) -> void {
    ref floor = make InvoiceFloor(12500u32, 2000u16)
    ref units = make WholeUnits(2u8)
    ref limit = make FirstN(2u64)

    here stages = floor |> units |> limit
    here selected = src !> stages
    here reporting = stages $> report

    selected $> report
    return
}
```

The three bindings demonstrate the bounded partial shapes: `stages` has both ends open,
`selected` needs a consumer, and `reporting` needs a source. Binding prepares operands without
starting traversal. `src !> reporting` is an alternative to the final invocation, not an additional
required pass.

For an input offering 8000, 12050, 14099 and 16000 cents, the invocation reports 120 and 140. The
floor is 10000 cents, conversion divides by 100, and the limit stops upstream before it offers the
fourth item. Reusing a stage gives it fresh traversal-local state; reusing a source does not promise
that an already consumed cursor will replay.

For a policy that another caller must drive after its builder returns, see [Building reusable domain operations in CoD](BuildingReusableDomainOperations.md#keep-a-rule-after-the-builder-returns). Its fare search adds an audit terminal and a replay inventory to the same rule.

For prebound operands, the corresponding downstream-first spelling is:

```cod
report <$ limit <| units <| floor <! src
```

This has the same processing order. Computed operands still prepare once in their written order,
so reversing expressions with observable effects can change behavior. Bind those computations
explicitly before changing direction. Domain operator overloads retain their own effects and result
contracts; composition is not permission to cache mutable configuration or reorder library work.

Keep every receiver and source dependency alive through use. Rebinding an original processor slot
does not redirect the saved target, while permitted writes to that target remain observable.
A lexical connection cannot escape into a region or outlive its supplying activation. Forward
feedback and checked failures through every stage, including the feedback needed for filtered
items. A terminal outward return crosses the entered pipeline cleanups.

[Composition and attachment](../design/ProducersAndTransformers.md#processor-composition-and-source-attachment),
[lexical partials](../design/ProducersAndTransformers.md#lexical-variables-for-open-pipelines)
and [confirmed composite scope](../design/ProducersAndTransformers.md#composite-implementation-target).
All three processor owners in the example remain live until the single traversal and its cleanup
finish. Neither the partial bindings nor the reverse spelling grant ownership or access.

## Make module boundaries express intent

Use `using` when a published component should retain the interpretation its author selected.
Choose `import` when the receiving module deliberately adapts callable dependencies or extends a
selected family. Those choices can change a helper's behavior even when its body text is unchanged.

| Operation | Use it to express |
| --- | --- |
| `using` | Reuse a published definition with its selected source implementation context. |
| `import` | Select the receiving callable implementation context and obtain the applicable extension permission. Source bodies, non-callable lexical resources and captures retain their contracts. |
| `mixin` | Redefine an admitted procedure body in the receiving lexical and dynamic context, with its symbol-role mappings and definition-time capture rules. |
| `export` | Publish a selected identity under a public name. This is an onward interface decision. |

These operations do not make a source module mutable. Importing a type does not automatically turn
every same-named local function into an extension of an associated source family. Explicitly import
the callable you intend to extend. A local shadow and an extension carry different identities and
can lead to different dispatch.

For `cartons_sent(shipment)`, look up the callable name. The optional `with methods` clause can
bring associated public callable names into scope explicitly. For `matrix[row, col]`, selecting
the matrix type already carries its published indexed read family. An adapter can extend that family
by importing the receiver type. To extend the free indexing operator, import that callable.
These grants let a consumer choose the adapter’s view while another consumer keeps the original.
[Associated callables](../design/ModuleWorlds.md#explicit-associated-callables-with-with-methods) ·
[Receiver-owned operations](../design/DomainLanguageDesign.md#how-receiver-dispatch-works)

Treat the public declarations in the shipment example as `shipping.cod`. A consumer that uses the
model and its query together can state that dependency in one selection:

```cod
using "./shipping.cod" { Shipment, cartons_sent }
```

Keep the verb and the semantic purpose consistent within a group. Preserve separate selections when
they explain separate dependencies or when intervening declarations matter. A single selected name
needs no artificial group. Put `export` at a public definition so its visibility is apparent where
the reader learns what it means.

A library may designate a default public selection. After those shipment declarations, write:

```cod
default { Shipment, cartons_sent }
```

Now `using "./shipping.cod"` selects that same list. It does not select every public name.
`using "./shipping.cod" {..}` selects the complete explicit public table. If the library has no
default selection, name the required entries or request the complete table explicitly.

Omitted selectors substitute the publisher's default list under the verb actually written.
Replacing `using` with `import` or `mixin` preserves that verb's ordinary permissions and
requirements; defaults cannot make an otherwise invalid selection valid. `default export` can also
mark a name directly at its definition.

A facade that only republishes a model can use:

```cod
export from "./shipping.cod" Shipment
```

This publishes the upstream identity without introducing a local alias or requesting import
adaptation. Use `export import` when the facade intentionally combines receiving-context import
with publication. Neither form grants downstream users access to private definitions.

[Reuse and interpretation](../design/DomainLanguageDesign.md#extension-module-callable-identities-and-local-unions) ·
[Explicit extension](../design/ModuleWorlds.md#type-selection-and-shadowable-associated-methods) ·
[Grouped public selections](../design/ModuleWorlds.md#public-definitions-and-grouped-selections) ·
[Defaults and complete selections](../design/ModuleWorlds.md#default-and-complete-public-selections) ·
[Publication-only re-export](../design/ModuleWorlds.md#publication-only-re-export)

### State whether a module path is relative or logical

Use `./` for a source-relative dependency. Other module paths identify entries in the project's
resolved package namespace; a bare filename is a logical root entry.

| Path | Meaning |
| --- | --- |
| `"./shipping.cod"` | The sibling of the selecting source module. |
| `"warehouse/shipping.cod"` | That complete logical path in the resolved package namespace. |
| `"string.cod"` | The logical root entry, with the bundled library as fallback when no package module exists. |

Write the complete logical path even for another module in the same package. Package modules take
precedence over a bundled module of the same path, with a warning for that shadowing. A failed read
or check of the selected module reports its error; it does not silently choose the fallback.
Neither the process working directory nor an ordered directory search list determines the
interpretation of a logical dependency.

[Source module paths](../design/ModuleWorlds.md#resolving-source-module-paths)

### Select a symbol's identity before using its current binding

A module can share a symbolic variable's identity without reading an installed value. These two
alternative declarations select the same published output identity:

```cod
using "string.cod" { var @sout }
```

```cod
symbol from "string.cod" { @sout }
```

The `var` selector checks that the published resource is a symbolic variable; it does not grant a
writable target or acquire ownership. A later procedural read uses the binding installed at that
execution point. Select the identity in the vocabulary that needs it, and establish a live,
suitably accessible target before any path reads it.

A symbolic procedure selector uses `proc @name` under its receiving verb's rules. It does not turn
a procedure into an ordinary function value. Keep publication, identity selection, binding
installation and invocation as separate decisions.

[Symbolic variable identity](../design/ModuleWorlds.md#selecting-a-symbolic-variable) ·
[Kind-qualified selectors](../design/ModuleWorlds.md#default-and-complete-public-selections)

## Give domain values a display vocabulary

Keep a value's display logic with its domain vocabulary. The standard text library separates
streaming from collection: `emitStr` and `emitRepr` write to the scoped output; `str` and `repr`
collect the corresponding representation into immutable text. This lets one representation serve
a streamed report and a collected diagnostic without duplicating field-by-field formatting.

The following module publishes a small `Price` type and its adapted formatting families:

```cod
export data Price {
    cents:u64
}

export import "string.cod" { str, repr, emitStr, emitRepr, (++) }
import "string.cod" emit

proc emitStr(price:const Price in P) -> void where {const owner P} {
    emitStr(price.cents)
    emit(" cents")
    return
}

proc emitRepr(price:const Price in P) -> void where {const owner P} {
    emit("Price(cents=")
    emitStr(price.cents)
    emit(")")
    return
}
```

The imported families are adapted in this module's interpretation. Publishing the adapted
collectors with their hooks ensures that a consumer selects the interpretation containing these
methods. This is a concrete reason to use `import` when extending a library family; merely selecting
the original source-preserving collector does not move its internal lookup into this adapter.

Inside a procedure with this module's vocabulary selected:

```cod
ref price = make Price { cents <- 4200u64 }
ref label = str(price)
ref detail = repr(price)
```

The results contain `4200 cents` and `Price(cents=4200)`. The collectors install their own private
output for rendering and restore the preceding `@sout`. They require no existing writer for this
collection. A call to `emitStr(price)` instead requires a live installed output and writes there.
Ignoring the result of `str(price)` still performs collection; it never changes the call into
streaming.

Use `emitStr` or `emitRepr` for a nested value when writing a larger representation incrementally.
Use `str` or `repr` when the caller needs the text as a value. Give persisted or interoperable text
its own specified format: a user-defined diagnostic representation does not promise reconstruction
or version stability.

### Keep text working storage local; choose the owner of retained text

`StringBuilder` is lexical working storage. `++` builds text there, while `make String(builder) in D`
constructs immutable text in the explicitly chosen allocation-capable owner D. For example, inside
a procedure selecting `(++)` and the Price vocabulary from this adapter, with the standard String
construction selected and a live allocation-capable owner D:

```cod
ref price = make Price { cents <- 4200u64 }
ref builder = "Total: " ++ price
ref retained = make String(builder) in D
```

`retained` contains `Total: 4200 cents` and follows D's lifetime. The builder remains usable in its
own lexical scope. Constructing the String consumes the accumulated contents for the result, not
the builder's ownership or ability to be used again.

The standard text-only concatenations are functions. Formatting an arbitrary domain value uses
procedural display hooks, so choose a procedural context for that path. Operand preparation occurs
before rendering. A borrowed operand remains connected to its selected target; a later operand
that writes that target can affect what the formatter sees. When an earlier scalar snapshot is the
intent, prepare it explicitly in a `val` before the operation. Prefer such named preparation when
side effects would make a compact concatenation hard to read.

Put per-record builders inside a helper or explicit full scope when their workspace should end with
that record. Stream completed pieces when a report need not be retained as one String. Collect when
the caller actually needs the complete text, and make its owner visible when it must outlive the
current scope.

[Building text](../design/StringBuilding.md#building-a-piece-of-text) ·
[Display operand preparation](../design/StringBuilding.md#preparing-display-operands) ·
[Shared rendering hooks](../design/StringBuilding.md#user-defined-display-and-diagnostic-representations)

## Use canonical source form

CoD source follows the canonical style, including the initial maximum of **100 columns**.
Break long signatures at their structural boundaries and keep related information recognizable.
Canonical style is an acceptance requirement. CoD's planned formatter is almost configuration-free.
Automatic formatting in commit hooks, backed by
independent formatting checks at submission or integration, is the recommended enforcement workflow.

Omit a final empty return when falling through completes the same invocation with the same result
and cleanup. Keep an early return and a single-brace callback’s outward return. In a function or
full-scope here, check what becomes the tail before removing a return.
[Formatter examples](IdiomaticsGuideline.md#canonical-code-style)

Put spaces around an augmented transfer token, as in `stock <-:- 1u64`. The four characters
together specify a read, arithmetic operation and transfer back to the selected place.
Separate consecutive operators you intend to apply individually. Write `n <- -1` for a negative
assignment, `ready && !closed` for a negated condition, or `not not flag` for double negation.
[Symbolic operator tokens](../design/SymbolicDynamicScope.md#symbolic-operator-tokens) · [Augmented transfer](../design/SymbolicDynamicScope.md#augmented-transfer)

Omit semicolons when token boundaries already make the intended structure clear. Retain necessary
separators, including those separating ambiguous expressions or required by a receiving list.
A newline has the same effect as horizontal whitespace between tokens; moving code to another line
does not end a call or change callback attachment. A semicolon separates an expression without
discarding its result. Use an explicit `: void` when the intent is to discard a typed result;
its effects and cleanup still occur.

For example, suppose `run` has a defaulted niladic callback and `work() -> void` completes normally.
When the work must run independently after the call, make that boundary explicit:

```cod
run();
{{ work() }}
```

Omitting the separator attaches the block to the call. Its execution then depends on `run`'s
callback behavior. A line break alone cannot express the independent-scope intent.

In a method list, close a short expression with `;` before the next arm. A directly braced method
body ends at its own `}`. When a method body supplies a parameterized trailing `here` callback at
the list's direct level, put single braces around the **entire method body**. For example, suppose
`read_fee()` returns 10 and `apply_to_fee` calls its supplied body with that same fee.

```cod
proc adjusted_fee(extra:u64) -> u64 of {
    (0u64) => { read_fee() }
    (n:u64) => {
        apply_to_fee() (fee) => { fee + n }
    }
}
```

`adjusted_fee(0u64)` returns 10 and `adjusted_fee(2u64)` returns 12. The first arm can also be
written `(0u64) => read_fee();`. In the second, the inner `}` ends the callback and the outer `}`
ends the method. Keep both levels when wrapping lines. The same boundary applies in a grouped
`effect` installation. [Method arm boundaries](../design/SymbolicDynamicScope.md#method-arm-boundaries)

The shipment example's triple-quoted doclet sits directly before the exported function. It documents
that definition and contributes no runtime text value or effect. Its `[Shipment](Shipment)` link
resolves in the documented declaration's context. A renamed import does not retarget it to an
unrelated type in the consumer.

Put the operation's intent and useful preconditions in its doclet. Use ordinary `#` comments to
explain a local choice where the code needs it. Keep doclets in leading documentation position:
an intervening semicolon or unrelated item breaks attachment, while a triple-quoted expression in
a return remains a text result.

[Canonical style](IdiomaticsGuideline.md#canonical-code-style) ·
[Separators and whitespace](../design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux) ·
[Definition doclets](../design/DomainLanguageDesign.md#definition-doclets)

### Write numeric literals in context

Write `func marker() -> UInt8 { 255 }` and `grid[7]` when the result or index formal supplies the type. For an example comparing method bodies, use `1`, `2` and `3` under the same declared integer result. Use `7u64` when choosing `UInt64` changes which method runs. A module that routinely needs unsigned 32-bit integers can write `default UInt32` once. A spaced prefix `- 1` invokes negation, while `-1` is a signed literal. With Julia-style power grouping and `e` equal to 2, `-2^e` gives −4 and `(-2)^e` gives 4. [Language contract and compiler coverage](../design/NumericLiterals.md)

