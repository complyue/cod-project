#!/usr/bin/env python3
"""Prepared numeric values retain complete-image cleanup and allocation-free stable calls."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
func wide_real(x:f64)->f64{return x;}
func widened(f:func(f64)->f64,x:u32)->f64{return f(x);}
''' + runner.SOURCE
runner.SOURCE = runner.SOURCE.replace('val total<-0u64;', 'val total<-0u64;val real_total<-0f64;')
runner.SOURCE = runner.SOURCE.replace('for(i in 0u64..10000u64){total<-total+f(i);};',
    'for(i in 0u32..10000u32){total<-total+f(i);real_total<-real_total+widened(wide_real,i);};')
runner.SOURCE = runner.SOURCE.replace('observe(9011u64);observe(total);',
    'if(real_total==49995000f64){}else{observe(9999u64);};observe(9011u64);observe(total);')
for original, widened in (
    ('f(2u64)', 'f(2u32)'), ('f(3u64)', 'f(3u32)'),
    ('f({{observe(9999u64);4u64}})', 'f({{observe(9999u64);4u32}})'),
    ('defer f(5u64)', 'defer f(5u8)'), ('defer f(99u64)', 'defer f(99u8)'),
    ('replaced()(6u64)', 'replaced()(6u8)'), ('retained()(7u64)', 'retained()(7u8)'),
    ('copy_recovery()(8u64)', 'copy_recovery()(8u8)'),
):
    assert original in runner.SOURCE
    runner.SOURCE = runner.SOURCE.replace(original, widened)
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
