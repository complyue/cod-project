#!/usr/bin/env python3
"""Declaration coverage preserves independent owners, access and actual input identities."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
SYSTEM = 'using "std:exceptions" SystemException;'
MISSING = 'using "std:exceptions" MissingReturnException;'
BROAD = 'func choose(ref x:i64 in A,ref y:i64 in B)->ref i64 in B where {owner A,owner B};'
METHOD = 'func choose(ref x:i64 in O,ref y:i64 in O) where {owner O}{return y;}'
runner.CASES = {
 'stronger_owner_relation': (IO+BROAD+METHOD+'val x<-1,y<-2;choose(x,y)<-7;behave @print(y);',0,'7\n',''),
 'declared_owner_roles_collapse': (IO+BROAD+BROAD.replace('in B where','in A where')+METHOD+'val x<-1,y<-2;choose(x,y)<-8;behave @print(y);',0,'8\n',''),
 'explicit_result_satisfies_broader_owners': (IO+BROAD+METHOD.replace(' where {owner O}{','->ref i64 in O where {owner O}{')+'val x<-1,y<-2;choose(x,y)<-9;behave @print(y);',0,'9\n',''),
 'declaration_after_narrow_method': (IO+METHOD+BROAD+'val x<-1,y<-2;behave @print(choose(x,y));',0,'2\n',''),
 'method_still_requires_same_owner': (BROAD+METHOD+'val x<-1;{{val y<-2;choose(x,y);}};',2,'','different actual owners'),
 'declaration_cannot_bypass_readonly_input': (BROAD+METHOD+'const val x<-1,y<-2;choose(x,y);',2,'','readonly argument cannot supply'),
 'generic_owner_containment': (IO+'func choose(ref x:T in A,ref y:T in B)->ref T in B where {T,owner A,owner B};func choose(ref x:U in O,ref y:U in O) where {U,owner O}{return y;}val x<-1,y<-2,z<-true;choose(x,y)<-7;choose(z,z)<-false;behave @print(y);behave @print(z);',0,'7\nfalse\n',''),
 'independent_owner_relations_remain_unproved': ('func f(ref x:i64 in O,ref y:i64 in O)->i64 where {owner O};func f(ref x:i64 in A,ref y:i64 in B) where {owner A,owner B}{return x;}',2,'','declaration applicability requires'),
 'return_owner_not_in_method_inputs': ('func f(ref x:i64 in A)->ref i64 in B where {owner A,owner B};func f(ref x:i64 in O) where {owner O}{return x;}',2,'','declared result owner is not established'),
 'declared_type_still_checked': (BROAD.replace('->ref i64','->ref bool')+METHOD,2,'','borrowed return requires'),
 'value_input_const_declaration': (IO+'func f(x:const u64)->u64;func f(x:u64){x<-x+1u64;return x;}val x<-3u64;behave @print(f(x));behave @print(x);',0,'4\n3\n',''),
 'value_input_const_implementation': (IO+'func f(x:u64)->u64;func f(x:const u64){return x;}behave @print(f(3u64));',0,'3\n',''),
 'copied_readonly_input': (IO+'func f(x:const u64)->u64;func f(x:u64){x<-9u64;return x;}const val x<-3u64;behave @print(f(x));behave @print(x);',0,'9\n3\n',''),
 'all_covering_value_access_declarations': ('func f(x:const u64)->u64;func f(x:u64)->bool;func f(x:u64){return true;}',2,'','declarations do not establish one result contract'),
 'explicit_result_must_satisfy_all_access_contracts': ('func f(x:const u64)->u64;func f(x:u64)->bool;func f(x:u64)->bool{return true;}',2,'','does not satisfy callable declaration'),
 'narrow_borrow_access': (IO+'func f(ref x:const i64 in A)->i64 where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}val x<-3;behave @print(f(x));',0,'3\n',''),
 'borrowed_result_keeps_declared_access': (IO+'func f(ref x:const i64 in A)->ref const i64 in A where {owner A};func f(ref x:i64 in O) where {owner O}{defer{x<-7;};return x;}val x<-3;behave @print(f(x));behave @print(x);',0,'7\n7\n',''),
 'declared_readonly_result_cannot_write': ('func f(ref x:const i64 in A)->ref const i64 in A where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}val x<-3;f(x)<-7;',2,'','readonly'),
 'readonly_actual_does_not_gain_permission': ('func f(ref x:const i64 in A)->i64 where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}const val x<-3;f(x);',2,'','readonly argument cannot supply'),
 'partial_borrow_access_unproved': ('func f(ref x:i64 in A)->i64 where {owner A};func f(ref x:const i64 in O) where {owner O}{return x;}',2,'','declaration applicability requires'),
 'passing_modes_stay_distinct': ('func f(ptr x:i64 in A)->i64 where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}',2,'','different parameter passing modes'),
 'combined_owner_access_containment': (IO+'func choose(ref x:const i64 in A,ref y:const i64 in B)->i64 where {owner A,owner B};'+METHOD+'val x<-1,y<-2;behave @print(choose(x,y));',0,'2\n',''),
 'readonly_copy_callable_view': (IO+'func f(x:const u64)->u64;func f(x:u64){return x;}func g:func(u64)->u64=f;behave @print(g(4u64));',0,'4\n',''),
 'borrowed_callable_keeps_method_contract': (IO+'func f(ref x:const i64 in A)->i64 where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}func g=f;val x<-3;behave @print(g(x));',0,'3\n',''),
 'borrowed_callable_cannot_gain_access': ('func f(ref x:const i64 in A)->i64 where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}func g=f;const val x<-3;g(x);',2,'','readonly'),
 'retained_owner_return_after_cleanup': (IO+BROAD+METHOD.replace('return y;','defer{x<-8;};return x;')+'val x<-1,y<-2;behave @print(choose(x,y));',0,'8\n',''),
 'narrow_declared_throw_obligation': (NPE+'func f(ref x:const i64 in A)->i64 throws NullPointerException where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}val x<-1;f(x);',2,'','requires a covering catch or throws'),
 'deferred_narrow_contract': (IO+'proc show(x:const i64 in A,y:const i64 in B)->void where {owner A,owner B};proc show(x:i64 in O,y:i64 in O) where {owner O}{behave @print(y);}val x<-1,y<-2;{{defer show(x,y);y<-7;}};',0,'7\n',''),
 'deferred_declared_throws': (NPE+'func f(ref x:const i64 in A)->i64 throws NullPointerException where {owner A};func f(ref x:i64 in O) where {owner O}{return x;}val x<-1;defer f(x);',2,'','requires a covering catch or throws'),
 'operator_value_access': (IO+'func (+)(x:const i64,y:const i64)->i64;behave @print(2+3);',0,'5\n',''),
 'operator_owner_containment': (IO+'func (+)(ref x:const i64 in A,ref y:const i64 in B)->i64 where {owner A,owner B};func (+)(ref x:i64 in O,ref y:i64 in O) where {owner O}{return intrinsic (+)(x,y);}val x<-2,y<-3;behave @print(x+y);',0,'5\n',''),
 'disjoint_swap_builtin': (IO+'proc (<->)(x:const i64 in A,y:const i64 in B)->void where {owner A,owner B};val x<-1,y<-2;x<->y;behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'disjoint_swap_method': (IO+'func (<->)(x:i64 in A,y:i64 in B)->void where {owner A,owner B};proc (<->)(x:const i64 in O,y:const i64 in P)->void where {owner O,owner P}{behave @print(x);}const val x<-1,y<-2;x<->y;',0,'1\n',''),
 'disjoint_swap_throws_not_required': (IO+NPE+'func (<->)(x:const i64 in A,y:const i64 in B)->void throws NullPointerException where {owner A,owner B};val x<-1,y<-2;{{defer (<->)(x,y);}};behave @print(x);',0,'2\n',''),
 'swap_owner_containment': (IO+'func (<->)(x:i64 in A,y:i64 in B)->void where {owner A,owner B};func (<->)(x:i64 in O,y:i64 in O) where {owner O}{intrinsic (<->)(x,y);}val x<-1,y<-2;x<->y;behave @print(x);',0,'2\n',''),
 'independent_exception_categories': (SYSTEM+'func f(x:T,y:T)->const T where T;func f(x:SystemException,y:SystemException){return x;}',2,'','declaration applicability requires'),
 'explicit_result_cannot_assume_same_category': (SYSTEM+'func f(x:T,y:T)->bool where T;func f(x:SystemException,y:SystemException)->bool{return true;}',2,'','declaration applicability requires'),
 'broad_exception_declaration': (SYSTEM+'func f(x:SystemException,y:SystemException)->bool;func f(x:SystemException,y:SystemException){return true;}',0,'',''),
 'independent_generic_exception_inputs': (SYSTEM+'func f(x:T,y:U)->bool where {T,U};func f(x:SystemException,y:SystemException){return true;}',0,'',''),
 'concrete_exception_same_type': (MISSING+'func f(x:T,y:T)->bool where T;func f(x:MissingReturnException,y:MissingReturnException){return true;}',0,'',''),
 'generic_same_type_relation_preserved': (IO+'func f(x:T,y:T)->const T where T;func f(x:U,y:U) where U{return y;}behave @print(f(1u64,2u64));behave @print(f(false,true));',0,'2\ntrue\n',''),
 'query_retains_declared_owner': (IO+BROAD+METHOD+'val x<-1,y<-2,z<-make local typeof(choose(x,y)){7};behave @print(z);',0,'7\n',''),
 'query_value_access': (IO+'func f(x:const u64)->u64;func f(x:u64){return x;}val x<-make local typeof(f(1u64)){7u64};behave @print(x);',0,'7\n',''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
