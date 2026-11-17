#!/usr/bin/env python3
"""Conditional initialization preserves native borrowed and deferred cleanup."""
from pathlib import Path
from native_test_support import run_native
import call_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
helper = '''
proc fill(ptr output:u64 in O,value:u64,flag:bool)->void throws NullPointerException where {owner O}{
 if(flag){output<-value;{}}else{};
}
proc optional_read(ptr input:u64 in O,flag:bool)->void throws NullPointerException where {owner O}{
 if(flag){observe(input);}else{};
}
'''
FIXTURE = FIXTURE.updated(source=FIXTURE.source + helper)
for before, after in (
    ('x<-71u64;', 'fill(x,71u64,true);'),
    ('fail_after(ptr x:u64 in O)', 'fail_after(ptr x:u64 in O,flag:bool)'),
    ('x<-73u64;', 'if(flag){x<-73u64;{}}else{};'),
    ('fail_after(a);', 'fail_after(a,true);'),
    ('defer{x<-74u64;};', 'val gate<-true;defer{if(gate){x<-74u64;{}}else{};};'),
    ('defer{x<-75u64;observe(y);};', 'val gate<-true;defer{if(gate){x<-75u64;{}}else{};observe(y);};'),
):
    assert before in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, after))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
val unused<-make local u64;
try{optional_read(unused,false);}catch(e:const NullPointerException){observe(99u64);};
''')
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
