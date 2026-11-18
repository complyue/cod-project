#!/usr/bin/env python3
"""Boolean results and writeback preserve the exception snapshots they inspect."""
from itertools import product
from pathlib import Path
import foundation_tests as runner
import exception_projection_tests as exceptions

IO = 'using symbol "std:io" @print;'
CLASSIFY = 'func classify(value:Exception)->bool{try{throw value;}catch(e:const MissingReturnError){return true;}catch(e:const Exception){return false;};}'
LOCAL = 'val target<-make local u64;'
FIRST = 'if(classify(payload)){target<-7u64;{}}else{};'
TAIL = 'try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){target<-8u64;{}};'
PRINT = 'behave @print(target);'
OUTPUT = '7\n8\n'
WRITEBACK = 'proc publish(ptr output:bool in O,value:Exception)->void throws NullPointerException where {owner O}{output<-classify(value);}'
runner.CASES = {}


def case(name, body=LOCAL+FIRST+TAIL+PRINT, declarations=CLASSIFY, output=OUTPUT, reject=False,
         diagnostic='initialization', signature='payload:Exception', arguments=('unchecked()', 'checked()')):
    source = IO+exceptions.ERROR+declarations+'proc run('+signature+')->void{'+body+'}'
    source += ''.join('run('+argument+');' for argument in arguments)
    runner.CASES[name] = (source, 2 if reject else 0, '' if reject else output,
                          diagnostic if reject else exceptions.WARN)


case('direct')
case('generic', declarations=CLASSIFY.replace('value:Exception','value:T').replace('->bool{','->bool where T{'))
case('forwarded', declarations=CLASSIFY+'func relay(value:Exception)->bool{return classify(value);}', body=(LOCAL+FIRST+TAIL+PRINT).replace('if(classify(payload))','if(relay(payload))'))
case('recursive', declarations=CLASSIFY.replace('value:Exception','value:Exception,n:u64').replace('{try{','{if(n>0u64){return classify(value,n-1u64);}else{};try{'), body=(LOCAL+FIRST+TAIL+PRINT).replace('classify(payload)','classify(payload,8u64)'))
case('callable', body=LOCAL+'func action=classify;'+FIRST.replace('classify','action')+TAIL+PRINT)
case('captured_callable', body=LOCAL+'func action=classify;func relay[action](value:Exception)->bool{return action(value);};'+FIRST.replace('classify','relay')+TAIL+PRINT)
case('borrowed_input', declarations=CLASSIFY.replace('value:Exception','ref value:const Exception in O').replace('->bool{','->bool where {owner O}{'))
case('nullable_borrow', declarations=CLASSIFY.replace('value:Exception','ptr value:const Exception in O').replace('->bool{','->bool throws NullPointerException where {owner O}{if(value is null){value;{}}else{};'), body=LOCAL+'try{'+FIRST+TAIL+PRINT+'}catch(e:const NullPointerException){};')
case('nullable_ingress_catch', declarations=CLASSIFY.replace('value:Exception','ptr value:const Exception in O').replace('->bool{','->bool where {owner O}{'))
case('saved_boolean', body=LOCAL+'val ready<-classify(payload),saved<-ready;ready<-false;'+FIRST.replace('classify(payload)','saved')+TAIL+PRINT)
case('changed_boolean', body=LOCAL+'val ready<-classify(payload);ready<-false;'+FIRST.replace('classify(payload)','ready')+TAIL+PRINT, reject=True)
case('saved_exception', body=LOCAL+'val saved:Exception<-payload;'+FIRST+'payload<-unchecked();'+TAIL.replace('throw payload','throw saved')+PRINT)
case('changed_exception', body=LOCAL+FIRST+'payload<-unchecked();'+TAIL+PRINT, reject=True)
case('independent_exception', body=LOCAL+FIRST+TAIL.replace('throw payload','throw other')+PRINT,
     signature='payload:Exception,other:Exception', arguments=('checked(),unchecked()',), reject=True)
