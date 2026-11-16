#!/usr/bin/env python3
"""Writes through returned selections preserve exact deferred resource release."""
from pathlib import Path
import returned_view_runtime_tests as fixture

runner = fixture.runner
relay = 'func relay(ptr incoming:u64 in O)->ptr u64 in O where {owner O}{return incoming;}'
before = 'try{observe(initialized(p));}'
after = 'try{relay(p)<-83u64;observe(p);observe(initialized(p));}'
assert before in runner.SOURCE
runner.SOURCE = relay+runner.SOURCE.replace(before, after)
assert runner.EXPECTED_VALUES[-1] == '84'
runner.EXPECTED_VALUES.insert(-1, '83')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
