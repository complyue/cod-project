#!/usr/bin/env python3
"""Common-root handlers preserve native allocation, causes and checked failures."""
from pathlib import Path
from native_test_support import run_native
import exception_value_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])

FIXTURE = FIXTURE.updated(source='using "std:exceptions" Exception;\n'+FIXTURE.source.replace(
    'catch(e:const OutOfMemoryError)', 'catch(e:const Exception)'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
ptr absent:i64=null in here;
try{absent+0;{}}catch(e:const Exception){behave @print(e.category);};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['4'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
