#!/usr/bin/env python3
"""Reference swaps select complete callable images without copying ownership."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'observe(g(1u64));observe(f(1u64));',
    'ptr p=f,q=g;p<=>q;observe(p(1u64));observe(q(1u64));'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