case('negated_result', declarations=CLASSIFY.replace('return true','return !true').replace('return false','return !false'), body=(LOCAL+FIRST+TAIL+PRINT).replace('if(classify(payload))','if(!classify(payload))'))
case('boolean_forwarder', declarations=CLASSIFY+'func echo(value:T)->const T where T{return value;}', body=(LOCAL+FIRST+TAIL+PRINT).replace('classify(payload)','echo(classify(payload))'))
case('helper_snapshot', declarations=CLASSIFY.replace('{try{','{val saved:Exception<-value;value<-checked();try{').replace('throw value','throw saved'))
case('helper_finally_snapshot', declarations=CLASSIFY.replace('{try{','{try{try{').replace('return false;};}','return false;};}finally{value<-checked();{}};}'))
case('helper_replaced_return', declarations=CLASSIFY.replace('{try{','{try{try{').replace('return false;};}','return false;};}finally{return false;};}'), reject=True)
case('local_classifier', body=LOCAL+'val ready<-false;try{throw payload;}catch(e:const MissingReturnError){ready<-true;{}}catch(e:const Exception){};'+FIRST.replace('classify(payload)','ready')+TAIL+PRINT)
case('deferred_block', body=LOCAL+'{{defer{'+FIRST+TAIL+'};}};'+PRINT)
case('nested_cleanup', body=LOCAL+'{{defer{defer{'+FIRST+TAIL+'};};}};'+PRINT)
case('cleanup_snapshot', body=LOCAL+'{{defer{ {{val saved:Exception<-payload;'+FIRST+'payload<-unchecked();'+TAIL.replace('throw payload','throw saved')+'}}; };}};'+PRINT)
case('cleanup_mutation', body=LOCAL+'{{defer{'+FIRST+'payload<-unchecked();'+TAIL+'};}};'+PRINT, reject=True)
case('separate_actions', body=LOCAL+'{{defer{'+TAIL+'};defer{'+FIRST+'};}};'+PRINT)
case('different_action_snapshots', body=LOCAL+'{{defer{'+TAIL+'};defer{'+FIRST+'payload<-unchecked();};}};'+PRINT, reject=True)
case('finally', body=LOCAL+'try{'+FIRST+'}finally{'+TAIL+'};'+PRINT)
case('finally_recovered_prefix', body=LOCAL+'try{'+FIRST+'}catch(e:const Exception){}finally{'+TAIL+'};'+PRINT, reject=True)
case('finally_changed_snapshot', body=LOCAL+'try{'+FIRST+'}finally{payload<-unchecked();'+TAIL+'};'+PRINT, reject=True)
case('recursive_unproved', declarations='func classify(value:Exception)->bool{return classify(value);}', reject=True)
case('exceptional_finally_complements', body=LOCAL+'try{try{if(flag){target<-7u64;{}}else{};throw unchecked();}finally{if(!flag){target<-8u64;{}}else{};};}catch(e:const MissingReturnError){'+PRINT+'};', signature='flag:bool', arguments=('true','false'))
case('exceptional_finally_prefix', body=LOCAL+'try{try{missing();if(flag){target<-7u64;{}}else{};}finally{if(!flag){target<-8u64;{}}else{};};}catch(e:const MissingReturnError){'+PRINT+'};', signature='flag:bool', arguments=('true','false'), reject=True)

