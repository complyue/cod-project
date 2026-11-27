# Numeric literals and module defaults

## Design authority

Revision 4, 2026-11-27. [D-11-LITERAL-DEFAULTS](DesignAuthorityRegistry.md#d-11-literal-defaults) records the [project user's dispatch and inference order](../CHANGELOG.md#h-dispatch-before-literal-typing-20261127). A program can use an ordinary numeral wherever a signature supplies its type and can rely on a diagnostic when two signatures fit equally well. The [integer](../CHANGELOG.md#h-perfect-numeric-overload-matches-20261127) and [floating](../CHANGELOG.md#h-floating-literal-signature-matching-20261127) applicability rules, written-site defaults, typed arithmetic and checked failures continue from r3 under P-782/P-783. [Signed literal grouping](DesignAuthorityRegistry.md#d-10-literal-sign) retains its separate scope under P-784/P-785. The examples below specify acceptance. General untyped constant arithmetic and compiler delivery have separate scope.

## Dispatch before literal typing

1. Match the call with each unsuffixed numeric literal still untyped. Use its spelling and mathematical value for the applicable integer or floating matching rule, alongside the other arguments and the ordinary dispatch rules for the call form. If no candidate applies, report a no-match error. Multiple candidates still tied after specificity give a static ambiguity.
2. Once dispatch selects a unique signature, infer each literal argument’s type from its corresponding parameter and the available constraints. For a concrete formal the parameter type determines the representation, including any rounding of a floating literal during argument preparation.
3. Consult the written site’s ordered numeric `default` list only for a literal whose type is still undetermined after all other type inference is exhausted. This includes a standalone binding with no other type evidence. A failed or ambiguous dispatch ends at step 1.

For example, under `default UInt8`, a sole `send(x:UInt64)` receives `send(7)` as `UInt64`. Adding an equally applicable `send(x:UInt8)` makes the same call ambiguous. Writing `val n <- 7` separately gives `n` type `UInt8` by default, so `send(n)` selects the `UInt8` method.

## Write the number the program means

Use a bare number when the context determines its type. In `func marker() -> UInt8 { 255 }` the return type determines `UInt8`. In `packet[7]` an independently selected `UInt64` index formal supplies the type. A suffix such as `7u64` explicitly chooses a type when that choice matters. Once a binding has a type, ordinary typed-value rules apply to its uses. A unitless number stays unitless, `10:ASCII` still decodes against the nominal enum, and `//` retains its planned fraction meaning.

The parser records the literal's spelling, span, integer or decimal form, sign and any written suffix. Preserve enough source information to interpret its mathematical value before choosing a width. Semantic analysis resolves types and checks representability. An integer in a known `UInt8` context must fit 0 through 255. Report `256` there even if a wider type appears in the defaults. In a standalone binding without a numeric directive or other type evidence, the baseline integer and decimal types are `Int64` and `Float64`.

## Numeric defaults in source order

`default UInt8, UInt64` records an ordered list of numeric type names. It applies to subsequent occurrences in the module until another numeric directive takes effect. Bind the written names semantically, including transparent synonyms. The braced `default { ... }` form selects [default exports](ModuleWorlds.md#default-and-complete-public-selections). Each form has its own syntax node.

When inference leaves the literal untyped, choose the first admissible type in the ordered list. An explicitly listed floating type can receive an integer spelling exactly. Thus `default Float32` can give a standalone `1` type `Float32`. With `default Float32, UInt64`, `16777217` takes `UInt64` because `Float32` would lose a unit. Decimal literals admit floating rounding. An integer-only directive leaves the ordinary decimal default available for `0.5`. An integer too large for the declared integer choices needs a wider stated type.

```cod
default UInt8, UInt64
val a <- 255
val b <- 256
val c <- 255 + 1
```

Read the bindings independently. `a` has type `UInt8` and `b` has type `UInt64`. For `c`, assume ordinary checked addition has already been selected and only its common operand type is undetermined. Both operands fit the first listed type, `UInt8`. If the visible `+` overloads instead leave two equally applicable signatures, the expression is ambiguous at the operator. The expression has type `UInt8` and raises overflow when evaluated. A future enhanced diagnostic should warn that overflow is certain and suggest `255u64 + 1` when 256 is intended. Constant folding must preserve the selected operation and failure. Report an unresolved numeric ambiguity as an error with the competing choices. For a permitted interpretation with material precision risk, explain the source value and the chosen representation.

**Integer literals during overload selection.** Compare a bare integer numeral’s mathematical value with each candidate’s independently known numeric formal. Every admissible type that can represent the value losslessly is a perfect match for that literal, regardless of its position or presence in the default list. Ordinary applicability and specificity still apply to the methods and the other arguments. If otherwise equally good methods both match the unresolved literals perfectly, diagnose ambiguity at the call. This comparison never consults the module’s numeric defaults.

```cod
func tag(x:Int64)  -> Int64 { 1 }
func tag(x:UInt64) -> Int64 { 2 }

default Int64
tag(7)                 # Ambiguous: 7 fits both formals exactly.
val value <- 7         # Standalone literal takes Int64.
tag(value)             # Returns 1: the argument already has type Int64.

default Int64, UInt64
tag(7)                 # Still ambiguous.
tag(7u64)              # Returns 2.
```

Compare the unresolved numeric positions across all arguments within the eligible dispatch group. For `default UInt32, UInt64` and `f(1, 2)`, the signatures `(UInt32, UInt64)` and `(UInt64, UInt32)` tie. Swapping the argument positions together with their formals preserves the outcome. A suffix can supply the missing type evidence.

```cod
default UInt8, UInt32, UInt64, Int64
func choose(x:UInt8,  y:Int64)  -> Int64 { 14 }
func choose(x:UInt32, y:UInt32) -> Int64 { 22 }
choose(1, 2)    # Static error at the call: both methods match perfectly.
```

Here both literals fit exactly into each method’s respective parameter types. Removing the directive gives the same ambiguity. The first method uses positions 1 and 4 of the list and the second uses positions 2 and 2. Both are perfect matches. A sum of ranks or a count of first-choice matches would incorrectly pick a body. Both signatures are valid. With `choose(1, 2i64)` only the first method can receive the typed second argument. The call `choose(1u32, 2)` selects the second. A standalone literal still uses the source list in order once other type evidence is exhausted.

**Scope.** The written site owns the numeric policy. An earlier function retains its defaults when called after a later directive or from another module. In a formal default expression `width = 7`, the `7` follows the defining site even when argument preparation runs at the caller. A caller writing `draw(7)` supplies a new literal under its own policy. Where `import` adapts a callable family, normal selection still uses that view, with each literal's source policy intact.

### Floating literals in overloaded calls

For a bare floating literal, every otherwise admissible floating formal matches, whether its format represents the value exactly or rounds it. If multiple methods remain after ordinary signature matching in the eligible dispatch group, report a static ambiguity at the call. Numeric defaults supply no preference between these candidates.

```cod
func sample(x:Float32) -> Int64 { 32 }
func sample(x:Float64) -> Int64 { 64 }

default Float32
sample(0.5)           # Ambiguous: exact in both formats.
sample(0.1)           # Ambiguous: rounds in both formats.
sample(16777217.0)    # Ambiguous: exact only in Float64.
```

Check these calls independently. If only the first method is visible, `sample(0.1)` selects it and the argument rounds to `Float32`. With both methods visible, `val x <- 0.1` under `default Float32` gives the standalone binding a type. A subsequent `sample(x)` selects the exact `Float32` signature under the rules for typed arguments. Rounding and precision diagnostics apply when preparing the chosen type.

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
