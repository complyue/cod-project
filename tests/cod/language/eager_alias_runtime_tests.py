#!/usr/bin/env python3
"""Aliased builtin arithmetic preserves native registration and failure cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source_files={'ops.cod': 'func (+)(a:bool,b:bool)->bool{return a||b;}export (+);'
    'func (/)(a:bool,b:bool)->bool{}export (/);'})
FIXTURE = FIXTURE.updated(source='using "ops.cod" (+) as add;using "ops.cod" (/) as divide;' + FIXTURE.source)
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
{{
 func kept=wrap(base);
 val x<-1u64;
 observe(9010u64);
 for(i in 0u64..30u64){add(i,x);};
 observe(9011u64);
 {{
  defer divide(1u64,1u64);
  observe(9050u64);
  try{defer add(x,2u64);}catch(e:const OutOfMemoryError){observe(9051u64);};
 }};
 try{ {{defer{observe(9u64);};defer divide(false,true);}};{}}
 catch(e:const MissingReturnError){observe(8u64);};
 observe(add(x,2u64));
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['9','8','3'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
