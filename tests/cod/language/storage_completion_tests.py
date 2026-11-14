#!/usr/bin/env python3
"""Storage consumers preserve operand exits without inventing writes or targets."""
from pathlib import Path
import operand_completion_tests as fixture
runner=fixture.runner
IO,OBS,EX=fixture.IO,fixture.OBS,fixture.EX
NPE='using "std:exceptions" NullPointerException;'
cases={}
for kind in ('val','ref','ptr'):
    declaration = kind+' x:i64'+('<-' if kind=='val' else '=')
    cases[kind+'_initializer_returns']=(OBS+'proc f()->const i64{defer observe(1);'+declaration+'{{return 7;}};return 0;}behave @print(f());',0,'1\n7\n','')
    cases[kind+'_initializer_throws']=(OBS+EX+'proc f()->void{defer observe(1);'+declaration+'{{throw error();}};observe(99);}try{f();}catch(e:const MissingReturnException){observe(2);};',0,'1\n2\n','[-Wmissing-return]')
    cases[kind+'_aborted_binding_unavailable']=(EX+'func f()->const i64{try{'+declaration+'{{throw error();}};}catch(e:const MissingReturnException){};return x;}f();',2,'','not available on every reaching path')
cases.update({
 'typed_callable_initializer':(IO+'func f()->const i64{func x:func(i64)->const i64={{return 8;}};return 0;}behave @print(f());',0,'8\n',''),
 'typed_procedure_initializer':(IO+'proc f()->const i64{proc x:proc(i64)->void={{return 9;}};return 0;}behave @print(f());',0,'9\n',''),
 'wrong_callable_kind_rejects':('func f()->const i64{func x:proc(i64)->void={{return 9;}};return 0;}f();',2,'','matching func/proc value'),
 'unknown_initializer_type_remains_unproved':('func f()->const i64{val x<-{{return 9;}};return 0;}f();',2,'','non-value or abruptly completing operand'),
 'group_retains_earlier_effects':(OBS+'proc f()->const i64{val a<-{observe(1);1},b:i64<-{{return 10;}},c<-{observe(99);2};return 0;}behave @print(f());',0,'1\n10\n',''),
 'group_body_returns':(OBS+'proc f()->const i64{val a<-1,{observe(a);return 11;};observe(99);return 0;}behave @print(f());',0,'1\n11\n',''),
 'later_group_type_still_checked':('func f()->const i64{val a:i64<-{{return 11;}},b:bool<-1;return 0;}f();',2,'','initializer does not satisfy'),
 'declaration_throw_does_not_publish_binding':(EX+'func f()->const i64{try{val x:i64<-{{throw error();}};}catch(e:const MissingReturnException){};return x;}f();',2,'','not available on every reaching path'),
 'declaration_throw_preserves_prior_write':(OBS+EX+'proc f()->void{val x<-make local i64;try{val y:i64<-{{x<-12;throw error();}};}catch(e:const MissingReturnException){observe(x);};}f();',0,'12\n','[-Wmissing-return]'),
 'initializer_nested_finally':(IO+'func f()->const i64{val x:i64<-try{return 12;}finally{return 13;};return 0;}behave @print(f());',0,'13\n',''),
 'construction_initializer_returns':(IO+'func f()->const i64{val x<-make local i64 {return 14;};return 0;}behave @print(f());',0,'14\n',''),
 'construction_initializer_throws':(OBS+EX+'proc f()->void{defer observe(1);ptr x=make local i64 {throw error();};observe(99);}try{f();}catch(e:const MissingReturnException){observe(15);};',0,'1\n15\n','[-Wmissing-return]'),
 'typeof_construction_initializer_returns':(IO+'func f()->const i64{val y<-make local i64;val x<-make local typeof(y) {return 16;};return 0;}behave @print(f());',0,'16\n',''),
 'uninitialized_live_initializer_rejects':('func f()->const i64{val y<-make local i64;val x<-make local i64 {y;return 16;};return 0;}f();',2,'','before definite initialization'),
 'constructor_normal_type_still_checked':('func f(flag:bool)->const i64{val x<-make local i64 {if(flag){return 16;}else{false}};return 0;}f(true);',2,'','initializer does not satisfy local storage type'),
 'typed_generic_initializer_returns':(IO+'func f(x:T)->const T where T{val unused:T<-{{return x;}};return x;}behave @print(f(17));behave @print(f(true));',0,'17\ntrue\n',''),
 'declaration_as_named_operand':(IO+'func target(x:i64)->void{}func f()->const i64{target({val x:i64<-{{return 18;}};1});return 0;}behave @print(f());',0,'18\n',''),
 'rebind_source_returns':(OBS+'proc f()->const i64{val x<-1;ptr p=x;defer{observe(p);};p={{return 19;}};return 0;}behave @print(f());',0,'1\n19\n',''),
 'ref_source_returns':(OBS+'proc f()->const i64{val x<-1;ref p=x;defer{observe(p);};p={{return 20;}};return 0;}behave @print(f());',0,'1\n20\n',''),
 'rebind_source_throws':(OBS+EX+'proc f()->void{val x<-1,y<-2;ptr p=x;try{p={{y<-3;throw error();}};}catch(e:const MissingReturnException){observe(p);observe(y);};}f();',0,'1\n3\n','[-Wmissing-return]'),
 'fixed_rebind_still_rejects':('func f()->const i64{val x<-1;const ptr p=x;p={{return 20;}};return 0;}f();',2,'','const reference binding'),
 'val_rebind_still_rejects':('func f()->const i64{val x<-1;x={{return 20;}};return 0;}f();',2,'','val storage has no reference binding'),
 'transfer_destination_returns':(OBS+'proc f()->const i64{defer observe(1);({{return 21;}})<-{observe(99);1};return 0;}behave @print(f());',0,'1\n21\n',''),
 'rebind_destination_returns':(OBS+'proc f()->const i64{val x<-1;({{return 22;}})={observe(99);x};return 0;}behave @print(f());',0,'22\n',''),
 'abrupt_destination_checks_later_name':('func f()->const i64{({{return 22;}})<-missing;return 0;}f();',2,'','unknown scalar value'),
 'transfer_rhs_is_still_read':('func f()->const i64{val x<-make local i64;({{return 22;}})<-x;return 0;}f();',2,'','before definite initialization'),
 'rebind_rhs_is_not_read':(IO+'func f()->const i64{val x<-make local i64;({{return 23;}})=x;return 0;}behave @print(f());',0,'23\n',''),
 'bits_swap_right_returns':(OBS+'proc f()->const i64{val x<-1;defer{observe(x);};x<->{{return 24;}};return 0;}behave @print(f());',0,'1\n24\n',''),
 'bits_swap_left_returns':(OBS+'proc f()->const i64{val x<-1;({{return 25;}})<->{observe(99);x};return 0;}behave @print(f());',0,'25\n',''),
 'swap_reads_wait_for_operands':(IO+'func f()->const i64{val x<-make local i64;x<->{{return 26;}};return 0;}behave @print(f());',0,'26\n',''),
 'swap_npe_waits_for_operands':(IO+'func f()->const i64{ptr p:i64=null in here;p<->{{return 27;}};return 0;}behave @print(f());',0,'27\n',''),
 'swap_operand_read_precedes_exit':(OBS+NPE+'proc f()->const i64{val x<-1;ptr p:i64=null in here;try{0;{p;x}<->{{return 27;}};}catch(e:const NullPointerException){observe(28);};return 29;}behave @print(f());',0,'28\n29\n',''),
 'swap_preserves_earlier_effect':(OBS+EX+'proc f()->void{val x<-1,y<-2;try{0;{x<-3;x}<->{{throw error();}};}catch(e:const MissingReturnException){observe(x);observe(y);};}f();',0,'3\n2\n','[-Wmissing-return]'),
 'reference_swap_returns':(OBS+'proc f()->const i64{val x<-1,y<-2;ptr p=x,q=y;defer{observe(p);observe(q);};p<=>{{return 30;}};return 0;}behave @print(f());',0,'1\n2\n30\n',''),
 'reference_swap_left_returns':(IO+'func f()->const i64{val x<-1;ptr p=x;({{return 31;}})<=>p;return 0;}behave @print(f());',0,'31\n',''),
 'swap_readonly_still_rejects':('func f()->const i64{const val x<-1;x<->{{return 31;}};return 0;}f();',2,'','readonly destination'),
 'swap_fixed_binding_still_rejects':('func f()->const i64{val x<-1;const ptr p=x;p<=>{{return 31;}};return 0;}f();',2,'','const reference binding'),
 'swap_normal_operand_requires_storage':('func f()->const i64{1<->{{return 31;}};return 0;}f();',2,'','storage destinations'),
 'deferred_block_initializer_return':(OBS+'proc f()->const i64{defer observe(1);defer{0;{{val x:i64<-{{return 32;}};}};};return 0;}behave @print(f());',0,'1\n32\n',''),
 'deferred_block_swap_return':(OBS+'proc f()->const i64{val x<-1;defer observe(1);defer{x<->{{return 33;}};};return 0;}behave @print(f());',0,'1\n33\n',''),
 'aborted_constructor_returns_complete_image':(IO+'func base(x:i64)->const i64{return x;}func factory()->const (func(i64)->const i64){func f=base;val unused<-make local i64 {return func[f](x:i64)->const i64{return f(x)+1;};};return base;}behave @print(factory()(33));',0,'34\n',''),
})
runner.CASES=cases
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
