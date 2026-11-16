#!/usr/bin/env python3
"""Exception-derived Boolean proofs retain shared recursive native bodies."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" Exception;
using "std:exceptions" NullPointerException;
func classify(value:Exception)->bool{
 try{throw value;}catch(e:const NullPointerException){return true;}catch(e:const Exception){return false;};
}
func failure()->const NullPointerException{
 ptr absent:u64=null in here;
 try{absent;{}}catch(e:const NullPointerException){return e;};return failure();
}
func fill(ptr output:T in O,value:T,payload:Exception)->void throws NullPointerException where {T,owner O}{
 if(output is null){output;{}}else{};
 if(classify(payload)){WRITE;{}}else{};
 try{throw payload;}catch(e:const NullPointerException){}catch(e:const Exception){WRITE;{}};
}
func step(marker:T,n:u64,payload:Exception)->const T where T{
 if(n==0u64){
  val saved<-make local T;
  try{fill(saved,marker,payload);return saved;}catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64,payload);};
}
val error:Exception<-failure();
behave @print(step(7,DEPTHu64,error));
behave @print(step(8u64,DEPTHu64,error));
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
