#!/usr/bin/env python3
"""Aliased builtin arithmetic preserves native registration and failure cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture
runner = fixture.runner
runner.SOURCE_FILES = {'ops.cod': 'func (+)(a:bool,b:bool)->bool{return a||b;}export (+);'
    'func (/)(a:bool,b:bool)->bool{}export (/);'}
runner.SOURCE = 'using "ops.cod" (+) as add;using "ops.cod" (/) as divide;' + runner.SOURCE
runner.SOURCE += '''
{{
 func kept=wrap(base);
 val x<-1u64;
 observe(9010u64);
 for(i in 0u64..30u64){add(i,x);};
 observe(9011u64);
 {{
  defer divide(1u64,1u64);
  observe(9050u64);
  try{defer add(x,2u64);}catch(e:const OutOfMemory){observe(9051u64);};
 }};
 try{ {{defer{observe(9u64);};defer divide(false,true);}};{}}
 catch(e:const MissingReturnException){observe(8u64);};
 observe(add(x,2u64));
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2')
runner.EXPECTED_VALUES += ['9','8','3']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
