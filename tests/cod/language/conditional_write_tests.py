#!/usr/bin/env python3
"""Conditional writes and reads compose through scopes and callable boundaries."""
from pathlib import Path
import re
import foundation_tests as runner
from exception_projection_tests import ERROR, WARN

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SET = 'proc set(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){x<-7u64;{}}else{};}'
READ = 'proc read(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){behave @print(x);}else{};}'
WRITE = 'if(flag){x<-7u64;{}}else{};'
LOCAL = 'val x<-make local u64;'
BORROWED_TARGET = 'val storage<-make local u64;ptr x:u64=storage;'
OBSERVE = 'behave @print(x);'
runner.CASES = {}


def case(name, body, declarations='', output='7\n', reject=False, warning='', diagnostic='initialization'):
    for original, actual in [('x','destination'), ('flag','condition'), ('payload','failure')]:
        body = re.sub(r'\b'+original+r'\b', actual, body)
    prefix = IO.replace('using "std:exceptions" NullPointerException;', '') if 'using "std:exceptions" NullPointerException;' in declarations else IO
    runner.CASES[name] = (prefix+declarations+body, 2 if reject else 0,
                          '' if reject else output, diagnostic if reject else warning)


def call(name, operation='set(x,true)', declarations=SET, setup=LOCAL, observe=OBSERVE, **kwargs):
    case(name, setup+'try { '+operation+';'+observe+' }catch(e:const NullPointerException){};', declarations, **kwargs)


case('scope_true', BORROWED_TARGET+'val flag<-true;{{defer{'+WRITE+'};}};'+OBSERVE)
case('scope_false', BORROWED_TARGET+'val flag<-false;{{defer{'+WRITE+'};}};'+OBSERVE, reject=True)
case('scope_late_true', BORROWED_TARGET+'val flag<-false;{{defer{'+WRITE+'};flag<-true;}};'+OBSERVE)
case('scope_late_false', BORROWED_TARGET+'val flag<-true;{{defer{'+WRITE+'};flag<-false;}};'+OBSERVE, reject=True)
case('scope_nested', BORROWED_TARGET+'val flag<-true;{{defer{defer{'+WRITE+'};};}};'+OBSERVE)
case('scope_iteration', BORROWED_TARGET+'val flag<-true;for(i in 0u64..1u64){ {{defer{'+WRITE+'};}}; };'+OBSERVE)
case('scope_zero', BORROWED_TARGET+'val flag<-true;{{for(i in 0u64..0u64){defer{'+WRITE+'};};}};'+OBSERVE, reject=True)
case('scope_snapshot', BORROWED_TARGET+'val flag<-true;{{defer{'+WRITE+'flag<-false;};}};'+OBSERVE)
case('scope_snapshot_false', BORROWED_TARGET+'val flag<-false;{{defer{'+WRITE+'flag<-true;};}};'+OBSERVE, reject=True)
case('scope_unknown', 'run(true);', 'proc run(flag:bool)->void{'+BORROWED_TARGET+'{{defer{'+WRITE+'};}};'+OBSERVE+'}', reject=True)
call('call_true')
call('call_false', operation='set(x,false)', reject=True)
call('call_value', setup='proc action=set;'+LOCAL, operation='action(x,true)')
call('call_forwarded', declarations=SET+'proc forward(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{set(x,flag);}', operation='forward(x,true)')
call('call_recursive', declarations='proc set(ptr x:u64 in O,flag:bool,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){'+WRITE+'}else{set(x,flag,n-1u64);};}', operation='set(x,true,8u64)')
call('call_recursive_false', declarations='proc set(ptr x:u64 in O,flag:bool,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){'+WRITE+'}else{set(x,flag,n-1u64);};}', operation='set(x,false,8u64)', reject=True)
call('call_generic', declarations='proc set(ptr x:T in O,value:T,flag:bool)->void throws NullPointerException where {T,owner O}{if(flag){x<-value;{}}else{};}', operation='set(x,7u64,true)')
call('call_snapshot', declarations=SET.replace(WRITE,WRITE+'flag<-false;'))
call('call_snapshot_false', declarations=SET.replace(WRITE,WRITE+'flag<-true;'), operation='set(x,false)', reject=True)
call('call_borrowed_condition', declarations=SET.replace('flag:bool','ref flag:const bool in B').replace('owner O}','owner O,owner B}'), setup=LOCAL+'val flag<-true;', operation='set(x,flag)')
call('call_prepared_value', setup=BORROWED_TARGET+'val flag<-true;', operation='{{defer set(x,flag);flag<-false;}}')
call('call_prepared_value_false', setup=BORROWED_TARGET+'val flag<-false;', operation='{{defer set(x,flag);flag<-true;}}', reject=True)
call('call_prepared_borrow', declarations=SET.replace('flag:bool','ref flag:const bool in B').replace('owner O}','owner O,owner B}'), setup=BORROWED_TARGET+'val flag<-false;', operation='{{defer set(x,flag);flag<-true;}}')
call('call_prepared_borrow_false', declarations=SET.replace('flag:bool','ref flag:const bool in B').replace('owner O}','owner O,owner B}'), setup=BORROWED_TARGET+'val flag<-true;', operation='{{defer set(x,flag);flag<-false;}}', reject=True)
call('conditional_read_skipped', declarations=READ, operation='read(x,false)', observe='', output='')
call('conditional_read_required', declarations=READ, operation='read(x,true)', observe='', reject=True)
call('conditional_write_then_read', declarations=SET.replace(WRITE,WRITE+OBSERVE), observe='', output='7\n')
call('conditional_write_then_read_false', declarations=SET.replace(WRITE,WRITE+OBSERVE), operation='set(x,false)', observe='', reject=True)
call('read_before_conditional_write', declarations=SET.replace(WRITE,OBSERVE+WRITE), reject=True)
call('conditional_read_forwarded', declarations=READ+'proc forward(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{read(x,flag);}', operation='forward(x,false)', observe='', output='')
call('conditional_read_deferred', declarations=READ, setup=BORROWED_TARGET, operation='{{defer read(x,false);}}', observe='', output='')
case('ref_promise_not_conditional', LOCAL+'read(x,false);', 'proc read(ref x:u64 in O,flag:bool)->void where {owner O}{if(flag){behave @print(x);}else{};}', reject=True)
call('copied_argument_snapshot', declarations=SET.replace('flag:bool','flag:bool,ignored:bool'), setup=LOCAL+'val flag<-true;', operation='set(x,flag,{flag<-false;false})')
call('copied_argument_false', declarations=SET.replace('flag:bool','flag:bool,ignored:bool'), setup=LOCAL+'val flag<-false;', operation='set(x,flag,{flag<-true;true})', reject=True)
call('borrowed_argument_entry', declarations=SET.replace('flag:bool','ref flag:const bool in B,ignored:bool').replace('owner O}','owner O,owner B}'), setup=LOCAL+'val flag<-false;', operation='set(x,flag,{flag<-true;true})')
call('borrowed_argument_false', declarations=SET.replace('flag:bool','ref flag:const bool in B,ignored:bool').replace('owner O}','owner O,owner B}'), setup=LOCAL+'val flag<-true;', operation='set(x,flag,{flag<-false;false})', reject=True)
call('selected_alias', setup=LOCAL+'ptr alias=x;', operation='set(alias,true)')
call('complementary_aliases', declarations='proc set(ptr a:u64 in O,ptr b:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){a<-7u64;{}}else{b<-7u64;{}};}', operation='set(x,x,true)')
call('different_aliases', declarations='proc set(ptr a:u64 in O,ptr b:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){a<-7u64;{}}else{b<-7u64;{}};}', setup=LOCAL+'val y<-make local u64;', operation='set(x,y,false)', reject=True)
call('failed_registration_no_write', setup=BORROWED_TARGET, operation='try{{defer set(x,true);}}catch(e){}', reject=True)

