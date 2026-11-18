#!/usr/bin/env python3
"""Loops in deferred actions preserve exact release through nested registration failure."""
from pathlib import Path
from native_test_support import run_native
from selected_loop_initialization_runtime_tests import loop_fixture

FIXTURE = loop_fixture(deferred=True).updated(sources=[Path(__file__)])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
