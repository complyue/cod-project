# Numeric literals and module defaults

## Design authority

Revision 2, 2026-11-27. The [perfect-match decision](../CHANGELOG.md#h-perfect-numeric-overload-matches-20261127) adds the two-method acceptance case below to r1’s source-ordered defaults and contextual typing. [Contextual numeric literals](DesignAuthorityRegistry.md#d-11-literal-defaults) covers typing, defaults and diagnostics. [Signed literal grouping](DesignAuthorityRegistry.md#d-10-literal-sign) covers syntax and formatting. The [project user's decisions](../CHANGELOG.md#h-numeric-literals-and-signs-20261127) add these scopes to the [scalar](ScalarUnitsAndIndexTypes.md#quantity-types-and-lossless-widening) and [operator grouping](JuliaPackagePortability.md#operator-grouping-belongs-to-the-selected-vocabulary) contracts. Ordinary numbers can be written once while their interpretation stays reproducible from the source and selected vocabulary. P-782–P-785 record the value and dependency edges. The examples below give the acceptance cases. Existing typed arithmetic, nominal identities and checked failures continue. General untyped constant arithmetic and implementation scheduling have separate scope.

## Write the number the program means

Use a bare number when the context determines its type. In `func marker() -> UInt8 { 255 }` the return type determines `UInt8`. In `packet[7]` an independently selected `UInt64` index formal supplies the type. A suffix such as `7u64` explicitly chooses a type when that choice matters. Once a binding has a type, ordinary typed-value rules apply to its uses. A unitless number stays unitless, `10:ASCII` still decodes against the nominal enum, and `//` retains its planned fraction meaning.

The parser records the literal's spelling, span, integer or decimal form, sign and any written suffix. Preserve enough source information to interpret its mathematical value before choosing a width. Semantic analysis resolves types and checks representability. An integer in a known `UInt8` context must fit 0 through 255. Report `256` there even if a wider type appears in the defaults. In a standalone binding without a numeric directive or other type evidence, the baseline integer and decimal types are `Int64` and `Float64`.

## Numeric defaults in source order

`default UInt8, UInt64` records an ordered list of numeric type names. It applies to subsequent occurrences in the module until another numeric directive takes effect. Bind the written names semantically, including transparent synonyms. The braced `default { ... }` form selects [default exports](ModuleWorlds.md#default-and-complete-public-selections). Each form has its own syntax node.

First use ordinary applicability, specificity and the available type constraints. A uniquely inferred type wins before consulting the directive. For an unresolved literal select an admissible type from the ordered defaults. An explicitly listed floating type can receive an integer spelling exactly. Thus `default Float32` can give a standalone `1` type `Float32`. With `default Float32, UInt64`, `16777217` takes `UInt64` because `Float32` would lose a unit. Decimal literals admit floating rounding. An integer-only directive leaves the ordinary decimal default available for `0.5`. An integer too large for the declared integer choices needs a wider stated type.

```cod
default UInt8, UInt64
val a <- 255
val b <- 256
val c <- 255 + 1
```

Read the bindings independently. `a` has type `UInt8` and `b` has type `UInt64`. Both operands of the ordinary checked addition in `c` fit `UInt8`. The expression has type `UInt8` and raises overflow when evaluated. A future enhanced diagnostic should warn that overflow is certain and suggest `255u64 + 1` when 256 is intended. Constant folding must preserve the selected operation and failure. Report an unresolved numeric ambiguity as an error with the competing choices. For a permitted interpretation with material precision risk, explain the source value and the chosen representation.

**Overload ties.** Apply numeric preferences to the eligible methods of an otherwise unresolved call. If `send` accepts either `UInt32` or `UInt64`, `default UInt32` settles `send(7)` in favor of the exact `UInt32` choice. With only `UInt8` and `UInt64` overloads the same directive gives neither an exact preference: report ambiguity. With only the `UInt8` overload, `7` takes `UInt8` immediately.

Compare the unresolved numeric positions across all arguments within the eligible dispatch group. For `default UInt32, UInt64` and `f(1, 2)`, the signatures `(UInt32, UInt64)` and `(UInt64, UInt32)` tie. Swapping the argument positions together with their formals preserves the outcome. A suffix can supply the missing type evidence.

```cod
default UInt8, UInt32, UInt64, Int64
func choose(x:UInt8,  y:Int64)  -> Int64 { 14 }
func choose(x:UInt32, y:UInt32) -> Int64 { 22 }
choose(1, 2)    # Static error at the call: both methods match perfectly.
```

Here both literals fit exactly into each method’s respective parameter types, and all those types occur in the directive. The first method uses positions 1 and 4 of the list and the second uses positions 2 and 2. Both are perfect matches. A sum of ranks or a count of first-choice matches would incorrectly pick a body. Both signatures are valid. With `choose(1, 2i64)` only the first method can receive the typed second argument. The call `choose(1u32, 2)` selects the second. A standalone literal still uses the source list in order once other type evidence is exhausted.

**Scope.** The written site owns the numeric policy. An earlier function retains its defaults when called after a later directive or from another module. In a formal default expression `width = 7`, the `7` follows the defining site even when argument preparation runs at the caller. A caller writing `draw(7)` supplies a new literal under its own policy. Where `import` adapts a callable family, normal selection still uses that view, with each literal's source policy intact.

## Group first, then recognize a sign

Read the superscript to see what is being raised to a power. In the Julia vocabulary, with `e` bound to 2:

| Expression | Grouping | Result |
| --- | --- | --- |
| `-2^e` | Negate the result of `2^e` | −4 |
| `(-2)^e` | Raise the literal −2 to `e` | 4 |
| `b^e` after `val b <- -2` | Raise the previously bound value of `b` to `e` | 4 |

The same grouping holds when `e` is supplied at runtime. After grouping, a prefix minus touching the first digit of its complete numeral operand belongs to the signed literal. Thus `-1` reports a negative literal, `- 1` and `-(1)` invoke unary negation, and `x-1` uses infix subtraction. A formatter preserves sign adjacency. Existing longest operator runs still apply, so `x - -1` subtracts a negative literal. Preserve the sign of floating `-0.0`. Semantic analysis accepts `-9223372036854775808` in an `Int64` context and rejects `-1u8` at the literal.

In parser code that handles numeric prefix minus, keep a comment with the `e = 2` example at the grouping decision. The source reader needs token spans and the selected grouping rules. It never needs the value of `e` or a numeric default to distinguish the two trees. [Operator tokens](SymbolicDynamicScope.md#symbolic-operator-tokens) · [Julia precedence](https://docs.julialang.org/en/v1/manual/mathematical-operations/#Operator-Precedence-and-Associativity)

## Compiler coverage

The bootstrap parsers currently cover smaller fixed grammars. The seed's [prefix parser](../cod/lang_syntax.hh) constructs a unary node and its [literal checker](../cod/lang_scalar_operations.hh) assigns concrete scalar types early. The booter's [expression parser](../applications/cod-booter/src/ExpressionParser.cod) has a limited signed-integer path. This revision records the intended reader and semantic behavior and the examples that distinguish them. Source-level default lists, general contextual typing, library power grouping and future overflow warnings require implementation evidence. [Idiomatic examples](../docs/IdiomaticCoD.md#write-numeric-literals-in-context)
