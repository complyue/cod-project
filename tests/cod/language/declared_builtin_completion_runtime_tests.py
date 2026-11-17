#!/usr/bin/env python3
"""Declared builtin preparation preserves complete-value cleanup without invocation."""
from pathlib import Path
from native_test_support import run_native
import builtin_completion_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
assert FIXTURE.source.count('intrinsic (+)') == 3
FIXTURE = FIXTURE.updated(source='func (+)(x:u64,y:u64)->u64;' + FIXTURE.source.replace('intrinsic (+)', '(+)'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
