#!/usr/bin/env python3
"""Deferred reads preserve the conditions of missing initialization paths."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN

OBSERVE = ' {{val observed<-make local u64;try{throw payload;}catch(e:const MissingReturnError){observed<-e.category;{}}catch(e:const Exception){};behave @print(observed);}}; '
BOOLEAN = ' {{val observed<-make local u64;if(flag){observed<-7u64;{}}else{};behave @print(observed);}}; '
VALUE = 'val payload:Exception<-unchecked();'
runner.CASES = {}


def case(name, body, output='5\n', declarations='', reject=False):
    runner.CASES[name] = (IO+ERROR+declarations+body, 2 if reject else 0,
                          '' if reject else output, 'initialization' if reject else WARN)


def bad(name, body, **kwargs):
    case(name, body, reject=True, **kwargs)


case('local_handler', VALUE+'{{defer{'+OBSERVE+'};}};')
case('written_before_invocation', 'val incoming<-unchecked(),payload:Exception<-checked();{{defer{'+OBSERVE+'};payload<-incoming:void;}};')
bad('earlier_action_write_failure_prefix', 'val payload:Exception<-checked();{{defer{'+OBSERVE+'};defer{payload<-unchecked();};}};')
case('nested_scope', VALUE+'{{defer{ {{defer{'+OBSERVE+'};}}; };}};')
case('nested_registration', VALUE+'{{defer{defer{'+OBSERVE+'};};}};')
case('nested_write', 'val incoming<-unchecked(),payload:Exception<-checked();{{defer{defer{'+OBSERVE+'};payload<-incoming;};}};')
case('loop_queue', VALUE+'{{for(i in 0u64..3u64){defer{'+OBSERVE+'};};}};', '5\n'*3)
case('iteration_scope', VALUE+'for(i in 0u64..3u64){ {{defer{'+OBSERVE+'};}}; };', '5\n'*3)
case('readonly_snapshot', 'val source:Exception<-unchecked();const val payload<-source;{{defer{'+OBSERVE+'};source<-checked():void;}};')
case('selected_read', 'val first:Exception<-checked(),second:Exception<-unchecked();ref payload=first;ref alternate=second;{{defer{'+OBSERVE+'};payload=alternate:void;}};')
bad('selected_write_failure_prefix', 'val first:Exception<-checked(),payload:Exception<-checked();'
    'ref selected=first;ref observed_target=payload;{{defer{'+OBSERVE.replace('throw payload','throw observed_target')+
    '};defer{selected<-unchecked();};selected=observed_target:void;}};')
bad('helper_write_failure_prefix', 'val payload:Exception<-checked();{{defer{'+OBSERVE+'};defer{write(payload);};}};', declarations='func write(ref value:Exception in O)->void where {owner O}{value<-unchecked():void;}')
bad('prepared_write_failure_prefix', 'val payload:Exception<-checked();{{defer{'+OBSERVE+'};defer write(payload);}};', declarations='func write(ref value:Exception in O)->void where {owner O}{value<-unchecked():void;}')
case('copied_helper_result', VALUE+'{{defer{ {{val saved<-keep(payload);'+OBSERVE.replace('throw payload','throw saved')+'}}; };}};', declarations='func keep(value:Exception)->const Exception{return value;}')
case('generic_helper_result', VALUE+'{{defer{ {{val saved<-keep(payload);'+OBSERVE.replace('throw payload','throw saved')+'}}; };}};', declarations='func keep(value:T)->const T where {T}{return value;}')
case('recursive_helper_result', VALUE+'{{defer{ {{val saved<-keep(payload,8u64);'+OBSERVE.replace('throw payload','throw saved')+'}}; };}};', declarations='func keep(value:Exception,n:u64)->const Exception{if(n==0u64){return value;}else{return keep(value,n-1u64);};}')
case('nested_handler', VALUE+'{{defer{ {{val observed<-make local u64;try{try{throw payload;}catch(e:const MissingReturnError){throw e;};}catch(e:const MissingReturnError){observed<-e.category;{}}catch(e:const Exception){};behave @print(observed);}}; };}};')
case('explicit_return', 'behave @print(run());', '5\n', declarations='proc run()->u64{'+VALUE+'defer{return {{val observed<-make local u64;try{throw payload;}catch(e:const MissingReturnError){observed<-e.category;{}}catch(e:const Exception){};return observed;}};};return 9u64;}')
case('category_partition', 'run(true);run(false);', '5\n4\n', declarations='proc run(flag:bool)->void{val left:Exception<-unchecked(),right:Exception<-checked();val payload:Exception<-if(flag){left}else{right};defer{'+OBSERVE.replace('catch(e:const Exception){}', 'catch(e:const NullPointerException){observed<-e.category;{}}catch(e:const Exception){}')+'};return;}')

bad('late_checked_write', VALUE+'{{defer{'+OBSERVE+'};payload<-checked():void;}};')
bad('earlier_checked_write', VALUE+'{{defer{'+OBSERVE+'};defer{payload<-checked();};}};')
bad('later_write_does_not_help', 'val payload:Exception<-checked();{{defer{payload<-unchecked();};defer{'+OBSERVE+'};}};')
bad('wrong_selected_target', VALUE+'val other:Exception<-checked();ref selected=payload;ref alternate=other;{{defer{'+OBSERVE.replace('throw payload','throw selected')+'};selected=alternate:void;}};')
bad('unknown_parameter', 'run(unchecked());', declarations='proc run(payload:Exception)->void{defer{'+OBSERVE+'};}')
bad('uninitialized_exception', 'val payload<-make local Exception;{{defer{'+OBSERVE+'};}};')
bad('missing_write', VALUE+'{{defer{'+OBSERVE.replace('observed<-e.category;{}','{}')+'};}};')
bad('unknown_recursive_result', VALUE+'{{defer{ {{val saved<-forever();'+OBSERVE.replace('throw payload','throw saved')+'}}; };}};', declarations='func forever()->const Exception{return forever();}')
bad('aliased_borrowed_write', 'run();', declarations='proc write(ref left:Exception in O,ref right:Exception in P)->void where {owner O,owner P}{left<-unchecked();right<-checked();}proc run()->void{'+VALUE+'defer{'+OBSERVE+'};defer write(payload,payload);}')

case('boolean_true', 'val flag<-true;{{defer{'+BOOLEAN+'};}};', '7\n')
case('boolean_late_true', 'val flag<-false;{{defer{'+BOOLEAN+'};flag<-true;}};', '7\n')
case('boolean_nested', 'val flag<-true;{{defer{defer{'+BOOLEAN+'};};}};', '7\n')
bad('boolean_earlier_action_failure_prefix', 'val flag<-false;{{defer{'+BOOLEAN+'};defer{flag<-true;};}};')
case('boolean_negation', 'val flag<-false;{{defer{'+BOOLEAN.replace('if(flag)','if(!flag)')+'};}};', '7\n')
case('boolean_copy', 'val source<-true,flag<-source;{{defer{'+BOOLEAN+'};source<-false;}};', '7\n')
case('category_repaired_registration', 'val incoming<-unchecked(),payload:Exception<-checked();{{defer{'+OBSERVE+'};try{defer{payload<-incoming;};}catch(e:const OutOfMemoryError){payload<-incoming;throw e;};}};', declarations='using "std:exceptions" OutOfMemoryError;')
case('boolean_repaired_registration', 'val flag<-false;{{defer{'+BOOLEAN+'};try{defer{flag<-true;};}catch(e:const OutOfMemoryError){flag<-true;throw e;};}};', '7\n', declarations='using "std:exceptions" OutOfMemoryError;')
case('helper_preserves_safe_prefix', VALUE+'{{defer{'+OBSERVE+'};defer{write(payload);};}};', declarations='func write(ref value:Exception in O)->void where {owner O}{value<-unchecked():void;}')
case('prepared_preserves_safe_prefix', VALUE+'{{defer{'+OBSERVE+'};defer write(payload);}};', declarations='func write(ref value:Exception in O)->void where {owner O}{value<-unchecked():void;}')
case('boolean_written_locally', 'val flag<-false;{{defer{flag<-true;'+BOOLEAN+'};}};', '7\n')
bad('boolean_false', 'val flag<-false;{{defer{'+BOOLEAN+'};}};')
bad('boolean_late_false', 'val flag<-true;{{defer{'+BOOLEAN+'};flag<-false;}};')
bad('boolean_unknown_parameter', 'run(true);', declarations='proc run(flag:bool)->void{defer{'+BOOLEAN+'};}')
bad('boolean_missing_write', 'val flag<-true;{{defer{'+BOOLEAN.replace('observed<-7u64;{}','{}')+'};}};')
bad('boolean_zero_loop', 'val flag<-true;{{defer{'+BOOLEAN.replace('observed<-7u64;{}','for(i in 0u64..0u64){observed<-7u64;};')+'};}};')


# The independent model observes the value at the handler/branch, before the
# later write. Changing that input afterward cannot rewrite the initialization.
for incoming in (False, True):
    for outgoing in (False, True):
        label = str(int(incoming))+'_'+str(int(outgoing))
        before = 'unchecked()' if incoming else 'checked()'
        after = 'unchecked()' if outgoing else 'checked()'
        body = 'val payload:Exception<-'+before+',replacement:Exception<-'+after+';{{defer{'+OBSERVE.replace('behave @print(observed);','payload<-replacement;behave @print(observed);')+'};}};'
        case('category_snapshot_'+label, body, reject=not incoming)
        body = 'val flag<-'+str(incoming).lower()+';{{defer{'+BOOLEAN.replace('behave @print(observed);','flag<-'+str(outgoing).lower()+';behave @print(observed);')+'};}};'
        case('boolean_snapshot_'+label, body, '7\n', reject=not incoming)

case('all_boolean_paths_initialize', 'val flag<-false;{{defer{'+BOOLEAN.replace('else{}','else{observed<-7u64;{}}')+'};}};', '7\n')
case('boolean_rebound_to_true_target', 'val known<-true,other<-false;ref flag=other;ref alternate=known;{{defer{'+BOOLEAN+'};flag=alternate;}};', '7\n')
case('category_prior_initialization', 'val payload:Exception<-checked();{{defer{'+OBSERVE.replace('val observed<-make local u64','val observed<-5u64')+'};}};')
bad('wrong_handler_initializes', VALUE+'{{defer{'+OBSERVE.replace('catch(e:const MissingReturnError)','catch(e:const NullPointerException)')+'};}};')

bad('boolean_rebound_to_false_target', 'val known<-true,other<-false;ref flag=known;ref alternate=other;{{defer{'+BOOLEAN+'};flag=alternate;}};')
bad('boolean_possible_alias', 'run(true);', declarations='proc run(choice:bool)->void{val first<-true,second<-false;ref flag=if(choice){first}else{second};defer{'+BOOLEAN+'};}')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('exception_projection_tests.py')]

from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_registration',
    'nested_write',
    'boolean_nested',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
