#!/usr/bin/env python3
"""Operator-selected callable factories retain ordinary image and cleanup contracts."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('func wrap(', 'func (+)(').replace('wrap(', '(+)(')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
