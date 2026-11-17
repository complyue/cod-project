#!/usr/bin/env python3
"""Aliases preserve cleanup guarantees and allocation-failure behavior."""
from pathlib import Path
from native_test_support import run_native
import deferred_initialization_runtime_tests as initialization

FIXTURE = initialization.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('observe(x)', 'observe(reader)').replace('x<-', 'alias<-'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('val alias<-make local u64;',
    'val x<-make local u64;const ptr alias=x;const ptr reader=alias;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + """
proc captured()->void {
  try{missing();{}}catch(e:const MissingReturnError){
    const ptr alias=e,other=e;
    defer{observe(alias.category);observe(other.category);};
  };
}
captured();
""")
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_record=false;',
    'static bool deny_record=false;static std::uint64_t captures=0;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('assert(bytes==16);',
    'assert(bytes==16 || bytes==72);if(bytes==72)++captures;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==6 && peak==2 && failures==2',
    'created==7 && peak==2 && failures==2 && captures==1'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5', '5'])
FIXTURE = FIXTURE.updated(expected_shadows=[])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
