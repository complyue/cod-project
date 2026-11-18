#!/usr/bin/env python3
"""Definition construction and direct capture preparation balance native resources."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
{{
 func source=wrap(base);
 func read[source](x:u64)->u64{return source(x);};
 func first[source](x:u64)->u64{return source(x);};
 func second[source](x:u64)->u64{return source(x);};
 func combined[first,second](x:u64)->u64{return first(x)+second(x);};
 observe(9022u64);
 try{func partial=combined;}catch(e:const OutOfMemoryError){observe(9023u64);};
 observe(combined(3u64));
 func a=read;source<-wrap(other);func b=read;
 observe(a(1u64));observe(b(1u64));observe(read(3u64));
 observe(9020u64);
 try{func failed=read;}catch(e:const OutOfMemoryError){observe(9021u64);};
 observe(9030u64);
 try{read({{observe(9999u64);return 0u64;}});{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 proc later[source](x:u64)->void{observe(source(x));};
 {{defer later(4u64);source<-wrap(base);}};
 observe(source(5u64));
 source<-wrap(base);
 observe(9050u64);
 try{defer later(99u64);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(9040u64);
 try{read(missing());{}}catch(e:const MissingReturnError){observe(9041u64);};
 observe(read(5u64));
}};
observe(9000u64);
{{
 func source=wrap(base);
 observe(9010u64);
 func direct[source](x:u64)->u64{return source(x);};
 val value<-make local typeof(direct(0u64)){7u64};
 val total<-0u64;
 for(i in 0u64..10000u64){total<-total+direct(i);};
 observe(9011u64);observe(total);observe(value);
}};
observe(9000u64);
symbol @bias:u64;
symbol @recursive:proc(u64)->u64;
proc fresh()->u64{
 proc recur(n:u64)->u64{
   if(n==0u64){return @bias;}else{};
   effect @bias<-20u64;proc next=recur;effect @recursive=next;
   return perform @recursive(0u64);
 };
 effect @bias<-10u64;effect @recursive=recur;
 return perform @recursive(1u64);
}
observe(fresh());observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5 && creation_failures==1 && copy_failures==3 && record_failures==1',
                                      'checkpoints==8 && creation_failures==3 && copy_failures==4 && record_failures==2'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_create=false',
    'static bool partial_create_failure=false; static unsigned remaining_creations=0;\nstatic bool deny_create=false'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('if(deny_create){++creation_failures;status[0]=1;return nullptr;}',
    'if(deny_create || (partial_create_failure && remaining_creations==0)){++creation_failures;status[0]=1;return nullptr;} if(partial_create_failure)--remaining_creations;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('if(value==9020)',
    'if(value==9022){partial_create_failure=true;remaining_creations=1;live_baseline=images.size();return;} '
    'if(value==9023){assert(partial_create_failure && remaining_creations==0 && images.size()==live_baseline);partial_create_failure=false;return;} '
    'if(value==9020)'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('&& !deny_create &&', '&& !partial_create_failure && !deny_create &&'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['8','2','12','14','15','16','6','50005000','7','20'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
