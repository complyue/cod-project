# UTF-8 strings and C string arguments

Design revision 5 · 2026-11-23 · Qualified for UTF-8 text, owner-bound ADT fields and the first synchronous native argument profile.

## Design authority

[D-07-UTF8](DesignAuthorityRegistry.md#d-07-utf8) covers the source and call rules below. The [project user](../CHANGELOG.md#h-utf8-native-text-20261121) requested built-in UTF-8 strings and C interoperability, selected direct compatible literals and explicit preparation for runtime text, and proposed `path:CString`. P-674/P-675 apply C-80 and charter §§4.3/6.3/8.1/12. A source-level filename passes through CoD to C with its spelling and actual owner intact. The [ordinary nonnumeric assertion](SymbolicDynamicScope.md#expression-type-conversion-and-assertion) supplies its failure policy. Acceptance in both compilers covers real object output, interior NUL, invalid native input and cleanup. Physical text layout, general collection APIs and non-UTF-8 locale integration have separate scope.

[D-07-STRING-FIELDS](DesignAuthorityRegistry.md#d-07-string-fields) covers the r2 baseline for [field storage and transfer policy](#storage-and-transfer-policies). [H-STRING-TRANSFER-POLICY-20261121](../CHANGELOG.md#h-string-transfer-policy-20261121) records the user’s direction and their choice to require a visible method. P-676/P-677 apply C-55, D-22-INIT-PLACES, D-22-TRANSFER and D-28-ADT. Test a field installation, replacement while a prior CString is live, cross-owner materialization, and rejection when no applicable method exists. Libraries choose their allocation strategies. The r3 prototype applies the user’s proposed `val var` to a direct text place and `init val var` to a promised first write under H-STRING-BUILD-20261121. The qualified r2 place and lifetime requirements continue.

[D-07-STRING-BUILD](DesignAuthorityRegistry.md#d-07-string-build) records the user's authorization of the [string.cod prototype](StringBuilding.md) and its booter workload under [H-STRING-BUILD-20261121](../CHANGELOG.md#h-string-build-20261121). The library owns rendering policy. The UTF-8 and field contracts here supply its text and destination guarantees.

[K-TEXT-PROC-RETURN](../CHANGELOG.md#k-text-proc-return) continues r3 for explicit successful completion in the field-transfer method examples.

At r5 the [newline and long-text decision](../CHANGELOG.md#h-newline-and-long-text-20261123) extends D-07-UTF8/P-674–P-675 to [multiline literals](#multiline-string-literals). The same UTF-8, byte-preservation, owner and NUL contracts apply. Verify the bytes passed to a bounded native writer and the indexed position following a multiline token.

## Source text

The builtin `String` denotes immutable, length-delimited UTF-8 text. Its byte length includes every encoded scalar, including U+0000. The empty string has byte length zero. Text preserves the source sequence, including canonically equivalent spellings with different byte sequences. Runtime text need not undergo Unicode normalization. UTF-8 validity follows [RFC 3629, §§3–4](https://www.rfc-editor.org/rfc/rfc3629#section-3).

A quoted runtime expression can supply a string wherever its backing lives long enough. Literal backing can live in the read-only image. Runtime borrows retain the actual owner of their backing. A readonly parameter alone cannot rule out mutation through another reference or native code. Validate external bytes once only when they remain stable throughout the loan. Otherwise establish immutable backing in a suitable owner before publishing the string. An address and a length do not make the text a transferable bits value or a relocatable regional field.

The ordinary double-quoted literal grammar accepts UTF-8 source characters, escaped quotes and backslashes, `\n`, `\r`, `\t`, `\0` and a Unicode scalar escape such as `\u{1F680}`. Malformed sequences, unknown escapes, unescaped newlines, surrogate escapes and values above U+10FFFF reject at the literal. Keep the module-path escape grammar and the exact foreign linkage marker `"C"` in their respective grammar positions. Inside a runtime literal `#` and `//` are text. The indexed source revision and its location table continue to use original byte offsets.

Use `String` and `CString` as builtin type names in the ordinary type-name grammar. `CString` refines `String` with the property that its content contains no U+0000. Its encoded byte length excludes any native terminator. A `CString` can be passed onward as `CString` or widened to `String` under its existing owner. A binding declared as `String` uses the explicit assertion when a `CString` is required. A NUL-free literal can also directly initialize that argument. This contextual literal rule does not infer a narrower declared variable type or select overloads by scanning a runtime argument.

### Multiline string literals

Three consecutive double quotes open a multiline `String` literal in a runtime expression. The first unescaped sequence of three double quotes closes it. Opening and closing delimiters may occur anywhere an ordinary text expression can occur. Six consecutive quotes give the empty string. One or two unescaped quotes in the content are ordinary characters unless they begin the closing delimiter. To include a run of three quotes or content quotes immediately before the closing delimiter, use the existing `\"` escape for each such content quote.

```cod
val text <- """first
  second"""
```

With an LF in the source this yields the same bytes as `"first\n  second"`. Preserve physical LF or CRLF as written, including a line break immediately after the opening delimiter or before the closing delimiter. Spaces and tabs are content even on the closing delimiter’s line. The ordinary text escapes and UTF-8 validation apply. A backslash followed by a physical newline is an invalid escape. Within the literal, `#`, `//`, braces and semicolons are text. An unterminated delimiter or malformed escape gives a located error in the original source revision.

The value uses the same immutable backing, byte length, owner and `CString` admission rules as ordinary text. The module path and foreign linkage productions keep their own quoted token forms. Either production may span whitespace between its tokens. A formatter may preserve a multiline literal verbatim or use an equivalent escaped spelling that yields identical bytes.

## Storage and transfer policies

When text participates in `destination <- source`, retain the ordinary `(<-)` overload family in the calling module world, including a method specialized for the destination’s allocation policy. This applies to text operands and to containing records. A compiler shortcut for `String` must preserve the method selected by ordinary dispatch, including its effects and failures. Allocation, interning, reuse and handling of overlapping transfers belong to the selected method. Access to the actual owners and permission to allocate follow the [graph-transfer contract](RegionalOwnership.md#root-policy-dispatch-and-actual-owner-arguments).

For example, a manifest library can intern filenames in its destination region. Here `FilenamePool` describes the library’s required region roots and `intern_filename` is its own procedure. It returns a reference to immutable text in the supplied region.

```cod
data ManifestEntry { ref filename:const String; }

proc (<-)(dst:ManifestEntry in D, src:const String in S) -> void
    where {region D:FilenamePool, const owner S} {
    dst.filename = intern_filename(D, src);
    return;
}
```

The method may reuse an existing destination-owned string when the pool finds a match. The stored edge must belong to the same actual owner as `dst`; an independently owned input may then expire. Rebinding a writable field preserves earlier live borrows of the selected immutable text, including its `CString` refinement and an active C loan. An overload taking only a reference to an existing string has access to the selected text. Replacing a field binding requires access to the field’s place. The ordinary result contract permits a void method as above or a const view of the original destination. The layout and addressing rules cover [all four lexical and regional owner combinations](RegionalAndLexicalStorageRefactor.md#graph-transfer-with-growing-source-or-destination).

### Direct String fields

A nominal product may declare `name:String`, and a nominal sum may declare it in a constructor payload. Each active field is a required, non-null place for immutable text owned by the same actual owner as the containing instance. This rule applies when the ADT is allocated in a lexical scope or a region. Such a type is non-bits, including when a different alternative happens to be active. In a region, the published text and its backing must survive relocation with the region. A native address into a literal image, another region or a lexical invocation cannot serve as a stored regional edge. A lexical representation may differ while preserving owner and target identity.

```cod
data Entry { name:String; }
data Finding = Named { name:String; } | Missing {};

# An ordinary library method for installing text already owned by D.
proc (<-)(init val var out:String in D, text:const String in D)
    -> void where {const owner D} {
    out = text;
    return;
}
```

A read of `entry.name` selects its current immutable text. A compatible `ref` binding or successful `:CString` assertion retains that selected instance. Selecting the field as a destination can instead supply the writable field place to a method that explicitly declares it. The direct-place formal is `val var out:String`. It names the caller’s selected text place, even when the implementation represents that place using an offset. An optional `in D` names the owner of the place. Use `init val var` for a method that also accepts a first initialization: it must establish the text before reading it through that place and on every successful completion. The same method may replace an existing selection. A plain `val var` method may inspect already established text. For an explicitly declared ref field the reference-slot form remains `ref var` with the matching access qualifier. The two `in D` operands require the same actual owner. The method returns `void`, so the transfer cannot be chained for a value. An ordinary target formal receives only the selected text; it cannot rebind the field. A readonly containing-object access cannot supply the writable place. The `out = text` in this method performs the direct field’s explicit same-owner installation. It does not recursively invoke the enclosing `(<-)` method. A direct place is writable storage owned by D. Its `val var` spelling does not admit a standalone `val s:String`, a raw descriptor transfer or rebinding a differently declared ref/ptr slot through it. This text-specific use of a direct place preserves the [declared place and initialization rules](ConstructionAndAcquisition.md#init-destinations-and-declared-place-aliasing).

```cod
# D owns both prior and next. The method above is visible.
ref entry = make Entry in D { name <- prior; };
ref old:const CString in D = entry.name:CString;
entry.name <- next;
# old still denotes prior, even if next contains an interior NUL.
```

`entry.name <- input` requires a visible, applicable `(<-)` method both in the construction initializer and on a later write. Use the containing module world's ordinary selection rules at the operation site. No applicable method gives a compile-time error. Equally good candidates give the ordinary ambiguity error. The compiler cannot silently allocate text, bind a same-owner source, or fall back after the selected policy throws. A library can explicitly provide a general method through ordinary module selection. Importing a policy once suffices for its eligible uses. A method returning a view still owes the original evaluated destination under the [ordinary result rule](RegionalOwnership.md#transfer-results-and-chaining); an unrelated newly allocated string cannot stand in for that result.

A policy that already has a suitable destination-owned string can write the binding with `=`. A policy that needs independent text can explicitly use owner-directed construction, `make String(input) in D`, under a live allocation-capable owner D. In a declaration the input may be described by `input:const String in S`. The construction call is `make String(input) in D`; the argument establishes S independently of D. Construction preserves the full UTF-8 sequence, including an interior NUL, and leaves the source usable. The constructed text can outlive a different source owner. A pool can avoid construction on a hit and return its existing destination-owned text. A writable binding alone grants no allocation through a readonly owner: the same-owner method above needs only `const owner D`. A writable containing object or a separately available allocating owner can supply the corresponding authority under the [ordinary owner rules](RegionalOwnership.md#owner-constness-and-allocation). Regional root-specific policies receive the actual instance; lexical destinations use their own owner.

For example, a general library can select the following policy for a writable entry in either a lexical scope or a region. The containing entry supplies allocation authority in D.

```cod
proc (<-)(dst:Entry in D, input:const String in S) -> void
    where {owner D, const owner S} {
    dst.name = make String(input) in D;
    return;
}
```

This method handles `entry <- input`. A call written `entry.name <- input` needs a method for the field operands. Defining one signature never silently enables the other. In a constructor, the enclosing instance need not be readable to initialize its field: the actual field owner is known from placement before it has a target. Only the active alternative's required fields must be initialized.

Select the destination place once. Reading the RHS before a binding write may select the old text, including on self-transfer. Its owner must stay live throughout each use. Until a policy writes a replacement, an initialized field retains its old binding. If a method writes and then throws, the completed write and other effects follow ordinary exception semantics. A newly constructed ADT cannot be published as ready with an uninitialized required field. Pools may reuse backing only while preserving every live text selection and synchronous C loan. No per-string release action or allocation rollback follows from a field update.

**Acceptance.** Exercise the same product in lexical and regional storage and an active sum field. Check first initialization, subsequent replacement, a same-owner pool hit, a cross-owner transfer whose source ends, and a region reopened or relocated with its text intact. Keep an earlier `CString` selected while replacing the field with text containing NUL and observe both values. Reject a missing policy on first and later transfer, an ambiguous pair, cross-owner `=`, a readonly destination, allocation without authority, and a policy returning normally without the promised init write. Test failure both before and after an allowed field write. The ordinary [storage and transfer rules](ConstructionAndAcquisition.md#storage-eligibility-and-payload-transfer) govern transfers of the enclosing ADT and C-layout records keep their recursively bits-only field rule.

## The string building library

The bundled [string.cod design](StringBuilding.md) covers lexical builders, user-defined display and diagnostic representations, scoped UTF-8 streams, owner-directed completion, and the booter prototype. Its prototype authority and physical implementation status are recorded separately from the builtin text type.

## Borrowing text for a C call

Consider the [LLVM output provider](../applications/cod-booter/native/llvm_provider.c). The launcher supplies the output filename as validated, bounded `String` under its live activation. The CoD driver forwards that text to the object writer. The writer requests a `CString` for the synchronous native publication operation. The input filename may continue to use its `Path` handle and the `SourceRevision` constructor. The indexed-source loan and line table belong to the actual revision.

```cod
# NativeLLVM.cod
proc "C" llvm_emit_object(handle session:LLVMSession in L, output:const CString in P)
    -> u8 where {owner L, const owner P};

proc emit_default(handle session:LLVMSession in L) -> u8 where {owner L} {
    return llvm_emit_object(session, "构建/结果.o");
}
proc emit_to(handle session:LLVMSession in L, path:const String in P)
    -> u8 where {owner L, const owner P} {
    return llvm_emit_object(session, path:CString);
}
```

Here `path` has declared type `String`. Supplying it directly to the `CString` formal rejects. The expression `path:CString` evaluates the operand once and tests the selected text. A statically false assertion rejects. Known success needs no scan. Otherwise a failed check raises the existing unchecked `TypeAssertionFailedError` during argument preparation, before later arguments or provider entry. Earlier effects and required cleanup still run. The assertion preserves target, access and owner. Reusing its result as `CString` needs no further membership check. An interior zero in a `String` is ordinary text. Thus `"report\0.old"` may be used as `String`, rejects in a literal `CString` position, and, when received as unknown runtime text, fails the explicit assertion before C can open the file named `report`.

The generated adapter must supply the provider with a non-null `const char *` and an accessible trailing zero for the entire synchronous call. Reuse proven stable NUL-terminated backing when possible. Otherwise prepare terminated storage whose lifetime covers the native use. Release temporary storage on normal and exceptional paths, including failure preparing a later argument. Empty text supplies an accessible terminator. Terminator preparation belongs to the call. Evaluating `path:CString` alone need not allocate native storage. The source owner and the duration of the native pointer loan are independent checks. The provider cannot keep that pointer after returning. Ordinary status, failure before entry and protected callback completion retain their separate channels.

From C, a length-delimited input needs a readable extent, an actual owner, stable bytes and validation before CoD can observe it as text. For a terminated C input with an explicit extent, search only within that extent and require a terminator. A documented host C-string contract, such as a live launcher argument, can establish a valid terminated range before constructing the bounded input. A raw pointer with neither an extent nor a trustworthy terminated-string contract supplies insufficient evidence. Null cannot be interpreted as empty terminated text. When C supplies malformed bytes for a typed entry argument or omits a promised terminator, the generated first-host adapter reports `COD_NATIVE_BOUNDARY_ERROR` before CoD entry. This follows the [existing invalid native argument contract](TypedEnums.md#generated-c-exchange). A runtime `String` with an embedded NUL remains valid at a length-aware entry. Its later `:CString` assertion has the ordinary CoD failure described above. The native side remains responsible for truthful backing and lifetime evidence. Permanent C data layout and locale-dependent adaptation are outside this profile.

## Acceptance for the first implementation

- Both the C++ seed and the CoD-written booter parse and type a runtime literal in an ordinary expression. Exercise empty text, an internal zero, non-ASCII text, a supplementary scalar and invalid literal bytes or escapes. Module path and foreign linkage parsing keep their existing contracts.
- Feed an actual bounded runtime input through a CoD helper. Distinguish valid empty text, an interior zero, truncated and overlong sequences, and an encoded surrogate. Reject a missing terminator within a declared native extent. Show that an unasserted dynamic `String` rejects, a proved literal passes directly, and a previously refined argument forwards. Catch `TypeAssertionFailedError` for unknown runtime text containing NUL. Count provider entries and later argument evaluations on failure. Check malformed native text on its separate boundary failure path.
- Use the real object-output operation with a UTF-8 filename and verify the named file is produced. A failing name containing an interior zero leaves any prefix-named file untouched. Preserve output failure reporting and exactly-once cleanup. No input-file migration or native constructor signature change is required for this example.
- Verify that a C pointer loan cannot escape and that a later write through an independently writable backing cannot invalidate previously validated text. A first implementation may use independent immutable backing when it cannot prove a zero-copy borrow sound.

General text editing, grapheme indexing, collation, locale fallback and lasting foreign pointer retention are outside this first slice.
