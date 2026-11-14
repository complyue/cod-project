#!/usr/bin/env python3
"""Physical target proofs survive native cleanup and injected allocation failures."""
from pathlib import Path
import cleanup_selection_runtime_tests as fixture
runner = fixture.runner
runner.SOURCE += '''
proc cleanup_targets(flag:bool)->void{
 val x<-make local u64,y<-make local u64,z<-make local u64;
 val w<-make local u64,v<-make local u64,t<-make local u64;
 val s<-make local u64,r<-make local u64,u<-make local u64,k<-make local u64,last<-make local u64,gated<-make local u64;
 ptr a=x,b=y;var p->if(flag){a}else{b};
 {{defer{z<-46u64;};defer{p=z;};}};observe(p);
 p=w;{{defer{p<-47u64;};}};observe(w);
 func f=wrap(base);func g=wrap(other);p=v;
 {{
  defer{try{p=v;p<-48u64;observe(9030u64);f<-g;{}}
        catch(e:const OutOfMemory){observe(9031u64);observe(v);};};
 }};
 p=z;observe(9050u64);
 try{{defer{p=w;};}}catch(e:const OutOfMemory){observe(9051u64);observe(p);};
 {{for(i in 0u64..100u64){defer{t<-49u64;};defer{p=t;};};}};observe(p);
 ptr n=if(flag){t}else{null};{{defer{t<-50u64;};}};
 if(isnull(n)){observe(51u64);}else{observe(n);};
 p=if(flag){x}else{y};
 {{defer{p=s;};defer{p<-52u64;};defer{p=s;};}};observe(p);
 {{defer{observe(r);}finally{};defer{p<-53u64;};defer{p=r;};}};
 p=u;{{defer{a=k;};defer{p<-54u64;};defer{k<-55u64;};}};observe(p);
 p=last;
 {{defer{try{p<-56u64;p=s;observe(9030u64);f<-g;{}}
         catch(e:const OutOfMemory){observe(9031u64);observe(last);};};}};
 p=null;{{defer{p=s;};}};ref ready=p;observe(ready);
 {{defer{p=null;};}};try{observe(p);}catch(e:const NullPointerException){observe(57u64);};
 p=if(flag){gated}else{null};{{defer{if(isnull(p)){}else{p<-58u64;{}};};}};
 try{observe(p);}catch(e:const NullPointerException){observe(59u64);};
}
cleanup_targets(true);cleanup_targets(false);observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==17', 'checkpoints==18').replace(
    'copy_failures==13 && record_failures==5', 'copy_failures==17 && record_failures==7')
runner.EXPECTED_VALUES += ['46', '47', '48', '46', '49', '50', '52', '53', '55', '56', '52', '57', '58',
                          '46', '47', '48', '46', '49', '51', '52', '53', '54', '56', '52', '57', '59']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__': raise SystemExit(runner.main())
