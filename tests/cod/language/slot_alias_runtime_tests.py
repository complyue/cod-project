#!/usr/bin/env python3
"""Shared slots retain native assignment order and faulted cleanup behavior."""
from pathlib import Path
import mutable_reference_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('ptr p=a;', 'ptr original=a;ptr var p->original;')
runner.SOURCE = runner.SOURCE.replace('defer{observe(p);', 'defer{observe(original);')
runner.SOURCE = runner.SOURCE.replace('p=b;', 'original=b;')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
