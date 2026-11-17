#!/usr/bin/env python3
"""Declared complete results preserve image ownership across native failure paths."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for kind, head, result in (
 ('proc', 'observe(x:u64)', 'void'), ('func', 'base(x:u64)', 'const u64'),
 ('func', 'other(x:u64)', 'const u64'), ('func', 'wrap(f:func(u64)->const u64)', 'func(u64)->const u64'),
 ('func', 'apply(f:func(u64)->const u64,x:u64)', 'const u64'), ('func', 'missing()', 'const u64'),
 ('proc', 'one(x:u64)', 'void'), ('proc', 'two(x:u64)', 'void'),
 ('proc', 'wrapped(p:proc(u64)->void)', 'proc(u64)->void'),
 ('func', 'replaced()', 'func(u64)->const u64'), ('proc', 'copy_recovery()', 'func(u64)->const u64'),
 ('func', 'retained()', 'func(u64)->const u64'),
):
 signature = kind + ' ' + head + '->' + result
 assert signature + '{' in FIXTURE.source
 FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(signature + '{', signature + ';' + kind + ' ' + head + '{'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
 raise SystemExit(run_native(FIXTURE))
