#!/usr/bin/env python3
"""Loop setup exits retain returned images and skip accumulator/queue allocation."""
from pathlib import Path
from native_test_support import run_native
import storage_completion_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc stop_bounds(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 func saved=f;defer observe(45u64);
 for(i in 0u64..{return saved(1u64);}){defer observe(9999u64);}else{observe(9999u64);};return 0u64;
}
proc skip_loop_records()->const u64{
 for(i in 0u64..{return 46u64;}){defer observe(9999u64);}else{defer observe(9999u64);};return 0u64;
}
func fold_setup(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 return fold(i in 0u64..{return 47u64;},a=f){a};
}
func image_bound(f:func(u64)->const u64)->const (func(u64)->const u64){
 for(i in 0u64..{return wrap(f);}){};return base;
}
func bound_error()->const MissingReturnError{
 try{missing();}catch(e:const MissingReturnError){return e;};
}
proc throw_bound(ref f:func(u64)->const u64 in O)->void where {owner O}{
 func saved=f;for(i in 0u64..{throw bound_error();}){observe(9999u64);}else{observe(9999u64);};
}
{{
 func f=wrap(base);observe(stop_bounds(f));func result=image_bound(f);observe(result(1u64));
 observe(9050u64);observe(skip_loop_records());observe(9060u64);
 observe(9030u64);observe(fold_setup(f));observe(9061u64);
 observe(9040u64);try{throw_bound(f);}catch(e:const MissingReturnError){observe(9041u64);observe(48u64);};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==8','checkpoints==9'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['45','2','3','46','47','48'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
