#!/usr/bin/env python3
"""Exiting alias selectors retain complete-image cleanup and perform no copy."""
from pathlib import Path
from native_test_support import run_native
import storage_completion_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
callable_declaration = 'func unused:func(u64)->const u64='
assert FIXTURE.source.count(callable_declaration) == 2
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(callable_declaration, 'var unused->'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
