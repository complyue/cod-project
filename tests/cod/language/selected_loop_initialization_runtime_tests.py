#!/usr/bin/env python3
"""Loop-selected initialized targets survive cleanup and injected registration failure."""
from pathlib import Path
from native_test_support import run_native
from deferred_initialization_runtime_tests import FIXTURE as ALLOCATION

PRELUDE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
proc observe(value:u64)->void{behave @print(value);return;}

proc advance_iteration(ptr selected:u64 in O,ptr first:const u64 in O,ptr next:u64 in O,fail:bool)
    ->ptr u64 in O where {owner O}{
 if(fail){observe(9001u64);}else{};
 try{
  defer{if(selected is first && next is not null){next<-88u64;return next;}else{};};
 }catch(e:const OutOfMemoryError){
  observe(9002u64);
  if(selected is first && next is not null){next<-88u64;return next;}else{};
 };
 return selected;
}

proc verify(fail:bool)->void{
 val first<-87u64,next<-make local u64;
 ptr selected=first;
'''
LOOP = '''
 for(i in 0u64..3u64){
  observe(selected);
  selected=advance_iteration(selected,first,next,fail);
 };
 observe(selected);
'''
CALLS = '''
 return;
}
verify(false);
verify(true);
'''


def loop_fixture(*, deferred=False):
    body = 'defer{'+LOOP+'};' if deferred else LOOP
    # Each of the three ordinary iterations registers once. The deferred loop
    # adds one outer record per call, released before the first iteration.
    created = 5 if deferred else 3
    original_counts = 'created==6 && peak==2 && failures==2'
    assert original_counts in ALLOCATION.harness
    harness = ALLOCATION.harness.replace(original_counts, f'created=={created} && peak==1 && failures==3')
    return ALLOCATION.updated(source=PRELUDE+body+CALLS, harness=harness,
                              expected_values=('87','88','88','88')*2, sources=[Path(__file__)])


FIXTURE = loop_fixture()
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
