# UTF-8 strings and C string arguments

Design revision 1 · 2026-11-21 · Qualified for UTF-8 text and the first synchronous native argument profile.

## Design authority

[D-07-UTF8](DesignAuthorityRegistry.md#d-07-utf8) covers the source and call rules below. The [project user](../CHANGELOG.md#h-utf8-native-text-20261121) requested built-in UTF-8 strings and C interoperability, selected direct compatible literals and explicit preparation for runtime text, and proposed `path:CString`. P-674/P-675 apply C-80 and charter §§4.3/6.3/8.1/12. A source-level filename passes through CoD to C with its spelling and actual owner intact. The [ordinary nonnumeric assertion](SymbolicDynamicScope.md#expression-type-conversion-and-assertion) supplies its failure policy. Acceptance in both compilers covers real object output, interior NUL, invalid native input and cleanup. Physical text layout, general collection APIs and non-UTF-8 locale integration have separate scope.

## Source text

The builtin `String` denotes immutable, length-delimited UTF-8 text. Its byte length includes every encoded scalar, including U+0000. The empty string has byte length zero. Text preserves the source sequence, including canonically equivalent spellings with different byte sequences. Runtime text need not undergo Unicode normalization. UTF-8 validity follows [RFC 3629, §§3–4](https://www.rfc-editor.org/rfc/rfc3629#section-3).

A quoted runtime expression can supply a string wherever its backing lives long enough. Literal backing can live in the read-only image. Runtime borrows retain the actual owner of their backing. A readonly parameter alone cannot rule out mutation through another reference or native code. Validate external bytes once only when they remain stable throughout the loan. Otherwise establish immutable backing in a suitable owner before publishing the string. An address and a length do not make the text a transferable bits value or a relocatable regional field.

The literal grammar accepts UTF-8 source characters, escaped quotes and backslashes, `\n`, `\r`, `\t`, `\0` and a Unicode scalar escape such as `\u{1F680}`. Malformed sequences, unknown escapes, unescaped newlines, surrogate escapes and values above U+10FFFF reject at the literal. Keep the module-path escape grammar and the exact foreign linkage marker `"C"` in their respective grammar positions. Inside a runtime literal `#` and `//` are text. The indexed source revision and its location table continue to use original byte offsets.

Use `String` and `CString` as builtin type names in the ordinary type-name grammar. `CString` refines `String` with the property that its content contains no U+0000. Its encoded byte length excludes any native terminator. A `CString` can be passed onward as `CString` or widened to `String` under its existing owner. A binding declared as `String` uses the explicit assertion when a `CString` is required. A NUL-free literal can also directly initialize that argument. This contextual literal rule does not infer a narrower declared variable type or select overloads by scanning a runtime argument.

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
