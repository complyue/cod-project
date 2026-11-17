#!/usr/bin/env python3
"""Selected aliases preserve original image ownership and failure-safe replacement."""
from pathlib import Path
from native_test_support import run_native
import lexical_query_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc selected_aliases(flag:bool)->void{
 func f=wrap(base);func g=wrap(other);func h=wrap(base);
 ptr a=f,b=g,c=h;
 observe(9010u64);
 var p->try{if(flag){a}else{b}}finally{};var q->p;
 flag<-!flag;q=c;a=g;b=f;
 observe(p(1u64));p<=>c;observe(p(1u64));
 observe(9011u64);
 var v->try{if(flag){f}else{g}}finally{};
 observe(9030u64);
 try{v<-h;{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(v(1u64));
 func saved=func [v](x:u64)->const u64{return v(x);};
 v<-h;observe(saved(1u64));observe(v(1u64));
}
selected_aliases(true);selected_aliases(false);observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==15','checkpoints==16').replace('copy_failures==9 && record_failures','copy_failures==11 && record_failures'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['12','2','12','12','2','2','2','2','2','2'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__=='__main__':
 raise SystemExit(run_native(FIXTURE))
