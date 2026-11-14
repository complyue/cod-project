#!/usr/bin/env python3
"""Ordered overload preparation from checked argument prefixes and native contexts."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
PICK = '''func pick(tag:bool,ref x:i64 in O)->const i64 where {owner O}{x<-x+1;return x;}
func pick(tag:i64,x:i64)->const i64{x<-x+10;return x;}
'''
RELAY = '''func relay(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return pick(tag,x);}
'''
SET = NPE + '''func initialize(tag:bool,ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7;}
func initialize(tag:i64,x:i64)->void{x;}
func dispatch(tag:T,ptr x:i64 in O)->void throws NullPointerException where {T,owner O}{initialize(tag,x);}
'''
READ = '''proc marker()->const i64{behave @print(7);return 0;}
proc read(tag:bool,ptr x:i64 in O,last:i64)->void where {owner O}{behave @print(isnull(x));}
proc read(tag:i64,x:i64,last:i64)->void{behave @print(x);}
proc visit(tag:T,ptr x:i64 in O)->void throws NullPointerException where {T,owner O}{read(tag,x,marker());}
'''
DEFER = '''proc show(tag:bool,ref x:i64 in O)->void where {owner O}{behave @print(x);}
proc show(tag:i64,x:i64)->void{behave @print(x);}
proc later(tag:T,ref x:i64 in O)->void where {T,owner O}{defer show(tag,x);x<-9;}
'''
runner.CASES = {
 'direct_borrow_and_copy':(IO+PICK+'val x=1;behave @print(pick(false,x));behave @print(x);behave @print(pick(0,x));behave @print(x);',0,'2\n2\n12\n2\n',''),
 'generic_preparation_modes':(IO+PICK+RELAY+'val x=1;behave @print(relay(false,x));behave @print(relay(0,x));behave @print(x);',0,'2\n12\n2\n',''),
 'reverse_demand_order':(IO+PICK+RELAY+'val x=1;behave @print(relay(0,x));behave @print(relay(false,x));behave @print(x);',0,'11\n2\n2\n',''),
 'borrowed_alias':(IO+PICK+RELAY+'val x=1;var a->x;behave @print(relay(false,a));behave @print(x);',0,'2\n2\n',''),
 'ordered_effectful_prefix':(IO+PICK+'proc tag()->const bool{behave @print(1);return false;}val x=2;behave @print(pick(tag(),{{behave @print(3);x}}));',0,'1\n3\n3\n',''),
 'generic_effectful_operand':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return pick(tag,{{behave @print(5);x}});}val x=1;behave @print(run(false,x));behave @print(run(0,x));',0,'5\n2\n5\n12\n',''),
 'scope_cleanup_respects_preparation_mode':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return pick(tag,{{defer{x<-5;};x}});}val x=1;behave @print(run(false,x));behave @print(x);x<-1;behave @print(run(0,x));behave @print(x);',0,'6\n6\n11\n5\n',''),
 'repeated_binder_prefix':(IO+'func take(tag:T,key:T,ref x:i64 in O)->const i64 where {T,owner O}{x<-7;return x;}func take(tag:bool,key:i64,x:i64)->const i64{return x+10;}val x=1;behave @print(take(false,0,x));behave @print(take(false,true,x));behave @print(x);',0,'11\n7\n7\n',''),
 'generic_recursive_modes':(IO+PICK+'func loop(tag:T,ref x:i64 in O,n:i64)->const i64 where {T,owner O}{if(n==0){return x;}else{pick(tag,x);return loop(tag,x,n-1);};}val x=1;behave @print(loop(false,x,30));behave @print(loop(0,x,40));',0,'31\n31\n',''),
 'deferred_late_borrow_and_saved_copy':(IO+DEFER+'val x=1;later(false,x);x<-2;later(0,x);behave @print(x);',0,'9\n2\n9\n',''),
 'loop_accumulated_preparations':(IO+DEFER+'proc enqueue(tag:T,ref x:i64 in O)->void where {T,owner O}{for(i in 0..3){defer show(tag,x);x<-x+1;};}val x=1;enqueue(false,x);enqueue(0,x);',0,'4\n4\n4\n6\n5\n4\n',''),
 'borrowed_ptr_initialization':(IO+SET+'val x=make local i64;try{dispatch(false,x);behave @print(x);}catch(e:const NullPointerException){behave @print(99);};',0,'7\n',''),
 'copied_input_needs_initialization':(SET+'val x=make local i64;try{dispatch(0,x);}catch(e:const NullPointerException){};',2,'','cannot prove initialization'),
 'null_borrow_before_later_effect':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{visit(false,p);}catch(e:const NullPointerException){behave @print(99);};',0,'7\ntrue\n',''),
 'null_value_read_before_later_effect':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{visit(0,p);}catch(e:const NullPointerException){behave @print(8);};',0,'8\n',''),
 'both_null_modes_in_shared_body':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{visit(false,p);}catch(e:const NullPointerException){behave @print(99);};try{visit(0,p);}catch(e:const NullPointerException){behave @print(8);};',0,'7\ntrue\n8\n',''),
 'borrowed_read_preserves_early_read':(IO+NPE+READ.replace('ptr x:i64 in O,last','x:i64 in O,last')+'val x=1;ptr p=x;p=null;try{visit(false,p);}catch(e:const NullPointerException){behave @print(8);};',0,'8\n',''),
 'ref_mode_rejects_nullable':(NPE+PICK+'proc test(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{return pick(false,x);}',2,'','ref argument requires a proven non-null target'),
 'borrow_rejects_readonly':(PICK+'const val x=1;pick(false,x);',2,'','readonly argument'),
 'copy_retains_independence_from_readonly':(IO+PICK+'const val x=1;behave @print(pick(0,x));behave @print(x);',0,'11\n1\n',''),
 'fresh_value_in_value_context':(IO+PICK+'behave @print(pick(0,{{val x=1;x}}));',0,'11\n',''),
 'scoped_borrow_retains_owner_check':(PICK+'pick(false,{{val x=1;x}});',2,'','owner has ended'),
 'unresolved_current_argument_mode':('func pick(x:i64)->const i64{return x;}func pick(ref x:bool in O)->const bool where {owner O}{return x;}val x=1;pick(x);',2,'','different argument preparation contracts'),
 'unresolved_later_argument_mode':('func pick(ref x:i64 in O,tag:bool)->const i64 where {owner O}{return x;}func pick(x:i64,tag:i64)->const i64{return x;}val x=1;pick(x,false);',2,'','different argument preparation contracts'),
 'no_applicable_prefix':(PICK+'pick(1u64,2);',2,'','no applicable method'),
 'prefix_does_not_select_result_type':('func choose(tag:bool,x:i64)->const i64{return x;}func choose(tag:bool,x:T)->const bool where T{return false;}func f()->const bool{return choose(false,1);}',2,'','return type'),
 'contextual_callable_view':(IO+'func id(x:T)->const T where T{return x;}func apply(tag:bool,ref f:func(i64)->const i64 in O)->const i64 where {owner O}{return f(8);}func apply(tag:i64,f:func(bool)->const bool)->const bool{return f(true);}func f:func(i64)->const i64=id;behave @print(apply(false,f));behave @print(apply(0,id));',0,'8\ntrue\n',''),
 'captured_type_context':(IO+PICK+'func number()->const i64{return 0;}func truth()->const bool{return false;}func factory(f:func()->const T)->func(i64)->const i64 where T{return func[f](x:i64)->const i64{return pick(f(),x);};}behave @print(factory(truth)(1));behave @print(factory(number)(1));',0,'2\n11\n',''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
