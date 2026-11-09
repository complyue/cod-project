# Fixed-width fractions and representation-independent units

Discussion revision: 4 · Recorded: 2026-11-09 · Status: design draft; implementation deferred.

## Design authority

- **Registry / scope:** [D-16](DesignAuthorityRegistry.md#d-16), revision 4, all sections. The complete document and candidate value path remain `pending-human`; the scoped decisions below are recorded in H-254/H-255. H-256 separately prioritizes the comment reservation and representation-independent UoM prerequisites; fraction implementation itself remains deferred. [K-401](DesignAuthorityRegistry.md#k-401) records H-255/H-256 comment reservation and H-260/H-261 quantity/widening decisions; full fraction execution remains deferred. [K-402](DesignAuthorityRegistry.md#k-402) records the implemented quantity/widening subset and56 focused checks.
- **Value lineage:** C-61/P-221 separates numerical representation from C-49's unit identity; C-62/P-222 relates explicit exact-number semantics to C-10/P-14/C-01 and the charter's inspectable typed business intent. D-16/P-223 organizes that numerical contract. These are candidate derivations, not retrospectively confirmed charter paths.
- **Dependencies:** P-224 depends on D-11 for unit identity, conversion selection and typed arithmetic. P-227 depends on D-10 for bits access, function/effect boundaries and borrow validity. These dependencies confer no implementation authority. Fractions contain bits, not owned regional objects or pointers.
- **Human baseline:** H-254 accepts independently sized integer components, fraction type spelling, unreduced ordinary arithmetic, explicit same-type canonicalization, representation-independent UoM, fixed bits storage and future conflict review. H-255 reserves `//` for construction/output and `#` for line comments in the planned grammar. H-256 subsequently requests prompt implementation planning for reserving `//` and the representation-independent UoM refactor; it does not schedule fraction arithmetic/storage implementation.
- **Invariants / non-goals:** Nonzero denominator; numerical equality; exact representable results; no implicit reduction, floating fallback, hidden heap allocation, denominator UoM, owner transfer or host-struct ABI assumption. Arbitrary precision, promotion tables and a full grammar/ABI are outside this draft's selected scope.
- **Acceptance:** The design examples and future checks below define the intended observations. There is no native fraction implementation or validation claim. [K-397](DesignAuthorityRegistry.md#k-397) records the initial draft; [K-398](DesignAuthorityRegistry.md#k-398) records the H-256 priority prerequisites and main-chat handoff scope; it does not qualify the full document.

## Priority prerequisites and deferred fraction implementation

H-256 requests prompt follow-up in the main project chat for two prerequisites:

1. **Reserve `//` in CoD source and retain `#` line comments.** Update the CoD lexer and migrate affected CoD comments, embedded/generated CoD sources and language examples coherently. Reserving the token does not require implementing fractions now; unsupported construction must not silently discard the rest of a line as a comment. C++ and other host-language comment grammars are unaffected.
2. **Refactor UoM to be independent of numerical representation.** Keep unit identities/aliases and selected conversion semantics while making accepted input and produced output representations explicit in typing. The same unit must be usable with multiple supported representations. Conversion-path applicability must account for representation compatibility. Preserve argument sequencing, module-world isolation and diagnostics. Consult D-11 for the implementation-planning boundary.

These priorities supersede only the blanket deferral of those two prerequisites. The fraction type, constructor execution, arithmetic, canonicalization and formatting remain planned future work. Their open grammar/promotion/layout decisions are not silently authorized by prioritizing the prerequisites. Main-chat implementation planning must surface concrete design obstacles before selecting an incompatible policy.

## Purpose and type model

A fraction is an intrinsic numerical bits type. It represents an exact ratio using a numerator and a nonzero denominator, each with its own fixed-width integer type. Its representation is independent of any unit of measure. A denominator is a component role, not a UoM; multiplying fractions does not manufacture powers of a denominator unit.

In a type position, `i64/i64` and `i64/i32` specify the two component types. In an expression, `/` remains an arithmetic operator and `//` constructs a fraction. Nested fraction component types such as `i64/i32/i16` are excluded from the initial scope. The complete signed/unsigned type family and promotion rules remain to be specified before implementation.

An illustrative quantity annotation is `i64/i32 (m)`: the numeric representation is the integer pair and the unit identity is `m`. This is not a new unit identity for each storage width. Different units remain distinct even when their numerical representations match. Exact qualifier/annotation grammar must be coordinated with D-11 and D-10.

## Construction, comments and output

The planned constructor is `numerator // denominator`. It preserves the supplied integer components without reduction, subject to the selected input/result types and the nonzero-denominator requirement. Ordinary division is not silently redefined as construction. Operand promotion and constructor result inference must be specified together; this draft does not silently widen either component.

Standard fraction output uses the same `numerator // denominator` form and displays the stored, potentially unreduced pair. Display is not an implicit call to `canonical`. Whitespace, type-width suffixes and unit annotations for typed textual round-tripping remain open; the common separator is confirmed.

The line-comment marker is `#`; `//` is reserved for fraction construction. The [lexer](../cod/lang_syntax.hh) diagnoses `//` outside comments and quoted paths rather than silently discarding the rest of the line. [K-401](DesignAuthorityRegistry.md#k-401) records the coordinated CoD comment migration and focused native evidence. This reservation does not implement fraction construction or select its precedence.

Operator precedence, interactions with unary minus and rejection of unparenthesized repeated construction need a grammar review. Examples below use parentheses where that avoids selecting a precedence rule.

## Arithmetic and numerical validity

Ordinary fraction arithmetic does not automatically reduce fractions. For example, `(1 // 2) + (1 // 2)` may produce `4 // 4`; explicit canonicalization produces `1 // 1`. No GCD reduction is inserted to rescue an otherwise overflowing arithmetic operation.

Equality and ordering concern mathematical values: `1 // 2` equals `2 // 4`. Comparisons do not mutate or normalize the operands. Their implementation must not obtain an incorrect result from an overflowing cross-product. This mathematical requirement does not prescribe a particular comparison algorithm.

Fixed component widths bound the representable values and intermediate arithmetic. An operation may overflow even if a reduced answer would fit. The result must follow an explicit numeric failure contract, not silently wrap, allocate larger storage or convert to floating point. A zero denominator is invalid; NaN/infinity encodings are not introduced. Failure syntax and recovery integrate with the language's numeric contract and are not designed here.

Integers embed mathematically as denominator-one fractions. Rules for automatic promotion, result component widths, signed/unsigned combinations, rational/floating mixed arithmetic and integer/floating conversion must be settled as part of numeric typing. Widening a result alone does not widen earlier intermediate operations. More storage bits do not automatically imply preservation of all integer precision when converting to floating point.

## Explicit canonicalization

The semantic signature is `canonical(n::i64/i32) -> i64/i32`, generalized over the supported component types. This notation describes input/result representation, not a new rule for default borrowing or returning references to local storage. Full declarations must follow the eventual bits/const value-result contract.

Canonicalization returns an equivalent reduced fraction of the same component types; it does not mutate the input:

1. Divide both components by the positive greatest common divisor of their magnitudes.
2. Represent zero as `0 // 1`.
3. Prefer a positive denominator when both sign-adjusted components fit their declared types.
4. Otherwise retain the representable negative-denominator orientation.

This defines a deterministic reduced representation within the fixed component types. For `i64/i32`, `1 // -2147483648` cannot acquire a positive denominator without widening. Likewise, negating a minimum signed numerator can overflow. The algorithm must handle signed minimum magnitudes without overflowing an intermediate absolute value. Canonicalization does not require all types to share the same stored sign orientation; mathematical equality remains independent of representation.

Checked construction, numerator/denominator access, canonicalization and numeric conversions form the initial intrinsic-support requirements. Exact accessor names and overload signatures remain open. Intrinsic support does not imply that every arithmetic algorithm belongs in compiler code. Reciprocal, for example, can use component access and checked construction, swapping `i64/i32` to `i32/i64` when the numerator is nonzero. No separate intrinsic is required without a demonstrated need.

## Storage, access and regional integration

A fraction always occupies fixed-size bits storage and contains no pointer, owner, hidden allocation or cleanup action. It may be held in lexical bits storage or embedded in regional fields and arrays. Its creation and bits transfer require no special regional memory management. This does not grant copy/move semantics to a containing regional object or relax reference lifetime and access permissions.

`i64/i32` has 96 bits of component payload. It does not require a native 96-bit arithmetic instruction, and this payload count does not silently choose a packed ABI or padded host-struct layout. Component offsets, alignment, byte order and array representation need explicit layout design. The established default for multiple fields is SoA unless an access-pattern justification and evidence support another arrangement.

The nonzero denominator is a validity invariant. Future initialization and writable-access designs must account for it: an all-zero storage pattern is not a valid fraction, and component access must not accidentally make an invalid pair observable as a valid value. This is an open integration requirement, not a selected validation wrapper or new ownership mechanism.

## Representation-independent UoM and conversions

Unit identity, numerical representation and admissibility constraints are separate. Defining a UoM does not fix a scalar width/layout. A unit may constrain admissible numerical domains without selecting one representation. Fractions can carry any applicable UoM just as integers and floating-point values can.

Conversion methods specify accepted input types and output representation. Their source/destination unit identities and representation requirements jointly determine whether a conversion path is usable. The selected unit route must not pass through incompatible intermediate representations. Conversion methods preserve the existing function/effect restrictions, visibility and argument evaluation order.

Two conversions with identical input types but different outputs require a defined destination-selection mechanism; this draft does not introduce general return-type overload dispatch. A conversion may deliberately change representation or round according to its contract. It does not implicitly canonicalize merely because a fraction participates.

The prototype implements `unit m;` with quantity types such as `u32(m)` and `f64(m)` under H-260/H-261. K-402 records typed conversion routes, lossless primitive widening and preserved unit aliases/module isolation. This establishes the UoM prerequisite; it does not implement fractions or select fraction-specific promotion rules.

## Required review for later language work

H-254 requires later CoD feature design and implementation to evaluate conflicts with this fraction plan. Relevant surfaces include tokenization/comments and operators; intrinsic naming/dispatch; bits and type construction; numerical promotion, overflow, comparison and conversions; const/borrow/assignment; regional/array layouts; and formatting/serialization.

When a proposed feature conflicts with an accepted fraction requirement, treat the conflict as a design obstacle: identify the incompatible requirements and alternatives, bring them to the user, and leave the affected choice unimplemented until resolved. Unrelated qualified work may continue. This requirement neither schedules fractions nor authorizes filling this draft's open choices silently.

## Future acceptance and remaining decisions

No build or test execution is required for this draft. A separately authorized implementation should establish compact evidence for:

- Distinct `i64/i64` and `i64/i32` representations, independent unit identity and rejected incompatible units.
- `//` construction/output, `#` comments and diagnosed uses of the incompatible former comment syntax where detectable; final precedence and migration rules must be selected first.
- Unreduced arithmetic, numerical comparison of unequal stored pairs, explicit reduction, zero normalization and signed-minimum canonicalization.
- Invalid denominators, arithmetic/conversion overflow and the chosen promotion/failure rules without hidden reduction or floating fallback.
- Fixed bits creation/transfer in lexical and regional storage, field/array access and no extra allocation, owner or relocation obligation.
- Typed conversion paths, preservation of evaluation order and output that does not normalize or mutate its input.

Remaining design work is the complete constructor/operator grammar, intrinsic signatures, component-access validity, zero/default initialization, promotion/result-width and numeric-error policies, concrete layout, and typed formatting. Explicit canonicalization and fixed-width storage are constraints on these choices, not reasons to introduce arbitrary precision, a denominator UoM or a new memory-management system.
