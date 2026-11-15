#!/usr/bin/env python3
"""Access-constrained family coverage preserves shared native methods."""
from pathlib import Path
import symbolic_family_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace('@f:proc(u64)', '@f:proc(const u64)')
runner.SOURCE = runner.SOURCE.replace('proc @f(x:u64)', 'proc @f(x:const u64)').replace('proc @f(x:T)', 'proc @f(x:const T)')
runner.SOURCE += 'proc @f(x:T)->u64 where T{return 99u64;}\n'
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
