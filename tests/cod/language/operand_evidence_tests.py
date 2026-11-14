#!/usr/bin/env python3
"""Signature evidence plans operand preparation without executing source effects."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
PICK = '''func pick(ref x:i64 in O,tag:bool)->const i64 where {owner O}{x<-x+1;return x;}
func pick(x:i64,tag:i64)->const i64{x<-x+10;return x;}
'''
RELAY = '''func relay(ref x:i64 in O,tag:T)->const i64 where {T,owner O}{return pick(x,tag);}
'''
READ = '''proc read(ptr x:i64 in O,tag:bool)->void where {owner O}{behave @print(isnull(x));}
proc read(x:i64,tag:i64)->void{behave @print(x);}
proc flag()->const bool{behave @print(7);return false;}
proc number()->const i64{behave @print(8);return 0;}
'''
OP = '''func (+)(ref x:i64 in O,y:i64)->const i64 where {owner O}{x<-intrinsic (+)(x,y);return x;}
'''
runner.CASES = {
 'later_type_selects_copy_and_borrow':(IO+PICK+'val x=1;behave @print(pick(x,0));behave @print(x);behave @print(pick(x,false));behave @print(x);',0,'11\n1\n2\n2\n',''),
 'generic_later_evidence':(IO+PICK+RELAY+'val x=1;behave @print(relay(x,false));behave @print(relay(x,0));behave @print(x);',0,'2\n12\n2\n',''),
 'effectful_later_signature':(IO+PICK+'proc tag()->const bool{behave @print(8);return false;}val x=1;behave @print(pick({behave @print(7);x},tag()));',0,'7\n8\n2\n',''),
 'scope_read_boundary':(IO+PICK+'val x=1;behave @print(pick({{defer{x<-5;};x}},0));behave @print(x);x<-1;behave @print(pick({{defer{x<-5;};x}},false));behave @print(x);',0,'11\n5\n6\n6\n',''),
 'declaration_changes_later_type':(IO+PICK+'val x=1,tag=0;{{behave @print(pick({val tag=false;x},tag));}};behave @print(x);',0,'2\n2\n','[-Wshadow]'),
 'alias_changes_later_type':(IO+PICK+'val x=1,tag=0,b=false;{{behave @print(pick({var tag->b;x},tag));}};behave @print(x);',0,'2\n2\n','[-Wshadow]'),
 'full_scope_binding_does_not_escape':(IO+PICK+'val x=1,tag=0;behave @print(pick({{val tag=false;x}},tag));behave @print(x);',0,'11\n1\n','[-Wshadow]'),
 'local_annotation_evidence':(IO+PICK+'val x=1;behave @print(pick(x,{{val tag:bool=false;tag}}));',0,'2\n',''),
 'uninitialized_local_read_not_authorized':(PICK+'val x=1;pick(x,{{val tag=make local bool;tag}});',2,'','before definite initialization'),
 'conditional_type_evidence':(IO+PICK+'val x=1;behave @print(pick(if(true){x}else{x},if(false){0}else{1}));behave @print(x);',0,'11\n1\n',''),
 'later_callable_result_evidence':(IO+PICK+'func truth()->const bool{return false;}func f=truth;val x=1;behave @print(pick(x,f()));',0,'2\n',''),
 'recursive_signature_evidence':(IO+PICK+'func tag(n:i64)->const bool{if(n==0){return false;}else{return tag(n-1);};}val x=1;behave @print(pick(x,tag(30)));',0,'2\n',''),
 'later_generic_result_evidence':(IO+PICK+'func identity(x:T)->const T where T{return x;}val x=1;behave @print(pick(x,identity(false)));behave @print(pick(x,identity(0)));',0,'2\n12\n',''),
 'repeated_binder_later_evidence':(IO+'func pick(ref x:T in O,y:T)->const T where {T,owner O}{x<-y;return x;}func pick(x:i64,y:bool)->const i64{return x+10;}val x=1;behave @print(pick(x,false));behave @print(pick(x,7));behave @print(x);',0,'11\n7\n7\n',''),
 'most_specific_establishes_mode':(IO+'func pick(x:T)->const T where T{return x;}func pick(ref x:i64 in O)->const i64 where {owner O}{x<-9;return x;}val x=1;behave @print(pick(x));behave @print(x);behave @print(pick(false));',0,'9\n9\nfalse\n',''),
 'null_borrow_allows_later_effect':(IO+NPE+READ+'val x=1;ptr p=x;p=null;read(p,flag());',0,'7\ntrue\n',''),
 'null_copy_skips_later_effect':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{read(p,number());}catch(e:const NullPointerException){behave @print(9);};',0,'9\n',''),
 'no_runtime_type_probe':(IO+PICK+'proc tag()->const bool{behave @print(8);return false;}val x=1;{{defer{behave @print(9);};pick({intrinsic (/)(1,0);x},tag());}};',70,'9\n','division by zero'),
 'deferred_late_read_and_snapshot':(IO+'proc show(ref x:i64 in O,tag:bool)->void where {owner O}{behave @print(x);}proc show(x:i64,tag:i64)->void{behave @print(x);}{{val x=1;defer show(x,false);defer show(x,0);x<-9;}};',0,'1\n9\n',''),
 'borrow_operator_and_builtin':(IO+OP+'val x=1;behave @print(x+2);behave @print(x);behave @print(1u8+2u64);behave @print(1f64+2f64);',0,'3\n3\n3\n3\n',''),
 'operator_preserves_read_order':(IO+OP+'val x=1;behave @print(x+(x<-3));behave @print(x);',0,'6\n6\n',''),
 'operator_scope_cleanup':(IO+OP+'val x=1;behave @print({{defer{x<-5;};x}}+2);behave @print(x);',0,'7\n7\n',''),
 'builtin_scope_snapshot':(IO+OP+'val x=1u64;behave @print({{defer{x<-5u64;};x}}+2u64);behave @print(x);',0,'3\n5\n',''),
 'intrinsic_ignores_borrow_candidate':(IO+OP+'val x=1;behave @print(intrinsic (+)(x,2));behave @print(x);',0,'3\n1\n',''),
 'generic_operator_copy_or_borrow':(IO+OP+'func sum(ref x:T in O,y:T)->const T where {T,owner O}{return x+y;}val x=1,y=1u64;behave @print(sum(x,2));behave @print(sum(y,2u64));behave @print(x);behave @print(y);',0,'3\n3\n3\n1\n',''),
 'operator_fallback_uses_later_type':(IO+'func (+)(ptr x:u8 in O,y:u8)->const bool where {owner O}{return isnull(x);}val x=1u8;ptr p=x;p=null;behave @print(p+2u8);',0,'true\n',''),
 'operator_fallback_null_skips_later_effect':(IO+NPE+'func (+)(ptr x:u8 in O,y:u8)->const bool where {owner O}{return isnull(x);}proc later()->const u64{behave @print(99);return 2u64;}val x=1u8;ptr p=x;p=null;try{p+later();{}}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'operator_borrow_read_is_early':(IO+NPE+'func (+)(x:i64 in O,y:i64)->const i64 where {owner O}{return x;}proc later()->const i64{behave @print(99);return 2;}val x=1;ptr p=x;p=null;try{p+later();{}}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'deferred_borrowed_operator':(IO+'proc (+)(ref x:i64 in O,y:i64)->void where {owner O}{behave @print(intrinsic (+)(x,y));}{{val x=1;defer (+)(x,2);x<-9;}};',0,'11\n',''),
 'borrowed_unary_operator':(IO+'func (!)(ref x:i64 in O)->const bool where {owner O}{x<-9;return true;}val x=1;behave @print(!x);behave @print(x);behave @print(!false);',0,'true\n9\ntrue\n',''),
 'readonly_operator_uses_builtin':(IO+OP+'const val x=1;behave @print(x+2);',0,'3\n',''),
 'borrowed_operator_checks_fresh_value':(OP+'1+2;',2,'','borrowed arguments require existing target views'),
 'borrowed_operator_checks_owner':(OP+'{{val x=1;x}}+2;',2,'','owner has ended'),
 'borrowed_operator_checks_ref_null':(NPE+OP+'func test(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{return x+2;}',2,'','ref argument requires a proven non-null target'),
 'guarded_operand_type':(IO+PICK+'val x=1;behave @print(pick(x,try{false}catch(e){false}));',0,'2\n',''),
 'unknown_binding_effects_do_not_use_outer_name':(PICK+'val x=1,tag=0;{{pick({try{val tag=false;}catch(e){};x},tag);}};',2,'','different argument preparation contracts'),
 'ambiguous_passing_modes_remain_ambiguous':('func pick(ref x:i64 in O,y:T)->const i64 where {T,owner O}{return x;}func pick(x:T,y:i64)->const i64 where T{return y;}val x=1;pick(x,2);',2,'','ambiguous methods'),
 'declared_type_does_not_hide_bad_initializer':(PICK+'val x=1;pick(x,{{val tag:bool=0;tag}});',2,'','initializer does not satisfy'),
 'signature_does_not_hide_bad_body':(PICK+'func tag()->const bool{return 0;}val x=1;pick(x,tag());',2,'','return type'),
 'function_cannot_invoke_borrowed_procedure':('proc (+)(ref x:i64 in O,y:i64)->const i64 where {owner O}{return x;}func bad(ref x:i64 in O)->const i64 where {owner O}{return x+2;}',2,'','function cannot invoke a procedure'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
