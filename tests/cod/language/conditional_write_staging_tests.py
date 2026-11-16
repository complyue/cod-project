#!/usr/bin/env python3
"""Conditional call guarantees preserve shared recursive native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func fill(ptr output:T in O,value:T,flag:bool)->void throws NullPointerException where {T,owner O}{
 if(output is null){output;{}}else{};
 if(flag){WRITE;{}}else{};
}
func step(marker:T,n:u64)->const T where {T}{
 if(n==0u64){
  val saved<-make local T;
  try{fill(saved,marker,true);return saved;}catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64);};
}
behave @print(step(7,DEPTHu64));
behave @print(step(8u64,DEPTHu64));
behave @print(9);
'''
runner.SPELLINGS = {
    'infix': ('output<-value', ''),
    'call': ('(<-)(output,value)', ''),
    'intrinsic': ('intrinsic (<-)(output,value)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
