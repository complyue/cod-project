#!/usr/bin/env python3
"""Compare canonical eager operations under renamed tokens with scalar arithmetic."""
import operator
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OPERATIONS = {'+':operator.add,'-':operator.sub,'*':operator.mul,'/':operator.truediv,
              '%':operator.mod,'==':operator.eq,'!=':operator.ne,'<':operator.lt,
              '<=':operator.le,'>':operator.gt,'>=':operator.ge}
runner.CASES = {}
for number, (token, operation) in enumerate(OPERATIONS.items()):
    for kind in ('i64','u8','u32','u64','f32','f64'):
        floating = kind.startswith('f')
        if floating and token == '%': continue
        suffix = '' if kind == 'i64' else kind
        expected = operation(7,2)
        if token == '/' and not floating: expected = 7//2
        literal = str(expected).lower() if isinstance(expected,bool) else str(expected)+suffix
        # A different operator spelling must not change the selected implementation.
        alias = '(*)' if token != '*' else '(+)'
        source = {'ops.cod':'func ('+token+')(a:bool,b:bool)->bool{return a;}export ('+token+');',
                  'main.cod':IO+'using "./ops.cod" ('+token+') as '+alias+';'
                  'behave @print('+alias+'(7'+suffix+',2'+suffix+')=='+literal+');'}
        runner.CASES['operation_'+str(number)+'_'+kind] = (source,0,'true\n','')
for left,right,result in (('1u8','2u64','3u64'),('1u32','2f32','3f64'),('1f32','2f64','3f64')):
    runner.CASES['widen_'+left] = ({'ops.cod':'func (+)(a:bool,b:bool)->bool{return a;}export (+);',
        'main.cod':IO+'using "./ops.cod" (+) as add;behave @print(add('+left+','+right+')=='+result+');'},0,'true\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
