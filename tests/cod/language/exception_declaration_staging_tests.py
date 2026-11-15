#!/usr/bin/env python3
"""Symbolic exception overlap preserves native contracts and shared recursive bodies."""
from pathlib import Path
import operator_declaration_staging_tests as runner

TYPES = 'using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
ERROR = 'func error()->const NullPointerException{ptr absent:u64=null in here;try{absent;{}}catch(caught:const NullPointerException){return caught;};return error();}'
runner.CASE_SHADOWS = {}
runner.CASES = (
    ('independent', 'func f(a:T,b:T)->bool where T;', '->bool', '{',
     runner.IO+TYPES+'func f(a:Exception,b:Exception)->bool{return true;}'+ERROR+'behave @print(f(error(),error()));', 'true\n'),
    ('narrow', 'func f(input:NullPointerException)->u64;', '->u64', '{',
     runner.IO+TYPES+'func f(input:Exception)->u64{return input.category;}'+ERROR+'behave @print(f(error()));', '4\n'),
    ('borrowed', 'func f(a:T,b:T,ref slot:i64 in O)->ref i64 in O where {T,owner O};', '', '',
     runner.IO+TYPES+'func f(a:Exception,b:Exception,ref slot:i64 in O)->ref i64 in O where {owner O}{defer{slot<-7;};return slot;}'+ERROR+'val target<-3;behave @print(f(error(),error(),target));', '7\n'),
)
runner.RECURSIVE = runner.IO+TYPES+'''
func (+)(input:NullPointerException,x:T)->const T where T;
func (+)(input:Exception,x:U)->const U where U{return x;}
'''+ERROR+'''
func relay(x:T,n:u64)->const T where T{if(n==0u64){return (+)(error(),x);}else{return relay(x,n-1u64);};}
behave @print(relay(7u64,30u64));behave @print(relay(true,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
