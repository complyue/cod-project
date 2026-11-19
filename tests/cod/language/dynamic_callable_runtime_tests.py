#!/usr/bin/env python3
"""Track captured TLS images through replacement, replay and allocation failures."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
proc observe(x:u64)->void{behave @print(x);return;}
symbol @x:u64;
symbol @p:proc()->u64;
proc read()->u64{return @x;}
proc fails()->u64{observe(@x);effect @x<-9u64;proc local=read;return local();}
effect @x<-1u64;
{{
 effect @p=read;effect @x<-2u64;
 observe(9010u64);
 val sum<-0u64;
 for(i in 0u64..1000u64){sum<-sum+perform @p()+behave @p();};
 observe(sum);observe(@x);observe(9011u64);
}};
observe(9000u64);
{{
 effect @p=read;observe(9020u64);
 try{effect @p=read;}catch(e:const OutOfMemoryError){observe(9021u64);};
 observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=read;observe(9030u64);
 try{effect @p=@p;}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=read;observe(9030u64);
 try{defer perform @p();}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=read;observe(9050u64);
 try{defer perform @p();}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=read;observe(9060u64);
 try{effect @p=proc[read]()->u64{return read();};}catch(e:const OutOfMemoryError){observe(9061u64);};
 observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=fails;effect @x<-2u64;observe(9020u64);
 try{perform @p();{}}catch(e:const OutOfMemoryError){observe(9021u64);observe(@x);};
}};
observe(9000u64);
{{
 effect @p=read;observe(9040u64);
 effect @p=proc[read]()->u64{return read();};
 observe(9041u64);observe(perform @p());
}};
observe(9000u64);
{{
 effect @p=read;
 for(i in 0u64..20u64){effect @x<-i;effect @p=read;defer perform @p();};
 observe(perform @p());
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_create=', 'static int creation_countdown=-1;\nstatic bool deny_create='))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('if(deny_create){', 'if(deny_create || (creation_countdown>=0 && creation_countdown--==0)){'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('if(value==9000)', '''if(value==9060){creation_countdown=1;live_baseline=images.size();return;}
  if(value==9061){assert(creation_countdown==-1 && images.size()==live_baseline);return;}
  if(value==9000)'''))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==9').replace('creation_failures==1', 'creation_failures==3').replace('copy_failures==3', 'copy_failures==2').replace('largest>4000', 'largest>0'))
FIXTURE = FIXTURE.updated(expected_values=['3000', '2', '1', '1', '1', '1', '1', '1', '2', '1', '1'])
FIXTURE = FIXTURE.updated(expected_shadows=['x'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
