#!/usr/bin/env python3
"""Caller binding methods retain complete copies and failure cleanup."""
from pathlib import Path
import swap_method_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace(
    'func (<->)(x:func(u64)->const u64 in O,y:func(u64)->const u64 in P)',
    'func (<=>)(ref x:func(u64)->const u64 in O,ref y:func(u64)->const u64 in P)')
runner.SOURCE = runner.SOURCE.replace('func g=wrap(other);f<->g;',
    'func g=wrap(other);ptr first=f,second=g;first<=>second;')
runner.SOURCE = runner.SOURCE.replace('f<->g','first<=>second').replace('defer (<->)(f,g)','defer (<=>)(first,second)')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
