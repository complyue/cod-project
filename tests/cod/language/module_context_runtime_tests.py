#!/usr/bin/env python3
"""Complete captures and failed preparations across a source module boundary."""
from pathlib import Path
import symbolic_family_runtime_tests as fixture

runner = fixture.runner
definitions, execution = runner.SOURCE.split('effect @x<-1u64;', 1)
runner.SOURCE_FILES = {'library.cod': definitions + 'export @x;export @y;export @f;export @bad;export observe;'}
runner.SOURCE = '''
using "std:exceptions" OutOfMemory;
using "library.cod" observe;
symbol from "library.cod" @x;
symbol from "library.cod" @y;
using symbol "library.cod" @f;
using symbol "library.cod" @bad;
effect @x<-1u64;
''' + execution
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
