#!/usr/bin/env python3
"""Variadic declarations and explicit fixed contracts have identical native programs."""
from pathlib import Path
import declaration_result_equivalence_tests as runner

IO='using symbol "std:io" @print;'

def cases():
    for name,declaration,definition,result,body,output in (
        ('empty_tail','func f(rest:u64...)->u64','func f()','->u64',
         '{return 7u64;}behave @print(f());','7\n'),
        ('fixed_pair','func f(rest:u64...)->u64','func f(x:u64,y:u64)','->u64',
         '{return x+y;}behave @print(f(1u8,2u8));','3\n'),
        ('generic','func f(first:T,rest:T...)->const T where T','func f(x:U,y:U)','->const U',
         ' where U{return y;}behave @print(f(1u64,2u64));behave @print(f(false,true));','2\ntrue\n'),
        ('recursive','func f(rest:u64...)->u64','func f(n:u64)','->u64',
         '{if(n==0u64){return 0u64;}else{return n+f(n-1u64);};}behave @print(f(30u64));','465\n'),
        ('deeper_recursion','func f(rest:u64...)->u64','func f(n:u64)','->u64',
         '{if(n==0u64){return 0u64;}else{return n+f(n-1u64);};}behave @print(f(300u64));','45150\n'),
        ('borrowed_result','func f(ref rest:i64 in A...)->ref i64 in A where {owner A}',
         'func f(ref x:i64 in O,ref y:i64 in O)','->ref i64 in O',
         ' where {owner O}{defer{y<-7;};return y;}val x<-1,y<-2;ref z=f(x,y);z<-8;behave @print(y);','8\n'),
        ('complete_result','func f(rest:u64...)->func(u64)->u64','func f(x:u64,y:u64)','->func(u64)->u64',
         '{return func[](x:u64)->u64{return x;};}func g=f(1u64,2u64);behave @print(g(7u64));','7\n'),
    ):
        explicit=IO+runner.blank(declaration+';')+definition+result+body
        inherited=IO+declaration+';'+definition+' '*len(result)+body
        assert len(explicit)==len(inherited)
        yield name,explicit,inherited,output

runner.cases=cases
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
