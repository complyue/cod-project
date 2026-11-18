#!/usr/bin/env python3
"""Prepared reference methods reload local slots and preserve cleanup on failure."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
using "std:exceptions" NullPointerException;
func (<=>)(ref a:u64 in O,ref b:u64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}
proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{behave @print(a);behave @print(b);}
{{
  func kept=wrap(base);
  val x<-1u64,y<-7u64,z<-9u64;ptr p=x,q=y;
  observe(9010u64);
  # The observer registered first runs after the last successful exchange.
  defer{observe(p);observe(q);observe(9011u64);};
  defer (<=>)(p,q);
  observe(9050u64);
  try{defer (<=>)(p,q);}catch(e:const OutOfMemoryError){observe(9051u64);};
  defer{observe(p);observe(q);};
  for(i in 0u64..30u64){defer (<=>)(p,q);};
  defer{observe(p);observe(q);};
  defer (<=>)(p,q);
  q=z;
}};
{{
  val b<-7;ptr r:i64=null in here,s=b;
  try{defer (<=>)(r,s);{}}catch(e:const NullPointerException){observe(8u64);};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2'))
FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values, '9','1','9','1','1','9','8'))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
