# Building and rendering UTF-8 text with string.cod

## Design authority

[D-07-STRING-BUILD](DesignAuthorityRegistry.md#d-07-string-build), revision 4. The project user requested one lexical working type, common formatting for immutable strings and UTF-8 streams, a scoped output for deep helpers, and user-defined display and diagnostic representations. They delegated library and intrinsic design and asked mainline to validate it in the booter. [H-STRING-BUILD-20261121](../CHANGELOG.md#h-string-build-20261121) identifies that prototype authority. The API choices below are the design review’s decisions within it. P-678 derives this scope from C-36, C-55 and C-80. P-679 identifies its borrowing, text, module-world and scoped-output dependencies. Existing [UTF-8, CString and field semantics](Utf8Strings.md) govern every output. Acceptance ends at the [booter workload](#booter-prototype). Public API stability, terminal encoding for arbitrary bytes, persistence formats, and a permanent intrinsic or C ABI have separate scope.

[H-STRING-SOUT-20261121](../CHANGELOG.md#h-string-sout-20261121) covers the user’s direction to omit the destination from public type formatters and name the scoped output `@sout`. The streaming and collecting family names below implement that direction under the delegated prototype scope. P-678/P-679 still apply. Verify direct output, nested collection without output leakage, and restoration after failure under the existing [reference-symbol contract](SymbolicDynamicScope.md#owner-parameter-reference-symbols-and-implementation-cost).

[H-UTF8-WRITE-ERROR-20261121](../CHANGELOG.md#h-utf8-write-error-20261121) selects the [unchecked output failure](#failure-cleanup-and-acceptance) for the delegated prototype under the existing [exception roots and handling obligations](ExceptionControl.md#declared-handling-obligations). Verify optional typed recovery, propagation through an unannotated formatter, partial progress and restoration.

The bundled module exposes one working type, `StringBuilder`. It builds a valid UTF-8 sequence in a full lexical scope. An immutable result belongs to the owner chosen for construction. A streaming consumer can write each piece while the producer is live. Standard operations are brought into scope through ordinary module selection.

Julia's `showerror(io, e)` can render directly to a stream, and `sprint(showerror, e)` uses that same formatter to produce a string. Its scoped values independently illustrate how an installation reaches nested calls. In CoD the destination can be an explicitly installed, owner-checked symbol. [Julia I/O](https://docs.julialang.org/en/v1/base/io-network/#Base.sprint) and [Julia scoped values](https://docs.julialang.org/en/v1/base/scopedvalues/).

Haskell's `bytestring` illustrates composition with a UTF-8 CSV encoder. The same composed `Builder` can feed `toLazyByteString` for collection or `hPutBuilder` to write directly to a handle. In CoD the intermediate has a lexical lifetime and the materialized result has an explicit owner. An implementation can write a builder's segments directly to a stream. [Data.ByteString.Builder](https://hackage-content.haskell.org/package/bytestring-0.12.2.0/docs/Data-ByteString-Builder.html).

Python lets a type supply a user-facing `__str__` and a diagnostic `__repr__`. Its data model suggests a reconstructible expression for the latter where practical and uses `__repr__` for informal display when only that hook exists. CoD adopts the distinction in purpose. Each selected CoD representation is an ordinary method with a visible owner, effect and failure contract. [Python data model](https://docs.python.org/3/reference/datamodel.html#object.__repr__).

## Building a piece of text

`left ++ right` returns a builder in the full lexical scope of the operator use. The standard family accepts two builders and supplies the String/String, String/builder and builder/String combinations. A `CString` can widen to `String`. Chaining associates left at additive precedence. Evaluate each operand once in source order. Each result denotes the operands' byte sequences at concatenation time. Later appends to either input or to the result leave the others unchanged. This also defines `b ++ b`. Use `append(b, piece)` to extend a particular builder. Appending a builder to itself appends its entry contents once.

The working owner comes from the use site; a lowering can pass its scratch destination into the operator. For a selected standard method, the result has the owner it would have in an explicit `make StringBuilder(left, right)` at that use site. An ordinary method returning a reference into its own activation would fail the lifetime check. An optimization that happens to allocate a return slot in the caller cannot by itself establish this source-level owner. A builder preserves its accumulated bytes if a shorter-lived input expires. Retain an immutable segment only with proof that its owner covers every use; otherwise materialize the contribution in the builder's own workspace. Segment layout and allocation are library choices. A sequence of appends can grow incrementally, and a chain of concatenations need not flatten every prefix. Ordinary owner checks prevent escaping the workspace. Neither a region field nor a native string argument accepts the working builder.

`make String(piece) in D` is the builder overload of the existing owner-directed text construction. It consumes the current *contents* and returns a reference to complete immutable text in D. The builder remains usable. An empty builder produces the empty string. If the result is destined for a direct field in D, construct first and install with `entry.name = ready`. Earlier borrowers keep their selected text. A separately written `entry.name <- input` still selects a visible transfer method. Selecting the building operations does not install a general transfer method.

## One formatter and two destinations

The public output protocol is `Utf8StreamOut`. The module exports one borrowed symbol for **the current UTF-8 output**. Its installer can choose a memory builder, file writer, socket writer or another implementation. No default process stream is implied.

```cod
# string.cod
export type Utf8StreamOut;
export ref symbol @sout:Utf8StreamOut in O where {owner O};
```

A caller selects this identity with `using symbol "string.cod" @sout;` and installs a destination with `effect @sout = writer;`. Direct procedural helpers can call `emit(piece)` at any depth. Every reachable lookup requires a live installation. Each binding borrows its own actual owner. A nested installation restores the preceding target and owner on normal and exceptional exit. The stream owner retains responsibility for flush and close.

`emit(piece)` writes the bytes of a String or builder. It delegates to the public writer extension point `writeUtf8(@sout, piece)`. A new writer type implements `writeUtf8(out, piece)`; the explicit `out` selects the concrete backend by ordinary [method dispatch](ModuleWorlds.md#actual-type-dispatch-and-concrete-type-evidence). A type implementing a representation uses the one-argument rendering hooks below. A func can compose text or append to an explicit builder. Reading or installing the dynamic output requires a proc context.

For example, a compiler can render an object manifest to a file or use identical bytes as an in-memory test fixture. Here `ObjectList` and the `objects` generator belong to the application.

```cod
proc emit_object(dir:const String in D, file:const String in F)
    where {const owner D, const owner F} {
    emit(dir ++ "/" ++ file ++ "\n");
    return;
}

proc render_manifest(list:const ObjectList in L)
    where {const owner L} {
    emit("objects\n");
    for ref item = objects(list) { emit_object(item.dir, item.file); }
    return;
}
```

The two calling contexts below use the same imported `@sout`. In the first, D is the allocation-capable owner chosen for the final String. In the second, `writer` is an existing writable output.

```cod
ref b = make StringBuilder();
{{
    effect @sout = b;
    render_manifest(list);
}};
ref text = make String(b) in D;

{{
    effect @sout = writer;
    render_manifest(list);
}};
```

Each call to `emit_object` has its own lexical workspace. A synchronous stream writer consumes a line before that invocation ends. With a bounded backend, streaming needs space for the current piece and the I/O buffer; the manifest can exceed the available memory for a single String. The collecting route retains the manifest bytes by design. An ordinary for body shares its enclosing full scope. Moving a long-running loop's temporary concatenations into a per-record helper or an explicit full scope gives those temporaries a per-record lifetime.

`buildString(entry.name) { render_manifest(list); };` is a candidate convenience call outside the initial prototype. Such a helper would be a **proc**: it creates a private lexical builder, installs it as `@sout`, invokes the synchronous body once, then on successful completion constructs and installs the result in the destination's owner. A replaceable direct field requires a place-taking formal with a first-write contract. A bare `init tgt:String in D` initializes a selected target. Use the two primitive operations in the prototype. The public convenience signature can follow real field uses. A library offering the helper must honor nonlocal exits and prove its promised write on every normal result path.

## User-defined display and diagnostic representations

| Library call | Result and destination |
| --- | --- |
| `emitStr(x)` | Write the display form to the currently installed `@sout` and return void. |
| `emitRepr(x)` | Write the diagnostic form to the currently installed `@sout` and return void. |
| `str(x)` and `repr(x)` | Return a reference to a completed immutable String in the caller’s full lexical scope. The library collects through the corresponding streaming method. |

The type author implements the first two ordinary proc families. A method takes the value being rendered. Standard output failure can propagate through an unannotated formatter. Other checked operations in its body follow the ordinary callable contract. It calls `emit` for literal pieces and `emitStr` or `emitRepr` for nested values. If the two representations coincide, a method can explicitly delegate. Ordinary module-world selection applies to every family. A missing or ambiguous method rejects on the required path, and a failing method propagates its exception once.

For example, the [booter adapter](../applications/cod-booter/src/DiagnosticFormatting.cod) represents a [SourcePosition](../applications/cod-booter/src/SourceStorage.cod) using a one-based line and byte column. Both coordinates are zero for an unavailable position. A receiving module can export its adapted collectors and renderers together.

```cod
export import "string.cod" { str, repr, emitStr, emitRepr };
import "string.cod" emit;
using "SourceStorage.cod" SourcePosition;

proc emitStr(p:const SourcePosition in P) -> void
    where {const owner P} {
    if (p.line == 0u64 && p.column == 0u64) {
        emit("<unknown>");
        return;
    }
    emitStr(p.line);
    emit(":");
    emitStr(p.column);
    return;
}

proc emitRepr(p:const SourcePosition in P) -> void
    where {const owner P} {
    emit("SourcePosition(line=");
    emitStr(p.line);
    emit(", column=");
    emitStr(p.column);
    emit(")");
    return;
}
```

A standard collector evaluates its argument once under the caller’s bindings. It allocates a private lexical builder and installs that builder as `@sout` for the dynamic extent of the streaming method. On normal completion it restores the previous binding and finishes the builder into the caller’s use-site owner. An escaping failure restores the previous binding and publishes no partial String. The collector requires no pre-existing output for its own rendering. Effects of evaluating an argument before entry still follow that argument’s contract. Any independently declared checked failures of the selected callable remain caller obligations.

For example, `ref text = repr(p);` leaves the outer writer alone, including when a formatter calls `repr(child)` while already streaming its parent. To contribute that collected text explicitly, use `emit(text);`. For incremental nested output use `emitRepr(child);` directly. Discarding `repr(p)` still performs collection and cleanup. Result use never switches a call from collection to streaming. A formatter that explicitly writes through a separately held writer still has its ordinary effects.

The collector must resolve its corresponding hook in its selected module interpretation. Renaming a family preserves its identity. A receiver-adapted collector can use an imported and extended hook; a source-preserving collector sees its original interpretation. Qualification still needs the correct target and selected writer method when a nested proc reads `@sout`. The only compiler-supported use-site result owner here belongs to the standard collecting adapters. Streaming methods need the ordinary borrowed symbol and proc checks.

The `12:7` display and `SourcePosition(line=12, column=7)` representation describe the same input. A sum can select a spelling for each constructor. A graph type can print a domain identifier or bounded summary. The core provides no field walker, automatic private-field exposure or address rendering. Custom `repr` has no parse, identity reconstruction or stability guarantee. Recursive traversal and its termination belong to the method that elects to follow edges.

Standard text display preserves its contents, including NUL. Standard text representation surrounds the contents with double quotes and escapes double quotes, backslashes, LF, CR, TAB and U+0000 using the literal spellings. Use `\u{HEX}` with minimal uppercase hexadecimal digits for the remaining C0 and U+007F–U+009F controls. Other Unicode scalar values retain their UTF-8 encoding. The resulting quoted String representation can re-enter the CoD literal grammar. For the booter’s u64 coordinates, display uses locale-independent ASCII decimal, including zero and the full u64 range. Fraction methods, when that type is implemented, must preserve the stored numerator and denominator and the [approved fraction output](FixedWidthFractions.md); this prototype does not choose floating-point, fraction suffix or unit-formatting policy.

## Library and intrinsic boundary

Keep the rendering hooks, collecting adapters, user-type methods, decimal digits, quoting, output selection, segment composition and partial-write handling in ordinary library or provider code wherever their existing operations suffice. Resolve imported builtins and ordinary callables by identity. A local callable named `str`, a method named `repr`, or a renamed import receives exactly its normal module meaning. No frontend keyword, per-ADT hook, universal textual conversion, locale formatter or type-name branch is needed. Re-export the same callable identity when a consumer needs the extended family. Verify that a source-preserving view and a receiver-adapted view retain their ordinary differences.

The compiler/runtime boundary has three responsibilities that source spelling alone cannot prove:

| Operation | Required check or support |
| --- | --- |
| Access validated text or pass it to a writer | Supply a bounded synchronous readonly loan with the true owner and UTF-8 byte length. Preserve interior zeroes. Only the established C string boundary prepares a terminator. The native write primitive must report progress or failure. |
| Create immutable text in a specified owner | Check allocation permission, size arithmetic and lifetime, establish complete immutable backing before publishing the result, and use a relocatable representation in a region. Reuse existing String construction for validated input. |
| Produce a standard result in the use-site owner | For `++`, `str(x)` and `repr(x)`, let the selected library adapter receive checked storage in the full lexical scope of the use. A compiler may specialize or inline it. Preserve operand evaluation, applicable method identity, failures, target lifetime and cleanup. The mechanism belongs to the resolved standard adapter, never to a name token. |

A new intrinsic is warranted only for an operation that genuinely needs compiler-owned evidence or native access. Use one checked semantic identity and one shared definition across seed and booter for each operation that needs catalog publication. Reuse the [catalog service](../applications/cod-booter/docs/LLVMInterop.md#builtin-catalog-and-rejection-reporting) when applicable. The library may choose chunks or contiguous scratch and eliminate intermediate materialization when the observable owners, bytes, effects and failure behavior permit it. Byte layout, buffering size and permanent ABI are implementation decisions.

## Failure, cleanup and acceptance

The library defines its own nominal `export data Utf8WriteError <: Error { code:u64; }` with the ordinary `Error` identity from `std:exceptions`. A terminal UTF-8 output failure raises that type. Calls to `writeUtf8`, `emit`, the streaming hooks and the collecting adapters can propagate it without a catch or a `throws` clause. A formatter usually cannot recover from losing its destination. The application boundary that knows how to abandon a report, handle a closed pipe or fail an export can catch the concrete error there. A higher-level operation promising durable publication can translate it into its own checked failure under its normal contract.

```cod
using "string.cod" Utf8WriteError;

# The caller has installed the stream. The recovery helper records the code
# through an independent channel.
try { emitRepr(position); }
catch (e:Utf8WriteError) { record_output_failure(e.code); };
```

A completed write contributes the supplied bytes in order, including U+0000. On successful return the writer has consumed the bytes or established independently live buffering. A short write reports progress; continue from the first unwritten byte when the backend permits it. For a nonempty write that cannot make progress, follow the backend’s wait-or-fail contract. On terminal failure, propagate the error once and preserve the accepted prefix, even if it ends inside a multibyte character. Scope cleanup restores the previous `@sout`. Flush, close and file publication retain the external owner’s contracts. Any adapter queuing bytes beyond a call must establish sufficiently long-lived storage.

The unchecked classification applies to this writer failure. Allocation failure retains its own category; a custom formatter or provider that calls an unrelated checked operation still owes the declared handling contract. The standard adapter does not catch arbitrary exceptions and relabel them as output errors.

For collection, finish before installing the field. If formatting or final construction throws, that final installation has not occurred. The former field and its borrowers remain usable unless the user's code independently changed them. Ordinary cleanup still runs for scratch resources, and prior initializer or output effects keep their semantics.

Verify the same manifest rendered both ways. Compare bytes for empty input, non-ASCII text and an embedded NUL. Reuse a builder after finishing and an operand after concatenation, then end the source scope and verify the finished text under D. Redirect a nested formatter to an inner builder, inject a write failure, catch it outside the installation and emit again to the restored outer target. Exercise short writes and failure following a successful prefix. Stream many records with a bounded writer and per-record scratch, without constructing the complete manifest. Test propagation from a formatter without a catch or `throws`, optional recovery by `catch (e:Utf8WriteError)`, and an unrelated checked operation that still needs coverage. A streaming call that needs the output with no live binding, an escaping reference to a dead workspace, and an attempt to access the symbol from a func must reject.

## Booter prototype

1. Exercise this on a rejected input entering the real [rejection reporting](../applications/cod-booter/src/NativeDiagnostics.cod) path. Use the coordinates it actually resolves in [SourceAccess.cod](../applications/cod-booter/src/SourceAccess.cod). A prototype adapter may format the recorded numeric facts after the source loan closes. Keep the production [launcher diagnostic record](../applications/cod-booter/docs/LLVMInterop.md#builtin-catalog-and-rejection-reporting), its stage/code, status, related position and established CLI output. Prototype renderings may be observed separately. A fabricated position alone cannot establish this integration.
2. Render one available position, an absent position and a UTF-8 string with a quote, LF and NUL through both an in-memory builder and a bounded writer. Compare the bytes for each mode. Compare collection with direct streaming. Collect with no outer installation, then collect and discard a representation inside an active writer. Verify that only an explicit emit reaches that writer. Have an argument emit a marker before collector entry. Make a nested custom renderer emit a prefix and throw, catch outside the collector, and send the next marker to the restored writer. Exercise receiver-adapted and renamed renderer families and reject an uncovered source-preserving call or ambiguity. An output failure after a prefix must propagate once without retrying the entire formatter, and nested output must restore its predecessor.
3. Demonstrate the ordinary CoD library and adapter declarations in the bootstrapping paths. Verify real behavior on the supported path and separately report parser/checker reach, generated code and actual runtime evidence for each compiler. Preserve the [compiler diagnostics boundary](../applications/cod-booter/README.md#diagnostics-by-compiler-stage). A semantic primitive used by both frontends needs one source of truth. Keep host code confined to the services it supplies, and remove parallel prototype logic when the library path replaces it. Bring a genuine owner, method or cost conflict back to design before accepting weaker semantics.
