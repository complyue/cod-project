#!/usr/bin/env python3
"""Scoped owner evidence preserves native callable preparation and cleanup."""
from pathlib import Path
from native_test_support import run_native
import owner_dispatch_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
old = '''func walk_owner(select:F,ref f:func(u64)->const u64 in O,n:u64)->const u64 where {F,owner O}{
 if(n==0u64){func g=base;ptr p=select(f,g);return owner_read(f,p);}else{return walk_owner(select,f,n-1u64);};
}'''
new = '''func walk_owner(select:F,ref f:func(u64)->const u64 in O,n:u64) where {F,owner O}{
 if(n==0u64){return {{func g=base;ptr p=select(f,g);p(0u64)+owner_code(owner_tag(f,p))}};}else{return walk_owner(select,f,n-1u64);};
}'''
assert old in FIXTURE.source
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace(old, new))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func owner_tag(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in O)->bool where {owner O}{return true;}
func owner_tag(ref f:func(u64)->const u64 in O,ref g:func(u64)->const u64 in P)->i64 where {owner O,owner P}{return 0;}
func owner_code(tag:bool)->u64{return 100u64;}
func owner_code(tag:i64)->u64{return 200u64;}
func scoped_query_factory(ref source:func(u64)->const u64 in O)->func()->u64 where {owner O}{
 return {{func local=base;
  func inspect(){val query<-make local typeof(owner_tag(source,local)){0};return owner_code(query);};inspect}};
}
{{
 func plain=wrap(base);observe(9010u64);
 func saved=scoped_query_factory(plain);observe(saved());observe(9011u64);
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==10', 'checkpoints==11'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['200'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
