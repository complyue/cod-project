#!/usr/bin/env python3
"""Account for complete family captures, replay and failed preparations."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
proc observe(x:u64)->void{behave @print(x);}
symbol @x:u64,@y:u64,@f:proc(u64)->u64;
proc @f(n:u64)->u64{return @x;}
proc @f(n:T)->u64 where T{return @y;}
proc read()->u64{return @x;}
proc @bad(n:u64)->u64{observe(@x);proc local=read;return local();}
effect @x<-1u64;effect @y<-7u64;
{{
 effect @f;effect @x<-3u64;effect @y<-4u64;
 observe(9010u64);val sum<-0u64;
 for(i in 0u64..1000u64){sum<-sum+perform @f(1u8)+perform @f(1u64)+behave @f(1u8);};
 observe(sum);observe(@x);observe(@y);observe(9011u64);
}};
observe(9000u64);
{{
 effect @f;observe(9020u64);
 try{effect @f;}catch(e:const OutOfMemory){observe(9021u64);};
 observe(perform @f(1u8));observe(perform @f(1u64));
}};
observe(9000u64);
{{
 effect @f;observe(9030u64);
 try{effect @f=@f;}catch(e:const OutOfMemory){observe(9031u64);};
 observe(perform @f(1u8));
 observe(9030u64);
 try{defer perform @f(1u8);}catch(e:const OutOfMemory){observe(9031u64);};
 observe(perform @f(1u64));
}};
observe(9000u64);
{{
 effect @f;observe(9050u64);
 try{defer perform @f(1u8);}catch(e:const OutOfMemory){observe(9051u64);};
 observe(perform @f(1u8));
}};
observe(9000u64);
{{
 effect @bad;effect @x<-9u64;observe(9020u64);
 try{perform @bad(1u8);{}}catch(e:const OutOfMemory){observe(9021u64);observe(@x);};
}};
observe(9000u64);
{{
 effect @f;
 for(i in 0u64..20u64){effect @x<-i;effect @y<-i+1u64;effect @f;defer perform @f(1u8);};
 observe(perform @f(1u8));
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6').replace('creation_failures==1', 'creation_failures==2').replace('copy_failures==3', 'copy_failures==2').replace('largest>4000', 'largest>0')
runner.EXPECTED_VALUES = ['12000', '3', '4', '7', '1', '7', '1', '7', '1', '9', '7']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
