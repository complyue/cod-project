#!/usr/bin/env python3
"""Copied reference observations survive binding mutation and remain distinct from slots."""
from itertools import product
from pathlib import Path
import nullable_predicate_tests as fixture

runner = fixture.runner
base = fixture.fixture
runner.CASES = {}
SELECT = fixture.SELECT
OBSERVE = 'val ready<-classify(selected);'
FIRST = base.FIRST.replace('classify(payload)', 'ready')
TAIL = 'if(saved is null){target<-8u64;{}}else{'+base.TAIL+'};'
ARGUMENTS = tuple(('unchecked()' if category else 'checked()')+','+str(present).lower()
                  for present,category in product((False,True),(False,True)))


def case(name, copy='ptr saved=selected;', before='', after='', tail=TAIL, reject=False, extra=False, observe=OBSERVE):
    body = base.LOCAL+SELECT+copy+before+observe+after+FIRST+tail+base.PRINT
    arguments = tuple(arg+','+str(other).lower() for arg in ARGUMENTS for other in (False,True)) if extra else ARGUMENTS
    base.case(name, body=body, declarations=fixture.CLASSIFY, signature='payload:Exception,present:bool'+(',other:bool' if extra else ''),
              arguments=arguments, reject=reject, output='8\n8\n8\n8\n8\n8\n7\n7\n' if extra else '8\n8\n8\n7\n')


case('copy')
case('source_rebound', after='selected=payload;')
case('source_cleared', after='selected=null;')
case('source_slot_alias_rebound', before='var writer->selected;', after='writer=payload;')
case('copy_chain', copy='ptr middle=selected;ptr saved=middle;')
case('intermediate_rebound', copy='ptr middle=selected;ptr saved=middle;middle=payload;')
case('scoped_copy', copy='ptr saved={{ptr local=selected;local}};')
case('saved_changed', after='saved=payload;', reject=True)
case('saved_slot_alias_rebound', before='var writer->saved;', after='writer=payload;', reject=True)
case('source_changed_before_observation', before='selected=null;', reject=True)
case('boolean_changed', after='ready<-false;', reject=True)
case('independent_selection', copy='ptr saved=if(other){payload}else{null in here};', reject=True, extra=True)
case('branch_copies', copy='ptr saved:Exception=null in here;if(other){saved=selected;{}}else{saved=selected;{}};', extra=True)
case('one_branch_differs', copy='ptr saved:Exception=null in here;if(other){saved=selected;{}}else{saved=payload;{}};', reject=True, extra=True)
case('branch_assignments_independent', copy='ptr saved:Exception=null in here;if(other){saved=payload;{}}else{saved=null;{}};', reject=True, extra=True)
case('joined_copy_then_source_rebound', copy='ptr saved:Exception=null in here;if(other){saved=selected;{}}else{saved=selected;{}};', after='selected=payload;', extra=True)
case('weak_rebinding_is_not_copy', copy='ptr saved=payload;ptr alternate=selected;var writer->if(other){saved}else{alternate};writer=selected;', extra=True, reject=True)
case('loop_rebinds_source', after='for(i in 0u64..2u64){selected=payload;};')
case('loop_rebinds_copy', after='for(i in 0u64..2u64){saved=payload;};', reject=True)
case('callable_classifier', before='func detector=classify;', observe='val ready<-detector(selected);', after='selected=payload;')

for name, body in (
    ('deferred_block', base.LOCAL+SELECT+'ptr saved=selected;{{defer{ {{'+OBSERVE+FIRST+TAIL+'}}; };}};'+base.PRINT),
    ('finally', base.LOCAL+SELECT+'ptr saved=selected;'+OBSERVE+'try{'+FIRST+'}finally{'+TAIL+'};'+base.PRINT),
    ('separate_actions', base.LOCAL+SELECT+'ptr saved=selected;{{defer{'+TAIL+'};defer{if(classify(selected)){target<-7u64;{}}else{};};}};'+base.PRINT),
):
    base.case(name, body=body, declarations=fixture.CLASSIFY, signature='payload:Exception,present:bool', arguments=ARGUMENTS, output='8\n8\n8\n7\n')

IO = base.IO+'using "std:exceptions" NullPointerException;'
REF = 'func read(ref value:const u64 in O)->u64 where {owner O}{return value;}'
for name, change, rejected in (
    ('copy_guard_ref', '', False),
    ('copy_guard_other_binding_changed', 'selected=null;', True),
    ('copy_guard_same_bounds', 'selected=if(other){item}else{null in here};', True),
):
    source=IO+REF+'proc run(flag:bool,other:bool)->void{val item<-7u64;ptr selected=if(flag){item}else{null in here};ptr saved=selected;'+change
    source+='if(saved is not null){behave @print(read(selected));}else{behave @print(8u64);};}'
    source+='run(true,false);run(false,true);'
    runner.CASES[name]=(source,2 if rejected else 0,'' if rejected else '7\n8\n','proven non-null' if rejected else '')

for name, copy, early, mutation, separate, rejected in (
    ('borrowed_read_after_write', 'ptr saved=observed;', '', '', False, False),
    ('borrowed_read_distinct_target', 'ptr saved=observed;', '', '', True, True),
    ('borrowed_read_before_write', 'ptr saved=observed;', 'behave @print(check);', '', False, True),
    ('borrowed_changed_payload', 'ptr saved=observed;', '', 'payload<-unchecked();', False, True),
):
    setup=SELECT+'ptr observed=selected;'+copy+'selected=null;'
    first=base.FIRST.replace('classify(payload)','classify(observed)').replace('target<-','output<-')
    tail=TAIL.replace('target<-','output<-')
    helper='proc fill(ptr output:u64 in O,ptr check:u64 in O,payload:Exception,present:bool)->void throws NullPointerException where {owner O}{'
    helper+=setup+'defer{'+early+first+mutation+tail+'behave @print(check);};}'
    body=base.LOCAL+('val other<-make local u64;' if separate else '')
    body+='try{fill(target,'+('other' if separate else 'target')+',payload,present);}catch(e:const NullPointerException){};'
    base.case(name,body=body,declarations=fixture.CLASSIFY+helper,signature='payload:Exception,present:bool',arguments=ARGUMENTS,
              reject=rejected,output='8\n8\n8\n7\n')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(base.__file__), Path(base.exceptions.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
