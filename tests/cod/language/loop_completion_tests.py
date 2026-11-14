#!/usr/bin/env python3
"""Loop setup and non-executing evidence preserve non-producing operands."""
from pathlib import Path
import operand_completion_tests as fixture
runner=fixture.runner
IO,OBS,EX=fixture.IO,fixture.OBS,fixture.EX
NPE='using "std:exceptions" NullPointerException;'
TARGET='func target(x:i64)->void{}'
cases={
 'first_bound_returns':(OBS+'proc f()->const i64{defer observe(1);for(i in {{observe(2);return 7;}}..{observe(99);3}){observe(98);}else{observe(97);};return 0;}behave @print(f());',0,'2\n1\n7\n',''),
 'last_bound_returns':(OBS+'proc f()->const i64{defer observe(1);for(i in {observe(2);0}..{{return 8;}}){observe(98);}else{observe(97);};return 0;}behave @print(f());',0,'2\n1\n8\n',''),
 'first_bound_throws':(OBS+EX+'proc f()->void{defer observe(1);for(i in {{throw error();}}..{observe(99);3}){observe(98);}else{observe(97);};}try{f();}catch(e:const MissingReturnException){observe(9);};',0,'1\n9\n','[-Wmissing-return]'),
 'last_bound_throws':(OBS+EX+'proc f()->void{defer observe(1);for(i in {observe(2);0}..{{throw error();}}){observe(98);}else{observe(97);};}try{f();}catch(e:const MissingReturnException){observe(10);};',0,'2\n1\n10\n','[-Wmissing-return]'),
 'first_bound_supplies_unsigned_index':(IO+'func use(x:u8)->void{}func f()->const i64{for(i in 0u8..{{return 11;}}){use(i);};return 0;}behave @print(f());',0,'11\n',''),
 'last_bound_supplies_unsigned_index':(IO+'func use(x:u32)->void{}func f()->const i64{for(i in {{return 12;}}..3u32){use(i);};return 0;}behave @print(f());',0,'12\n',''),
 'body_index_type_still_checked':('func use(x:bool)->void{}func f()->const i64{for(i in 0u8..{{return 12;}}){use(i);};return 0;}f();',2,'','no applicable method'),
 'both_bounds_need_index_type':('func f()->const i64{for(i in {{return 12;}}..{{return 13;}}){};return 0;}f();',2,'','index type is not established'),
 'remaining_bound_must_be_integer':('func f()->const i64{for(i in {{return 12;}}..false){};return 0;}f();',2,'','integer endpoints'),
 'later_bound_still_checks_reads':('func f()->const i64{val x=make local i64;for(i in {{return 12;}}..x){};return 0;}f();',2,'','before definite initialization'),
 'body_still_checks_reads':('func f()->const i64{val x=make local i64;for(i in 0..{{return 12;}}){x;};return 0;}f();',2,'','before definite initialization'),
 'else_still_checks_reads':('func f()->const i64{val x=make local i64;for(i in 0..{{return 12;}}){}else{x;};return 0;}f();',2,'','before definite initialization'),
 'body_declaration_still_forbidden':('func f()->const i64{for(i in 0..{{return 12;}}){val x=1;};return 0;}f();',2,'','nested per-iteration'),
 'else_declaration_still_forbidden':('func f()->const i64{for(i in 0..{{return 12;}}){}else{val x=1;};return 0;}f();',2,'','nested per-iteration'),
 'index_still_absent_from_else':('func f()->const i64{for(i in 0..{{return 12;}}){}else{i;};return 0;}f();',2,'','unknown scalar value'),
 'header_write_survives_throw':(OBS+EX+'proc f()->void{val x=make local i64;try{for(i in {x<-13;0}..{{throw error();}}){x<-99;}else{x<-98;};}catch(e:const MissingReturnException){observe(x);};}f();',0,'13\n','[-Wmissing-return]'),
 'unentered_body_does_not_initialize':(EX+'func f()->const i64{val x=make local i64;try{for(i in 0..{{throw error();}}){x<-1;}else{x<-2;};}catch(e:const MissingReturnException){};return x;}f();',2,'','before definite initialization'),
 'bound_null_read_precedes_return':(OBS+NPE+'proc f()->const i64{ptr p:i64=null;try{for(i in p..{{return 14;}}){};}catch(e:const NullPointerException){observe(15);};return 16;}behave @print(f());',0,'15\n16\n',''),
 'bound_null_read_obligation_remains':(NPE+'func f(ptr p:i64 in O)->const i64 where {owner O}{for(i in p..{{return 16;}}){};return 0;}',2,'','requires a covering catch'),
 'bound_finally_replaces_return':(OBS+'proc f()->const i64{defer observe(1);for(i in 0..try{return 17;}finally{return 18;}){}else{observe(99);};return 0;}behave @print(f());',0,'1\n18\n',''),
 'bound_cleanup_recovers_return':(IO+EX+'func f()->const i64{defer catch(e:const MissingReturnException){}finally{};defer{missing();};for(i in 0..{{return 19;}}){};return 0;}behave @print(f());',0,'19\n','[-Wmissing-return]'),
 'nested_loop_preserves_prior_registration':(OBS+'proc f()->const i64{defer observe(9);for(i in 0..3){defer observe(i);for(j in 0..{{return 20;}}){observe(99);}else{observe(98);};};return 0;}behave @print(f());',0,'0\n9\n20\n',''),
 'fold_bound_skips_initializer':(OBS+'proc f()->const i64{return fold(i in 0..{{return 21;}},n={observe(99);0}){observe(98);n+i};}behave @print(f());',0,'21\n',''),
 'fold_first_bound_returns':(OBS+'proc f()->const i64{return fold(i in {{return 22;}}..{observe(99);3},n={observe(98);0}){n+i};}behave @print(f());',0,'22\n',''),
 'fold_body_type_still_checked':('func f()->const i64{return fold(i in 0..{{return 22;}},n=0){false};}f();',2,'','fold body does not satisfy'),
 'fold_initializer_read_still_checked':('func f()->const i64{val x=make local i64;return fold(i in 0..{{return 22;}},n=x){n};}f();',2,'','before definite initialization'),
 'fold_accumulator_without_type_remains_unproved':('func f()->const i64{return fold(i in 0..3,n={{return 22;}}){n};}f();',2,'','non-value or abruptly completing operand'),
 'generic_index_contexts':(IO+'func f(x:T)->const i64 where T{for(i in x..{{return 23;}}){};return 0;}behave @print(f(0u8));behave @print(f(0u64));',0,'23\n23\n',''),
 'generic_noninteger_still_rejects':('func f(x:T)->const i64 where T{for(i in x..{{return 23;}}){};return 0;}f(false);',2,'','integer endpoints'),
 'header_returns_complete_callable':(IO+'func base(x:i64)->const i64{return x;}func factory()->const (func(i64)->const i64){func f=base;for(i in 0..{{return func[f](x:i64)->const i64{return f(x)+1;};}}){};return base;}behave @print(factory()(23));',0,'24\n',''),
 'nested_null_test_evidence':(IO+TARGET+'func f()->const i64{target(isnull({{return 25;}}));return 0;}behave @print(f());',0,'25\n',''),
 'nested_cause_evidence':(IO+TARGET+'func f()->const i64{target(cause_at({{return 26;}},0u64));return 0;}behave @print(f());',0,'26\n',''),
 'nested_callable_evidence':(IO+TARGET+'func inner(x:i64)->const bool{return true;}func f()->const i64{func invoke=inner;target(invoke({{return 27;}}));return 0;}behave @print(f());',0,'27\n',''),
 'nested_callable_arity_still_checked':(TARGET+'func inner(x:i64)->const bool{return true;}func f()->const i64{func invoke=inner;target(invoke({{return 27;}},1));return 0;}f();',2,'','argument count'),
 'nested_conditional_evidence':(IO+TARGET+'func f()->const i64{target(if({{return 28;}}){true}else{false});return 0;}behave @print(f());',0,'28\n',''),
 'nested_logical_evidence':(IO+TARGET+'func f()->const i64{target(({{return 29;}})&&false);return 0;}behave @print(f());',0,'29\n',''),
 'short_circuit_right_not_unconditionally_abrupt':(TARGET+'func f(flag:bool)->const i64{target(flag&&{{return 29;}});return 0;}f(false);',2,'','no applicable method'),
 'nested_swap_evidence':(IO+TARGET+'func f()->const i64{val x=1;target(x<->{{return 30;}});return 0;}behave @print(f());',0,'30\n',''),
 'nested_print_evidence':(IO+TARGET+'proc f()->const i64{target(behave @print({{return 31;}}));return 0;}behave @print(f());',0,'31\n',''),
 'nested_field_evidence':(IO+TARGET+'func f()->const i64{target(({{return 32;}}).category);return 0;}behave @print(f());',0,'32\n',''),
 'nested_field_name_still_checked':(TARGET+'func f()->const i64{target(({{return 32;}}).missing);return 0;}f();',2,'','unknown intrinsic exception field'),
 'nested_deferred_preparation_evidence':(OBS+TARGET+'proc f()->const i64{defer observe(1);target({defer observe({{return 33;}});false});return 0;}behave @print(f());',0,'1\n33\n',''),
 'deferred_body_is_not_registration_completion':(TARGET+'func f()->const i64{target({defer{return 33;};false});return 0;}f();',2,'','no applicable method'),
 'nested_loop_setup_evidence':(IO+TARGET+'func f()->const i64{target({for(i in 0..{{return 34;}}){};false});return 0;}behave @print(f());',0,'34\n',''),
 'nested_fold_setup_evidence':(IO+TARGET+'func f()->const i64{target(fold(i in 0..{{return 35;}},n=false){n});return 0;}behave @print(f());',0,'35\n',''),
 'abrupt_operand_has_no_readonly_view':(IO+'func take(ref x:i64 in O)->void where {owner O}{}func f()->const i64{const val x=1;take({return 36;x});return 0;}behave @print(f());',0,'36\n',''),
 'abrupt_prefix_has_no_readonly_view':(IO+'func take(ref x:i64 in O,y:i64)->void where {owner O}{}func f()->const i64{const val x=1;take({return 37;x},1);return 0;}behave @print(f());',0,'37\n',''),
}
runner.CASES=cases
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
