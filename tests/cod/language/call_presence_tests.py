#!/usr/bin/env python3
"""Call outcomes retain input presence independently of payload initialization."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SIGNATURE = 'func checked(ptr candidate:i64 in O)->void throws NullPointerException where {owner O}'
CHECK = SIGNATURE+'{candidate;}'
CALLS = 'val item<-7,other<-11;ptr absent:i64=null in here;caller(item,other);caller(absent,other);caller(other,item);caller(item,absent);'
COVER = 'requires a covering catch'
runner.CASES = {}


def source(call='checked(p);', declarations=CHECK, before='', after='', target='p'):
    return IO+declarations+'proc caller(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{ '+before+\
        'try{ '+call+' }catch(e:const NullPointerException){behave @print(0);return;};'+after+\
        'behave @print('+target+'); }'+CALLS


def safe(name, output='7\n0\n11\n7\n', **kwargs):
    runner.CASES[name] = (source(**kwargs), 0, output, '')


def reject(name, **kwargs):
    runner.CASES[name] = (source(**kwargs), 2, '', COVER)


safe('direct')
safe('readonly', declarations=CHECK.replace('candidate:i64', 'candidate:const i64'))
safe('generic', declarations=CHECK.replace('candidate:i64', 'candidate:T').replace('where {owner O}', 'where {T,owner O}'))
safe('guard_without_payload_read', declarations=SIGNATURE+'{if(candidate is null){candidate;{}}else{};}')
safe('input_alias', before='ptr saved=p;', target='saved')
safe('slot_alias', before='var saved->p;', target='saved')
safe('local_alias', declarations=SIGNATURE+'{ptr selected=candidate;selected;}')
safe('checked_then_rebound', declarations=SIGNATURE+'{candidate;candidate=null;}')
safe('alias_checked_after_rebind', declarations=SIGNATURE+'{ptr saved=candidate;candidate=null;saved;}')
safe('cleanup_rebind', declarations=SIGNATURE+'{defer{candidate=null;{}};candidate;}')
safe('finally_rebind', declarations=SIGNATURE+'{try{candidate;{}}finally{candidate=null;{}};}')
safe('deferred_read', declarations=SIGNATURE+'{defer{candidate;{}};}')
safe('deferred_call', declarations=CHECK+SIGNATURE.replace('checked','relay')+'{defer checked(candidate);}', call='relay(p);')
safe('prepared_cleanup', call='{{defer checked(p);}};')
safe('delayed_cleanup', call='{{defer{checked(p);};}};')
safe('nested_scope', declarations=SIGNATURE+'{ {{candidate;}}; }')
safe('scoped_argument_binding', call='checked({{ptr selected=p;selected}});')
safe('scoped_prepared_argument_binding', call='{{defer checked({{ptr selected=p;selected}});}};')
reject('scoped_argument_replaced', call='checked({{ptr selected=p;p=q;selected}});')
safe('finally_checks', declarations=SIGNATURE+'{try{return;}finally{candidate;{}};}')
safe('callee_loop', declarations=SIGNATURE+'{for(i in 0u64..2u64){candidate;};}')
safe('caller_loop', call='for(i in 0u64..2u64){checked(p);};')
safe('loop_after_check', after='for(i in 0u64..2u64){behave @print(p);};', output='7\n7\n7\n0\n11\n11\n11\n7\n7\n7\n')
safe('recursive', declarations='func checked(ptr candidate:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){candidate;{}}else{checked(candidate,n-1u64);};}', call='checked(p,20u64);')
safe('mutual', declarations='func checked(ptr candidate:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){candidate;{}}else{relay(candidate,n-1u64);};}func relay(ptr candidate:i64 in O,n:u64)->void throws NullPointerException where {owner O}{checked(candidate,n);}', call='checked(p,20u64);')
safe('value_call', before='func selected=checked;', call='selected(p);')
safe('copied_callable', before='func first=checked;func selected=first;', call='selected(p);')
safe('captured_callable', before='func first=checked;func selected=func[first](ptr argument:i64 in P)->void throws NullPointerException where {owner P}{first(argument);};', call='selected(p);')
safe('higher_order', declarations=CHECK+'func apply(checker:F,ptr argument:i64 in O)->void throws NullPointerException where {F,owner O}{checker(argument);}', call='apply(checked,p);')
safe('all_callable_alternatives', declarations=CHECK+SIGNATURE.replace('checked','second')+'{if(candidate is null){candidate;{}}else{};}', before='func selected=if(p is null){checked}else{second};', call='selected(p);')
safe('write_initializes', declarations=SIGNATURE+'{candidate<-17;}', output='17\n0\n17\n17\n')
safe('symbolic_current', declarations=CHECK.replace('func checked','proc @checked')+'effect @checked;', call='behave @checked(p);')
safe('symbolic_captured', declarations=CHECK.replace('func checked','proc @checked')+'effect @checked;', call='perform @checked(p);')

REBOUND = 'func checked(ptr candidate:i64 in O,ptr replacement:i64 in O)->void throws NullPointerException where {owner O}{candidate=replacement;candidate;}'
safe('rebound_source', declarations=REBOUND, call='checked(p,q);', target='q', output='11\n11\n7\n0\n')
reject('rebound_original', declarations=REBOUND, call='checked(p,q);')
reject('other_input', target='q')
reject('rebound_caller', after='p=null;')
reject('rebound_alias', before='var saved->p;', after='saved=null;')
reject('cleanup_rebound_caller', after='{{defer{p=null;{}};}};')
reject('nullable_normal_exit', declarations=SIGNATURE+'{if(candidate is not null){candidate;{}}else{};}')
reject('conditional_initialization', declarations=SIGNATURE+'{if(candidate is not null){candidate<-17;{}}else{};}')
reject('empty_loop', declarations=SIGNATURE+'{for(i in 0u64..0u64){candidate;};}')
reject('one_unchecked_alternative', declarations=CHECK+SIGNATURE.replace('checked','unchecked')+'{}', before='func selected=if(p is null){checked}else{unchecked};', call='selected(p);')
reject('caught_null_continues', declarations=SIGNATURE+'{try{candidate;{}}catch(e:const NullPointerException){};}')
reject('finally_returns_after_null', declarations=SIGNATURE+'{try{candidate;{}}finally{return;};}')
reject('declared_only', declarations=SIGNATURE+'{}')
reject('callee_checks_fresh_target', declarations=SIGNATURE+'{val local<-3;local;}')
reject('later_operand_rebind', declarations='func checked(ptr candidate:i64 in O,ignored:bool)->void throws NullPointerException where {owner O}{candidate;}', call='checked(p,{p=null;true});')
safe('later_operand_keeps_input_alias', declarations='func checked(ptr candidate:i64 in O,ignored:bool)->void throws NullPointerException where {owner O}{candidate;}', before='ptr saved=p;', call='checked(p,{p=null;true});', target='saved')
reject('prepared_changed_selection', before='ptr saved=p;', call='{{defer checked(p);p=null;}};')
safe('prepared_input_snapshot', before='ptr saved=p;', call='{{defer checked(p);p=null;}};', target='saved')

SLOT = 'func (<=>)(ptr left:i64 in O,ptr right:i64 in O)->void throws NullPointerException where {owner O}{left;left=null;}'
reject('slot_replaces_checked_binding', declarations=SLOT, call='p<=>q;')
safe('slot_preserves_input_snapshot', declarations=SLOT, before='ptr saved=p;', call='p<=>q;', target='saved')
SLOT_OTHER = SLOT.replace('left=null;', 'left=right;')
reject('slot_does_not_validate_replacement', declarations=SLOT_OTHER, call='p<=>q;')
reject('prepared_slot_uses_late_selection', declarations=SLOT, before='ptr saved=p;', call='{{defer (<=>)(p,q);p=q;}};', target='saved')

NONREADING = SIGNATURE+'{if(candidate is null){candidate;{}}else{};}'
runner.CASES['presence_not_initialization'] = (IO+NONREADING+'proc caller()->void{val item<-make local i64;try{checked(item);}catch(e:const NullPointerException){return;};behave @print(item);}caller();', 2, '', 'before definite initialization')
runner.CASES['presence_does_not_read_payload'] = (IO+NONREADING+'val item<-make local i64;try{checked(item);}catch(e:const NullPointerException){};', 0, '', '')
runner.CASES['readonly_stays_readonly'] = (IO+CHECK.replace('candidate:i64','candidate:const i64')+'proc caller(ptr p:const i64 in O)->void where {owner O}{try{checked(p);}catch(e:const NullPointerException){return;};p<-9;}val item<-7;caller(item);', 2, '', 'readonly destination')
runner.CASES['declared_throw_still_required'] = (IO+CHECK+'val item<-7;checked(item);', 2, '', COVER)

FAILURE = 'using "std:exceptions" MissingReturnError;func missing()->u64{}'
for name, body, accepted in (
    ('failure_after_check', 'candidate;missing();', True),
    ('failure_before_check', 'missing();candidate;', False),
):
    text = IO+FAILURE+SIGNATURE+'{'+body+'}proc caller(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{'
    text += 'try{checked(p);}catch(e:const MissingReturnError){behave @print(p);return;}catch(e:const NullPointerException){behave @print(0);return;};behave @print(p);}'+CALLS
    runner.CASES[name] = (text, 0 if accepted else 2, '7\n0\n11\n7\n' if accepted else '', '[-Wmissing-return]' if accepted else COVER)

for name, parameter, accepted in (
    ('preparation_failure_before_check', 'ptr candidate:i64 in O', False),
    ('preparation_failure_after_read', 'candidate:i64', True),
):
    text = IO+FAILURE+'func checked('+parameter+',ignored:u64)->void'+(' where {owner O}' if not accepted else '')+'{}'
    text += 'proc caller(ptr p:i64 in O,ptr q:i64 in O)->void where {owner O}{try{checked(p,missing());}catch(e:const MissingReturnError){}catch(e:const NullPointerException){behave @print(0);return;};behave @print(p);}'+CALLS
    runner.CASES[name] = (text, 0 if accepted else 2, '7\n0\n11\n7\n' if accepted else '', '[-Wmissing-return]' if accepted else COVER)

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
