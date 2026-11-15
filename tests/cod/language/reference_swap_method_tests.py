#!/usr/bin/env python3
"""Reference methods operate on caller slots and preserve checked binding invariants."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
OUT = lambda x: 'behave @print('+x+');'
REF = 'func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'
PTR = 'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}'
BASE = 'val x<-1,y<-7,z<-9;ptr p=x,q=y;'
runner.CASES = {
 'reference_parameters': (IO+REF+'val x<-1,y<-7;ref p=x,q=y;p<=>q;'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'pointer_parameters': (IO+PTR+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'explicit_and_intrinsic': (IO+'proc (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);'+OUT('4')+'}'+BASE+'p<=>q;(<=>)(p,q);intrinsic (<=>)(p,q);'+OUT('p'),0,'4\n4\n7\n',''),
 'nullable_binding': (IO+PTR+BASE+'q=null;p<=>q;'+OUT('(p is null)')+OUT('q'),0,'true\n1\n',''),
 'reload_after_operands': (IO+REF+BASE+'p<=>{p=z;q};'+OUT('p')+OUT('q'),0,'7\n9\n',''),
 'operand_order': (IO+REF+BASE+'{'+OUT('3')+'p}<=>{'+OUT('4')+'q};'+OUT('p'),0,'3\n4\n7\n',''),
 'self_binding_method_runs': (IO+'proc (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{'+OUT('3')+'intrinsic (<=>)(a,b);}'+BASE+'p<=>p;'+OUT('p'),0,'3\n1\n',''),
 'aliased_slot_write': (IO+NPE+'proc (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{if(!(a is null)){b=null;'+OUT('a')+'}else{};}'+BASE+'try{p<=>p;}catch(e:const NullPointerException){'+OUT('8')+'};'+OUT('(p is null)'),0,'8\ntrue\n',''),
 'cleanup_in_method': (IO+'func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{defer{intrinsic (<=>)(a,b);};}'+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'prepared_builtin_in_method': (IO+'func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{defer intrinsic (<=>)(a,b);}'+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'normal_helper_binding_local': (IO+'func helper(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{helper(a,b);}'+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'1\n7\n',''),
 'generic_shared_method': (IO+'func (<=>)(ref a:T in O,ref b:T in O)->void where {T,owner O}{intrinsic (<=>)(a,b);}'+BASE+'val f<-false,t<-true;ptr s=f,r=t;p<=>q;s<=>r;'+OUT('p')+OUT('s'),0,'7\ntrue\n',''),
 'invariant_ref_not_weakened': (PTR+'val x<-1,y<-2;ref p=x,q=y;p<=>q;',2,'','caller ref preservation'),
 'const_binding_rejected': (REF+'val x<-1,y<-2;const ptr p=x;ptr q=y;p<=>q;',2,'','const reference binding'),
 'val_rejected': (REF+'val x<-1,y<-2;x<=>y;',2,'','val storage has no reference binding'),
 'ref_alias_cannot_be_cleared': ('func (<=>)(ref a:i64 in O,ptr b:i64 in O)->void where {owner O}{b=null;}'+BASE+'p<=>q;',2,'','ref requires a proven non-null target'),
}
runner.CASES.update({
 'default_read_after_null_rebind': (IO+NPE+'proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{'+OUT('99')+'}'+BASE+'try{p<=>{p=null;q};}catch(e:const NullPointerException){'+OUT('8')+'};',0,'8\n',''),
 'default_read_after_restore': (IO+NPE+'proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{'+OUT('a')+OUT('b')+'}'+BASE+'p=null;p<=>{p=z;q};',0,'9\n7\n',''),
 'exception_after_mutation': (IO+NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{a=null;b;}'+BASE+'q=null;try{p<=>q;}catch(e:const NullPointerException){'+OUT('(p is null)')+'};',0,'true\n',''),
 'recursive_slot_method': (IO+'func (<=>)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{if(a<b){intrinsic (<=>)(a,b);a<=>b;}else{};}'+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'selected_alias': (IO+REF+BASE+'ptr r=z;ptr var chosen->if(true){p}else{q};chosen<=>r;'+OUT('p')+OUT('q')+OUT('r'),0,'9\n7\n1\n',''),
 'default_can_clear': (IO+'func (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{a=null;}'+BASE+'p<=>q;'+OUT('(p is null)'),0,'true\n',''),
 'ref_input_read_after_both_operands': (IO+REF+'val x<-make local i64,y<-7;ptr p=x,q=y;p<=>{x<-1;q};'+OUT('p')+OUT('q'),0,'7\n1\n',''),
 'caller_alias_cannot_read_uninitialized': (PTR+'val x<-1,y<-make local i64;ptr p=x,q=y;ptr var alias->p;p<=>q;alias;',2,'','before definite initialization'),
 'caller_alias_null_is_checked': (IO+NPE+PTR+BASE+'ptr var alias->p;q=null;try{p<=>q;alias;{}}catch(e:const NullPointerException){'+OUT('8')+'};',0,'8\n',''),
 'empty_keeps_targets': (IO+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{}'+BASE+'p<=>q;'+OUT('p')+OUT('q'),0,'1\n7\n',''),
})
runner.CASES.update({
 'builtin_fallback': (IO+REF+'val x<-false,y<-true;ptr p=x,q=y;p<=>q;'+OUT('p'),0,'true\n',''),
 'different_destination_types': (IO+'func (<=>)(ptr a:i64 in O,ptr b:bool in P)->void where {owner O,owner P}{a=null;b=null;}'+BASE+'val yes<-true;ptr r=yes;p<=>r;'+OUT('(p is null)')+OUT('(r is null)'),0,'true\ntrue\n',''),
 'readonly_slots': (IO+'func (<=>)(ref a:const i64 in O,ref b:const i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}val x<-1,y<-7;ptr p:const i64=x,q:const i64=y;p<=>q;'+OUT('p'),0,'7\n',''),
 'physical_permission_adaptation_unproved': ('func (<=>)(ptr a:const i64 in O,ptr b:const i64 in O)->void where {owner O}{}'+BASE+'ptr var ro:const->p;ptr var other:const->q;ro<=>other;',2,'','target permission preservation'),
 'uninitialized_pointer_write': (IO+NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{a<-3;b<-4;}val x<-make local i64,y<-make local i64;ptr p=x,q=y;try{p<=>q;'+OUT('p')+OUT('q')+'}catch(e:const NullPointerException){};',0,'3\n4\n',''),
 'reference_method_requires_read': (REF+'val x<-make local i64,y<-2;ptr p=x,q=y;p<=>q;',2,'','before definite initialization'),
 'selected_body_is_authoritative': ('func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{a<-true;}'+BASE+'p<=>q;',2,'','transfer'),
 'method_checked_declaration': (NPE+'proc (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{}'+BASE+'p<=>q;',2,'','requires a covering catch'),
 'builtin_checked_declaration': (NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O};'+BASE+'p<=>q;',2,'','requires a covering catch'),
 'intrinsic_bypasses_declaration': (NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O};'+BASE+'intrinsic (<=>)(p,q);',0,'',''),
 'defer_block_calls_method': (IO+REF+BASE+'{{defer{p<=>q;};q=z;}};'+OUT('p')+OUT('q'),0,'9\n1\n',''),
})
ERRORS = NPE+'using "std:exceptions" MissingReturnException;using "std:exceptions" SystemException;func missing()->const i64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}func absent()->const NullPointerException{ptr p:i64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};}'
runner.CASES['exception_targets'] = (IO+ERRORS+'func (<=>)(ref a:SystemException in O,ref b:SystemException in O)->void where {owner O}{intrinsic (<=>)(a,b);}val x:SystemException<-error(),y:SystemException<-absent();ptr p=x,q=y;p<=>q;'+OUT('p.category==y.category')+OUT('q.category==x.category'),0,'true\ntrue\n','[-Wmissing-return]')
runner.CASES.update({
 'inferred_method_value_unproved': ({'ops.cod':PTR+'export (<=>);','main.cod':'using "ops.cod" (<=>) as swap;func f=swap;'},2,'','unproved callable slot contract'),
 'contextual_method_value_unproved': ({'ops.cod':PTR+'export (<=>);','main.cod':'using "ops.cod" (<=>) as swap;func helper(ptr a:i64 in O,ptr b:i64 in O)->void where {owner O}{}func f=helper;f<-swap;'},2,'','unproved callable slot contract'),
 'imported_named_alias_unproved': ({'ops.cod':PTR+'export (<=>);','main.cod':'using "ops.cod" (<=>) as swap;'+BASE+'swap(p,q);'},2,'','unproved slot preparation'),
 'imported_operation_slots': ({'ops.cod':REF+'export (<=>);','main.cod':IO+'using "ops.cod" (<=>);'+BASE+'p<=>q;'+OUT('p')+OUT('q')},0,'7\n1\n',''),
})
runner.CASES['ordinary_family_as_slot_operation_unproved'] = ({'ops.cod':'func swap(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{intrinsic (<=>)(a,b);}export swap;','main.cod':'using "ops.cod" swap as (<=>);'+BASE+'p<=>q;'},2,'','unproved slot preparation adaptation')
runner.CASES['bits_family_as_slot_operation_unproved'] = ({'ops.cod':'func (<->)(ref a:i64 in O,ref b:i64 in O)->void where {owner O}{intrinsic (<->)(a,b);}export (<->);','main.cod':'using "ops.cod" (<->) as (<=>);'+BASE+'p<=>q;'},2,'','unproved slot preparation adaptation')
for suffix, body, result, output, diagnostic in (
    ('write_then_rebind', 'a<-3;a=b;', 'p;{}', '', 'before definite initialization'),
    ('write_entry_target', 'a<-3;a=b;', OUT('x'), '3\n', ''),
    ('rebind_then_write', 'a=b;a<-3;', OUT('p'), '3\n', ''),
    ('deferred_rebind', 'defer intrinsic (=)(a,b);a<-3;', 'p;{}', '', 'before definite initialization'),
):
    runner.CASES['initialization_'+suffix] = (IO+NPE+'func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{'+body+'}val x<-make local i64,y<-make local i64;ptr p=x,q=y;try{p<=>q;'+result+'}catch(e:const NullPointerException){};',2 if diagnostic else 0,output,diagnostic)
runner.CASES['initialization_exception_exit'] = (NPE+'using "std:exceptions" MissingReturnException;func missing()->const i64{}func (<=>)(ptr a:i64 in O,ptr b:i64 in O)->void throws NullPointerException where {owner O}{a<-3;a=b;missing();}val x<-1,y<-make local i64;ptr p=x,q=y;try{p<=>q;}catch(e:const MissingReturnException){p;{}}catch(e:const NullPointerException){};',2,'','before definite initialization')
runner.CASES['selected_initialization_follows_output'] = (IO+REF+'proc run(flag:bool)->void{val x<-make local i64,y<-make local i64,z<-7;ptr p=if(flag){x}else{y},q=z;p<-1;p<=>q;'+OUT('p')+OUT('q')+'}run(true);run(false);',0,'7\n1\n7\n1\n','')
for order in ('builtin_first','method_first'):
    calls = 'run(false,true);run(false,1);run(true,1);' if order=='builtin_first' else 'run(false,1);run(true,1);run(false,true);'
    runner.CASES['shared_default_read_'+order] = (IO+NPE+'proc (<=>)(a:i64 in O,b:i64 in O)->void where {owner O}{'+OUT('a')+'}proc run(flag:bool,value:T)->void where T{val x<-value,y<-value;ptr p=x,q=y;if(flag){p=null;{}}else{};try{p<=>q;}catch(e:const NullPointerException){'+OUT('8')+'};}'+calls,0,'1\n8\n','')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
