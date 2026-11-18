#!/usr/bin/env python3
"""Aborted builtin preparation releases complete values and registers no action."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc finish_builtin(ref f:func(u64)->const u64 in O)->u64 where {owner O}{
 defer observe(20u64);
 intrinsic (+)(apply(f,1u64),{defer observe(21u64);return f(2u64);});return 0u64;
}
proc failing_builtin(ref f:func(u64)->const u64 in O)->u64 where {owner O}{
 intrinsic (+)(apply(f,1u64),{{observe(9999u64);return 7u64;}});return 0u64;
}
proc abort_builtin_registration(ref f:func(u64)->const u64 in O)->u64 where {owner O}{
 defer intrinsic (+)(f(1u64),{return f(3u64);});return 0u64;
}
func return_builtin_image(f:func(u64)->const u64)->func(u64)->const u64{
 1u64+{return wrap(f);};return base;
}
{{
 func f=wrap(base);observe(finish_builtin(f));
 observe(9030u64);
 try{failing_builtin(f);{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(9050u64);observe(abort_builtin_registration(f));observe(9060u64);
 func g=return_builtin_image(f);observe(g(1u64));
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5','checkpoints==6').replace('copy_failures==3','copy_failures==4').replace('  assert(value!=9999);','  if(value==9060){assert(deny_record && record_failures==1 && images.size()==live_baseline);deny_record=false;return;}\n  assert(value!=9999);'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['21','20','3','4','3'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
