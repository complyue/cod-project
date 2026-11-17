#!/usr/bin/env python3
"""Callable holder aliases preserve complete values and exact image cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    '{{func f=wrap(wrap(base));', '{{func holder=wrap(wrap(base));val var f->holder;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'func f=base;for', 'func holder=base;var f->holder;for'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'func f=wrap(base);observe(9020u64);', 'func holder=wrap(base);var f->holder;observe(9020u64);'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'proc f=wrapped(one);', 'proc holder=wrapped(one);var f->holder;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('return f;\n}', 'var a->f;return a;\n}'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
