#!/usr/bin/env python3
"""Adapted family results preserve complete captures and failure cleanup."""
from pathlib import Path
import re
import symbolic_family_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;',
    'symbol @x:u32,@y:u32,@f:proc(u64)->u64,@bad:proc(u64)->u64;')
runner.SOURCE = runner.SOURCE.replace('proc @f(n:u64)->u64', 'proc @f(n:u64)->u32')
runner.SOURCE = runner.SOURCE.replace('proc @f(n:T)->u64', 'proc @f(n:T)->u32')
runner.SOURCE = runner.SOURCE.replace('proc read()->u64', 'proc read()->u32').replace('proc @bad(n:u64)->u64', 'proc @bad(n:u64)->u32')
runner.SOURCE = re.sub(r'(effect @[xy]<-\d+)u64', r'\1u32', runner.SOURCE)
runner.SOURCE = runner.SOURCE.replace('0u64..20u64', '0u32..20u32').replace('effect @y<-i+1u64', 'effect @y<-i+1u32')
runner.EXPECTED_SHADOWS = ['x']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
