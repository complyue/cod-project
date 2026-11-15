#!/usr/bin/env python3
"""Symbolic owner coverage preserves complete captures and prepared borrows."""
from pathlib import Path
import re
import symbolic_family_runtime_tests as fixture

runner = fixture.runner
parameters = ','.join(f'ref p{i}:const i64 in O{i}' for i in range(12))
roles = ','.join(f'owner O{i}' for i in range(12))
arguments = ','.join(['slot'] * 12)
runner.SOURCE = runner.SOURCE.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;', 'symbol @x:u64,@y:u64;val slot<-1;')
runner.SOURCE = runner.SOURCE.replace('proc @f(n:u64)->u64{', f'proc @f(n:u64,{parameters})->u64 where {{{roles}}}{{')
runner.SOURCE = runner.SOURCE.replace('proc @f(n:T)->u64 where T{', f'proc @f(n:T,{parameters})->u64 where {{T,{roles}}}{{')
runner.SOURCE = re.sub(r'@f\((1u8|1u64)\)', lambda m: f'@f({m[1]},{arguments})', runner.SOURCE)
runner.EXPECTED_SHADOWS = ['x']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
