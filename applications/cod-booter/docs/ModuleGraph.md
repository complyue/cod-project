# Module graph, workspace and dependency callbacks

## Design authority

[D-20-MODULE-GRAPH](../../../design/DesignAuthorityRegistry.md#d-20-module-graph), revision 1. The project user set the compiler/native boundary and delegated pre-code architecture review to the design discussion. [H-COMPILER-LAYERS-20261122](../../../CHANGELOG.md#h-compiler-layers-20261122) records the instruction and the reviewer's scoped clearance. P-680 derives the design from C-80. Inspectable compiler decisions and lifetimes make a real module compilation a test of the compiler language. P-681 identifies the applicable sum, regional storage, module selection, dynamic installation and native loan contracts. Coverage includes the first booter's declaration, C-provider and object routes. The [acceptance workload](#acceptance) checks their behavior and physical separation. Permanent ABI, persistent caches and changes to language semantics require their own decisions.

## Layer responsibilities

| Responsibility | Implementation |
| --- | --- |
| Compiler state and policy | The seed implements its compiler objects and algorithms in C++. The booter implements its module frame, typed workspace, canonical-key comparison and interning, dependency table, traversal, checking, diagnostic selection and publication in CoD. CoD decodes import spellings and decides which exports, interfaces, native units and generated provider content to produce. |
| Native operations | C performs OS path resolution, lends bounded canonical path bytes and mapped source with its line index, supplies aligned raw region storage, and exposes the LLVM and file operations the compiler requests. The launcher enters CoD. A synchronous native loan or call to LLVM has its own checked owner, bounds and completion contract. |

The migration removes compiler-owned decisions from the current [C graph provider](../native/module_graph_provider.c) and [typed scratch provider](../native/scratch_provider.c). In particular, selecting the root, recursing into a child, marking a module checked, interpreting source escapes and assigning a diagnostic to a requesting edge are compiler operations. The same division applies to declaration checking, provider generation and object construction.

## Owners across a visit

| Lifetime | Contents and use |
| --- | --- |
| Compilation graph G | Canonical keys, stable invocation-local module IDs, visit states, edges, and published spellings, interfaces, command and literal ranges. A checked entry becomes visible only after its required fields and ranges are initialized and the module succeeds. Retain the graph through final output. |
| Module visit F | A fresh CoD frame and typed workspace for each active visit. CoD computes checked counts and placement for tokens, AST nodes, dependency candidates, bindings, facts and commands. Raw storage gains its element type and readiness through checked construction. |
| Indexed source revision S | One successful owning acquisition per opened revision. Reacquire immutable source bytes and the matching line index from that same S for successive local stages. End each native loan before visiting a child. |
| Diagnostic destination | The launcher supplies separately owned storage. CoD resolves a span against its own revision while live. A dependency failure reported to the parent uses its requesting import span. An open failure without a source position retains its OS error. |

A suspended parent may keep its F and S. Across a child call it retains IDs and spans for graph data and reacquires views before use. An ID requires the graph it belongs to. Publish the required bytes or typed rows in G before their source owner ends. Growth must preserve actual-owner evidence. In particular, [consuming region resize](../../../design/RegionalAndLexicalStorageRefactor.md) requires accounting for the owner and all live dependent uses, including handler and cleanup bindings. A stable module ID alone cannot keep a borrow valid across replacement. The design permits storage strategies that avoid replacing a borrowed region.

Use named sum alternatives for states and syntax nodes with differing payloads. For example, a literal has its value, a call has its callee and argument indices, and a return has its applicable return payload. Keep the common half-open source span, the applicable operator anchor, and the association with the owning syntax revision. A consumer matches the constructor before accessing the variant fields. Child indices retain their meaning when the backing relocates. Use enums for simple closed codes. Project initialized native command rows at the LLVM boundary; the CoD AST layout is private to the compiler.

## Reentry and completion

The compilation activation installs a scoped symbolic dependency procedure. The driver visits the root, discovers edges in source order and invokes the installed procedure with `behave` for each edge. It reuses checked modules and applies the current compiler profile's cycle and depth checks to active visits. On a new visit it opens the child revision and creates its frame. Once all required children succeed, the parent binds, checks and publishes using their stable IDs.

The first root runs with no preinstalled dependency handler. A lookup while executing `behave` sees the current installation. An actual definition-activation snapshot or `touch` dependency must still be satisfied at activation. Recursive calls can use ordinary named CoD procedures. A nested compilation may execute the same lexical installation site for its own graph and restore its predecessor on exit. The application needs neither a stored handler nor a new rule for recursive capture or method-family composition. [Definition-installed procedures](../../../design/SymbolicDynamicScope.md#definition-installed-effectful-procedures)

Normal return, rejection and exceptional exit preserve the parent binding and frame. Release each successfully acquired resource exactly once in reverse acquisition order within its lifetime. A failed native acquisition cleans its acquired prefix before reporting failure. An abrupt CoD completion crosses a native loan only after its protected cleanup. A failed visit publishes no checked entry for that module. Retain completed graph data needed by LLVM after source mappings close. File output follows the existing publication contract.

## Acceptance

- Build real booters in both seed modes. Exercise a root with no previous handler, then nested compilations with distinct graphs. Observe outer → inner → outer dispatch on success and after a caught child exception. Check the actual graph and frame selected at each step, cleanup counts, and restoration before the outer handler resumes.
- Exercise a diamond with a reused leaf and a renamed re-export, the 49-unit graph, cycle and depth failures, failed open/canonicalization, and child rejection at the parent's import span. Check module and definition identity, initialized publication, and preservation of existing object output on rejection.
- Exercise typed construction, legal relocation or storage growth, and subsequent matching and lowering of the same indexed AST. Reject wrong-variant access and incomplete fields. Verify that transient source and frame views cannot enter G or survive owner replacement.
- Compare the real checked cross-module output (`0, 11, 0`) and deferred-release output (`1, 1, 1`). Compare generated CompilerCLI provider bytes with the seed, then relink and run using those providers. Verify that acquired revisions close before final LLVM construction from graph-owned units. Review the C entry points as part of acceptance.
