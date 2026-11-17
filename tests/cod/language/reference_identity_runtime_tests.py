#!/usr/bin/env python3
"""Shared owner identities preserve native resource balance through calls and cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func same(ptr lhs:T in A,ptr rhs:U in B)->bool where {T,U,owner A,owner B}{return lhs is rhs;}
func repeated(ptr lhs:T in A,ptr rhs:T in B,n:u64)->bool where {T,owner A,owner B}{
 if(n==0u64){return same(lhs,rhs);}else{return repeated(lhs,rhs,n-1u64);};
}
proc identity_later(ptr lhs:u64 in A,ptr rhs:u64 in B)->void where {owner A,owner B}{
 if(lhs is rhs){observe(1u64);}else{observe(0u64);};
}
{{
 func kept=wrap(base);
 ptr first:u64=null in here;
 {{
  ptr second:u64=null in here;
  observe(9010u64);
  val count<-0u64;
  for(i in 0u64..1000u64){
   if(repeated(first,first,10u64)&&not repeated(first,second,10u64)){count<-count+1u64;{}}else{};
  };
  observe(9011u64);observe(count);
  defer identity_later(first,second);
  defer identity_later(first,first);
  defer (is)(first,second);
 }};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5', 'checkpoints==6'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['1000','1','0'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
