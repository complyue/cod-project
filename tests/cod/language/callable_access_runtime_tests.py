#!/usr/bin/env python3
"""View-constrained families retain complete captures and cleanup accounting."""
from pathlib import Path
import symbolic_family_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;',
    'symbol @x:u64,@y:u64,@unused:u64,@f:proc(const u64)->u64;')
runner.SOURCE = runner.SOURCE.replace('proc @f(n:u64)', 'proc @f(n:const u64)').replace('proc @f(n:T)', 'proc @f(n:const T)')
runner.SOURCE += '''
proc @f(n:u64)->u64{return @unused;}
proc @f(n:T)->u64 where T{return @unused;}
'''
runner.EXPECTED_SHADOWS = ['x']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
