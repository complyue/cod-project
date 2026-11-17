#!/usr/bin/env python3
"""Invariant callable targets preserve native images and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='using "std:exceptions" NullPointerException;' + FIXTURE.source + '''
{{
  func held=base;ptr selected:func(u64)->const u64=null in here;selected=held;
  for(i in 0u64..2u64){observe(selected(i+7u64));};
  try{for(i in 0u64..2u64){observe(selected(9u64));selected=null;};}
  catch(e:const NullPointerException){observe(4u64);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['7', '8', '9', '4'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
