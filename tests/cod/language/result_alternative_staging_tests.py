#!/usr/bin/env python3
"""Recursive expression results preserve finite generic native implementations."""
from pathlib import Path
import result_inference_staging_tests as fixture

runner = fixture.runner
original = 'if(n==0u64){return f(x);}else{return relay(f,x,n-1u64);};'
assert original in runner.SOURCE
runner.SOURCE = runner.SOURCE.replace(original, 'return if(n==0u64){f(x)}else{relay(f,x,n-1u64)};')
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
