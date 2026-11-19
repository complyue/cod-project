#!/usr/bin/env python3
"""Composed operand evidence and callable view adapters retain native ownership."""
from pathlib import Path
from native_test_support import run_native
import operand_evidence_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('return choose_mode(f,tag,x);', '''return choose_mode(f,{{
 func identity=func(v:T)->const T{return v;};
 fold(i in 0..1,current=identity(tag)){current}
}},x);'''))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('choose_mode(f,{observe(9101u64);tag},', 'choose_mode(f,try{observe(9101u64);tag}catch(e){tag}finally{observe(9102u64);},'))
FIXTURE = FIXTURE.updated(source=FIXTURE.source.replace('defer report_mode(f,tag,23u64);', 'defer report_mode(f,try{tag}finally{},23u64);'))
FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values[:FIXTURE.expected_values.index('9100')], '9102', *FIXTURE.expected_values[FIXTURE.expected_values.index('9100'):]))
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
using "std:exceptions" Exception;
func error()->const MissingReturnError{try{missing():void;}catch(e:const MissingReturnError){return e;};}
func exception_forward(e:T)->const u64 where T{return e.category;}
func common(e:Exception)->const u64{return exception_forward(e);}
proc common_proc(e:Exception)->void{observe(exception_forward(e));}
{{
 func view:func(MissingReturnError)->const u64=common;
 observe(view(error()));
 proc selected:proc(MissingReturnError)->void=common_proc;
 proc retained=proc[selected](e:MissingReturnError)->void{selected(e);};
 defer retained(error());
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==8', 'checkpoints==9'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5'])
FIXTURE = FIXTURE.updated(expected_shadows=['retained'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
