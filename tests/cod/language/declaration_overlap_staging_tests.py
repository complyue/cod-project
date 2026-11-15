#!/usr/bin/env python3
"""Overlapping declarations preserve native identity and shared recursive bodies."""
from pathlib import Path
import operator_declaration_staging_tests as runner

IO = runner.IO
runner.CASE_SHADOWS = {}
runner.CASES = (
    ('owners', 'func choose(ref a:const i64 in O,ref b:const i64 in O)->ref const i64 in O where {owner O};', '', '',
     IO+'func choose(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in B where {owner A,owner B}{return b;}val outer<-3;{{val inner<-7;behave @print(choose(outer,inner));}};behave @print(choose(outer,outer));', '7\n3\n'),
    ('access', 'func f(ref a:i64 in O)->i64 where {owner O};', '->i64', ' where {owner O}{',
     IO+'func f(ref a:const i64 in O)->i64 where {owner O}{return a;}const val slot<-7;behave @print(f(slot));', '7\n'),
    ('borrowed_cleanup', 'func choose(ref a:i64 in O,ref b:i64 in O)->ref i64 in O where {owner O};', '', '',
     IO+'func choose(ref a:i64 in A,ref b:i64 in B)->ref i64 in B where {owner A,owner B}{defer{b<-9;};return b;}val outer<-3;{{val inner<-7;behave @print(choose(outer,inner));}};', '9\n'),
)
runner.RECURSIVE = IO+'''
func (+)(ref a:const T in O,ref b:const T in O)->const T where {T,owner O};
func (+)(ref a:const U in A,ref b:const U in B) where {U,owner A,owner B}{return b;}
func relay(x:T,n:u64)->const T where T{if(n==0u64){return x+x;}else{return relay(x,n-1u64);};}
behave @print(relay(7u64,30u64));behave @print(relay(true,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
