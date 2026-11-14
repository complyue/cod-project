#!/usr/bin/env python3
"""Operand completion releases prepared images and never registers unfinished work."""
from pathlib import Path
import captured_callable_runtime_tests as fixture
runner=fixture.runner
runner.SOURCE += '''
proc finish_operand(f:func(u64)->const u64)->const u64{
 func invoke=apply;defer observe(20u64);
 return invoke(f,{{defer observe(21u64);return f(1u64);}});
}
proc failing_operand(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 func invoke=apply;return invoke(f,{{observe(9999u64);return 7u64;}});
}
proc defer_operand(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 func invoke=apply;defer observe(23u64);
 defer invoke(f,{{return f(2u64);}});return 0u64;
}
proc abort_registration(ref f:func(u64)->const u64 in O)->const u64 where {owner O}{
 func invoke=apply;defer invoke(f,{{return f(3u64);}});return 0u64;
}
func return_image(f:func(u64)->const u64)->func(u64)->const u64{
 func invoke=apply;invoke(f,{{return wrap(f);}});return base;
}
func logical_image(f:func(u64)->const u64,flag:bool)->func(u64)->const u64{
 flag&&{{return wrap(f);}};return f;
}
{{
 func f=wrap(base);observe(finish_operand(f));observe(defer_operand(f));
 observe(9030u64);
 try{failing_operand(f);{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9050u64);observe(abort_registration(f));observe(9060u64);
 func g=return_image(f);observe(g(1u64));
 func a=logical_image(f,true);func b=logical_image(f,false);observe(a(2u64));observe(b(2u64));
}};
observe(9000u64);
'''
runner.SOURCE += '''
using "std:exceptions" NullPointerException;
func pick_image(tag:bool,ptr f:func(u64)->const u64 in O)->ptr (func(u64)->const u64) in O where {owner O}{return f;}
func pick_image(tag:i64,ptr f:func(u64)->const u64 in O)->func(u64)->const u64 where {owner O}{return wrap(base);}
proc observe_application(f:func(u64)->const u64,x:u64)->void{observe(f(x));}
proc schedule_image(tag:T,ref f:func(u64)->const u64 in O)->void throws NullPointerException where {T,owner O}{
 ptr p=f;p=null;proc invoke=observe_application;
 defer invoke({{pick_image(tag,p)}},{observe(9u64);5u64});
}
{{func f=wrap(base);try{schedule_image(false,f);}catch(e:const NullPointerException){observe(8u64);};try{schedule_image(0,f);}catch(e:const NullPointerException){};}};
observe(9000u64);
'''
runner.HARNESS=runner.HARNESS.replace('checkpoints==5','checkpoints==7').replace('copy_failures==3','copy_failures==4').replace('  assert(value!=9999);', '  if(value==9060){assert(deny_record && record_failures==1 && images.size()==live_baseline);deny_record=false;return;}\n  assert(value!=9999);')
runner.EXPECTED_VALUES += ['21','20','2','23','3','4','3','4','3','8','9','6']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
