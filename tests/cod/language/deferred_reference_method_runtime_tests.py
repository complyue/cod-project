#!/usr/bin/env python3
"""Saved scalar slots preserve native cleanup on registration and entry failures."""
from pathlib import Path
import captured_callable_runtime_tests as fixture
runner = fixture.runner
runner.SOURCE += '''
using "std:exceptions" NullPointerException;
func (<=>)(ref a:u64 in O,ref b:u64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}
proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{behave @print(a);behave @print(b);}
{{
 func kept=wrap(base);
 val x<-1u64,y<-7u64,z<-9u64;ptr p=x,q=y;
 {{defer (<=>)(p,q);q=z;}};observe(p);observe(q);
 observe(9010u64);
 {{for(i in 0u64..30u64){defer (<=>)(p,q);};}};
 observe(9011u64);observe(p);observe(q);
 {{
  defer (<=>)(p,q);
  observe(9050u64);
  try{defer (<=>)(p,q);}catch(e:const OutOfMemory){observe(9051u64);};
 }};observe(p);observe(q);
 val a<-1,b<-7;ptr r=a,s=b;
 {{
  defer catch(e:const NullPointerException){observe(8u64);};
  defer (<=>)(r,s);r=null;
 }};
 observe(p);observe(q);
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2')
runner.EXPECTED_VALUES += ['9','1','9','1','1','9','8','1','9']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
