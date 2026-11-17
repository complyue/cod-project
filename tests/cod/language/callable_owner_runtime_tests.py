#!/usr/bin/env python3
"""Symbolic owner coverage preserves complete captures and prepared borrows."""
from pathlib import Path
import re
from native_test_support import run_native
import symbolic_family_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
parameters = ','.join(f'ref p{i}:const i64 in O{i}' for i in range(12))
roles = ','.join(f'owner O{i}' for i in range(12))
arguments = ','.join(['slot'] * 12)
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;', 'symbol @x:u64,@y:u64;val slot<-1;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc @f(n:u64)->u64{', f'proc @f(n:u64,{parameters})->u64 where {{{roles}}}{{'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc @f(n:T)->u64 where T{', f'proc @f(n:T,{parameters})->u64 where {{T,{roles}}}{{'))
FIXTURE = FIXTURE.updated(source=re.sub(r'@f\((1u8|1u64)\)', lambda m: f'@f({m[1]},{arguments})', FIXTURE.source))
FIXTURE = FIXTURE.updated(expected_shadows=['x'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
