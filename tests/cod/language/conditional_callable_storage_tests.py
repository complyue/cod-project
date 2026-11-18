#!/usr/bin/env python3
"""Callable receivers retain targets or complete independent values through cleanup."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
TYPE = 'func()->const u64'
VALUES = 'func one()->const u64{return 1u64;}func two()->const u64{return 2u64;}func three()->const u64{return 3u64;}'
WRAP = 'func wrap(value:'+TYPE+')->'+TYPE+'{return func[value]()->const u64{return value()+10u64;};}'
NPE = 'using "std:exceptions" NullPointerException;'
MISSING = 'using "std:exceptions" MissingReturnError;func missing()->'+TYPE+'{}'
BASE = 'func first=one;func second=two;'
READERS = NPE+'func write(ptr target:i64 in O)->void throws NullPointerException where {owner O}{target<-7;}func read(ptr target:i64 in O)->void throws NullPointerException where {owner O}{target;}'
def run(body):
    return IO+VALUES+WRAP+'proc run(flag:bool)->void{'+body+'}run(true);run(false);'

runner.CASES = {
 'target_or_fresh': (run(BASE+'ptr selected=if(flag){first}else{two};first<-three;'+OUT('selected()')+OUT('selected is first')),0,'3\ntrue\n2\nfalse\n',''),
 'reversed_arms': (run(BASE+'ptr selected=if(flag){two}else{first};first<-three;'+OUT('selected()')),0,'2\n3\n',''),
 'write_selected_payload': (run(BASE+'ptr selected=if(flag){first}else{two};selected<-three;'+OUT('first()')+OUT('selected()')),0,'3\n3\n1\n3\n',''),
 'independent_copy': (run(BASE+'ptr selected=if(flag){first}else{wrap(two)};func saved=selected;selected<-three;'+OUT('saved()')+OUT('selected()')),0,'1\n3\n12\n3\n',''),
 'nested_capture': (run('func first=wrap(one);ptr selected=if(flag){first}else{wrap(wrap(two))};func saved=wrap(selected);first<-three;selected<-one;'+OUT('saved()')),0,'21\n32\n',''),
 'alias_receiver_slot': (run(BASE+'ptr selected=if(flag){first}else{wrap(two)};ptr var alias->selected;alias=second;'+OUT('selected()')+OUT('first()')),0,'2\n1\n2\n1\n',''),
 'fixed_slot': (run(BASE+'const ptr selected=if(flag){first}else{two};selected<-three;'+OUT('selected()')+OUT('first()')),0,'3\n3\n3\n1\n',''),
 'readonly_target': (run('const func first=one;ptr selected=if(flag){first}else{two};'+OUT('selected()')),0,'1\n2\n',''),
 'full_scope_callable_result_is_independent': (run('func first=one;ptr target:'+TYPE+'=first;'
     'func selected={{defer{target<-three;};if(flag){return target;}else{return wrap(two);};}};'
     +OUT('selected()')+OUT('first()')),0,'1\n3\n12\n3\n',''),
 'finally_retains_selection': (run(BASE+'ptr original=first;ptr selected=try{if(flag){original}else{wrap(two)}}finally{original=second;first<-three;{}};'+OUT('selected()')+OUT('original()')),0,'3\n2\n12\n2\n',''),
 'catch_fresh': (MISSING+run(BASE+'ptr selected=try{if(flag){first}else{missing()}}catch(e:const MissingReturnError){wrap(two)};first<-three;'+OUT('selected()')),0,'3\n12\n','[-Wmissing-return]'),
 'catch_target': (MISSING+run(BASE+'ptr selected=try{if(flag){missing()}else{wrap(two)}}catch(e:const MissingReturnError){first};first<-three;'+OUT('selected()')),0,'3\n12\n','[-Wmissing-return]'),
 'null_or_fresh': (NPE+run('ptr absent:'+TYPE+'=null in here;ptr selected=if(flag){absent}else{wrap(two)};'+OUT('selected is null')+'try{'+OUT('selected()')+'}catch(e:const NullPointerException){'+OUT('9')+'};'),0,'true\n9\nfalse\n12\n',''),
 'nested_selection': (run(BASE+'ptr selected=if(flag){try{first}finally{ {}; }}'
     'else{if(flag){first}else{second}};first<-three;'+OUT('selected()')),0,'3\n2\n',''),
 'deferred_snapshot': (run(BASE+'ptr selected=if(flag){first}else{wrap(two)};defer ('+'proc(value:'+TYPE+')->void{'+OUT('value()')+'})(selected);selected<-three;'),0,'1\n12\n',''),
 'deferred_late_read': (run(BASE+'ptr selected=if(flag){first}else{wrap(two)};defer{'+OUT('selected()')+'};selected<-three;'),0,'3\n3\n',''),
 'loop_release_boundary': (run('for(i in 0..3){{func first=one;ptr selected=if(flag){first}else{wrap(two)};'+OUT('selected()')+'}};'),0,'1\n1\n1\n12\n12\n12\n',''),
 'abrupt_receiver': (IO+VALUES+'proc work(flag:bool)->u64{func first=one;func second=two;ptr selected:'+TYPE+
     '=try{if(flag){first}else{second}}finally{return 8u64;};'+OUT('99')+'return selected();}'
     +OUT('work(true)')+OUT('work(false)'),0,'8\n8\n',''),
 'abrupt_target_arm': (IO+VALUES+'func work(flag:bool)->u64{func first=one;'
     'ptr selected=if(flag){try{first}finally{return 8u64;};}else{first};return selected();}'
     +OUT('work(true)')+OUT('work(false)'),0,'8\n1\n',''),
 'shared_generic_modes': (IO+VALUES+'func copy(value:T)->const T where T{return value;}func work(value:T,flag:bool,n:u64)->const T where T{ptr local=copy(value);ptr selected=if(flag){local}else{copy(value)};if(n==0u64){return selected;}else{return work(value,flag,n-1u64);};}'+OUT('work(one,true,30u64)()')+OUT('work(two,false,30u64)()')+OUT('work(7u64,true,30u64)'),0,'1\n2\n7\n',''),
 'procedure_values': (IO+'proc one()->void{'+OUT('1')+'}proc two()->void{'+OUT('2')+'}proc run(flag:bool)->void{proc first=one;ptr selected=if(flag){first}else{two};first<-two;selected();}run(true);run(false);',0,'2\n2\n',''),
 'writer_initializes': (IO+READERS+'proc run(flag:bool)->void{func writer=write;val raw<-make local i64;ptr selected=if(flag){writer}else{write};try{selected(raw);'+OUT('raw')+'}catch(e:const NullPointerException){};}run(true);run(false);',0,'7\n7\n',''),
 'reader_requires_initialization': (READERS+'proc run(flag:bool)->void{func writer=write;val raw<-make local i64;ptr selected=if(flag){writer}else{read};try{selected(raw);}catch(e:const NullPointerException){};}run(true);',2,'',"initialization of 'raw'"),
 'different_owners': (VALUES+'func stored=one;ptr outer:'+TYPE+'=stored;'
     '{{func local=two;ptr selected=if(true){outer}else{local};}};',2,'','reference initializer joins different owners'),
 'external_owner': (VALUES+'func work(ref source:'+TYPE+' in O,flag:bool)->void where {owner O}{ptr selected=if(flag){source}else{two};}',2,'','reference initializer joins different owners'),
 'readonly_write': (VALUES+'const func first=one;ptr selected=if(true){first}else{two};selected<-three;',2,'','readonly destination'),
 'readonly_upgrade': (VALUES+'const func first=one;ptr selected:'+TYPE+'=if(true){first}else{two};',2,'','readonly source cannot initialize'),
 'fixed_rebind': (VALUES+BASE+'const ptr selected=if(true){first}else{two};selected=second;',2,'','const reference binding'),
 'fresh_rebind_remains_invalid': (VALUES+BASE+'ptr selected=first;selected=if(true){first}else{two};',2,'','fresh value without an existing target'),
 'fresh_borrow_remains_invalid': (VALUES+'func take(ref source:'+TYPE+' in O)->void where {owner O}{}'+BASE+'take(if(true){first}else{two});',2,'','borrowed arguments require existing target views'),
 'local_result_cannot_escape': (VALUES+'func work(ref source:'+TYPE+' in O)->ptr '+TYPE+' in O where {owner O}{func first=one;ptr selected=if(true){first}else{two};return selected;}',2,'','declared external owner'),
}
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
