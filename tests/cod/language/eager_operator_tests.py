#!/usr/bin/env python3
"""Eager operators share semantic method selection and checked builtin delegation."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
BOOL = 'func (+)(x:bool,y:bool)->const bool{return x||y;}'
ADD = 'func (+)(x:i64,y:i64)->const i64{return intrinsic (+)(x,y)*2;}'
ERROR = '''using "std:exceptions" MissingReturnException;
using "std:exceptions" SystemException;
func missing()->const i64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
'''
runner.CASES = {
 'infix_parenthesized_and_intrinsic':(IO+ADD+'behave @print(2+3);behave @print((+)(2,3));behave @print(intrinsic (+)(2,3));',0,'10\n10\n5\n',''),
 'builtin_fallback':(IO+BOOL+'behave @print(false+true);behave @print(2+3);behave @print(1u8+2u64);',0,'true\n5\n3\n',''),
 'same_t_generic_operator':(IO+'func (+)(x:T,y:T)->const T where T{return x;}behave @print(1+2);behave @print(false+true);behave @print(1u8+2u64);',0,'1\nfalse\n3\n',''),
 'concrete_over_generic':(IO+'func (+)(x:T,y:T)->const T where T{return x;}func (+)(x:i64,y:i64)->const i64{return y;}behave @print(1+2);behave @print(false+true);',0,'2\nfalse\n',''),
 'mixed_callable_kinds':(IO+'func (+)(x:i64,y:i64)->const i64{return intrinsic (+)(x,y);}proc (+)(x:bool,y:bool)->const bool{behave @print(7);return x||y;}behave @print(1+2);behave @print(false+true);',0,'3\n7\ntrue\n',''),
 'function_selects_function':(IO+'proc (+)(x:bool,y:bool)->const bool{return x||y;}func sum(x:i64,y:i64)->const i64{return x+y;}behave @print(sum(2,3));',0,'5\n',''),
 'procedure_result_void':(IO+'proc (+)(x:bool,y:bool)->void{behave @print(x);behave @print(y);}false+true;',0,'false\ntrue\n',''),
 'operands_once_left_to_right':(IO+'proc arg(x:i64)->const i64{behave @print(x);return x;}'+ADD+'behave @print(arg(1)+arg(2));',0,'1\n2\n6\n',''),
 'operand_snapshot_before_later_write':(IO+ADD+'val x<-1;behave @print(x+(x<-3));behave @print(x);',0,'8\n3\n',''),
 'parameter_copies_are_independent':(IO+'func (+)(x:i64,y:i64)->const i64{x<-9;return y;}val x<-1,y<-2;behave @print(x+y);behave @print(x);',0,'2\n1\n',''),
 'unary_and_binary_family':(IO+'func (-)(x:i64)->const i64{return intrinsic (-)(x)*2;}func (-)(x:i64,y:i64)->const i64{return intrinsic (-)(x,y)*3;}behave @print(-2);behave @print(9-2);behave @print(intrinsic (-)(2));',0,'-4\n21\n-2\n',''),
 'unary_plus_method':(IO+'func (+)(x:bool)->const bool{return !x;}behave @print((+)(false));behave @print((+)(3));',0,'true\n3\n',''),
 'logical_negation_method':(IO+'func (!)(x:i64)->const bool{return x==0;}behave @print(!0);behave @print(!1);behave @print(!false);',0,'true\nfalse\ntrue\n',''),
 'comparison_method':(IO+'func (<)(x:bool,y:bool)->const bool{return !x&&y;}behave @print(false<true);behave @print(true<false);behave @print(1<2);',0,'true\nfalse\ntrue\n',''),
 'equality_method':(IO+'func (==)(x:i64,y:i64)->const bool{return intrinsic (!=)(x,y);}behave @print(1==2);behave @print(1==1);behave @print(true==true);',0,'true\nfalse\ntrue\n',''),
 'multiplicative_methods':(IO+'func (*)(x:bool,y:bool)->const bool{return x&&y;}func (/)(x:bool,y:bool)->const bool{return x||y;}func (%)(x:bool,y:bool)->const bool{return x!=y;}behave @print(true*false);behave @print(true/false);behave @print(true%false);',0,'false\ntrue\ntrue\n',''),
 'generic_body_local_selection':(IO+BOOL+'func twice(x:T)->const T where T{return x+x;}behave @print(twice(4));behave @print(twice(true));',0,'8\ntrue\n',''),
 'generic_method_recursion':(IO+BOOL+'func repeat(x:T,n:i64)->const T where T{if(n==0){return x;}else{return repeat(x,n-1)+x;};}behave @print(repeat(1,30));behave @print(repeat(true,40));',0,'31\ntrue\n',''),
 'overloaded_bound_runtime_selection':(IO+'func (-)(x:i64)->const i64{return x;}for(i in -1..0){behave @print(99);}else{behave @print(7);};',0,'7\n',''),
 'overloaded_bound_does_not_prove_initialization':('func (-)(x:i64)->const i64{return x;}val x<-make local i64;for(i in -1..0){x<-7;};x;',2,'','before definite initialization'),
 'builtin_bound_proves_initialization':(IO+'val x<-make local i64;for(i in intrinsic (-)(1)..0){x<-i;};behave @print(x);',0,'-1\n',''),
 'recursive_operator_is_not_builtin':(IO+'func (+)(x:i64,y:i64)->const i64{if(y==0){return x;}else{return (+)(intrinsic (+)(x,1),y-1);};}behave @print(2+3);',0,'5\n',''),
 'lexical_body_uses_operator':(IO+BOOL+'func f=func[](x:bool)->const bool{return (+)(x,true);};behave @print(f(false));',0,'true\n',''),
 'lexical_body_uses_parenthesized_builtin':(IO+'func f=func[](x:i64)->const i64{return (+)(x,1);};behave @print(f(2));',0,'3\n',''),
 'pure_intrinsic_ignores_procedure':(IO+'proc (+)(x:i64,y:i64)->const i64{behave @print(99);return x;}func sum(x:i64,y:i64)->const i64{return intrinsic (+)(x,y);}behave @print(sum(2,3));',0,'5\n',''),
 'deferred_method_arguments':(IO+'proc (+)(x:i64,y:i64)->void{behave @print(intrinsic (+)(x,y));}{{val x<-1;defer (+)(x,2);x<-9;behave @print(4);}};',0,'4\n3\n',''),
 'deferred_builtin_failure_runs_cleanup':(IO+'proc show(x:i64)->void{behave @print(x);}{{defer show(7);defer intrinsic (/)(1,0);behave @print(6);}};',70,'6\n7\n','division by zero'),
 'builtin_negation_preserves_null_test':(IO+'proc show(ptr p:i64 in O)->void where {owner O}{if(intrinsic (!)(isnull(p))){behave @print(p);}else{};}val x<-8;show(x);',0,'8\n',''),
 'overloaded_negation_is_not_null_refinement':('func (!)(x:bool)->const bool{return x;}proc show(ptr p:i64 in O)->void where {owner O}{if(!isnull(p)){p;}else{0};}',2,'','requires a covering catch'),
 'common_exception_argument_preparation':(IO+ERROR+'func (+)(e:SystemException,n:u64)->const u64{return intrinsic (+)(e.category,n);}behave @print(error()+2u64);',0,'7\n','[-Wmissing-return]'),
 'explicit_cause_builtin':(IO+ERROR+'val e<-error();val chain<-try{throw e from e;}catch(x:const MissingReturnException){x};behave @print(intrinsic (cause_at)(chain,0u64).category);',0,'5\n','[-Wmissing-return]'),
 'later_operand_failure_skips_method':(IO+ERROR+'proc (+)(x:i64,y:i64)->void{behave @print(99);}try{1+missing();}catch(e:const MissingReturnException){behave @print(1);};',0,'1\n','[-Wmissing-return]'),
 'checked_method_failure':(IO+'using "std:exceptions" NullPointerException;func (+)(x:bool,y:bool)->const bool throws NullPointerException{ptr p:bool=null in here;return p;}try{behave @print(false+true);}catch(e:const NullPointerException){behave @print(1);};',0,'1\n',''),
 'callable_operand_and_result':(IO+'func one()->const i64{return 1;}func two()->const i64{return 2;}func (+)(f:func()->const i64,g:func()->const i64)->const func()->const i64{return func[f,g]()->const i64{return f()+g();};}func f=one+two;behave @print(f());',0,'3\n',''),
 'function_cannot_select_procedure':('proc (+)(x:i64,y:i64)->const i64{return x;}func f()->const i64{return 1+2;}',2,'','function cannot invoke a procedure'),
 'selected_bad_body_does_not_fallback':('func (+)(x:i64,y:i64)->const i64{return false;}1+2;',2,'','return type'),
 'selected_generic_bad_body_does_not_fallback':('func (+)(x:T,y:T)->const T where T{return !x;}1+2;',2,'','logical negation requires bool'),
 'overload_ambiguity':('func (+)(x:i64,y:T)->const i64 where T{return x;}func (+)(x:T,y:i64)->const i64 where T{return y;}1+2;',2,'','ambiguous methods'),
 'duplicate_kind_is_not_tiebreaker':('func (+)(x:i64,y:i64)->const i64{return x;}proc (+)(x:i64,y:i64)->const i64{return y;}',2,'','duplicate method signature'),
 'intrinsic_does_not_fallback_to_user':(BOOL+'intrinsic (+)(false,true);',2,'','no applicable numeric operation'),
 'intrinsic_wrong_arity':('intrinsic (!)(true,false);',2,'','builtin operator argument count'),
 'borrowed_operator_method':(IO+'func (+)(ref x:i64 in O,ref y:i64 in O)->const i64 where {owner O}{x<-y;return x;}val x<-1,y<-3;behave @print(x+y);behave @print(x);',0,'3\n3\n',''),
 'short_circuit_overload_boundary':('func (&&)(x:bool,y:bool)->const bool{return x;}',2,'','short-circuit operators are control expressions'),
 'signed_minimum_overload_boundary':('func (-)(x:i64)->const i64{return x;}-9223372036854775808;',2,'','signed minimum literal with overloaded unary negation'),
 'ordinary_family_kind_boundary':('func f(x:i64)->const i64{return x;}proc f(x:bool)->const bool{return x;}',2,'','mixed function/procedure family'),
 'intrinsic_overflow_cleanup':(IO+'proc show(x:i64)->void{behave @print(x);}{{defer show(9);intrinsic (+)(9223372036854775807,1);}};',70,'9\n','i64 addition overflow'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
