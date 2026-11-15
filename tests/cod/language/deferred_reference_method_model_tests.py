#!/usr/bin/env python3
"""Compare saved slot methods with an independent deferred binding interpreter."""
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
for number, (schedule, alias, late, count) in enumerate(product(SCHEDULES, (False,True), ('z','null'), (1,2))):
    bindings = {'p':'x','q':'y'}
    parameters = {'a':'p','b':'p' if alias else 'q'}
    body = ''.join(left+'=null;' if action=='clear' else left+'='+right+';' if action=='set'
                   else 'intrinsic (<=>)('+left+','+right+');' for action,left,right in schedule)
    source = PRELUDE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{'+body+'}'
    source += 'val x<-1,y<-7,z<-9;ptr p=x,q=y;{{'
    source += ('defer (<=>)(p,'+('p' if alias else 'q')+');')*count+'q='+late+';}};'
    bindings['q']=late
    for _ in range(count):
        for action,left,right in schedule:
            a=parameters[left]
            if action=='clear': bindings[a]='null'
            elif action=='set': bindings[a]=bindings[parameters[right]]
            else:
                b=parameters[right];bindings[a],bindings[b]=bindings[b],bindings[a]
    expected=''
    for name in ('p','q'):
        source+='try{behave @print('+name+');}catch(e:const NullPointerException){behave @print(-1);};'
        expected += {'x':'1\n','y':'7\n','z':'9\n','null':'-1\n'}[bindings[name]]
    runner.CASES['queue_'+str(number)]=(source,0,expected,'')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
