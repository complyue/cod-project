# CoD Project Structure & Package Management

Design revision: 2 · Recorded: 2026-11-25 · Status: package-management reference with scoped module-entry terminology.

## Design authority

At r2, [shilos retirement](../CHANGELOG.md#h-retire-shilos-20261125) removes the C++ storage and converter prescriptions. Package metadata, local overrides and deterministic input selection retain their existing qualification.

- **Registry / scope:** [D-03](DesignAuthorityRegistry.md#d-03) remains pending-human for the package-management design. The scoped terminology decision and the retirement of the C++ implementation leave broader package policy pending.
- **Baseline / value:** [H-500](../CHANGELOG.md#h-500)/[D-15-ENTRY](DesignAuthorityRegistry.md#d-15-entry) distinguishes the dependency/version package from the internal namespace selected to run `__main__`, permitting multiple entry namespaces per package. P-452–P-454 through C-35/C-44/C-45/C-59 is a candidate charter §§2/3.2/4.2/8 path for explicit identity, source organization and execution roots. P-05's broader package value path remains pending.
- **Invariants / acceptance:** preserve VCS/manifest/local-override requirements and module identity/privacy; directory ancestry creates no owner, implicit import or initialization. Entry selection must distinguish two namespaces in one package and reject absent/ambiguous entries without activating siblings. Exact CLI qualification/path mapping and broader package qualification remain separate; [D-15](ModuleWorlds.md#packages-namespaces-and-entry-selection) owns those boundaries.
- **Continuity:** [K-STATIC-MODULE-STARTUP-20261116](../CHANGELOG.md#k-static-module-startup-20261116) records the scoped terminology update against the registered baseline blob, without a package-manager or compiler implementation change.

---

## Goals

1. Provide a **deterministic, VCS-native** package workflow for the CoD ecosystem.
2. Guarantee **reproducible builds** via a lock-file (`CodManifest.yaml`).
3. Make git the **single source of truth** – no central registry, no tarballs, no version numbers, only commit hashes and branch names.
4. Optimise for **zero-cost sharing** of rich, pointer-heavy data structures through relocatable, memory-mapped regions shared across cooperating processes.

## Terminology

| Term                    | Meaning                                                                                                                                                                                                                                                                                                |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| _CoD package_ | A dependency-resolution and version-management unit whose source tree contains CodProject.yaml. It may contain multiple modules and independently selectable entry namespaces. |
| _Project_ | The development source tree/configuration for a package; this role is not an internal namespace level. |
| _Namespace / subnamespace_ | An internal named grouping of modules and nested namespaces, corresponding to a source-directory level. |
| _Namespace segment_ | One name component of a namespace path. |
| _Module_ | A semantic unit with declarations, a module world and any executable initialization. |
| _Directory_ | A physical source-tree location; source-root and filename mapping remain separate tooling rules. |
| _Canonical repo_        | The authoritative git repository declared by the package author (usually the repo’s _origin_ URL). Users typically depend on it rather than forking it.                                                                                                                                                |
| _Bare mirror_           | A local mirror of the canonical repo stored under `~/.cod/pkgs/repos/` (read-only; kept up-to-date by `codp`).                                                                                                                                                                                         |
| _Snapshot_              | A read-only checkout of a specific branch/commit under `~/.cod/pkgs/snapshots/ <url_key>/<branch-or-sha>/`. Users must **not** modify it; `codp` recreates / fast-forwards as needed.                                                                                                                  |
| _Manifest_              | `CodManifest.yaml` – records the last dependency resolution. It can optionally pin commits (lock) when the caller requests, or just record branch heads for normal floating behaviour. Additionally, it records **local dependencies** that point to developer-supplied paths instead of remote repos. |

The CLI entry design selects the **namespace containing `__main__`**, rather than a dependency package as one executable unit. One package may contain `tools/server/__main__` and `tools/client/__main__`; these are illustrative logical paths, not a selected CLI grammar. [Module entry selection](ModuleWorlds.md#packages-namespaces-and-entry-selection) defines reserved naming, non-reusability and initialization of only the selected dependency closure. Namespace ancestry does not itself publish/import definitions or execute sibling modules. This terminology does not add package dependency-cycle support or settle package-management policies elsewhere in this document.

## On-disk layout

```
~/.cod/
  pkgs/
    repos/                 # bare mirrors
      <url_key>.git/
    branches/              # mutable work-trees (one folder per branch)
      <url_key>/
        <branch>/   <--- checkout here
```

`<url_key>` is a filesystem-safe encoding of the repo URL:

```
https://github.com/llvm/llvm-project.git  ->  https___github.com_llvm_llvm-project.git
```

## `CodProject.yaml` schema (author-maintained)

```yaml
uuid: "<RFC-4122 UUID>" # immutable identity of the package
name: "pretty-name"
repo_url: "https://…/myrepo.git"
branches: [main] # branches **this** pkg publishes for others to depend on

deps: # direct dependencies, order = preference
  - uuid: "…"
    repo_url: "https://…/dep.git"
    branches: [main, dev]
    # OPTIONAL: during multi-repo development you may replace the remote source
    # with a developer-local checkout.  If present, `path` wins over `repo_url`.
    path: "../deps/dep" # relative or absolute path on local filesystem
  - …
```

### Package metadata I/O

Read and validate YAML at the package input boundary. Resolution uses typed dependency records. Writing the manifest serializes the selected revisions. The on-disk schema and local overrides apply independently of how the package manager stores its working data.

Notes:

- **No versions:** a dependency is a `(uuid, repo_url | path, branch-list)` triple.
- Branch list is **priority-ordered** – first branch that resolves wins.
- If `path` is present the dependency is treated as **local**. `codp` performs **no git clone/fetch** for that entry – it reads sources directly from the given directory and resolves its `CodProject.yaml` in-place.

## `CodManifest.yaml` schema (machine-generated)

```yaml
root:
  uuid: "…"
  repo_url: "…"

locals:
  # Map of UUID → filesystem path, populated if any dep entry used a `path` key
  "123e4567-e89b-12d3-a456-426614174000": "/home/alice/work/depA"

resolved:
  - uuid: "…"
    repo_url: "…"
    branch: "main"
    commit: "0123456abcdef" # *pinned!* 40-char SHA-1/2 whatever the repo uses
  - …
```

Rules:

- Contains **every transitive dependency**.
- Each entry has **exactly one** `(branch, commit)`.
- Order is topological (parents before children).

## `codp` command-line interface (stage 1)

```
# Default command – can omit "solve"
codp solve [--project <path>]

# Other management commands
codp update                 # fetch remotes & fast-forward every snapshot
codp add <uuid-or-url>      # add a direct dependency (writes CodProject.yaml)
codp rm  <uuid-or-url>      # remove a direct dependency
codp gc                     # clean unused cached mirrors & snapshots
```

### Behaviours

- **solve** – reads the current `CodProject.yaml`, clones/fetches any missing canonical repos, sets up snapshot directories (or honours `path:` for local deps) and writes `CodManifest.yaml`. The command can be executed from **any sub-directory** inside the project; `codp` walks parent directories until it finds `CodProject.yaml`.
- **update** – performs network fetches for every canonical repo already present, then updates each snapshot directory to the latest commit of its configured branch. Local deps (`path:`) are untouched.
- **add / rm** – convenience helpers that edit the project’s `CodProject.yaml` and then run an implicit `solve`.
  - _Key syntax_ for `<uuid-or-url>`:
    1. **UUID** – always unique.
    2. **repo-URL** – full HTTPS/SSH URL.
    3. **short name** – the `name:` field of the target package _when that name is unique among already-known dependencies_.
       _If ambiguity occurs (`codp` detects multiple candidates), it asks the user to use the UUID instead._
    4. Future: `owner/name` GitHub shorthand → expanded to `https://github.com/owner/name.git`.
- **gc** – garbage-collects mirrors & snapshots that are _unused **across all** specified projects_. The user must provide a list of manifest roots to keep, e.g. `codp gc --roots proj1 proj2 …`. Nothing is deleted if `--roots` is omitted, preventing accidental loss of the global cache.

## Dependency resolution algorithm

1. Input set = _direct_ deps from `CodProject.yaml`.
2. For each dependency (depth-first, deduplicating by `uuid`):
   1. Ensure bare mirror is up-to-date.
   2. Read _that_ package’s `CodProject.yaml` from the target branch (via `git show`).
   3. Recurse into its deps.
3. Stop on first satisfying branch per dependency.
4. Detect & report cycles (by tracking the visiting stack of UUIDs).

## Compiler inputs

- The compiler consumes package roots prepared by `codp` for a source invocation. [Package inputs and module selection](ModuleWorlds.md#packages-namespaces-and-entry-selection)
- Cooperating processes can share a mapped image. Its owner and publication follow the [DBMR contract](ConstructionAndAcquisition.md#publishing-a-newly-created-dbmr).

---

_This document will evolve as the implementation matures._
