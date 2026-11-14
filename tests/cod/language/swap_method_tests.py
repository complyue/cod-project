#!/usr/bin/env python3
"""Destination methods share dispatch while preserving address and read timing."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OOM = 'using "std:exceptions" OutOfMemory;'
METHOD = 'proc (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{intrinsic (<->)(x,y);behave @print(9);}'
GENERIC = 'func (<->)(x:T in O,y:T in P)->void where {T,owner O,owner P}{intrinsic (<->)(x,y);}'
ERROR = 'using "std:exceptions" MissingReturnException;func missing()->const i64{}'
runner.CASES = {
 'infix_explicit_and_builtin': (IO+METHOD+'val x<-1,y<-2;x<->y;(<->)(x,y);intrinsic (<->)(x,y);behave @print(x);behave @print(y);',0,'9\n9\n2\n1\n',''),
 'generic_same_t': (IO+GENERIC+'val x<-1,y<-2,a<-false,b<-true;x<->y;a<->b;behave @print(x);behave @print(a);',0,'2\ntrue\n',''),
 'concrete_over_generic': (IO+GENERIC+METHOD+'val x<-1,y<-2,a<-false,b<-true;x<->y;a<->b;behave @print(x);behave @print(a);',0,'9\n2\ntrue\n',''),
 'builtin_fallback': (IO+METHOD+'val x<-false,y<-true;x<->y;behave @print(x);behave @print(y);',0,'true\nfalse\n',''),
 'self_alias_method': (IO+METHOD+'val x<-1;ptr p=x,q=x;p<->q;behave @print(x);',0,'9\n1\n',''),
 'generic_local_method_and_builtin': (IO+METHOD+'proc run(val x:T,val y:T)->void where T{x<->y;behave @print(x);behave @print(y);}run(1,2);run(false,true);',0,'9\n2\n1\ntrue\nfalse\n',''),
 'recursive_method': (IO+'func (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{if(x<y){intrinsic (<->)(x,y);(<->)(x,y);}else{};}val x<-1,y<-2;x<->y;behave @print(x);behave @print(y);',0,'2\n1\n',''),
 'both_operands_before_reads': (IO+METHOD+'val x<-make local i64,y<-2;x<->{x<-3;y};behave @print(x);behave @print(y);',0,'9\n2\n3\n',''),
 'addresses_before_later_rebind': (IO+METHOD+'val x<-1,y<-2,z<-3;ptr p=x,q=y;p<->{p=z;q};behave @print(x);behave @print(y);behave @print(p);',0,'9\n2\n1\n3\n',''),
 'address_before_scope_cleanup': (IO+METHOD+'val x<-1,y<-2,z<-3;ptr p=x,q=y;{{defer{p=z;};p}}<->q;behave @print(x);behave @print(y);behave @print(p);',0,'9\n2\n1\n3\n',''),
 'operand_effects_once_in_order': (IO+METHOD+'val x<-1,y<-2;{behave @print(3);x}<->{behave @print(4);y};',0,'3\n4\n9\n',''),
 'null_read_after_second_operand': (IO+NPE+METHOD+'val x<-1;ptr p=x;p=null;try{p<->{behave @print(4);x};}catch(e:const NullPointerException){behave @print(5);};behave @print(x);',0,'4\n5\n1\n',''),
 'pointer_method_can_handle_null': (IO+NPE+'proc (<->)(ptr x:i64 in O,ptr y:i64 in P)->void where {owner O,owner P}{if(isnull(x)||isnull(y)){behave @print(4);}else{intrinsic (<->)(x,y);};}val x<-1;ptr p=x;p=null;p<->x;behave @print(x);',0,'4\n1\n',''),
 'ref_promises_checked_after_operands': (IO+'func (<->)(ref x:i64 in O,ref y:i64 in P)->void where {owner O,owner P}{intrinsic (<->)(x,y);}val x<-make local i64,y<-2;x<->{x<-3;y};behave @print(y);',0,'3\n',''),
 'deferred_addresses_and_late_payload': (IO+METHOD+'val x<-1,y<-2,z<-3;ptr p=x,q=y;{{defer (<->)(p,q);p=z;x<-4;}};behave @print(x);behave @print(y);behave @print(p);',0,'9\n2\n4\n3\n',''),
 'deferred_pointer_write_initializes': (IO+NPE+'proc (<->)(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{x<-7;y<-8;}val x<-make local i64,y<-make local i64;try{ {{defer (<->)(x,y);}};behave @print(x);behave @print(y);}catch(e:const NullPointerException){};',0,'7\n8\n',''),
 'failure_after_preparation_no_invocation': (IO+ERROR+METHOD+'val x<-1,y<-2;try{x<->{x<-3;missing();y};}catch(e:const MissingReturnException){behave @print(x);};',0,'3\n','[-Wmissing-return]'),
 'early_return_skips_later_operand': (IO+METHOD+'proc run()->void{val x<-1,y<-2;{return;x}<->{behave @print(7);y};}run();',0,'',''),
 'same_owner_more_specific': (IO+'proc (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{behave @print(1);}proc (<->)(x:i64 in O,y:i64 in O)->void where {owner O}{behave @print(2);}val x<-1,y<-2;x<->y;{{val z<-3;x<->z;}};',0,'2\n1\n',''),
 'readonly_destinations_match_exactly': (IO+'proc (<->)(x:const i64 in O,y:const i64 in P)->void where {owner O,owner P}{behave @print(x);behave @print(y);}val x<-1,y<-2;x<->y;ptr p:const i64=x,q:const i64=y;p<->q;behave @print(x);',0,'2\n1\n2\n',''),
 'no_numeric_destination_conversion': (IO+'proc (<->)(x:u64 in O,y:u64 in P)->void where {owner O,owner P}{behave @print(9);}val x<-1u8,y<-2u8;x<->y;behave @print(x);',0,'2\n',''),
 'no_exception_destination_conversion': (IO+ERROR+'using "std:exceptions" SystemException;func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}proc (<->)(x:SystemException in O,y:SystemException in P)->void where {owner O,owner P}{behave @print(9);}val x<-error(),y<-error();x<->y;behave @print(x.category);',0,'5\n','[-Wmissing-return]'),
 'mixed_type_user_destinations': (IO+'func (<->)(x:i64 in O,y:bool in P)->void where {owner O,owner P}{x<-7;y<-true;}val x<-1,y<-false;x<->y;behave @print(x);behave @print(y);',0,'7\ntrue\n',''),
 'callable_holder_method': (IO+'func (<->)(x:func()->const i64 in O,y:func()->const i64 in P)->void where {owner O,owner P}{func saved=x;x<-y;y<-saved;}func one()->const i64{return 1;}func two()->const i64{return 2;}func a=one;func b=two;a<->b;behave @print(a());behave @print(b());',0,'2\n1\n',''),
 'ordinary_helper_rebind_stays_local': (IO+'func (<->)(ptr x:i64 in O,ptr y:i64 in O)->void where {owner O}{intrinsic (<=>)(x,y);}val x<-1,y<-2;ptr p=x,q=y;p<->q;behave @print(p);behave @print(q);',0,'1\n2\n',''),
 'const_signature_cannot_write': ('func (<->)(x:const i64 in O,y:const i64 in P)->void where {owner O,owner P}{x<-1;}const val x<-1,y<-2;x<->y;',2,'','readonly'),
 'fixed_bindings_still_writable_targets': (IO+METHOD+'const ptr x=1,y=2;x<->y;behave @print(x);',0,'9\n2\n',''),
 'selected_alias_destinations': (IO+METHOD+'proc run(flag:bool)->void{val x<-1,y<-2,z<-3;var a->if(flag){x}else{y};a<->z;behave @print(x);behave @print(y);behave @print(z);}run(true);run(false);',0,'9\n3\n2\n1\n9\n1\n3\n2\n',''),
 'function_cannot_select_procedure': (IO+METHOD+'func run()->void{val x<-1,y<-2;x<->y;}',2,'','function cannot invoke a procedure'),
 'selected_bad_body_no_fallback': ('func (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{x<-true;}val x<-1,y<-2;x<->y;',2,'','transfer'),
 'ambiguous_methods': ('func (<->)(x:i64 in O,y:T in P)->void where {T,owner O,owner P}{}func (<->)(x:T in O,y:i64 in P)->void where {T,owner O,owner P}{}val x<-1,y<-2;x<->y;',2,'','ambiguous methods'),
 'same_signature_kind_not_tiebreaker': ('func (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{}proc (<->)(x:i64 in O,y:i64 in P)->void where {owner O,owner P}{}',2,'','duplicate method signature'),
 'external_owners_required': ('func (<->)(x:i64,y:i64)->void{}',2,'','explicit external owners'),
 'void_result_required': ('func (<->)(x:i64 in O,y:i64 in P)->const i64 where {owner O,owner P}{return x;}',2,'','swap method must return void'),
 'exact_arity_required': ('func (<->)(x:i64 in O)->void where {owner O}{}',2,'','operator method arity'),
 'fresh_operand_is_not_destination': (IO+METHOD+'val x<-1;x<->2;',2,'','swap requires two storage destinations'),
 'uninitialized_input_read': (IO+METHOD+'val x<-make local i64,y<-2;x<->y;',2,'','before definite initialization'),
 'ref_rejects_nullable_input': (NPE+'func (<->)(ref x:i64 in O,ref y:i64 in P)->void where {owner O,owner P}{}val x<-1;ptr p=x;p=null;p<->x;',2,'','ref argument requires a proven non-null'),
 'readonly_not_weakened_for_selection': (IO+METHOD+'val x<-1,y<-2;ptr p:const i64=x;p<->y;',2,'','readonly destination'),
 'checked_declaration_retained': (NPE+'proc (<->)(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{}val x<-1,y<-2;x<->y;',2,'','requires a covering catch'),
 'deferred_default_read_at_registration': (IO+METHOD+'val x<-make local i64,y<-2;defer (<->)(x,y);x<-3;',2,'','before definite initialization'),
 'failed_deferred_setter_gives_no_write': (NPE+OOM+'proc (<->)(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{x<-7;y<-8;}val x<-make local i64,y<-make local i64;try{ {{defer (<->)(x,y);}};}catch(e:const OutOfMemory){}catch(e:const NullPointerException){};x;',2,'','before definite initialization'),
 'expired_destination_rejected': (IO+METHOD+'val y<-1;{{val x<-2;x}}<->y;',2,'','lexical owner has ended'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
