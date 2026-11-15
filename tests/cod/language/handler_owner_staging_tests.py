#!/usr/bin/env python3
"""Handler-derived result evidence preserves shared recursive native methods."""
from pathlib import Path
import scoped_owner_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func first(ref a:const NullPointerException in A,ref b:const NullPointerException in B)->ref const NullPointerException in A where {owner A,owner B}{return a;}
func second(ref a:const NullPointerException in A,ref b:const NullPointerException in B)->ref const NullPointerException in B where {owner A,owner B}{return b;}
func tag(ref a:const NullPointerException in O,ref b:const NullPointerException in O)->bool where {owner O}{return true;}
func tag(ref a:const NullPointerException in O,ref b:const NullPointerException in P)->i64 where {owner O,owner P}{return 0;}
func code(kind:bool)->u64{return 100u64;}
func code(kind:i64)->u64{return 200u64;}
func inspect(select:F,ref input:const NullPointerException in O,n:u64) where {F,owner O}{
 if(n==0u64){try{throw input;}catch(caught:const NullPointerException){return code(tag(input,select(input,caught)));};}
 else{return inspect(select,input,n-1u64);};
}
ptr absent:i64=null in here;
try{absent;{}}catch(slot:const NullPointerException){
 behave @print(inspect(first,slot,30u64));behave @print(inspect(second,slot,30u64));
};
'''
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
