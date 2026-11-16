#!/usr/bin/env python3
"""Caught exception values retain entry facts through completion and cleanup."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN, COVER

VALUE = 'val payload:Exception<-unchecked(),replacement:Exception<-checked();'
SIGNATURE = 'func carry(item:const Exception,other:const Exception)->const Exception'
runner.CASES = {}
def case(name, body, expected=5, setup=VALUE, expression='carry(payload,replacement)'):
    declaration = SIGNATURE+'{'+body+'}'
    catch = 'Exception' if expected == 4 else 'MissingReturnError'
    source = IO+ERROR+declaration+setup+'try{throw '+expression+';}catch(e:const '+catch+'){behave @print(e.category);};'
    runner.CASES[name] = (source,0,str(expected)+'\n',WARN)
    if expected == 4:
        runner.CASES[name+'_requires_cover'] = (ERROR+declaration+setup+'throw '+expression+';',2,'',COVER)

for name, body in {
    'ordinary': 'try{throw item;}catch(e){return e;};',
    'typed_root': 'try{throw item;}catch(e:const Exception){return e;};',
    'typed_partition': 'try{throw item;}catch(e:const MissingReturnError){return e;}catch(e){return e;};',
    'bare_rethrow': 'try{try{throw item;}catch(e){throw;};}catch(e){return e;};',
    'explicit_rethrow': 'try{try{throw item;}catch(e){throw e;};}catch(e){return e;};',
    'rethrow_alias': 'try{try{throw item;}catch(e){ {{var saved->e;throw saved;}};};}catch(e){return e;};',
    'copied_catch': 'try{throw item;}catch(e){ {{const val saved<-e;return saved;}};};',
    'catch_snapshot_after_write': 'val current<-item;try{throw current;}catch(e){current<-other;return e;};',
    'full_scope': 'try{ {{throw item;}}; }catch(e){return e;};',
    'scope_cleanup_mutation': 'val current<-item;try{ {{defer{current<-other;{}};throw current;}}; }catch(e){return e;};',
    'finally': 'try{try{throw item;}finally{};}catch(e){return e;};',
    'finally_mutation': 'val current<-item;try{try{throw current;}finally{current<-other;{}};}catch(e){return e;};',
    'returned_catch_finally': 'try{throw item;}catch(e){try{return e;}finally{};};',
    'deferred_recovery': 'defer catch(e){return e;};throw item;',
    'deferred_typed_partition': 'defer catch(e:const MissingReturnError){return e;}catch(e){return e;};throw item;',
    'deferred_rethrow': 'defer catch(last){return last;};defer catch(first){throw first;};throw item;',
    'deferred_bare_rethrow': 'defer catch(last){return last;};defer catch(first){throw;};throw item;',
    'deferred_finally': 'defer catch(e){return e;}finally{};throw item;',
    'deferred_block_failure': 'defer catch(e){return e;};defer{throw item;};return other;',
    'captured_ordinary_catch': 'defer catch(last){return last;};try{throw item;}catch(saved){defer{throw saved;};};return other;',
    'nested_saved_rethrow': 'defer catch(last){return last;};defer catch(original){defer catch(current){throw original;};throw other;};throw item;',
    'nested_saved_return': 'defer catch(original){defer catch(current){return original;};throw other;};throw item;',
    'nested_scope_saved_return': 'defer catch(original){ {{defer catch(current){return original;};throw other;}};};throw item;',
    'nested_helper': 'defer catch(original){defer catch(current){return keep(original);};throw other;};throw item;',
    'cleanup_batch': 'for(i in 0u64..3u64){defer catch(e){return e;};};throw item;',
}.items():
    case(name,body)
# Helper calls preserve the captured arriving-value relationship.
runner.CASES['nested_helper'] = (runner.CASES['nested_helper'][0].replace(SIGNATURE,'func keep(value:Exception)->const Exception{return value;}'+SIGNATURE),*runner.CASES['nested_helper'][1:])
for name, body in {
    'checked_input': 'try{throw other;}catch(e){return e;};',
    'finally_replaces_exception': 'try{try{throw item;}finally{throw other;};}catch(e){return e;};',
    'deferred_replacement': 'defer catch(last){return last;};defer catch(first){throw other;};throw item;',
    'nested_current_rethrow': 'defer catch(last){return last;};defer catch(original){defer catch(current){throw current;};throw other;};throw item;',
    'nested_current_return': 'defer catch(original){defer catch(current){return current;};throw other;};throw item;',
    'deferred_finally_replaces_return': 'defer catch(last){return last;};defer catch(first){return first;}finally{throw other;};throw item;',
}.items():
    case(name,body,4)
case('replacement_can_remove_checked','try{try{throw other;}finally{throw item;};}catch(e){return e;};')
case('deferred_replacement_can_remove_checked','defer catch(last){return last;};defer catch(first){throw item;};throw other;')
RECURSIVE = 'func cycle(item:Exception,n:u64)->const Exception{if(n==0u64){try{throw item;}catch(e){return e;};}else{return cycle(item,n-1u64);};}'
runner.CASES['recursive'] = (IO+ERROR+RECURSIVE+VALUE+'try{throw cycle(payload,20u64);}catch(e:const MissingReturnError){behave @print(e.category);};',0,'5\n',WARN)
BODY = SIGNATURE+'{try{throw item;}catch(e){return e;};}'
runner.CASES['callable_value'] = (IO+ERROR+BODY+VALUE+'func held=carry;try{throw held(payload,replacement);}catch(e:const MissingReturnError){behave @print(e.category);};',0,'5\n',WARN)
runner.CASES['broad_body_still_checked'] = (ERROR+BODY+'func use(item:Exception)->void{throw carry(item,item);}',2,'',COVER)
runner.CASES['declared_failure_still_checked'] = (ERROR+BODY.replace('->const Exception{','->const Exception throws NullPointerException{')+VALUE+'carry(payload,replacement);',2,'',COVER)
runner.CASES['prepared_failure_replaces_pending'] = (IO+ERROR+'func fail()->void throws NullPointerException{throw checked();}'+SIGNATURE+'{defer catch(e){return e;};defer fail();return item;}'+VALUE+'try{throw carry(payload,replacement);}catch(e:const NullPointerException){behave @print(e.category);};',0,'4\n',WARN)
runner.CASES['explicit_cause_preserves_primary'] = (IO+ERROR+SIGNATURE+'{try{throw item from other;}catch(e){return e;};}'+VALUE+'try{throw carry(payload,replacement);}catch(e:const MissingReturnError){behave @print(e.category);behave @print(e.cause_count);behave @print(cause_at(e,0u64).category);};',0,'5\n1\n4\n',WARN)
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__),Path(__file__).with_name('exception_projection_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
