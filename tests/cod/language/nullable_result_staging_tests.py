#!/usr/bin/env python3
"""Nullable borrowed results preserve shared generic and recursive native bodies."""
from pathlib import Path
import nullable_alias_staging_tests as fixture

runner = fixture.runner
relay = 'func relay(ptr p:const T in O)->ptr const T in O where {T,owner O}{return p;}'
needle = 'ptr saved=selected;'
assert needle in runner.SOURCE
runner.SOURCE = relay+runner.SOURCE.replace(needle, 'ptr saved=relay(selected);')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
