#!/usr/bin/env python3
"""Boolean call results retain conditional presence of their incoming targets."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SIGNATURE = 'func has(ptr a:i64 in O)->bool where {owner O}'
CHECK = SIGNATURE+'{return a is not null;}'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;'+''.join('behave @print(choose('+args+'));' for args in ('x,y','absent,y','x,absent','absent,absent'))
NPE = 'requires a covering catch'
runner.CASES = {}


def source(condition='has(p)', yes='p', no='0', definitions=CHECK, setup='', before=''):
    return IO+definitions+'proc choose(ptr p:i64 in O,ptr q:i64 in O)->i64 where {owner O}{'+setup+before+'if('+condition+'){return '+yes+';}else{return '+no+';};}'+CALLS


def safe(name, output='7\n0\n7\n0\n', **kwargs):
    runner.CASES[name] = (source(**kwargs), 0, output, '')


def reject(name, **kwargs):
    runner.CASES[name] = (source(**kwargs), 2, '', NPE)


safe('direct')
safe('generic', definitions=CHECK.replace('a:i64','a:T').replace('where {owner O}','where {T,owner O}'))
safe('readonly', definitions=CHECK.replace('a:i64','a:const i64'))
safe('false_result', definitions=SIGNATURE+'{return a is null;}', yes='0', no='p')
safe('negated_result', condition='!has(p)', yes='0', no='p')
safe('double_negation', condition='!!has(p)')
safe('stored', setup='val ok<-has(p);', condition='ok')
safe('copied_boolean', setup='val ok<-has(p);val saved<-ok;ok<-false;', condition='saved')
safe('overwritten_boolean', setup='val ok<-true;ok<-has(p);', condition='ok')
safe('guarded_expression', condition='try{has(p)}finally{}')
safe('scoped_expression', condition='{{return has(p);}}')
safe('scoped_argument', condition='has({ptr selected=p;{{defer{selected=q;};}};selected})',
     yes='q', output='11\n11\n0\n0\n')
TARGET = 'func target(ptr a:i64 in O)->ptr i64 in O where {owner O}{return a;}'
safe('borrowed_result_predicate', definitions=TARGET+SIGNATURE+'{return target(a) is not null;}')
safe('borrowed_result_argument', definitions=TARGET+CHECK, condition='has(target(p))')
safe('nullable_argument_projection', condition='has(if(q is null){null}else{p})', output='7\n0\n0\n0\n')
reject('unrelated_borrowed_result', definitions=TARGET+CHECK, condition='has(target(q))')
safe('forwarded', definitions=CHECK+SIGNATURE.replace('has','relay')+'{return has(a);}', condition='relay(p)')
safe('negated_forwarding', definitions=CHECK+SIGNATURE.replace('has','relay')+'{return !has(a);}', condition='!relay(p)')
safe('stored_in_helper', definitions=SIGNATURE+'{val ok<-a is not null;return ok;}')
safe('scoped_predicate', definitions=SIGNATURE+'{return {{ptr selected=a;return selected is not null;}};}')
safe('multiple_returns', definitions=SIGNATURE+'{if(a is null){return false;}else{return true;};}')
safe('local_alias', definitions=SIGNATURE+'{ptr selected=a;return selected is not null;}')
safe('saved_alias_after_local_rebind', definitions=SIGNATURE+'{ptr selected=a;a=null;return selected is not null;}')
safe('saved_predicate_after_local_rebind', definitions=SIGNATURE+'{val ok<-a is not null;a=null;return ok;}')
safe('saved_return_before_cleanup', definitions=SIGNATURE+'{defer{a=null;{}};return a is not null;}')
safe('saved_return_before_finally', definitions=SIGNATURE+'{try{return a is not null;}finally{a=null;{}};}')
safe('deferred_return', definitions=SIGNATURE+'{defer{return a is not null;};return false;}')
safe('finally_return', definitions=SIGNATURE+'{try{return false;}finally{return a is not null;};}')
safe('nested_cleanup', definitions=SIGNATURE+'{return {{defer{a=a;};return a is not null;}};}')
safe('helper_loop', definitions=SIGNATURE+'{for(i in 0u64..2u64){if(a is null){return false;}else{};};return true;}')
safe('caller_loop', setup='val ok<-false;for(i in 0u64..2u64){ok<-has(p);};', condition='ok')
safe('recursive', definitions='func has(ptr a:i64 in O,n:u64)->bool where {owner O}{if(n==0u64){return a is not null;}else{return has(a,n-1u64);};}', condition='has(p,20u64)')
safe('mutual', definitions='func has(ptr a:i64 in O,n:u64)->bool where {owner O}{if(n==0u64){return a is not null;}else{return relay(a,n-1u64);};}func relay(ptr a:i64 in O,n:u64)->bool where {owner O}{return has(a,n);}', condition='has(p,20u64)')
safe('callable_value', setup='func f=has;', condition='f(p)')
safe('callable_copy', setup='func f=has;func g=f;', condition='g(p)')
safe('captured_callable', setup='func f=has;func g[f](ptr a:i64 in P)->bool where {owner P}{return f(a);};', condition='g(p)')
safe('higher_order', definitions=CHECK+'func apply(f:F,ptr a:i64 in O)->bool where {F,owner O}{return f(a);}', condition='apply(has,p)')
safe('symbolic_current', definitions=CHECK.replace('func has','proc @has')+'effect @has;', condition='behave @has(p)')
safe('symbolic_captured', definitions=CHECK.replace('func has','proc @has')+'effect @has;', condition='perform @has(p)')

PAIR = 'func has(ptr a:i64 in O,ptr b:i64 in O)->bool where {owner O}'
safe('both_inputs', definitions=PAIR+'{return (a is not null)&&(b is not null);}', condition='has(p,q)', yes='p+q', output='18\n0\n0\n0\n')
safe('neither_absent', definitions=PAIR+'{return (a is null)||(b is null);}', condition='!has(p,q)', yes='p+q', output='18\n0\n0\n0\n')
safe('second_input', definitions=PAIR+'{a=b;return a is not null;}', condition='has(p,q)', yes='q', output='11\n11\n0\n0\n')
reject('rebound_input_is_distinct', definitions=PAIR+'{a=b;return a is not null;}', condition='has(p,q)')
reject('one_of_two_present', definitions=PAIR+'{return (a is not null)||(b is not null);}', condition='has(p,q)')
reject('constant_true', definitions=SIGNATURE+'{return true;}')
reject('unknown_true_alternative', definitions=PAIR+'{if(b is null){return true;}else{return a is not null;};}', condition='has(p,q)')
reject('opposite_condition', condition='has(p)', yes='0', no='p')
reject('other_input', yes='q')
reject('overwritten_stored_result', setup='val ok<-has(p);ok<-true;', condition='ok')
reject('caller_rebound', setup='val ok<-has(p);p=q;', condition='ok')
reject('caller_alias_rebound', setup='var alias->p;val ok<-has(p);alias=q;', condition='ok')
safe('caller_saved_input', setup='ptr saved=p;val ok<-has(p);p=q;', condition='ok', yes='saved')
reject('cleanup_rebound', setup='val ready<-{{defer{p=q;};return has(p);}};p=q;', condition='ready')
safe('cleanup_keeps_snapshot', setup='ptr saved=p;val ready<-{{defer{p=q;};return has(p);}};p=q;',
     condition='ready', yes='saved')
reject('finally_rebound', condition='try{has(p)}finally{p=q;{}}')
reject('finally_replaces_proof', definitions=SIGNATURE+'{try{return a is not null;}finally{return true;};}')
reject('defer_replaces_proof', definitions=SIGNATURE+'{defer{return true;};return a is not null;}')
reject('callee_checks_new_value', definitions=SIGNATURE+'{val local<-9;return local is not null;}')

LATER = 'func has(ptr a:i64 in O,ignored:bool)->bool where {owner O}{return a is not null;}'
reject('later_argument_changes_binding', definitions=LATER, condition='has(p,{p=q;true})')
safe('later_argument_keeps_snapshot', definitions=LATER, setup='ptr saved=p;', condition='has(p,{p=q;true})', yes='saved')
ALTERNATIVES = CHECK+SIGNATURE.replace('has','other')+'{return !(a is null);}'
safe('all_callable_alternatives', definitions=ALTERNATIVES, setup='func f=if(q is null){has}else{other};', condition='f(p)')
reject('one_callable_lacks_guarantee', definitions=CHECK+SIGNATURE.replace('has','other')+'{return true;}', setup='func f=if(q is null){has}else{other};', condition='f(p)')
SLOT = 'func (<=>)(ptr left:i64 in O,ptr right:i64 in O)->void where {owner O}{left=null;}'
safe('slot_preserves_input_snapshot', definitions=CHECK+SLOT, setup='ptr saved=p;', condition='{val ok<-has(p);p<=>q;ok}', yes='saved')
reject('slot_replacement_not_proven', definitions=CHECK+SLOT, condition='{val ok<-has(p);p<=>q;ok}')
safe('helper_slot_preserves_input', definitions=SLOT+SIGNATURE+'{val ok<-a is not null;a<=>a;return ok;}')

runner.CASES['presence_does_not_initialize'] = (IO+CHECK+'val x<-make local i64;if(has(x)){behave @print(x);}else{};',2,'','before definite initialization')
runner.CASES['readonly_stays_readonly'] = (IO+CHECK.replace('a:i64','a:const i64')+'proc choose(ptr p:const i64 in O)->void where {owner O}{if(has(p)){p<-1;}else{};}val x<-7;choose(x);',2,'','readonly destination')
runner.CASES['declared_failure_remains'] = (IO+CHECK.replace('->bool where','->bool throws NullPointerException where')+'val x<-7;has(x);',2,'',NPE)
runner.CASES['guard_without_read'] = (IO+CHECK+'val x<-make local i64;behave @print(has(x));',0,'true\n','')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
