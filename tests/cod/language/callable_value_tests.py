#!/usr/bin/env python3
"""Complete empty-environment callable values and prepared invocation."""
from pathlib import Path
import sys
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
PRE = IO + 'using "std:exceptions" NullPointerException;\n'
INC = 'func inc(x:i64)->const i64{return x+1;}func dec(x:i64)->const i64{return x-1;}\n'
PROC = 'proc one(x:i64)->void{behave @print(x+1);}proc two(x:i64)->void{behave @print(x+2);}\n'
MISS = 'using "std:exceptions" MissingReturnException;func missing()->const i64{}\n'
WARN = 'callable may finish without an explicit return'
runner.CASES = {
    'module_function_value': (IO+INC+'func f=inc;behave @print(f(1));',0,'2\n',''),
    'anonymous_value': (IO+'func f=func(x:i64)->const i64{return x+2;};behave @print(f(2));',0,'4\n',''),
    'explicit_empty_capture': (IO+'func f=func[](x:i64)->const i64{return x+3;};behave @print(f(2));',0,'5\n',''),
    'named_local_definition': (IO+'proc run()->void{func f(x:i64)->const i64{return x+4;};behave @print(f(2));}run();',0,'6\n',''),
    'independent_copy': (IO+INC+'func f=inc;func g=f;f<-dec;behave @print(g(10));behave @print(f(10));',0,'11\n9\n',''),
    'conditional_value': (IO+INC+'func f=if(false){inc}else{dec};behave @print(f(12));',0,'11\n',''),
    'value_expression_call': (IO+INC+'behave @print((if(true){inc}else{dec})(12));',0,'13\n',''),
    'anonymous_expression_call': (IO+'behave @print((func(x:i64)->const i64{return x+5;})(1));',0,'6\n',''),
    'copy_outlives_original_holder': (IO+INC+'func f={{func g=inc;g}};behave @print(f(14));',0,'15\n',''),
    'pointer_observes_replacement': (PRE+INC+'func f=inc;ptr p=f;f<-dec;behave @print(p(16));',0,'15\n',''),
    'nullable_pointer': (PRE+INC+'func f=inc;ptr p=f;p=null;try{p(1);{}}catch(e:const NullPointerException){behave @print(17);};',0,'17\n',''),
    'nullable_pointer_requires_catch': (PRE+INC+'func f=inc;ptr p=f;p=null;p(1);',2,'','requires a covering catch'),
    'expired_pointer': (IO+INC+'ptr p={{func f=inc;f}};p(1);',2,'','lexical owner has ended'),
    'null_callee_precedes_operands': (PRE+INC+'proc arg()->const i64{behave @print(99);return 1;}func f=inc;ptr p=f;p=null;try{p(arg());{}}catch(e:const NullPointerException){behave @print(18);};',0,'18\n',''),
    'target_snapshot_before_operand': (IO+INC+'func f=inc;behave @print(f({{f<-dec;20}}));behave @print(f(20));',0,'21\n19\n',''),
    'procedure_value': (IO+PROC+'proc f=one;f(21);f<-two;f(21);',0,'22\n23\n',''),
    'deferred_target_snapshot': (IO+PROC+'proc f=one;{{defer f(23);f<-two;}};f(23);',0,'24\n25\n',''),
    'deferred_expression_target': (IO+PROC+'{{defer (if(true){one}else{two})(25);}};',0,'26\n',''),
    'deferred_operand_replacement': (IO+PROC+'proc f=one;{{defer f({{f<-two;27}});}};f(27);',0,'28\n29\n',''),
    'delayed_block_reads_current_value': (IO+PROC+'proc f=one;{{defer{f(29);};f<-two;}};',0,'31\n',''),
    'default_parameter_copy': (IO+INC+'func apply(f:func(i64)->const i64,x:i64)->const i64{f<-dec;return f(x);}func f=inc;behave @print(apply(f,32));behave @print(f(32));',0,'31\n33\n',''),
    'parameter_preparation_snapshot': (IO+INC+'func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}func f=inc;behave @print(apply(f,{{f<-dec;33}}));',0,'34\n',''),
    'factory_result': (IO+INC+'func factory()->func(i64)->const i64{return inc;}func f=factory();behave @print(f(34));',0,'35\n',''),
    'factory_expression_call': (IO+INC+'func factory()->func(i64)->const i64{return inc;}behave @print(factory()(35));',0,'36\n',''),
    'nested_factory_result': (IO+INC+'func inner()->func(i64)->const i64{return inc;}func outer()->func()->func(i64)->const i64{return inner;}func factory=outer();func f=factory();behave @print(f(36));',0,'37\n',''),
    'returned_snapshot_before_cleanup': (IO+INC+'func factory()->func(i64)->const i64{func f=inc;defer{f<-dec;};return f;}func f=factory();behave @print(f(37));',0,'38\n',''),
    'deferred_return_replaces_value': (IO+INC+'func factory()->func(i64)->const i64{defer{return dec;};return inc;}func f=factory();behave @print(f(39));',0,'38\n',''),
    'finally_return_replaces_value': (IO+INC+'func factory()->func(i64)->const i64{try{return inc;}finally{return dec;};}behave @print(factory()(40));',0,'39\n',''),
    'recursive_factory': (IO+INC+'func factory(n:i64)->func(i64)->const i64{if(n==0){return inc;}else{return factory(n-1);};}behave @print(factory(40)(40));',0,'41\n',''),
    'recursive_callable_parameter': (IO+INC+'func apply(f:func(i64)->const i64,n:i64)->const i64{return if(n==0){f(41)}else{apply(f,n-1)};}behave @print(apply(inc,40));',0,'42\n',''),
    'recursive_local_callable': (IO+'proc run()->void{func factorial(n:i64)->const i64{return if(n==0){1}else{n*factorial(n-1)};};behave @print(factorial(5));}run();',0,'120\n',''),
    'generic_callable_parameter': (IO+'func identity(x:T)->const T where T{return x;}func apply(f:F,x:T)->const T where {F,T}{return f(x);}func a:func(i64)->const i64=identity;func b:func(bool)->const bool=identity;behave @print(apply(a,43));behave @print(apply(b,true));',0,'43\ntrue\n',''),
    'generic_value_forwarding': (IO+INC+'func identity(x:T)->const T where T{return x;}func f=identity(inc);behave @print(f(43));behave @print(identity(44));',0,'44\n44\n',''),
    'overloaded_family_view': (IO+'func f(x:T)->const i64 where T{return 1;}func f(x:i64)->const i64{return 45;}func a:func(i64)->const i64=f;func b:func(bool)->const i64=f;behave @print(a(0));behave @print(b(false));',0,'45\n1\n',''),
    'const_holder_can_invoke': (IO+INC+'const func f=inc;behave @print(f(45));',0,'46\n',''),
    'const_holder_cannot_replace': (INC+'const func f=inc;f<-dec;',2,'','readonly destination'),
    'holder_cannot_rebind': (INC+'func f=inc;f=dec;',2,'',"use '<-'"),
    'signature_mismatch': (INC+'func f:func(bool)->const i64=inc;',2,'','no applicable method'),
    'argument_type_mismatch': (INC+'func f=inc;f(true);',2,'','callable view type'),
    'argument_count_mismatch': (INC+'func f=inc;f();',2,'','argument count'),
    'wrong_callable_category': (IO+PROC+'func f=one;',2,'','matching func/proc'),
    'function_cannot_call_procedure_value': (IO+PROC+'func wrong()->void{proc f=one;f(1);}',2,'','function cannot invoke a procedure'),
    'lexical_capture_not_promoted': ('proc run()->void{val x=1;func f=func[x]()->const i64{return x;};}',2,'','implementation gap'),
    'callable_not_numeric_bits': (INC+'func f=inc;f+f;',2,'','no applicable numeric operation'),
    'checked_view_declaration': (PRE+'func f(x:i64)->const i64 throws NullPointerException{return x;}func g=f;g(1);',2,'','requires a covering catch'),
    'checked_view_handler': (PRE+'func f(x:i64)->const i64 throws NullPointerException{return x;}func g=f;try{behave @print(g(47));}catch(e:const NullPointerException){};',0,'47\n',''),
    'checked_view_cannot_hide_throw': (PRE+'func f(x:i64)->const i64 throws NullPointerException{return x;}func g:func(i64)->const i64=f;',2,'','omits a required checked exception'),
    'voluntary_checked_view_contract': (PRE+INC+'func f:func(i64)->const i64 throws NullPointerException=inc;f(1);',2,'','requires a covering catch'),
    'deferred_checked_view': (PRE+'proc f(x:i64)->void throws NullPointerException{behave @print(x);}proc g=f;try{{defer g(48);}}catch(e:const NullPointerException){};',0,'48\n',''),
    'failure_retains_previous_holder': (IO+INC+MISS+'func failed()->func(i64)->const i64{missing();return dec;}func f=inc;try{f<-failed();{}}catch(e:const MissingReturnException){behave @print(f(48));};',0,'49\n',WARN),
    'generic_body_constructs_distinct_views': (IO+'func id(x:T)->const T where T{return x;}func adapt(sample:F)->const F where F{return id;}func a:func(i64)->const i64=id;func b:func(bool)->const bool=id;func fa=adapt(a);func fb=adapt(b);behave @print(fa(50));behave @print(fb(true));',0,'50\ntrue\n',''),
    'generic_factory_result_contract': (IO+'func id(x:T)->const T where T{return x;}func factory(x:T)->func(T)->const T where T{return id;}func a=factory(1);func b=factory(false);behave @print(a(51));behave @print(b(true));',0,'51\ntrue\n',''),
    'generic_local_view_annotation': (IO+'func id(x:T)->const T where T{return x;}func apply(x:T)->const T where T{func f:func(T)->const T=id;return f(x);}behave @print(apply(52));behave @print(apply(true));',0,'52\ntrue\n',''),
    'expected_argument_view': (IO+'func id(x:T)->const T where T{return x;}func apply(f:func(i64)->const i64,x:i64)->const i64{return f(x);}behave @print(apply(id,53));',0,'53\n',''),
    'expected_conditional_view': (IO+'func id(x:T)->const T where T{return x;}func other(x:T)->const T where T{return x;}func f:func(i64)->const i64=if(true){id}else{other};behave @print(f(54));',0,'54\n',''),
    'expected_scope_view': (IO+'func id(x:T)->const T where T{return x;}func f:func(i64)->const i64={{id}};behave @print(f(55));',0,'55\n',''),
    'borrowed_callable_parameter': (IO+INC+'proc change(f:func(i64)->const i64 in O)->void where {owner O}{f<-dec;}func f=inc;change(f);behave @print(f(57));',0,'56\n',''),
    'borrowed_callable_result': (IO+INC+'func view(f:func(i64)->const i64 in O)->(func(i64)->const i64) in O where {owner O}{return f;}func f=inc;ptr p=view(f);f<-dec;behave @print(p(58));',0,'57\n',''),
    'borrowed_result_copy_is_independent': (IO+INC+'func view(f:func(i64)->const i64 in O)->(func(i64)->const i64) in O where {owner O}{return f;}func f=inc;func g=view(f);f<-dec;behave @print(g(57));',0,'58\n',''),
    'readonly_borrowed_holder': (INC+'proc change(f:const func(i64)->const i64 in O)->void where {owner O}{f<-dec;}',2,'','readonly destination'),
    'local_definition_is_void': (IO+'try{func f()->const i64{return 1;};}catch(e){};behave @print(59);',0,'59\n',''),
    'loop_callable_storage': (IO+INC+'for(i in 0..3){{func f=if(i==0){inc}else{dec};behave @print(f(i));}};',0,'1\n0\n1\n',''),
    'loop_direct_callable_declaration': (INC+'for(i in 0..3){func f=inc;};',2,'','nested per-iteration'),
    'defer_direct_callable_declaration': (INC+'defer{func f=inc;};',2,'','nested explicit'),
    'nested_function_parameter': ('func apply(f:func(T)->const T,x:T)->const T where T{return f(x);}',0,'',''),
    'view_requires_valid_throw_contract': (INC+'func f:func(i64)->const i64 throws UnknownException=inc;',2,'','callable view throws require intrinsic exceptions'),
    'view_cannot_borrow_method_type_binder': ('func id(x:T)->const T where T{return x;}func f:func(T)->const T=id;',2,'','non-scalar signature type'),
    'nested_result_requires_determined_type': ('func factory(x:U)->func()->const T where {U,T}{}factory(1);',2,'','type parameter is not determined'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    sys.exit(runner.main())
