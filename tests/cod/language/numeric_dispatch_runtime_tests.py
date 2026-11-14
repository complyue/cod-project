#!/usr/bin/env python3
"""Widened named calls preserve ordered preparation and complete cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
func real(x:f64)->f64{return x;}
func forward(ref f:func(u64)->const u64 in O,x:u64)->u64 where {owner O}{return f(x);}
proc invoke(p:proc(u64)->void,x:u64)->void{p(x);}
''' + runner.SOURCE
for original, replacement in (
    ('val total<-0u64;', 'val total<-0u64;val real_total<-0f64;'),
    ('for(i in 0u64..10000u64){total<-total+f(i);};',
     'for(i in 0u32..10000u32){total<-total+forward(f,i);real_total<-real_total+real(i);};'),
    ('observe(9011u64);observe(total);',
     'if(real_total==49995000f64){}else{observe(9999u64);};observe(9011u64);observe(total);'),
    ('func missing()->const u64{}', 'func missing()->const u32{}'),
    ('f({{observe(9999u64);4u64}})', 'apply(f,{{observe(9999u64);4u8}})'),
    ('defer f(5u64)', 'defer invoke(f,5u8)'),
    ('defer f(99u64)', 'defer invoke(f,99u8)'),
):
    assert original in runner.SOURCE
    runner.SOURCE = runner.SOURCE.replace(original,replacement)
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
