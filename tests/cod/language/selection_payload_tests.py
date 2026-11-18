#!/usr/bin/env python3
"""Deferred payloads retain the entry reference selection until composition."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import ERROR, WARN

IO = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
SET = 'func set(ref out:bool in B,ptr p:T in O)->void where {T,owner B,owner O}{out<-p is not null;}'
CALLS = 'val x<-7,y<-11;ptr absent:i64=null in here;'+''.join(
    'behave @print(choose('+args+'));' for args in ('x,y','absent,y','x,absent','absent,absent'))
runner.CASES = {}


def case(name, work, setup='val ok<-false,other<-true;ref output=other;', yes='p', condition='ok',
         definitions=SET, output='7\n0\n7\n0\n', diagnostic=''):
    source = IO+definitions+'proc choose(ptr p:i64 in O,ptr q:i64 in O)->i64 where {owner O}{'+setup+work
    source += 'if('+condition+'){return '+yes+';}else{return 0;};}'+CALLS
    runner.CASES[name] = (source, 2 if diagnostic else 0, '' if diagnostic else output, diagnostic)


def reject(name, work, **kwargs):
    case(name, work, diagnostic='requires a covering catch', **kwargs)


case('direct_write', '{{defer{output<-p is not null;};output=ok;}};')
case('call_write', '{{defer{set(output,p);};output=ok;}};')
case('nested_block', '{{defer{defer{set(output,p);};};output=ok;}};')
case('nested_scope', '{{defer{ {{defer{set(output,p);};}}; };output=ok;}};')
case('prepared_inside_block', '{{defer{defer set(output,p);};output=ok;}};')
case('prepared_transfer_inside_block', '{{defer{defer (<-)(output,p is not null);};output=ok;}};')
case('cleanup_rebind', '{{defer{set(output,p);};defer{output=ok;{}};}};')
case('same_action_rebind', '{{defer{output=ok;set(output,p);};}};')
case('write_before_rebind', '{{defer{set(output,p);output=other;{}};output=ok;}};')
case('cleanup_rebind_after_write', '{{defer{output=other;{}};defer{set(output,p);};output=ok;}};')
case('aliased_binding', 'var alias->output;{{defer{set(alias,p);};output=ok;}};')
case('copied_ref', 'ref saved=output;{{defer{set(output,p);};output=ok;}};')
case('copy_from_selected', '{{defer{ok<-input;};input=source;}};',
     setup='val ok<-false,source<-p is not null,other<-true;ref input=other;')
case('borrow_from_selected', '{{defer{copy(ok,input);};input=source;}};',
     setup='val ok<-false,source<-p is not null,other<-true;ref input=other;',
     definitions='func copy(ref out:bool in B,ref value:const bool in B)->void where {owner B}{out<-value;}')
case('selected_read_write', '{{defer{output<-input;};input=source;output=ok;}};',
     setup='val ok<-false,source<-p is not null,other<-true;ref input=other,output=other;')
case('two_actions_copy', '{{defer{ok<-input;};defer{set(output,p);};input=other;output=other;}};',
     setup='val ok<-false,other<-true;ref output=ok,input=ok;')
case('two_actions_rebind_copy', '{{defer{ok<-input;};defer{input=other;{}};defer{set(output,p);};output=other;}};',
     setup='val ok<-false,other<-true;ref output=ok,input=ok;')
case('repeated_write', '{{for(i in 0u64..3u64){defer{set(output,p);};};output=ok;}};')
case('recursive_call', '{{defer{set(output,p,10u64);};output=ok;}};',
     definitions='func set(ref out:bool in B,ptr p:T in O,n:u64)->void where {T,owner B,owner O}{if(n==0u64){out<-p is not null;{}}else{set(out,p,n-1u64);};}')
case('captured_call', 'func write=set;{{defer{write(output,p);};output=ok;}};', definitions=SET.replace('p:T', 'p:i64').replace('{T,', '{'))
case('same_payload_alternatives', '{{defer{set(output,p);};output=if(q is null){ok}else{other};}};',
     setup='val ok<-p is not null,other<-p is not null;ref output=other;')

reject('wrong_selected_value', '{{defer{set(output,q);};output=ok;}};')
reject('write_to_other', '{{defer{set(output,p);};output=other;}};', setup='val ok<-true,other<-false;ref output=ok;')
reject('write_to_other_after_rebind', '{{defer{output=other;set(output,p);};output=ok;}};', setup='val ok<-true,other<-false;ref output=other;')
reject('later_overwrite', '{{defer{output<-true;};defer{set(output,p);};output=ok;}};')
reject('selected_input_overwrite', '{{defer{ok<-input;};input=other;}};',
       setup='val ok<-false,source<-p is not null,other<-true;ref input=source;')
reject('pointer_rebound', '{{defer{set(output,p);};output=ok;}};p=q;')
reject('copied_ref_keeps_old_target', 'ref saved=output;{{defer{set(saved,p);};output=ok;}};', setup='val ok<-true,other<-false;ref output=other;')
reject('possible_other_value', '{{defer{output<-true;};output=if(q is null){ok}else{other};}};', setup='val ok<-p is not null,other<-false;ref output=other;')
reject('conditional_registration', '{{if(q is null){defer{set(output,p);};}else{};output=ok;}};', setup='val ok<-true,other<-false;ref output=other;')
reject('zero_registrations', '{{for(i in 0u64..0u64){defer{set(output,p);};};output=ok;}};', setup='val ok<-true,other<-false;ref output=other;')

# Enumerate two writes separated by a rebind in actual cleanup order. The
# independent model computes the final cell, then whether its truth permits p.
states = [(True, True), (False, True), (True, False), (False, False)]
expressions = [('false', lambda p, q: False), ('true', lambda p, q: True),
               ('p is not null', lambda p, q: p), ('q is not null', lambda p, q: q)]
for first, (first_source, first_value) in enumerate(expressions):
    for second, (second_source, second_value) in enumerate(expressions):
        for destination in ('ok', 'other'):
            values = []
            for p, q in states:
                cells = dict(ok=False, other=True)
                cells['ok'] = second_value(p, q)
                cells[destination] = first_value(p, q)
                values.append(cells['ok'])
            safe = all(p or not value for (p, q), value in zip(states, values))
            work = '{{defer{output<-'+first_source+';};defer{output='+destination+';{}};defer{output<-'+second_source+';};output=ok;}};'
            case('write_rebind_model_'+str(first)+'_'+str(second)+'_'+destination, work,
                 output=''.join('7\n' if value else '0\n' for value in values),
                 diagnostic='' if safe else 'requires a covering catch')

# A covering root handler discharges the declared obligation. Actual category
# facts independently decide whether every normal path initializes the result.
EXCEPTION_SET = 'func set(ref out:Exception in O,value:Exception)->void where {owner O}{out<-value;}'
def exception_case(name, work, setup='val payload:Exception<-checked(),other:Exception<-checked();ref output=other;', good=True, helpers=''):
    source = 'using symbol "std:io" @print;'+ERROR+EXCEPTION_SET+helpers+'proc run()->i64{'+setup+work
    source += 'val result<-make local i64;try{throw payload;}catch(e:const MissingReturnError){result<-7;{}}catch(e:const Exception){};return result;}behave @print(run());'
    runner.CASES['exception_'+name] = (source, 0 if good else 2, '7\n' if good else '', WARN if good else 'before definite initialization')

exception_case('selected_write', '{{defer{output<-unchecked();};output=payload;}};')
exception_case('selected_call', '{{defer{set(output,unchecked());};output=payload;}};')
exception_case('nested_scope', '{{defer{ {{defer{set(output,unchecked());};}}; };output=payload;}};')
exception_case('selected_read', '{{defer{payload<-input;};input=other;}};',
               setup='val payload:Exception<-checked(),other:Exception<-unchecked();ref input=payload;')
exception_case('two_actions', '{{defer{payload<-input;};defer{set(output,unchecked());};output=other;input=other;}};',
               setup='val payload:Exception<-checked(),other:Exception<-checked();ref input=payload,output=payload;')
exception_case('other_target', '{{defer{output<-unchecked();};output=other;}};', good=False)
exception_case('later_overwrite', '{{defer{output<-checked();};defer{output<-unchecked();};output=payload;}};', good=False)
exception_case('borrowed_alias_write', 'write_alias(payload,payload);', good=False,
    helpers='func write_alias(ref left:Exception in O,ref right:Exception in P)->void where {owner O,owner P}{left<-unchecked();right<-checked();}')
exception_case('borrowed_alias_cleanup', 'write_alias(payload,payload);', good=False,
    helpers='func write_alias(ref left:Exception in O,ref right:Exception in P)->void where {owner O,owner P}{defer{left<-unchecked();right<-checked();};}')
for first in ('unchecked()', 'checked()'):
    for second in ('unchecked()', 'checked()'):
        for destination in ('payload', 'other'):
            selected = first if destination == 'payload' else second
            exception_case('write_rebind_'+first[:-2]+'_'+second[:-2]+'_'+destination,
                '{{defer{output<-'+first+';};defer{output='+destination+';{}};defer{output<-'+second+';};output=payload;}};',
                good=selected=='unchecked()')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('exception_projection_tests.py')]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_block',
    'prepared_inside_block',
    'prepared_transfer_inside_block',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
