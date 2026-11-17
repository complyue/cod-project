#!/usr/bin/env python3
"""Loop-selected initialized targets survive cleanup and injected registration failure."""
from pathlib import Path
import call_initialization_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE+='''
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
'''
runner.EXPECTED_VALUES+=['87','88','88','88']*2
assert 'created==5 && peak==1 && failures==1' in runner.HARNESS
runner.HARNESS=runner.HARNESS.replace('created==5 && peak==1 && failures==1','created==8 && peak==1 && failures==4')
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
