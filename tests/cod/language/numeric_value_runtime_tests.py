#!/usr/bin/env python3
"""Prepared numeric values retain complete-image cleanup and allocation-free stable calls."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
func wide_real(x:f64)->f64{return x;}
func widened(f:func(f64)->f64,x:u32)->f64{return f(x);}
''' + FIXTURE.source)
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('val total<-0u64;', 'val total<-0u64;val real_total<-0f64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('for(i in 0u64..10000u64){total<-total+f(i);};',
    'for(i in 0u32..10000u32){total<-total+f(i);real_total<-real_total+widened(wide_real,i);};'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('observe(9011u64);observe(total);',
    'if(real_total==49995000f64){}else{observe(9999u64);};observe(9011u64);observe(total);'))
for original, widened in (
    ('f(2u64)', 'f(2u32)'), ('f(3u64)', 'f(3u32)'),
    ('f({{observe(9999u64);return 4u64;}})', 'f({{observe(9999u64);return 4u32;}})'),
    ('defer f(5u64)', 'defer f(5u8)'), ('defer f(99u64)', 'defer f(99u8)'),
    ('replaced()(6u64)', 'replaced()(6u8)'), ('retained()(7u64)', 'retained()(7u8)'),
    ('copy_recovery()(8u64)', 'copy_recovery()(8u8)'),
):
    assert original in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(original, widened))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
