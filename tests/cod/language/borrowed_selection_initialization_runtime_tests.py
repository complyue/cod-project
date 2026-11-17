#!/usr/bin/env python3
"""Borrowed aliases retain selected writes and exact cleanup under allocation failure."""
from pathlib import Path
from native_test_support import run_native
import selected_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
prefix,_,_=FIXTURE.source.partition('verify(false,false,false,false);')
original='proc verify(first:bool,second:bool,fail:bool,conditional:bool)->void{\n val a<-make local u64,b<-make local u64;'
borrowed='proc verify(first:bool,second:bool,fail:bool,conditional:bool,ptr a:u64 in O,ptr b:u64 in O)->void where {owner O}{'
assert original in prefix
FIXTURE = FIXTURE.updated(source=prefix.replace(original,borrowed).replace('null in here','null in O'))
FIXTURE = FIXTURE.updated(expected_values=[])
for alias in (False,True):
    for conditional in (False,True):
        for fail in (False,True):
            for first,second in ((False,False),(False,True),(True,False),(True,True)):
                args=','.join(str(value).lower() for value in (first,second,fail,conditional))
                FIXTURE = FIXTURE.updated(source=FIXTURE.source + '{{val left<-make local u64,right<-make local u64;verify('+args+',left,'+('left' if alias else 'right')+');}};\n')
                FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values, '7' if alias or first==second else '0'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==24 && peak==2 && failures==8','created==48 && peak==2 && failures==16'))
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
