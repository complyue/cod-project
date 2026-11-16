#!/usr/bin/env python3
"""Reference-equality evidence composes with shared generic native methods."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func same(ptr left:const T in O,ptr right:const T in O)->bool where {T,owner O}{return left is right;}
func fill(ptr output:T in O,ptr observed:const T in O,value:T)->const T throws NullPointerException where {T,owner O}{
 if(output is null){output;{}}else{};
 WRITE;
 return observed;
}
func step(marker:T,n:u64,choose:bool)->const T where T{
 if(n==0u64){
  val a<-make local T,b<-make local T;
  ptr selected=if(choose){a}else{b};
  ptr observed=if(choose){a}else{b};
  try{if(same(selected,observed)){return fill(selected,observed,marker);}else{return marker;};}
  catch(e:const NullPointerException){return marker;};
 }else{return step(marker,n-1u64,choose);};
}
behave @print(step(7,DEPTHu64,false));
behave @print(step(8u64,DEPTHu64,true));
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
