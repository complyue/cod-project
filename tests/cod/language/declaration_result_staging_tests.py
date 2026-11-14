#!/usr/bin/env python3
"""Signature declarations create no executable methods or extra native bodies."""
from pathlib import Path
import generic_callable_staging_tests as runner

for head, result, binders in (
 ('first(x:i64)', 'const i64', ''), ('second(x:i64)', 'const i64', ''),
 ('truth(x:bool)', 'const bool', ''), ('wrap(f:func(i64)->const i64)', 'func(i64)->const i64', ''),
 ('relay(f:func(T)->const T,x:T,n:u64)', 'const T', ' where T'),
 ('empty()', 'const void', ''), ('number()', 'const i64', ''),
 ('finish(f:func()->const T)', 'const T', ' where T'),
):
 signature = 'func ' + head + '->' + result + binders
 assert signature + '{' in runner.SOURCE
 runner.SOURCE = runner.SOURCE.replace(signature + '{', signature + ';func ' + head + binders + '{')
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
