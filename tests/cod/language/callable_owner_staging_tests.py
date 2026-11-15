#!/usr/bin/env python3
"""Symbolic owner roles do not multiply semantic methods or native bodies."""
from pathlib import Path
import re
import symbolic_family_staging_tests as runner

parameters = ','.join(f'ref p{i}:const i64 in O{i}' for i in range(12))
roles = ','.join(f'owner O{i}' for i in range(12))
arguments = ','.join(['slot'] * 12)
runner.SOURCE = runner.SOURCE.replace('symbol @f:proc(u64)->u64;', 'val slot<-1;')
runner.SOURCE = runner.SOURCE.replace('proc @f(x:u64)->u64{', f'proc @f(x:u64,{parameters})->u64 where {{{roles}}}{{')
runner.SOURCE = runner.SOURCE.replace('proc @f(x:T)->u64 where T{', f'proc @f(x:T,{parameters})->u64 where {{T,{roles}}}{{')
runner.SOURCE = re.sub(r'@f\((1u8|1u64)\)', lambda m: f'@f({m[1]},{arguments})', runner.SOURCE)
runner.UNUSED_INPUT_TYPES = ('u32',) + ('i64',) * 12
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
