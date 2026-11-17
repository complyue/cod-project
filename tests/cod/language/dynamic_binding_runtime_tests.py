#!/usr/bin/env python3
"""TLS restoration survives allocation failure without creating captured images."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
symbol @context:u64;
proc context_value()->u64{return @context;}
proc contextual(n:u64)->u64{
 if(n==0u64){return context_value();}else{
  effect @context<-@context+1u64;
  return contextual(n-1u64);
 };
}
effect @context<-10u64;
{{
 func retained=wrap(base);
 observe(9010u64);
 observe(contextual(30u64));observe(@context);
 for(i in 0u64..10u64){effect @context<-@context+1u64;};
 observe(@context);
 observe(9011u64);
}};
observe(9000u64);
try{{{
 effect @context<-20u64;
 observe(9050u64);
 defer observe(9999u64);
}}}catch(e:const OutOfMemoryError){observe(9051u64);observe(@context);};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==7').replace('record_failures==1', 'record_failures==2'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['40', '10', '10', '10'])
FIXTURE = FIXTURE.updated(expected_shadows=['retained'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
