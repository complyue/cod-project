(comment) @comment
(string) @string
(number) @number
(boolean) @boolean
(null_literal) @constant.builtin
(builtin_type) @type.builtin
(builtin_function) @function
(symbol) @variable.special.cod-symbol

; Only color names as types in recognizable syntax positions. Bare calls,
; argument arrows, and module selections do not identify a type.
(identifier) @variable.cod

(source_file
  (keyword) @_type_declaration
  . (comment)*
  . (identifier) @type.cod
  (#any-of? @_type_declaration "array" "data" "type" "unit"))

; In a declaration head, `<:` names an abstract parent or conformance type.
(source_file
  (keyword) @_abstract_relation
  . (comment)*
  . (identifier)
  . (comment)*
  . (operator) @_abstract_relation_operator
  . (comment)*
  . (identifier) @type.cod
  (#any-of? @_abstract_relation "data" "type")
  (#eq? @_abstract_relation_operator "<:"))

; `extends` is an identifier except in a product declaration head.
(source_file
  (keyword) @_product
  . (comment)*
  . (identifier) @type.cod
  . (comment)*
  . (identifier) @keyword.declaration.cod
  . (comment)*
  . (identifier) @type.cod
  . (comment)*
  . (block)
  (#eq? @_product "data")
  (#eq? @keyword.declaration.cod "extends"))

; The dot distinguishes ancestor selection from a parent constructor call.
((keyword) @_super
  . (comment)*
  . (parenthesized "(" . (comment)* . (identifier) @type.cod . (comment)* . ")")
  . (comment)*
  . (delimiter) @_super_dot
  . (comment)*
  . (identifier)
  . (comment)*
  . (parenthesized)
  (#eq? @_super "super")
  (#eq? @_super_dot "."))

((keyword) @_member_declaration
  . [
    (identifier) @type.cod
    ((keyword) @_receiver_const . (identifier) @type.cod)
    (parenthesized
      "(" . (keyword)? @_receiver_const
      . (identifier) @type.cod
      . (keyword) @_receiver_in
      . (identifier) . ")")
  ]
  . (delimiter) @_member_dot
  . [(identifier) @function (symbol) @function (parenthesized)]
  . (parenthesized)
  (#any-of? @_member_declaration "func" "function" "proc" "procedure")
  (#eq? @_receiver_const "const")
  (#eq? @_receiver_in "in")
  (#eq? @_member_dot "."))

((delimiter) @_type_separator
  . (identifier) @type.cod
  (#eq? @_type_separator ":"))

((delimiter) @_type_separator
  . (keyword) @_type_modifier
  . (identifier) @type.cod
  (#eq? @_type_separator ":")
  (#eq? @_type_modifier "const"))

((keyword) @_type_operator
  . (identifier) @type.cod
  (#any-of? @_type_operator "make" "isa")
  (#not-eq? @type.cod "exception")
  (#not-eq? @type.cod "handle"))

((keyword) @_callable_declaration
  . (identifier)
  . (parenthesized)
  . (operator) @_result_arrow
  . (identifier) @type.cod
  (#any-of? @_callable_declaration "func" "function" "proc" "procedure")
  (#eq? @_result_arrow "->"))

((keyword) @_throws_clause
  . (identifier) @type.cod
  (#eq? @_throws_clause "throws"))

; The parentheses after a numeric representation name a unit only in a type.
; Check optional modifiers together with the required representation so the
; predicate also matches an unqualified type.
((delimiter) @_unit_colon
  . (keyword)? @_unit_head
  . (builtin_type) @_unit_head
  . (parenthesized (identifier) @type.cod)
  (#eq? @_unit_colon ":")
  (#any-of? @_unit_head "const" "Int64" "UInt8" "UInt32" "UInt64" "Float32" "Float64"))

((keyword) @_unit_callable
  . [(identifier) (parenthesized)]?
  . (parenthesized)
  . (operator) @_unit_arrow
  . (keyword)* @_unit_head
  . (builtin_type) @_unit_head
  . (parenthesized (identifier) @type.cod)
  (#any-of? @_unit_callable "func" "function" "proc" "procedure" "here")
  (#eq? @_unit_arrow "->")
  (#any-of? @_unit_head "const" "val" "ref" "ptr"
    "Int64" "UInt8" "UInt32" "UInt64" "Float32" "Float64"))

(source_file
  (keyword) @_using
  . (keyword) @_units
  . (string)
  . (block (identifier) @type.cod)
  (#eq? @_using "using")
  (#eq? @_units "units"))

; Contextual `by` after a simple written stop, or after an omitted stop in a
; bracketed selector or parenthesized pattern. Leave ordinary `by` names alone.
((operator) @_range
  . (comment)*
  . (operator)? @_stop_sign
  . [(identifier) (number) (parenthesized)]
  . (comment)*
  . (identifier) @keyword
  . (comment)*
  . [(identifier) (number) (parenthesized) (operator) @_step_sign]
  (#any-of? @_range ".." "..=")
  (#any-of? @_stop_sign "+" "-")
  (#eq? @keyword "by")
  (#any-of? @_step_sign "+" "-"))

([
  (bracketed
    (operator) @_open_range
    . (comment)*
    . (identifier) @keyword
    . (comment)*
    . [(identifier) (number) (parenthesized) (operator) @_open_step_sign])
  (parenthesized
    (operator) @_open_range
    . (comment)*
    . (identifier) @keyword
    . (comment)*
    . [(identifier) (keyword) @_step_binder])
 ]
  (#any-of? @_open_range ".." "..=")
  (#eq? @keyword "by")
  (#eq? @_step_binder "val")
  (#any-of? @_open_step_sign "+" "-"))

; More specific keyword patterns follow the generic fallback.
(keyword) @keyword

((keyword) @keyword.control.cod
  (#any-of? @keyword.control.cod
    "assert" "break" "case" "catch" "continue" "defer" "else" "fallback"
    "finally" "for" "if" "of" "return" "returning" "throw" "try"
    "yield"))

((keyword) @keyword.declaration.cod
  (#any-of? @keyword.declaration.cod
    "array" "data" "from" "func" "function" "global" "here" "in" "own" "owner"
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
