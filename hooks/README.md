# Git Hooks

## Setup

Run from project root:
```bash
./setup-hooks.sh
```

## pre-commit

Formats `.hh` and `.cc` files using `build/bin/clang-format`, falling back to
`built/bin/clang-format` for release/pre-release installs.

Requirements:
- `build/bin/clang-format` or `built/bin/clang-format` (build project first)

Skip formatting:
```bash
git commit --no-verify
``` 
