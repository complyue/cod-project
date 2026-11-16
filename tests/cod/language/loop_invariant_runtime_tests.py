#!/usr/bin/env python3
"""Invariant callable targets preserve native images and failure cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = 'using "std:exceptions" NullPointerException;' + runner.SOURCE + '''
{{
  func held=base;ptr selected:func(u64)->const u64=null in here;selected=held;
  for(i in 0u64..2u64){observe(selected(i+7u64));};
  try{for(i in 0u64..2u64){observe(selected(9u64));selected=null;};}
  catch(e:const NullPointerException){observe(4u64);};
}};
'''
runner.EXPECTED_VALUES += ['7', '8', '9', '4']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