# The guard's input determines the write; a later assignment cannot change it.
for incoming in (False, True):
    for outgoing in (False, True):
        label=str(int(incoming))+'_'+str(int(outgoing))
        call('input_model_'+label, declarations=SET.replace(WRITE,WRITE+'flag<-'+str(outgoing).lower()+';'),
             operation='set(x,'+str(incoming).lower()+')', reject=not incoming)

CATEGORY = 'try{throw payload;}catch(e:const MissingReturnError){x<-7u64;{}}catch(e:const Exception){};'
for value in ('unchecked','checked'):
    good=value=='unchecked'
    case('scope_category_'+value, BORROWED_TARGET+'val payload:Exception<-'+value+'();{{defer{'+CATEGORY+'};}};'+OBSERVE,
         ERROR, reject=not good, warning=WARN)
    call('call_category_'+value, declarations=ERROR+'proc set(ptr x:u64 in O,payload:Exception)->void throws NullPointerException where {owner O}{'+CATEGORY+'}',
         operation='set(x,'+value+'())', reject=not good, warning=WARN)

case('declared_failure_not_waived', LOCAL+'read(x,false);', READ, reject=True, diagnostic='requires a covering catch')
call('read_replaced_by_return', declarations='proc read(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{try{if(flag){behave @print(x);}else{};}finally{return;};}', operation='read(x,true)', observe='', reject=True)
call('skipped_read_replaced_by_return', declarations='proc read(ptr x:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{try{if(flag){behave @print(x);}else{};}finally{return;};}', operation='read(x,false)', observe='', output='')
call('call_unknown_condition', declarations=SET+'proc run(flag:bool)->void throws NullPointerException{'+LOCAL+'set(x,flag);'+OBSERVE+'}', operation='run(true)', setup='', observe='', reject=True)

ALIASED = 'proc paired(ptr a:u64 in O,ptr b:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){a<-7u64;{}}else{};behave @print(b);}'
call('conditional_alias_read', declarations=ALIASED, operation='paired(x,x,true)', observe='')
call('conditional_alias_read_false', declarations=ALIASED, operation='paired(x,x,false)', observe='', reject=True)
call('conditional_alias_read_other', declarations=ALIASED, setup=LOCAL+'val y<-make local u64;', operation='paired(x,y,true)', observe='', reject=True)
call('conditional_alias_read_order', declarations=ALIASED.replace('if(flag){a<-7u64;{}}else{};behave @print(b);','behave @print(b);if(flag){a<-7u64;{}}else{};'), operation='paired(x,x,true)', observe='', reject=True)
call('conditional_alias_read_prepared', declarations=ALIASED, setup=BORROWED_TARGET, operation='{{defer paired(x,x,true);}}', output='7\n7\n')
call('callable_alternatives_write', declarations=SET+SET.replace('proc set','proc other').replace('7u64','8u64')+'proc run(choice:bool)->void throws NullPointerException{'+LOCAL+'proc action=if(choice){set}else{other};action(x,true);'+OBSERVE+'}', setup='', operation='run(true);run(false)', observe='', output='7\n8\n')
call('callable_alternative_no_write', declarations=SET+SET.replace('proc set','proc other').replace(WRITE,'')+'proc run(choice:bool)->void throws NullPointerException{'+LOCAL+'proc action=if(choice){set}else{other};action(x,true);'+OBSERVE+'}', setup='', operation='run(true)', observe='', reject=True)

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('exception_projection_tests.py')]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'scope_nested',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
