#!/usr/bin/env python3
"""Checked null failures compose with allocation denial, recovery and snapshots."""
from pathlib import Path
import deferred_initialization_runtime_tests as initialization

runner = initialization.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
using "std:exceptions" OutOfMemory;
proc observe(x:u64)->void{behave @print(x);}
proc body_null()->void {
  ptr p:u64=null in here;
  defer{observe(31u64);observe(9002u64);};observe(9001u64);
  try{observe(p);}catch(e:const NullPointerException){
    observe(e.category);observe(e.requested);observe(e.available);observe(e.cause_count);
  };
}
proc deferred_null()->void throws NullPointerException {
  val a<-32u64;ptr p=a;
  defer{observe(33u64);};defer{observe(p);};p=null;
}
func recover()->const u64 {
  ptr p:u64=null in here;
  defer catch(e:const NullPointerException){return 34u64;};
  defer{p;};return 35u64;
}
proc registration()->void throws NullPointerException {
  ptr p:u64=null in here;
  defer{if(isnull(p)){observe(36u64);}else{observe(99u64);};observe(9002u64);};
  observe(9001u64);defer{observe(p);};
}
proc captured()->void {
  ptr p:u64=null in here;
  try{observe(p);}catch(e:const NullPointerException){
    ptr q=e;
    defer{if(isnull(q)){observe(37u64);}else{observe(q.category);};};
    q=null;{};
  };
}
body_null();
try{deferred_null();}catch(e:const NullPointerException){observe(4u64);};
observe(recover());
try{registration();}catch(e:const OutOfMemory){observe(1u64);}catch(e:const NullPointerException){observe(99u64);};
captured();
'''
runner.HARNESS = runner.HARNESS.replace('static bool deny_record=false;',
    'static bool deny_record=false;static std::uint64_t captures=0;')
runner.HARNESS = runner.HARNESS.replace('assert(bytes==16);',
    'assert(bytes==16 || bytes==72);if(bytes==72)++captures;')
runner.HARNESS = runner.HARNESS.replace('created==6 && peak==2 && failures==2',
    'created==7 && peak==2 && failures==1 && captures==1')
runner.EXPECTED_VALUES = ['4','0','0','0','31','33','4','34','36','1','37']
runner.EXTRA_SOURCES = [Path(initialization.__file__), Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
