#!/usr/bin/env python3
"""Nullable result forwarding preserves deferred reads and exact resource release."""
from pathlib import Path
from native_test_support import run_native
import nullable_alias_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
relay = 'func relay(ptr p:const Exception in O)->ptr const Exception in O where {owner O}{return p;}'
needle = 'ptr observed=selected;ptr saved=observed;'
assert needle in FIXTURE.source
FIXTURE = FIXTURE.updated(source=relay+FIXTURE.source.replace(needle, 'ptr observed=relay(selected);ptr saved=relay(observed);'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
