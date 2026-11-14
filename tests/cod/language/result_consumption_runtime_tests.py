#!/usr/bin/env python3
"""Complete callable results survive context-selected reads, failures and cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func choose_result(tag:bool,ref f:func(u64)->const u64 in O)->(func(u64)->const u64) in O where {owner O}{return f;}
func choose_result(tag:i64,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {owner O}{return f;}
func copy_selected(tag:T,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {T,owner O}{return choose_result(tag,f);}
proc fail_selected(tag:T,ref f:func(u64)->const u64 in O)->void where {T,owner O}{
 observe(9030u64);
 try{apply(copy_selected(tag,f),{observe(9999u64);1u64});{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9040u64);
 try{apply(copy_selected(tag,f),missing());{}}catch(e:const MissingReturnException){observe(9041u64);};
}
proc discard_selected(tag:T,ref f:func(u64)->const u64 in O)->void where {T,owner O}{defer choose_result(tag,f);}
func repeat_selected(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{
 val sum<-0u64;for(i in 0u64..100u64){sum<-sum+choose_result(tag,f)(i);};return sum;
}
proc later_selected(tag:T,ref f:func(u64)->const u64 in O)->void where {T,owner O}{
 defer apply(copy_selected(tag,f),20u64);f<-wrap(other);
}
{{
 func f=wrap(base);func a=copy_selected(false,f);func b=copy_selected(0,f);f<-wrap(other);
 observe(a(1u64));observe(b(2u64));observe(f(3u64));
 fail_selected(false,f);fail_selected(0,f);
 discard_selected(false,f);discard_selected(0,f);observe(f(4u64));
 observe(repeat_selected(false,f));observe(repeat_selected(0,f));
}};
observe(9000u64);
{{
 func f=wrap(base);later_selected(false,f);f<-wrap(base);later_selected(0,f);
 observe(9050u64);
 try{later_selected(false,f);}catch(e:const OutOfMemory){observe(9051u64);};
 observe(9050u64);
 try{later_selected(0,f);}catch(e:const OutOfMemory){observe(9051u64);};
}};
observe(9000u64);
'''
runner.SOURCE += '''
func cleanup_result(tag:bool,ref f:func(u64)->const u64 in O)->(func(u64)->const u64) in O where {owner O}{defer{f<-wrap(other);};return f;}
func cleanup_result(tag:i64,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {owner O}{defer{f<-wrap(other);};return f;}
func copy_cleanup(tag:T,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {T,owner O}{return cleanup_result(tag,f);}
{{func f=wrap(base);func g=copy_cleanup(false,f);f<-base;observe(g(30u64));}};
observe(9000u64);
{{func f=wrap(base);func g=copy_cleanup(0,f);f<-base;observe(g(31u64));}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5', 'checkpoints==9').replace('copy_failures==3', 'copy_failures==5').replace('record_failures==1', 'record_failures==3')
runner.EXPECTED_VALUES += ['2','3','14','15','6050','6050','41','32']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
