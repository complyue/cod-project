#!/usr/bin/env python3
"""Deferred caller-slot methods retain target selection and exact resource release."""
from pathlib import Path
import deferred_reference_swap_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('defer intrinsic (<=>)((if(flag)', 'defer (<=>)((if(flag)')
runner.SOURCE += '''
func (<=>)(ref left:func(u64)->const u64 in O,ref right:func(u64)->const u64 in O)->void where {owner O}{
 intrinsic (<=>)(left,right);
}
'''
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
