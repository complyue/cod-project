#!/usr/bin/env python3
"""Numeric overlap retains ordered captures, allocation-free calls and exact cleanup."""
from pathlib import Path
import numeric_dispatch_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
func real(x:u32)->f64;
func forward(ref f:func(u64)->const u64 in O,x:u32)->u64 where {owner O};
proc invoke(p:proc(u64)->void,x:u8)->void;
''' + runner.SOURCE
runner.EXTRA_SOURCES += [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
