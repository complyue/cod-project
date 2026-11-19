#!/usr/bin/env python3
"""Borrowed boolean writes share finite call and cleanup payload evidence."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SET = 'func set(ref out:bool in B,ptr p:i64 in O)->void where {owner B,owner O}{out<-p is not null;}'
COPY = 'func copy(ref out:bool in B,flag:bool)->void where {owner B}{out<-flag;}'
BORROW = 'func copy(ref out:bool in B,ref flag:const bool in C)->void where {owner B,owner C}{out<-flag;}'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;'+''.join(
    'behave @print(choose('+args+'));' for args in ('x,y','absent,y','x,absent','absent,absent'))
VALUE_OUTPUT = 'val ok<-false;'
BORROWED_OUTPUT = 'ref ok:bool=false;'
runner.CASES = {}


def case(name, work='set(ok,p);', definitions=SET, setup=VALUE_OUTPUT, condition='ok', yes='p',
         output='7\n0\n7\n0\n', diagnostic=''):
    source = IO+definitions+'proc choose(ptr p:i64 in O,ptr q:i64 in O)->i64 where {owner O}{'+setup+work
    source += 'if('+condition+'){return '+yes+';}else{return 0;};}'+CALLS
    runner.CASES[name] = (source, 2 if diagnostic else 0, '' if diagnostic else output, diagnostic)


def reject(name, **kwargs):
    case(name, diagnostic='requires a covering catch', **kwargs)


case('direct')
case('generic', definitions=SET.replace('p:i64','p:T').replace('where {','where {T,'))
case('false_result', definitions=SET.replace('p is not null','p is null'), condition='!ok')
case('copied_input', definitions=COPY, work='copy(ok,p is not null);')
case('borrowed_input', definitions=BORROW, setup='val ok<-false,source<-p is not null;', work='copy(ok,source);')
case('local_saved_input', definitions=COPY.replace('out<-flag;', 'val saved<-flag;flag<-true;out<-saved;'), work='copy(ok,p is not null);')
case('writable_no_write', definitions='func keep(ref flag:bool in B)->void where {owner B}{}', setup='val ok<-p is not null;', work='keep(ok);')
case('writable_borrowed_result', definitions='func set(ref out:bool in B,ptr p:i64 in O)->ref bool in B where {owner B,owner O}{out<-p is not null;return out;}', work='', condition='set(ok,p)')
case('saved_return_separate_output', definitions='func set(ref out:bool in B,ptr p:i64 in O)->bool where {owner B,owner O}{out<-p is not null;defer{out<-true;};return out;}', work='val saved<-set(ok,p);', condition='saved')
case('cleanup_writes_output', definitions=SET.replace('out<-p is not null;', 'defer{out<-p is not null;};'))
case('finally_writes_output', definitions=SET.replace('out<-p is not null;', 'try{}finally{out<-p is not null;{}};'))
case('borrowed_return_after_cleanup', definitions='func set(ref out:bool in B,ptr p:i64 in O)->ref bool in B where {owner B,owner O}{defer{out<-p is not null;};return out;}', work='', condition='set(ok,p)')
case('branch_writes', definitions=SET.replace('out<-p is not null;', 'if(p is null){out<-false;}else{out<-true;};'))
case('recursive', definitions='func set(ref out:bool in B,ptr p:T in O,n:u64)->void where {T,owner B,owner O}{if(n==0u64){out<-p is not null;{}}else{set(out,p,n-1u64);};}', work='set(ok,p,20u64);')
case('mutual', definitions='func set(ref out:bool in B,ptr p:i64 in O,n:u64)->void where {owner B,owner O}{if(n==0u64){out<-p is not null;{}}else{relay(out,p,n-1u64);};}func relay(ref out:bool in B,ptr p:i64 in O,n:u64)->void where {owner B,owner O}{set(out,p,n);}', work='set(ok,p,20u64);')
case('callable', work='func write=set;write(ok,p);')
case('captured_callable', work='func write=set;func use[write](ref out:bool in B,ptr target:i64 in P)->void where {owner B,owner P}{write(out,target);};use(ok,p);')
case('higher_order', definitions=SET+'func apply(f:F,ref out:bool in B,ptr target:i64 in P)->void where {F,owner B,owner P}{f(out,target);}', work='apply(set,ok,p);')
case('current_symbol', definitions=SET.replace('func set','proc @set')+'effect @set;', work='behave @set(ok,p);')
case('captured_symbol', definitions=SET.replace('func set','proc @set')+'effect @set;', work='perform @set(ok,p);')
case('callable_alternatives', definitions=SET+SET.replace('set(','other(').replace('p is not null','!(p is null)'), work='func write=if(q is null){set}else{other};write(ok,p);')
case('output_copy', work='set(ok,p);val saved<-ok;ok<-true;', condition='saved')
case('output_alias', setup='val ok<-false;ref output=ok;', work='set(output,p);')
case('saved_pointer', setup='val ok<-false;ptr saved=p;', work='set(ok,p);p=q;', yes='saved')
case('ordered_pointer_snapshot', definitions=SET.replace(')->void',',ignored:bool)->void'), setup='val ok<-false;ptr saved=p;', work='set(ok,p,{p=q;true});', yes='saved')
case('ordered_value_snapshot', definitions=COPY.replace(')->void',',ignored:bool)->void'), setup='val ok<-false,source<-p is not null;', work='copy(ok,source,{source<-true;true});')
case('ordered_borrow_entry', definitions=BORROW.replace(')->void',',ignored:bool)->void'), setup='val ok<-false,source<-true;', work='copy(ok,source,{source<-p is not null;true});')
case('alias_same_input_output', definitions=BORROW, setup='val ok<-p is not null;', work='copy(ok,ok);')
case('failure_postcondition', definitions=SET.replace(')->void',',ptr other:i64 in O)->void throws NullPointerException').replace('out<-p is not null;', 'out<-p is not null;other;{}'), work='try{set(ok,p,q);}catch(e:const NullPointerException){};')
case('loop_call', work='for(i in 0u64..3u64){set(ok,p);};')

MAYBE = 'func maybe(ref out:bool in B,flag:bool,change:bool)->void where {owner B}{if(change){out<-flag;{}}else{};}'
for choice in ('true', 'false', 'q is not null'):
    for prepared in (False, True):
        call = 'maybe(ok,p is not null,'+choice+');'
        suffix = choice.replace(' ', '_')+('_prepared' if prepared else '')
        case('preserve_or_write_'+suffix, definitions=MAYBE, setup='val ok<-p is not null;',
             work='{{defer '+call+'}};' if prepared else call)
        reject('preserve_or_write_unknown_'+suffix, definitions=MAYBE, setup='val ok<-true;',
               work=('{{defer '+call+'}};' if prepared else call).replace('p is not null', 'q is not null'))
case('recursive_preserve_or_write', definitions=MAYBE.replace('change:bool', 'change:bool,n:u64').replace(
    'if(change)', 'if(n>0u64){maybe(out,flag,change,n-1u64);}else{};if(change)'),
    setup='val ok<-p is not null;', work='maybe(ok,p is not null,q is not null,20u64);')
case('branch_alias_preserve', definitions='func maybe(ref out:bool in B,ref other:bool in B,flag:bool,change:bool)->void where {owner B}{ref target=if(change){out}else{other};target<-flag;}',
    setup='val ok<-p is not null,other<-p is not null;', work='maybe(ok,other,p is not null,q is not null);')

case('prepared_call', setup=BORROWED_OUTPUT, work='{{defer set(ok,p);}};')
case('deferred_block', setup=BORROWED_OUTPUT, work='{{defer{ok<-p is not null;};}};')
case('deferred_block_call', setup=BORROWED_OUTPUT, work='{{defer{set(ok,p);};}};')
case('nested_deferred_block', work='{{defer{defer{ok<-p is not null;};};}};')
case('prepared_copy', setup=BORROWED_OUTPUT, definitions=COPY, work='{{defer copy(ok,p is not null);}};')
case('prepared_transfer', setup=BORROWED_OUTPUT, work='{{defer (<-)(ok,p is not null);}};')
case('prepared_swap', setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer (<->)(ok,source);}};')
case('prepared_value_snapshot', definitions=COPY, setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer copy(ok,source);source<-true;}};')
case('prepared_borrow_entry', definitions=BORROW, setup=BORROWED_OUTPUT+'val source<-true;', work='{{defer copy(ok,source);source<-p is not null;}};')
case('prepared_target_snapshot', setup='ref ok:bool=false,other:bool=true;ref output=ok;', work='{{defer set(output,p);output=other;}};')
case('rebound_target_direct', setup='val ok<-false,other<-true;ref output=other;', work='output=ok;set(output,p);')
case('deferred_target_checked', setup='ref ok:bool=false,other:bool=true;ref output=other;', work='{{defer{set(output,p);};output=ok;}};', yes='try{p}catch(e:const NullPointerException){-1}')
case('ordered_cleanup', setup=BORROWED_OUTPUT, work='{{defer{ok<-p is not null;};defer{ok<-true;};}};')
case('cleanup_copy_chain', setup=BORROWED_OUTPUT+'val source<-false;', work='{{defer{ok<-source;};defer{source<-p is not null;};}};')
case('cleanup_prepared_chain', definitions=COPY+BORROW.replace('copy(','borrow('), setup=BORROWED_OUTPUT+'val source<-false;', work='{{defer borrow(ok,source);defer copy(source,p is not null);}};')
case('cleanup_input_snapshot', definitions=COPY, setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer copy(ok,source);defer{source<-true;};}};')
case('prepared_loop', setup=BORROWED_OUTPUT, work='for(i in 0u64..3u64){ {{defer set(ok,p);}};};')
case('repeated_cleanup', setup=BORROWED_OUTPUT, work='{{for(i in 0u64..3u64){defer set(ok,p);};}};')
case('repeated_block_cleanup', setup=BORROWED_OUTPUT, work='{{for(i in 0u64..3u64){defer{ok<-p is not null;};};}};')
case('cleanup_saved_pointer', setup=BORROWED_OUTPUT+'ptr saved=p;', work='{{defer set(ok,p);p=q;}};', yes='saved')
for name, operator, before, after, check in (
    ('saved_assignment_source', '=', 'p', 'q', case),
    ('independent_saved_assignment_source', '=', 'q', 'p', reject),
    ('live_swap_source', '<=>', 'q', 'p', case),
    ('independent_live_swap_source', '<=>', 'p', 'q', reject),
):
    check(name, setup=f'ptr side:i64=null in O;ptr source:i64 in O={before};',
          work=f'val ok<-{{{{defer{{return side is not null;}};defer ({operator})(side,source);source={after};return false;}}}};')
case('captured_null_keeps_read_unreachable', setup='ptr side:i64=null in O;ptr source:i64=null in O;',
     work='val ok<-{{defer{return side is not null;};defer (=)(side,source);source=p;return false;}};',
     output='0\n0\n0\n0\n')

PICK_REFERENCE = 'ptr source:u64 in O=if(flag){input}else{peer};'
GUARDED_REFERENCE_CALLS = (
    'ref first:u64=5u64,second:u64=6u64;ptr absent:u64=null in here;'
    + ''.join('behave @print(inspect('+args+'));'
              for args in ('first,second,true','absent,second,true','first,second,false','absent,second,false'))
)
for name, initial, operator, before, after, repeat, succeeds in (
    ('guarded_saved_assignment', 'null in O', '=', '', 'source=peer;', False, True),
    ('guarded_live_swap', 'null in O', '<=>', '', '', False, True),
    ('guarded_live_swap_after_slot_update', 'null in O', '<=>', 'source=peer;', 'source=if(flag){input}else{peer};', False, True),
    ('repeated_guarded_live_swap', 'null in O', '<=>', '', '', True, True),
    ('guard_reassigned_after_target_selection', 'null in O', '=', 'flag<-!flag;', '', False, False),
    ('guarded_failed_registration_can_read_peer', 'peer', '=', '', '', False, False),
):
    registration = 'if(flag){defer ('+operator+')(side,source);{}}else{};'
    if repeat:
        registration = 'for(i in 0u64..3u64){'+registration+'};'
    source = (IO+'func inspect(ptr input:u64 in O,ptr peer:u64 in O,flag:bool)->u64 where {owner O}{'
              'val present<-{{ptr side:u64 in O='+initial+';'+PICK_REFERENCE+'defer{return side is not null;};'
              +before+registration+after+'return false;}};return if(present){input}else{7u64};}'
              +GUARDED_REFERENCE_CALLS)
    runner.CASES[name] = (source, 0, '5\n7\n7\n7\n', '') if succeeds else (
        source, 2, '', 'conditional scalar value requires a proven non-null target')

# Independent registration decisions preserve the selected input and LIFO order.
guards = tuple(range(1, 9))
guard_parameters = ''.join(f',gate{i}:bool' for i in guards)
registrations = ''.join(f'if(gate{i}){{defer work({i}u64);{{}}}}else{{}};' for i in guards)
for placement, setup in (
    ('early', PICK_REFERENCE+registrations),
    ('late', registrations+PICK_REFERENCE),
):
    source = (IO+'proc work(label:u64)->void{behave @print(label);}'
              'proc inspect(ptr input:u64 in O,ptr peer:u64 in O,flag:bool'+guard_parameters+')->u64 where {owner O}{'
              'val present<-{{ptr side:u64=null in O;defer{return side is not null;};'
              +setup+'if(flag){defer (<=>)(side,source);{}}else{};'
              'return false;}};return if(present){input}else{7u64};}'
              'ref item:u64=5u64,other:u64=6u64;ptr absent:u64=null in here;'
              'behave @print(inspect(item,other,true'+',true'*len(guards)+'));'
              'behave @print(inspect(absent,other,false'+',true'*len(guards)+'));')
    order = ''.join(f'{i}\n' for i in reversed(guards))
    runner.CASES['independent_cleanup_guards_'+placement] = (source, 0, order+'5\n'+order+'7\n', '')

case('deferred_return_payload', definitions='func echo(flag:bool)->bool{defer{return flag;};return true;}', setup='', work='', condition='echo(p is not null)')

reject('constant_output', definitions=SET.replace('p is not null','true'))
reject('wrong_pointer', work='set(ok,q);')
reject('changed_pointer', work='set(ok,p);p=q;')
reject('changed_output', work='set(ok,p);ok<-true;')
reject('unknown_output_alternative', definitions=SET.replace('out<-p is not null;', 'if(p is null){out<-true;}else{out<-p is not null;};'))
reject('borrowed_result_replaced', definitions='func set(ref out:bool in B,ptr p:i64 in O)->ref bool in B where {owner B,owner O}{out<-p is not null;defer{out<-true;};return out;}', work='', condition='set(ok,p)')
reject('ordered_borrow_overwrite', definitions=BORROW.replace(')->void',',ignored:bool)->void'), setup='val ok<-false,source<-p is not null;', work='copy(ok,source,{source<-true;true});')
reject('callee_alias_overwrite', definitions='func set(ref out:bool in B,ref alias:bool in B,ptr p:i64 in O)->void where {owner B,owner O}{out<-p is not null;alias<-true;}', work='set(ok,ok,p);')
reject('caller_alias_overwrite', setup='val ok<-false;ref alias=ok;', work='set(ok,p);alias<-true;')
reject('callable_unknown_alternative', definitions=SET+SET.replace('set(','other(').replace('p is not null','true'), work='func write=if(q is null){set}else{other};write(ok,p);')
reject('declared_failure', definitions=SET.replace(')->void',')->void throws NullPointerException'))
reject('cleanup_overwrite', setup=BORROWED_OUTPUT, work='{{defer{ok<-true;};defer set(ok,p);}};')
reject('source_pointer_rebound_after_cleanup', setup=BORROWED_OUTPUT, work='{{defer set(ok,p);}};p=q;')
case('cleanup_local_pointer_rebind', setup=BORROWED_OUTPUT+'ptr saved=p;',
     work='{{defer{p=q;{}};defer set(ok,p);}};', yes='saved')
reject('prepared_borrow_overwrite', definitions=BORROW, setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer copy(ok,source);source<-true;}};')
reject('deferred_borrow_overwrite', setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer{ok<-source;};source<-true;}};')
reject('cleanup_bad_chain', setup=BORROWED_OUTPUT+'val source<-p is not null;', work='{{defer{ok<-source;};defer{source<-true;};}};')
case('deferred_rebound_target', setup='ref ok:bool=false,other:bool=true;ref output=other;', work='{{defer{set(output,p);};output=ok;}};')
reject('repeated_cleanup_bad', setup=BORROWED_OUTPUT, work='{{for(i in 0u64..3u64){defer set(ok,q);};}};')
case('initialization_independent', setup='val x<-make local i64;ptr target=x;val ok<-false;', work='set(ok,target);', yes='target', diagnostic='before definite initialization')
case('readonly_output', setup='const val ok<-false;', diagnostic='readonly argument')

# Validate output facts against all nonconstant two-input truth tables.
states = [(True, True), (False, True), (True, False), (False, False)]
for mask in range(1, 15):
    terms = ['('+('a' if a else '!a')+'&&'+('b' if b else '!b')+')'
             for i, (a, b) in enumerate(states) if mask & (1 << i)]
    definitions = 'func set(ref out:bool in B,a:bool,b:bool)->void where {owner B}{out<-'+'||'.join(terms)+';}'
    for target, name, payload in ((0, 'p', 7), (1, 'q', 11)):
        valid = all(state[target] for i, state in enumerate(states) if mask & (1 << i))
        output = ''.join(str(payload if mask & (1 << i) else 0)+'\n' for i in range(4))
        for prepared in (False, True):
            call = 'set(ok,p is not null,q is not null);'
            case('truth_table_'+str(mask)+'_'+name+('_prepared' if prepared else ''), definitions=definitions,
                 setup=BORROWED_OUTPUT if prepared else VALUE_OUTPUT,
                 work='{{defer '+call+'}};' if prepared else call, yes=name, output=output,
                 diagnostic='' if valid else 'requires a covering catch')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_deferred_block',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
