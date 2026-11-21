# Typed enums

Design revision: 2 · 2026-11-21

## Design authority

[H-TYPED-ENUM-20261121](../CHANGELOG.md#h-typed-enum-20261121) covers the project user's proposal, explicit requirement for old and new member names, and delegation of the detailed review. [D-07-ENUM](DesignAuthorityRegistry.md#d-07-enum), P-658/P-659, qualifies the closed nominal type, member identities, selection and coverage. [D-20-C-ENUM](DesignAuthorityRegistry.md#d-20-c-enum), P-660/P-661, qualifies generated current-target by-value exchange for the first booter workload. Charter §§3.2, 6.3, 8.1 and 12 connect named domain outcomes to readable programs and checked native execution. [K-TYPED-ENUM-REVIEW-20261121](../CHANGELOG.md#k-typed-enum-review-20261121) records the delegated resolutions below. Acceptance exercises the same identities from declaration through case analysis and the C boundary. Shared foreign storage and permanent binary interfaces remain outside this scope.

[H-MODULE-SOURCE-STYLE-20261121](../CHANGELOG.md#h-module-source-style-20261121) continues r1 to r2 for the public declaration example. The [module contract](ModuleWorlds.md#public-definitions-and-grouped-selections) supplies its publication semantics.

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

An enum value has ordinary scalar `val` semantics and the size/alignment of its backing type on the current target. A stored slot, ref or ptr to an enum follows the usual owner, access and initialization rules. Enums can be bits fields and array elements in CoD lexical or regional storage. Equal backing bits alone grant no implicit conversion between integers or enums. Equality compares values of the same enum; both spellings of Rejected compare equal. No arithmetic, ordering, flags, indexing conversion, units or [fraction operations](FixedWidthFractions.md) follow from the backing type. Ordinary typed initialization must establish a declared member before a read. Zero-filled storage does not initialize a variable, and zero need not denote any member. A nominal sum with payloads retains its own allocation, constructors and tag matching.

## Names and case matching

```cod
using "./Compiler.cod" NativeError { Rejected, OpenFailed };
val result1 <- Complete:NativeError;
val result2 <- Rejected;

func exit_status(ne:NativeError) -> u8 {
    return case ne : NativeError of {
        (OldRejected) -> { 1u8 };
        (FormUnsupported) -> { 2u8 };
        () -> throw;
    };
}
```

Exporting the type publishes its complete member table. `using` and `import` may select individual spellings or `{..}` for all, following the existing [type selector grammar](DomainLanguageDesign.md#publishing-and-selecting-data-constructors). A selector is checked against that type, including old spellings. It introduces the selected names for ordinary expression lookup. Selecting the type alone still supplies its full definition for typed access and coverage. Unselected members do not become ordinary module names merely through publication of the type. Re-export preserves type and member identities. Multiple paths to the same definition retain one domain. Neither imports, adapters nor parallel module worlds may add codes or further member names to that definition. Ordinary method selection has its existing permissions.

In an expression `name:E`, first resolve the visible type E and use ordinary lookup for the operand. A lexically bound value takes precedence, even if asserting it as E then fails. Only when the single identifier has no ordinary binding and E identifies a concrete enum may the checker resolve it in E's member table. An ambiguous or inaccessible name stays an error. For example, a local `Complete:u8` makes `Complete:NativeError` an integer-to-enum assertion that is statically impossible. With no such binding, it denotes member Complete. A bare `Complete` without an ordinary binding still fails. An expected argument or return type does not search for enum members. General `expr : E` retains [ordinary expression typing](SymbolicDynamicScope.md#nonnumeric-assertion-contract); a numeric code needs explicit domain decoding by the application or a checked native boundary.

In a member-pattern position such as `(OldRejected)`, the static enum type of the subject supplies the member table. The token names a constant pattern; it introduces no variable and reads no shadowing local. A misspelling rejects. In a guard or arm body the same spelling uses ordinary expression lookup. A member has no variant payload to receive. `isa` remains a type test. [Case semantics](DomainLanguageDesign.md#case-expressions-and-branch-local-variables) govern subject evaluation, source-order effects, guards, fallthrough and result completion. A bare `() -> throw;` raises `UnexpectedCaseError` when reached.

Coverage counts distinct canonical members of the complete declaration, independently of the selected names. For `enum Decision:u8 { Yes; OldYes = Yes; No; }`, unguarded completing arms for OldYes and No cover the entire domain. Arms for Yes and OldYes leave No uncovered. A guard that can decline or a matching arm that always falls through supplies no independent proof of completion. Repeated spellings may still have different guards or lead to later observable work. A void case may be partial. Value cases follow the existing static completion/fallback rule. A nullable subject also needs the usual null handling. Coverage must respect possible changes to a borrowed subject during matching.

## Generated C exchange

Use a checked CoD signature to generate a current-target C typedef of the exact backing width and signedness when a matching C type exists. For `u8`, use `uint8_t`. Generate collision-free module/type-scoped integer constants for **every published spelling**, including two constants of value 1 in the example. Keep each nominal enum in checked callable signatures even when the C typedefs have the same underlying type. Avoid relying on the implementation-defined representation of a C enum. A missing exact mapping rejects that boundary declaration.

- Encode a valid CoD value using its declared code. When C supplies an ordinary argument, result or normal callback feedback, check exact membership before calling CoD with it or publishing it to a CoD destination. Sparse domains need membership, not a min/max test. Preserve the full backing range, including valid extreme values. Multiple names for one code decode to one value.
- An undeclared incoming code is a native boundary failure. The first generated profile reports `COD_NATIVE_BOUNDARY_ERROR` through its existing invalid-provider/entry path. An invalid C argument prevents entry and an invalid normal result leaves the receiving output unpublished. On the CoD side use the same failure/cleanup path as a malformed native provider response. The invalid byte cannot reach a case fallback or become a domain rejection status.
- Inspect completion **before** reading a normal result or feedback slot. A checked constructor failure and a protected here return or throw follow their own carriers and preserve the original failure/control transfer and cleanup. Their normal payload storage may be uninitialized. For a valid normal status, including a nonzero failure reported *as a value*, the native transport itself succeeds.
- The first native profile covers by-value arguments, ordinary results and the normal feedback/result forwarding required by the CLI. CoD-owned bits storage stays eligible under its ordinary rules. A writable foreign view, foreign pointer, record containing an enum, or file-backed input from another producer would require validation for all observable reads and writes. By-value admission alone cannot authorize such a crossing. Keep those shared-storage interfaces outside this slice, including zero-copy validation and permanent ABI/versioning.

## First workload and acceptance

The compiler status domain includes `Complete=0`, `Rejected=1`, `FormUnsupported=2`, `OpenFailed=3`, `LoweringFailed=4`, `FinishFailed=5`, `ScratchFailed=6` and `SourceOpenFailed=7`. The seven [Compiler.cod results](../applications/cod-booter/src/Compiler.cod) cover 0–6. [ModuleDriver.cod](../applications/cod-booter/src/ModuleDriver.cod) also defines 7, which [ObjectCompiler.cod](../applications/cod-booter/src/ObjectCompiler.cod) returns for a caught source-open exception. At the [CLI entry](../applications/cod-booter/src/CompilerCLI.cod) keep all eight CLI exit codes, diagnostic fields, and resource cleanup. Trace the type across the ordinary compile result and the normal here feedback carried by `source_use`, `scratch_use` and `catalog_use`. A small leaf declaration module with a publication-only re-export from Compiler is available if needed to avoid a dependency cycle. Raw LLVM operation statuses, OS errors, diagnostic subcodes, launcher-only exit codes and existing arbitrary-`u8` callbacks have separate domains.

Acceptance for the seed and seed-built booter includes the following observations.

1. Parse, publish and select the enum across modules. Preserve nominal signatures, lexical shadowing, sparse and signed codes where supported, checked numbering and name/identity aliases. Reject duplicate codes, overflow, cross-enum equality and invalid selections. A normal initialized regional bits field retains its typed value.
2. Demonstrate a complete case using either old or new spelling. Two spellings cannot stand in for a missing member. Exercise a guard that declines, fallthrough and the throwing fallback. Preserve the established static completion rule for value cases, including with a partial member selection.
3. Generate the header and exercise both directions through real linked stubs. Round-trip every code 0–7. On a sparse test enum reject an interior hole as well as an out-of-domain upper byte. Reject invalid inbound arguments before the body runs and invalid provider results before delivery. Preserve protected callback completion and checked acquisition failure with the correct cleanup count. Preserve arbitrary ordinary byte feedback on interfaces still declared `u8`.
4. Run actual CLI success, rejected source and source-open failure and verify their distinct ordinary outcomes and diagnostics. Stub-only fixtures establish the other codes without fabricating a production failure. Builds and unsupported implementation paths follow the existing [bootstrap acceptance policy](../applications/cod-booter/README.md#initial-seed-delivery). The generated membership check is small. The larger implementation work is preserving nominal types through selection, stored signatures and callback feedback in both compilers, and implementing the admitted case control flow. Check a sound subset explicitly while coverage grows. A numeric switch that skips guard effects or treats fallthrough as an unconditional body jump cannot implement those arms. Full diagnostics, general reflection and a portable binary ABI are separate work.
