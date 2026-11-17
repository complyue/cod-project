#!/usr/bin/env python3
"""Caught-owner proofs preserve delayed recovery, images and query-only values."""
from pathlib import Path
from native_test_support import run_native
import loop_owner_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func handler_tag(ref a:const A in O,ref b:const B in O)->bool where {A,B,owner O}{return true;}
func handler_tag(ref a:const A in O,ref b:const B in P)->u64 where {A,B,owner O,owner P}{return 0u64;}
func handler_code(value:bool)->u64{return 101u64;}
func handler_code(value:u64)->u64{return 202u64;}
func handler_probe(ref input:const MissingReturnError in O,n:u64) where {owner O}{
 if(n==0u64){try{throw input;}catch(caught:const MissingReturnError){return handler_code(handler_tag(input,caught));};}
 else{return handler_probe(input,n-1u64);};
}
func handler_factory(){
 try{missing();}catch(caught:const MissingReturnError){return {{
  func inspect(){val query<-make local typeof(handler_tag(caught,caught)){true};return handler_code(query);};inspect
 }};};
}
func handler_recover(input:func(u64)->const u64){
 defer catch(caught:const OutOfMemoryError){return handler_code(handler_tag(caught,caught));};
 func built=wrap(input);
}
{{
 try{missing();{}}catch(seed:const MissingReturnError){{
  func plain=wrap(base);observe(9010u64);
  func saved=handler_factory();observe(saved());observe(handler_probe(seed,40u64));
  val total<-0u64;for(count in 0u64..10000u64){total<-total+handler_probe(seed,0u64);};
  observe(9011u64);observe(total);
 }};
 observe(9020u64);observe(handler_recover(base));observe(9021u64);
 try{handler_recover(base);{}}catch(caught:const MissingReturnError){observe(303u64);};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==12', 'checkpoints==13').replace('creation_failures==4', 'creation_failures==5'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['101', '202', '2020000', '101', '303'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
