#!/usr/bin/env python3
"""Composed operand evidence and callable view adapters retain native ownership."""
from pathlib import Path
import operand_evidence_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = runner.SOURCE.replace('return choose_mode(f,tag,x);', '''return choose_mode(f,{{
 func identity=func(v:T)->const T{return v;};
 fold(i in 0..1,current=identity(tag)){current}
}},x);''')
runner.SOURCE = runner.SOURCE.replace('choose_mode(f,{observe(9101u64);tag},', 'choose_mode(f,try{observe(9101u64);tag}catch(e){tag}finally{observe(9102u64);},')
runner.SOURCE = runner.SOURCE.replace('defer report_mode(f,tag,23u64);', 'defer report_mode(f,try{tag}finally{},23u64);')
runner.EXPECTED_VALUES.insert(runner.EXPECTED_VALUES.index('9100'), '9102')
runner.SOURCE += '''
using "std:exceptions" SystemException;
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func exception_forward(e:T)->const u64 where T{return e.category;}
func common(e:SystemException)->const u64{return exception_forward(e);}
proc common_proc(e:SystemException)->void{observe(exception_forward(e));}
{{
 func view:func(MissingReturnException)->const u64=common;
 observe(view(error()));
 proc selected:proc(MissingReturnException)->void=common_proc;
 proc retained=proc[selected](e:MissingReturnException)->void{selected(e);};
 defer retained(error());
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==8', 'checkpoints==9')
runner.EXPECTED_VALUES += ['5','5']
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
