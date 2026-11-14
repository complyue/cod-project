#!/usr/bin/env python3
"""Definition-site result contracts preserve callable identity and native context."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
MISSING = 'using "std:exceptions" MissingReturnException;'
runner.CASES = {
 'typed_initializer_widens_return': (IO+'func f:func()->u64=func(){return 1u32;};behave @print(f());',0,'1\n',''),
 'copied_identity_retains_result': (IO+'func f:func()->u64=func(){return 1u32;};func g=f;func tag(x:u64)->u64{return 64u64;}behave @print(tag(g()));',0,'64\n',''),
 'body_must_satisfy_expected': ('func f:func()->u64=func(){return true;};',2,'','return type'),
 'explicit_result_is_not_retargeted': ('func f:func()->u64=func()->u32{return 1u32;};',2,'','callable view contract'),
 'named_method_is_not_retargeted': ('func f(){return 1u32;}func g:func()->u64=f;',2,'','callable view contract'),
 'existing_value_is_not_retargeted': ('func f=func(){return 1u32;};func g:func()->u64=f;',2,'','initializer does not satisfy'),
 'receiving_value_is_not_result_evidence': ('func f(){return 1u32;}val x:u64<-f();func g:func()->u64=f;',2,'','callable view contract'),
 'void_expected_does_not_discard_return': ('func f:func()->void=func(){return 1u64;};',2,'','return type'),
 'missing_return_uses_expected_contract': (IO+MISSING+'func f:func()->u64=func(){};try{f();{}}catch(e:const MissingReturnException){behave @print(7u64);};',0,'7\n','[-Wmissing-return]'),
 'throw_only_has_expected_contract': (MISSING+'func f:func(MissingReturnException)->u64=func(e:MissingReturnException){throw e;};',0,'',''),
 'nested_complete_results': (IO+'func f:func()->func()->u64=func(){return func(){return 2u32;};};func g=f();behave @print(g());',0,'2\n',''),
 'explicit_callable_return_supplies_context': (IO+'func factory()->func()->u64{return func(){return 3u32;};}func f=factory();behave @print(f());',0,'3\n',''),
 'declared_callable_return_supplies_context': (IO+'func factory()->func()->u64;func factory(){return func(){return 4u32;};}func f=factory();behave @print(f());',0,'4\n',''),
 'generic_outer_result_context': (IO+'func factory(x:T)->func()->const T where T{return func(){return 5u32;};}func a=factory(1u64);func b=factory(1f64);behave @print(a());behave @print(b());',0,'5\n5\n',''),
 'generic_type_only_expected_context': (IO+'func factory(x:T)->func()->const T where T{func f:func()->const T=func(){return 6u32;};return f;}func a=factory(1u64);func b=factory(1f64);behave @print(a());behave @print(b());',0,'6\n6\n',''),
 'generic_context_preserves_downstream_dispatch': (IO+'func factory(x:T)->func()->const T where T{return func(){return 7u32;};}func tag(x:u64)->u64{return 64u64;}func tag(x:f64)->u64{return 640u64;}func a=factory(1u64);func b=factory(1f64);behave @print(tag(a()));behave @print(tag(b()));',0,'64\n640\n',''),
 'expected_context_from_existing_destination': (IO+'func f:func()->u64=func()->u64{return 0u64;};f<-func(){return 8u32;};behave @print(f());',0,'8\n',''),
 'generic_destination_contract': (IO+'func initial64()->u64{return 0u64;}func initial_float()->f64{return 0f64;}func replace(f:F)->const F where F{f<-func(){return 9u32;};return f;}func a=replace(initial64);func b=replace(initial_float);behave @print(a());behave @print(b());',0,'9\n9\n',''),
 'expected_argument': (IO+'func apply(f:func()->u64)->u64{return f();}behave @print(apply(func(){return 10u32;}));',0,'10\n',''),
 'expected_argument_from_later_type': (IO+'func apply(f:func()->const T,x:T)->const T where T{return f();}behave @print(apply(func(){return 11u32;},1u64));',0,'11\n',''),
 'contextual_query_has_no_execution': (IO+'func apply(f:func()->u64)->u64{return f();}val x<-make local typeof(apply(func(){return true;})){12u64};behave @print(x);',0,'12\n',''),
 'contextual_query_then_real_body_rejects': ('func apply(f:func()->u64)->u64{return f();}apply(func(){return true;});',2,'','return type'),
 'query_does_not_execute_throw_only_body': (IO+MISSING+'func apply(f:func(MissingReturnException)->u64)->u64{return 1u64;}val x<-make local typeof(apply(func(e:MissingReturnException){throw e;})){13u64};behave @print(x);',0,'13\n',''),
 'borrowed_result_owner_mapping': (IO+'func anchor(ref x:i64 in A)->ref i64 in A where {owner A}{return x;}func f=anchor;f<-func(ref x:i64 in B) where {owner B}{return x;};val x<-1;f(x)<-14;behave @print(x);',0,'14\n',''),
 'readonly_borrowed_result': (IO+'func anchor(ref x:const i64 in A)->ref const i64 in A where {owner A}{return x;}func f=anchor;f<-func(ref x:const i64 in B) where {owner B}{return x;};val x<-15;behave @print(f(x));',0,'15\n',''),
 'readonly_cannot_supply_writable_result': ('func anchor(ref x:const i64 in A)->ref i64 in A where {owner A}{}func f=anchor;f<-func(ref x:const i64 in B) where {owner B}{return x;};',2,'','readonly target cannot supply'),
 'borrowed_result_cannot_escape_owner': ('func anchor(ref x:i64 in A,ref y:i64 in B)->ref i64 in A where {owner A,owner B}{return x;}func f=anchor;f<-func(ref x:i64 in X,ref y:i64 in Y) where {owner X,owner Y}{return y;};',2,'','does not retain its declared external owner'),
 'nullable_result': (IO+'func anchor(ptr x:i64 in A)->ptr i64 in A where {owner A}{return x;}func f=anchor;f<-func(ptr x:i64 in B) where {owner B}{return x;};val x<-1;ptr p=x;p=null;ptr q=f(p);behave @print(isnull(q));',0,'true\n',''),
 'nonnull_expected_checks_body': ('func anchor(ptr x:i64 in A)->ref i64 in A where {owner A}{}func f=anchor;f<-func(ptr x:i64 in B) where {owner B}{return x;};',2,'','requires a proven non-null target'),
 'expected_value_result_snapshots_before_cleanup': (IO+'func anchor(ref x:i64 in A)->i64 where {owner A}{return x;}func f=anchor;f<-func(ref x:i64 in B) where {owner B}{defer{x<-17;};return x;};val x<-16;behave @print(f(x));behave @print(x);',0,'16\n17\n',''),
 'expected_borrowed_result_retains_cleanup_write': (IO+'func anchor(ref x:i64 in A)->ref i64 in A where {owner A}{return x;}func f=anchor;f<-func(ref x:i64 in B) where {owner B}{defer{x<-18;};return x;};val x<-1;behave @print(f(x));',0,'18\n',''),
 'deferred_return_context': (IO+'func factory()->func()->u64{defer{return func(){return 19u32;};};}func f=factory();behave @print(f());',0,'19\n',''),
 'finally_return_context': (IO+'func factory()->func()->u64{try{}finally{return func(){return 20u32;};};}func f=factory();behave @print(f());',0,'20\n',''),
 'conditional_definition_contexts': (IO+'func f:func()->u64=if(true){func(){return 21u32;}}else{func(){return 22u32;}};behave @print(f());',0,'21\n',''),
 'guarded_definition_contexts': (IO+'func f:func()->u64=try{func(){return 23u32;}}catch(e){func(){return 24u32;}};behave @print(f());',0,'23\n',''),
 'ordinary_and_explicit_scopes': (IO+'func f:func()->u64={{{func(){return 25u32;}}}};behave @print(f());',0,'25\n',''),
 'captured_callback': (IO+'func id(x:u64)->u64{return x;}func factory(f:func(u64)->u64)->func()->u64{return func[f](){return f(26u64);};}func f=factory(id);behave @print(f());',0,'26\n',''),
 'expected_throws_remains_caller_obligation': (NPE+'func f:func()->bool throws NullPointerException=func(){return true;};f();',2,'','requires a covering catch or throws'),
 'expected_throws_does_not_waive_body_boundary': (NPE+'func anchor(ptr x:i64 in A)->i64 throws NullPointerException where {owner A}{return x;}func f=anchor;f<-func(ptr x:i64 in B) where {owner B}{return x;};',2,'','requires a covering catch or throws'),
 'explicit_throws_cannot_be_erased': (NPE+'func f:func()->bool=func() throws NullPointerException{return true;};',2,'','required checked exception'),
 'callable_kind_not_changed': ('func f:func()->u64=proc(){return 1u32;};',2,'','callable view contract'),
 'overload_cannot_guess_result': (MISSING+'func apply(f:func(MissingReturnException)->u64)->void{}func apply(f:func(MissingReturnException)->bool)->void{}apply(func(e:MissingReturnException){throw e;});',2,'','unproved callable result'),
 'other_argument_selects_contract': (IO+'func apply(f:func()->u64,x:u64)->u64{return f();}func apply(f:func()->bool,x:bool)->bool{return f();}behave @print(apply(func(){return 27u32;},1u64));',0,'27\n',''),
 'independent_body_result_selects_overload': (IO+'func apply(f:func()->u32)->u64{return 32u64;}func apply(f:func()->u64)->u64{return 64u64;}behave @print(apply(func(){return 1u32;}));',0,'32\n',''),
 'earlier_numeric_overflow_not_widened': ('func f:func()->u64=func(){return 4294967295u32+1u32;};f();',70,'','u32 addition overflow'),
 'generic_literal_result_type_relation': (IO+'func apply(f:func(u64)->u64)->u64{return f(28u64);}behave @print(apply(func(x:T) where T{return x;}));',0,'28\n',''),
 'generic_expected_value_and_view_modes': (IO+'func copy(ref x:i64 in O)->i64 where {owner O}{return x;}func alias(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func replace(f:F)->const F where F{f<-func(ref x:i64 in P) where {owner P}{return x;};return f;}func a=replace(copy);func b=replace(alias);val x<-1;ref y=a(x);ref z=b(x);y<-5;z<-7;behave @print(y);behave @print(x);',0,'5\n7\n',''),
 'generic_expected_result_owner_roles': (IO+'func first(ref x:i64 in O,ref y:i64 in P)->ref i64 in O where {owner O,owner P}{return x;}func replace(f:F)->const F where F{f<-func(ref x:i64 in A,ref y:i64 in B) where {owner A,owner B}{return x;};return f;}func a=replace(first);val x<-1,y<-2;a(x,y)<-8;behave @print(x);',0,'8\n',''),
 'typed_local_in_body_inferred_outer': (IO+'func factory(){func f:func()->u64=func(){return 30u32;};return f;}func g=factory();behave @print(g());',0,'30\n',''),
 'receiver_cannot_retarget_factory': ('func factory(){return func(){return 1u32;};}func f:func()->u64=factory();',2,'','initializer does not satisfy'),
 'expected_common_exception_result': (IO+MISSING+'using "std:exceptions" SystemException;func f:func(MissingReturnException)->const SystemException=func(e:MissingReturnException){return e;};func missing()->u64{}try{missing();{}}catch(e:const MissingReturnException){val copied<-f(e);behave @print(copied.category==e.category);};',0,'true\n','[-Wmissing-return]'),
 'expected_unmapped_owner_is_unproved': ('func anchor(ref x:i64 in A)->ref i64 in B where {owner A,owner B}{}func f=anchor;f<-func(ref x:i64 in C) where {owner C}{return x;};',2,'','expected result owner is not established by a parameter'),
}
for kind in ('func','proc'):
 runner.CASES[kind+'_initializer']=(IO+kind+' f:'+kind+'()->u64='+kind+'(){return 29u32;};behave @print(f());',0,'29\n','')
 runner.CASES[kind+'_void']=(kind+' f:'+kind+'()->void='+kind+'(){};f();',0,'','')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
