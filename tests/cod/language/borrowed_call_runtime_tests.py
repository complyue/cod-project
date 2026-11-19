#!/usr/bin/env python3
"""Borrowed argument records preserve addresses, cleanup and failure timing."""
from pathlib import Path
from native_test_support import run_native
import deferred_initialization_runtime_tests as initialization

FIXTURE = initialization.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
using "std:exceptions" OutOfMemoryError;
proc observe(x:u64)->void{behave @print(x);return;}
proc show(x:u64 in O)->void where {owner O}{observe(x);return;}
proc set(x:u64 in O)->void where {owner O}{x<-33u64;return;}
proc nullable(ptr x:u64 in O)->void throws NullPointerException where {owner O}{observe(x);return;}
proc prepared()->void {
  val a<-31u64,b<-32u64;ptr p=a;
  {{defer show(p);defer set(p);p=b:void;}};
  observe(a);observe(p);
  return;
}
proc failed()->void {
  val a<-34u64;
  defer{observe(a);observe(9002u64);};
  observe(9001u64);defer set(a);
  return;
}
proc null_deferred()->void throws NullPointerException {
  val a<-36u64;ptr p=a;p=null;
  defer{observe(37u64);};defer nullable(p);p=a;
  return;
}
func cleanup(x:u64 in O)->ref u64 in O where {owner O}{defer{x<-39u64;};return x;}
func recursive(ref x:T in O,n:u64)->ref T in O where {T,owner O}{
  if(n==0u64){return x;}else{return recursive(x,n-1u64);};
}
prepared();
try{failed();}catch(e:const OutOfMemoryError){observe(1u64);};
try{null_deferred();}catch(e:const NullPointerException){observe(4u64);};
val a<-38u64;observe(cleanup(a));recursive(a,40u64)<-40u64;observe(a);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_record=false;',
    'static bool deny_record=false;static std::uint64_t borrows=0;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('assert(bytes==16);',
    'assert(bytes==16 || bytes==72);if(bytes==72 && !deny_record)++borrows;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==6 && peak==2 && failures==2',
    'created==6 && peak==2 && failures==1 && borrows==3'))
FIXTURE = FIXTURE.updated(expected_values=['33','33','32','34','1','37','4','39','40'])
FIXTURE = FIXTURE.updated(expected_shadows=['a', 'a', 'a'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
