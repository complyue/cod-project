#!/usr/bin/env python3
"""Generic value consumers preserve each selected method's declared result contract."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
PICK = '''func pick(tag:bool,ref x:i64 in O)->ref i64 in O where {owner O}{defer{x<-x+1;};return x;}
func pick(tag:i64,ref x:i64 in O)->const i64 where {owner O}{defer{x<-x+1;};return x;}
'''
READ = '''func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return pick(tag,x);}
'''
NULL = '''func pick(tag:bool,ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}
func pick(tag:i64,ptr x:i64 in O)->const i64 where {owner O}{return 7;}
'''
CALLABLES = '''func base(x:u64)->const u64{return x;}
func other(x:u64)->const u64{return x+10u64;}
func wrap(f:func(u64)->const u64)->func(u64)->const u64{return func[f](x:u64)->const u64{return f(x)+1u64;};}
func pick_fn(tag:bool,ref f:func(u64)->const u64 in O)->(func(u64)->const u64) in O where {owner O}{defer{f<-wrap(other);};return f;}
func pick_fn(tag:i64,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {owner O}{defer{f<-wrap(other);};return f;}
func read_fn(tag:T,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {T,owner O}{return pick_fn(tag,f);}
'''
runner.CASES = {
 'cleanup_view_vs_snapshot':(IO+PICK+READ+'val x=1;behave @print(read(false,x));behave @print(x);x<-1;behave @print(read(0,x));behave @print(x);',0,'2\n2\n1\n2\n',''),
 'reverse_demand_order':(IO+PICK+READ+'val x=1;behave @print(read(0,x));x<-1;behave @print(read(false,x));',0,'1\n2\n',''),
 'direct_contracts_are_preserved':(IO+PICK+'val x=1;ref a=pick(false,x);val b=pick(0,x);x<-9;behave @print(a);behave @print(b);',0,'9\n2\n',''),
 'independent_local_value':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{val v=pick(tag,x);x<-9;return v;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'2\n1\n',''),
 'conditional_value_consumption':(IO+PICK+'func read(tag:T,ref x:i64 in O,c:bool)->const i64 where {T,owner O}{return if(c){pick(tag,x)}else{4};}val x=1;behave @print(read(false,x,true));x<-1;behave @print(read(0,x,true));behave @print(read(false,x,false));',0,'2\n1\n4\n',''),
 'full_scope_value_consumption':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return {{defer{x<-9;};pick(tag,x)}};}val x=1;behave @print(read(false,x));behave @print(x);x<-1;behave @print(read(0,x));behave @print(x);',0,'2\n9\n1\n9\n',''),
 'guarded_value_consumption':(IO+NPE+NULL+'func read(tag:T,ptr x:i64 in O)->const i64 where {T,owner O}{return try{pick(tag,x)}catch(e:const NullPointerException){8};}val x=1;ptr p=x;p=null;behave @print(read(false,p));behave @print(read(0,p));',0,'8\n7\n',''),
 'once_only_operands':(IO+PICK+'proc read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return pick({behave @print(3);tag},{behave @print(4);x});}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'3\n4\n2\n3\n4\n1\n',''),
 'value_before_later_operand_effect':(IO+PICK+'func first(x:i64,y:i64)->const i64{return x;}func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return first(pick(tag,x),x<-9);}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'2\n1\n',''),
 'ordinary_recursion_reuses_result_choice':(IO+PICK+'func walk(tag:T,ref x:i64 in O,n:i64)->const i64 where {T,owner O}{if(n==0){return pick(tag,x);}else{return walk(tag,x,n-1);};}val x=1;behave @print(walk(false,x,30));x<-1;behave @print(walk(0,x,40));',0,'2\n1\n',''),
 'mutual_recursion_reuses_result_choice':(IO+PICK+'func even(tag:T,ref x:i64 in O,n:i64)->const i64 where {T,owner O}{if(n==0){return pick(tag,x);}else{return odd(tag,x,n-1);};}func odd(tag:T,ref x:i64 in O,n:i64)->const i64 where {T,owner O}{return even(tag,x,n);}val x=1;behave @print(even(false,x,30));x<-1;behave @print(odd(0,x,40));',0,'2\n1\n',''),
 'loop_value_consumption':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{val sum=0;for(i in 0..3){sum<-sum+pick(tag,x);};return sum;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n6\n',''),
 'fold_value_consumption':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return fold(i in 0..3,sum=0){sum+pick(tag,x)};}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'9\n6\n',''),
 'deferred_value_snapshot':(IO+PICK+'proc show(x:i64)->void{behave @print(x);}proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{defer show(pick(tag,x));x<-9;}val x=1;run(false,x);x<-1;run(0,x);',0,'2\n1\n',''),
 'deferred_block_value_read':(IO+PICK+'proc run(tag:T,ref x:i64 in O)->void where {T,owner O}{defer{behave @print(pick(tag,x));};x<-9;}val x=1;run(false,x);x<-1;run(0,x);',0,'10\n9\n',''),
 'deferred_replacement_return':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{defer{return pick(tag,x);};return 0;}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'2\n1\n',''),
 'finally_replacement_return':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{try{return 0;}finally{return pick(tag,x);};}val x=1;behave @print(read(false,x));x<-1;behave @print(read(0,x));',0,'2\n1\n',''),
 'borrowed_null_checks_before_later_operand':(IO+NPE+NULL+'proc consume(x:i64,y:i64)->void{behave @print(x);}proc run(tag:T,ptr x:i64 in O)->void throws NullPointerException where {T,owner O}{consume(pick(tag,x),{behave @print(99);0});}val x=1;ptr p=x;p=null;try{run(false,p);}catch(e:const NullPointerException){behave @print(8);};try{run(0,p);}catch(e:const NullPointerException){behave @print(8);};',0,'8\n99\n7\n',''),
 'discarded_null_result_is_not_read':(IO+NULL+'proc run(tag:T,ptr x:i64 in O)->void where {T,owner O}{pick(tag,x);behave @print(8);}val x=1;ptr p=x;p=null;run(false,p);run(0,p);',0,'8\n8\n',''),
 'deferred_discarded_result_is_not_read':(IO+NULL+'proc run(tag:T,ptr x:i64 in O)->void where {T,owner O}{defer pick(tag,x);behave @print(8);}val x=1;ptr p=x;p=null;run(false,p);run(0,p);',0,'8\n8\n',''),
 'uninitialized_borrow_still_rejected':(NPE+NULL+'func read(tag:T,ptr x:i64 in O)->const i64 throws NullPointerException where {T,owner O}{return pick(tag,x);}val x=make local i64;try{read(false,x);read(0,x);{}}catch(e:const NullPointerException){};',2,'','cannot prove initialization'),
 'nullable_borrow_still_requires_handler':(NPE+NULL+'func read(tag:T,ptr x:i64 in O)->const i64 where {T,owner O}{return pick(tag,x);}val x=1;read(false,x);read(0,x);',2,'','requires a covering catch'),
 'fresh_borrowed_result_remains_gap':('func pick(tag:bool,ref x:i64 in O)->ref i64 in O where {owner O}{return x+1;}func pick(tag:i64,ref x:i64 in O)->const i64 where {owner O}{return x;}',2,'','fresh ordinary borrowed results'),
 'returned_local_view_is_not_promoted':('func pick(tag:bool,x:i64)->ref i64 in O where {owner O}{return x;}',2,'','declared external owner'),
 'reference_storage_layout_composes':(IO+PICK+'func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{ref y=pick(tag,x);return y;}val x=1;behave @print(read(false,x));behave @print(read(0,x));',0,'2\n2\n',''),
 'borrowed_result_does_not_adopt_value_contract':(PICK+'func read(tag:T,ref x:i64 in O)->ref i64 in O where {T,owner O}{return pick(tag,x);}val x=1;read(false,x);read(0,x);',2,'','fresh ordinary borrowed results'),
 'read_for_argument_preparation_modes':(IO+PICK+'func take(tag:bool,ref x:i64 in O)->const i64 where {owner O}{x<-9;return x;}func take(tag:i64,x:i64)->const i64{return x;}func read(tag:T,ref x:i64 in O)->const i64 where {T,owner O}{return take(tag,pick(tag,x));}val x=1;behave @print(read(false,x));behave @print(x);x<-1;behave @print(read(0,x));behave @print(x);',0,'9\n9\n1\n2\n',''),
 'borrowed_operator_or_builtin_result':(IO+'func (+)(ref x:i64 in O,y:i64)->ref i64 in O where {owner O}{x<-intrinsic (+)(x,y);return x;}func read(ref x:T in O,y:T)->const T where {T,owner O}{return x+y;}val x=1,y=1u64;behave @print(read(x,2));behave @print(read(y,2u64));behave @print(x);behave @print(y);',0,'3\n3\n3\n1\n',''),
 'complete_callable_result_contracts':(IO+CALLABLES+'{{func f=wrap(base);func g=read_fn(false,f);f<-base;behave @print(g(1u64));}};{{func f=wrap(base);func g=read_fn(0,f);f<-base;behave @print(g(1u64));}};',0,'12\n2\n',''),
 'immediate_callable_result_invocation':(IO+CALLABLES+'{{func f=wrap(base);behave @print(read_fn(false,f)(1u64));}};{{func f=wrap(base);behave @print(read_fn(0,f)(1u64));}};',0,'12\n2\n',''),
 'callable_result_saved_before_later_effect':(IO+CALLABLES+'func apply(f:func(u64)->const u64,n:u64)->const u64{return f(n);}func read(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{return apply(pick_fn(tag,f),{f<-base;1u64});}{{func f=wrap(base);behave @print(read(false,f));}};{{func f=wrap(base);behave @print(read(0,f));}};',0,'12\n2\n',''),
 'captured_type_result_context':(IO+CALLABLES+'func truth()->const bool{return false;}func number()->const i64{return 0;}func factory(g:func()->const T)->func(func(u64)->const u64)->func(u64)->const u64 where T{return func[g](f:func(u64)->const u64)->func(u64)->const u64{return pick_fn(g(),f);};}func a=factory(truth);func b=factory(number);behave @print(a(wrap(base))(1u64));behave @print(b(wrap(base))(1u64));',0,'12\n2\n',''),
 'exception_payload_result_context':(IO+'using "std:exceptions" MissingReturnException;func missing()->const i64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}func pick(tag:bool,ref e:MissingReturnException in O)->ref MissingReturnException in O where {owner O}{return e;}func pick(tag:i64,ref e:MissingReturnException in O)->const MissingReturnException where {owner O}{return e;}func read(tag:T,ref e:MissingReturnException in O)->const MissingReturnException where {T,owner O}{return pick(tag,e);}val e=error();behave @print(read(false,e).category);behave @print(read(0,e).category);',0,'5\n5\n','[-Wmissing-return]'),

}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