READY = FIRST.replace('classify(payload)','ready')
case('writeback', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false;try{publish(ready,payload);'+READY+TAIL+PRINT+'}catch(e:const NullPointerException){};')
case('prepared_writeback', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false;try{ {{defer publish(ready,payload);}};'+READY+TAIL+PRINT+'}catch(e:const NullPointerException){};')
case('prepared_value_snapshot', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false,saved:Exception<-payload;try{ {{defer publish(ready,payload);payload<-unchecked();}};'+READY+TAIL.replace('throw payload','throw saved')+PRINT+'}catch(e:const NullPointerException){};')
case('prepared_value_changed', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false;try{ {{defer publish(ready,payload);payload<-unchecked();}};'+READY+TAIL+PRINT+'}catch(e:const NullPointerException){};', reject=True)
BORROWED = WRITEBACK.replace('value:Exception','ref value:const Exception in B').replace('owner O}', 'owner O,owner B}')
case('prepared_borrowed_value', declarations=CLASSIFY+BORROWED, body=LOCAL+'val ready<-false;try{ {{defer publish(ready,payload);payload<-unchecked();}};'+READY+TAIL+PRINT+'}catch(e:const NullPointerException){};', output='7\n7\n')
case('failed_registration', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false;try{ {{defer publish(ready,payload);}}; }catch(e:const Exception){};'+READY+TAIL+PRINT, reject=True)
case('late_copied_operand', declarations=CLASSIFY.replace('value:Exception','value:Exception,ignored:bool'), body=LOCAL+FIRST.replace('classify(payload)','classify(payload,{payload<-unchecked();true})')+TAIL+PRINT, reject=True)
case('late_borrowed_operand', declarations=CLASSIFY.replace('value:Exception','ref value:const Exception in O,ignored:bool').replace('->bool{','->bool where {owner O}{'), body=LOCAL+FIRST.replace('classify(payload)','classify(payload,{payload<-unchecked();true})')+TAIL+PRINT, output='7\n7\n')
case('ref_entry_obligation', body='val uninitialized<-make local Exception;classify(uninitialized);', reject=True, output='')
case('declared_obligation', declarations=CLASSIFY.replace('->bool{','->bool throws Exception{'), reject=True, diagnostic='declared checked exception')
case('failed_null_output', declarations=CLASSIFY+WRITEBACK, body=LOCAL+'val ready<-false;ptr output:bool=null in here;try{publish(output,payload);}catch(e:const NullPointerException){};'+READY+TAIL+PRINT, reject=True)
case('saved_output_target', body=LOCAL+'val ready<-false,other<-false;ptr output=ready;output<-{output=other;classify(payload)};'+READY+TAIL+PRINT)
case('rebound_output_is_unwritten', body=LOCAL+'val ready<-false,other<-false;ptr output=ready;output<-{output=other;classify(payload)};'+READY.replace('if(ready)','if(other)')+TAIL+PRINT, reject=True)

# Evaluate the complete intrinsic category set and an independent Boolean input.
# The first write follows the helper result; the second follows direct catches.
categories = ('OutOfMemoryError', 'RegionCapacityExceededError', 'InvalidCapacityError',
              'NullPointerException', 'MissingReturnError', 'UnsoundRuntimeTypingError',
              'StaticallyUnknownTypeException', 'IndexOutOfBoundsError')
imports = ''.join('using "std:exceptions" '+name+';' for name in categories
                  if name not in ('NullPointerException', 'MissingReturnError'))
for index, selected in enumerate(({4}, {3,4}, {0,2,4,6})):
    classifier = 'func classify(value:Exception)->bool{try{throw value;}'+''.join(
        'catch(e:const '+categories[i]+'){return true;}' for i in sorted(selected))+'catch(e:const Exception){return false;};}'
    tail = 'try{throw payload;}'+''.join('catch(e:const '+categories[i]+'){}' for i in sorted(selected))+'catch(e:const Exception){target<-8u64;{}};'
    for label, expression, evaluate in (
        ('plain', 'classify(payload)', lambda category,flag: category in selected),
        ('or', 'classify(payload)||flag', lambda category,flag: category in selected or flag),
        ('and', 'classify(payload)&&flag', lambda category,flag: category in selected and flag),
    ):
        def result(category, flag):
            value = 7 if evaluate(category,flag) else None
            return 8 if category not in selected else value
        values = [result(category,flag) for category,flag in product(range(8),(False,True))]
        arguments = tuple(('unchecked()' if category==4 else 'checked()')+','+str(flag).lower()
                          for category,flag in product((4,3),(False,True)))
        output = ''.join(str(result(category,flag))+'\n' for category,flag in product((4,3),(False,True)))
        case('model_'+str(index)+'_'+label, LOCAL+FIRST.replace('classify(payload)',expression)+tail+PRINT,
             declarations=imports+classifier, signature='payload:Exception,flag:bool', arguments=arguments,
             reject=None in values, output=output)

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(exceptions.__file__), Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_cleanup',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
