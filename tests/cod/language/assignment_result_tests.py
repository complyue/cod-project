#!/usr/bin/env python3
"""Assignment results retain target identity without exporting binding slots."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
BASE = 'val x<-1,y<-7;ptr p=x,q=y;'
BORROWED_TARGETS = 'ref x:i64=1,y:i64=7;ptr p=x,q=y;'
runner.CASES = {}
for spelling in ('infix','explicit','intrinsic'):
    def rebind(left, right):
        return '{ '+left+'='+right+' }' if spelling == 'infix' else ('intrinsic ' if spelling == 'intrinsic' else '')+'(=)('+left+','+right+')'
    result = rebind('p','y')
    for name, source, diagnostic in [
        ('rebind', rebind(result,'x')+';', 'requires a binding slot'),
        ('swap', '{ '+result+'}<=>q;', 'requires a binding slot'),
        ('alias', 'ptr var alias->'+result+';', 'alias of a non-binding storage place'),
        ('conditional', '{if(true){ '+result+'}else{q}}=x;', 'requires a binding slot'),
        ('conditional_reverse', '{if(true){q}else{ '+result+'}}=x;', 'requires a binding slot'),
        ('scope', '{{return '+result+';}}=x;', 'storage destination'),
        ('finally', '{try{ '+result+'}finally{}}=x;', 'requires a binding slot'),
        ('deferred', 'defer (=)('+result+',x);', 'requires a binding slot'),
    ]:
        runner.CASES[spelling+'_'+name] = (BASE+source,2,'',diagnostic)
    runner.CASES[spelling+'_new_binding'] = (IO+BASE+'ptr other='+result+';other=x;'+OUT('p')+OUT('other'),0,'7\n1\n','')
    runner.CASES[spelling+'_right_chain'] = (IO+BASE+rebind('q',result)+';'+OUT('p')+OUT('q'),0,'7\n7\n','')
    runner.CASES[spelling+'_write_target'] = (IO+BASE+'{ '+result+'}<-9;'+OUT('y'),0,'9\n','')
    runner.CASES[spelling+'_swap_bits'] = (IO+BASE+'{ '+result+'}<->x;'+OUT('x')+OUT('y')+OUT('p'),0,'7\n1\n1\n','')
    runner.CASES[spelling+'_retained_target'] = (IO+BASE+'ref target='+result+';p=x;target<-9;'+OUT('p')+OUT('y'),0,'1\n9\n','')
    runner.CASES[spelling+'_cleanup_target'] = (IO+BASE+'{try{ '+result+'}finally{p=x;{}}}<-9;'+OUT('p')+OUT('y'),0,'1\n9\n','')
    runner.CASES[spelling+'_deferred_target'] = (IO+BORROWED_TARGETS+'{{defer (<-)('+result+',9);p=x;}};'+OUT('p')+OUT('y'),0,'1\n9\n','')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
