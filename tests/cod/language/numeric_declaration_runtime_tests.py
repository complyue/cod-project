#!/usr/bin/env python3
"""Numeric overlap retains ordered captures, allocation-free calls and exact cleanup."""
from pathlib import Path
from native_test_support import run_native
import numeric_dispatch_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
func real(x:u32)->f64;
func forward(ref f:func(u64)->const u64 in O,x:u32)->u64 where {owner O};
proc invoke(p:proc(u64)->void,x:u8)->void;
''' + FIXTURE.source)
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
