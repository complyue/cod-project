#!/usr/bin/env python3
"""Returned addresses survive failed registrations, recovery and replacement."""
from pathlib import Path
import deferred_recovery_runtime_tests as recovery

runner = recovery.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
using "std:exceptions" NullPointerException;
proc observe(x:u64)->void{behave @print(x);}
proc retained(ptr x:u64 in O,ptr y:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemory){observe(1u64);observe(9002u64);};
  defer{observe(9001u64);defer observe(99u64);};
  return x;
}
proc replacement(ptr x:u64 in O,ptr y:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemory){observe(2u64);observe(9002u64);return y;};
  defer{observe(9001u64);defer observe(99u64);};
  return x;
}
proc absent(ptr x:u64 in O)->ptr u64 in O where {owner O}{
  defer catch(e:const OutOfMemory){observe(3u64);observe(9002u64);};
  observe(9001u64);defer observe(99u64);return x;
}
func initialized(ptr x:u64 in O)->ptr u64 in O throws NullPointerException where {owner O}{
  defer{x<-84u64;};return x;
}
val a<-81u64,b<-make local u64,c<-82u64;
try{observe(retained(a,b));observe(replacement(a,c));}catch(e:const NullPointerException){observe(99u64);};
try{absent(a);{}}catch(e:const MissingReturnException){observe(4u64);};
val d<-make local u64,e<-make local u64;ptr p=if(true){d}else{e};
try{observe(initialized(p));}catch(e:const NullPointerException){observe(99u64);};
'''
runner.HARNESS = runner.HARNESS.replace('created==4 && peak==2 && failures==3',
    'created==6 && peak==2 && failures==3')
runner.EXPECTED_VALUES = ['1', '81', '2', '82', '3', '4', '84']
runner.EXTRA_SOURCES = [Path(recovery.__file__), Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
