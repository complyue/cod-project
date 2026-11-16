#!/usr/bin/env python3
"""Callable result facts preserve entry snapshots, mutation and cleanup."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN, COVER

KEEP = 'func keep(item:Exception)->const Exception{return item;}'
VALUE = 'val payload:Exception<-unchecked();'
runner.CASES = {}
def safe(name, declarations, setup, expression, output='5\n'):
    source = IO+ERROR+declarations+setup+'try{throw '+expression+';}catch(e:const Exception){behave @print(e.category);};'
    runner.CASES[name] = (source,0,output,WARN)
def checked(name, declarations, setup, expression):
    source = ERROR+declarations+setup+'throw '+expression+';'
    runner.CASES[name] = (source,2,'',COVER)
    source = IO+ERROR+declarations+setup+'try{throw '+expression+';}catch(e:const Exception){behave @print(e.category);};'
    runner.CASES[name+'_covered'] = (source,0,'4\n',WARN)

safe('constant', 'func produce()->const Exception{return unchecked();}', '', 'produce()')
safe('copied_input', KEEP, VALUE, 'keep(payload)')
safe('copied_local', 'func keep(item:Exception)->const Exception{val saved<-item;return saved;}', VALUE, 'keep(payload)')
safe('generic', 'func keep(item:T)->const T where {T}{return item;}', VALUE, 'keep(payload)')
safe('recursive', 'func keep(item:Exception,n:u64)->const Exception{if(n==0u64){return item;}else{return keep(item,n-1u64);};}', VALUE, 'keep(payload,20u64)')
safe('mutual', 'func left(item:Exception,n:u64)->const Exception{if(n==0u64){return item;}else{return right(item,n-1u64);};}func right(item:Exception,n:u64)->const Exception{return left(item,n);}', VALUE, 'left(payload,20u64)')
safe('value_precedes_later_operand', 'func keep(item:Exception,ignored:bool)->const Exception{return item;}', VALUE, 'keep(payload,{payload<-checked();true})')
READ = 'func keep(ref item:const Exception in O,ignored:bool)->const Exception where {owner O}{return item;}'
checked('borrow_observes_later_operand', READ, VALUE, 'keep(payload,{payload<-checked();true})')
safe('borrow_retains_selected_address', READ, VALUE+'val other:Exception<-checked();ptr selected=payload;', 'keep(selected,{selected=other;true})')
safe('borrowed_result', 'func keep(ref item:const Exception in O)->ref const Exception in O where {owner O}{return item;}', VALUE, 'keep(payload)')
safe('borrowed_return_finally', 'func keep(ref item:Exception in O)->ref Exception in O where {owner O}{try{return item;}finally{item<-unchecked();{}};}', 'val payload:Exception<-checked();', 'keep(payload)')
checked('borrowed_return_mutated_finally', 'func keep(ref item:Exception in O)->ref Exception in O where {owner O}{try{return item;}finally{item<-checked();{}};}', VALUE, 'keep(payload)')
safe('copied_return_finally', 'func keep(ref item:Exception in O)->const Exception where {owner O}{try{return item;}finally{item<-checked();{}};}', VALUE, 'keep(payload)')
safe('finally_replaces_result', 'func keep(item:Exception)->const Exception{try{return checked();}finally{return item;};}', VALUE, 'keep(payload)')
checked('finally_replaces_with_checked', 'func keep(item:Exception)->const Exception{try{return item;}finally{return checked();};}', VALUE, 'keep(payload)')
safe('deferred_result', 'func keep(item:const Exception)->const Exception{defer{return item;};return checked();}', VALUE, 'keep(payload)')
safe('deferred_copied_snapshot', 'func keep(item:Exception)->const Exception{const val saved<-item;defer{return saved;};item<-checked();return item;}', VALUE, 'keep(payload)')
checked('deferred_checked_replacement', 'func keep(item:Exception)->const Exception{defer{return checked();};return item;}', VALUE, 'keep(payload)')
safe('deferred_recovery', 'func keep()->const Exception{defer catch(e:const MissingReturnError){return e;};missing();}', '', 'keep()')
safe('interrupted_return_recovers', 'func keep(item:Exception)->const Exception{defer catch(e:const NullPointerException){};defer{throw checked();};return item;}', VALUE, 'keep(payload)')
safe('readonly_call', 'func keep(ref item:const Exception in O)->const Exception where {owner O}{return item;}', VALUE, 'keep(payload)')
PAIR = 'func keep(ref item:Exception in O,ref other:Exception in P)->const Exception where {owner O,owner P}'
checked('aliased_write_before_copy', PAIR+'{item<-unchecked();other<-checked();return item;}', VALUE, 'keep(payload,payload)')
safe('copy_before_aliased_write', PAIR+'{item<-unchecked();val saved<-item;other<-checked();return saved;}', VALUE, 'keep(payload,payload)')
READ_PAIR = 'func keep(ref item:const Exception in O,ref other:Exception in P)->const Exception where {owner O,owner P}'
checked('readonly_alias_write', READ_PAIR+'{other<-checked();return item;}', VALUE, 'keep(payload,payload)')
checked('readonly_alias_cleanup', READ_PAIR+'{ {{defer{other<-checked();{}};}};return item;}', VALUE, 'keep(payload,payload)')
checked('readonly_alias_loop', READ_PAIR+'{for(i in 0u64..2u64){other<-checked();{}};return item;}', VALUE, 'keep(payload,payload)')
safe('callable_value', KEEP, VALUE+'func held=keep;', 'held(payload)')
safe('higher_order', KEEP+'func apply(f:F,item:Exception)->const Exception where {F}{return f(item);}', VALUE+'func held=keep;', 'apply(held,payload)')
safe('captured_callable', KEEP, VALUE+'func held=keep;func outer=func[held](item:Exception)->const Exception{return held(item);};', 'outer(payload)')
runner.CASES['non_callable_capture_remains_unproved'] = (ERROR+VALUE+'func held=func[payload]()->const Exception{return payload;};held();',2,'','capture of ordinary lexical cells or non-callable values')
safe('symbolic_current', 'proc @keep(item:Exception)->const Exception{return item;}effect @keep;', VALUE, 'behave @keep(payload)')
safe('symbolic_captured', 'proc @keep(item:Exception)->const Exception{return item;}effect @keep;', VALUE, 'perform @keep(payload)')
checked('copied_checked_input', KEEP, 'val payload:Exception<-checked();', 'keep(payload)')
checked('voluntary_throws', 'func keep(item:Exception)->const Exception throws NullPointerException{return item;}', VALUE, 'keep(payload)')
# A declared failure remains an obligation even when this execution returns an unchecked value.
runner.CASES['voluntary_throws_covered'] = (IO+ERROR+'func keep(item:Exception)->const Exception throws NullPointerException{return item;}'+VALUE+'try{throw keep(payload);}catch(e:const Exception){behave @print(e.category);};',0,'5\n',WARN)
checked('borrowed_return_deferred_write', 'func keep(ref item:Exception in O)->ref Exception in O where {owner O}{defer{item<-checked();{}};return item;}', VALUE, 'keep(payload)')
checked('callable_choice_union', KEEP+'func replace(item:Exception)->const Exception{return checked();}', VALUE+'func chosen=if(false){keep}else{replace};', 'chosen(payload)')
safe('nested_forwarding', KEEP+'func outer(item:Exception)->const Exception{return keep(keep(item));}', VALUE, 'outer(payload)')
runner.CASES['broad_body_retains_obligation'] = (ERROR+KEEP+'func use(item:Exception)->void{throw keep(item);}',2,'',COVER)
runner.CASES['provisional_common_catch_value'] = (IO+ERROR+'func produce()->const Exception{return unchecked();}val reply<-try{throw produce();}catch(e:const Exception){7u64};behave @print(reply);',0,'7\n',WARN)
runner.CASES['recursive_catch_summary_converges'] = (ERROR+'func cycle()->const Exception throws Exception{try{throw cycle();}catch(e:const Exception){return checked();};}',0,'',WARN)
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__),Path(__file__).with_name('exception_projection_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
