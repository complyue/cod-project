#!/usr/bin/env python3
"""Nullable catch predicates preserve exact callable and region release."""
from pathlib import Path
from native_test_support import run_native
import category_predicate_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
before = 'func classify(value:Exception)->bool{'
assert before in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, 'func classify(ptr value:const Exception in E)->bool where {owner E}{'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
