#!/usr/bin/env python3
"""Complementary writes preserve native failure prefixes and exact release."""
from pathlib import Path
from native_test_support import run_native
import call_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc fill(ptr output:u64 in O,value:u64,flag:bool)->void throws NullPointerException where {owner O}{
 if(flag){output<-value;{}}else{};
 if(!flag){output<-value;{}}else{};
}
''')
for before, after in (
    ('x<-71u64;', 'fill(x,71u64,false);'),
    ('fail_after(ptr x:u64 in O)', 'fail_after(ptr x:u64 in O,flag:bool)'),
    ('x<-73u64;', 'if(flag){x<-73u64;{}}else{};if(!flag){x<-73u64;{}}else{};'),
    ('fail_after(a);', 'fail_after(a,true);'),
    ('cleanup(ptr x:u64 in O)', 'cleanup(ptr x:u64 in O,flag:bool)'),
    ('defer{x<-74u64;};', 'defer{if(flag){x<-74u64;{}}else{};if(!flag){x<-74u64;{}}else{};};'),
    ('cleanup(b);', 'cleanup(b,false);'),
    ('alias(ptr x:u64 in O,ptr y:u64 in O)', 'alias(ptr x:u64 in O,ptr y:u64 in O,flag:bool)'),
    ('defer{x<-75u64;observe(y);};', 'defer{if(flag){x<-75u64;{}}else{};if(!flag){x<-75u64;{}}else{};observe(y);};'),
    ('alias(c,c);', 'alias(c,c,true);'),
):
    assert before in FIXTURE.source
    FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(before, after))
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
