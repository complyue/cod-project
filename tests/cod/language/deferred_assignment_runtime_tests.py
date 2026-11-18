#!/usr/bin/env python3
"""Prepared assignment images survive mutation and release on registration failure."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func selected_image(ref input:func(u64)->const u64 in O)->ref (func(u64)->const u64) in O where {owner O}{return input;}
{{
 func f=wrap(base);func g=wrap(other);
 ptr destination=f,source=g;
 {{
  defer intrinsic (<-)(destination,source);
  observe(destination(1u64));source<-wrap(base);observe(source(1u64));
  observe(9010u64);
 }};
 observe(9011u64);observe(f(1u64));
 observe(9030u64);
 try{defer (<-)(f,g);}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(f(1u64));
 observe(9050u64);
 try{defer intrinsic (<-)(f,g);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(f(1u64));
 func select_image=selected_image;
 {{
  ptr p=destination,q=source;
  defer{observe(p(1u64));};
  defer (=)(if(true){p}else{q},select_image(source));
 }};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('copy_failures==3', 'copy_failures==4').replace('record_failures==1', 'record_failures==2'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['2', '2', '12', '12', '12', '2'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
