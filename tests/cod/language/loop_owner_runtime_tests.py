#!/usr/bin/env python3
"""Loop result/query evidence preserves complete images and bounded invocation."""
from pathlib import Path
from native_test_support import run_native
import scoped_owner_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func loop_tag(ref a:const u64 in O,ref b:const u64 in O)->bool where {owner O}{return true;}
func loop_tag(ref a:const u64 in O,ref b:const u64 in P)->u64 where {owner O,owner P}{return 0u64;}
func loop_code(value:bool)->u64{return 101u64;}
func loop_code(value:u64)->u64{return 202u64;}
func loop_probe(ref input:const u64 in O,n:u64) where {owner O}{
 if(n==0u64){for(index in 0u64..1u64){return loop_code(loop_tag(input,index));};}
 else{return loop_probe(input,n-1u64);};
}
func loop_factory(){
 for(index in 0u64..1u64){return {{
  func inspect(){val query<-make local typeof(loop_tag(index,index)){true};return loop_code(query);};inspect
 }};};
}
{{
 val input<-1u64;
 func plain=wrap(base);observe(9010u64);
 func saved=loop_factory();observe(saved());observe(loop_probe(input,40u64));
 val total<-0u64;for(count in 0u64..10000u64){total<-total+loop_probe(input,0u64);};
 observe(9011u64);observe(total);
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==11', 'checkpoints==12'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['101', '202', '2020000'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
