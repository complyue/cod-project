#!/usr/bin/env python3
"""Exiting alias selectors retain complete-image cleanup and perform no copy."""
from pathlib import Path
import storage_completion_runtime_tests as fixture

runner = fixture.runner
callable_declaration = 'func unused:func(u64)->const u64='
assert runner.SOURCE.count(callable_declaration) == 2
runner.SOURCE = runner.SOURCE.replace(callable_declaration, 'var unused->')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
