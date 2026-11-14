#!/usr/bin/env python3
"""Broader declarations add neither native owner context nor executable methods."""
from pathlib import Path
import operator_declaration_staging_tests as runner

IO = runner.IO
runner.CASES = (
    ('owners', 'func choose(ref x:i64 in A,ref y:i64 in B)->ref i64 in B where {owner A,owner B};',
     '->ref i64 in O', ' where {owner O}{',
     IO+'func choose(ref x:i64 in O,ref y:i64 in O)->ref i64 in O where {owner O}{return y;}val x<-1,y<-2;choose(x,y)<-7;behave @print(y);', '7\n'),
    ('value_access', 'func f(x:const u64)->u64;', '->u64', '{',
     IO+'func f(x:u64)->u64{x<-x+1u64;return x;}const val x<-2u64;behave @print(f(x));', '3\n'),
    ('borrowed_access', 'func f(ref x:const i64 in A)->ref const i64 in A where {owner A};',
     '->ref const i64 in O', ' where {owner O}{',
     IO+'func f(ref x:i64 in O)->ref const i64 in O where {owner O}{defer{x<-7;};return x;}val x<-1;behave @print(f(x));', '7\n'),
    ('disjoint_swap', 'proc (<->)(x:const i64 in A,y:const i64 in B)->void where {owner A,owner B};', '', '',
     IO+'val x<-1,y<-2;x<->y;behave @print(x);', '2\n'),
)
runner.RECURSIVE = IO+'''
func (+)(ref x:const T in A,ref y:const T in B)->ref const T in B where {T,owner A,owner B};
func (+)(ref x:U in O,ref y:U in O) where {U,owner O}{return y;}
func relay(x:T,n:u64)->const T where T{if(n==0u64){return x+x;}else{return relay(x,n-1u64);};}
behave @print(relay(7u64,30u64));behave @print(relay(true,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
