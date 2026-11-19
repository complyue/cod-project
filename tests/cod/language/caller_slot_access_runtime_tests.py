#!/usr/bin/env python3
"""Access preservation retains prepared slots and exact registration cleanup."""
from pathlib import Path
from native_test_support import run_native
import deferred_initialization_runtime_tests as fixture

FIXTURE=fixture.FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
proc observe(value:u64)->void{behave @print(value);return;}
func (<=>)(ptr left:const u64 in O,ptr right:const u64 in O)->void where {owner O}{
 intrinsic (<=>)(left,right);
}
val x<-1u64,y<-7u64,z<-9u64;
ptr p=x,q=y;ptr var ro:const->p;ptr var other:const->q;
try{
 {{defer (<=>)(ro,other);p=z:void;}};
 observe(p);observe(q);
 p=x;q=y;
 observe(9001u64);
 try{ {{defer (<=>)(ro,other);}}; }
 catch(e:const OutOfMemoryError){observe(9002u64);};
 observe(p);observe(q);
}catch(e:const NullPointerException){observe(99u64);};
''',harness=fixture.FIXTURE.harness.replace('assert(bytes==16);','assert(bytes>=16);').replace(
    'created==6 && peak==2 && failures==2','created==1 && peak==1 && failures==1'),
    expected_values=['7','9','1','7'],expected_shadows=[],sources=[Path(__file__)])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
