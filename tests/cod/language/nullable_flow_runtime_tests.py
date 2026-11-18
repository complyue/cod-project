#!/usr/bin/env python3
"""Nullable result provenance retains image and cleanup ownership."""
from pathlib import Path
from native_test_support import run_native
import callable_flow_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func nullable_holder(ptr f:F in O)->ptr F in O where {F,owner O}{return f;}
func walk_nullable(f:F,ptr value:T in O,n:u64)->ptr T in O where {F,T,owner O}{
 if(n==0u64){return f(value);}else{return walk_nullable(f,value,n-1u64);};
}
proc register_during_nullable_helper()->void{observe(9050u64);defer observe(9999u64);}
proc null_recovery(ptr x:u64 in O,ptr y:u64 in O)->ptr u64 in O where {owner O}{
 defer catch(e:const OutOfMemoryError){observe(9051u64);return y;};
 defer register_during_nullable_helper();
 return x;
}
{{
 func f=wrap(wrap(base));func g=wrap(other);
 ptr p=nullable_holder(f);observe(p(1u64));
 observe(9030u64);try{p<-g;{}}catch(e:const OutOfMemoryError){observe(9031u64);};observe(p(2u64));
 nullable_holder(f)<-g;observe(f(3u64));
 func select=func(ptr h:func(u64)->const u64 in O)->ptr (func(u64)->const u64) in O where {owner O}{return h;};
 func captured=func[select](ptr h:func(u64)->const u64 in O)->ptr (func(u64)->const u64) in O where {owner O}{return select(h);};
 ptr q=walk_nullable(captured,f,40u64);observe(q(4u64));
 observe(9010u64);val total<-0u64;
 for(i in 0u64..10000u64){{ptr live=nullable_holder(f);total<-total+live(i);}};
 observe(9011u64);observe(total);
 ptr absent=f;absent=null;
 try{nullable_holder(absent)(5u64);{}}catch(e:const NullPointerException){observe(17u64);};
 val x<-18u64,y<-19u64;observe(null_recovery(x,y));
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==7','checkpoints==8').replace('copy_failures==6','copy_failures==7').replace('record_failures==3','record_failures==4'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['3','4','14','15','50105000','17','19'])
FIXTURE = FIXTURE.updated(expected_shadows=['other'])
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
