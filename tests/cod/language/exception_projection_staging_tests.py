#!/usr/bin/env python3
"""Late target reads share recursive native code across assignment spellings."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" Exception;
using "std:exceptions" IndexOutOfBoundsError;
using "std:exceptions" NullPointerException;
func unchecked()->const IndexOutOfBoundsError{try{cause_at(checked(),0u64);{}}catch(e:const IndexOutOfBoundsError){return e;};return unchecked();}
func checked()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};return checked();}
func step(marker:T,ref payload:Exception in O,a:IndexOutOfBoundsError,b:NullPointerException,n:u64)->const T throws Exception where {T,owner O}{
 if(n==0u64){
  payload<-a;ptr selected=payload;
  throw WRITE;
 }else{return step(marker,payload,a,b,n-1u64);};
}
val value:Exception<-unchecked();
try{step(7,value,unchecked(),checked(),DEPTHu64);{}}catch(e:const NullPointerException){behave @print(7);}catch(e:const Exception){behave @print(99);};
try{step(8u64,value,unchecked(),checked(),DEPTHu64);{}}catch(e:const NullPointerException){behave @print(8u64);}catch(e:const Exception){behave @print(99);};
behave @print(9);
'''
selection = 'try{payload}finally{payload<-b;{}}'
runner.SPELLINGS = {
    'infix': ('(selected='+selection+')', ''),
    'call': ('(=)(selected,'+selection+')', ''),
    'intrinsic': ('intrinsic (=)(selected,'+selection+')', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
