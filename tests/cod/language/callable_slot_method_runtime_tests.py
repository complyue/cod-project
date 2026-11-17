#!/usr/bin/env python3
"""Caller binding methods retain complete copies and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import swap_method_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
    'func (<->)(x:func(u64)->const u64 in O,y:func(u64)->const u64 in P)',
    'func (<=>)(ref x:func(u64)->const u64 in O,ref y:func(u64)->const u64 in P)'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('func g=wrap(other);f<->g;',
    'func g=wrap(other);ptr first=f,second=g;first<=>second;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('f<->g','first<=>second').replace('defer (<->)(f,g)','defer (<=>)(first,second)'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
