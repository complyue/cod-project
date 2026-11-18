#!/usr/bin/env python3
"""Declared place aliases of callable holders preserve values and cleanup."""
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
    'proc f=wrapped(one);\n  {{', 'proc holder=wrapped(one);\n  {{var f->holder;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'f<-two;}};', 'f<-two;holder(6u64);}};\n  holder(7u64);'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('return f;\n}', 'var a->f;return a;\n}'))
# The inner holder changes, the prepared action retains its target, and the outer holder keeps its value.
FIXTURE = FIXTURE.updated(expected_values=['3','12','50795000','3','4','16','5','7','17','8','9'], expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
