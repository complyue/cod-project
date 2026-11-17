#!/usr/bin/env python3
"""Contextual caught-bit records keep exact layouts and failure-safe image ownership."""
from pathlib import Path
from native_test_support import run_native
import loop_completion_runtime_tests as fixture
import contextual_capture_tests as source
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__), Path(__file__).with_name('contextual_capture_tests.py'), Path(__file__).with_name('deferred_capture_tests.py'), Path(__file__).with_name('foundation_tests.py')])

FIXTURE = FIXTURE.updated(source=FIXTURE.source + source.PICK + '''
proc contextual_records(tag:T)->void where T {
 func held=wrap(base);
 try{throw bound_error();}catch(e:const MissingReturnError){
   ref y=pick(tag,e);
   defer{observe(held(1u64));};
   observe(9070u64);
   defer{defer{observe(y.category);};};
   observe(9071u64);
   defer{observe(e.category);observe(y.category);};
   observe(9050u64);
   try{defer{observe(y.category);};}catch(failure:const OutOfMemoryError){observe(9051u64);};
 };
}
contextual_records(false);contextual_records(0);
observe(9000u64);
observe(9072u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==9', 'checkpoints==10'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('record_failures==1 && largest', 'record_failures==3 && largest'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_create', 'static unsigned capture_mode=0;\nstatic std::uint64_t capture_sizes[2]={};\nstatic bool deny_create'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace(
    '  if(deny_record){++record_failures;',
    '  if(capture_mode){assert(capture_mode<=2 && !capture_sizes[capture_mode-1]);capture_sizes[capture_mode-1]=size;}\n  if(deny_record){++record_failures;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('  if(value==9000)', '''  if(value==9070){assert(!capture_mode);capture_mode=capture_sizes[0]?2:1;return;}
  if(value==9071){assert(capture_mode && capture_sizes[capture_mode-1]);capture_mode=0;return;}
  if(value==9072){assert(capture_sizes[0]==80 && capture_sizes[1]==16 && !capture_mode);return;}
  if(value==9000)'''))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5', '5', '5', '2', '5', '5', '5', '2'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
