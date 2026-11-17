#!/usr/bin/env python3
"""Caller binding methods preserve native cleanup and allocation balance."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + """
func (<=>)(ref a:u64 in O,ref b:u64 in O)->void where {owner O}{
 defer intrinsic (<=>)(a,b);
}
func repeat(ref a:T in O,ref b:T in O,n:u64)->void where {T,owner O}{
 if(n==0u64){return;}else{a<=>b;repeat(a,b,n-1u64);};
}
{{
 func kept=wrap(base);
 val x<-1u64,y<-7u64,z<-9u64;ptr p=x,q=y;
 p<=>q;observe(p);observe(q);
 p<=>{p=z;q};observe(p);observe(q);
 p<=>p;observe(p);
 observe(9010u64);repeat(p,q,30u64);observe(9011u64);observe(p);observe(q);
}};
observe(9000u64);
""")
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['7','1','1','9','1','1','9'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
