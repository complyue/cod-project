#!/usr/bin/env python3
"""Selected cleanup writes survive real failure paths with exact resource release."""
from pathlib import Path
from native_test_support import run_native
import selected_alias_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func fill_in_helper(ref target:u64 in O)->void where {owner O}{defer{target<-44u64;};}
proc cleanup_selected(flag:bool)->void{
 val x<-make local u64,y<-make local u64,z<-make local u64;
 ptr a=x,b=y;var p->if(flag){a}else{b};
 {{defer{p<-41u64;};}};observe(p);
 {{defer{p<-42u64;};defer{p=z;};}};observe(p);
 func f=wrap(base);func g=wrap(other);
 {{
  defer catch(e:const OutOfMemoryError){observe(9031u64);observe(p);};
  defer{p<-43u64;observe(9030u64);f<-g;};
 }};
 {{
  defer{observe(p);}finally{};
  defer{fill_in_helper(p);};
 }};
 observe(9050u64);
 try{{defer{p<-99u64;};}}catch(e:const OutOfMemoryError){observe(9051u64);observe(p);};
 {{for(i in 0u64..100u64){defer{p<-45u64;};};}};observe(p);
 return;
}
cleanup_selected(true);cleanup_selected(false);observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==16','checkpoints==17').replace('copy_failures==11 && record_failures==3','copy_failures==13 && record_failures==5'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['41','42','43','44','44','45']*2)
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':raise SystemExit(run_native(FIXTURE))
