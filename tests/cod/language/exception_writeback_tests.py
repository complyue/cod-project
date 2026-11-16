#!/usr/bin/env python3
"""Borrowed payload postconditions preserve entry snapshots and failure paths."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN, COVER

runner.CASES = {}
SIGNATURE = 'func update(ref target:Exception in O)->void where {owner O}'
COPY = 'func update(ref target:Exception in O,item:Exception)->void where {owner O}{target<-item;}'
VALUE = 'val payload:Exception<-checked();'
def safe(name, declarations, body, target='payload'):
    runner.CASES[name] = (IO+ERROR+declarations+body+'try{throw '+target+';}catch(e:const MissingReturnError){behave @print(e.category);};',0,'5\n',WARN)
def checked(name, declarations, body, target='payload', output='4\n'):
    runner.CASES[name] = (ERROR+declarations+body+'throw '+target+';',2,'',COVER)
    runner.CASES[name+'_covered'] = (IO+ERROR+declarations+body+'try{throw '+target+';}catch(e:const Exception){behave @print(e.category);};',0,output,WARN)

safe('constant',SIGNATURE+'{target<-unchecked();}',VALUE+'update(payload);')
safe('entry_value',COPY,VALUE+'update(payload,unchecked());')
safe('unchanged_writable',SIGNATURE+'{}','val payload:Exception<-unchecked();update(payload);')
safe('unchanged_readonly',SIGNATURE.replace('target:Exception','target:const Exception')+'{}','val payload:Exception<-unchecked();update(payload);')
safe('generic','func update(ref target:T in O,item:T)->void where {T,owner O}{target<-item;}',VALUE+'val value:Exception<-unchecked();update(payload,value);')
safe('callable',COPY,VALUE+'func writer=update;writer(payload,unchecked());')
safe('higher_order',COPY+'func apply(writer:F,ref target:Exception in O,item:Exception)->void where {F,owner O}{writer(target,item);}',VALUE+'apply(update,payload,unchecked());')
safe('captured_callable',COPY,VALUE+'func writer=update;func outer=func[writer](ref target:Exception in O,item:Exception)->void where {owner O}{writer(target,item);};outer(payload,unchecked());')
safe('recursive','func update(ref target:Exception in O,item:Exception,n:u64)->void where {owner O}{if(n==0u64){target<-item;{}}else{update(target,item,n-1u64);};}',VALUE+'update(payload,unchecked(),20u64);')
safe('recursive_before_write','func update(ref target:Exception in O,item:Exception,n:u64)->void where {owner O}{if(n!=0u64){update(target,item,n-1u64);}else{};target<-item;}',VALUE+'update(payload,unchecked(),20u64);')
safe('mutual','func update(ref target:Exception in O,item:Exception,n:u64)->void where {owner O}{if(n==0u64){target<-item;{}}else{again(target,item,n-1u64);};}func again(ref target:Exception in O,item:Exception,n:u64)->void where {owner O}{update(target,item,n);}',VALUE+'update(payload,unchecked(),20u64);')
safe('copied_operand_before_mutation','func update(ref target:Exception in O,item:Exception,ignored:bool)->void where {owner O}{target<-item;}','val payload:Exception<-unchecked();update(payload,payload,{payload<-checked();true});')
safe('borrowed_selected_address','func update(ref target:Exception in O,item:Exception,ignored:bool)->void where {owner O}{target<-item;}',VALUE+'val other:Exception<-checked();ptr selected=payload;update(selected,unchecked(),{selected=other;true});')
safe('read_at_entry','func update(ref target:Exception in O,ref incoming:const Exception in P,ignored:bool)->void where {owner O,owner P}{target<-incoming;}',VALUE+'val source:Exception<-checked();update(payload,source,{source<-unchecked();true});')
safe('local_parameter_rebind','func update(ptr target:Exception in O,ref destination:Exception in O)->void where {owner O}{target=destination;}','val payload:Exception<-unchecked(),other:Exception<-checked();update(payload,other);')
runner.CASES['exceptional_write'] = (IO+ERROR+'func update(ref target:Exception in O,item:MissingReturnError)->void where {owner O}{target<-item;throw item;}'+VALUE+'val failure<-unchecked();try{try{update(payload,failure);}catch(e:const MissingReturnError){throw payload;};}catch(e:const MissingReturnError){behave @print(e.category);};',0,'5\n',WARN)
safe('checked_exit','func update(ref target:Exception in O,item:MissingReturnError,failure:NullPointerException)->void throws NullPointerException where {owner O}{target<-item;throw failure;}',VALUE+'try{update(payload,unchecked(),checked());}catch(e:const NullPointerException){};')
safe('finally_write',SIGNATURE+'{try{return;}finally{target<-unchecked();{}};}',VALUE+'update(payload);')
safe('nested_scope',SIGNATURE+'{ {{target<-unchecked();}};}',VALUE+'update(payload);')
safe('symbolic_current',COPY.replace('func update','proc @update')+'effect @update;',VALUE+'behave @update(payload,unchecked());')
safe('symbolic_captured',COPY.replace('func update','proc @update')+'effect @update;',VALUE+'perform @update(payload,unchecked());')
checked('checked_output',SIGNATURE+'{target<-checked();}','val payload:Exception<-unchecked();update(payload);')
checked('failure_before_write',SIGNATURE+'{throw unchecked();target<-unchecked();}',VALUE+'try{update(payload);}catch(e:const MissingReturnError){};')
checked('borrow_observes_later_mutation','func update(ref target:Exception in O,ref incoming:const Exception in P,ignored:bool)->void where {owner O,owner P}{target<-incoming;}','val payload:Exception<-unchecked(),source:Exception<-unchecked();update(payload,source,{source<-checked();true});')
checked('callable_choice',COPY+'func replacement(ref target:Exception in O,item:Exception)->void where {owner O}{target<-checked();}',VALUE+'func writer=if(false){update}else{replacement};writer(payload,unchecked());')
checked('aliased_formals','func update(ref left:Exception in O,ref right:Exception in P)->void where {owner O,owner P}{left<-unchecked();right<-checked();}','val payload:Exception<-unchecked();update(payload,payload);')
checked('caller_alias','func update(ref target:Exception in O)->void where {owner O}{target<-checked();}func wrapper(ref observed:const Exception in O,ref target:Exception in P)->void throws Exception where {owner O,owner P}{update(target);throw observed;}','val payload:Exception<-unchecked();try{wrapper(payload,payload);}catch(e:const Exception){payload<-e;{}};')
safe('prepared_write',SIGNATURE+'{target<-unchecked();}',VALUE+'{{defer update(payload);}};')
safe('deferred_block_write',SIGNATURE+'{defer{target<-unchecked();{}};}',VALUE+'update(payload);')
runner.CASES['initialization_obligation']=(ERROR+'func inspect(ref target:Exception in O)->void where {owner O}{}val payload:Exception;inspect(payload);',2,'','initialization')
runner.CASES['readonly_destination']=(ERROR+SIGNATURE+'{target<-unchecked();}const val payload:Exception<-checked();update(payload);',2,'','readonly argument')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__),Path(__file__).with_name('exception_projection_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
