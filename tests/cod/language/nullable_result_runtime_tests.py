#!/usr/bin/env python3
"""Nullable result forwarding preserves deferred reads and exact resource release."""
from pathlib import Path
import nullable_alias_runtime_tests as fixture

runner = fixture.runner
relay = 'func relay(ptr p:const Exception in O)->ptr const Exception in O where {owner O}{return p;}'
needle = 'ptr observed=selected;ptr saved=observed;'
assert needle in runner.SOURCE
runner.SOURCE = relay+runner.SOURCE.replace(needle, 'ptr observed=relay(selected);ptr saved=relay(observed);')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
