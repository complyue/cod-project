#!/usr/bin/env python3
"""Known named-call contracts preserve abrupt preparation and cleanup."""
from pathlib import Path
import operand_completion_tests as fixture
runner=fixture.runner
IO=fixture.IO
OBS=fixture.OBS
EX=fixture.EX
VIEW=fixture.VIEW
NPE='using "std:exceptions" NullPointerException;'
selected=[
 'callable_view_first_operand_return','callable_view_later_operand_return',
 'live_later_argument_read_still_rejects','aborted_invocation_has_no_throws_obligation',
 'surviving_invocation_keeps_throws_obligation','deferred_preparation_returns',
 'deferred_preparation_throws','deferred_preparation_prior_registration',
]
cases={}
for name in selected:
    source,code,out,diagnostic=runner.CASES[name]
    target='first' if 'invoke=first;' in source else 'target' if 'invoke=target;' in source else 'show'
    source=source.replace('func invoke='+target+';','').replace('proc invoke='+target+';','').replace('invoke(',target+'(')
    cases[name.replace('callable_view_','named_')]=(source,code,out,diagnostic)
cases.update({
 'generic_other_argument_establishes_type':(IO+'func same(x:T,y:T)->const T where T{return x;}func f()->const i64{return same({{return 9;}},1);}behave @print(f());',0,'9\n',''),
 'overload_other_argument_establishes_type':(IO+'func pick(x:i64,tag:bool)->const i64{return x;}func pick(x:bool,tag:i64)->const bool{return x;}func f()->const i64{return pick({{return 10;}},false);}behave @print(f());',0,'10\n',''),
 'nested_abrupt_call':(IO+VIEW+'func f()->const i64{return first(first({{return 11;}},1),2);}behave @print(f());',0,'11\n',''),
 'late_lexical_declaration':(IO+VIEW+'func f()->const i64{return first({{return 12;}},{val later=2;later});}behave @print(f());',0,'12\n',''),
 'finally_replaces_operand_return':(OBS+VIEW+'proc f()->const i64{defer observe(1);return first(try{return 12;}finally{return 13;},1);}behave @print(f());',0,'1\n13\n',''),
 'callee_body_still_checked':('func target(x:i64)->const bool{return x;}func f()->const i64{target({{return 14;}});return 0;}f();',2,'','return type'),
 'known_later_type_still_checked':(VIEW+'func f()->const i64{return first({{return 14;}},false);}f();',2,'','parameter type'),
 'unknown_name_still_rejects':('func f()->const i64{return absent({{return 14;}});}f();',2,'','unknown callable'),
 'function_procedure_boundary':('proc target(x:i64)->void{}func f()->const i64{target({{return 14;}});return 0;}f();',2,'','function cannot invoke a procedure'),
 'generic_without_type_evidence_remains_unproved':('func target(x:T)->const T where T{return x;}func f()->const i64{return target({{return 14;}});}f();',2,'','parameter type'),
 'borrowed_ptr_precedes_return':(IO+'func target(ptr p:i64 in O,x:i64)->void where {owner O}{}func f()->const i64{val x=make local i64;target(x,{{return 15;}});return 0;}behave @print(f());',0,'15\n',''),
 'borrowed_read_precedes_return':(OBS+NPE+'func target(p:i64 in O,x:i64)->void where {owner O}{}proc f()->const i64{val x=1;ptr p=x;p=null;try{target(p,{{return 15;}});}catch(e:const NullPointerException){observe(16);};return 17;}behave @print(f());',0,'16\n17\n',''),
 'borrowed_operand_itself_returns':(IO+'func target(ref p:i64 in O)->void where {owner O}{}func f()->const i64{target({{return 18;}});return 0;}behave @print(f());',0,'18\n',''),
 'ref_preparation_initialization_still_required':('func target(ref p:i64 in O,x:i64)->void where {owner O}{}func f()->const i64{val p=make local i64;target(p,{{return 18;}});return 0;}f();',2,'','before definite initialization'),
 'aborted_call_does_not_initialize':(IO+EX+NPE+'func set(ptr p:i64 in O,x:i64)->void throws NullPointerException where {owner O}{p<-x;}proc run()->void{val x=make local i64;try{set(x,{{throw error();}});}catch(e:const MissingReturnException){behave @print(x);};}run();',2,'','before definite initialization'),
 'operand_write_survives_aborted_call':(IO+EX+'func target(x:i64,y:i64)->void{}proc run()->void{val x=make local i64;try{target({x<-19;x},{{throw error();}});}catch(e:const MissingReturnException){behave @print(x);};}run();',0,'19\n','[-Wmissing-return]'),
 'defer_borrowed_preparation_returns':(OBS+'proc target(ptr p:i64 in O,x:i64)->void where {owner O}{observe(x);}proc f()->const i64{val x=make local i64;defer observe(1);defer target(x,{{return 20;}});return 0;}behave @print(f());',0,'1\n20\n',''),
 'return_complete_callable_from_operand':(IO+'func base(x:i64)->const i64{return x;}func target(x:i64)->void{}func wrap(f:func(i64)->const i64)->const (func(i64)->const i64){return func[f](x:i64)->const i64{return f(x)+1;};}func make_callback()->const (func(i64)->const i64){target({{return wrap(base);}});return base;}behave @print(make_callback()(20));',0,'21\n',''),
 'lexical_recursive_aborted_preparation':(IO+'proc run()->void{func recur(ref x:i64 in O,n:i64)->const i64 where {owner O}{if(n==0){return x;}else{return recur(x,{{return 22;}});};};val x=1;behave @print(recur(x,30));}run();',0,'22\n',''),
 'generic_shared_normal_and_abrupt':(IO+VIEW+'func run(tag:T)->const i64 where T{return first(if(tag==tag){return 23;}else{1},2);}behave @print(run(true));behave @print(run(1));',0,'23\n23\n',''),
 'user_operator_aborted_preparation':(IO+'func (+)(x:i64,y:i64)->const i64{return x;}func f()->const i64{return ({{return 24;}})+1;}behave @print(f());',0,'24\n',''),
 'null_test_operand_return':(IO+'func f()->const i64{isnull({{return 25;}});return 0;}behave @print(f());',0,'25\n',''),
 'null_test_operand_throw':(OBS+EX+'proc run()->void{defer observe(1);isnull({{throw error();}});observe(99);}try{run();}catch(e:const MissingReturnException){observe(26);};',0,'1\n26\n','[-Wmissing-return]'),
 'cause_first_operand_returns':(IO+'func f()->const i64{cause_at({{return 27;}},0u64);return 0;}behave @print(f());',0,'27\n',''),
 'cause_index_returns':(IO+EX+'func f()->const i64{cause_at(error(),{{return 28;}});return 0;}behave @print(f());',0,'28\n','[-Wmissing-return]'),
 'intrinsic_cause_index_returns':(IO+EX+'func f()->const i64{intrinsic (cause_at)(error(),{{return 29;}});return 0;}behave @print(f());',0,'29\n','[-Wmissing-return]'),
 'cause_later_type_still_checked':('func f()->const i64{cause_at({{return 29;}},false);return 0;}f();',2,'','cause_at requires an exception and a u64 index'),
 'preparation_return_recovers_through_cleanup':(IO+EX+'func target(x:i64)->void{}func f()->const i64{defer catch(e:const MissingReturnException){}finally{};defer{missing();};target({{return 30;}});return 0;}behave @print(f());',0,'30\n','[-Wmissing-return]'),
})
runner.CASES=cases
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
