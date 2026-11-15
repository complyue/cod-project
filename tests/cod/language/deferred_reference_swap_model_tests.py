#!/usr/bin/env python3
"""Compare queued binding updates with a small independent execution model."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

PRELUDE = '''using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
val a<-1,b<-7,c<-9;ptr p=a,q=b,r=c;
'''
SCHEDULES = (
    (('swap','p','q'), ('swap','q','r')),
    (('swap','p','q'), ('rebind','q','r')),
    (('swap','p','q'), ('swap','p','r'), ('swap','q','r')),
)
runner.CASES = {}
for number, (schedule, q, r) in enumerate(product(SCHEDULES, ('null','a','b','c'), ('null','b','c'))):
    bindings = {'p':'a','q':'b','r':'c'}
    registered = []
    source = PRELUDE+'{{'
    for action, left, right in schedule:
        operation = '<=>' if action == 'swap' else '='
        source += 'defer intrinsic ('+operation+')('+left+','+right+');'
        registered.append((action,left,right if action == 'swap' else bindings[right]))
    source += 'q='+q+';r='+r+';}};'
    bindings['q'], bindings['r'] = q, r
    for action, left, right in reversed(registered):
        if action == 'swap': bindings[left], bindings[right] = bindings[right], bindings[left]
        else: bindings[left] = right
    expected = ''
    for name in ('p','q','r'):
        selected = bindings[name]
        output = 'behave @print('+name+');'
        if selected == 'null':
            source += 'try{'+output+'}catch(e:const NullPointerException){behave @print(-1);};'
            expected += '-1\n'
        else:
            source += output
            expected += {'a':'1\n','b':'7\n','c':'9\n'}[selected]
    runner.CASES['queue_'+str(number)] = (source,0,expected,'')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
