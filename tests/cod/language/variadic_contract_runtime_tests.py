#!/usr/bin/env python3
"""Variadic promises on fixed implementations preserve complete-image cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
for signature, definition, declaration in (
    ('proc observe(x:u64)->void','proc observe(x:u64)','proc observe(rest:u64...)->void'),
    ('func base(x:u64)->const u64','func base(x:u64)','func base(rest:u64...)->const u64'),
    ('func other(x:u64)->const u64','func other(x:u64)','func other(rest:u64...)->const u64'),
    ('func wrap(f:func(u64)->const u64)->func(u64)->const u64','func wrap(f:func(u64)->const u64)',
     'func wrap(rest:func(u64)->const u64...)->func(u64)->const u64'),
    ('func apply(f:func(u64)->const u64,x:u64)->const u64','func apply(f:func(u64)->const u64,x:u64)',
     'func apply(f:func(u64)->const u64,rest:u64...)->const u64'),
    ('func missing()->const u64','func missing()','func missing(rest:u64...)->const u64'),
    ('func replaced()->func(u64)->const u64','func replaced()','func replaced(rest:u64...)->func(u64)->const u64'),
    ('func retained()->func(u64)->const u64','func retained()','func retained(rest:u64...)->func(u64)->const u64'),
):
    assert signature+'{' in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(signature+'{',declaration+';'+definition+'{'))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
