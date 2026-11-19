#!/usr/bin/env python3
"""Conditional selected writes preserve initialization across borrowed aliases."""
from pathlib import Path
import selected_initialization_tests as fixture

runner=fixture.runner
local_cases=dict(runner.CASES)
runner.CASES={}
HEADER='proc run(flag:bool,other:bool)->void{'+fixture.fixture.LOCAL
BORROWED='proc run(flag:bool,other:bool,ptr a:u64 in O,ptr b:u64 in O,ptr c:u64 in O)->void where {owner O}{ptr p=if(flag){a}else{b};ptr q=if(other){a}else{b};'


def borrowed(source, aliases=False):
    definitions,body=source.split(HEADER,1)
    body=body.removesuffix(fixture.fixture.CALLS).replace('null in here','null in O')
    calls=''
    for first,second in ((False,False),(False,True),(True,False),(True,True)):
        calls+='{{val left<-make local u64,right<-make local u64,third<-make local u64;'
        calls+='run('+str(first).lower()+','+str(second).lower()+',left,'+('left' if aliases else 'right')+',third);}};'
    return definitions+BORROWED+body+calls


for name,(source,code,output,diagnostic) in local_cases.items():
    runner.CASES[name]=(borrowed(source),code,output,diagnostic)
for name in ('protected_registration','protected_conditional_write','protected_conditional_recovery','protected_both_conditional','action_no_write','recovery_no_write','read_before_action_write','failed_action_write'):
    source,code,output,diagnostic=local_cases[name]
    runner.CASES['aliased_'+name]=(borrowed(source,True),code,'' if code else '7\n7\n7\n7\n',diagnostic)

# Unknown runtime choices cover every target; caller aliases determine whether
# the selected write guarantees initialization of the observed input.
from itertools import product
SELECTOR='func write_choice(ptr a:u64 in O,ptr b:u64 in O,ptr c:u64 in O,first:bool,second:bool)->void throws NullPointerException where {owner O}{ptr selected=if(first){a}else{if(second){b}else{c}};selected<-7u64;{}}'
for aliases in product(range(3),repeat=3):
    names=('left','right','third')
    actual=','.join(names[index] for index in aliases)
    safe=len(set(aliases))==1
    source=fixture.fixture.IO+SELECTOR+'proc run(first:bool,second:bool)->void{val left<-make local u64,right<-make local u64,third<-make local u64;'
    source+='try{write_choice('+actual+',first,second);behave @print('+names[aliases[0]]+');}catch(e:const NullPointerException){behave @print(99u64);};return;}'+fixture.fixture.CALLS
    runner.CASES['origin_alias_model_'+''.join(map(str,aliases))]=(source,0 if safe else 2,'7\n7\n7\n7\n' if safe else '', '' if safe else 'initialization')

WRITER='func write_choice(ptr a:u64 in O,ptr b:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{ptr selected=if(flag){a}else{b};selected<-7u64;{}}'
OBSERVER='proc run(flag:bool)->void{val left<-make local u64,right<-make local u64;try{CALL;behave @print(left);}catch(e:const NullPointerException){behave @print(99u64);};return;}run(false);run(true);'
for name,definition,call,rejected in (
    ('aliased_output',WRITER,'write_choice(left,left,flag)',False),
    ('distinct_output',WRITER,'write_choice(left,right,flag)',True),
    ('conditional_alias_output',WRITER.replace('selected<-7u64;','if(flag){selected<-7u64;{}}else{};'),'write_choice(left,left,flag)',True),
    ('unknown_alias_output',WRITER.replace('selected<-7u64;','{};'),'write_choice(left,left,flag)',True),
    ('generic_alias_output',WRITER.replace('u64','T').replace('in O,flag:bool','in O,value:T,flag:bool').replace('7T','value').replace('where {owner O}','where {T,owner O}'),'write_choice(left,left,7u64,flag)',False),
    ('recursive_alias_output',WRITER.replace('flag:bool)','flag:bool,n:u64)').replace('ptr selected=', 'if(n>0u64){write_choice(a,b,flag,n-1u64);return;}else{};ptr selected='),'write_choice(left,left,flag,8u64)',False),
    ('recursive_without_write',WRITER.replace('ptr selected=if(flag){a}else{b};selected<-7u64;', 'if(flag){write_choice(a,b,false);}else{};'),'write_choice(left,left,flag)',True),
):
    runner.CASES[name]=(fixture.fixture.IO+definition+OBSERVER.replace('CALL',call),2 if rejected else 0,'' if rejected else '7\n7\n','initialization' if rejected else '')

PICK='func pick(ptr a:u64 in O,ptr b:u64 in O,flag:bool)->ptr u64 in O where {owner O}{return if(flag){a}else{b};}'
for name,selection in (('returned_origins','pick(a,b,flag)'),('direct_origins','if(flag){a}else{b}')):
    definition=PICK+WRITER.replace('if(flag){a}else{b}',selection)
    runner.CASES[name]=(fixture.fixture.IO+definition+OBSERVER.replace('CALL','write_choice(left,left,flag)'),0,'7\n7\n','')

runner.CASE_ARGUMENTS={}
runner.CASE_SHADOWS={name:[] for name in runner.CASES}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(fixture.fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
