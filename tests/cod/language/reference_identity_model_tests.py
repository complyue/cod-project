#!/usr/bin/env python3
"""Compare source observation with an independent owner/target identity model."""
from itertools import product
from pathlib import Path
import foundation_tests as runner

PRELUDE = 'using symbol "std:io" @print;func same(ptr lhs:T in A,ptr rhs:U in B)->bool where {T,U,owner A,owner B}{return lhs is rhs;}'
runner.CASES = {}
for index, (clear_outer, clear_inner, reverse) in enumerate(product((False,True), repeat=3)):
    bindings = {'p': (0,'x'), 'q': (0,'y'), 'r': (0,None), 's': (1,'z'), 't': (1,None)}
    source = PRELUDE+'val x<-1,y<-1;ptr p=x,q=y,r:i64=null in here;{{val z<-1;ptr s=z,t:i64=null in here;'
    if clear_outer: source+='p=null;'; bindings['p']=(0,None)
    if clear_inner: source+='s=null;'; bindings['s']=(1,None)
    expected=[]
    for a,b in product(bindings,repeat=2):
        left,right=(b,a) if reverse else (a,b)
        same=bindings[left]==bindings[right]
        source+='behave @print('+left+' is '+right+');behave @print(same('+left+','+right+'));'
        expected += [str(same).lower()]*2
    # RHS rebinding cannot retroactively select the left target. Owners stay fixed.
    for destination, replacement in [('p','q'),('p','r'),('s','t'),('q','p')]:
        selected=bindings[destination]
        bindings[destination]=bindings[replacement]
        source+='behave @print('+destination+' is {'+destination+'='+replacement+';'+replacement+'});'
        expected.append(str(selected==bindings[replacement]).lower())
    source+='}};'
    runner.CASES['identity_'+str(index)]=(source,0,'\n'.join(expected)+'\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
