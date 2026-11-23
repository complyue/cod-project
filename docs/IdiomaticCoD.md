# Idiomatic CoD

**Discussion draft.** These excerpts show the intended style, including confirmed forms whose frontend support is being rolled out.

## Semicolons

Omit a semicolon when the intended boundary is clear. Write it when the grammar needs it to distinguish two readings or when the reader would otherwise have to stop and reconstruct the grouping.

```cod
func increment(n:u64) -> u64 {
    n + 1u64
}
```

For a call that accepts a trailing block, these two independent operations need a separator:

```cod
run();
{{ work() }}
```

Without the semicolon, the block attaches to `run()` as its callback. A semicolon also leaves a function tail's result intact. In grammar design, minimize the forms that need explicit separators. [Semicolon omission and syntax UX](../design/SymbolicDynamicScope.md#semicolon-omission-and-syntax-ux)

## Modules

### Do put publication at the definition and group related selections

```cod
export data SourcePosition {
    line:u64;
    column:u64;
}

using "SourceStorage.cod" { SourceBytes, SourceLines };
using "SourceStorage.cod" SourcePosition;
```

Here the byte and line views form one input. A separate selection can call out a dependency used for a different purpose, such as formatting positions.

### Don’t repeat paths or mix selection purposes

```cod
using "ParserStorage.cod" Tokens;
using "ParserStorage.cod" ExprNodes;

data ParsePlan {
    node_capacity:u64;
}
export ParsePlan;
```

