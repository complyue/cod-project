#!/usr/bin/env python3
"""Callable holder aliases preserve complete values and exact image cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace(
    '{{func f=wrap(wrap(base));', '{{func holder=wrap(wrap(base));val var f->holder;')
runner.SOURCE = runner.SOURCE.replace(
    'func f=base;for', 'func holder=base;var f->holder;for')
runner.SOURCE = runner.SOURCE.replace(
    'func f=wrap(base);observe(9020u64);', 'func holder=wrap(base);var f->holder;observe(9020u64);')
runner.SOURCE = runner.SOURCE.replace(
    'proc f=wrapped(one);', 'proc holder=wrapped(one);var f->holder;')
runner.SOURCE = runner.SOURCE.replace('return f;\n}', 'var a->f;return a;\n}')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
