#!/usr/bin/env python3
"""Result adaptation adds no semantic methods or runtime registration."""
from pathlib import Path
import symbolic_family_staging_tests as runner

runner.SOURCE = runner.SOURCE.replace('proc @f(x:u64)->u64{return 1u64;}', 'proc @f(x:u64)->u32{return 1u32;}')
runner.SOURCE = runner.SOURCE.replace('proc @f(x:T)->u64 where T{return 7u64;}', 'proc @f(x:T)->u8 where T{return 7u8;}')
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
