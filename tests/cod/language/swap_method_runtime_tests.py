#!/usr/bin/env python3
"""Destination dispatch retains complete values and exact failure cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
proc (<->)(x:bool in O,y:bool in P)->void where {owner O,owner P}{
 intrinsic (<->)(x,y);observe(81u64);
}
func (<->)(x:func(u64)->const u64 in O,y:func(u64)->const u64 in P)->void where {owner O,owner P}{
 func saved=x;x<-y;y<-saved;
}
proc dispatch(ref x:T in O,ref y:T in P,n:u64)->void where {T,owner O,owner P}{
 if(n==0u64){return;}else{x<->y;dispatch(x,y,n-1u64);};
}
proc show_bool(x:bool)->void{observe(if(x){1u64}else{0u64});}
{{
 val a<-false,b<-true,x<-1u64,y<-2u64;
 dispatch(a,b,3u64);show_bool(a);show_bool(b);
 dispatch(x,y,2u64);observe(x);observe(y);
 a<-false;b<-true;val c<-false;ptr p=a,q=b;
 {{defer (<->)(p,q);p=c;}};show_bool(a);show_bool(b);show_bool(p);
 a<-false;b<-true;observe(9050u64);
 try{ {{defer (<->)(a,b);}};}catch(e:const OutOfMemory){observe(9051u64);};
 show_bool(a);show_bool(b);
 func f=wrap(base);func g=wrap(other);f<->g;observe(f(1u64));observe(g(1u64));
 observe(9030u64);try{f<->g;}catch(e:const OutOfMemory){observe(9031u64);};
 observe(f(2u64));observe(g(2u64));
 observe(9060u64);try{f<->g;}catch(e:const OutOfMemory){observe(9061u64);};
 observe(f(2u64));observe(g(2u64));
 observe(9062u64);try{f<->g;}catch(e:const OutOfMemory){observe(9061u64);};
 observe(f(2u64));observe(g(2u64));
 {{defer (<->)(f,g);f<-wrap(wrap(base));}};observe(f(3u64));observe(g(3u64));
 observe(9010u64);dispatch(a,b,2u64);observe(9011u64);show_bool(a);
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==6').replace(
    'copy_failures==3', 'copy_failures==6').replace('record_failures==1', 'record_failures==2')
runner.HARNESS = runner.HARNESS.replace('static void track', 'static std::uint64_t copy_budget=0;\nstatic void track').replace(
    'if(deny_copy){', 'if(deny_copy || (copy_budget && !--copy_budget)){').replace(
    'assert(value!=9999);', '''
  if(value==9060 || value==9062){assert(!copy_budget);copy_budget=value==9060?2:3;live_baseline=images.size();return;}
  if(value==9061){assert(!copy_budget && images.size()==live_baseline);return;}
  assert(value!=9999);''')
runner.EXPECTED_VALUES += ['81', '81', '81', '1', '0', '1', '2',
                          '81', '1', '0', '0', '0', '1',
                          '12', '2', '13', '3', '13', '3', '3', '3', '4', '5', '81', '81', '0']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
