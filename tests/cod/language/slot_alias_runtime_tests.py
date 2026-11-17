#!/usr/bin/env python3
"""Shared slots retain native assignment order and faulted cleanup behavior."""
from pathlib import Path
from native_test_support import run_native
import mutable_reference_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('ptr p=a;', 'ptr original=a;ptr var p->original;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer{observe(p);', 'defer{observe(original);'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('p=b;', 'original=b;'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
