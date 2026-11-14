#!/usr/bin/env python3
"""Reference swaps select complete callable images without copying ownership."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace(
    'observe(g(1u64));observe(f(1u64));',
    'ptr p=f,q=g;p<=>q;observe(p(1u64));observe(q(1u64));')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
