#!/usr/bin/env python3
"""Loop-selected initialized targets survive cleanup and injected registration failure."""
from pathlib import Path
from native_test_support import run_native
import call_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc verify(fail:bool)->void{
 val first<-87u64,next<-make local u64;
 ptr selected=first;
 for(i in 0u64..3u64){
  observe(selected);
  {{
   if(fail){observe(9001u64);}else{};
   try{defer{if(selected is first){next<-88u64;selected=next;{}}else{};};}
   catch(e:const OutOfMemoryError){
    observe(9002u64);
    if(selected is first){next<-88u64;selected=next;{}}else{};
    {}
   };
  }};
 };
 observe(selected);
}
verify(false);
verify(true);
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['87','88','88','88']*2)
assert 'created==5 && peak==1 && failures==1' in FIXTURE.harness
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==5 && peak==1 && failures==1','created==8 && peak==1 && failures==4'))
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
