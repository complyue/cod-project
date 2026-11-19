#!/usr/bin/env python3
"""Nullable input branches preserve immutable absence and payload evidence."""
from itertools import product
from pathlib import Path
import category_predicate_tests as fixture

runner = fixture.runner
runner.CASES = {}
CLASSIFY = fixture.CLASSIFY.replace('value:Exception', 'ptr value:const Exception in O').replace('->bool{', '->bool where {owner O}{')
BODY = fixture.LOCAL + fixture.FIRST + fixture.TAIL + fixture.PRINT


def case(name, body=BODY, declarations=CLASSIFY, **kwargs):
    fixture.case(name, body=body, declarations=declarations, **kwargs)


case('direct')
case('generic', declarations=CLASSIFY.replace('value:const Exception','value:const T').replace('owner O}', 'T,owner O}'))
case('recursive', declarations=CLASSIFY.replace('in O)', 'in O,n:u64)').replace('{try{', '{if(n>0u64){return classify(value,n-1u64);}else{};try{'), body=BODY.replace('classify(payload)', 'classify(payload,8u64)'))
case('forwarded', declarations=CLASSIFY+'func relay(ptr value:const Exception in O)->bool where {owner O}{return classify(value);}', body=BODY.replace('classify(payload)', 'relay(payload)'))
case('callable', body=fixture.LOCAL+'func selected=classify;'+(fixture.FIRST+fixture.TAIL+fixture.PRINT).replace('classify(payload)','selected(payload)'))
case('captured_callable', body=fixture.LOCAL+'func selected=classify;func relay[selected](ptr value:const Exception in O)->bool where {owner O}{return selected(value);};'+(fixture.FIRST+fixture.TAIL+fixture.PRINT).replace('classify(payload)','relay(payload)'))
case('saved_boolean', body=fixture.LOCAL+'val ready<-classify(payload);payload<-checked();'+fixture.FIRST.replace('classify(payload)','ready')+fixture.TAIL+fixture.PRINT, output='8\n8\n')
case('changed_boolean', body=fixture.LOCAL+'val ready<-classify(payload);ready<-false;'+fixture.FIRST.replace('classify(payload)','ready')+fixture.TAIL+fixture.PRINT, reject=True)
case('null_argument', body=fixture.LOCAL+'ptr absent:Exception=null in here;'+fixture.FIRST.replace('classify(payload)','classify(absent)')+fixture.TAIL+fixture.PRINT, reject=True)
case('rebound_to_null', declarations=CLASSIFY.replace('{try{','{value=null;try{'), reject=True)
case('saved_before_rebinding', declarations=CLASSIFY.replace('{try{','{ptr saved=value;value=null;try{').replace('throw value','throw saved'))
case('checked_before_rebinding', declarations=CLASSIFY.replace('{try{','{try{try{').replace('return false;};}','return false;};}finally{value=null;{}};}'))
case('independent_input', declarations=CLASSIFY, body=fixture.LOCAL+fixture.FIRST.replace('classify(payload)','classify(other)')+fixture.TAIL+fixture.PRINT, signature='payload:Exception,other:Exception', arguments=('unchecked(),checked()',), reject=True)
case('late_selection_rebind', declarations=CLASSIFY.replace('in O)', 'in O,ignored:bool)'), body=fixture.LOCAL+'ptr selected=payload;'+fixture.FIRST.replace('classify(payload)','classify(selected,{selected=null;true})')+fixture.TAIL+fixture.PRINT)
BORROWED_TARGET = fixture.LOCAL+'ptr destination=target;'
BORROWED_FIRST = fixture.FIRST.replace('target<-', 'destination<-')
BORROWED_TAIL = fixture.TAIL.replace('target<-', 'destination<-')
case('deferred_block', body=BORROWED_TARGET+'{{defer{'+BORROWED_FIRST+BORROWED_TAIL+'};}};'+fixture.PRINT)
case('separate_actions', body=BORROWED_TARGET+'{{defer{'+BORROWED_TAIL+'};defer{'+BORROWED_FIRST+'};}};'+fixture.PRINT)
case('finally', body=fixture.LOCAL+'try{'+fixture.FIRST+'}finally{'+fixture.TAIL+'};'+fixture.PRINT)
WRITEBACK = 'proc publish(ref output:bool in O,ptr value:const Exception in E)->void where {owner O,owner E}{output<-classify(value);}'
READY = fixture.FIRST.replace('classify(payload)', 'ready')
case('writeback', declarations=CLASSIFY+WRITEBACK, body=fixture.LOCAL+'val ready<-false;try{publish(ready,payload);'+READY+fixture.TAIL+fixture.PRINT+'}catch(e:const NullPointerException){};')
case('prepared_writeback', declarations=CLASSIFY+WRITEBACK, body=fixture.LOCAL+'val ready<-false;ref output=ready;ptr input=payload;try{ {{defer publish(output,input);}};'+READY+fixture.TAIL+fixture.PRINT+'}catch(e:const NullPointerException){};')
case('prepared_selection_snapshot', declarations=CLASSIFY+WRITEBACK, body=fixture.LOCAL+'val ready<-false;ref output=ready;ptr selected=payload;try{ {{defer publish(output,selected);selected=null;}};'+READY+fixture.TAIL+fixture.PRINT+'}catch(e:const NullPointerException){};')
case('prepared_payload_mutation', declarations=CLASSIFY+WRITEBACK, body=fixture.LOCAL+'val ready<-false;ref output=ready;ptr input=payload;try{ {{defer publish(output,input);input<-unchecked();}};'+READY+fixture.TAIL+fixture.PRINT+'}catch(e:const NullPointerException){};', output='7\n7\n')
case('failed_registration', declarations=CLASSIFY+WRITEBACK, body=fixture.LOCAL+'val ready<-false;ref output=ready;ptr input=payload;try{ {{defer publish(output,input);}};}catch(e:const Exception){};'+READY+fixture.TAIL+fixture.PRINT, reject=True)
case('declared_obligation', declarations=CLASSIFY.replace('->bool where', '->bool throws Exception where'), reject=True, diagnostic='declared checked exception')
case('payload_initialization', body='val payload<-make local Exception;classify(payload);', signature='', arguments=('',), reject=True)
SELECT = 'ptr selected=if(present){payload}else{null in here};'
case('local_selection_rebound', body=fixture.LOCAL+SELECT+'val ready<-classify(selected);selected=payload;'+fixture.FIRST.replace('classify(payload)','ready')+
     'if(selected is null){target<-8u64;{}}else{'+fixture.TAIL+'};'+fixture.PRINT,
     signature='payload:Exception,present:bool', arguments=('unchecked(),false',), reject=True)
