#!/usr/bin/env python3
"""Deferred-loop entry proofs preserve shared generic recursive native bodies."""
from pathlib import Path
import selected_loop_initialization_staging_tests as fixture

runner=fixture.runner
assert runner.SOURCE.count('  for(i in')==1
runner.SOURCE = runner.SOURCE.replace('  for(i in', '  defer{for(i in').replace(
    '  return selected;', '  return selected;};\n  return marker;')
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
