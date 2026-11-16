#!/usr/bin/env python3
"""Complementary call proofs share recursive native code at every depth."""
from pathlib import Path
import conditional_write_staging_tests as fixture

runner = fixture.runner
for before, after in (
    ('step(marker:T,n:u64)', 'step(marker:T,n:u64,flag:bool)'),
    ('fill(saved,marker,true);', 'fill(saved,marker,flag);fill(saved,marker,!flag);'),
    ('step(marker,n-1u64)', 'step(marker,n-1u64,flag)'),
    ('step(7,DEPTHu64)', 'step(7,DEPTHu64,true)'),
    ('step(8u64,DEPTHu64)', 'step(8u64,DEPTHu64,false)'),
):
    assert before in runner.SOURCE
    runner.SOURCE = runner.SOURCE.replace(before, after)
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
