#!/usr/bin/env python3
"""Aliases preserve cleanup guarantees and allocation-failure behavior."""
from pathlib import Path
import deferred_initialization_runtime_tests as initialization

runner = initialization.runner
runner.SOURCE = runner.SOURCE.replace('val x=make local u64;',
    'val x=make local u64;const ptr alias=x;const ptr reader=alias;')
runner.SOURCE = runner.SOURCE.replace('observe(x)', 'observe(reader)').replace('x<-', 'alias<-')
runner.SOURCE += """
proc captured()->void {
  try{missing();{}}catch(e:const MissingReturnException){
    const ptr alias=e,other=e;
    defer{observe(alias.category);observe(other.category);};
  };
}
captured();
"""
runner.HARNESS = runner.HARNESS.replace('static bool deny_record=false;',
    'static bool deny_record=false;static std::uint64_t captures=0;')
runner.HARNESS = runner.HARNESS.replace('assert(bytes==16);',
    'assert(bytes==16 || bytes==72);if(bytes==72)++captures;')
runner.HARNESS = runner.HARNESS.replace('created==6 && peak==2 && failures==2',
    'created==7 && peak==2 && failures==2 && captures==1')
runner.EXPECTED_VALUES += ['5', '5']
runner.EXTRA_SOURCES = [Path(initialization.__file__), Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
