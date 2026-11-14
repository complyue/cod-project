#!/usr/bin/env python3
"""Non-effecting signature evidence composes guards, loops and callable construction."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
NPE = 'using "std:exceptions" NullPointerException;\n'
PICK = '''func pick(ref x:i64 in O,tag:bool)->const i64 where {owner O}{x<-x+1;return x;}
func pick(x:i64,tag:i64)->const i64{x<-x+10;return x;}
'''
READ = '''proc read(ptr x:i64 in O,tag:bool)->void where {owner O}{behave @print(isnull(x));}
proc read(x:i64,tag:i64)->void{behave @print(x);}
'''
EXPLODE = '''proc explode()->const bool throws NullPointerException{behave @print(8);ptr p:bool=null;return p;}
'''
CALLABLE = '''func one()->const i64{return 1;}func truth()->const bool{return false;}
func accept(x:i64,f:func()->const i64)->const i64{return x+f();}
func accept(ref x:i64 in O,f:func()->const bool)->const i64 where {owner O}{if(f()){x<-x+1;}else{x<-x+2;};return x;}
'''
EXCEPTION = '''using "std:exceptions" MissingReturnException;
using "std:exceptions" SystemException;
func missing()->const i64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func forward(e:T)->const u64 where T{return e.category;}
'''
runner.CASES = {
 'guarded_later_type':(IO+PICK+'val x=1;behave @print(pick(x,try{false}catch(e){false}));behave @print(pick(x,try{0}catch(e){0}));behave @print(x);',0,'2\n12\n2\n',''),
 'guarded_first_operand':(IO+PICK+'val x=1;behave @print(pick(try{x}catch(e){x},false));behave @print(x);',0,'2\n2\n',''),
 'handler_and_finalizer_effect_order':(IO+NPE+PICK+EXPLODE+'val x=1;behave @print(pick({behave @print(7);x},try{explode()}catch(e:const NullPointerException){behave @print(9);false}finally{behave @print(10);}));',0,'7\n8\n9\n10\n2\n',''),
 'handler_return_cancels_invocation':(IO+NPE+PICK+EXPLODE+'proc run(ref x:i64 in O)->const i64 where {owner O}{return pick(x,try{explode()}catch(e:const NullPointerException){return 9;});}val x=1;behave @print(run(x));behave @print(x);',0,'8\n9\n1\n',''),
 'null_copy_precedes_later_guard':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{read(p,try{behave @print(99);0}finally{behave @print(98);});}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'null_borrow_allows_later_guard':(IO+NPE+READ+'val x=1;ptr p=x;p=null;read(p,try{behave @print(8);false}finally{behave @print(9);});',0,'8\n9\ntrue\n',''),
 'guarded_snapshot_boundary':(IO+PICK+'val x=1;behave @print(pick(try{x}finally{x<-5;{}},0));behave @print(x);x<-1;behave @print(pick(try{x}finally{x<-5;{}},false));behave @print(x);',0,'11\n5\n6\n6\n',''),
 'catch_parameter_is_local':(IO+PICK+'val x=1,tag=0;behave @print(pick({try{false}catch(tag){false};x},tag));',0,'11\n',''),
 'guarded_shadow_remains_uncertain':(PICK+'val x=1,tag=0;{{pick({try{val tag=false;}catch(e){};x},tag);}};',2,'','different argument preparation contracts'),
 'finally_declaration_supplies_later_type':(IO+PICK+'val x=1;behave @print(pick({try{}finally{val tag=false;};x},tag));',0,'2\n',''),
 'try_arm_type_mismatch_not_authorized':(PICK+'val x=1;pick(try{x}catch(e){false},false);',2,'','try body and catch arms must have the same result type'),
 'loop_before_operand_result':(IO+PICK+'val x=1;behave @print(pick({for(i in 0..3){behave @print(i);};x},0));',0,'0\n1\n2\n11\n',''),
 'fold_bool_selects_borrow':(IO+PICK+'val x=1;behave @print(pick(x,fold(i in 0..3,b=false){b||i==1}));behave @print(x);',0,'2\n2\n',''),
 'empty_fold_selects_copy':(IO+PICK+'val x=1;behave @print(pick(x,fold(i in 0..0,n=0){n+i}));behave @print(x);',0,'11\n1\n',''),
 'loop_controls_do_not_escape':(IO+PICK+'val x=1,tag=0;behave @print(pick({for(tag in 0u64..2u64){};x},tag));',0,'11\n',''),
 'full_iteration_shadow_does_not_escape':(IO+PICK+'val x=1,tag=0;behave @print(pick({for(i in 0..2){{val tag=false;}};x},tag));',0,'11\n','[-Wshadow]'),
 'null_copy_precedes_fold_effects':(IO+NPE+READ+'val x=1;ptr p=x;p=null;try{read(p,fold(i in 0..3,n=0){behave @print(99);n+i});}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'fold_body_remains_checked':(PICK+'val x=1;pick(x,fold(i in 0..3,n=0){false});',2,'','fold body does not satisfy'),
 'loop_declaration_remains_checked':(PICK+'val x=1;pick({for(i in 0..3){val a=1;};x},false);',2,'','nested per-iteration'),
 'inferred_callable_local_signature':(IO+CALLABLE+'val x=1;behave @print(accept(x,{{func f=truth;f}}));behave @print(accept(x,{{func f=one;f}}));behave @print(x);',0,'3\n4\n3\n',''),
 'anonymous_callable_local_signature':(IO+CALLABLE+'val x=1;behave @print(accept(x,{{func f=func()->const bool{return true;};f}}));',0,'2\n',''),
 'named_lexical_callable_signature':(IO+PICK+'val x=1;behave @print(pick(x,{{func tag()->const bool{return false;};tag()}}));',0,'2\n',''),
 'anonymous_callable_invocation_signature':(IO+PICK+'val x=1;behave @print(pick(x,(func()->const bool{return false;})()));',0,'2\n',''),
 'callable_capture_construction_is_not_evaluated_early':(IO+NPE+READ+'func number()->const i64{return 0;}proc run(ptr p:i64 in O)->void throws NullPointerException where {owner O}{read(p,{{behave @print(99);func f=func[number]()->const i64{return number();};f()}});}val x=1;ptr p=x;p=null;try{run(p);}catch(e:const NullPointerException){behave @print(7);};',0,'7\n',''),
 'inherited_callable_type_context':(IO+PICK+'func run(ref x:i64 in O,tag:T)->const i64 where {T,owner O}{return pick(x,{{func identity(v:T)->const T{return v;};identity(tag)}});}val x=1;behave @print(run(x,false));behave @print(run(x,0));',0,'2\n12\n',''),
 'checked_callable_context_can_widen_throws':(IO+NPE+'func one()->const i64{return 1;}func choose(f:func()->const i64 throws NullPointerException,tag:i64)->const i64 throws NullPointerException{return f();}func choose(ref f:func()->const bool in O,tag:bool)->const i64 where {owner O}{return 9;}try{behave @print(choose(one,0));}catch(e:const NullPointerException){};',0,'1\n',''),
 'annotation_keeps_callable_view_type':(IO+NPE+'func one()->const i64{return 1;}func take(x:i64,f:func()->const i64 throws NullPointerException)->const i64 throws NullPointerException{return x+f();}func take(ref x:i64 in O,f:func()->const bool)->const i64 where {owner O}{return x;}val x=1;try{behave @print(take(x,{{func f:func()->const i64 throws NullPointerException=one;f}}));}catch(e:const NullPointerException){};',0,'2\n',''),
 'capture_validity_remains_checked':(PICK+'val x=1;pick(x,{{func f=func[x]()->const bool{x;return false;};f()}});',2,'','capture of ordinary lexical cells'),
 'shadowed_callable_does_not_supply_outer_signature':(PICK+'val x=1;pick(x,{{func tag()->const bool{return false;};{{val tag=0;tag()}}}});',2,'','different argument preparation contracts'),
 'concrete_exception_view_adapts_to_common_method':(IO+EXCEPTION+'func common(e:SystemException)->const u64{return forward(e);}func f:func(MissingReturnException)->const u64=common;behave @print(f(error()));',0,'5\n','[-Wmissing-return]'),
 'callable_adapter_retains_common_dispatch':(IO+EXCEPTION+'func select(e:SystemException)->const u64{return 1u64;}func select(e:MissingReturnException)->const u64{return 2u64;}func dispatch(e:T)->const u64 where T{return select(e);}func common(e:SystemException)->const u64{return dispatch(e);}func f:func(MissingReturnException)->const u64=common;behave @print(f(error()));',0,'1\n','[-Wmissing-return]'),
 'captured_exception_view_adapter':(IO+EXCEPTION+'func common(e:SystemException)->const u64{return forward(e);}func wrap(f:func(MissingReturnException)->const u64)->func(MissingReturnException)->const u64{return func[f](e:MissingReturnException)->const u64{return f(e);};}func f:func(MissingReturnException)->const u64=common;func g=wrap(f);behave @print(g(error()));',0,'5\n','[-Wmissing-return]'),
 'deferred_exception_view_adapter':(IO+EXCEPTION+'proc common(e:SystemException)->void{behave @print(forward(e));}proc p:proc(MissingReturnException)->void=common;defer p(error());',0,'5\n','[-Wmissing-return]'),
 'exception_adapter_preserves_payload':(IO+EXCEPTION+'func common(e:SystemException)->const u64{return e.cause_count;}func f:func(MissingReturnException)->const u64=common;val e=error();val chain=try{throw e from e;}catch(e:const MissingReturnException){e};behave @print(f(chain));',0,'1\n','[-Wmissing-return]'),

}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
