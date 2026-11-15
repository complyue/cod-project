#!/usr/bin/env python3
"""Caller binding methods preserve native cleanup and allocation balance."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += """
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
"""
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6')
runner.EXPECTED_VALUES += ['7','1','1','9','1','1','9']
runner.EXPECTED_SHADOWS = []
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
