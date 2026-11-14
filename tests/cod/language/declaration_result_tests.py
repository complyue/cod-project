#!/usr/bin/env python3
"""Independent declaration contracts, executable coverage and return provenance."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
MISSING = 'using "std:exceptions" MissingReturnException;'
runner.CASES = {
 'unused_declaration': ('func absent(x:u64)->u64;',0,'',''),
 'missing_implementation': ('func absent(x:u64)->u64;absent(1u64);',2,'','no applicable implementation'),
 'missing_callable_value': ('func absent(x:u64)->u64;func f=absent;',2,'','no applicable implementation'),
 'recursive_contract': (IO+'func sum(n:u64)->u64;func sum(n:u64){if(n==0u64){return 0u64;}else{};return n+sum(n-1u64);}behave @print(sum(30u64));',0,'465\n',''),
 'unanchored_contract_without_execution': ('func f(n:u64)->u64;func f(n:u64){return n+f(n-1u64);}func value=f;',0,'',''),
 'declaration_after_definition': (IO+'func f(x:u64){return x+1u64;}func f(x:u64)->u64;behave @print(f(1u64));',0,'2\n',''),
 'procedure_contract': (IO+'proc f(x:u64)->u64;proc f(x:u64){behave @print(x);return x+1u64;}behave @print(f(1u64));',0,'1\n2\n',''),
 'void_contract': (IO+'proc f()->void;proc f(){behave @print(1u64);}f();',0,'1\n',''),
 'declaration_widens_at_return': (IO+'func f()->u64;func f(){return 1u32;}func tag(x:u64)->u64{return 64u64;}behave @print(tag(f()));',0,'64\n',''),
 'incompatible_body': ('func f()->u64;func f(){return true;}',2,'','return type'),
 'incompatible_explicit_result': ('func f()->u64;func f()->bool{return true;}',2,'','does not satisfy callable declaration'),
 'explicit_compatible_result_remains_method_signature': (IO+'func f()->u64;func f()->u32{return 1u32;}func tag(x:u32)->u64{return 32u64;}behave @print(tag(f()));',0,'32\n',''),
 'identical_declarations': (IO+'func f(x:u64)->u64;func f(y:u64)->u64;func f(z:u64){return z;}behave @print(f(1u64));',0,'1\n',''),
 'conflicting_declarations': ('func f()->u64;func f()->bool;func f(){return true;}',2,'','declarations do not establish one result contract'),
 'annotation_cannot_waive_declaration': ('func f()->u64;func f()->bool;func f()->u64{return 1u64;}',2,'','does not satisfy callable declaration'),
 'declarations_are_not_methods': (IO+'func f(x:T)->const T where T;func f(x:u64){return x;}behave @print(f(1u64));',0,'1\n',''),
 'no_negative_override': (IO+'func f(x:u64)->u64;func f(x:T)->const T where T{return x;}behave @print(f(2u64));',0,'2\n',''),
 'generic_parameter_renaming': (IO+'func f(x:T)->const T where T;func f(x:U) where U{return x;}behave @print(f(2u64));behave @print(f(true));',0,'2\ntrue\n',''),
 'generic_instances_have_separate_results': (IO+'func f(x:u32)->u64;func f(x:bool)->bool;func f(x:T) where T{return x;}func tag(x:u64)->u64{return 64u64;}behave @print(tag(f(1u32)));behave @print(f(true));',0,'64\ntrue\n',''),
 'borrowed_result_owner_renaming': (IO+'func f(ref x:i64 in O)->ref i64 in O where {owner O};func f(ref x:i64 in P) where {owner P}{return x;}val x<-1;ref y=f(x);y<-4;behave @print(x);',0,'4\n',''),
 'borrowed_result_type_query': (IO+'func f(ref x:i64 in O)->ref i64 in O where {owner O};func f(ref x:i64 in P) where {owner P}{return x;}val x<-1;val y<-make local typeof(f(x)){5};behave @print(y);',0,'5\n',''),
 'borrowed_generic_result': (IO+'func f(ref x:T in O)->ref T in O where {T,owner O};func f(ref x:U in P) where {U,owner P}{return x;}val x<-1,y<-true;f(x)<-4;f(y)<-false;behave @print(x);behave @print(y);',0,'4\nfalse\n',''),
 'borrowed_owner_rejected': ('func f(ref x:i64 in O,ref y:i64 in P)->ref i64 in O where {owner O,owner P};func f(ref x:i64 in A,ref y:i64 in B) where {owner A,owner B}{return y;}',2,'','does not retain its declared external owner'),
 'readonly_result': (IO+'func f(ref x:const i64 in O)->ref const i64 in O where {owner O};func f(ref x:const i64 in P) where {owner P}{return x;}val x<-1;behave @print(f(x));',0,'1\n',''),
 'readonly_cannot_supply_writable_result': ('func f(ref x:const i64 in O)->ref i64 in O where {owner O};func f(ref x:const i64 in P) where {owner P}{return x;}',2,'','readonly target cannot supply'),
 'nullable_result': (IO+'func f(ptr x:i64 in O)->ptr i64 in O where {owner O};func f(ptr x:i64 in P) where {owner P}{return x;}val x<-1;ptr p=x;p=null;ptr q=f(p);behave @print(isnull(q));',0,'true\n',''),
 'declaration_requires_nonnull': ('func f(ptr x:i64 in O)->ref i64 in O where {owner O};func f(ptr x:i64 in P) where {owner P}{return x;}',2,'','requires a proven non-null target'),
 'deferred_return_uses_declared_view': (IO+'func f(ref x:i64 in O)->ref i64 in O where {owner O};func f(ref x:i64 in P) where {owner P}{defer{return x;};}val x<-1;f(x)<-5;behave @print(x);',0,'5\n',''),
 'copied_declared_callable': (IO+'func f(x:u64)->u64;func f(x:u64){return x+1u64;}func g=f;func h=g;behave @print(h(2u64));',0,'3\n',''),
 'checked_throws_at_call': (NPE+'func f()->bool throws NullPointerException;func f(){return true;}f();',2,'','requires a covering catch or throws'),
 'checked_throws_caught': (IO+NPE+'func f()->bool throws NullPointerException;func f(){return true;}try{behave @print(f());}catch(e:const NullPointerException){};',0,'true\n',''),
 'checked_throws_in_callable_view': (NPE+'func f()->bool throws NullPointerException;func f(){return true;}func g:func()->bool=f;',2,'','callable view omits a required checked exception'),
 'checked_throws_inferred_callable': (NPE+'func f()->bool throws NullPointerException;func f(){return true;}func g=f;g();',2,'','requires a covering catch or throws'),
 'implementation_throws_exceed_declaration': (NPE+'func f()->bool;func f()->bool throws NullPointerException{return true;}',2,'','checked exception outside its callable declaration'),
 'kind_mismatch': ('func f()->u64;proc f(){return 1u64;}',2,'','implementation kind'),
 'declared_missing_return': (IO+MISSING+'func f()->u64;func f(){}try{f();{}}catch(e:const MissingReturnException){behave @print(9u64);};',0,'9\n','[-Wmissing-return]'),
 'declaration_does_not_resolve_overload': ('func f(x:u64,y:u64)->u64;func f(x:u64,y:T)->u64 where T{return x;}func f(x:T,y:u64)->u64 where T{return y;}f(1u64,2u64);',2,'','ambiguous methods'),
 'declaration_does_not_select_result': ('func f(x:u64)->u64;func f(x:u64)->u64{return x;}func f(x:u64)->bool{return true;}',2,'','duplicate method signature'),
 'receiving_type_cannot_retarget': ('func f()->u64;func f(){return 1u32;}val x:u32<-f();',2,'','initializer does not satisfy'),
 'partial_owner_applicability_unproved': ('func f(ref x:i64 in O,ref y:i64 in O)->i64 where {owner O};func f(ref x:i64 in A,ref y:i64 in B) where {owner A,owner B}{return x;}',2,'','declaration applicability requires'),
 'narrow_exception_declaration_unproved': (MISSING+'using "std:exceptions" SystemException;func f(x:MissingReturnException)->bool;func f(x:SystemException)->bool{return true;}func g(x:MissingReturnException)->bool{return f(x);}',2,'','declaration applicability requires'),
 'instance_specific_value_and_view_modes': (IO+'func f(ref x:i64 in O)->ref i64 in O where {owner O};func f(ref x:bool in O)->bool where {owner O};func f(ref x:T in P) where {T,owner P}{return x;}val x<-1,y<-true;f(x)<-7;ref copy=f(y);copy<-false;behave @print(x);behave @print(y);behave @print(copy);',0,'7\ntrue\nfalse\n',''),
 'returned_complete_callable': (IO+'func inc(x:u64)->u64{return x+1u64;}func f()->func(u64)->u64;func f(){return inc;}func g=f();behave @print(g(3u64));',0,'4\n',''),
 'output_only_declaration_binder_unproved': ('func f()->const T where T;func f(){return 1u64;}',2,'','declared result type parameter is not determined'),
 'shadowed_declaration': ('func f(x:u64)->u64;func g(f:u64)->u64{return f(1u64);}',2,'','nearest binding'),
 'declaration_only_type_query': (IO+'func absent(x:u64)->u64;val x<-make local typeof(absent(1u64)){7u64};behave @print(x);',0,'7\n',''),
 'generic_declaration_only_type_query': (IO+'func absent(x:T)->const T where T;val x<-make local typeof(absent(true)){true};behave @print(x);',0,'true\n',''),
 'declaration_query_has_no_payload_read': (IO+'func absent(ref x:i64 in O)->ref i64 in O where {owner O};val x<-make local i64;ptr p=x;p=null;val y<-make local typeof(absent(p)){7};behave @print(y);',0,'7\n',''),
 'declaration_query_incompatible_inputs': ('func absent(x:bool)->u64;val x<-make local typeof(absent(1u64)){7u64};',2,'','no applicable declared signature'),
 'declaration_query_conflict': ('func absent(x:u64)->u64;func absent(x:u64)->bool;val x<-make local typeof(absent(1u64)){7u64};',2,'','declarations do not establish one inspected result contract'),
 'instance_specific_complete_value_and_callable_view': ('using symbol "std:io" @print;func inc(x:u64)->u64{return x+1u64;}func yes(x:bool)->bool{return x;}func f(ref x:func(u64)->u64 in O)->func(u64)->u64 where {owner O};func f(ref x:func(bool)->bool in O)->ref func(bool)->bool in O where {owner O};func f(ref x:T in P) where {T,owner P}{return x;}func a=inc;func b=yes;func c=f(a);func d=f(b);behave @print(c(2u64));behave @print(d(true));',0,'3\ntrue\n',''),
 'body_keeps_own_checked_boundary': (NPE+'func f(ptr x:i64 in O)->i64 throws NullPointerException where {owner O};func f(ptr x:i64 in P) where {owner P}{return x;}',2,'','requires a covering catch or throws'),
 'declaration_query_mode_conflict': ('func f(ptr x:i64 in O)->ref i64 in O where {owner O};func f(ptr x:i64 in O)->ptr i64 in O where {owner O};val x<-1;val y<-make local typeof(f(x)){7};',2,'','declarations do not establish one inspected result contract'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
