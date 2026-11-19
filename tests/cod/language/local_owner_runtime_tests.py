#!/usr/bin/env python3
"""Explicit owner aliases preserve native borrows, absent results and cleanup."""
from pathlib import Path
from native_test_support import run_native
import borrowed_call_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 'proc show(x:u64 in O)->void where {owner O}{observe(x);return;}',
 'proc show(x:u64 in O)->void where {owner O}{ref p:u64 in O=x;observe(p);return;}'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 'proc set(x:u64 in O)->void where {owner O}{x<-33u64;return;}',
 'proc set(x:u64 in O)->void where {owner O}{ptr p:u64 in O=null;p=x;p<-33u64;return;}'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 'throws NullPointerException where {owner O}{observe(x);}',
 'throws NullPointerException where {owner O}{ptr p:u64 in O=x;observe(p);}'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer{x<-39u64;};return x;',
 'defer{{ref p:u64 in O=x;p<-39u64;}};return x;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(
 'if(n==0u64){return x;}else{return recursive(x,n-1u64);};',
 'ref p:T in O=x;if(n==0u64){return p;}else{return recursive(p,n-1u64);};'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func absent(ptr x:T in O)->ptr T in O where {T,owner O}{ptr p:T in O=null;return p;}
ptr p=absent(a);behave @print((p is null));p=a;p<-41u64;observe(a);
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['true','41'])
FIXTURE = FIXTURE.updated(expected_shadows=['p', 'p', 'p', 'a', 'p', 'a', 'a', 'p', 'p', 'p', 'p'])
if __name__=='__main__':
 raise SystemExit(run_native(FIXTURE))
