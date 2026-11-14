#!/usr/bin/env python3
"""Retained declaration contexts do not own images; real captures still do."""
from pathlib import Path
import anonymous_query_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE+='''
proc lexical_queries(f:func(u64)->const u64)->void{
 observe(9010u64);
 func q=func()->const u64{val x<-make local typeof(f(1u64)){36u64};return x;};
 func nested=func()->func()->const u64{return func()->const u64{val x<-make local typeof(f(1u64)){37u64};return x;};};
 observe(q());observe(nested()());
 observe(9011u64);
}
func mixed_query(f:func(u64)->const u64,tag:T)->func(u64)->const u64 where T{
 return func(x:u64)->const u64{val unused<-make local typeof(tag);return f(x);};
}
proc caught_query_context()->void{
 try{throw bound_error();}catch(e:const MissingReturnException){
  observe(9090u64);
  defer{{func q=func()->const u64{val x<-make local typeof(e.category){38u64};return x;};observe(q());}};
  observe(9091u64);
 };
}
{{
 func f=wrap(base);lexical_queries(f);
 func a=mixed_query(f,1);func b=mixed_query(f,true);
 observe(a(1u64));observe(b(2u64));
 observe(9030u64);
 try{query_apply(a,query_later());{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(a(3u64));
}};
caught_query_context();observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==14','checkpoints==15').replace('copy_failures==8 && record_failures','copy_failures==9 && record_failures')
runner.HARNESS=runner.HARNESS.replace('  if(value==9000)', '''  if(value==9090){assert(!query_record && query_records==1);query_records=0;query_record=true;return;}
  if(value==9091){assert(query_record && query_records==1);query_record=false;return;}
  if(value==9000)''')
runner.EXPECTED_VALUES+=['36','37','2','3','4','38']
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
 raise SystemExit(runner.main())
