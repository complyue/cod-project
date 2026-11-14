#!/usr/bin/env python3
"""Reference exchange and bits exchange preserve distinct storage contracts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
ERROR = '''using "std:exceptions" MissingReturnException;
func missing()->const i64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
'''
WARN = '[-Wmissing-return]'
EXCEPTIONS = NPE+ERROR+'''using "std:exceptions" SystemException;
func absent()->const NullPointerException{ptr p:i64=null;try{p;{}}catch(e:const NullPointerException){return e;};}
'''
runner.CASES = {
    'reference_exchange': (IO+'val x=1,y=2;ptr p=x,q=y;p<=>q;behave @print(p);behave @print(q);behave @print(x);behave @print(y);',0,'2\n1\n1\n2\n',''),
    'bits_exchange': (IO+'val x=1,y=2;ptr p=x,q=y;p<->q;behave @print(p);behave @print(q);behave @print(x);behave @print(y);',0,'2\n1\n2\n1\n',''),
    'intrinsic_spellings': (IO+'val x=1,y=2;ptr p=x,q=y;intrinsic (<=>)(p,q);(<->)(p,q);behave @print(p);behave @print(q);',0,'1\n2\n',''),
    'fixed_binding_bits_exchange': (IO+'const ptr p=1,q=2;p<->q;behave @print(p);behave @print(q);',0,'2\n1\n',''),
    'reference_aliases': (IO+'val x=1,y=2;ptr p=x,q=y;var a->p;var b->q;a<=>b;behave @print(p);behave @print(q);',0,'2\n1\n',''),
    'value_aliases': (IO+'val x=1,y=2;var a->x;var b->y;a<->b;behave @print(x);behave @print(y);',0,'2\n1\n',''),
    'reference_self_alias': (IO+'val x=1;ptr p=x;var a->p;p<=>a;behave @print(p);',0,'1\n',''),
    'bits_self_alias': (IO+'val x=1;ptr p=x,q=x;p<->q;behave @print(x);',0,'1\n',''),
    'reference_reload_after_operands': (IO+'val x=1,y=2,z=3;ptr p=x,q=y;{behave @print(7);p}<=>{behave @print(8);p=z;q};behave @print(p);behave @print(q);',0,'7\n8\n2\n3\n',''),
    'bits_addresses_before_later_rebind': (IO+'val x=1,y=2,z=3;ptr p=x,q=y;p<->{p=z;q};behave @print(x);behave @print(y);behave @print(p);',0,'2\n1\n3\n',''),
    'bits_values_after_operand_effects': (IO+'val x=1,y=2;x<->{x<-3;y};behave @print(x);behave @print(y);',0,'2\n3\n',''),
    'reference_reload_after_cleanup': (IO+'val x=1,y=2,z=3;ptr p=x,q=y;{{defer{p=z;};p}}<=>q;behave @print(p);behave @print(q);',0,'2\n3\n',''),
    'bits_address_survives_cleanup': (IO+'val x=1,y=2,z=3;ptr p=x,q=y;{{defer{p=z;};p}}<->q;behave @print(x);behave @print(y);behave @print(p);',0,'2\n1\n3\n',''),
    'reference_operand_failure': (IO+ERROR+'val x=1,y=2,z=3;ptr p=x,q=y;try{p<=>{p=z;missing();q};}catch(e:const MissingReturnException){};behave @print(p);behave @print(q);',0,'3\n2\n',WARN),
    'bits_operand_failure': (IO+ERROR+'val x=1,y=2;try{x<->{x<-3;missing();y};}catch(e:const MissingReturnException){};behave @print(x);behave @print(y);',0,'3\n2\n',WARN),
    'null_pointer_exchange': (IO+'val x=1;ptr p=x,q:i64=null;p<=>q;behave @print(isnull(p));behave @print(q);p<=>q;behave @print(p);behave @print(isnull(q));',0,'true\n1\n1\ntrue\n',''),
    'null_self_exchange': (IO+'ptr p:i64=null;p<=>p;behave @print(isnull(p));',0,'true\n',''),
    'bits_null_failure_after_operands': (IO+NPE+'val x=1;ptr p:i64=null;try{p<->{behave @print(2);x};}catch(e:const NullPointerException){behave @print(3);};behave @print(x);',0,'2\n3\n1\n',''),
    'right_null_leaves_left_unchanged': (IO+NPE+'val x=1;ptr p:i64=null;try{x<->p;}catch(e:const NullPointerException){};behave @print(x);',0,'1\n',''),
    'uninitialized_pointer_targets_exchange': (IO+'val x=make local i64,y=make local i64;ptr p=x,q=y;p<=>q;p<-4;q<-5;behave @print(x);behave @print(y);',0,'5\n4\n',''),
    'ref_and_initialized_ptr_exchange': (IO+'val x=1,y=2;ref p=x;ptr q=y;p<=>q;behave @print(p);behave @print(q);',0,'2\n1\n',''),
    'readonly_reference_targets': (IO+'val x=1,y=2;ptr p:const i64=x,q:const i64=y;p<=>q;behave @print(p);behave @print(q);',0,'2\n1\n',''),
    'bits_across_owners': (IO+'val x=1;{{val y=2;x<->y;behave @print(y);}};behave @print(x);',0,'1\n2\n',''),
    'generic_bits': (IO+'func swapped(val x:T,val y:T)->const T where T{x<->y;return x;}behave @print(swapped(1,6));behave @print(swapped(false,true));behave @print(swapped(1u8,7u8));',0,'6\ntrue\n7\n',''),
    'generic_references': (IO+'func swapped(ref x:T,ref y:T)->const T where T{x<=>y;return x;}behave @print(swapped(1,8));behave @print(swapped(false,true));',0,'8\ntrue\n',''),
    'callable_body_intrinsics_not_captures': (IO+'func f=func[](ref x:i64,ref y:i64)->const i64{intrinsic (<=>)(x,y);return x;};behave @print(f(1,9));',0,'9\n',''),
    'inferred_callable_swap_body': (IO+'func f=func(val x:i64,val y:i64)->const i64{x<->y;return x;};behave @print(f(1,10));',0,'10\n',''),
    'borrowed_bits_setters': (IO+NPE+'proc swap(ptr x:i64 in O,ptr y:i64 in P)->void throws NullPointerException where {owner O,owner P}{x<->y;}val x=1;{{val y=2;try{swap(x,y);}catch(e:const NullPointerException){};behave @print(y);}};behave @print(x);',0,'1\n2\n',''),
    'callee_slot_exchange_is_local': (IO+'proc swap(ref x:i64 in O,ref y:i64 in O)->void where {owner O}{x<=>y;x<-11;}val x=1,y=2;ptr p=x,q=y;swap(p,q);behave @print(p);behave @print(q);',0,'1\n11\n',''),
    'borrowed_result_after_exchange': (IO+'func select(ref x:T in O,ref y:T in O)->ref T in O where {T,owner O}{x<=>y;return x;}val x=1,y=2;select(x,y)<-12;behave @print(x);behave @print(y);',0,'1\n12\n',''),
    'loop_invariants': (IO+'func run(n:i64)->const i64{val x=1,y=10;ref p=x,q=y;for(i in 0..n){p<=>q;p<-p+1;};return p*100+q;}behave @print(run(0));behave @print(run(1));behave @print(run(2));behave @print(run(5));',0,'110\n1101\n211\n1303\n',''),
    'nested_loop_alias_exchange': (IO+'val x=1,y=2;ref p=x,q=y;var a->p;for(i in 0..3){for(j in 0..3){a<=>q;};};behave @print(p);behave @print(q);',0,'2\n1\n',''),
    'recursive_bits': (IO+'func rotate(val x:T,val y:T,n:i64)->const T where T{if(n==0){return x;}else{x<->y;return rotate(x,y,n-1);};}behave @print(rotate(1,13,31));behave @print(rotate(false,true,30));',0,'13\nfalse\n',''),
    'deferred_exchange': (IO+'val x=1,y=2;ptr p=x,q=y;{{defer{p<=>q;};}};behave @print(p);behave @print(q);{{defer{p<->q;};}};behave @print(x);behave @print(y);',0,'2\n1\n2\n1\n',''),
    'deferred_observation_order': (IO+'{{val x=1,y=2;ptr p=x,q=y;defer{behave @print(p);};defer{p<=>q;};}};',0,'2\n',''),
    'callable_holder_reference_exchange': (IO+'func one()->const i64{return 1;}func two()->const i64{return 2;}func a=one;func b=two;ptr p=a,q=b;p<=>q;behave @print(p());behave @print(q());',0,'2\n1\n',''),
    'exception_bits_and_causes': (IO+ERROR+'val first=error();val second=try{throw first from first;}catch(e:const MissingReturnException){e};first<->second;behave @print(first.cause_count);behave @print(second.cause_count);',0,'1\n0\n',WARN),
    'exception_categories_exchange': (IO+EXCEPTIONS+'val a:SystemException=error(),b:SystemException=absent();a<->b;try{throw a;}catch(e:const NullPointerException){behave @print(1);};try{throw b;}catch(e:const MissingReturnException){behave @print(2);};',0,'1\n2\n',WARN),
    'exception_payload_after_operand_effects': (IO+EXCEPTIONS+'val a:SystemException=error(),b:SystemException=error();a<->{a<-absent();b};try{throw a;}catch(e:const MissingReturnException){behave @print(1);};try{throw b;}catch(e:const NullPointerException){behave @print(2);};',0,'1\n2\n',WARN),
    'exception_swap_keeps_checked_obligation': (EXCEPTIONS+'val a:SystemException=error(),b:SystemException=error();a<->{a<-absent();b};throw b;',2,'','requires a covering catch'),
    'later_operand_initializes_first': (IO+'val x=make local i64,y=2;x<->{x<-14;y};behave @print(x);behave @print(y);',0,'2\n14\n',''),
    'selected_initialization_exchanges': (IO+'proc run(flag:bool)->void{val x=make local i64,y=make local i64,z=15;ptr p=if(flag){x}else{y},q=z;p<-16;p<=>q;behave @print(p);behave @print(q);}run(true);run(false);',0,'15\n16\n15\n16\n',''),
    'reference_swap_invalidates_nonnull': ('val x=1;ptr p=x,q:i64=null;p<=>q;p;',2,'','requires a covering catch'),
    'reference_swap_no_initialization': ('val x=1,y=make local i64;ptr p=x,q=y;p<=>q;p;',2,'','before definite initialization'),
    'ref_rejects_null': ('val x=1;ref p=x;ptr q:i64=null;p<=>q;',2,'','ref requires a proven non-null target'),
    'ref_rejects_uninitialized': ('val x=1,y=make local i64;ref p=x;ptr q=y;p<=>q;',2,'','before definite initialization'),
    'bits_requires_initialized': ('val x=1,y=make local i64;x<->y;',2,'','before definite initialization'),
    'bits_requires_exact_type': ('val x=1u8,y=2u64;x<->y;',2,'','exact same type'),
    'references_require_exact_type': ('ptr p=1,q=true;p<=>q;',2,'','exact target type'),
    'references_require_same_owner': ('ptr p=1;{{ptr q=2;p<=>q;}};',2,'','initializer-fixed owner'),
    'readonly_reference_cannot_enter_writable': ('val x=1,y=2;ptr p=x;ptr q:const i64=y;p<=>q;',2,'','readonly source cannot rebind'),
    'readonly_alias_cannot_launder': ('val x=1,y=2;ptr p=x,q=y;var a:const->p;a<=>q;',2,'','readonly source cannot rebind'),
    'const_reference_binding': ('const ptr p=1;ptr q=2;p<=>q;',2,'','const reference binding'),
    'const_alias_binding': ('ptr p=1,q=2;const var a->p;a<=>q;',2,'','const reference binding'),
    'readonly_bits': ('val x=1,y=2;ptr p:const i64=x;p<->y;',2,'','readonly destination'),
    'const_value_bits': ('const val x=1;val y=2;x<->y;',2,'','readonly destination'),
    'val_has_no_reference_slot': ('val x=1,y=2;x<=>y;',2,'','val storage has no reference binding'),
    'fresh_value_not_destination': ('val x=1;x<->2;',2,'','swap requires two storage destinations'),
    'no_bits_swap_for_callable_values': ('func one()->const i64{return 1;}func a=one;func b=one;a<->b;',2,'','builtin bits swap requires two bits storage'),
    'swap_is_void': ('val x=1,y=2;val z=x<->y;',2,'','non-value or abruptly completing operand'),
    'swap_not_chainable': ('ptr p=1,q=2,r=3;p<=>q<=>r;',2,'','swap operators cannot be chained'),
    'wrong_arity': ('ptr p=1;intrinsic (<=>)(p);',2,'','swap requires two destinations'),
    'expired_bits_target': ('val y=1;{{val x=2;x}}<->y;',2,'','lexical owner has ended'),
    'expired_reference_slot': ('val x=1,y=2;ptr q=y;{{ptr p=x;p}}<=>q;',2,'','reference binding\'s lexical scope has ended'),
    'plain_deferred_builtin_boundary': ('ptr p=1,q=2;defer intrinsic (<=>)(p,q);',2,'','deferred reference-swap intrinsics'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
