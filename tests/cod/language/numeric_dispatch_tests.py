#!/usr/bin/env python3
"""Exact-before-lossless dispatch preserves prepared inputs and declaration obligations."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERROR = 'using "std:exceptions" MissingReturnException;using "std:exceptions" SystemException;func missing()->const u64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
OWNERS = 'func pick(x:u32,ref a:const i64 in O,ref b:const i64 in O)->u64 where {owner O}{return 1u64;}func pick(x:u64,ref a:const i64 in A,ref b:const i64 in B)->u64 where {owner A,owner B}{return 2u64;}'
TYPES = ('u8','u32','u64','i64','f32','f64')
LITERALS = dict(zip(TYPES,('1u8','1u32','1u64','1','1f32','1f64')))
WIDEN = {'u8':{'u32','u64','i64','f32','f64'},'u32':{'u64','i64','f64'},'f32':{'f64'}}
runner.CASES = {}
for source in TYPES:
    for target in TYPES:
        accepts = source == target or target in WIDEN.get(source,set())
        runner.CASES['argument_'+source+'_'+target] = (
            IO+f'func f(x:{target})->{target}{{return x;}}behave @print(f({LITERALS[source]}));',
            0 if accepts else 2,'1\n' if accepts else '', '' if accepts else 'no applicable method')

for first, second, actual, output in (
    ('u32','u64','1u8','1\n'),('u32','i64','1u8','1\n'),('u32','f64','1u8','1\n'),
    ('f32','f64','1u8','1\n'),('f32','f64','1u32','2\n'),('u64','u32','1u64','1\n'),
    ('i64','f64','1u32',None),('u32','f32','1u8',None),('u64','i64','1u8',None),
):
    a=f'func pick(x:{first})->u64{{return 1u64;}}'
    b=f'func pick(x:{second})->u64{{return 2u64;}}'
    for reverse in (False,True):
        runner.CASES[f'overloads_{first}_{second}_{actual}_{reverse}'] = (
            IO+(b+a if reverse else a+b)+'behave @print(pick('+actual+'));',
            2 if output is None else 0,output or '', 'ambiguous methods' if output is None else '')

runner.CASES.update({
 'mixed_parameter_specificity': (IO+'func pick(x:u32,y:f32)->u64{return 1u64;}func pick(x:u64,y:f64)->u64{return 2u64;}behave @print(pick(1u8,2u8));',0,'1\n',''),
 'crossed_numeric_specificity': ('func pick(x:u32,y:u64)->u64{return 1u64;}func pick(x:u64,y:u32)->u64{return 2u64;}pick(1u8,2u8);',2,'','ambiguous methods'),
 'access_and_numeric_tradeoff': ('func pick(x:const u32)->u64{return 1u64;}func pick(x:u64)->u64{return 2u64;}pick(1u8);',2,'','ambiguous methods'),
 'readonly_numeric_specificity': (IO+'func pick(x:const u32)->u64{return 1u64;}func pick(x:u64)->u64{return 2u64;}const val x<-1u8;behave @print(pick(x));',0,'1\n',''),
 'owner_and_numeric_specificity': (IO+OWNERS+'val a<-1,b<-2;behave @print(pick(1u8,a,b));',0,'1\n',''),
 'different_owners_choose_wider_method': (IO+OWNERS+'val a<-1;{{val b<-2;behave @print(pick(1u8,a,b));}};',0,'2\n',''),
 'unknown_owners_do_not_choose_wider_method': (OWNERS+'func run(ref a:i64 in A,ref b:i64 in B)->u64 where {owner A,owner B}{return pick(1u8,a,b);}val a<-1,b<-2;run(a,b);',2,'','unproved owner equality'),
 'exact_bad_body_not_widened_fallback': ('func f(x:u8)->u64{return false;}func f(x:u64)->u64{return x;}f(1u8);',2,'','return type'),
 'no_literal_value_fitting': ('func f(x:f32)->f32{return x;}f(1u32);',2,'','no applicable method'),
 'bool_not_numeric': ('func f(x:u64)->u64{return x;}f(true);',2,'','no applicable method'),
 'formal_parameter_controls_body': (IO+'func tag(x:u8)->u64{return 8u64;}func tag(x:u64)->u64{return 64u64;}func f(x:u64)->u64{return tag(x);}behave @print(f(1u8));',0,'64\n',''),
 'formal_parameter_controls_inferred_result': (IO+'func f(x:f64){return x;}func tag(x:f64)->u64{return 64u64;}func tag(x:u32)->u64{return 32u64;}behave @print(tag(f(16777217u32)));',0,'64\n',''),
 'copied_parameter_local_mutation': (IO+'func f(x:u64)->u64{x<-9u64;return x;}const val x<-1u8;behave @print(f(x));behave @print(x);',0,'9\n1\n',''),
 'snapshot_before_later_mutation': (IO+'func first(x:f64,y:u32)->f64{return x;}val x<-16777217u32;behave @print(first(x,{x<-1u32;2u32}));behave @print(x);',0,'16777217\n1\n',''),
 'borrowed_parameter_keeps_target': (IO+'func f(x:f64,ref y:f32 in O)->f64 where {owner O}{y<-9f32;return x;}val y<-2f32;behave @print(f(1u32,y));behave @print(y);',0,'1\n9\n',''),
 'borrowed_parameter_does_not_widen': ('func f(ref x:u64 in O,y:f64)->void where {owner O}{}val x<-1u32;f(x,2f32);',2,'','no applicable method'),
 'read_before_later_effect': (IO+NPE+'func first(x:f64,y:u32)->f64{return x;}val x<-1u32;ptr p=x;p=null;try{first(p,{behave @print(99u64);2u32});{}}catch(e:const NullPointerException){behave @print(7u64);};',0,'7\n',''),
 'proc_effects_remain_ordered': (IO+'proc value(x:u64)->u64{behave @print(x);return x;}proc first(x:i64,y:u64)->i64{behave @print(3u64);return x;}behave @print(first(1u32,value(2u8)));',0,'2\n3\n1\n',''),
 'function_cannot_choose_widened_proc': ('proc f(x:u64)->u64{return x;}func g()->u64{return f(1u8);}',2,'','function cannot invoke a procedure'),
 'borrowed_result_keeps_owner': (IO+'func f(x:f64,ref y:i64 in O)->ref i64 in O where {owner O}{return y;}val y<-3;ref z=f(1u32,y);z<-7;behave @print(y);',0,'7\n',''),
 'deferred_snapshot': (IO+'proc show(x:f64)->void{behave @print(x);}val x<-16777217u32;{{defer show(x);x<-1u32;}};',0,'16777217\n',''),
 'deferred_borrow_stays_late': (IO+'proc show(x:f64,ref y:f32 in O)->void where {owner O}{behave @print(x);behave @print(y);}val x<-1u32,y<-2f32;{{defer show(x,y);x<-3u32;y<-4f32;}};',0,'1\n4\n',''),
 'checked_failure_contract': (NPE+'func f(x:f64)->f64 throws NullPointerException{return x;}f(1u32);',2,'','requires a covering catch or throws'),
 'checked_failure_caught': (IO+NPE+'func f(x:f64)->f64 throws NullPointerException{ptr p:f64=null in here;return p;}try{f(1u32);{}}catch(e:const NullPointerException){behave @print(7u64);};',0,'7\n',''),
 'type_query_does_not_read': (IO+'func f(x:f64)->u64{return 7u64;}val x<-make local u32;val y<-make local typeof(f(x)){9u64};behave @print(y);',0,'9\n',''),
 'type_query_does_not_call_proc': (IO+'proc f(x:f64)->u64{behave @print(99u64);return 7u64;}val y<-make local typeof(f(1u32)){9u64};behave @print(y);',0,'9\n',''),
 'query_ambiguity_preserved': ('func f(x:u32)->u64{return 1u64;}func f(x:f32)->u64{return 2u64;}val y<-make local typeof(f(1u8));',2,'','ambiguous methods'),
 'generic_named_forwarding': (IO+'func f(x:f64)->f64{return x;}func apply(x:T) where T{return f(x);}behave @print(apply(16777217u32));behave @print(apply(3f32));',0,'16777217\n3\n',''),
 'same_type_constraint_preserved': ('func f(x:T,y:T)->const T where T{return x;}f(1u8,2u64);',2,'','no applicable method'),
 'same_type_nested_callable_preserved': ('func identity(x:u64)->u64{return x;}func f(g:func(T)->const T,x:T)->const T where T{return g(x);}f(identity,1u8);',2,'','no applicable method'),
 'own_and_concrete_parameters': (IO+'func f(x:T,y:f64)->const T where T{return x;}behave @print(f(3u8,16777217u32));',0,'3\n',''),
 'retained_type_is_fixed': (IO+'func outer(x:T)->const T where T{func inner(y:T,done:bool)->const T{if(done){return y;}else{return inner(1u8,true);};};return inner(x,false);}behave @print(outer(7u64));behave @print(outer(9f64));',0,'1\n1\n',''),
 'recursion': (IO+'func f(x:f64,n:u64)->f64{if(n==0u64){return x;}else{return f(x,n-1u64);};}behave @print(f(16777217u32,30u8));',0,'16777217\n',''),
 'mutual_recursion': (IO+'func f(x:f64,n:u64)->f64{if(n==0u64){return x;}else{return g(x,n-1u64);};}func g(x:f64,n:u64)->f64{return f(x,n);}behave @print(f(16777217u32,30u8));',0,'16777217\n',''),
 'contextual_callback_before_widened_argument': (IO+'func use(f:func(u64)->u64,x:u64)->u64{return f(x);}behave @print(use(func(y:u64){return y;},7u8));',0,'7\n',''),
 'contextual_callback_after_widened_argument': (IO+'func use(x:u64,f:func(u64)->u64)->u64{return f(x);}behave @print(use(7u8,func(y:u64){return y;}));',0,'7\n',''),
 'callable_signature_not_adapted': ('func f(x:u64)->u64{return x;}func g:func(u8)->u64=f;',2,'','no applicable method'),
 'declared_widened_result': (IO+'func f(x:f64)->f64;func f(x:f64){return x;}behave @print(f(16777217u32));',0,'16777217\n',''),
 'wider_declaration_keeps_existing_result': (IO+'func f(x:f64)->f64;func f(x:u32)->u32{return x;}func tag(x:u32)->u64{return 32u64;}func tag(x:f64)->u64{return 64u64;}behave @print(tag(f(1u8)));',0,'32\n',''),
 'wider_declaration_incompatible_result': ('func f(x:f64)->bool;func f(x:u32)->u32{return x;}f(1u8);',2,'','does not satisfy callable declaration'),
 'wider_declaration_checked_failure': (NPE+'func f(x:f64)->f64 throws NullPointerException;func f(x:u32)->u32{return x;}f(1u8);',2,'','requires a covering catch or throws'),
 'declaration_result_retains_original_input': ('func f(x:T)->const T where T;func f(x:u64){return x;}f(1u8);',2,'','does not satisfy callable declaration'),
 'cached_declaration_result_retains_input': ('func f(x:T)->const T where T;func f(x:u64){return x;}f(1u64);f(1u8);',2,'','does not satisfy callable declaration'),
 'query_declaration_result_retains_input': ('func f(x:T)->const T where T;func f(x:u64){return x;}val y<-make local typeof(f(1u8));',2,'','does not satisfy callable declaration'),
 'exception_preparation_keeps_input_promise': (ERROR+'func f(x:T)->const T where T;func f(x:SystemException)->const SystemException{return x;}f(error());',2,'','does not satisfy callable declaration'),
 'exception_query_keeps_input_promise': (ERROR+'func f(x:T)->const T where T;func f(x:SystemException)->const SystemException{return x;}val y<-make local typeof(f(error()));',2,'','does not satisfy callable declaration'),
 'fixed_result_satisfies_original_declaration': (IO+'func f(x:T)->const T where T;func f(x:u64)->u8{return 7u8;}behave @print(f(1u32));behave @print(f(1u8));',0,'7\n7\n',''),
 'partial_declaration_domain_unproved': ('func f(x:u32)->f64;func f(x:f64)->f64{return x;}f(1u32);',2,'','declaration applicability requires'),
 'bodyless_query_widens_input': (IO+'func f(x:f64)->u64;val y<-make local typeof(f(1u32)){7u64};behave @print(y);',0,'7\n',''),
 'bodyless_call_still_needs_body': ('func f(x:f64)->u64;f(1u32);',2,'','no applicable implementation'),
 'abrupt_argument_keeps_signature': (IO+'func f(x:f64,y:u64)->f64{return x;}proc run()->u64{f({return 7u64;1u8},2u8);return 0u64;}behave @print(run());',0,'7\n',''),
})
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
