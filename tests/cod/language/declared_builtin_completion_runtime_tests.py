#!/usr/bin/env python3
"""Declared builtin preparation preserves complete-value cleanup without invocation."""
from pathlib import Path
import builtin_completion_runtime_tests as fixture

runner = fixture.runner
assert runner.SOURCE.count('intrinsic (+)') == 3
runner.SOURCE = 'func (+)(x:u64,y:u64)->u64;' + runner.SOURCE.replace('intrinsic (+)', '(+)')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
