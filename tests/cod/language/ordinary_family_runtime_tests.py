#!/usr/bin/env python3
"""Ordinary family materialization retains complete images through failure."""
from pathlib import Path
import symbolic_family_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;',
    'symbol @x:u64,@y:u64,@f:proc(u64)->u64,@bad:proc(u64)->u64;')
runner.SOURCE = runner.SOURCE.replace('proc @f', 'proc selected').replace('effect @f;', 'effect @f=selected;')
runner.SOURCE = runner.SOURCE.replace('proc @bad', 'proc failing').replace('effect @bad;', 'effect @bad=failing;')
runner.EXPECTED_SHADOWS = ['x']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
