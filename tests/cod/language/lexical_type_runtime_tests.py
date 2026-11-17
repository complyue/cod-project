#!/usr/bin/env python3
"""Retained type context shares complete image ownership across void/value calls."""
from pathlib import Path
from native_test_support import run_native
import generic_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'func wrap(f:func()->const u64)->func()->const u64{\n  return func[f]()->const u64{return f();};',
    'func wrap(f:func()->const T)->func()->const T where T{\n  return func[f]()->const T{return f();};'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('finish(empty);', '{{func v=wrap(empty);finish(v);}};\nfinish(empty);'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('registered==5', 'registered==6'))
FIXTURE = FIXTURE.updated(expected_values=['1', *list(FIXTURE.expected_values)])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
