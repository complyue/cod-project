#!/usr/bin/env python3
"""Selected aliases preserve original image ownership and failure-safe replacement."""
from pathlib import Path
import lexical_query_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE+='''
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
 try{v<-h;{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(v(1u64));
 func saved=func [v](x:u64)->const u64{return v(x);};
 v<-h;observe(saved(1u64));observe(v(1u64));
}
selected_aliases(true);selected_aliases(false);observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==15','checkpoints==16').replace('copy_failures==9 && record_failures','copy_failures==11 && record_failures')
runner.EXPECTED_VALUES+=['12','2','12','12','2','2','2','2','2','2']
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
