#!/usr/bin/env python3
"""Aliased exception writes retain one shared recursive native body."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" Exception;
using "std:exceptions" IndexOutOfBoundsError;
using "std:exceptions" NullPointerException;
func unchecked()->const IndexOutOfBoundsError{try{cause_at(checked(),0u64);{}}catch(e:const IndexOutOfBoundsError){return e;};return unchecked();}
func checked()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};return checked();}
func step(marker:T,ref x:Exception in O,ref y:Exception in P,a:IndexOutOfBoundsError,b:NullPointerException,n:u64)->const T throws NullPointerException where {T,owner O,owner P}{
 var left->x;var right->y;
 if(n==0u64){
  WRITE;
  throw x;
 }else{return step(marker,x,y,a,b,n-1u64);};
}
val first:Exception<-unchecked(),second:Exception<-unchecked();
try{step(7,first,second,unchecked(),checked(),DEPTHu64);{}}catch(e:const IndexOutOfBoundsError){behave @print(7);}catch(e:const NullPointerException){behave @print(0);};
try{step(8u64,first,first,unchecked(),checked(),DEPTHu64);{}}catch(e:const IndexOutOfBoundsError){behave @print(0);}catch(e:const NullPointerException){behave @print(8u64);};
behave @print(9);
'''
runner.SPELLINGS = {
    'direct': ('x<-a;y<-b', ''),
    'aliases': ('left<-a;right<-b', ''),
    'calls': ('(<-)(x,a);(<-)(y,b)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
