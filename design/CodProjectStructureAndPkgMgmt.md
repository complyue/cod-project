# Preparing a bootstrap project

## Design authority

At r4 the [repository-name correction](../CHANGELOG.md#h-package-repository-name-20261125) identifies the production repository. The r3 scope continues. The [project-local layout decision](../CHANGELOG.md#h-manual-bootstrap-packages-20261125) retires `codp` from the bootstrap. [D-03](DesignAuthorityRegistry.md#d-03) assigns future package management to `e-wrks/cod`. [D-15-MODULE-PATH](DesignAuthorityRegistry.md#d-15-module-path) and P-690/P-691 govern how a compiler uses the supplied trees. Deterministic selection, observable ambiguity and the toolchain fallback apply to both compilers. The [prior package-manager draft](https://github.com/complyue/cod-project/blob/3545f504243558aeb923ba2fadc84c6de054715b/design/CodProjectStructureAndPkgMgmt.md) is archived.

## Directory layout

Put the project's own sources under the project root. Place each dependency's source tree in one immediate subdirectory of `.cod/packages/`. Stage the full set needed by the entry, including transitive dependencies, at that same level. An empty `.cod/packages/` marks a project with no staged dependencies.

```text
project/
  app/
    Main.cod
  .cod/
    packages/
      vendor-a/
        billing/
          rates.cod
```

Here `project` and `project/.cod/packages/vendor-a` are the two source roots. The label `vendor-a` organizes the staged directories. The namespace path starts inside that root.

`app/Main.cod`

```cod
using "billing/rates.cod" { rate }
using symbol "std:io" @print;
behave @print(rate());
```

`.cod/packages/vendor-a/billing/rates.cod`

```cod
export func rate() -> u64 { return 42u64 }
```

Start from the original entry file to find the nearest enclosing project with a `.cod/packages/` directory. The seed's explicit `--project DIR` selects that directory as the project root. Without a dependency directory, this explicit project supplies only its own sources. Every dependency edge in the invocation uses this same set of roots. An entry outside a marked project can use its source-relative imports and the active toolchain library.

For the example, a seed run should print `42`. Declaration checking, provider generation and object emission in the booter should select that same file. Adding `vendor-b/billing/rates.cod` under `.cod/packages/` makes the import ambiguous. A project claim that shadows the active toolchain's bundled module wins with a warning. A broken selected module reports its own error. Each new invocation observes the directories as prepared for that run. [Lookup and diagnostics](ModuleWorlds.md#resolving-source-module-paths)

## Cutover status

The C++ seed built from `2bd8b365` ran the two files above from an unrelated working directory and printed `42`. On a copy with a second staged provider, the seed reported both claim paths and preserved the existing LLVM output. The linked booter at the [pre-cutover revision](https://github.com/complyue/cod-project/tree/3545f504243558aeb923ba2fadc84c6de054715b) still invokes `codp compiler-inputs`. Its normal declaration, provider and object commands need independent verification on this prepared tree.

## Terminology

A **package** is a dependency and versioning unit. A **project root** supplies the project's own namespace tree. A **staged dependency root** supplies one more namespace tree. A **module** has its own declarations and module world. A namespace can contain several modules or an entry. [Namespace and entry selection](ModuleWorlds.md#packages-namespaces-and-entry-selection)

Source acquisition and version selection happen before the bootstrap compiler runs. The user prepares the directories directly. Future package acquisition, resolution and tools belong in `e-wrks/cod`. Its commands, metadata formats and version policy are future design work.
