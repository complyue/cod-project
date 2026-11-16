#!/usr/bin/env python3
"""Loop payload invariants distinguish values, target writes and slot changes."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN

VALUE = 'val payload:Exception<-unchecked();'
# Root coverage satisfies the declaration; initialization independently requires
# proof that the concrete handler runs on every normal continuation.
OBSERVE = ' {{val observed<-make local u64;try{throw payload;}catch(e:const MissingReturnError){observed<-e.category;{}}catch(e:const Exception){};behave @print(observed);}}; '
WRITE = 'func write(ref target:Exception in O,value:Exception)->void where {owner O}{target<-value;}'
READ = 'func read(ref target:const Exception in O)->void where {owner O}{}'
runner.CASES = {}


def safe(name, source, output='5\n5\n'):
    runner.CASES[name] = (IO+ERROR+source, 0, output, WARN)


def rejected(name, source):
    runner.CASES[name] = (IO+ERROR+source, 2, '', 'before definite initialization')


safe('unchanged_payload', VALUE+'for(i in 0u64..2u64){'+OBSERVE+'};')
safe('unchanged_alias', VALUE+'ptr selected=payload;for(i in 0u64..2u64){'+OBSERVE.replace('throw payload', 'throw selected')+'};')
safe('unrelated_write', VALUE+'val other:Exception<-unchecked();for(i in 0u64..2u64){other<-checked();'+OBSERVE+'};')
safe('unrelated_call', WRITE+VALUE+'val other:Exception<-unchecked();for(i in 0u64..2u64){write(other,checked());'+OBSERVE+'};')
safe('readonly_call', READ+VALUE+'for(i in 0u64..2u64){read(payload);'+OBSERVE+'};')
safe('readonly_prepared_call', READ+VALUE+'for(i in 0u64..2u64){defer read(payload);'+OBSERVE+'};')
safe('saved_value', 'val source:Exception<-unchecked();val payload:Exception<-source;for(i in 0u64..2u64){source<-checked();'+OBSERVE+'};')
safe('later_write', VALUE+'for(i in 0u64..2u64){'+OBSERVE+'};payload<-checked();')
safe('single_iteration_write', VALUE+'for(i in 0u64..1u64){'+OBSERVE+'payload<-checked();};', '5\n')
safe('empty_loop', VALUE+'for(i in 0u64..0u64){payload<-checked();};'+OBSERVE, '5\n')
safe('nested_invariant', VALUE+'for(i in 0u64..2u64){for(j in 0u64..2u64){'+OBSERVE+'};};', '5\n'*4)
safe('fold_invariant', VALUE+'fold(i in 0u64..2u64,total=0u64){'+OBSERVE+'total+1u64};')
safe('dynamic_count', 'proc run(n:u64)->void{'+VALUE+'for(i in 0u64..n){'+OBSERVE+'};}run(0u64);run(2u64);')
safe('generic_value', 'proc run(value:T,n:u64)->void where {T}{val current:T<-value;for(i in 0u64..n){try{throw current;}catch(e:const Exception){behave @print(e.category);};};}'+VALUE+'run(payload,2u64);')
safe('strong_write_before_read', VALUE+'for(i in 0u64..2u64){payload<-unchecked();'+OBSERVE+'payload<-checked();};')
safe('slot_rebind_preserves_old_payload', VALUE+'val other:Exception<-checked();ptr selected=payload;for(i in 0u64..2u64){selected=other;'+OBSERVE+'};')
safe('per_iteration_cleanup_preserves_payload', VALUE+'for(i in 0u64..2u64){ {{defer{behave @print(payload.category);};}};'+OBSERVE+'};', '5\n'*4)

for name, mutation in {
    'direct_write': 'payload<-checked();',
    'alias_write': 'selected<-checked();',
    'call_write': 'write(payload,checked());',
    'callable_write': 'writer(payload,checked());',
    'finally_write': 'try{}finally{payload<-checked();{}};',
    'scope_cleanup_write': '{{defer{payload<-checked();{}};}};',
    'prepared_cleanup_write': '{{defer write(payload,checked());}};',
    'nested_write': 'for(j in 0u64..1u64){payload<-checked();};',
    'nested_else_write': 'for(j in 0u64..0u64){}else{payload<-checked();{}};',
}.items():
    rejected(name, WRITE+VALUE+'ptr selected=payload;func writer=write;for(i in 0u64..2u64){'+OBSERVE+mutation+'};')

rejected('write_equal_on_first_iteration', VALUE+'val incoming:Exception<-unchecked();for(i in 0u64..3u64){'+OBSERVE+'payload<-incoming;incoming<-checked();};')
rejected('rebound_write_target', VALUE+'val other:Exception<-unchecked();ptr selected=other;for(i in 0u64..3u64){'+OBSERVE+'selected<-checked();selected=payload;};')
rejected('rebound_call_target', WRITE+VALUE+'val other:Exception<-unchecked();ptr selected=other;for(i in 0u64..3u64){'+OBSERVE+'write(selected,checked());selected=payload;};')
rejected('borrowed_aliases', 'proc run(ref payload:Exception in O,ref other:Exception in P)->void where {owner O,owner P}{payload<-unchecked();other<-unchecked();for(i in 0u64..2u64){'+OBSERVE+'other<-checked();};}'+VALUE+'run(payload,payload);')
safe('covered_backedge_write', VALUE+'try{for(i in 0u64..2u64){try{throw payload;}catch(e:const MissingReturnError){behave @print(e.category);};payload<-checked();};}catch(e:const Exception){behave @print(e.category);};', '5\n4\n')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('exception_projection_tests.py')]

if __name__ == '__main__':
    raise SystemExit(runner.main())
