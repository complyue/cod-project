#!/usr/bin/env python3
"""Returned addresses survive failed registrations, recovery and replacement."""
from pathlib import Path
from native_test_support import run_native
import deferred_recovery_runtime_tests as recovery

FIXTURE = recovery.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" MissingReturnError;
using "std:exceptions" NullPointerException;
proc observe(x:u64)->void{behave @print(x);}
proc retained(ptr x:u64 in O,ptr y:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemoryError){observe(1u64);observe(9002u64);};
  defer{observe(9001u64);defer observe(99u64);};
  return x;
}
proc replacement(ptr x:u64 in O,ptr y:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemoryError){observe(2u64);observe(9002u64);return y;};
  defer{observe(9001u64);defer observe(99u64);};
  return x;
}
proc absent(ptr x:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemoryError){observe(3u64);observe(9002u64);};
  observe(9001u64);defer observe(99u64);return x;
}
func initialized(ptr x:u64 in O)->ptr u64 in O throws NullPointerException where {owner O}{
  defer{x<-84u64;};return x;
}
val a<-81u64,b<-make local u64,c<-82u64;
try{observe(retained(a,b));observe(replacement(a,c));}catch(e:const NullPointerException){observe(99u64);};
try{absent(a);{}}catch(e:const MissingReturnError){observe(4u64);};
val d<-make local u64,e<-make local u64;ptr p=if(true){d}else{e};
try{observe(initialized(p));}catch(e:const NullPointerException){observe(99u64);};
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==4 && peak==2 && failures==3',
    'created==6 && peak==2 && failures==3'))
FIXTURE = FIXTURE.updated(expected_values=['1', '81', '2', '82', '3', '4', '84'])
FIXTURE = FIXTURE.updated(expected_shadows=['e', 'e', 'e', 'e'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
