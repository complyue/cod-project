#!/usr/bin/env python3
"""Declared operator results preserve complete callable images and failed cleanup paths."""
from pathlib import Path
from native_test_support import run_native
import eager_operator_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
head = 'func (+)(f:func(u64)->const u64)'
result = '->func(u64)->const u64'
assert head + result + '{' in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(head + result + '{', head + result + ';' + head + '{'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
