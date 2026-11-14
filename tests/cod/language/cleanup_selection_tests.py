#!/usr/bin/env python3
"""Cleanup composes selected-target writes, reads and rebind invalidation."""
from pathlib import Path
import foundation_tests as runner

IO='using symbol "std:io" @print;'
NPE='using "std:exceptions" NullPointerException;'
OOM='using "std:exceptions" OutOfMemory;'
ERROR='using "std:exceptions" MissingReturnException;func missing()->const i64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}'
WARN='[-Wmissing-return]'
SETUP='val x<-make local i64,y<-make local i64,z<-make local i64;ptr a=x,b=y,c=z;'
ALIASES='var p->if(flag){a}else{b};var q->p;'
SETTER=NPE+'proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-21;}'
READER=NPE+'proc show(ptr x:const i64 in O)->void throws NullPointerException where {owner O}{behave @print(x);}'
def run(body,selection=ALIASES,setup=SETUP,throws=''):
 source=IO+'proc run(flag:bool)->void '+throws+'{'+setup+selection+body+'}'
 return source+('try{run(true);run(false);}catch(e:const NullPointerException){};' if throws else 'run(true);run(false);')
def good(body,output='1\n1\n',**kwargs):return run(body,**kwargs),0,output,''
def bad(body,diagnostic='before definite initialization',**kwargs):return run(body,**kwargs),2,'',diagnostic
runner.CASES={
 'selected_alias_scope_write':good('{{defer{p<-1;};}};behave @print(q);'),
 'ordinary_pointer_scope_write':good('{{defer{p<-1;};}};behave @print(p);',selection='ptr p=if(flag){x}else{y};'),
 'value_alias_scope_write':good('{{defer{p<-1;};}};behave @print(p);',selection='var p->if(flag){x}else{y};'),
 'guarded_reader_follows_writer':good('defer{behave @print(p);}finally{};defer{q<-1;};'),
 'registration_failure_does_not_write':bad('defer{behave @print(p);};defer{q<-1;};','cannot prove initialization'),
 'reader_precedes_writer':bad('defer{p<-1;};defer{behave @print(p);}finally{};','cannot prove initialization'),
 'entry_fact_supplies_delayed_read':good('p<-1;defer{behave @print(q);};'),
 'late_body_write_supplies_delayed_read':good('defer{behave @print(q);};p<-1;'),
 'alias_does_not_initialize_all_targets':bad('{{defer{p<-1;};}};behave @print(x);'),
 'both_action_branches_write':good('{{defer{if(flag){p<-1;}else{q<-1;};};}};behave @print(p);'),
 'one_action_branch_does_not_write':bad('{{defer{if(flag){p<-1;{}}else{};};}};behave @print(p);'),
 'both_registered_branches_write':good('{{if(flag){defer{p<-1;};}else{defer{q<-1;};};}};behave @print(p);'),
 'conditional_registration_does_not_write':bad('{{if(flag){defer{p<-1;};}else{};}};behave @print(p);'),
 'rebind_after_write_invalidates':bad('{{defer{p=z;};defer{p<-1;};}};behave @print(p);'),
 'write_after_rebind_initializes':good('{{defer{p<-1;};defer{p=z;};}};behave @print(p);'),
 'rebind_inside_action_invalidates':bad('{{defer{p<-1;p=z;};}};behave @print(p);'),
 'write_after_action_rebind':good('{{defer{p=z;p<-1;};}};behave @print(p);'),
 'physical_rebind_invalidates_alias':bad('{{defer{a=z;};defer{p<-1;};}};behave @print(p);'),
 'overlapping_alias_rebind_invalidates':bad('var other->if(flag){b}else{a};{{defer{other=z;};defer{p<-1;};}};behave @print(p);'),
 'unrelated_rebind_preserves_fact':good('{{defer{c=x;};defer{p<-1;};}};behave @print(p);'),
 'initialized_rebind_supplies_fact':good('z<-1;{{defer{p=z;};}};behave @print(p);'),
 'cleanup_invalidates_entry_fact':bad('p<-1;{{defer{p=z;};}};behave @print(p);'),
 'rebind_before_delayed_read_rejects':bad('p<-1;defer{behave @print(p);}finally{};defer{p=z;};','cannot prove initialization'),
 'read_before_action_rebind':good('p<-1;defer{behave @print(p);p=z;};'),
 'earlier_write_supplies_read_before_rebind':good('defer{behave @print(p);p=z;}finally{};defer{p<-1;};'),
 'write_to_old_target_does_not_initialize_new':bad('{{defer{p<-{p=z;1};};}};behave @print(p);'),
 'same_action_write_then_read':good('defer{p<-1;behave @print(q);};'),
 'nested_cleanup_read':good('defer{{{{defer{p<-1;};}};behave @print(q);}};'),
 'nested_registration_supplies_scope_fact':good('{{defer{defer{p<-1;};};}};behave @print(p);'),
 'nested_registration_rebind_invalidates':bad('{{defer{defer{p=z;};p<-1;};}};behave @print(p);'),
 'loop_one_registration':good('{{for(i in 0..1){defer{p<-1;};};}};behave @print(p);'),
 'loop_zero_registrations':bad('{{for(i in 0..0){defer{p<-1;};};}};behave @print(p);'),
 'loop_branch_registration':good('{{for(i in 0..3){if(flag){defer{p<-1;};}else{defer{q<-1;};};};}};behave @print(p);'),
 'loop_local_storage_is_fresh':bad('for(i in 0..2){ {{val v<-make local i64,w<-make local i64;var r->if(flag){v}else{w};behave @print(r);r<-1;}}; };'),
 'generic_cleanup':(IO+'func fill(ptr x:T in O,ptr y:T in O,flag:bool,value:T)->const T throws NullPointerException where {T,owner O}{ptr p=if(flag){x}else{y};{{defer{p<-value;};}};return p;}'+NPE+'val a<-make local i64,b<-make local i64,c<-make local bool,d<-make local bool;try{behave @print(fill(a,b,true,22));behave @print(fill(c,d,false,true));}catch(e:const NullPointerException){};',0,'22\ntrue\n',''),
 'setter_call_inside_action':(SETTER+run('{{defer{set(p);};}};behave @print(p);',throws='throws NullPointerException'),0,'21\n21\n',''),
 'reader_call_inside_action':(READER+run('defer{show(p);}finally{};defer{p<-1;};',throws='throws NullPointerException'),0,'1\n1\n',''),
 'prepared_setter_stable_selection':(SETTER+run('{{defer set(p);}};behave @print(p);',throws='throws NullPointerException'),0,'21\n21\n',''),
 'prepared_setter_old_target':(SETTER+run('{{defer set(p);p=z;}};behave @print(p);',throws='throws NullPointerException'),2,'','before definite initialization'),
 'prepared_reader_stable_selection':(READER+run('defer show(p);p<-1;',throws='throws NullPointerException'),0,'1\n1\n',''),
 'prepared_reader_cannot_use_new_selection':(READER+run('defer show(p);p=z;p<-1;',throws='throws NullPointerException'),2,'','cannot prove initialization'),
 'failure_after_selected_write':(ERROR+run('val e<-error();try{{defer{p<-1;throw e;};}}catch(e:const MissingReturnException){behave @print(p);};'),0,'1\n1\n',WARN),
 'failure_before_selected_write':(ERROR+run('val e<-error();try{{defer{throw e;p<-1;};}}catch(e:const MissingReturnException){behave @print(p);};'),2,'','before definite initialization'),
 'recovery_reader_after_writer_failure':(ERROR+run('val e<-error();defer catch(e:const MissingReturnException){behave @print(p);};defer{p<-1;throw e;};'),0,'1\n1\n',WARN),
 'recovering_handler_initializes':(ERROR+run('val e<-error();{{defer catch(e:const MissingReturnException){p<-1;};throw e;}};behave @print(p);'),0,'1\n1\n',WARN),
 'category_specific_finally':(ERROR+OOM+run('val e<-error();defer{}catch(e:const MissingReturnException){p<-1;}finally{behave @print(p);};throw e;'),0,'1\n1\n',WARN),
 'returning_cleanup_keeps_write':(IO+'proc run(flag:bool)->const i64{'+SETUP+ALIASES+'defer{behave @print(p);}finally{};defer{p<-1;return 2;};return 3;}behave @print(run(true));behave @print(run(false));',0,'1\n2\n1\n2\n',''),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':raise SystemExit(runner.main())
