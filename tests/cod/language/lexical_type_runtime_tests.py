#!/usr/bin/env python3
"""Retained type context shares complete image ownership across void/value calls."""
from pathlib import Path
import generic_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace(
    'func wrap(f:func()->const u64)->func()->const u64{\n  return func[f]()->const u64{return f();};',
    'func wrap(f:func()->const T)->func()->const T where T{\n  return func[f]()->const T{return f();};')
runner.SOURCE = runner.SOURCE.replace('finish(empty);', '{{func v=wrap(empty);finish(v);}};\nfinish(empty);')
runner.HARNESS = runner.HARNESS.replace('registered==5', 'registered==6')
runner.EXPECTED_VALUES = ['1', *runner.EXPECTED_VALUES]
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
