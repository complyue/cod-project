(comment) @comment
(string) @string
(number) @number
(boolean) @boolean
(null_literal) @constant.builtin
(builtin_type) @type.builtin
(builtin_function) @function
(symbol) @variable.special.cod-symbol
; More specific keyword patterns follow the generic fallback.
(keyword) @keyword

((keyword) @keyword.control.cod
  (#any-of? @keyword.control.cod
    "assert" "break" "case" "catch" "continue" "defer" "else" "fallback"
    "finally" "for" "if" "of" "return" "returning" "throw" "try"
    "yield"))

((keyword) @keyword.declaration.cod
  (#any-of? @keyword.declaration.cod
    "array" "data" "from" "func" "function" "global" "here" "in" "owner"
    "proc" "procedure" "ptr" "ref" "region" "symbol" "throws" "type" "unit"
    "units" "val" "var" "where" "yields"))

((keyword) @keyword.allocation.cod
  (#eq? @keyword.allocation.cod "make"))

((keyword) @keyword.modifier.const.cod
  (#eq? @keyword.modifier.const.cod "const"))

((keyword) @keyword.module.cod
  (#any-of? @keyword.module.cod "export" "import" "mixin" "using"))

((keyword) @operator.cod
  (#any-of? @operator.cod "is" "isa" "not"))

[
  (operator)
  (pipeline_operator)
  (reserved_operator)
] @operator.cod

(delimiter) @punctuation.delimiter
["(" ")" "[" "]" "{" "}"] @punctuation.bracket