Group names that work together in one operation. Put the public marker where readers meet the definition. Keep distinct selection verbs and meaningful groups separate. [Public definitions and grouped selections](../design/ModuleWorlds.md#public-definitions-and-grouped-selections)

## Signatures

### Do indent long signatures by structure

```cod
proc walk_expression(
    here begin(first:u64, past:u64) -> bool,
    here visit(
        ordinal:u64, kind:u8, number:u64, left:u64, right:u64, operation_at:u64
    ) -> bool,
    here finish(visited:u64) -> void,
    source:const SourceBytes in S, nodes:const ExprNodes in N,
    result:VisitResult in R
) -> bool where {const owner S, const owner N, const owner R} {
    # Body omitted.
}
```

Start a long parameter list on the next line. Give each `here` formal its own line, indent its nested parameters one level, and keep short related inputs together. Keep the closing parenthesis, result and `where` together when they fit.

### Don’t pad continuations to the opening parenthesis

```cod
func record_parse_diagnostic(destination:ParseDiagnostic in D,
                             stage:u8, code:u32, first:u64,
                             past:u64) -> void where {const owner D} {
    # Body omitted.
}
```

Use a normal indentation level. Keep the span bounds together so a reader can see the range at a glance.

## Callable kind and dynamic context

### Do choose the callable kind by what the body needs

```cod
func increment(cursor:u64 in O) -> void where {const owner O} {
    cursor <- cursor + 1u64
}

symbol @budget:u64;

proc current_budget() -> u64 {
    return @budget
}
```

A `func` can read and write explicitly supplied storage and use intrinsics whose inputs are explicit. Access to dynamic symbolic bindings such as `@budget` and calls to a `proc` require a procedure. Mutation through a writable borrow is part of the ordinary function contract. Purity has its own contract. [Functions and procedures](../design/SymbolicDynamicScope.md#functions-and-procedures) · [Const-function boundary](../design/ConstFunctionsAndTailCalls.md#boundary-of-a-const-function)

### Don’t hide a procedure call in a func

```cod
func within_budget(cost:u64) -> bool {
    return cost <= current_budget()
}
```

`current_budget` is a procedure, so a wrapper that calls it needs `proc`. The restriction also applies to indirect calls. A `here` body supplied by a procedure runs under the permissions of its own lexical site when a function invokes it. [Here callables and direct lexical access](../design/CallableViewsAndCaptures.md#here-callables-and-direct-lexical-access)

### Do distinguish a direct call, symbolic dispatch, and installation

```cod
check_revision(source, lines)

symbol @rate:u64;
symbol @quote:proc()->u64;
effect @rate <- 10u64
effect @quote() -> u64 { return @rate }
effect @rate <- 20u64
val current <- behave @quote()    # 20
val captured <- perform @quote()  # 10
```

A direct call resolves a lexical name. An `effect` installs a binding in the current dynamic scope. Both symbolic call forms select the procedure through its active symbol binding. `behave` uses the current dynamic bindings. `perform` temporarily installs the selected procedure’s captured dynamic values. [Functions and procedures](../design/SymbolicDynamicScope.md#functions-and-procedures)

## Access and storage

### Do show access, shared owners, and storage operations

```cod
func next_byte(source:const SourceBytes in S, cursor:u64 in O) -> u8
    where {const owner S, const owner O} {
    val byte <- source[cursor]
    cursor <- cursor + 1u64
    return byte
}

val checkpoint <- cursor
ref active = cursor
active <- checkpoint
```

`const T` grants readonly access through that parameter. `const owner O` withholds allocation authority through that owner parameter, and the writable borrow still permits the cursor update. Use `val` for independent bits. In `ref active = cursor`, `=` binds the reference. `active <- checkpoint` writes the selected target.

### Don’t demand extra access or tie independent inputs to one owner

```cod
func source_size(source:SourceBytes in S) -> u64 where {owner S} {
    return len(source)
}

func compare_sources(left:const SourceBytes in S, right:const SourceBytes in S)
    -> bool where {const owner S} {
    # Body omitted.
}
```

A length query needs readonly access. Give independent source inputs separate owner parameters. Reuse an owner parameter when the operation actually requires the same owner. [Owner-qualified parameters and access](../design/RegionalOwnership.md#confirmed-owner-syntax)

## Nullable arguments

### Do use an ordinary branch for an optional target

```cod
proc update_count(count:u64 in O) -> void where {const owner O} {
    count <- count + 1u64
    return
}

proc update_if_present(ptr pending_count:u64 in O) -> bool where {const owner O} {
    if (pending_count is null) return false
    update_count(pending_count)
    return true
}
```

Absence returns an ordinary result. On the remaining path the branch proves that the pointer is present, so it can be passed directly to the `ref` formal written as `count:u64 in O`. This use has no null exception to catch or declare. The booter’s current scalar checker can prove presence from a pointer initializer. Branch-based proof still needs booter support.

### Don’t pass an unproved pointer directly to a required reference

```cod
proc update_without_proof(ptr pending_count:u64 in O) -> void
    where {const owner O} {
    update_count(pending_count)
    return
}
```

The call is rejected because presence is unproved. A `throws` clause would leave that obligation in place. Declare a `ptr` formal when handling absence belongs to the callee. [Owner-qualified reference arguments](../design/RegionalOwnership.md#owner-qualified-reference-arguments)

### Don’t turn normal absence into an exception

```cod
using "std:exceptions" NullPointerException;

proc visit_next(ptr next_token:const Token in O) -> void
    throws NullPointerException where {const owner O} {
    visit_token(not null next_token)
    return
}
```

When an absent next token means the input ended, take the end-of-input branch. Reserve `not null` for a selection whose absence should actually fail with checked `NullPointerException`.

## Loop scopes

### Do give iteration-local declarations an explicit scope

```cod
for (index in 0u64 .. used) {{
    val first <- nodes[index].first
    check_offset(first)
}}
```

Use `{{ ... }}` when an iteration needs local declarations or a separate cleanup boundary.

### Don’t add a scope just to group statements

```cod
for (index in 0u64 .. used) {{
    check_node(nodes[index])
}}
```

Use ordinary braces for a body that needs no local scope. A full scope has its own lifetime, cleanup and return destination. [Full scopes in a loop body](../design/RangedForAndGenerators.md#full-scopes-inside-a-loop-body)
