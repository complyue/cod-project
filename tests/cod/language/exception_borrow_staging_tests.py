#!/usr/bin/env python3
"""Borrowed exception declarations preserve native code and recursive reuse."""
from pathlib import Path
import operator_declaration_staging_tests as runner

IO = runner.IO
TYPES = 'using "std:exceptions" SystemException;using "std:exceptions" NullPointerException;'
ERROR = 'func error()->const NullPointerException{ptr absent:u64=null in here;try{absent;{}}catch(caught:const NullPointerException){return caught;};return error();}'
runner.CASE_SHADOWS = {}
runner.CASES = (
    ('reference', 'func f(ref input:T in A)->ref T in A where {T,owner A};',
     '->ref SystemException in O', ' where {owner O}{',
     IO+TYPES+'func f(ref input:SystemException in O)->ref SystemException in O where {owner O}{return input;}'+ERROR+'func held=f;val common:SystemException<-error();behave @print(held(common).category);','4\n'),
    ('nullable', 'func f(ptr input:T in A)->ptr T in A where {T,owner A};',
     '->ptr SystemException in O', ' where {owner O}{',
     IO+TYPES+'func f(ptr input:SystemException in O)->ptr SystemException in O where {owner O}{return input;}func held=f;ptr absent:SystemException=null in here;behave @print(isnull(held(absent)));','true\n'),
    ('copied_result', 'func f(ref input:const T in A)->const T where {T,owner A};',
     '->const SystemException', ' where {owner O}{',
     IO+TYPES+'func f(ref input:const SystemException in O)->const SystemException where {owner O}{return input;}'+ERROR+'func held=f;val common:SystemException<-error();behave @print(held(common).category);','4\n'),
)
runner.RECURSIVE = IO+TYPES+'''
func (+)(ref left:const T in A,ref right:const T in B)->ref const T in B where {T,owner A,owner B};
func (+)(ref left:const U in X,ref right:const U in Y) where {U,owner X,owner Y}{return right;}
func relay(ref input:const T in O,n:u64)->const T where {T,owner O}{
 if(n==0u64){return input+input;}else{return relay(input,n-1u64);};
}
func code(input:SystemException)->u64{return 7u64;}
'''+ERROR+'''
val common:SystemException<-error(),flag<-true;
behave @print(code(relay(common,30u64)));behave @print(relay(flag,30u64));
'''
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
