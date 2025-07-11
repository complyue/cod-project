# CoD Project Structure & Package Management

_Initial design & implementation notes (stage 1)_

---

## Goals

1. Provide a **deterministic, VCS-native** package workflow for the CoD ecosystem.
2. Guarantee **reproducible builds** via a lock-file (`CodManifest.yaml`).
3. Make git the **single source of truth** – no central registry, no tarballs, no version numbers, only commit hashes and branch names.
4. Optimise for **zero-cost sharing** of rich, pointer-heavy data structures via shilos memory regions (writable, mmap-shared across cooperating processes; no (de)serialization needed).

## Terminology

| Term                    | Meaning                                                                                                                                                                                                                                                                                                |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| _CoD package / project_ | A source tree that contains a `CodProject.yaml` file.                                                                                                                                                                                                                                                  |
| _Canonical repo_        | The authoritative git repository declared by the package author (usually the repo’s _origin_ URL). Users typically depend on it rather than forking it.                                                                                                                                                |
| _Bare mirror_           | A local mirror of the canonical repo stored under `~/.cod/pkgs/repos/` (read-only; kept up-to-date by `codp`).                                                                                                                                                                                         |
| _Snapshot_              | A read-only checkout of a specific branch/commit under `~/.cod/pkgs/snapshots/ <url_key>/<branch-or-sha>/`. Users must **not** modify it; `codp` recreates / fast-forwards as needed.                                                                                                                  |
| _Manifest_              | `CodManifest.yaml` – records the last dependency resolution. It can optionally pin commits (lock) when the caller requests, or just record branch heads for normal floating behaviour. Additionally, it records **local dependencies** that point to developer-supplied paths instead of remote repos. |

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

### Implementation note – YAML ↔️ In-memory conversion

At implementation level **all** parsing / serialisation of `CodProject.yaml` is handled via the generic
`YamlConvertible` concept defined in `shilos/prelude.hh`.

Concrete data types (`cod::project::CodProject`, `cod::project::CodDep`, *etc.*) provide ADL-visible
free functions:

```
yaml::Node to_yaml(const CodProject &);
template<typename RT> void from_yaml(shilos::memory_region<RT>&, const yaml::Node&, CodProject*);
```

This separation guarantees that **no component outside the model classes touches raw YAML nodes** – the
CLI tool (`codp`) or future REPL simply loads a YAML document, hands the root node to
`from_yaml`, and all further mutations happen through the typed C++ API on the in-memory graph.

Benefits:

1. Strong data ownership – the YAML layer becomes a pure I/O concern.
2. Centralised validation logic – any structural check lives inside `from_yaml` of the relevant type.
3. Future alternative front-ends (binary format, DB, …) can reuse the exact same data model by
   plugging in a different converter set.

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

## Compile-on-Demand (future)

- The `cod` REPL will parse `CodManifest.yaml`, map each work-tree, generate include paths and other compiler flags, then JIT/Orc-JIT the requested entry points on-demand.
- Shilos memory regions are shared via `mmap`-backed files; process orchestration happens over region root objects.

---

_This document will evolve as the implementation matures._
