#!/usr/bin/env python3
"""Aborted declarations preserve complete values and failure-prefix cleanup."""
from pathlib import Path
from native_test_support import run_native
import operand_completion_runtime_tests as fixture

source = fixture.FIXTURE.source
for original, receiving in (
    ('return invoke(f,{{defer observe(21u64);return f(1u64);}});',
     'val pending<-invoke(f,{{defer observe(21u64);return f(1u64);}});'),
    ('return invoke(f,{{observe(9999u64);return 7u64;}});',
     'func pending=invoke(f,{{observe(9999u64);return 7u64;}});'),
    ('invoke(f,{{return wrap(f);}});return base;',
     'func pending=invoke(f,{{return wrap(f);}});return base;'),
    ('flag&&{{return wrap(f);}};return f;',
     'val pending<-flag&&{{return wrap(f);}};return f;'),
):
    assert source.count(original) == 1, original
    source = source.replace(original, receiving)
FIXTURE = fixture.FIXTURE.updated(source=source, sources=[Path(__file__)])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
