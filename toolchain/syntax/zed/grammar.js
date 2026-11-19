// An editor grammar, not a second CoD compiler. Keep grouping and lexical
// categories, but leave declaration validity and operator roles to the compiler.
module.exports = grammar({
  name: 'cod',
  word: $ => $.identifier,
  extras: $ => [/\s/, $.comment],
  rules: {
    source_file: $ => repeat($._item),
    _item: $ => choice(
      $.block, $.parenthesized, $.bracketed, $.identifier, $.symbol,
      $.string, $.number, $.boolean, $.null_literal, $.builtin_type,
      $.keyword, $.builtin_function, $.operator, $.pipeline_operator,
      $.reserved_operator,
      $.delimiter,
    ),

    block: $ => seq('{', repeat($._item), '}'),
    parenthesized: $ => seq('(', repeat($._item), ')'),
    bracketed: $ => seq('[', repeat($._item), ']'),

    identifier: _ => /[a-zA-Z_][a-zA-Z_0-9]*/,
    symbol: _ => /@[a-zA-Z_][a-zA-Z_0-9]*/,
    comment: _ => /#[^\r\n]*/,
    // An incomplete path stays highlighted up to the end of the line. Unlike
    // the compiler, the editor must tolerate incomplete input during an edit.
    string: _ => token(/"([^"\\\r\n]|\\[^\r\n])*"?/),
    number: _ => token(choice(
      /[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?f(32|64)/,
      /[0-9]+(\.[0-9]+([eE][+-]?[0-9]+)?|[eE][+-]?[0-9]+)/,
      /[0-9]+(u(8|32|64))?/,
    )),
    boolean: _ => choice('true', 'false'),
    null_literal: _ => 'null',
    builtin_type: _ => choice('bool', 'void', 'i64', 'u8', 'u32', 'u64', 'f32', 'f64'),
    keyword: _ => choice(
      'array', 'as', 'assert', 'behave', 'break', 'capacity', 'case', 'catch',
      'const', 'continue', 'conversion', 'data', 'defer', 'effect', 'else',
      'export', 'fallback', 'finally', 'for', 'from', 'func', 'function',
      'global', 'here', 'if', 'import', 'in', 'intrinsic', 'is', 'isa', 'make',
      'mixin', 'not', 'of', 'owned', 'owner', 'perform', 'proc', 'procedure',
      'ptr', 'ref', 'region', 'return', 'returning', 'symbol', 'throw',
      'throws', 'touch', 'try', 'type', 'typeof', 'unit', 'units', 'using',
      'val', 'var', 'where', 'with', 'yield', 'yields',
    ),
    builtin_function: _ => choice('cause_at', 'deref', 'scalar'),
    pipeline_operator: _ => choice('|>', '<|', '!>', '<!', '$>', '<$'),
    reserved_operator: _ => '//',
    operator: _ => choice(
      '<=>', '<->', '...', '->', '<-', '<:', '=>', '..', '==', '!=', '<=',
      '>=', '&&', '||', '+', '-', '*', '/', '%', '<', '>', '=', '!', '|',
    ),
    delimiter: _ => choice(';', ',', ':', '.'),
  },
});
