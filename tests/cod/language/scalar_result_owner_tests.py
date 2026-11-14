#!/usr/bin/env python3
"""Scalar result modes, receiving storage and statically established null owners."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
ERROR = 'using "std:exceptions" MissingReturnException;func missing()->const i64{}'
OWNER_PICK = '''proc pick(ptr x:i64 in O,ptr y:i64 in O)->void where {owner O}{behave @print(1);}
proc pick(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{behave @print(2);}'''
runner.CASES = {
 'bare_scalar_snapshot': (IO+'func get(x:i64 in O)->i64 where {owner O}{defer{x<-9;};return x;}val x<-1;val n<-get(x);behave @print(n);behave @print(x);',0,'1\n9\n',''),
 'explicit_val_snapshot': (IO+'func get(x:i64 in O)->val i64 where {owner O}{defer{x<-9;};return x;}val x<-1;ref r=get(x);x<-8;behave @print(r);behave @print(x);',0,'1\n8\n',''),
 'borrowed_owner_shorthand': (IO+'func get(x:i64 in O)->i64 in O where {owner O}{defer{x<-9;};return x;}val x<-1;ref r=get(x);x<-8;behave @print(r);',0,'8\n',''),
 'readonly_owner_shorthand': (IO+'func get(x:const i64 in O)->const i64 in O where {owner O}{return x;}val x<-1;val n<-get(x);x<-9;behave @print(n);',0,'1\n',''),
 'borrowed_copy_after_cleanup': (IO+'func get(x:i64 in O)->i64 in O where {owner O}{defer{x<-9;};return x;}val x<-1;val n<-get(x);x<-8;behave @print(n);',0,'9\n',''),
 'value_result_from_local_parameter': (IO+'func get(x:i64)->i64{return x;}ref r=get(4);r<-5;behave @print(r);',0,'5\n',''),
 'callable_bare_value_type': (IO+'func get(x:i64)->i64{return x;}func f:func(i64)->i64=get;behave @print(f(6));',0,'6\n',''),
 'callable_explicit_val_type': (IO+'func get(x:i64)->val i64{return x;}func f:func(i64)->val i64=get;behave @print(f(7));',0,'7\n',''),
 'value_const_is_not_target_access': (IO+'func get(x:i64)->const i64{return x;}func f:func(i64)->i64=get;ref r=f(8);r<-9;behave @print(r);',0,'9\n',''),
 'existing_storage_ref_alias': (IO+'val x<-1;ref r=x;r<-2;behave @print(x);',0,'2\n',''),
 'bare_scalar_recursive': (IO+'func sum(n:i64)->i64{if(n==0){return 0;}else{return n+sum(n-1);};}behave @print(sum(30));',0,'465\n',''),
 'mixed_value_returns': (IO+'func get(x:i64 in O,flag:bool)->i64 where {owner O}{if(flag){return x;}else{return x+1;};}val x<-2;behave @print(get(x,true));behave @print(get(x,false));',0,'2\n3\n',''),
 'val_equals_rejected': ('val x=1;',2,'','val initialization requires'),
 'ref_transfer_rejected': ('ref r<-1;',2,'','ref initialization requires'),
 'val_result_owner_rejected': ('func f(x:i64 in O)->val i64 in O where {owner O}{return x;}',2,'','val result has no owner'),
 'fresh_result_not_rebindable': ('func f()->i64{return 1;}ref r=2;r=f();',2,'','rebinding from a fresh value'),
 'borrowed_const_cannot_write': ('func get(x:i64 in O)->const i64 in O where {owner O}{return x;}val x<-1;get(x)<-2;',2,'','readonly destination'),
 'null_here_then_target': (IO+'ptr p:i64=null in here;val x<-10;p=x;behave @print(p);',0,'10\n',''),
 'null_named_owner': (IO+'func f(x:i64 in O)->i64 where {owner O}{ptr p:i64=null in O;p=x;return p;}val x<-11;behave @print(f(x));',0,'11\n',''),
 'null_owners_agree': (IO+'func f(x:i64 in O)->bool where {owner O}{ptr p:i64 in O=null in O;return isnull(p);}val x<-1;behave @print(f(x));',0,'true\n',''),
 'null_propagates_outer_owner': (IO+'ptr anchor:i64=null in here;val x<-12;{{ptr p=anchor;p=x;behave @print(p);}};behave @print(isnull(anchor));',0,'12\ntrue\n',''),
 'ordinary_group_preserves_here': (IO+'ptr p:i64={null in here};val x<-13;p=x;behave @print(p);',0,'13\n',''),
 'per_iteration_here': (IO+'for(i in 0..2){{ptr p:i64=null in here;val x<-14;p=x;behave @print(p);}};',0,'14\n14\n',''),
 'return_null_uses_declared_owner': (IO+'func empty(x:i64 in O)->ptr i64 in O where {owner O}{return null;}val x<-1;ptr p=empty(x);p=x;behave @print(p);',0,'1\n',''),
 'ownerless_null_rejected': ('ptr p:i64=null;',2,'','statically provable owner'),
 'qualified_owner_mismatch': ('func f(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p:i64 in O=null in P;}',2,'','does not belong to the declared owner'),
 'here_cannot_change_to_external': ('func f(x:i64 in O)->void where {owner O}{ptr p:i64=null in here;p=x;}',2,'','initializer-fixed owner'),
 'nested_here_cannot_escape': ('ptr p:i64={{null in here}};',2,'','lexical owner has ended'),
 'null_owner_name_shadowed': ('func f(x:i64 in O)->void where {owner O}{{val O<-1;ptr p:i64=null in O;}}',2,'','nearest binding'),
 'null_does_not_initialize_payload': (NPE+'val x<-make local i64;ptr p:i64=null in here;p=x;try{p;{}}catch(e:const NullPointerException){};',2,'','initialization'),
 'different_live_owners_rejected': ('func f(x:i64 in O,y:i64 in P,flag:bool)->void where {owner O,owner P}{ref r=if(flag){x}else{y};}',2,'','joins different owners'),
 'qualified_null_return_mismatch': ('func f(x:i64 in O,y:i64 in P)->ptr i64 in O where {owner O,owner P}{return null in P;}',2,'','declared external owner'),
 'named_null_owner_guides_preparation': (IO+OWNER_PICK+'proc run(x:i64 in O)->void where {owner O}{pick({ptr p:i64=null in O;p},x);}val x<-1;run(x);',0,'1\n',''),
 'here_owner_guides_preparation': (IO+OWNER_PICK+'val x<-1;pick({ptr p:i64=null in here;p},x);',0,'1\n',''),
 'declaration_owner_guides_preparation': (IO+OWNER_PICK+'proc run(x:i64 in O)->void where {owner O}{pick({ptr p:i64 in O=null;p},x);}val x<-1;run(x);',0,'1\n',''),
 'qualified_null_type_query_is_not_a_read': (IO+'func run(x:i64 in O)->bool where {owner O}{val b<-make local typeof(isnull(null in O)){true};return b;}val x<-1;behave @print(run(x));',0,'true\n',''),
 'type_query_checks_null_owner': ('val b<-make local typeof(isnull(null in Missing)){true};',2,'','unknown owner'),
 'bare_value_cleanup_replacement': (IO+'func get()->i64{defer{return 8;};return 1;}behave @print(get());',0,'8\n',''),
 'borrowed_return_does_not_follow_rebind': (IO+'func get(x:i64 in O,y:i64 in O)->i64 in O where {owner O}{ptr p=x;defer{p=y;x<-9;};return p;}val x<-1,y<-2;ref r=get(x,y);behave @print(r);r<-8;behave @print(x);behave @print(y);',0,'9\n8\n2\n',''),
 'borrowed_return_can_be_replaced': (IO+'func get(x:i64 in O,y:i64 in O)->i64 in O where {owner O}{defer{return y;};return x;}val x<-1,y<-2;ref r=get(x,y);r<-3;behave @print(x);behave @print(y);',0,'1\n3\n',''),
 'failed_rhs_retains_destination': (IO+ERROR+'func get(x:i64 in O)->i64 where {owner O}{x<-3;missing();return 9;}val x<-1,y<-2;try{y<-get(x);{}}catch(e:const MissingReturnException){};behave @print(x);behave @print(y);',0,'3\n2\n','[-Wmissing-return]'),
 'copy_then_alias_is_independent': (IO+'val x<-1;val copy<-x;ref r=copy;x<-2;r<-3;behave @print(x);behave @print(copy);',0,'2\n3\n',''),
 'ptr_transfer_shorthand_preserved': (IO+'val x<-1;ptr p<-x;x<-2;behave @print(p);',0,'1\n',''),
 'captured_scalar_callable_result': (IO+'func wrap(f:func(i64)->i64)->func(i64)->i64{return func[f](x:i64)->i64{return f(x)+1;};}func inc(x:i64)->val i64{return x+1;}func f=wrap(inc);behave @print(f(3));',0,'5\n',''),
 'different_const_spellings_share_callable_type': (IO+'func same(x:T,y:T)->bool where T{return true;}func a(x:i64)->i64{return x;}func b(x:i64)->const i64{return x;}behave @print(same(a,b));',0,'true\n',''),
 'nullable_return_retains_owner_after_cleanup': (IO+'func get(x:i64 in O)->ptr i64 in O where {owner O}{ptr p:i64=null in O;defer{p=x;};return p;}val x<-1;ptr p=get(x);behave @print(isnull(p));p=x;behave @print(p);',0,'true\n1\n',''),
 'same_owner_runtime_choice': (IO+'func get(x:i64 in O,y:i64 in O,flag:bool)->i64 in O where {owner O}{return if(flag){x}else{y};}val x<-1,y<-2;ref r=get(x,y,false);r<-3;behave @print(x);behave @print(y);',0,'1\n3\n',''),
 'null_clear_cannot_change_owner': ('func f(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p=x;p=null;p=y;}',2,'','initializer-fixed owner'),
 'qualified_null_assignment_mismatch': ('func f(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{ptr p=x;p=null in P;}',2,'','initializer-fixed owner'),
 'borrowed_local_cannot_escape': ('func get()->i64 in O where {owner O}{val x<-1;return x;}',2,'','declared external owner'),
 'bare_generic_default_remains_unselected': ('func get(x:T)->T where T{return x;}get(1);',2,'','default result mode for non-scalar or generic types'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
