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

if [ -f "built/bin/clang-format" ]; then
    echo "✓ clang-format binary found"
else
    echo "⚠ Warning: clang-format binary not found at built/bin/clang-format"
fi

echo "Git hooks setup complete." 
