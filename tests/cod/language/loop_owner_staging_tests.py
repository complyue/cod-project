#!/usr/bin/env python3
"""Loop-derived result evidence preserves one shared recursive implementation."""
from pathlib import Path
import scoped_owner_staging_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
func first(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in A where {owner A,owner B}{return a;}
func second(ref a:const i64 in A,ref b:const i64 in B)->ref const i64 in B where {owner A,owner B}{return b;}
func tag(ref a:const i64 in O,ref b:const i64 in O)->bool where {owner O}{return true;}
func tag(ref a:const i64 in O,ref b:const i64 in P)->i64 where {owner O,owner P}{return 0;}
func code(kind:bool)->u64{return 100u64;}
func code(kind:i64)->u64{return 200u64;}
func inspect(select:F,ref input:const i64 in O,n:u64) where {F,owner O}{
 if(n==0u64){for(index in 0..1){return code(tag(input,select(input,index)));};}
 else{return inspect(select,input,n-1u64);};
}
val slot<-1;
behave @print(inspect(first,slot,30u64));behave @print(inspect(second,slot,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
