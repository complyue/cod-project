#!/usr/bin/env python3
"""Named calls release prepared images without invoking or registering aborted work."""
from pathlib import Path
from native_test_support import run_native
import operand_completion_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('func invoke=apply;','').replace('invoke(f,','apply(f,'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('proc invoke=observe_application;','').replace('defer invoke(', 'defer observe_application('))
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
