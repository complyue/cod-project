#!/usr/bin/env python3
"""Uninitialized borrowed inputs acquire guarantees only through executed writes."""
from pathlib import Path
import deferred_initialization_runtime_tests as initialization

runner=initialization.runner
runner.SOURCE='''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
proc observe(x:u64)->void{behave @print(x);}
func missing()->const u64{}
proc set(ptr x:u64 in O)->void throws NullPointerException where {owner O}{observe(9003u64);x<-71u64;}
proc read(ptr x:u64 in O)->void throws NullPointerException where {owner O}{observe(x);}
proc direct()->void throws NullPointerException{val x=make local u64;set(x);read(x);}
proc prepared()->void throws NullPointerException{val x=make local u64;{{defer set(x);}};read(x);}
proc denied()->void throws NullPointerException{
  val x=make local u64;defer{observe(9002u64);};observe(9001u64);defer set(x);
}
proc fail_after(ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-73u64;missing();}
proc cleanup(ptr x:u64 in O)->void throws NullPointerException where {owner O}{defer{x<-74u64;};}
proc alias(ptr x:u64 in O,ptr y:u64 in O)->void throws NullPointerException where {owner O}{defer{x<-75u64;observe(y);};}
try{direct();prepared();}catch(e:const NullPointerException){observe(99u64);};
try{denied();}catch(e:const OutOfMemory){observe(1u64);}catch(e:const NullPointerException){observe(99u64);};
val a=make local u64;
try{fail_after(a);}catch(e:const MissingReturnException){observe(a);}catch(e:const NullPointerException){observe(99u64);};
val b=make local u64;
try{cleanup(b);observe(b);}catch(e:const NullPointerException){observe(99u64);};
val c=make local u64;
try{alias(c,c);observe(c);}catch(e:const NullPointerException){observe(99u64);};
val d=make local u64;
try{{defer read(d);d<-76u64;{}}}catch(e:const NullPointerException){observe(99u64);};
'''
runner.HARNESS=runner.HARNESS.replace('static bool deny_record=false;',
    'static bool deny_record=false;static std::uint64_t borrows=0,setters=0;')
runner.HARNESS=runner.HARNESS.replace('assert(bytes==16);',
    'assert(bytes==16 || bytes==72);if(bytes==72 && !deny_record)++borrows;')
runner.HARNESS=runner.HARNESS.replace('if(value==9001)',
    'if(value==9003){++setters;return;}if(value==9001)')
runner.HARNESS=runner.HARNESS.replace('created==6 && peak==2 && failures==2',
    'created==5 && peak==1 && failures==1 && borrows==2 && setters==2')
runner.EXPECTED_VALUES=['71','71','1','73','74','75','75','76']
runner.EXTRA_SOURCES=[Path(initialization.__file__),Path(__file__)]

if __name__=='__main__':raise SystemExit(runner.main())
