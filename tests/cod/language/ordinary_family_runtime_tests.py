#!/usr/bin/env python3
"""Ordinary family materialization retains complete images through failure."""
from pathlib import Path
from native_test_support import run_native
import symbolic_family_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('symbol @x:u64,@y:u64,@f:proc(u64)->u64;',
    'symbol @x:u64,@y:u64,@f:proc(u64)->u64,@bad:proc(u64)->u64;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc @f', 'proc selected').replace('effect @f;', 'effect @f=selected;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc @bad', 'proc failing').replace('effect @bad;', 'effect @bad=failing;'))
FIXTURE = FIXTURE.updated(expected_shadows=['x'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
