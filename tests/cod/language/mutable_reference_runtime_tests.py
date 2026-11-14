#!/usr/bin/env python3
"""Faulted registration and RHS evaluation preserve current reference targets."""
from pathlib import Path
import deferred_initialization_runtime_tests as initialization

runner = initialization.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
func missing()->const u64{}
proc observe(x:u64)->void{behave @print(x);}
proc normal()->void {
  val a<-11u64,b<-22u64;ptr p=a;
  defer{observe(p);};p=b;defer{p=a;};
}
proc failed_registration()->void {
  val a<-31u64,b<-32u64;ptr p=a;
  defer{observe(p);observe(9002u64);};p=b;
  observe(9001u64);defer{p=a;};
}
proc failed_rhs()->void {
  val a<-41u64,b<-42u64;ptr p=a;
  defer{observe(p);observe(a);observe(b);};
  try{p={p=b;missing();a};{}}catch(e:const MissingReturnException){observe(p);};
}
proc destination()->void {
  val a<-51u64,b<-52u64;ptr p=a;
  p<-{{defer{p=b;};99u64}};
  observe(a);observe(p);
}
proc queued(n:u64)->void {
  val a<-61u64,b<-62u64;ptr p=a;
  for(i in 0u64..n){defer{observe(p);};p=b;};
}
normal();
try{failed_registration();}catch(e:const OutOfMemory){observe(1u64);};
failed_rhs();destination();queued(3u64);
'''
runner.HARNESS = runner.HARNESS.replace('created==6 && peak==2 && failures==2',
    'created==8 && peak==3 && failures==1')
runner.EXPECTED_VALUES = ['11','32','1','42','42','41','42','99','52','62','62','62']
runner.EXTRA_SOURCES = [Path(initialization.__file__), Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
