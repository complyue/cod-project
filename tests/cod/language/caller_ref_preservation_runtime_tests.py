#!/usr/bin/env python3
"""Caller ref preservation composes with native images and failed registration."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source_files={
    'swap.cod': ('func (<=>)(ptr a:T in O,ptr b:T in O)->void where {T,owner O}{'
                 'intrinsic (<=>)(a,b);}export (<=>);'),
})
FIXTURE = FIXTURE.updated(source='using "std:exceptions" NullPointerException;'
    'using "./swap.cod" (<=>) as inner;' + FIXTURE.source + '''
func (<=>)(ptr a:T in O,ptr b:T in O)->void where {T,owner O}{inner(a,b);}
func (<=>)(ptr a:u64 in O,ptr b:u64 in O)->void throws NullPointerException where {owner O}{
 if(a>0u64){a<-a-1u64;intrinsic (<=>)(a,b);a<=>b;}else{};
}
proc exercise(ref a:func(u64)->const u64 in O,ref b:func(u64)->const u64 in O)->void where {owner O}{
 observe(9010u64);
 a<=>b;
 for(i in 0u64..1000u64){a<=>b;};
 {{defer (<=>)(a,b);}};
 observe(9011u64);
 observe(a(1u64));observe(b(1u64));
 return;
}
{{
 func kept=wrap(base);
 val x<-1u32,y<-7u32;ref p=x,q=y;
 p<=>q;observe(p);observe(q);
 {{defer (<=>)(p,q);}};observe(p);observe(q);
 observe(9050u64);
 try{defer (<=>)(p,q);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(p);observe(q);
 func first=wrap(base);func second=wrap(other);exercise(first,second);
 observe(9010u64);
 val left<-80u64,right<-80u64;ref u=left,v=right;
 try{u<=>v;}catch(e:const NullPointerException){observe(9999u64);};
 observe(9011u64);
 observe(u);observe(v);
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6')
    .replace('record_failures==1', 'record_failures==2'))
FIXTURE = FIXTURE.updated(expected_values=[*FIXTURE.expected_values, '7', '1', '1', '7', '1', '7', '2', '12', '0', '0'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
