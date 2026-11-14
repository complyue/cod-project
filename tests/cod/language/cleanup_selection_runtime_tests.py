#!/usr/bin/env python3
"""Selected cleanup writes survive real failure paths with exact resource release."""
from pathlib import Path
import selected_alias_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE+='''
proc cleanup_selected(flag:bool)->void{
 val x=make local u64,y=make local u64,z=make local u64;
 ptr a=x,b=y;var p->if(flag){a}else{b};
 {{defer{p<-41u64;};}};observe(p);
 {{defer{p<-42u64;};defer{p=z;};}};observe(p);
 func f=wrap(base);func g=wrap(other);
 {{
  defer catch(e:const OutOfMemory){observe(9031u64);observe(p);};
  defer{p<-43u64;observe(9030u64);f<-g;};
 }};
 {{
  defer{observe(p);}finally{};
  defer{defer{p<-44u64;};};
 }};
 observe(9050u64);
 try{{defer{p<-99u64;};}}catch(e:const OutOfMemory){observe(9051u64);observe(p);};
 {{for(i in 0u64..100u64){defer{p<-45u64;};};}};observe(p);
}
cleanup_selected(true);cleanup_selected(false);observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==16','checkpoints==17').replace('copy_failures==11 && record_failures==3','copy_failures==13 && record_failures==5')
runner.EXPECTED_VALUES+=['41','42','43','44','44','45']*2
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':raise SystemExit(runner.main())