case('late_nullable_selection_rebound', declarations=CLASSIFY.replace('in O)', 'in O,ignored:bool)'),
     body=fixture.LOCAL+SELECT+fixture.FIRST.replace('classify(payload)','classify(selected,{selected=payload;true})')+
     'if(selected is null){target<-8u64;{}}else{'+fixture.TAIL+'};'+fixture.PRINT,
     signature='payload:Exception,present:bool', arguments=('unchecked(),false',), reject=True)

PRESENT = 'func present(ptr candidate:u64 in O)->bool where {owner O}{try{candidate;return true;}catch(e:const NullPointerException){return false;};}'
for name, before, first, second, rejected in (
    ('presence_complements', '', 'present(p)', 'p is null', False),
    ('presence_other_input', '', 'present(p)', 'q is null', True),
    ('presence_saved_selection', 'ptr saved=p;', 'present(saved)', 'saved is null', False),
    ('presence_copied_result', 'val ready<-present(p);', 'ready', 'p is null', False),
    ('presence_changed_result', 'val ready<-present(p);ready<-false;', 'ready', 'p is null', True),
    ('presence_uninitialized_payload', '', 'present(p)', 'p is null', True),
):
    source = fixture.IO+'using "std:exceptions" NullPointerException;'+PRESENT
    source += 'proc run(ptr p:u64 in O,ptr q:u64 in O)->void where {owner O}{'+fixture.LOCAL+before
    source += 'if('+first+'){target<-7u64;{}}else{};if('+second+'){target<-8u64;{}}else{};'+fixture.PRINT+'}'
    source += 'val item<-'+('make local u64' if name=='presence_uninitialized_payload' else '1u64')+';ptr absent:u64=null in here;run(item,absent);run(absent,item);'
    runner.CASES[name] = (source, 2 if rejected else 0, '' if rejected else '7\n8\n', 'initialization' if rejected else '')

# Presence and payload category are independent inputs. Enumerate null and both
# payload categories, then compare a conditional second write with that model.
for variant in ('complement', 'wrong_category', 'unconditional'):
    outcomes = []
    for present, category in product((False, True), (False, True)):
        value = 7 if present and category else None
        second = (not present or not category) if variant == 'complement' else category if variant == 'wrong_category' else True
        if second: value = 8
        outcomes.append(value)
    tail = 'if(selected is null){target<-8u64;{}}else{'+fixture.TAIL+'};' if variant == 'complement' else \
        'if(classify(payload)){target<-8u64;{}}else{};' if variant == 'wrong_category' else 'target<-8u64;'
    body = fixture.LOCAL+SELECT+fixture.FIRST.replace('classify(payload)','classify(selected)')+tail+fixture.PRINT
    case('model_'+variant, body=body, signature='payload:Exception,present:bool',
         arguments=tuple(('unchecked()' if category else 'checked()')+','+str(present).lower() for present,category in product((False,True),(False,True))),
         reject=None in outcomes, output=''.join(str(value)+'\n' for value in outcomes))

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(fixture.exceptions.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
