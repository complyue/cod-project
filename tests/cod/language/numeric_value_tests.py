#!/usr/bin/env python3
"""Whole-type lossless value arguments and least common builtin operand types."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
runner.CASES = {}
TYPES = ('u8','u32','u64','i64','f32','f64')
LITERALS = dict(zip(TYPES,('1u8','1u32','1u64','1','1f32','1f64')))
WIDEN = {'u8':{'u32','u64','i64','f32','f64'},'u32':{'u64','i64','f64'},'f32':{'f64'}}
for source in TYPES:
    for target in TYPES:
        accepted = source == target or target in WIDEN.get(source,set())
        text = IO+f'func identity(x:{target})->{target}{{return x;}}func f=identity;behave @print(f({LITERALS[source]}));'
        runner.CASES['argument_'+source+'_'+target] = (text,0 if accepted else 2,'1\n' if accepted else '',
            '' if accepted else 'argument does not satisfy the callable view type')

runner.CASES.update({
 'cross_type_sum': (IO+'behave @print(16777217u32+1f32);behave @print(1f32+16777217u32);',0,'16777218\n16777218\n',''),
 'cross_type_precision': (IO+'behave @print(16777217u32==16777216f32);behave @print(16777217u32>16777216f32);behave @print(16777216f32<16777217u32);',0,'false\ntrue\ntrue\n',''),
 'cross_type_multiply_divide': (IO+'behave @print(3u32*0.5f32);behave @print(3u32/2f32);behave @print(5f32-3u32);',0,'1.5\n1.5\n2\n',''),
 'cross_type_result_identity': (IO+'func tag(x:f64)->u64{return 64u64;}func tag(x:f32)->u64{return 32u64;}behave @print(tag(1u32+2f32));behave @print(tag(1u8+2f32));',0,'64\n32\n',''),
 'cross_type_inferred_result': (IO+'func f(x:u32,y:f32){return x+y;}func tag(x:f64)->u64{return 64u64;}behave @print(tag(f(1u32,2f32)));',0,'64\n',''),
 'cross_type_query': (IO+'val x<-make local typeof(1u32+2f32){7f64};behave @print(x);',0,'7\n',''),
 'cross_type_type_only_query': (IO+'val x<-make local u32,y<-make local f32;val z<-make local typeof(x+y){7f64};behave @print(z);',0,'7\n',''),
 'no_integer_float64_join': ('1u64+1f64;',2,'','no lossless common type'),
 'no_signed_unsigned_join': ('1+1u64;',2,'','no lossless common type'),
 'remainder_stays_integral': ('3u32%2f32;',2,'','remainder requires integer operands'),
 'earlier_integer_overflow': ('(4294967295u32+1u32)+1f32;',70,'','u32 addition overflow'),
 'same_numeric_policy_at_result_boundary': (IO+'func f(flag:bool){if(flag){return 1u32;}else{return 2f32;};}behave @print(f(true)+2f32);',0,'3\n',''),
 'callable_argument_precision': (IO+'func identity(x:f64)->f64{return x;}func f=identity;behave @print(f(16777217u32));',0,'16777217\n',''),
 'negative_zero': (IO+'func identity(x:f64)->f64{return x;}func f=identity;behave @print(1f64/f(-0f32)<0f64);',0,'true\n',''),
 'infinity': (IO+'func identity(x:f64)->f64{return x;}func f=identity;behave @print(f(1f32/0f32)==1f64/0f64);',0,'true\n',''),
 'nan_value': (IO+'func identity(x:f64)->f64{return x;}func f=identity;val x<-f(0f32/0f32);behave @print(x==x);',0,'false\n',''),
 'callee_local_writes': (IO+'func f(x:u64)->u64{x<-9u64;return x;}func g=f;val x<-1u8;behave @print(g(x));behave @print(x);',0,'9\n1\n',''),
 'readonly_source_copy': (IO+'func f(x:u64)->u64{x<-9u64;return x;}func g=f;const val x<-1u8;behave @print(g(x));behave @print(x);',0,'9\n1\n',''),
 'argument_snapshot_before_later_write': (IO+'func first(x:f64,y:u32)->f64{return x;}func f=first;val x<-16777217u32;behave @print(f(x,{x<-1u32;2u32}));behave @print(x);',0,'16777217\n1\n',''),
 'operands_left_to_right': (IO+'proc value(x:u32)->u32{behave @print(x);return x;}func sum(x:f64,y:f64)->f64{return x+y;}func f=sum;behave @print(f(value(1u32),value(2u32)));',0,'1\n2\n3\n',''),
 'selected_target_before_argument': (IO+'func first(x:f64)->f64{return x;}func second(x:f64)->f64{return x+10f64;}func f=first;behave @print(f({f<-second;1u32}));behave @print(f(1u32));',0,'1\n11\n',''),
 'temporary_callable_target': (IO+'func f(x:f64)->f64{return x;}func factory()->func(f64)->f64{return f;}behave @print(factory()(16777217u32));',0,'16777217\n',''),
 'captured_callable_target': (IO+'func base(x:f64)->f64{return x+1f64;}func wrap(f:func(f64)->f64)->func(f64)->f64{return func[f](x:f64)->f64{return f(x);};}func g=wrap(base);behave @print(g(16777217u32));',0,'16777218\n',''),
 'generic_argument_contexts': (IO+'func integer(x:u64)->u64{return x;}func real(x:f64)->f64{return x;}func small(x:f32)->f32{return x;}func apply(f:func(T)->const T,x:U) where {T,U}{return f(x);}behave @print(apply(integer,1u8));behave @print(apply(real,16777217u32));behave @print(apply(small,3u8));',0,'1\n16777217\n3\n',''),
 'generic_recursion': (IO+'func f(x:f64)->f64{return x;}func relay(f:func(T)->const T,x:U,n:u64)->const T where {T,U}{if(n==0u64){return f(x);}else{return relay(f,x,n-1u64);};}behave @print(relay(f,16777217u32,30u64));',0,'16777217\n',''),
 'deferred_argument_snapshot': (IO+'proc show(x:f64)->void{behave @print(x);}proc p=show;val x<-16777217u32;{{defer p(x);x<-1u32;}};',0,'16777217\n',''),
 'deferred_mixed_builtin': (IO+'{{defer (+)(16777217u32,1f32);behave @print(7u64);}};',0,'7\n',''),
 'argument_abrupt_completion': (IO+'func f(x:f64,y:f64)->f64{return x+y;}proc run()->u64{func g=f;g(1u32,{return 7u64;2u32});return 0u64;}behave @print(run());',0,'7\n',''),
 'no_checked_invocation_after_argument_exit': (IO+NPE+'func f(x:f64,y:f64)->f64 throws NullPointerException{return x+y;}func run()->u64{func g=f;g(1u32,{return 7u64;2u32});return 0u64;}behave @print(run());',0,'7\n',''),
 'declared_failure_remains': (NPE+'func f(x:f64)->f64 throws NullPointerException{return x;}func g=f;g(1u32);',2,'','requires a covering catch or throws'),
 'null_read_before_later_operand': (IO+NPE+'func first(x:f64,y:u32)->f64{return x;}func f=first;val x<-1u32;ptr p=x;p=null;try{f(p,{behave @print(99u64);2u32});{}}catch(e:const NullPointerException){behave @print(7u64);};',0,'7\n',''),
 'failed_widened_defer_preparation': (IO+NPE+'proc show(x:f64)->void{behave @print(x);}proc g=show;val x<-1u32;ptr p=x;p=null;{{defer g(3u8);try{defer g(p);}catch(e:const NullPointerException){behave @print(7u64);};}};',0,'7\n3\n',''),
 'callable_query_adaptation': (IO+'func identity(x:f64)->f64{return x;}func f=identity;val x<-make local typeof(f(1u32)){7f64};behave @print(x);',0,'7\n',''),
 'callable_query_does_not_read': (IO+'func identity(x:f64)->f64{return x;}func f=identity;val x<-make local u32;val y<-make local typeof(f(x)){7f64};behave @print(y);',0,'7\n',''),
 'callable_query_rejects_loss': ('func identity(x:f32)->f32{return x;}func f=identity;val x<-make local typeof(f(1u32));',2,'','argument does not satisfy the callable view type'),
 'writable_borrow_not_widened': ('func f(ref x:u64 in O)->u64 where {owner O}{return x;}func g=f;val x<-1u32;g(x);',2,'','argument does not satisfy the callable view type'),
 'readonly_borrow_not_widened': ('func f(ref x:const u64 in O)->u64 where {owner O}{return x;}func g=f;const val x<-1u32;g(x);',2,'','argument does not satisfy the callable view type'),
 'ptr_target_not_widened': ('func f(ptr x:u64 in O)->void where {owner O}{}func g=f;val x<-make local u32;g(x);',2,'','argument does not satisfy the callable view type'),
 'query_borrow_not_widened': ('func f(ref x:u64 in O)->u64 where {owner O}{return x;}func g=f;val x<-1u32;val y<-make local typeof(g(x));',2,'','argument does not satisfy the callable view type'),
 'bool_is_not_numeric': ('func f(x:u64)->u64{return x;}func g=f;g(true);',2,'','argument does not satisfy the callable view type'),
 'callable_signature_not_retargeted': ('func f(x:u32)->u32{return x;}func g:func(u64)->u64=f;',2,'','no applicable method'),
 'builtin_declaration_checks_joined_result': ('func (+)(x:u32,y:f32)->f32;1u32+2f32;',2,'','does not satisfy callable declaration'),
 'builtin_declaration_accepts_joined_result': (IO+'func (+)(x:u32,y:f32)->f64;behave @print(1u32+2f32);',0,'3\n',''),
 'user_operator_still_selected': (IO+'func (+)(x:u32,y:f32)->u64{return 7u64;}behave @print(1u32+2f32);behave @print(intrinsic (+)(1u32,2f32));',0,'7\n3\n',''),
 'widening_operator_not_bypassed': ('func (+)(x:f64,y:f64)->u64{return 7u64;}1u32+2f32;',2,'','implementation gap: numeric widening during method selection'),
 'widening_integer_operator_not_bypassed': ('func (+)(x:u64,y:u64)->u64{return 7u64;}1u8+2u64;',2,'','implementation gap: numeric widening during method selection'),
 'widening_operator_query_not_builtin': ('func (+)(x:f64,y:f64)->u64{return 7u64;}val x<-make local typeof(1u32+2f32);',2,'','implementation gap: numeric widening during method selection'),
 'widening_operator_defer_not_builtin': ('func (+)(x:f64,y:f64)->u64{return 7u64;}{{defer (+)(1u32,2f32);}};',2,'','implementation gap: numeric widening during method selection'),
 'widening_comparison_not_bypassed': ('func (<)(x:f64,y:f64)->bool{return false;}1u32<2f32;',2,'','implementation gap: numeric widening during method selection'),
 'widening_unary_not_bypassed': ('func (-)(x:f64)->u64{return 7u64;}-2f32;',2,'','implementation gap: numeric widening during method selection'),
 'widening_method_has_explicit_boundary': ('func f(x:f64)->u64{return 7u64;}f(1u32);',2,'','implementation gap: numeric widening during method selection'),
 'widening_method_query_has_explicit_boundary': ('func f(x:f64)->u64{return 7u64;}val x<-make local typeof(f(1u32));',2,'','implementation gap: numeric widening during method selection'),
 'generic_with_concrete_widened_parameter': ('func (+)(x:f64,y:T)->u64 where T{return 7u64;}1u32+2f32;',2,'','implementation gap: numeric widening during method selection'),
 'borrowed_operand_does_not_hide_widened_candidate': ('func (+)(x:f64,ref y:f32 in O)->u64 where {owner O}{return 7u64;}val y<-2f32;1u32+y;',2,'','implementation gap: numeric widening during method selection'),
 'exact_operator_before_widening': (IO+'func (+)(x:f64,y:f64)->u64{return 9u64;}func (+)(x:u32,y:f32)->u64{return 7u64;}behave @print(1u32+2f32);',0,'7\n',''),
 'exact_generic_before_widening': (IO+'func (+)(x:u64,y:u64)->u64{return 9u64;}func (+)(x:T,y:T)->const T where T{return x;}behave @print(1u8+2u8);',0,'1\n',''),
 'intrinsic_ignores_widening_candidate': (IO+'func (+)(x:f64,y:f64)->u64{return 7u64;}behave @print(intrinsic (+)(1u32,2f32));val x<-make local typeof(intrinsic (+)(1u32,2f32)){7f64};behave @print(x);',0,'3\n7\n',''),
 'generic_same_type_not_repaired_by_widening': (IO+'func (+)(x:T,y:T)->const T where T{return x;}behave @print(1u32+2f32);',0,'3\n',''),
 'borrowed_target_not_a_widening_candidate': (IO+'func (+)(ref x:f64 in O,y:f32)->u64 where {owner O}{return 7u64;}val x<-1u32;behave @print(x+2f32);',0,'3\n',''),
 'readonly_disqualifies_writable_borrow': (IO+'func (+)(x:f64,ref y:f32 in O)->u64 where {owner O}{return 7u64;}const val y<-2f32;behave @print(1u32+y);',0,'3\n',''),
 'lossy_candidate_does_not_hide_builtin': (IO+'func (+)(x:f32,y:f32)->u64{return 7u64;}behave @print(1u32+2f32);',0,'3\n',''),
})
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
