#!/usr/bin/env python3
"""Writes through returned selections preserve exact deferred resource release."""
from pathlib import Path
from native_test_support import run_native
import returned_view_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
relay = 'func relay(ptr incoming:u64 in O)->ptr u64 in O where {owner O}{return incoming;}'
before = 'try{observe(initialized(p));}'
after = 'try{relay(p)<-83u64;observe(p);observe(initialized(p));}'
assert before in FIXTURE.source
FIXTURE = FIXTURE.updated(source=relay+FIXTURE.source.replace(before, after))
assert list(FIXTURE.expected_values)[-1] == '84'
FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values[:-1], '83', *FIXTURE.expected_values[-1:]))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
