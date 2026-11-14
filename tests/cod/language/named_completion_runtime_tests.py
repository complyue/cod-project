#!/usr/bin/env python3
"""Named calls release prepared images without invoking or registering aborted work."""
from pathlib import Path
import operand_completion_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE=runner.SOURCE.replace('func invoke=apply;','').replace('invoke(f,','apply(f,')
runner.SOURCE=runner.SOURCE.replace('proc invoke=observe_application;','').replace('defer invoke(', 'defer observe_application(')
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
