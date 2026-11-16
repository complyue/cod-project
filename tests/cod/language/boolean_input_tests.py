#!/usr/bin/env python3
"""Boolean input snapshots preserve caller predicates through ordinary results."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
ECHO = 'func echo(flag:bool)->bool{return flag;}'
BORROW = 'func echo(ref flag:const bool in B)->bool where {owner B}{return flag;}'
VIEW = 'func echo(ref flag:const bool in B)->ref const bool in B where {owner B}{return flag;}'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;'+''.join(
    'behave @print(choose('+args+'));' for args in ('x,y','absent,y','x,absent','absent,absent'))
runner.CASES = {}


def case(name, definitions=ECHO, setup='', condition='echo(p is not null)', yes='p', no='0',
         output='7\n0\n7\n0\n', diagnostic=''):
    source = IO+definitions+'proc choose(ptr p:i64 in O,ptr q:i64 in O)->i64 where {owner O}{'+setup
    source += 'if('+condition+'){return '+yes+';}else{return '+no+';};}'+CALLS
    runner.CASES[name] = (source, 2 if diagnostic else 0, '' if diagnostic else output, diagnostic)


def reject(name, **kwargs):
    case(name, diagnostic='requires a covering catch', **kwargs)


case('copy')
case('generic', definitions='func echo(flag:T)->const T where T{return flag;}')
case('nested', condition='echo(echo(p is not null))')
case('negated', definitions='func echo(flag:bool)->bool{return !flag;}', condition='!echo(p is not null)')
case('false_result', condition='echo(p is null)', yes='0', no='p')
case('input_true_branch', definitions='func echo(flag:bool)->bool{if(flag){return true;}else{return false;};}')
case('input_false_branch', definitions='func echo(flag:bool)->bool{if(!flag){return false;}else{return true;};}')
case('stored_result', setup='val ok<-echo(p is not null);', condition='ok')
case('saved_argument', setup='val ok<-p is not null;', condition='echo(ok)')
case('saved_input_copy', definitions='func echo(flag:bool)->bool{val saved<-flag;flag<-true;return saved;}')
case('local_boolean_write', definitions='func echo(flag:bool)->bool{val result<-false;result<-flag;return result;}')
case('negated_local_write', definitions='func echo(flag:bool)->bool{val result<-false;result<-!flag;return !result;}')
case('merged_local', definitions='func echo(flag:bool,branch:bool)->bool{val result<-false;if(branch){result<-flag;}else{result<-!!flag;};return result;}', condition='echo(p is not null,q is not null)')
case('scoped_input', condition='echo({{val ok<-p is not null;ok}})')
case('scoped_result', definitions='func echo(flag:bool)->bool{return {{val saved<-flag;saved}};}')
case('two_inputs', definitions='func echo(a:bool,b:bool)->bool{return a&&b;}', condition='echo(p is not null,q is not null)', yes='p+q', output='18\n0\n0\n0\n')
case('disjunction_false', definitions='func echo(a:bool,b:bool)->bool{return a||b;}', condition='!echo(p is null,q is null)', yes='p+q', output='18\n0\n0\n0\n')
case('argument_reordering', definitions='func echo(a:bool,b:bool)->bool{return b;}', condition='echo(q is not null,p is not null)')
case('input_and_target_facts', definitions='func echo(flag:bool,ptr target:i64 in P)->bool where {owner P}{return flag&&(target is not null);}', condition='echo(p is not null,q)', yes='p+q', output='18\n0\n0\n0\n')
case('recursive', definitions='func echo(flag:bool,n:u64)->bool{if(n==0u64){return flag;}else{return echo(flag,n-1u64);};}', condition='echo(p is not null,20u64)')
case('mutual', definitions='func echo(flag:bool,n:u64)->bool{if(n==0u64){return flag;}else{return relay(flag,n-1u64);};}func relay(flag:bool,n:u64)->bool{return echo(flag,n);}', condition='echo(p is not null,20u64)')
case('generic_recursive', definitions='func echo(flag:T,n:u64)->const T where T{if(n==0u64){return flag;}else{return echo(flag,n-1u64);};}', condition='echo(p is not null,20u64)')
case('callable_value', setup='func f=echo;', condition='f(p is not null)')
case('captured_callable', setup='func f=echo;func g[f](flag:bool)->bool{return f(flag);};', condition='g(p is not null)')
case('higher_order', definitions=ECHO+'func apply(f:F,flag:bool)->bool where F{return f(flag);}', condition='apply(echo,p is not null)')
case('symbol_current', definitions=ECHO.replace('func echo','proc @echo')+'effect @echo;', condition='behave @echo(p is not null)')
case('symbol_captured', definitions=ECHO.replace('func echo','proc @echo')+'effect @echo;', condition='perform @echo(p is not null)')
case('callable_alternatives', definitions=ECHO+'func other(flag:bool)->bool{return !!flag;}', setup='func f=if(q is null){echo}else{other};', condition='f(p is not null)')
case('saved_return_before_cleanup', definitions='func echo(flag:bool)->bool{defer{flag<-true;};return flag;}')
case('saved_return_before_finally', definitions='func echo(flag:bool)->bool{try{return flag;}finally{flag<-true;{}};}')
case('replacement_return', definitions='func echo(flag:bool)->bool{try{return true;}finally{return flag;};}')
case('caller_loop', setup='val ok<-false;for(i in 0u64..2u64){ok<-echo(p is not null);};', condition='ok')
case('borrowed_input', definitions=BORROW, setup='val ok<-p is not null;', condition='echo(ok)')
case('writable_borrowed_input', definitions=BORROW.replace('const bool','bool'), setup='val ok<-p is not null;', condition='echo(ok)')
case('nullable_borrowed_input', definitions=BORROW.replace('ref flag','ptr flag').replace('where {','throws NullPointerException where {'), setup='val ok<-p is not null;', condition='try{echo(ok)}catch(e:const NullPointerException){false}')
case('borrowed_saved_snapshot', definitions='func echo(ref flag:bool in B)->bool where {owner B}{val saved<-flag;flag<-true;return saved;}', setup='val ok<-p is not null;', condition='echo(ok)')
case('borrowed_cleanup_snapshot', definitions='func echo(ref flag:bool in B)->bool where {owner B}{defer{flag<-true;};return flag;}', setup='val ok<-p is not null;', condition='echo(ok)')
case('readonly_borrowed_result', definitions=VIEW, setup='val ok<-p is not null;', condition='echo(ok)')
case('borrowed_result_copy', definitions=VIEW, setup='val ok<-p is not null;val saved<-echo(ok);ok<-true;', condition='saved')
case('borrowed_result_alias', definitions=VIEW, setup='val ok<-p is not null;ref saved=echo(ok);', condition='saved')
case('borrowed_result_forwarding', definitions=VIEW+BORROW.replace('echo','relay'), setup='val ok<-p is not null;', condition='relay(echo(ok))')
case('borrowed_result_multiple_targets', definitions='func echo(ref a:const bool in B,ref b:const bool in B,choice:bool)->ref const bool in B where {owner B}{if(choice){return a;}else{return b;};}', setup='val a<-p is not null,b<-p is not null;', condition='echo(a,b,q is not null)')
case('copy_before_later_write', definitions='func echo(flag:bool,ignored:bool)->bool{return flag;}', setup='val ok<-p is not null;', condition='echo(ok,{ok<-true;true})')
case('borrow_after_later_write', definitions='func echo(ref flag:const bool in B,ignored:bool)->bool where {owner B}{return flag;}', setup='val ok<-true;', condition='echo(ok,{ok<-p is not null;true})')
case('borrow_keeps_selected_target', definitions=BORROW.replace(')->bool',',ignored:bool)->bool'), setup='val ok<-p is not null,other<-true;ref selected=ok;', condition='echo(selected,{selected=other;true})')
case('saved_pointer_after_rebind', setup='ptr saved=p;val ok<-p is not null;p=q;', condition='echo(ok)', yes='saved')
case('input_fact_after_callee_rebind', definitions='func echo(flag:bool,ptr target:i64 in P)->bool where {owner P}{target=null;return flag;}', condition='echo(p is not null,p)')
case('aliased_read_before_write', definitions='func echo(ref flag:const bool in B,ref other:bool in B)->bool where {owner B}{val saved<-flag;other<-true;return saved;}', setup='val ok<-p is not null;', condition='echo(ok,ok)')
case('write_from_other_input', definitions='func echo(ref flag:bool in B,copy:bool)->bool where {owner B}{flag<-copy;return flag;}', setup='val ok<-true;', condition='echo(ok,p is not null)')

reject('constant_result', definitions='func echo(flag:bool)->bool{return true;}')
reject('overwritten_input', definitions='func echo(flag:bool)->bool{flag<-true;return flag;}')
reject('unknown_result_alternative', definitions='func echo(flag:bool,choice:bool)->bool{if(choice){return flag;}else{return true;};}', condition='echo(p is not null,q is not null)')
reject('wrong_input', definitions='func echo(a:bool,b:bool)->bool{return b;}', condition='echo(p is not null,q is not null)')
reject('disjunctive_result', definitions='func echo(a:bool,b:bool)->bool{return a||b;}', condition='echo(p is not null,q is not null)')
reject('cleanup_replaces_result', definitions='func echo(flag:bool)->bool{try{return flag;}finally{return true;};}')
reject('defer_replaces_result', definitions='func echo(flag:bool)->bool{defer{return true;};return flag;}')
reject('rebound_caller_pointer', setup='val ok<-p is not null;p=q;', condition='echo(ok)')
reject('later_argument_rebind', definitions='func echo(flag:bool,ignored:bool)->bool{return flag;}', condition='echo(p is not null,{p=q;true})')
reject('later_write_invalidates_borrow', definitions=BORROW.replace(')->bool',',ignored:bool)->bool'), setup='val ok<-p is not null;', condition='echo(ok,{ok<-true;true})')
reject('overwritten_borrow', definitions='func echo(ref flag:bool in B)->bool where {owner B}{flag<-true;return flag;}', setup='val ok<-p is not null;', condition='echo(ok)')
reject('overlapping_borrow_write', definitions='func echo(ref flag:const bool in B,ref other:bool in B)->bool where {owner B}{other<-true;return flag;}', setup='val ok<-p is not null;', condition='echo(ok,ok)')
reject('borrowed_result_overwrite', definitions=VIEW, setup='val ok<-p is not null;ref saved=echo(ok);ok<-true;', condition='saved')
reject('borrowed_result_cleanup', definitions='func echo(ref flag:bool in B)->ref bool in B where {owner B}{defer{flag<-true;};return flag;}', setup='val ok<-p is not null;', condition='echo(ok)')
reject('borrowed_result_other_target', definitions=VIEW, setup='val ok<-q is not null;', condition='echo(ok)')
reject('callable_alternative_unknown', definitions=ECHO+'func other(flag:bool)->bool{return true;}', setup='func f=if(q is null){echo}else{other};', condition='f(p is not null)')
reject('declared_failure', definitions=ECHO.replace('->bool{','->bool throws NullPointerException{'))
case('input_proof_not_initialization', setup='val x<-make local i64;ptr present=x;', condition='echo(present is not null)', yes='present', diagnostic='before definite initialization')

# Enumerate nonconstant two-input truth tables independently of checker facts.
# Each minterm uses ordinary short-circuit expressions in a separate helper.
states = [(True, True), (False, True), (True, False), (False, False)]
for mask in range(1, 15):
    terms = ['('+('a' if a else '!a')+'&&'+('b' if b else '!b')+')'
             for i, (a, b) in enumerate(states) if mask & (1 << i)]
    definitions = 'func echo(a:bool,b:bool)->bool{return '+'||'.join(terms)+';}'
    for target, name, payload in ((0, 'p', 7), (1, 'q', 11)):
        valid = all(state[target] for i, state in enumerate(states) if mask & (1 << i))
        output = ''.join(str(payload if mask & (1 << i) else 0)+'\n' for i in range(4))
        case('truth_table_'+str(mask)+'_'+name, definitions=definitions,
             condition='echo(p is not null,q is not null)', yes=name, output=output,
             diagnostic='' if valid else 'requires a covering catch')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
