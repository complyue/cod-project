#!/usr/bin/env python3
"""Prepared methods share native bodies as runtime recursion grows."""
from pathlib import Path
import reference_swap_method_staging_tests as fixture
runner = fixture.runner
runner.SPELLINGS = {'prepared': ('{{defer (<=>)(p,q);}}','')}
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
