#!/bin/bash

set -e

echo "Setting up Git hooks..."

if [ ! -d ".git" ]; then
    echo "Error: Not in git repository root."
    exit 1
fi

if [ ! -d "hooks" ]; then
    echo "Error: hooks/ directory not found."
    exit 1
fi

mkdir -p .git/hooks

if [ -f "hooks/pre-commit" ]; then
    [ -f ".git/hooks/pre-commit" ] && rm .git/hooks/pre-commit
    ln -s "../../hooks/pre-commit" .git/hooks/pre-commit
    echo "✓ Pre-commit hook installed"
fi

if [ -x "build/bin/clang-format" ] || [ -x "build/bin/clang-format.exe" ]; then
    echo "✓ clang-format binary found"
elif [ -x "built/bin/clang-format" ] || [ -x "built/bin/clang-format.exe" ]; then
    echo "✓ clang-format binary found in built/"
else
    echo "⚠ Warning: clang-format binary not found under build/bin or built/bin"
fi

echo "Git hooks setup complete." 
