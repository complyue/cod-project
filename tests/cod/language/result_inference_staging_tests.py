#!/usr/bin/env python3
"""Inferred generic and captured results preserve finite shared native coverage."""
from pathlib import Path
import generic_callable_staging_tests as runner

for head, result in (
 ('first(x:i64)', 'const i64'), ('second(x:i64)', 'const i64'), ('truth(x:bool)', 'const bool'),
 ('wrap(f:func(i64)->const i64)', 'func(i64)->const i64'), ('[f](x:i64)', 'const i64'),
 ('relay(f:func(T)->const T,x:T,n:u64)', 'const T'), ('empty()', 'const void'), ('number()', 'const i64'),
):
 signature = head + '->' + result
 assert signature in runner.SOURCE
 runner.SOURCE = runner.SOURCE.replace(signature, head)
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
