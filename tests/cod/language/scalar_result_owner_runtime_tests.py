#!/usr/bin/env python3
"""Scalar results retain snapshots or targets through cleanup and failed delivery."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func snapshot(ref x:u64 in O)->u64 where {owner O}{defer{x<-9u64;};return x;}
func target(ref x:u64 in O)->u64 in O where {owner O}{defer{x<-9u64;};return x;}
func selected_target(ref x:u64 in O,ref y:u64 in O)->const u64 in O where {owner O}{
 ptr p=x;defer{p=y;x<-7u64;};return p;
}
func replaced_snapshot()->val u64{defer{return 8u64;};return 1u64;}
func failed_snapshot(ref x:u64 in O)->u64 where {owner O}{x<-30u64;defer{x<-40u64;};return 99u64;}
func scalar_increment(x:u64)->val u64{return x+1u64;}
{{
 val x<-1u64,y<-2u64;
 val copied<-snapshot(x);observe(copied);observe(x);
 x<-1u64;ref materialized=snapshot(x);x<-8u64;observe(materialized);observe(x);
 x<-1u64;val read_after<-target(x);x<-8u64;observe(read_after);
 x<-1u64;ref retained=target(x);x<-8u64;observe(retained);
 x<-1u64;ref selected=selected_target(x,y);observe(selected);observe(y);
 observe(replaced_snapshot());
 func value_call=snapshot;func view_call=target;
 x<-1u64;ref copied_call=value_call(x);observe(copied_call);observe(x);
 x<-1u64;ref borrowed_call=view_call(x);observe(borrowed_call);
 observe(9050u64);
 try{y<-failed_snapshot(x);{}}catch(e:const OutOfMemory){observe(9051u64);};
 observe(x);observe(y);
 ptr absent:u64=null in here;ptr forwarded=absent;
 forwarded=x;observe(forwarded);behave @print(isnull(absent));
 func live=wrap(base);val total<-0u64;
 observe(9010u64);
 for(i in 0u64..10000u64){total<-total+scalar_increment(i);};
 observe(9011u64);observe(total);observe(live(1u64));
}};
observe(9000u64);
'''
runner.EXPECTED_VALUES += ['1', '9', '1', '8', '9', '8', '7', '2', '8', '1', '9', '9', '30', '2', '30', 'true', '50005000', '2']
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6').replace('record_failures==1', 'record_failures==2')
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
