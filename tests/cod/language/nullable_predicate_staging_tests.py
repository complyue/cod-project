#!/usr/bin/env python3
"""Nullable catch classifiers retain shared finite native implementations."""
from pathlib import Path
import category_predicate_staging_tests as fixture

runner = fixture.runner
before = 'func classify(value:Exception)->bool{'
assert before in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(before, 'func classify(ptr value:const Exception in E)->bool where {owner E}{')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
