#!/usr/bin/env python3
"""Structural callable type relations, same-T and native completion contracts."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;\n'
BASE='func inc(x:i64)->const i64{return x+1;}func yes(x:bool)->const bool{return x;}func positive(x:i64)->const bool{return x>0;}\n'
ID='func identity(x:T)->const T where T{return x;}\n'
EX='using "std:exceptions" NullPointerException;using "std:exceptions" MissingReturnException;using "std:exceptions" SystemException;\n'
APPLY='func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}\n'
WRAP='func wrap(f:func(i64)->const i64)->func(i64)->const i64{return func[f](x:i64)->const i64{return f(x)+1;};}\n'
runner.CASES={
 'infer_callback_parameter':(IO+BASE+APPLY+'behave @print(apply(inc,1));behave @print(apply(yes,true));',0,'2\ntrue\n',''),
 'infer_callback_result':(IO+'func number()->const i64{return 3;}func read(f:func()->const T)->const T where T{return f();}behave @print(read(number));',0,'3\n',''),
 'distinct_input_result':(IO+BASE+'func transform(f:func(X)->const Y,x:X)->const Y where {X,Y}{return f(x);}behave @print(transform(positive,4));',0,'true\n',''),
 'nested_factory_signature':(IO+BASE+'func factory()->func(i64)->const i64{return inc;}func use(f:func()->func(T)->const T,x:T)->const T where T{return f()(x);}behave @print(use(factory,4));',0,'5\n',''),
 'whole_callable_result_binder':(IO+BASE+'func factory()->const func(i64)->const i64{return inc;}func read(f:func()->const T)->const T where T{return f();}behave @print(read(factory)(5));',0,'6\n',''),
 'prior_argument_context':(IO+ID+'func apply(x:T,f:func(T)->const T)->const T where T{return f(x);}behave @print(apply(7,identity));behave @print(apply(true,identity));',0,'7\ntrue\n',''),
 'prior_nested_context':(IO+BASE+ID+'func apply(a:func(T)->const T,b:func(T)->const T,x:T)->const T where T{return b(a(x));}behave @print(apply(inc,identity,7));',0,'8\n',''),
 'callback_and_scalar_same_type':(BASE+APPLY+'apply(inc,true);',2,'','no applicable method'),
 'callback_internal_same_type':(BASE+'func use(f:func(T)->const T)->const i64 where T{return 0;}use(positive);',2,'','no applicable method'),
 'repeated_callback_inputs':(IO+'func add(x:i64,y:i64)->const i64{return x+y;}func use(f:func(T,T)->const T,x:T)->const T where T{return f(x,x);}behave @print(use(add,4));',0,'8\n',''),
 'reject_distinct_callback_inputs':('func first(x:i64,y:bool)->const i64{return x;}func use(f:func(T,T)->const T)->const i64 where T{return 0;}use(first);',2,'','no applicable method'),
 'const_parameter_contract':(IO+'func read(x:const i64)->const i64{return x;}func use(f:func(const T)->const T,x:T)->const T where T{return f(x);}behave @print(use(read,9));',0,'9\n',''),
 'const_parameter_not_erased':('func read(x:const i64)->const i64{return x;}'+APPLY+'apply(read,1);',2,'','callable view contract'),
 'procedure_parameter':(IO+'proc emit(x:i64)->void{behave @print(x);}proc use(f:proc(T)->void,x:T)->void where T{f(x);}use(emit,10);',0,'10\n',''),
 'callable_category_not_erased':(IO+'proc emit(x:i64)->const i64{behave @print(x);return x;}'+APPLY+'apply(emit,1);',2,'','callable view contract'),
 'checked_throws_contract':(IO+EX+'func f(x:i64)->const i64 throws NullPointerException{return x;}func use(f:func(T)->const T throws NullPointerException,x:T)->const T throws NullPointerException where T{return f(x);}try{behave @print(use(f,11));}catch(e:const NullPointerException){};',0,'11\n',''),
 'throws_contract_not_erased':(EX+'func f(x:i64)->const i64 throws NullPointerException{return x;}'+APPLY+'apply(f,1);',2,'','required checked exception'),
 'structural_overload_order':(IO+BASE+'func pick(f:F)->const i64 where F{return 0;}func pick(f:func(T)->const T)->const i64 where T{return 1;}func pick(f:func(i64)->const i64)->const i64{return 2;}behave @print(pick(inc));behave @print(pick(yes));behave @print(pick(positive));',0,'2\n1\n0\n',''),
 'repeated_nested_constraint_specificity':(IO+BASE+'func pick(f:func(A)->const B,x:C)->const i64 where {A,B,C}{return 0;}func pick(f:func(T)->const T,x:T)->const i64 where T{return 12;}behave @print(pick(inc,0));behave @print(pick(positive,0));',0,'12\n0\n',''),
 'alpha_equivalent_duplicate':('func pick(f:func(T)->const T)->const i64 where T{return 0;}func pick(f:func(U)->const U)->const i64 where U{return 1;}',2,'','duplicate method signature'),
 'incomparable_nested_constraints':(BASE+'func pick(f:func(T)->const U,x:T)->const i64 where {T,U}{return 1;}func pick(f:func(T)->const U,x:U)->const i64 where {T,U}{return 2;}pick(inc,0);',2,'','ambiguous methods'),
 'distinct_nested_constraints':(IO+BASE+'func pick(f:func(T)->const U,x:T)->const i64 where {T,U}{return 1;}func pick(f:func(T)->const U,x:U)->const i64 where {T,U}{return 2;}behave @print(pick(positive,0));behave @print(pick(positive,true));',0,'1\n2\n',''),
 'captured_argument':(IO+BASE+WRAP+APPLY+'func f=wrap(inc);behave @print(apply(f,11));',0,'13\n',''),
 'captured_generic_factory_argument':(IO+BASE+WRAP+'func pass(f:func(T)->const T)->func(T)->const T where T{return f;}func f=pass(wrap(inc));behave @print(f(12));',0,'14\n',''),
 'recursive_higher_order':(IO+BASE+'func recurse(f:func(T)->const T,x:T,n:i64)->const T where T{if(n==0){return f(x);}else{return recurse(f,x,n-1);};}behave @print(recurse(inc,14,60));behave @print(recurse(yes,true,20));',0,'15\ntrue\n',''),
 'void_result_is_determined':(IO+'func empty()->const void{}func read(f:func()->const T)->const T where T{return f();}read(empty);behave @print(16);',0,'16\n',''),
 'void_result_normal_completion':(IO+'func empty()->const void{}func finish(f:func()->const T)->const T where T{f();}finish(empty);behave @print(17);',0,'17\n',''),
 'mixed_void_and_value_completion':(IO+EX+'func empty()->const void{}func number()->const i64{return 1;}func finish(f:func()->const T)->const T where T{f();}finish(empty);try{finish(number);{}}catch(e:const MissingReturnException){behave @print(18);};',0,'18\n','callable may finish without an explicit return'),
 'void_does_not_equal_unbound':('func factory(f:func()->const U)->const T where {T,U}{}func empty()->const void{}factory(empty);',2,'','type parameter is not determined'),
}

runner.CASES.update({
 'selected_body_is_checked':(BASE+'func use(f:func(T)->const T,x:T)->const T where T{return f(x)+1;}use(yes,true);',2,'','no applicable numeric operation'),
 'prior_overload_filter':(IO+ID+'func use(x:i64,f:func(i64)->const i64)->const i64{return f(x);}func use(x:bool,f:func(bool)->const bool)->const bool{return f(x);}behave @print(use(19,identity));behave @print(use(true,identity));',0,'19\ntrue\n',''),
 'later_argument_context':(IO+ID+APPLY+'behave @print(apply(identity,1));',0,'1\n',''),
 'independent_common_exceptions':(EX+'func pick(a:SystemException,b:SystemException)->const i64{return 0;}func pick(a:T,b:T)->const i64 where T{return 1;}func probe(e:MissingReturnException)->const i64{return pick(e,e);}',2,'','ambiguous methods'),
 'concrete_exception_pair':(EX+'func pick(a:SystemException,b:SystemException)->const i64{return 0;}func pick(a:T,b:T)->const i64 where T{return 1;}func pick(a:MissingReturnException,b:MissingReturnException)->const i64{return 2;}func probe(e:MissingReturnException)->const i64{return pick(e,e);}',0,'',''),
 'nested_common_exception_is_exact':(EX+'func f(e:MissingReturnException)->const u64{return e.category;}func use(f:func(SystemException)->const T)->const i64 where T{return 0;}use(f);',2,'','no applicable method'),
 'nested_common_exception_generic':(EX+'func f(e:SystemException)->const u64{return e.category;}func use(f:func(T)->const u64)->const i64 where T{return 0;}use(f);',0,'',''),
 'alpha_equivalent_independent_binders':('func pick(f:func(T,U)->const T)->const i64 where {T,U}{return 0;}func pick(f:func(B,A)->const B)->const i64 where {A,B}{return 1;}',2,'','duplicate method signature'),
 'deferred_generic_result':(IO+'func empty()->const void{}func number()->const i64{return 20;}func finish(f:func()->const T)->const T where T{defer{return f();};}finish(empty);behave @print(finish(number));',0,'20\n',''),
 'captured_value_missing_return':(IO+EX+'func number()->const i64{return 1;}func wrap(f:func()->const i64)->func()->const i64{return func[f]()->const i64{return f();};}func empty()->const void{}func finish(f:func()->const T)->const T where T{f();}finish(empty);try{finish(wrap(number));{}}catch(e:const MissingReturnException){behave @print(21);};',0,'21\n','callable may finish without an explicit return'),
})
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
