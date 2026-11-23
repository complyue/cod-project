#!/usr/bin/env python3
"""Reaching paths require consistent facts about each immutable snapshot."""
from itertools import product
from pathlib import Path
import foundation_tests as runner
import exception_projection_tests as exceptions

ERROR, WARN = exceptions.ERROR, exceptions.WARN

IO = 'using symbol "std:io" @print;'
NULL = 'using "std:exceptions" NullPointerException;'
LOCAL = 'val target<-make local u64;'
PRINT = 'behave @print(target);'
FIRST = 'if(flag){target<-7u64;{}}else{};'
SECOND = 'if(!flag){target<-8u64;{}}else{};'
SET = 'proc set(ptr destination:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{if(flag){destination<-7u64;{}}else{};return;}'
OUTPUT = LOCAL+'ptr output=target;'
FIRST_OUTPUT = FIRST.replace('target<-', 'output<-')
SECOND_OUTPUT = SECOND.replace('target<-', 'output<-')
GUARDED_SET = SET.replace(' throws NullPointerException', '').replace('if(flag)', 'if(flag && (destination is not null))')
runner.CASES = {}


def case(name, body, declarations='', output='7\n8\n', reject=False, warning='', signature='flag:bool', arguments=('true', 'false')):
    source = IO+declarations+'proc run('+signature+')->void{'+body+'return;}'
    source += ''.join('run('+argument+');' for argument in arguments)
    runner.CASES[name] = (source, 2 if reject else 0, '' if reject else output,
                          'initialization' if reject else warning)


case('complementary', LOCAL+FIRST+SECOND+PRINT)
case('saved_boolean', LOCAL+'val saved<-flag;'+FIRST+'flag<-true;'+SECOND.replace('!flag', '!saved')+PRINT)
case('changed_boolean', LOCAL+FIRST+'flag<-true;'+SECOND+PRINT, reject=True)
case('unrelated_boolean', LOCAL+FIRST+SECOND.replace('!flag', '!other')+PRINT,
     signature='flag:bool,other:bool', arguments=('true,false', 'false,true'), reject=True)
case('impossible_read', LOCAL+'if(flag){if(!flag){'+PRINT+'}else{};}else{};', output='')
case('possible_read', LOCAL+'if(flag){if(flag){'+PRINT+'}else{};}else{};', reject=True)
case('deferred_complements', OUTPUT+'{{defer{'+FIRST_OUTPUT+SECOND_OUTPUT+'};}};'+PRINT)
case('deferred_snapshot', OUTPUT+'{{defer{ {{val saved<-flag;'+FIRST_OUTPUT+'flag<-true;'+SECOND_OUTPUT.replace('!flag', '!saved')+'}}; };}};'+PRINT)
case('deferred_changed_snapshot', OUTPUT+'{{defer{'+FIRST_OUTPUT+'flag<-true;'+SECOND_OUTPUT+'};}};'+PRINT, reject=True)
case('nested_deferred', LOCAL+'{{defer{defer{'+FIRST+SECOND+'};};}};'+PRINT)
case('separate_actions', OUTPUT+'{{defer{'+SECOND_OUTPUT+'};defer{'+FIRST_OUTPUT+'};}};'+PRINT)
case('changed_action_snapshot', OUTPUT+'{{defer{'+SECOND_OUTPUT+'};defer{'+FIRST_OUTPUT+'flag<-true;};}};'+PRINT, reject=True)
case('deferred_read_before_writes', LOCAL+'{{defer{'+PRINT+FIRST+SECOND+'};}};', reject=True)
case('finally_complements', LOCAL+'try{'+FIRST+'}finally{'+SECOND+'};'+PRINT)
case('loop_once', LOCAL+'for(i in 0u64..1u64){'+FIRST+SECOND+'};'+PRINT)
case('loop_zero', LOCAL+'for(i in 0u64..0u64){'+FIRST+SECOND+'};'+PRINT, reject=True)

for name, declarations, first, second in (
    ('calls', SET, 'set(target,flag)', 'set(target,!flag)'),
    ('forwarded', SET+'proc forward(ptr output:u64 in O,choice:bool)->void throws NullPointerException where {owner O}{set(output,choice);return;}', 'forward(target,flag)', 'forward(target,!flag)'),
    ('generic', SET.replace('destination:u64','destination:T').replace('flag:bool','flag:bool,value:T').replace('where {owner O}', 'where {T,owner O}').replace('7u64','value'), 'set(target,flag,7u64)', 'set(target,!flag,7u64)'),
    ('recursive', 'proc set(ptr destination:u64 in O,flag:bool,n:u64)->void throws NullPointerException where {owner O}{if(n>0u64){set(destination,flag,n-1u64);}else{if(flag){destination<-7u64;{}}else{};};return;}', 'set(target,flag,8u64)', 'set(target,!flag,8u64)'),
):
    case(name, LOCAL+'try{'+first+';'+second+';'+PRINT+'}catch(e:const NullPointerException){};', NULL+declarations, output='7\n7\n')
case('callable', LOCAL+'proc action=set;try{action(target,flag);action(target,!flag);'+PRINT+'}catch(e:const NullPointerException){};', NULL+SET, output='7\n7\n')
case('prepared', OUTPUT+'{{defer{set(output,flag);set(output,!flag);};}};'+PRINT, GUARDED_SET, output='7\n7\n')
case('copied_preparation_snapshot', OUTPUT+'{{defer set(output,flag);defer set(output,!flag);}};'+PRINT, GUARDED_SET, output='7\n7\n')
case('failed_registration', OUTPUT+'try{ {{defer set(output,flag);defer set(output,!flag);}}; }catch(e:const Exception){};'+PRINT,
     'using "std:exceptions" Exception;'+GUARDED_SET, reject=True)
