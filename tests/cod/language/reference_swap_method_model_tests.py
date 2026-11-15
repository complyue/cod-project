#!/usr/bin/env python3
"""Compare caller-slot method effects with an independent binding interpreter."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

PRELUDE = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SCHEDULES = (
    (('swap','a','b'),),
    (('set','a','b'),),
    (('clear','a',''),('swap','a','b')),
    (('swap','a','b'),('clear','b','')),
    (('set','a','b'),('clear','b','')),
    (('swap','a','b'),('set','b','a'),('swap','a','b')),
)
runner.CASES = {}
for number, (schedule, alias, nullable) in enumerate(product(SCHEDULES, (False,True), (False,True))):
    bindings = {'p':'x','q':'null' if nullable else 'y'}
    parameters = {'a':'p','b':'p' if alias else 'q'}
    body = ''
    for action, left, right in schedule:
        if action == 'clear':
            body += left+'=null;'
            bindings[parameters[left]] = 'null'
        elif action == 'set':
            body += left+'='+right+';'
            bindings[parameters[left]] = bindings[parameters[right]]
        else:
            body += 'intrinsic (<=>)('+left+','+right+');'
            a,b = parameters[left],parameters[right]
            bindings[a],bindings[b] = bindings[b],bindings[a]
    source = PRELUDE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{'+body+'}'
    source += 'val x<-1,y<-7;ptr p=x,q='+('null in here' if nullable else 'y')+';'
    # A standalone null initializer needs a target type.
    if nullable: source = source.replace('q=null in here','q:i64=null in here')
    source += 'p<=>'+('p' if alias else 'q')+';'
    expected = ''
    for name in ('p','q'):
        source += 'try{behave @print('+name+');}catch(e:const NullPointerException){behave @print(-1);};'
        expected += {'x':'1\n','y':'7\n','null':'-1\n'}[bindings[name]]
    runner.CASES['slots_'+str(number)] = (source,0,expected,'')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
