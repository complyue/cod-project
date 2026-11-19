# CoD syntax preview

Open [`showcase.cod`](showcase.cod) for idiomatic examples of CoD syntax. The numbered sections are independent excerpts, not one compilable module. Confirmed design targets awaiting seed migration, parser-only forms, and design-only forms are marked separately from seed examples. Keep lexer edge cases and intentionally rejected forms in the compiler fixtures.

The editor grammars follow [D-10-ORDINARY-NAMES](../../design/DesignAuthorityRegistry.md#d-10-ordinary-names) and [D-22-MAKE-OWNER](../../design/DesignAuthorityRegistry.md#d-22-make-owner) ahead of the seed migration. The released names, including `fold` and `len`, and the former `local` modifier are highlighted as ordinary identifiers. `touch` remains special, and ordinary construction is written `make`.

- [`CoD.tmLanguage.json`](CoD.tmLanguage.json) is a TextMate grammar with scope `source.cod`, available for future Linguist submission. It is not registered with GitHub.
- [`zed/`](zed/) contains the Tree-sitter grammar, extension metadata, and Zed queries for `.cod`. The grammar recognizes tokens and balanced groups for highlighting and bracket matching. It does not validate declarations, infer operator roles, or establish CoD source compatibility.

## Activate in local Zed

From the repository root on macOS or Linux:

```sh
sh toolchain/syntax/install-zed.sh
```

The installer requires Node.js and the Tree-sitter CLI 0.27.x (on `PATH`, in `.local/tree-sitter` next to this README, or specified as an absolute path in `TREE_SITTER`). For its WebAssembly build, Tree-sitter uses `TREE_SITTER_WASI_SDK_PATH` if set and otherwise can download an editor-only WASI SDK to its cache. This does not configure or build the CoD compiler.

The installer builds a bundle under the ignored `toolchain/syntax/.local/` directory and links it into Zed's installed extensions directory as `cod-preview`. It will not replace an unrelated extension already using that name. Use `ZED_DATA_DIR` for a nondefault Zed user-data directory. The entry links a locally built bundle; Zed's **Install Dev Extension** action (which fetches grammar revisions from Git) is not needed.

Fully quit and restart Zed, then open `showcase.cod`. The language indicator should say **CoD**. To update the preview after editing the grammar or queries, rerun the installer and restart Zed. To remove it, uninstall **CoD Preview** in Zed or remove just the `extensions/installed/cod-preview` symlink from your Zed user-data directory and restart.

Zed distinguishes module directives (`keyword.module.cod`), declarations (`keyword.declaration.cod`), control flow (`keyword.control.cod`), the `const` modifier (`keyword.modifier.const.cod`), and `@symbols` (`variable.special.cod-symbol`). Operators (`operator.cod`) use slate blue, while `make` (`keyword.allocation.cod`) uses bold yellow-green. The example below also uses italic gold for module directives, green for declarations (including `in`), red for `const`, and rose for symbols. Control flow keeps the theme's usual keyword color.

```json
{
  "theme_overrides": {
    "Ayu Dark": {
      "syntax": {
        "keyword.module.cod": { "color": "#E6C164", "font_style": "italic", "font_weight": 600 },
        "keyword.declaration.cod": { "color": "#69C6A0", "font_weight": 600 },
        "keyword.modifier.const.cod": { "color": "#FF4248" },
        "keyword.allocation.cod": { "color": "#C5DC68", "font_weight": 700 },
        "variable.special.cod-symbol": { "color": "#F58FA1" },
        "operator.cod": { "color": "#91A7C4" }
      }
    },
    "One Light": {
      "syntax": {
        "keyword.module.cod": { "color": "#805803", "font_style": "italic", "font_weight": 600 },
        "keyword.declaration.cod": { "color": "#186448", "font_weight": 600 },
        "keyword.modifier.const.cod": { "color": "#C60016" },
        "keyword.allocation.cod": { "color": "#506400", "font_weight": 700 },
        "variable.special.cod-symbol": { "color": "#A43650" },
        "operator.cod": { "color": "#405B78" }
      }
    }
  }
}
```

When changing either grammar, check the showcase for comments, quoted module paths, fraction `//` versus division `/`, range `..` versus decimal points, and both orientations of the pipeline connectors. For redundant path separators and other lexer edge cases, see [`comments.cod`](../../tests/cod/language/comments.cod). A highlight match is not a feature-support claim; see the [seed implementation scope](../../tests/cod/language/README.md#current-implementation-boundary) and the [design authority registry](../../design/DesignAuthorityRegistry.md).
