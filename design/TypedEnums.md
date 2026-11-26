# Typed enums

Design revision: 8 · 2026-11-25

## Design authority

At r8 the [guide’s ASCII vocabulary example](../docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner) uses the [preferred constant spelling and default selection](../CHANGELOG.md#k-ascii-vocabulary-20261125). Read a default member by name and a non-default member in an expression with its enum type. The r7 value and membership rules continue.

At r7 the [project user’s decision](../CHANGELOG.md#h-enum-backing-coercion-20261125) qualifies [D-07-ENUM](DesignAuthorityRegistry.md#d-07-enum), P-658/P-659, for implicit use of a member's backing value and explicit decoding with `code:E`. Check a runtime member, an alias, an invalid code and the raw-byte lexer workload. Nominal coverage and checked inbound C exchange continue from r6.

The [grouped method decision](../CHANGELOG.md#h-grouped-method-dispatch-20261125) carries the r5 enum identity and member-lookup contract into method dispatch at r6 under [D-10-METHOD-GROUPS](DesignAuthorityRegistry.md#d-10-method-groups). Check a runtime member, an alternate spelling and a same-code value of a different enum against the public parameter domain. The generated C exchange keeps its existing contract.

[H-TYPED-ENUM-20261121](../CHANGELOG.md#h-typed-enum-20261121) covers the project user's proposal, explicit requirement for old and new member names, and delegation of the detailed review. [D-07-ENUM](DesignAuthorityRegistry.md#d-07-enum), P-658/P-659, qualifies the closed nominal type, member identities, selection and coverage. [D-20-C-ENUM](DesignAuthorityRegistry.md#d-20-c-enum), P-660/P-661, qualifies generated current-target by-value exchange for the first booter workload. Charter §§3.2, 6.3, 8.1 and 12 connect named domain outcomes to readable programs and checked native execution. [K-TYPED-ENUM-REVIEW-20261121](../CHANGELOG.md#k-typed-enum-review-20261121) records the delegated resolutions below. Acceptance exercises the same identities from declaration through case analysis and the C boundary. Shared foreign storage and permanent binary interfaces remain outside this scope.

[H-MODULE-SOURCE-STYLE-20261121](../CHANGELOG.md#h-module-source-style-20261121) continues r1 to r2 for the public declaration example. The [module contract](ModuleWorlds.md#public-definitions-and-grouped-selections) supplies its publication semantics. [K-ENUM-STATUS-WIRING](../CHANGELOG.md#k-enum-status-wiring) continues r2 to r3 for the first workload’s source links and keeps the accepted exchange and validation contract.

The [short case arm decision](../CHANGELOG.md#h-short-case-arms-20261123) qualifies enum r4 for source spelling and case coverage under D-07-ENUM. Verify both spellings of one member with the subject’s member table and a shadowing lexical name. The [r5 guard spelling](../CHANGELOG.md#h-guarded-short-heads-20261123) applies the same identity and coverage rules when a predicate follows a bare member.

## Declaration and value identity

```cod
export enum NativeError:u8 {
    Complete = 0;
    Rejected = 1;
    OldRejected = Rejected;
    FormUnsupported;
    OpenFailed;
    LoweringFailed;
    FinishFailed;
    ScratchFailed;
    SourceOpenFailed;
}
```

This declares eight distinct values with codes 0 through 7. `OldRejected` is a **name/identity alias** for `Rejected`. Either spelling produces the same nominal value and backing code. A different enum with the same backing type and codes has a different type identity. A name ending in `Error` has no bearing on the exception hierarchy.

- An enum declaration has a name, an explicit fixed-width integer backing type and at least one value. Use a supported signed or unsigned integral bits type. The first native workload requires `u8`. Recognize `enum` contextually at a declaration head, preserving ordinary identifiers in other positions.
- A canonical member can specify an integer literal representable in the backing type, including a negative literal where admissible. With no explicit code, the first canonical member gets zero and each subsequent one gets the previous canonical member’s code plus one. Compute that successor exactly, then check the backing range and uniqueness. For example, `A = 2; B = 1; C;` rejects because C would also be 2. Explicit gaps and out-of-order explicit codes are valid. Overflow rejects at declaration time.
- The RHS of a name/identity alias must name an earlier member of this declaration. A chain resolves to one canonical member. It consumes no code and leaves the next automatic number unchanged. Reject duplicate spellings, independently declared equal codes, self/forward/cyclic references and names from another enum. A full enum can still add another spelling for an existing value.
- The first declaring name identifies the canonical member in declaration metadata. Runtime values do not remember which spelling supplied them. Adding a spelling preserves codes and coverage. Removing a spelling can break source imports. Adding a distinct member can invalidate an exhaustive case. Published codes that must survive reordering should be explicit. These rules supply no default textual format, serialization policy or cross-version binary guarantee.

An enum value has ordinary scalar `val` semantics and the size/alignment of its backing type on the current target. A stored slot, ref or ptr to an enum follows the usual owner, access and initialization rules. Enums can be bits fields and array elements in CoD lexical or regional storage. Ordinary typed initialization must establish a declared member before a read. Zero-filled storage does not initialize a variable, and zero need not denote any member. A nominal sum with payloads retains its own allocation, constructors and tag matching.

## Using an enum as its backing value

Given `enum E:B`, a value of `E` implicitly supplies its declared integer code wherever
a by-value `B` is required. This applies to runtime values and to constants. Assigning to a
`B` value slot, returning from a function with result `B`, or passing an argument to a
parameter of type `B` each supplies that code. In `byte == LF`, an existing `u8` operand
and an `ASCII:u8` member can therefore be compared as bytes. Aliases yield the same code.
A call with an exact enum overload uses ordinary exact-match priority. Given only the
backing-typed overload, its body receives the prepared integer.

The source enum retains its nominal type. Same-enum equality uses member identity,
including aliases. Direct equality between unrelated enum domains retains its existing
rejection. A caller can compare encodings by supplying the values to explicitly
backing-typed slots. Integer values enter an enum through explicit checked decoding.
The value coercion grants no writable view of enum storage through a backing-typed reference.
See the [byte-oriented lexer example](../docs/IdiomaticCoD.md#ascii-members-in-a-byte-scanner).

### Decoding an integer into an enum

Use `10:ASCII` to obtain the member declared with code 10. A parameter or result expecting
`ASCII` requires the caller or producer to write the conversion when supplying an integer.
For an integer literal or a value of the backing type, compare its integer value to the
declared codes. A sparse domain requires an exact member. Check before narrowing: with
`LF = 10` in a `u8`-backed enum, `266:ASCII` is invalid. Aliases for 10 yield one member.

Use the [existing assertion failure rules](SymbolicDynamicScope.md#nonnumeric-assertion-contract).
A provably invalid code rejects statically. For a runtime byte, `byte:ASCII` evaluates
the operand once, returns the matching member or raises the unchecked
`TypeAssertionFailedError`. Preserve operand effects even when a proof removes the check.
A native caller passing an invalid code follows the [generated C boundary](#generated-c-exchange).

## Names and case matching

```cod
using "./Compiler.cod" NativeError { Rejected, OpenFailed };
val result1 <- Complete:NativeError;
val result2 <- Rejected;

func exit_status(ne:NativeError) -> u8 {
    return case ne : NativeError of {
        OldRejected -> 1u8;
        FormUnsupported -> 2u8;
        _ -> throw;
    };
}
```

Exporting the type publishes its complete member table. `using` and `import` may select individual spellings or `{..}` for all, following the existing [type selector grammar](DomainLanguageDesign.md#publishing-and-selecting-data-constructors). A selector is checked against that type, including old spellings. It introduces the selected names for ordinary expression lookup. Selecting the type alone still supplies its full definition for typed access and coverage. Unselected members do not become ordinary module names merely through publication of the type. Re-export preserves type and member identities. Multiple paths to the same definition retain one domain. Neither imports, adapters nor parallel module worlds may add codes or further member names to that definition. Ordinary method selection has its existing permissions.

In an expression `name:E`, first resolve the visible type E and use ordinary lookup for the operand. A lexically bound value takes precedence, even if asserting it as E then fails. Only when the single identifier has no ordinary binding and E identifies a concrete enum may the checker resolve it in E's member table. An ambiguous or inaccessible name stays an error. For example, with a local `Complete:u8` equal to 1, `Complete:NativeError` decodes that byte and yields `Rejected`. With no such binding, the same expression denotes member `Complete`. A bare `Complete` without an ordinary binding still fails. An expected argument or return type does not search for enum members. An integer operand uses [explicit enum decoding](#decoding-an-integer-into-an-enum).

In either member-pattern spelling, `OldRejected -> expr` or `(OldRejected) -> { expr }`, the static enum type of the subject supplies the member table. The token names a constant pattern; it introduces no variable and reads no shadowing local. A misspelling rejects. In `OldRejected | retryable() -> retry()`, the member test precedes the guard. If the guard declines, scanning continues. Names inside the guard and body use ordinary expression lookup. A member has no variant payload to receive. `isa` remains a type test. [Case semantics](DomainLanguageDesign.md#case-expressions-and-branch-local-variables) govern subject evaluation, source-order effects, guards, fallthrough and result completion. The explicit wildcard `_ -> throw;` raises `UnexpectedCaseError` when reached.

Coverage counts distinct canonical members of the complete declaration, independently of the selected names. For `enum Decision:u8 { Yes; OldYes = Yes; No; }`, unguarded completing arms for OldYes and No cover the entire domain. Arms for Yes and OldYes leave No uncovered. A guard that can decline or a matching arm that always falls through supplies no independent proof of completion. Repeated spellings may still have different guards or lead to later observable work. A void case may be partial. Value cases follow the existing static completion/fallback rule. A nullable subject also needs the usual null handling. Coverage must respect possible changes to a borrowed subject during matching.

## Enum members in method heads

In a [grouped method definition](SymbolicDynamicScope.md#grouped-method-definitions-and-value-dispatch) a parameter declared `ne:NativeError` gives the pattern `(Rejected)` its enum domain. It matches the prepared argument whether the caller used `Rejected`, used `OldRejected`, or obtained the value at runtime. A local variable of either name cannot change the pattern's meaning. Two otherwise identical methods for the two spellings are duplicate definitions of one domain. The integer `1u8` and a member with code 1 in a different enum do not denote that domain. When complete coverage is required, count distinct members of the full enum. `case` continues to try its arms in source order. Method selection uses the unique most-specific applicable method across the selected family.

When the public slot is instead the backing integer type, a lexically selected member in
`=name` supplies its code in that slot. With `LF` selected from `ASCII:u8`, `(=LF)` and
`(10u8)` denote the same singleton in a `u8` slot. Defining both with otherwise identical
heads is a duplicate. An `ASCII` slot uses `(LF)` for its nominal member pattern.

## Generated C exchange

Use a checked CoD signature to generate a current-target C typedef of the exact backing width and signedness when a matching C type exists. For `u8`, use `uint8_t`. Generate collision-free module/type-scoped integer constants for **every published spelling**, including two constants of value 1 in the example. Keep each nominal enum in checked callable signatures even when the C typedefs have the same underlying type. Avoid relying on the implementation-defined representation of a C enum. A missing exact mapping rejects that boundary declaration.

- Encode a valid CoD value using its declared code. When C supplies an ordinary argument, result or normal callback feedback, check exact membership before calling CoD with it or publishing it to a CoD destination. Sparse domains need membership, not a min/max test. Preserve the full backing range, including valid extreme values. Multiple names for one code decode to one value.
- An undeclared incoming code is a native boundary failure. The first generated profile reports `COD_NATIVE_BOUNDARY_ERROR` through its existing invalid-provider/entry path. An invalid C argument prevents entry and an invalid normal result leaves the receiving output unpublished. On the CoD side use the same failure/cleanup path as a malformed native provider response. The invalid byte cannot reach a case fallback or become a domain rejection status.
- Inspect completion **before** reading a normal result or feedback slot. A checked constructor failure and a protected here return or throw follow their own carriers and preserve the original failure/control transfer and cleanup. Their normal payload storage may be uninitialized. For a valid normal status, including a nonzero failure reported *as a value*, the native transport itself succeeds.
- The first native profile covers by-value arguments, ordinary results and the normal feedback/result forwarding required by the CLI. CoD-owned bits storage stays eligible under its ordinary rules. A writable foreign view, foreign pointer, record containing an enum, or file-backed input from another producer would require validation for all observable reads and writes. By-value admission alone cannot authorize such a crossing. Keep those shared-storage interfaces outside this slice, including zero-copy validation and permanent ABI/versioning.

## First workload and acceptance

[CompilerStatus](../applications/cod-booter/src/CompilerStatus.cod) defines `Complete=0`, `Rejected=1`, `FormUnsupported=2`, `OpenFailed=3`, `LoweringFailed=4`, `FinishFailed=5`, `ScratchFailed=6` and `SourceOpenFailed=7`. [Compiler.cod](../applications/cod-booter/src/Compiler.cod) re-exports the type. [ObjectCompiler.cod](../applications/cod-booter/src/ObjectCompiler.cod) returns `SourceOpenFailed` for a caught source-open exception. The [CLI entry](../applications/cod-booter/src/__main__.cod) carries this type through ordinary results and normal `source_use`, `scratch_use` and `catalog_use` feedback. Preserve all eight CLI exit codes, diagnostic fields and resource cleanup. Raw LLVM operation statuses, OS errors, diagnostic subcodes, launcher-only exit codes and existing arbitrary-`u8` callbacks have separate domains.

Acceptance for the seed and seed-built booter includes the following observations.

1. Parse, publish and select the enum across modules. Preserve nominal signatures, lexical shadowing, sparse and signed codes where supported, checked numbering and name/identity aliases. Reject duplicate codes, overflow, cross-enum equality and invalid selections. A normal initialized regional bits field retains its typed value.
2. Demonstrate a complete case using either old or new spelling. Two spellings cannot stand in for a missing member. Exercise a guard that declines, fallthrough and the throwing fallback. Preserve the established static completion rule for value cases, including with a partial member selection.
3. Generate the header and exercise both directions through real linked stubs. Round-trip every code 0–7. On a sparse test enum reject an interior hole as well as an out-of-domain upper byte. Reject invalid inbound arguments before the body runs and invalid provider results before delivery. Preserve protected callback completion and checked acquisition failure with the correct cleanup count. Preserve arbitrary ordinary byte feedback on interfaces still declared `u8`.
4. Run actual CLI success, rejected source and source-open failure and verify their distinct ordinary outcomes and diagnostics. Stub-only fixtures establish the other codes without fabricating a production failure. Builds and unsupported implementation paths follow the existing [bootstrap acceptance policy](../applications/cod-booter/README.md#initial-seed-delivery). The generated membership check is small. The larger implementation work is preserving nominal types through selection, stored signatures and callback feedback in both compilers, and implementing the admitted case control flow. Check a sound subset explicitly while coverage grows. A numeric switch that skips guard effects or treats fallthrough as an unconditional body jump cannot implement those arms. Full diagnostics, general reflection and a portable binary ABI are separate work.
