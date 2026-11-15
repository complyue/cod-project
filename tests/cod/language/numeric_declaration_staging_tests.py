#!/usr/bin/env python3
"""Narrow input declarations preserve native results and shared generic bodies."""
from pathlib import Path
import operator_declaration_staging_tests as runner

IO = runner.IO
runner.CASE_SHADOWS = {}
runner.CASES = (
    ('scalar', 'func f(x:u32)->f64;', '->f64', '{',
     IO+'func f(x:f64)->f64{return x;}behave @print(f(16777217u32));behave @print(f(7f32));', '16777217\n7\n'),
    ('borrowed', 'func choose(x:u32,ref slot:i64 in O)->ref i64 in O where {owner O};', '', '',
     IO+'func choose(x:f64,ref slot:i64 in O)->ref i64 in O where {owner O}{defer{slot<-9;};return slot;}val target<-3;behave @print(choose(1u32,target));', '9\n'),
    ('complete', 'func make_value(x:u32)->func(u64)->u64;', '', '',
     IO+'func base(y:u64)->u64{return y+7u64;}func make_value(x:f64)->func(u64)->u64{func saved=base;return func[saved](y:u64)->u64{return saved(y);};}func selected=make_value(1u32);behave @print(selected(2u64));', '9\n'),
)
runner.RECURSIVE = IO+'''
func (+)(x:u32,y:T)->const T where T;
func (+)(x:f64,y:U)->const U where U{return y;}
func relay(x:T,n:u64)->const T where T{if(n==0u64){return (+)(1u32,x);}else{return relay(x,n-1u64);};}
behave @print(relay(7u64,30u64));behave @print(relay(true,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
