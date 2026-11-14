#!/usr/bin/env python3
"""Declared operator results preserve complete callable images and failed cleanup paths."""
from pathlib import Path
import eager_operator_runtime_tests as fixture

runner = fixture.runner
head = 'func (+)(f:func(u64)->const u64)'
result = '->func(u64)->const u64'
assert head + result + '{' in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(head + result + '{', head + result + ';' + head + '{')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
