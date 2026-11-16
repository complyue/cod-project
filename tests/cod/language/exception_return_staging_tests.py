#!/usr/bin/env python3
"""Exception-result summaries preserve shared recursive native implementations."""
from pathlib import Path
import assignment_call_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" Exception;
using "std:exceptions" IndexOutOfBoundsError;
using "std:exceptions" NullPointerException;
func unchecked()->const IndexOutOfBoundsError{try{cause_at(checked(),0u64);{}}catch(e:const IndexOutOfBoundsError){return e;};return unchecked();}
func checked()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};return checked();}
func step(marker:T,item:Exception,n:u64)->const Exception where {T}{
 if(n==0u64){ptr selected=item;return WRITE;}
 else{return step(marker,item,n-1u64);};
}
val value:Exception<-unchecked();
try{throw step(7,value,DEPTHu64);}catch(e:const IndexOutOfBoundsError){behave @print(7);}catch(e:const Exception){behave @print(99);};
try{throw step(8u64,value,DEPTHu64);}catch(e:const IndexOutOfBoundsError){behave @print(8u64);}catch(e:const Exception){behave @print(99);};
behave @print(9);
'''
runner.SPELLINGS = {
    'infix': ('(selected=item)', ''),
    'call': ('(=)(selected,item)', ''),
    'intrinsic': ('intrinsic (=)(selected,item)', ''),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
