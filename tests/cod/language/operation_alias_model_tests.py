#!/usr/bin/env python3
"""Compare renamed destination operations with independent slot/payload state."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
runner.CASES = {}
for number, (kind, deferred, same_slot, alternate) in enumerate(product(
        ('reference', 'bits', 'ordinary'), (False, True), (False, True), (False, True))):
    targets = {'p':'x', 'q':'y'}
    values = {'x':1, 'y':7, 'z':9}
    second = 'p' if same_slot else 'q'
    operation = '<->' if kind == 'bits' else '<=>'
    definition = 'helper' if kind == 'ordinary' else '('+operation+')'
    method = 'func '+definition+'(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{intrinsic ('+operation+')(a,b);}'
    # The left selection precedes a right operand which can rebind its slot.
    left_target = targets['p']
    if alternate:
        targets['p'] = 'z'
    right_target = targets[second]
    body = 'val x<-1,y<-7,z<-9;ptr p=x,q=y;'
    call = 'exchange(p,'+('{p=z;'+second+'}' if alternate else second)+');'
    if deferred:
        body += '{{defer '+call+'q=z:void;}};'
        targets['q'] = 'z'
    else:
        body += call
    if kind == 'reference':
        targets['p'], targets[second] = targets[second], targets['p']
    elif kind == 'bits':
        values[left_target], values[right_target] = values[right_target], values[left_target]
    # An ordinary helper swaps its own local parameter bindings only.
    expected = ''
    for name in ('x','y','z','p','q'):
        body += 'behave @print('+name+');'
        expected += str(values[targets.get(name,name)])+'\n'
    for slot in ('p','q'):
        for target in ('x','y','z'):
            body += 'behave @print('+slot+' is '+target+');'
            expected += str(targets[slot] == target).lower()+'\n'
    runner.CASES['state_'+str(number)] = ({'ops.cod':method+'export '+definition+';',
        'main.cod':IO+'using "./ops.cod" '+definition+' as exchange;'+body},0,expected,'')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