case('changed_call_input', LOCAL+'try{set(target,flag);flag<-true;set(target,!flag);'+PRINT+'}catch(e:const NullPointerException){};', NULL+SET, reject=True)
case('ref_entry_promise', LOCAL+'skip(target,flag);', 'proc skip(ref value:u64 in O,condition:bool)->void where {owner O}{if(condition && !condition){behave @print(value);}else{};}', reject=True)
case('borrowed_alias_mutation', LOCAL+'val control<-flag;try{mutate(control,control,target);'+PRINT+'}catch(e:const NullPointerException){};', NULL+
     'proc mutate(ref first:bool in B,ref second:bool in B,ptr output:u64 in O)->void throws NullPointerException where {owner B,owner O}{if(first){output<-7u64;{}}else{};second<-true;if(!first){output<-8u64;{}}else{};}', reject=True)

# The model evaluates actual writes for every input pair, independently of the
# compiler's condition representation and the chosen call-site values.
guards = {
    'a': ('a', lambda a,b: a),
    'not_a': ('!a', lambda a,b: not a),
    'and': ('a&&b', lambda a,b: a and b),
    'not_and': ('!a || !b', lambda a,b: not a or not b),
    'or': ('a||b', lambda a,b: a or b),
    'neither': ('!a && !b', lambda a,b: not a and not b),
}
for left, right in [('a','not_a'), ('and','not_and'), ('or','neither'), ('and','not_a'),
                    ('or','not_a'), ('a','neither'), ('and','or'), ('not_and','or')]:
    first, first_fn = guards[left]
    second, second_fn = guards[right]
    outputs = []
    for a,b in product((False,True), repeat=2):
        result = 7 if first_fn(a,b) else None
        if second_fn(a,b): result = 8
        outputs.append(result)
    body = LOCAL+'if('+first+'){target<-7u64;{}}else{};if('+second+'){target<-8u64;{}}else{};'+PRINT
    case('model_'+left+'_'+right, body, signature='a:bool,b:bool',
         arguments=tuple(str(a).lower()+','+str(b).lower() for a,b in product((False,True),repeat=2)),
         reject=None in outputs, output=''.join(str(value)+'\n' for value in outputs))

HEAD = 'try{throw payload;}catch(e:const MissingReturnError){target<-7u64;{}}catch(e:const Exception){};'
TAIL = 'try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){target<-8u64;{}};'


def errors(name, body, reject=False, declarations='', output='7\n8\n', signature='payload:Exception', arguments=('unchecked()', 'checked()')):
    case(name, body, ERROR+declarations, output=output, reject=reject, warning=WARN,
         signature=signature, arguments=arguments)


errors('category_complements', LOCAL+HEAD+TAIL+PRINT)
errors('category_saved', LOCAL+'val saved:Exception<-payload;'+HEAD+'payload<-unchecked();'+TAIL.replace('throw payload','throw saved')+PRINT)
errors('category_changed', LOCAL+HEAD+'payload<-unchecked();'+TAIL+PRINT, reject=True)
errors('category_independent', LOCAL+HEAD+TAIL.replace('throw payload', 'throw other')+PRINT, reject=True,
       signature='payload:Exception,other:Exception', arguments=('checked(),unchecked()',))
errors('category_deferred', OUTPUT+'{{defer{'+HEAD.replace('target<-','output<-')+TAIL.replace('target<-','output<-')+'};}};'+PRINT)
errors('category_nested', LOCAL+'{{defer{defer{'+HEAD+TAIL+'};};}};'+PRINT)
errors('category_finally', LOCAL+'try{'+HEAD+'}finally{'+TAIL+'};'+PRINT)
errors('category_snapshot_actions', LOCAL+'{{defer{'+TAIL+'};defer{'+HEAD+'payload<-unchecked();};}};'+PRINT, reject=True)
errors('category_missing_branch', LOCAL+HEAD+HEAD+PRINT, reject=True)
errors('category_constant_independence', LOCAL+'val first:Exception<-checked(),second:Exception<-unchecked();'+HEAD.replace('throw payload','throw first')+TAIL.replace('throw payload','throw second')+PRINT, reject=True)

# Typed catch partitions cover the complete supported category set even though
# these calls execute only the two failures constructed by the fixture.
categories = ('OutOfMemoryError', 'RegionCapacityExceededError', 'InvalidCapacityError',
              'NullPointerException', 'MissingReturnError', 'UnsoundRuntimeTypingError',
              'StaticallyUnknownTypeException', 'IndexOutOfBoundsError')
imports = ''.join('using "std:exceptions" '+name+';' for name in categories
                  if name not in ('NullPointerException', 'MissingReturnError'))
partitions = [({4}, set(range(8))-{4}), ({3,4}, set(range(8))-{3,4}),
              ({0,2,4,6}, {1,3,5,7}), (set(range(6)), {3,4,5,6,7}),
              ({3,4}, {0,1,2,5,6}), ({4}, {4})]
for index, (left, right) in enumerate(partitions):
    def handling(selected, number):
        return 'try{throw payload;}'+''.join('catch(e:const '+categories[i]+'){target<-'+str(number)+'u64;{}}'
                                             for i in sorted(selected))+'catch(e:const Exception){};'
    outputs = {i: 8 if i in right else 7 if i in left else None for i in range(8)}
    errors('category_model_'+str(index), LOCAL+handling(left,7)+handling(right,8)+PRINT,
           reject=None in outputs.values(), declarations=imports,
           output=str(outputs[4])+'\n'+str(outputs[3])+'\n')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(exceptions.__file__), Path(__file__)]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_deferred',
    'category_nested',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
