#!/usr/bin/env python3
"""Context-dependent local callable storage retains complete ownership and cleanup."""
from pathlib import Path
from native_test_support import run_native
import result_consumption_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('return choose_result(tag,f);', 'ptr local=choose_result(tag,f);func marker=base;return local;'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('for(i in 0u64..100u64){sum<-sum+choose_result(tag,f)(i);}', 'for(i in 0u64..100u64){{ptr local=choose_result(tag,f);func marker=base;sum<-sum+local(i);}}'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer apply(copy_selected(tag,f),20u64);', 'ptr local=choose_result(tag,f);defer apply(local,20u64);'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func choose_storage(tag:bool,ref f:func(u64)->const u64 in O)->(func(u64)->const u64) in O where {owner O}{return f;}
func choose_storage(tag:i64,ref f:func(u64)->const u64 in O)->func(u64)->const u64 where {owner O}{return f;}
proc replace_local(tag:T,ref f:func(u64)->const u64 in O)->void where {T,owner O}{
 ptr p=choose_storage(tag,f);func saved=wrap(base);p<-wrap(other);observe(p(1u64));observe(f(1u64));
 observe(9030u64);
 try{p<-wrap(saved);{}}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(p(2u64));
}
{{func f=wrap(base);replace_local(false,f);}};
observe(9000u64);
{{func f=wrap(base);replace_local(0,f);}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func stable_storage(tag:T,ref f:func(u64)->const u64 in O)->const u64 where {T,owner O}{
 ptr local=choose_storage(tag,f);val sum<-0u64;
 for(i in 0u64..10000u64){sum<-sum+local(i);};return sum;
}
{{func f=wrap(base);observe(9010u64);observe(stable_storage(false,f));observe(9011u64);observe(stable_storage(0,f));}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==9', 'checkpoints==12').replace('copy_failures==5','copy_failures==7'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['12','12','13','12','2','13','50005000','50005000'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
