# Building and rendering UTF-8 text with string.cod

## Design authority

[D-07-STRING-BUILD](DesignAuthorityRegistry.md#d-07-string-build), revision 1. The project user requested one lexical working type, common formatting for immutable strings and UTF-8 streams, a scoped output for deep helpers, and user-defined display and diagnostic representations. They delegated library and intrinsic design and asked mainline to validate it in the booter. [H-STRING-BUILD-20261121](../CHANGELOG.md#h-string-build-20261121) identifies that prototype authority. The API choices below are the design review’s decisions within it. P-678 derives this scope from C-36, C-55 and C-80. P-679 identifies its borrowing, text, module-world and scoped-output dependencies. Existing [UTF-8, CString and field semantics](Utf8Strings.md) govern every output. Acceptance ends at the [booter workload](#booter-prototype). Public API stability, terminal encoding for arbitrary bytes, persistence formats, and a permanent intrinsic or C ABI have separate scope.

The bundled module exposes one working type, `StringBuilder`. It builds a valid UTF-8 sequence in a full lexical scope. An immutable result belongs to the owner chosen for construction. A streaming consumer can write each piece while the producer is live. Standard operations are brought into scope through ordinary module selection.

Julia's `showerror(io, e)` can render directly to a stream, and `sprint(showerror, e)` uses that same formatter to produce a string. Its scoped values independently illustrate how an installation reaches nested calls. In CoD the destination can be an explicitly installed, owner-checked symbol. [Julia I/O](https://docs.julialang.org/en/v1/base/io-network/#Base.sprint) and [Julia scoped values](https://docs.julialang.org/en/v1/base/scopedvalues/).

Haskell's `bytestring` illustrates composition with a UTF-8 CSV encoder. The same composed `Builder` can feed `toLazyByteString` for collection or `hPutBuilder` to write directly to a handle. In CoD the intermediate has a lexical lifetime and the materialized result has an explicit owner. An implementation can write a builder's segments directly to a stream. [Data.ByteString.Builder](https://hackage-content.haskell.org/package/bytestring-0.12.2.0/docs/Data-ByteString-Builder.html).

Python lets a type supply a user-facing `__str__` and a diagnostic `__repr__`. Its data model suggests a reconstructible expression for the latter where practical and uses `__repr__` for informal display when only that hook exists. CoD adopts the distinction in purpose. Each selected CoD representation is an ordinary method with a visible owner, effect and failure contract. [Python data model](https://docs.python.org/3/reference/datamodel.html#object.__repr__).

## Building a piece of text

`left ++ right` returns a builder in the full lexical scope of the operator use. The standard family accepts two builders and supplies the String/String, String/builder and builder/String combinations. A `CString` can widen to `String`. Chaining associates left at additive precedence. Evaluate each operand once in source order. Each result denotes the operands' byte sequences at concatenation time. Later appends to either input or to the result leave the others unchanged. This also defines `b ++ b`. Use `append(b, piece)` to extend a particular builder. Appending a builder to itself appends its entry contents once.

The working owner comes from the use site; a lowering can pass its scratch destination into the operator. For a selected standard method, the result has the owner it would have in an explicit `make StringBuilder(left, right)` at that use site. An ordinary method returning a reference into its own activation would fail the lifetime check. An optimization that happens to allocate a return slot in the caller cannot by itself establish this source-level owner. A builder preserves its accumulated bytes if a shorter-lived input expires. Retain an immutable segment only with proof that its owner covers every use; otherwise materialize the contribution in the builder's own workspace. Segment layout and allocation are library choices. A sequence of appends can grow incrementally, and a chain of concatenations need not flatten every prefix. Ordinary owner checks prevent escaping the workspace. Neither a region field nor a native string argument accepts the working builder.

`make String(piece) in D` is the builder overload of the existing owner-directed text construction. It consumes the current *contents* and returns a reference to complete immutable text in D. The builder remains usable. An empty builder produces the empty string. If the result is destined for a direct field in D, construct first and install with `entry.name = ready`. Earlier borrowers keep their selected text. A separately written `entry.name <- input` still selects a visible transfer method. Selecting the building operations does not install a general transfer method.

## One formatter and two destinations

Use a public output protocol `Utf8StreamOut`. `StringBuilder` is a writable in-memory implementation. A file or socket writer implements the same protocol under its own resource lifetime. Expose one module-level borrowed symbol using the existing [owner-parameter reference-symbol contract](SymbolicDynamicScope.md#owner-parameter-reference-symbols-and-implementation-cost).

```cod
# string.cod
type Utf8StreamOut;
export Utf8StreamOut;
ref symbol @io:Utf8StreamOut in O where {owner O};
export @io;
```

`writeUtf8(out, piece)` writes a String or the contents of a builder to an explicit destination. `emit(piece)` is an ordinary proc that calls `writeUtf8(@io, piece)`. Selection of the writer uses the installed target's actual type under ordinary [method selection](ModuleWorlds.md#actual-type-dispatch-and-concrete-type-evidence). A direct proc several calls below the installation sees the same symbol identity. Every reachable lookup needs an established binding. Different installations can borrow different live actual owners. A nested installation shadows the outer one and restores its target and owner on every exit. Installing the reference does not acquire or close the stream. A func can compose builders or append to an explicit builder. Looking up or installing `@io` requires a proc context.

For example, a compiler can render an object manifest to a file or use identical bytes as an in-memory test fixture. Here `ObjectList` and the `objects` generator belong to the application. `Utf8WriteException` is the prototype library’s checked output failure.

```cod
proc emit_object(dir:const String in D, file:const String in F)
    throws Utf8WriteException where {const owner D, const owner F} {
    emit(dir ++ "/" ++ file ++ "\n");
}

proc render_manifest(list:const ObjectList in L)
    throws Utf8WriteException where {const owner L} {
    emit("objects\n");
    for ref item = objects(list) { emit_object(item.dir, item.file); }
}
```

The two calling contexts below use the same imported `@io`. Each handles or declares the formatter's checked failure. In the first, D is the allocation-capable owner chosen for the final String. In the second, `writer` is an existing writable output.

```cod
ref b = make StringBuilder();
{{
    effect @io = b;
    render_manifest(list);
}};
ref text = make String(b) in D;

{{
    effect @io = writer;
    render_manifest(list);
}};
```

Each call to `emit_object` has its own lexical workspace. A synchronous stream writer consumes a line before that invocation ends. With a bounded backend, streaming needs space for the current piece and the I/O buffer; the manifest can exceed the available memory for a single String. The collecting route retains the manifest bytes by design. An ordinary for body shares its enclosing full scope. Moving a long-running loop's temporary concatenations into a per-record helper or an explicit full scope gives those temporaries a per-record lifetime.

`buildString(entry.name) { render_manifest(list); };` is a candidate convenience call outside the initial prototype. Such a helper would be a **proc**: it creates a private lexical builder, installs it as `@io`, invokes the synchronous body once, then on successful completion constructs and installs the result in the destination's owner. A replaceable direct field requires a place-taking formal with a first-write contract. A bare `init tgt:String in D` initializes a selected target. Use the two primitive operations in the prototype. The public convenience signature can follow real field uses. A library offering the helper must honor nonlocal exits and prove its promised write on every normal result path.

## User-defined display and diagnostic representations

`str(out, value)` writes text intended for a person. `repr(out, value)` writes text a developer can use to distinguish values, including information that display may omit. Both take the destination first, finish synchronously and return void. The two outputs can be entirely different. For user-defined types, a module imports and extends the relevant callable families with ordinary methods. If both representations should be identical, one method can call the other explicitly. Missing or ambiguous interpretations use the existing checking and [module dispatch rules](ModuleWorlds.md#actual-type-dispatch-and-concrete-type-evidence). A declared checked write failure is preserved. The compiler never tries the other representation after a method throws.

When their standard collecting overloads are selected, the one-argument library operations `str(x)` and `repr(x)` yield a reference to a complete immutable `String` in the full lexical scope of the call. They evaluate x once, render through the selected family's two-argument method into a fresh private lexical builder, and finish into the caller's scope. This is the same use-site destination mechanism used for library concatenation. The two-argument method is selected in that family’s implementation context using ordinary overload resolution. A source-preserving import cannot see an unrelated module’s private formatter. The helper can use shorter-lived scratch before publication. Later mutation of x cannot change the completed text. The unary forms call a proc renderer and retain its proc permissions and declared failures. A generic call with a builder as destination still checks the declared writer contract. Collection does not install or require an ambient output for the explicit method.

Use `repr(@io, x)` to stream directly to the current output. A method calling other explicit formatters passes `out` onward. A method reusing helpers that call `emit` first installs `effect @io = out` in a nested full scope. All of its formatted bytes must reach the supplied destination even if a different output was already installed. A reference to an owner outside the method needs the ordinary proofs; a writer cannot retain a borrowed argument by formatting it. `emit` accepts text or a builder. Formatting arbitrary data remains an explicit call. Neither a `:String` assertion nor a `(<-)` transfer implicitly selects a formatter.

For example, the booter already has a [SourcePosition](../applications/cod-booter/src/SourceStorage.cod) with a line and a byte column. The actual location calculation uses one-based coordinates and leaves zeroes for an unavailable position. An application-side adapter can extend both families without adding a method to the seed for this type.

```cod
import "string.cod" { str, repr };
using "string.cod" { Utf8StreamOut, Utf8WriteException, writeUtf8 };
using "SourceStorage.cod" SourcePosition;

proc str(out:Utf8StreamOut in O, p:const SourcePosition in P)
    throws Utf8WriteException where {owner O, const owner P} {
    if (p.line == 0u64 && p.column == 0u64) {
        writeUtf8(out, "<unknown>");
        return;
    }
    str(out, p.line);
    writeUtf8(out, ":");
    str(out, p.column);
}

proc repr(out:Utf8StreamOut in O, p:const SourcePosition in P)
    throws Utf8WriteException where {owner O, const owner P} {
    writeUtf8(out, "SourcePosition(line=");
    str(out, p.line);
    writeUtf8(out, ", column=");
    str(out, p.column);
    writeUtf8(out, ")");
}
```

The `12:7` display and `SourcePosition(line=12, column=7)` representation describe the same input. A sum can select a spelling for each constructor. A graph type can print a domain identifier or bounded summary. The core provides no field walker, automatic private-field exposure or address rendering. Custom `repr` has no parse, identity reconstruction or stability guarantee. Recursive traversal and its termination belong to the method that elects to follow edges.

Standard text display preserves its contents, including NUL. Standard text representation surrounds the contents with double quotes and escapes double quotes, backslashes, LF, CR, TAB and U+0000 using the literal spellings. Use `\u{HEX}` with minimal uppercase hexadecimal digits for the remaining C0 and U+007F–U+009F controls. Other Unicode scalar values retain their UTF-8 encoding. The resulting quoted String representation can re-enter the CoD literal grammar. For the booter’s u64 coordinates, display uses locale-independent ASCII decimal, including zero and the full u64 range. Fraction methods, when that type is implemented, must preserve the stored numerator and denominator and the [approved fraction output](FixedWidthFractions.md); this prototype does not choose floating-point, fraction suffix or unit-formatting policy.

## Library and intrinsic boundary

Keep the two formatter families, user-type methods, decimal digits, quoting, output selection, segment composition and partial-write handling in ordinary library or provider code wherever their existing operations suffice. Resolve imported builtins and ordinary callables by identity. A local callable named `str`, a method named `repr`, or a renamed import receives exactly its normal module meaning. No frontend keyword, per-ADT hook, universal textual conversion, locale formatter or type-name branch is needed. Re-export the same callable identity when a consumer needs the extended family. Verify that a source-preserving view and a receiver-adapted view retain their ordinary differences.

The compiler/runtime boundary has three responsibilities that source spelling alone cannot prove:

| Operation | Required check or support |
| --- | --- |
| Access validated text or pass it to a writer | Supply a bounded synchronous readonly loan with the true owner and UTF-8 byte length. Preserve interior zeroes. Only the established C string boundary prepares a terminator. The native write primitive must report progress or failure. |
| Create immutable text in a specified owner | Check allocation permission, size arithmetic and lifetime, establish complete immutable backing before publishing the result, and use a relocatable representation in a region. Reuse existing String construction for validated input. |
| Produce a standard result in the use-site owner | For `++`, `str(x)` and `repr(x)`, let the selected library adapter receive checked storage in the full lexical scope of the use. A compiler may specialize or inline it. Preserve operand evaluation, applicable method identity, failures, target lifetime and cleanup. The mechanism belongs to the resolved standard adapter, never to a name token. |

A new intrinsic is warranted only for an operation that genuinely needs compiler-owned evidence or native access. Use one checked semantic identity and one shared definition across seed and booter for each operation that needs catalog publication. Reuse the [catalog service](../applications/cod-booter/docs/LLVMInterop.md#builtin-catalog-and-rejection-reporting) when applicable. The library may choose chunks or contiguous scratch and eliminate intermediate materialization when the observable owners, bytes, effects and failure behavior permit it. Byte layout, buffering size and permanent ABI are implementation decisions.

## Failure, cleanup and acceptance

A completed write contributes the supplied bytes in order, including U+0000. On successful return the writer has consumed the bytes or established independently live buffering. A synchronous backend may retry a short write from the first unwritten byte. A nonempty write that cannot make progress follows the backend’s wait-or-fail contract. An unrecoverable failure propagates under the writer's contract. Bytes already accepted by an external stream remain observable and the last accepted byte can fall inside a multibyte character. A checked failure declared by the generic formatter remains a caller obligation even when the installed writer happens to be a builder. Flush, close and publication of a file follow the external owner's contract. Installing a borrowed output acquires none of those responsibilities. Any adapter queuing bytes beyond a call must establish its own sufficiently long-lived storage.

For collection, finish before installing the field. If formatting or final construction throws, that final installation has not occurred. The former field and its borrowers remain usable unless the user's code independently changed them. Ordinary cleanup still runs for scratch resources, and prior initializer or output effects keep their semantics.

Verify the same manifest rendered both ways. Compare bytes for empty input, non-ASCII text and an embedded NUL. Reuse a builder after finishing and an operand after concatenation, then end the source scope and verify the finished text under D. Redirect a nested formatter to an inner builder, inject a write failure, catch it outside the installation and emit again to the restored outer target. Exercise short writes and failure following a successful prefix. Stream many records with a bounded writer and per-record scratch, without constructing the complete manifest. A formatter reached with no live binding, an escaping reference to a dead workspace, and an attempt to access the symbol from a func must reject.

## Booter prototype

1. Exercise this on a rejected input entering the real [rejection reporting](../applications/cod-booter/src/NativeDiagnostics.cod) path. Use the coordinates it actually resolves in [SourceAccess.cod](../applications/cod-booter/src/SourceAccess.cod). A prototype adapter may format the recorded numeric facts after the source loan closes. Keep the production [launcher diagnostic record](../applications/cod-booter/docs/LLVMInterop.md#builtin-catalog-and-rejection-reporting), its stage/code, status, related position and established CLI output. Prototype renderings may be observed separately. A fabricated position alone cannot establish this integration.
2. Render one available position, an absent position and a UTF-8 string with a quote, LF and NUL through both an in-memory builder and a bounded writer. Compare the bytes for each mode. Exercise the unary forms against explicit rendering, extend a family in another module, use a renamed family, and verify a missing or ambiguous method does not silently fall back. An output failure after a prefix must propagate once without retrying the entire formatter, and nested output must restore its predecessor.
3. Demonstrate the ordinary CoD library and adapter declarations in the bootstrapping paths. Verify real behavior on the supported path and separately report parser/checker reach, generated code and actual runtime evidence for each compiler. Preserve the [compiler diagnostics boundary](../applications/cod-booter/README.md#diagnostics-by-compiler-stage). A semantic primitive used by both frontends needs one source of truth. Keep host code confined to the services it supplies, and remove parallel prototype logic when the library path replaces it. Bring a genuine owner, method or cost conflict back to design before accepting weaker semantics.
