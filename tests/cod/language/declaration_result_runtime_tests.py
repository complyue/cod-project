#!/usr/bin/env python3
"""Declared complete results preserve image ownership across native failure paths."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
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
 assert signature + '{' in runner.SOURCE
 runner.SOURCE = runner.SOURCE.replace(signature + '{', signature + ';' + kind + ' ' + head + '{')
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
