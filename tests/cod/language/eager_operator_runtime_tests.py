#!/usr/bin/env python3
"""Operator-selected callable factories retain ordinary image and cleanup contracts."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('func wrap(', 'func (+)(').replace('wrap(', '(+)('))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
