# Git Hooks

## Setup

Run from project root:
```bash
./setup-hooks.sh
```

## pre-commit

Formats `.hh` and `.cc` files using `built/bin/clang-format`.

Requirements:
- `built/bin/clang-format` (build project first)

Skip formatting:
```bash
git commit --no-verify
``` 
