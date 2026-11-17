#!/usr/bin/env python3
"""View-constrained families retain complete captures and cleanup accounting."""
from pathlib import Path
from native_test_support import run_native
import symbolic_family_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;',
    'symbol @x:u64,@y:u64,@unused:u64,@f:proc(const u64)->u64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc @f(n:u64)', 'proc @f(n:const u64)').replace('proc @f(n:T)', 'proc @f(n:const T)'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc @f(n:u64)->u64{return @unused;}
proc @f(n:T)->u64 where T{return @unused;}
''')
FIXTURE = FIXTURE.updated(expected_shadows=['x'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
