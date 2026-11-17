#!/usr/bin/env python3
"""Selected write proofs preserve shared generic recursive native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
using "std:exceptions" OutOfMemoryError;
func step(marker:T,n:u64,choose_left:bool)->const T where T{
 if(n==0u64){
  val a<-make local T,b<-make local T;
  ptr output=if(choose_left){a}else{b};
  ptr observed=if(choose_left){a}else{b};
  ptr selected=if(output is observed){null in here}else{a};
  try{
   defer{if(selected is null){return observed;}else{return marker;};};
   try{defer{
    if(output is null){output;{}}else{};
    WRITE;
    {}
   };}catch(registration:const OutOfMemoryError){
    if(output is null){output;{}}else{};
    WRITE;
    {}
   };
   return marker;
  }catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64,choose_left);};
}
behave @print(step(7,DEPTHu64,false));
behave @print(step(8u64,DEPTHu64,true));
behave @print(9);
'''
runner.SPELLINGS = {
    'infix': ('output<-marker', ''),
    'call': ('(<-)(output,marker)', ''),
    'intrinsic': ('intrinsic (<-)(output,marker)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
