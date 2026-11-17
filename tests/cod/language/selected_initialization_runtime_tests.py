#!/usr/bin/env python3
"""Registration failures recover selected writes with exact cleanup accounting."""
from pathlib import Path
import deferred_recovery_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
proc observe(x:u64)->void{behave @print(x);}
proc verify(first:bool,second:bool,fail:bool,conditional:bool)->void{
 val a<-make local u64,b<-make local u64;
 ptr p=if(first){a}else{b};ptr q=if(second){a}else{b};
 ptr selected=if(p is q){null in here}else{a};
 try{ {{
  defer{if(selected is null){observe(q);}else{observe(0u64);};};
  if(fail){behave @print(9001u64);}else{};
  try{defer{if(!conditional||selected is null){p<-7u64;{}}else{};};}
  catch(registration:const OutOfMemoryError){behave @print(9002u64);p<-7u64;{}};
  {}
 }};}catch(e:const NullPointerException){observe(99u64);};
}
'''
runner.EXPECTED_VALUES = []
for conditional in (False, True):
    for fail in (False, True):
        for first,second in ((False,False),(False,True),(True,False),(True,True)):
            args=','.join(str(value).lower() for value in (first,second,fail,conditional))
            runner.SOURCE += 'verify('+args+');\n'
            runner.EXPECTED_VALUES.append('7' if first==second else '0')
runner.HARNESS = runner.HARNESS.replace('assert(bytes==72)', 'assert(bytes==16)')
runner.HARNESS = runner.HARNESS.replace('assert(live.empty() && created==released);deny_record=false',
    'assert(live.size()==1 && created==released+1);deny_record=false')
runner.HARNESS = runner.HARNESS.replace('created==4 && peak==2 && failures==3',
    'created==24 && peak==2 && failures==8')
runner.EXPECTED_SHADOWS = []
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
