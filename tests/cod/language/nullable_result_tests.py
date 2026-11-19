#!/usr/bin/env python3
"""Borrowed results retain nullness conditions through calls and completion."""
from pathlib import Path
import nullable_alias_tests as fixture

runner = fixture.runner
runner.CASES = {}
RELAY = 'func relay(ptr p:const Exception in O)->ptr const Exception in O where {owner O}{return p;}'
OTHER = 'func relay(ptr p:const Exception in O,ref other:const Exception in O)->ptr const Exception in O where {owner O}{return other;}'


def case(name, declaration=RELAY, copy='ptr saved=relay(selected);', output=None, **kwargs):
    fixture.case(name, copy=copy, **kwargs)
    source, code, expected, diagnostic = runner.CASES[name]
    runner.CASES[name] = (declaration+source, code, expected if output is None else output, diagnostic)


case('direct')
case('source_rebound', after='selected=payload;')
case('source_cleared', after='selected=null;')
case('source_slot_rebound', before='var writer->selected;', after='writer=payload;')
case('result_rebound', after='saved=payload;', reject=True)
case('result_strong_slot_update', copy='ptr saved:const Exception=payload;var writer->saved;writer=relay(selected);')
case('result_weak_slot_update', copy='ptr saved:const Exception=payload;ptr alternate:const Exception=selected;var writer->if(other){saved}else{alternate};writer=relay(selected);', extra=True, reject=True)
case('result_chain', copy='ptr first=relay(selected);ptr saved=relay(first);')
case('generic', declaration=RELAY.replace('Exception','T').replace('where {owner O}', 'where {T,owner O}'))
case('recursive', declaration=RELAY.replace('in O)', 'in O,n:u64)').replace('{return p;}', '{if(n>0u64){return relay(p,n-1u64);}else{};return p;}'), copy='ptr saved=relay(selected,8u64);')
case('callable', copy='func forward=relay;ptr saved=forward(selected);')
case('captured_callable', copy='func forward[relay](ptr p:const Exception in O)->ptr const Exception in O where {owner O}{return relay(p);};func called=forward;ptr saved=called(selected);')
case('saved_callee_selection', declaration=RELAY.replace('return p;', 'ptr saved=p;p=null;return saved;'))
case('callee_independent_target', declaration=OTHER, copy='ptr saved=relay(selected,payload);', reject=True)
case('callee_conditional_target', declaration=OTHER.replace('in O)->', 'in O,choose:bool)->').replace('return other;', 'if(choose){return p;}else{return other;}'), copy='ptr saved=relay(selected,payload,other);', extra=True, reject=True)
case('callee_independent_null', declaration=RELAY.replace('return p;', 'p=null;return p;'), output='8\n8\n8\n8\n')
case('callee_conditional_null', declaration=RELAY.replace('in O)->', 'in O,choose:bool)->').replace('return p;', 'if(choose){return p;}else{return null in O;}'), copy='ptr saved=relay(selected,other);', extra=True, output='8\n8\n8\n8\n8\n8\n8\n7\n')
case('conditional_result', copy='ptr saved=if(other){relay(selected)}else{relay(selected)};', extra=True)
case('conditional_result_and_copy', copy='ptr saved=if(other){relay(selected)}else{selected};', extra=True)
case('conditional_independent_result', copy='ptr saved=if(other){relay(selected)}else{payload};', extra=True, reject=True)
case('scoped_result', copy='ptr saved={{relay(selected)}};')
case('finalized_result', copy=fixture.OBSERVE+'ptr saved=try{relay(selected)}finally{selected=null;{}};', observe='')
case('cleanup_preserves_selection', declaration=RELAY.replace('return p;', 'try{return p;}finally{p=null;{}};'))
case('cleanup_replaces_selection', declaration=OTHER.replace('return other;', 'try{return p;}finally{return other;}'), copy='ptr saved=relay(selected,payload);', reject=True)
case('cleanup_replaces_with_null', declaration=RELAY.replace('return p;', 'try{return p;}finally{return null in O;}'), output='8\n8\n8\n8\n')
case('deferred_result', declaration=RELAY.replace('return p;', 'defer{return p;};'))
case('deferred_replacement', declaration=OTHER.replace('return other;', 'defer{return other;};return p;'), copy='ptr saved=relay(selected,payload);', reject=True)
case('argument_snapshot', declaration=RELAY.replace('in O)', 'in O,ignored:u64)'), copy=fixture.OBSERVE+'ptr saved=relay(selected,{selected=null;0u64});', observe='')
case('nested_classifier', copy='ptr saved=selected;', observe='val ready<-classify(relay(selected));')
case('nested_classifier_known_payload', copy='ptr saved=selected;', observe='val ready<-classify(relay(payload));')
case('nested_classifier_independent_null', copy='ptr saved=selected;ptr absent:Exception=null in here;', observe='val ready<-classify(relay(absent));', reject=True)
case('loop_rebinds_source', after='for(i in 0u64..2u64){selected=payload;};')
case('loop_rebinds_result', after='for(i in 0u64..2u64){saved=payload;};', reject=True)

case('handler_result', copy='ptr saved=try{if(other){throw checked();}else{};relay(selected)}catch(e:const Exception){relay(selected)};', extra=True)
case('handler_independent_result', copy='ptr saved=try{if(other){throw checked();}else{};relay(selected)}catch(e:const Exception){payload};', extra=True, reject=True)
case('explicit_recovery_result', declaration=RELAY.replace('return p;', 'try{try{return p;}finally{throw checked();}}catch(e:const Exception){return p;};'))
case('local_finally_recovery', declaration=RELAY.replace('return p;', 'try{return p;}finally{try{throw checked();}catch(e:const Exception){};}'))

for name, change, rejected in (
    ('result_guard_ref', '', False),
    ('result_guard_changed_source', 'selected=null;', True),
    ('result_guard_independent_source', 'selected=if(other){item}else{null in here};', True),
):
    relay=RELAY.replace('Exception','u64')
    source=fixture.IO+relay+fixture.REF+'proc run(flag:bool,other:bool)->void{val item<-7u64;ptr selected=if(flag){item}else{null in here};ptr saved=relay(selected);'+change
    source+='if(saved is not null){behave @print(read(selected));}else{behave @print(8u64);};}'
    source+='run(true,false);run(false,true);'
    runner.CASES[name]=(source,2 if rejected else 0,'' if rejected else '7\n8\n','proven non-null' if rejected else '')

for name, operand, declaration, rejected in (
    ('raw_result_guard', 'selected', RELAY.replace('Exception','u64'), False),
    ('raw_result_guard_rebound', 'selected,{selected=null;0u64}', RELAY.replace('Exception','u64').replace('in O)', 'in O,ignored:u64)'), True),
):
    source=fixture.IO+declaration+fixture.REF+'proc run(flag:bool)->void{val item<-7u64;ptr selected=if(flag){item}else{null in here};'
    source+='if(relay('+operand+') is not null){behave @print(read(selected));}else{behave @print(8u64);};}'
    source+='run(true);run(false);'
    runner.CASES[name]=(source,2 if rejected else 0,'' if rejected else '7\n8\n','proven non-null' if rejected else '')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
