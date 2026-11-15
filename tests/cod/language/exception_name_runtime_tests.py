#!/usr/bin/env python3
"""Common-root handlers preserve native allocation, causes and checked failures."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = 'using "std:exceptions" Exception;\n'+runner.SOURCE.replace(
    'catch(e:const OutOfMemoryError)', 'catch(e:const Exception)')
runner.SOURCE += '''
ptr absent:i64=null in here;
try{absent+0;{}}catch(e:const Exception){behave @print(e.category);};
'''
runner.EXPECTED_VALUES += ['4']
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
