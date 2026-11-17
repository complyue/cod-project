#!/usr/bin/env python3
"""Deferred caller-slot methods retain target selection and exact resource release."""
from pathlib import Path
from native_test_support import run_native
import deferred_reference_swap_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer intrinsic (<=>)((if(flag)', 'defer (<=>)((if(flag)'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func (<=>)(ref left:func(u64)->const u64 in O,ref right:func(u64)->const u64 in O)->void where {owner O}{
 intrinsic (<=>)(left,right);
}
''')
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
