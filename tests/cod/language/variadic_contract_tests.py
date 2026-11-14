#!/usr/bin/env python3
"""Variadic signature obligations retain fixed implementation and argument identities."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
SYSTEM = 'using "std:exceptions" SystemException;'
MISSING = 'using "std:exceptions" MissingReturnException;'
runner.CASES = {
 'unused_concrete_contract': ('func f(rest:u64...)->u64;',0,'',''),
 'unused_generic_contract': ('func f(first:T,rest:T...)->const T where T;',0,'',''),
 'empty_concrete_tail': (IO+'func f(rest:u64...)->u64;func f(){return 7u64;}behave @print(f());',0,'7\n',''),
 'empty_generic_tail_from_prefix': (IO+'func f(first:T,rest:T...)->const T where T;func f(x:U) where U{return x;}behave @print(f(7u64));behave @print(f(true));',0,'7\ntrue\n',''),
 'empty_concrete_tail_after_prefix': (IO+'func f(first:T,rest:u64...)->const T where T;func f(x:U) where U{return x;}behave @print(f(true));',0,'true\n',''),
 'empty_uninferred_tail': ('func f(rest:T...)->u64 where T;func f(){return 7u64;}f();',2,'','variadic element type is not determined'),
 'query_empty_uninferred_tail': ('func f(rest:T...)->u64 where T;val x<-make local typeof(f());',2,'','variadic element type is not determined'),
 'fixed_implementations_multiple_arities': (IO+'func sum(rest:u64...)->u64;func sum(){return 0u64;}func sum(x:u64){return x;}func sum(x:u64,y:u64){return x+y;}func sum(x:u64,y:u64,z:u64){return x+y+z;}behave @print(sum());behave @print(sum(1u64));behave @print(sum(1u64,2u64));behave @print(sum(1u64,2u64,3u64));',0,'0\n1\n3\n6\n',''),
 'signature_is_not_an_implementation': ('func f(rest:u64...)->u64;f(1u64,2u64);',2,'','no applicable implementation'),
 'uncovered_arity_stays_missing': ('func f(rest:u64...)->u64;func f(x:u64){return x;}f(1u64,2u64);',2,'','no applicable method'),
 'concrete_tail_widening': (IO+'func f(rest:f64...)->f64;func f(x:f64,y:f64){return x+y;}behave @print(f(16777217u32,1f32));',0,'16777218\n',''),
 'narrow_implementation_satisfies_wide_contract': (IO+'func f(rest:f64...)->f64;func f(x:u32,y:u32)->u32{return x+y;}behave @print(f(1u8,2u8));',0,'3\n',''),
 'mixed_input_identity_not_same_type': (IO+'func f(rest:T...)->bool where T;func f(x:u32,y:f32)->u64{return 7u64;}behave @print(f(1u32,2f32));',0,'7\n',''),
 'generic_fixed_implementation': (IO+'func f(first:T,rest:T...)->const T where T;func f(x:U,y:U) where U{return y;}behave @print(f(1u64,2u64));behave @print(f(false,true));',0,'2\ntrue\n',''),
 'generic_tail_independent_of_prefix': (IO+'func f(first:T,rest:U...)->const U where {T,U};func f(x:A,y:B,z:B) where {A,B}{return z;}behave @print(f(true,2u64,3u64));',0,'3\n',''),
 'original_input_result_promise': ('func f(rest:T...)->const T where T;func f(x:u64,y:u64){return y;}f(1u8,2u8);',2,'','does not satisfy callable declaration'),
 'cached_original_input_result_promise': ('func f(rest:T...)->const T where T;func f(x:u64,y:u64){return y;}f(1u64,2u64);f(1u8,2u8);',2,'','does not satisfy callable declaration'),
 'original_input_compatible_narrow_result': (IO+'func f(rest:T...)->const T where T;func f(x:u64,y:u64)->u8{return 7u8;}behave @print(f(1u32,2u32));',0,'7\n',''),
 'partial_numeric_domain_unproved': ('func f(rest:u32...)->u64;func f(x:u64,y:u64)->u64{return x;}f(1u32,2u32);',2,'','declaration applicability requires'),
 'all_overlapping_contracts_apply': ('func f(rest:u64...)->u64;func f(x:u64,y:u64)->bool;func f(x:u64,y:u64){return true;}',2,'','declarations do not establish one result contract'),
 'explicit_result_checked_against_all': ('func f(rest:u64...)->bool;func f(x:u64,y:u64)->u64{return x;}',2,'','does not satisfy callable declaration'),
 'independent_exception_tail_categories': (SYSTEM+'func f(rest:T...)->bool where T;func f(x:SystemException,y:SystemException)->bool{return true;}',2,'','declaration applicability requires'),
 'common_exception_tail_contract': (SYSTEM+'func f(rest:SystemException...)->bool;func f(x:SystemException,y:SystemException){return true;}',0,'',''),
 'concrete_exception_tail_same_type': (MISSING+'func f(rest:T...)->bool where T;func f(x:MissingReturnException,y:MissingReturnException){return true;}',0,'',''),
 'readonly_value_tail_copies': (IO+'func f(rest:const u64...)->u64;func f(x:u64,y:u64){x<-9u64;return x+y;}const val x<-1u64;behave @print(f(x,x));behave @print(x);',0,'10\n1\n',''),
 'owner_tail_maps_result': (IO+'func f(ref rest:const i64 in A...)->ref const i64 in A where {owner A};func f(ref x:i64 in O,ref y:i64 in O) where {owner O}{return y;}val x<-1,y<-2;behave @print(f(x,y));',0,'2\n',''),
 'owner_tail_checks_every_argument': ('func f(ref rest:const i64 in A...)->i64 where {owner A};func f(ref x:i64 in O,ref y:i64 in O,ref z:i64 in P)->i64 where {owner O,owner P}{return z;}val x<-1,y<-2,z<-3;f(x,y,z);',2,'','declaration applicability requires'),
 'owner_tail_multiple_arguments': (IO+'func f(ref rest:const i64 in A...)->ref const i64 in A where {owner A};func f(ref x:i64 in O,ref y:i64 in O,ref z:i64 in O) where {owner O}{return z;}val x<-1,y<-2,z<-3;behave @print(f(x,y,z));',0,'3\n',''),
 'tail_owner_distinct_from_prefix': (IO+'func f(ref first:const i64 in A,ref rest:const i64 in B...)->ref const i64 in B where {owner A,owner B};func f(ref x:i64 in O,ref y:i64 in P,ref z:i64 in P) where {owner O,owner P}{return z;}val x<-1;{{val y<-2,z<-3;behave @print(f(x,y,z));}};',0,'3\n',''),
 'empty_tail_has_no_owner_obligation': (IO+'func f(ref first:i64 in A,ref rest:i64 in B...)->ref i64 in A where {owner A,owner B};func f(ref x:i64 in O) where {owner O}{return x;}val x<-7;behave @print(f(x));',0,'7\n',''),
 'empty_tail_cannot_supply_result_owner': ('func f(ref rest:i64 in A...)->ref i64 in A where {owner A};func f(){val x<-7;return x;}f();',2,'','declared result owner is not established'),
 'tail_readonly_cannot_gain_write': ('func f(ref rest:const i64 in A...)->ref const i64 in A where {owner A};func f(ref x:i64 in O,ref y:i64 in O) where {owner O}{return y;}val x<-1,y<-2;f(x,y)<-7;',2,'','readonly'),
 'tail_modes_stay_distinct': ('func f(ptr rest:i64 in A...)->i64 where {owner A};func f(ref x:i64 in O,ref y:i64 in O)->i64 where {owner O}{return y;}',2,'','different parameter passing modes'),
 'tail_partial_access_unproved': ('func f(ref rest:i64 in A...)->i64 where {owner A};func f(ref x:const i64 in O,ref y:const i64 in O)->i64 where {owner O}{return y;}',2,'','declaration applicability requires'),
 'query_zero_arguments': (IO+'func f(rest:u64...)->u64;val x<-make local typeof(f()){7u64};behave @print(x);',0,'7\n',''),
 'query_multiple_arguments': (IO+'func f(first:T,rest:T...)->const T where T;val x<-make local typeof(f(1u64,2u64,3u64)){7u64};behave @print(x);',0,'7\n',''),
 'query_does_not_read': (IO+'func f(rest:u64...)->u64;val x<-make local u64;val y<-make local typeof(f(x,x)){7u64};behave @print(y);',0,'7\n',''),
 'query_keeps_prefix_minimum': ('func f(first:u64,rest:u64...)->u64;val x<-make local typeof(f());',2,'','no applicable declared signature'),
 'query_same_type_mismatch': ('func f(first:T,rest:T...)->const T where T;val x<-make local typeof(f(1u8,2u64));',2,'','no applicable declared signature'),
 'query_lossless_widening': (IO+'func f(rest:f64...)->u64;val x<-make local typeof(f(1u8,16777217u32,2f32)){7u64};behave @print(x);',0,'7\n',''),
 'query_rejects_lossy_tail': ('func f(rest:f32...)->u64;val x<-make local typeof(f(1u8,2u32));',2,'','no applicable declared signature'),
 'query_rejects_borrowed_temporary': ('func f(ref rest:i64 in O...)->u64 where {owner O};val x<-make local typeof(f(1));',2,'','borrowed parameter requires an existing view'),
 'query_checks_last_owner': ('func f(ref rest:i64 in O...)->u64 where {owner O};val x<-1,y<-2;{{val z<-3;val q<-make local typeof(f(x,y,z));}};',2,'','no applicable declared signature'),
 'query_empty_tail_owner': (IO+'func f(ref first:i64 in A,ref rest:i64 in B...)->i64 where {owner A,owner B};val x<-7,y<-make local typeof(f(x)){9};behave @print(y);',0,'9\n',''),
 'query_checks_last_access': ('func f(ref rest:i64 in O...)->u64 where {owner O};val x<-1,y<-2;const val z<-3;val q<-make local typeof(f(x,y,z));',2,'','no applicable declared signature'),
 'query_conflicting_contracts': ('func f(rest:u64...)->u64;func f(x:u64,y:u64)->bool;val q<-make local typeof(f(1u64,2u64));',2,'','declarations do not establish one inspected result'),
 'required_checked_failure': (NPE+'func f(rest:u64...)->u64 throws NullPointerException;func f(x:u64,y:u64){return x;}f(1u64,2u64);',2,'','requires a covering catch or throws'),
 'checked_failure_in_fixed_callable': (NPE+'func f(rest:u64...)->u64 throws NullPointerException;func f(x:u64,y:u64){return x;}func g=f;g(1u64,2u64);',2,'','requires a covering catch or throws'),
 'query_has_no_runtime_failure': (IO+NPE+'func f(rest:u64...)->u64 throws NullPointerException;val x<-make local typeof(f(1u64,2u64)){7u64};behave @print(x);',0,'7\n',''),
 'kind_promise_retained': ('func f(rest:u64...)->u64;proc f(x:u64,y:u64){return x;}',2,'','implementation kind does not satisfy'),
 'fixed_callable_retains_contract': (IO+'func f(rest:u64...)->u64;func f(x:u64,y:u64){return x+y;}func g=f;behave @print(g(1u8,2u8));',0,'3\n',''),
 'complete_callable_result': (IO+'func id(x:u64)->u64{return x;}func f(rest:u64...)->func(u64)->u64;func f(x:u64,y:u64){return id;}func g=f(1u64,2u64);behave @print(g(7u64));',0,'7\n',''),
 'nested_callable_tail_relation': (IO+'func id(x:u64)->u64{return x;}func f(rest:func(T)->const T...)->const T where T;val q<-make local typeof(f(id,id)){7u64};behave @print(q);',0,'7\n',''),
 'nested_callable_tail_mismatch': ('func id(x:u64)->u64{return x;}func real(x:f64)->f64{return x;}func f(rest:func(T)->const T...)->const T where T;val q<-make local typeof(f(id,real));',2,'','no applicable declared signature'),
 'recursive_declared_result': (IO+'func sum(rest:u64...)->u64;func sum(x:u64){if(x==0u64){return 0u64;}else{return x+sum(x-1u64);};}behave @print(sum(30u64));',0,'465\n',''),
 'operator_builtin_contract': (IO+'func (+)(rest:i64...)->i64;behave @print(2+3);',0,'5\n',''),
 'operator_builtin_bad_contract': ('func (+)(rest:i64...)->bool;2+3;',2,'','does not satisfy callable declaration'),
 'operator_method_contract': (IO+'func (+)(rest:bool...)->bool;func (+)(x:bool,y:bool){return x||y;}behave @print(false+true);',0,'true\n',''),
 'intrinsic_bypasses_contract': (IO+'func (+)(rest:i64...)->bool;behave @print(intrinsic (+)(2,3));',0,'5\n',''),
 'deferred_fixed_call_snapshot': (IO+'proc show(rest:u64...)->void;proc show(x:u64,y:u64){behave @print(x);behave @print(y);}val x<-1u64;{{defer show(x,2u64);x<-7u64;}};',0,'1\n2\n',''),
 'deferred_checked_promise': (NPE+'proc show(rest:u64...)->void throws NullPointerException;proc show(x:u64,y:u64){}defer show(1u64,2u64);',2,'','requires a covering catch or throws'),
 'nonfinal_pack_rejected': ('func f(rest:u64...,last:u64)->u64;',2,'','only the last parameter can be variadic'),
 'executable_pack_boundary': ('func f(rest:u64...)->u64{return 1u64;}',2,'','implementation gap: variadic method bodies'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
