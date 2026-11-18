#!/usr/bin/env python3
"""Deferred exception values preserve runtime categories and declared obligations."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN, COVER

VALUE = 'val payload:Exception<-unchecked();'
HANDLER = 'catch(e:const Exception){behave @print(e.category);}'
OBSERVE = 'try{throw payload;}'+HANDLER+';'
# Narrow coverage cannot waive a root-typed throw's declared obligation.
NARROW_HANDLER = 'catch(e:const MissingReturnError){behave @print(e.category);}'
NARROW_OBSERVE = 'try{throw payload;}'+NARROW_HANDLER+';'
runner.CASES = {}


def safe(name, source, output='5\n'):
    runner.CASES[name] = (IO+ERROR+source,0,output,WARN)


def rejected(name, source, diagnostic=COVER):
    runner.CASES[name] = (IO+ERROR+source,2,'',diagnostic)


safe('internal_handler',VALUE+'{{defer{'+OBSERVE+'};}};')
safe('outer_handler',VALUE+'try{ {{defer{throw payload;};}}; }'+HANDLER+';')
safe('deferred_handler',VALUE+'try{ {{defer catch(e:const MissingReturnError){behave @print(e.category);};defer{throw payload;};}}; }catch(unexpected:const Exception){behave @print(99);};')
safe('nested_scope',VALUE+'{{defer{ {{defer{'+OBSERVE+'};}};};}};')
safe('nested_registration',VALUE+'{{defer{defer{'+OBSERVE+'};};}};')
safe('loop_iteration',VALUE+'for(i in 0u64..2u64){ {{defer{'+OBSERVE+'};}};};','5\n5\n')
safe('loop_queue',VALUE+'{{for(i in 0u64..2u64){defer{'+OBSERVE+'};};}};','5\n5\n')
safe('written_before_invocation','val payload:Exception<-checked();val incoming<-unchecked();{{defer{'+OBSERVE+'};payload<-incoming;}};')
safe('earlier_action_write',VALUE+'val incoming<-unchecked();{{defer{'+OBSERVE+'};defer{payload<-incoming;{}};payload<-checked();}};')
safe('finally_replacement',VALUE+'try{ {{defer{try{throw payload;}finally{payload<-checked();{}};};}}; }'+HANDLER+';behave @print(payload.category);','5\n4\n')
safe('saved_throw_before_write',VALUE+'try{ {{defer{throw payload;};defer{try{throw payload;}finally{payload<-checked();{}};};}}; }catch(e:const Exception){behave @print(e.category);};','4\n')
safe('return_after_handler','proc run()->u64{'+VALUE+'defer{'+OBSERVE+'};return 7u64;}behave @print(run());','5\n7\n')
safe('saved_value','val source:Exception<-unchecked();val payload:Exception<-source;{{defer{'+OBSERVE+'};source<-checked();}};')
safe('readonly_alias',VALUE+'ptr selected:const Exception=payload;{{defer{'+OBSERVE.replace('throw payload','throw selected')+'};}};')
safe('write_after_cleanup',VALUE+'{{defer{'+OBSERVE+'};}};payload<-checked();')
safe('recursive_known_value','func value(n:u64)->const Exception{if(n==0u64){return unchecked();}else{return value(n-1u64);};}val payload:Exception<-value(4u64);{{defer{'+OBSERVE+'};}};')

rejected('late_write',VALUE+'{{defer{'+NARROW_OBSERVE+'};payload<-checked();}};')
rejected('earlier_action_checked',VALUE+'{{defer{'+NARROW_OBSERVE+'};defer{payload<-checked();{}};}};')
rejected('replacement_failure',VALUE+'{{defer{try{throw payload;}'+HANDLER+'finally{throw checked();};};}};', COVER)
rejected('unhandled_checked','val payload:Exception<-checked();{{defer{'+NARROW_OBSERVE+'};}};')
rejected('declared_call_obligation', 'func replay(value:Exception)->void throws Exception{throw value;}'+VALUE+'{{defer{try{replay(payload);}catch(e:const MissingReturnError){};};}};', COVER)
rejected('unknown_parameter','proc run(value:Exception)->void{val payload:Exception<-value;defer{'+NARROW_OBSERVE+'};}run(unchecked());')
rejected('recursive_checked_result','func value(n:u64)->const Exception{if(n==0u64){return checked();}else{return value(n-1u64);};}val payload:Exception<-value(4u64);{{defer{'+NARROW_OBSERVE+'};}};')
rejected('empty_recursive_summary_is_unknown','func forever()->const Exception{return forever();}val payload:Exception<-forever();{{defer{'+NARROW_OBSERVE+'};}};')
rejected('empty_deferred_result_is_unknown','func forever()->const Exception{return forever();}'+VALUE+'{{defer{'+NARROW_OBSERVE+'};defer{payload<-forever();{}};}};')
rejected('uninitialized_read','val payload<-make local Exception;{{defer{'+NARROW_OBSERVE+'};}};')
runner.CASES['uninitialized_read'] = (runner.CASES['uninitialized_read'][0],2,'','initialization')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name:[] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__),Path(__file__).with_name('exception_projection_tests.py')]


from language_test_support import expect_source_rejections
expect_source_rejections(runner, (
    'nested_registration',
), 'currently draining scope')

if __name__ == '__main__':
    raise SystemExit(runner.main())
