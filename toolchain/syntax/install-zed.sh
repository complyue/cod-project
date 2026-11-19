#!/bin/sh
# Build and link the local editor extension. This does not build CoD.
set -eu

syntax_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
if [ -n "${TREE_SITTER:-}" ]; then
    cli=$TREE_SITTER
elif [ -x "$syntax_dir/.local/tree-sitter" ]; then
    cli="$syntax_dir/.local/tree-sitter"
else
    cli=tree-sitter
fi
if ! cli=$(command -v "$cli"); then
    echo 'Tree-sitter CLI not found. Install 0.27.x or set TREE_SITTER to its absolute path.' >&2
    exit 1
fi
case $cli in
    /*) ;;
    */*) cli="$PWD/$cli" ;;
esac
command -v node >/dev/null 2>&1 || {
    echo 'Node.js is required to generate the Tree-sitter parser.' >&2
    exit 1
}

if [ -z "${ZED_DATA_DIR:-}" ]; then
    case $(uname -s) in
        Darwin) ZED_DATA_DIR="$HOME/Library/Application Support/Zed" ;;
        Linux) ZED_DATA_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/zed" ;;
        *) echo 'Set ZED_DATA_DIR to your Zed data directory.' >&2; exit 1 ;;
    esac
fi

bundle="$syntax_dir/.local/zed-extension"
installed="$ZED_DATA_DIR/extensions/installed"
link="$installed/cod-preview"
if [ -e "$link" ] || [ -L "$link" ]; then
    if [ ! -L "$link" ] || [ "$(readlink "$link")" != "$bundle" ]; then
        echo "Refusing to replace an existing extension at $link" >&2
        exit 1
    fi
fi

mkdir -p "$syntax_dir/.local"
stage=$(mktemp -d "$syntax_dir/.local/zed-build.XXXXXXXX")
trap 'rm -rf "$stage"' EXIT
trap 'exit 1' HUP INT TERM
cp "$syntax_dir/zed/grammar.js" "$stage/grammar.js"
(
    cd "$stage"
    "$cli" generate --abi 14
    "$cli" build --wasm --output cod.wasm
)

mkdir -p "$bundle/grammars" "$bundle/languages/cod" "$installed"
cp "$syntax_dir/zed/extension.toml" "$bundle/extension.toml"
cp "$syntax_dir/zed/languages/cod/"* "$bundle/languages/cod/"
mv "$stage/cod.wasm" "$bundle/grammars/cod.wasm"
if [ ! -L "$link" ]; then
    ln -s "$bundle" "$link"
fi
printf 'CoD preview installed at %s\nRestart Zed and open toolchain/syntax/showcase.cod.\n' "$link"
