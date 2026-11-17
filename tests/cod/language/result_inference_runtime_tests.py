#!/usr/bin/env python3
"""Inferred complete results retain native preparation, failure and cleanup contracts."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for head, result in (
 ('observe(x:u64)', 'void'), ('base(x:u64)', 'const u64'), ('other(x:u64)', 'const u64'),
 ('wrap(f:func(u64)->const u64)', 'func(u64)->const u64'),
 ('apply(f:func(u64)->const u64,x:u64)', 'const u64'),
 ('one(x:u64)', 'void'), ('two(x:u64)', 'void'),
 ('wrapped(p:proc(u64)->void)', 'proc(u64)->void'),
 ('replaced()', 'func(u64)->const u64'), ('copy_recovery()', 'func(u64)->const u64'),
 ('retained()', 'func(u64)->const u64'), ('[f](x:u64)', 'const u64'), ('[p](x:u64)', 'void'),
):
 signature = head + '->' + result
 assert signature in FIXTURE.source
 FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(signature, head))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
 raise SystemExit(run_native(FIXTURE))
