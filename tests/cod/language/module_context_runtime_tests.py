#!/usr/bin/env python3
"""Complete captures and failed preparations across a source module boundary."""
from pathlib import Path
from native_test_support import run_native
import symbolic_family_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
definitions, execution = FIXTURE.source.split('effect @x<-1u64;', 1)
FIXTURE = FIXTURE.updated(source_files={'library.cod': definitions + 'export var @x;export var @y;export proc @f;export proc @bad;export observe;'})
FIXTURE = FIXTURE.updated(source='''
using "std:exceptions" OutOfMemoryError;
using "./library.cod" observe;
symbol from "./library.cod" @x;
symbol from "./library.cod" @y;
using symbol "./library.cod" @f;
using symbol "./library.cod" @bad;
effect @x<-1u64;
''' + execution)
FIXTURE = FIXTURE.updated(expected_shadows=['x'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
