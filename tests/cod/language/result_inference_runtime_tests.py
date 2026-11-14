#!/usr/bin/env python3
"""Inferred complete results retain native preparation, failure and cleanup contracts."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
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
 assert signature in runner.SOURCE
 runner.SOURCE = runner.SOURCE.replace(signature, head)
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
