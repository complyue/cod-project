#!/usr/bin/env python3
"""Borrowed-call read requirements and path-specific completed writes."""
from pathlib import Path
import sys
import foundation_tests as runner

IO='using symbol "std:io" @print;\n'
PRE=IO+'using "std:exceptions" NullPointerException;\n'
SET='proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-41;}\n'
GET='proc get(ptr x:i64 in O)->void throws NullPointerException where {owner O}{behave @print(x);}\n'
MISS='using "std:exceptions" MissingReturnException;func missing()->const i64{}\n'
WARN='callable may finish without an explicit return'
UNPROVED='cannot prove initialization'
runner.CASES={
    'initializes_caller': (PRE+SET+'val x=make local i64;try{set(x);behave @print(x);}catch(e:const NullPointerException){};',0,'41\n',''),
    'read_requires_input': (PRE+GET+'val x=make local i64;try{get(x);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'read_initialized_input': (PRE+GET+'val x=42;try{get(x);}catch(e:const NullPointerException){};',0,'42\n',''),
    'null_test_needs_no_payload': (IO+'func test(ptr x:i64 in O)->const bool where {owner O}{return isnull(x);}val x=make local i64;behave @print(test(x));',0,'false\n',''),
    'write_does_not_cover_earlier_read': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{behave @print(x);x<-1;}val x=make local i64;try{f(x);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'write_covers_later_read': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-43;behave @print(x);}val x=make local i64;try{f(x);}catch(e:const NullPointerException){};',0,'43\n',''),
    'alias_write_before_read': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x<-44;behave @print(y);}val a=make local i64;try{f(a,a);}catch(e:const NullPointerException){};',0,'44\n',''),
    'different_target_still_uninitialized': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x<-44;behave @print(y);}val a=make local i64,b=make local i64;try{f(a,b);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'alias_later_write_cannot_help': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{behave @print(y);x<-44;}val a=make local i64;try{f(a,a);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'independent_owner_aliases': (PRE+'proc f(ptr x:i64 in D,ptr y:i64 in S)->void throws NullPointerException where {owner D,owner S}{x<-45;behave @print(y);}val a=make local i64;try{f(a,a);}catch(e:const NullPointerException){};',0,'45\n',''),
    'forwarded_requirements': (PRE+GET+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{get(x);}val x=make local i64;try{f(x);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'forwarded_writes': (PRE+SET+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{set(x);}val x=make local i64;try{f(x);behave @print(x);}catch(e:const NullPointerException){};',0,'41\n',''),
    'forwarded_alias_order': (PRE+'proc inner(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x<-46;behave @print(y);}proc outer(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{inner(x,y);}val x=make local i64;try{outer(x,x);}catch(e:const NullPointerException){};',0,'46\n',''),
    'later_argument_initializes_before_entry': (PRE+'proc f(ptr x:i64 in O,n:i64)->void throws NullPointerException where {owner O}{behave @print(x);}val x=make local i64;try{f(x,x<-47);}catch(e:const NullPointerException){};',0,'47\n',''),
    'copied_argument_reads_before_later_write': (PRE+'proc f(x:i64,n:i64)->void{}val x=make local i64;f(x,x<-1);',2,'','before definite initialization'),
    'ref_argument_reads_before_later_write': ('proc f(ref x:i64 in O,n:i64)->void where {owner O}{}val x=make local i64;f(x,x<-1);',2,'','before definite initialization'),
    'ordinary_borrow_reads_before_later_write': ('proc f(x:i64 in O,n:i64)->void where {owner O}{}val x=make local i64;f(x,x<-1);',2,'','before definite initialization'),
    'branch_write_guarantee': (PRE+'proc f(ptr x:i64 in O,b:bool)->void throws NullPointerException where {owner O}{if(b){x<-48;}else{x<-49;};}val x=make local i64;try{f(x,false);behave @print(x);}catch(e:const NullPointerException){};',0,'49\n',''),
    'partial_branch_write': (PRE+'proc f(ptr x:i64 in O,b:bool)->void throws NullPointerException where {owner O}{if(b){x<-48;{}}else{};}val x=make local i64;try{f(x,true);behave @print(x);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'failed_write_no_guarantee': (PRE+SET+'val x=make local i64;try{set(x);}catch(e:const NullPointerException){};behave @print(x);',2,'','before definite initialization'),
    'specific_failed_exit_keeps_write': (PRE+MISS+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-50;missing();}val x=make local i64;try{f(x);}catch(e:const MissingReturnException){behave @print(x);}catch(e:const NullPointerException){};',0,'50\n',WARN),
    'partial_failed_exit': (PRE+MISS+'proc f(ptr x:i64 in O,b:bool)->void throws NullPointerException where {owner O}{if(b){x<-50;{}}else{};missing();}val x=make local i64;try{f(x,true);}catch(e:const MissingReturnException){behave @print(x);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'return_write': (PRE+'func f(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{x<-51;return x;}val x=make local i64;try{behave @print(f(x));behave @print(x);}catch(e:const NullPointerException){};',0,'51\n51\n',''),
    'borrowed_ref_return_after_write': (PRE+'func f(ptr x:i64 in O)->ref i64 in O throws NullPointerException where {owner O}{x<-52;return x;}val x=make local i64;try{behave @print(f(x));}catch(e:const NullPointerException){};',0,'52\n',''),
    'uninitialized_ptr_result_forwarding': (IO+'func f(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}val x=make local i64;ptr p=f(x);behave @print(isnull(p));p<-53;behave @print(x);',0,'false\n53\n',''),
    'forwarded_result_not_initialized': (PRE+'func f(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}val x=make local i64;try{behave @print(f(x));}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'recursive_write': (PRE+'proc f(ptr x:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){x<-54;{}}else{f(x,n-1u64);};}val x=make local i64;try{f(x,6u64);behave @print(x);}catch(e:const NullPointerException){};',0,'54\n',''),
    'recursive_missing_write': (PRE+'proc f(ptr x:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){}else{f(x,n-1u64);};}val x=make local i64;try{f(x,6u64);behave @print(x);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'recursive_read_requirement': (PRE+'proc f(ptr x:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){behave @print(x);}else{f(x,n-1u64);};}val x=make local i64;try{f(x,6u64);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'mutual_recursive_write': (PRE+'proc g(ptr x:i64 in O,n:u64)->void throws NullPointerException where {owner O}{f(x,n);}proc f(ptr x:i64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){x<-55;{}}else{g(x,n-1u64);};}val x=make local i64;try{f(x,6u64);behave @print(x);}catch(e:const NullPointerException){};',0,'55\n',''),
    'generic_write': (PRE+'proc f(ptr x:T in O,y:T)->void throws NullPointerException where {T,owner O}{x<-y;}val a=make local i64,b=make local bool;try{f(a,56);f(b,true);behave @print(a);behave @print(b);}catch(e:const NullPointerException){};',0,'56\ntrue\n',''),
    'selected_target_guarantee': (PRE+SET+'proc f(flag:bool)->void throws NullPointerException{val a=make local i64,b=make local i64;ptr p=if(flag){a}else{b};set(p);behave @print(p);}try{f(true);f(false);}catch(e:const NullPointerException){};',0,'41\n41\n',''),
    'selected_target_not_all_candidates': (PRE+SET+'val a=make local i64,b=make local i64;ptr p=if(true){a}else{b};try{set(p);behave @print(a);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'argument_rebind_does_not_redirect_write': (PRE+'proc f(ptr x:i64 in O,b:bool)->void throws NullPointerException where {owner O}{x<-57;}val a=make local i64,b=make local i64;ptr p=a;try{f(p,{p=b;true});behave @print(a);}catch(e:const NullPointerException){};',0,'57\n',''),
    'argument_rebind_invalidates_selection': (PRE+'proc f(ptr x:i64 in O,b:bool)->void throws NullPointerException where {owner O}{x<-57;}val a=make local i64,b=make local i64;ptr p=a;try{f(p,{p=b;true});behave @print(p);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'callee_rebind_guarantees_selected_parameter': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x=y;x<-58;}val a=make local i64,b=make local i64;try{f(a,b);behave @print(b);}catch(e:const NullPointerException){};',0,'58\n',''),
    'callee_rebind_does_not_write_original': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x=y;x<-58;}val a=make local i64,b=make local i64;try{f(a,b);behave @print(a);}catch(e:const NullPointerException){};',2,'','before definite initialization'),
    'deferred_callee_write': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{defer{x<-59;};}val x=make local i64;try{f(x);behave @print(x);}catch(e:const NullPointerException){};',0,'59\n',''),
    'deferred_caller_write': (PRE+SET+'val x=make local i64;try{ {{defer set(x);}};behave @print(x);}catch(e:const NullPointerException){};',0,'41\n',''),
    'delayed_call_read_after_body_write': (PRE+GET+'val x=make local i64;try{{defer get(x);x<-60;{}}}catch(e:const NullPointerException){};',0,'60\n',''),
    'deferred_call_requires_read': (PRE+GET+'val x=make local i64;try{{defer get(x);}}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'deferred_read_after_successful_write': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{defer{behave @print(x);};x<-61;}val x=make local i64;try{f(x);}catch(e:const NullPointerException){};',0,'61\n',''),
    'finally_write': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{try{}finally{x<-62;{}};}val x=make local i64;try{f(x);behave @print(x);}catch(e:const NullPointerException){};',0,'62\n',''),
    'failed_setter_registration': (PRE+SET+GET+'val x=make local i64;try{{defer get(x);defer set(x);}}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'guarded_body_write_before_deferred_read': (PRE+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{if(isnull(x)){}else{defer{behave @print(x);};x<-63;{}};}val x=make local i64;try{f(x);}catch(e:const NullPointerException){};',0,'63\n',''),
    'deferred_alias_order': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{defer{x<-64;behave @print(y);};}val x=make local i64;try{f(x,x);}catch(e:const NullPointerException){};',0,'64\n',''),
    'deferred_alias_earlier_read': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{defer{behave @print(y);x<-64;};}val x=make local i64;try{f(x,x);}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'selected_alias_order': (PRE+'proc f(ptr x:i64 in O,ptr y:i64 in O)->void throws NullPointerException where {owner O}{x<-65;behave @print(y);}val a=make local i64,b=make local i64;ptr p=if(true){a}else{b};try{f(p,p);behave @print(p);}catch(e:const NullPointerException){};',0,'65\n65\n',''),
    'initialized_ptr_result': (PRE+'func f(ptr x:i64 in O)->ptr i64 in O throws NullPointerException where {owner O}{x<-66;return x;}val x=make local i64;try{behave @print(f(x));}catch(e:const NullPointerException){};',0,'66\n',''),
    'uninitialized_view_can_be_forwarded': (PRE+'func f(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}val x=make local i64;ptr p=f(x);behave @print(isnull(p));try{p<-67;behave @print(x);}catch(e:const NullPointerException){};',0,'false\n67\n',''),
    'local_call_initialization': (PRE+SET+'proc f()->const i64 throws NullPointerException{val x=make local i64;set(x);return x;}try{behave @print(f());}catch(e:const NullPointerException){};',0,'41\n',''),
    'missing_return_after_cleanup_write': (PRE+MISS+'func f(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{defer{x<-68;};}val x=make local i64;try{f(x);{}}catch(e:const MissingReturnException){behave @print(x);}catch(e:const NullPointerException){};',0,'68\n',WARN),
    'replacement_return_cannot_erase_read': (PRE+'func f(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{try{x;{}}finally{return 1;};}val x=make local i64;try{f(x);{}}catch(e:const NullPointerException){};',2,'',UNPROVED),
    'finally_guarantee_on_exception': (PRE+MISS+'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{try{missing();{}}finally{x<-69;{}};}val x=make local i64;try{f(x);}catch(e:const MissingReturnException){behave @print(x);}catch(e:const NullPointerException){};',0,'69\n',WARN),
}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':sys.exit(runner.main())
