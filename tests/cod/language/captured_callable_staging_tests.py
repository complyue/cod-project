#!/usr/bin/env python3
"""Runtime environment growth retains finite method and type obligations."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from callable_value_staging_tests import inventory
from foundation_tests import ROOT, MODE, COD

SOURCE = '''
using symbol "std:io" @print;
func base(x:i64)->const i64{return x;}
func wrap(f:func(i64)->const i64)->func(i64)->const i64{return func[f](x:i64)->const i64{return f(x)+1;};}
func grow(f:func(i64)->const i64,n:i64)->func(i64)->const i64{
  if(n==0){return f;}else{return grow(wrap(f),n-1);};
}
func number()->const i64{return 1;}
func truth()->const bool{return true;}
func inspect(f:F)->func()->const bool where F{return func[f]()->const bool{f();return true;};}
behave @print(grow(base,30)(2));
behave @print(inspect(number)());behave @print(inspect(truth)());
'''

def main():
    results=[]
    def record(name,passed,**details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-captured-staging-') as directory:
        root=Path(directory)
        for general in (False,True):
            mode='general' if general else 'default'
            source=root/'main.cod';source.write_text(SOURCE)
            ir=root/'main.ll'
            command=[str(COD),'--run',str(source),'--emit-llvm',str(ir)]
            if general:command.append('--no-specialize')
            run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            record(mode+'/native',run.returncode==0 and run.stdout=='32\ntrue\ntrue\n' and not run.stderr,
                   command=command,exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
            text=ir.read_text() if ir.exists() else ''
            bodies=inventory(text)
            record(mode+'/shared_methods',len(bodies)==(8 if general else 13) and sum(x.endswith('.general') for x in bodies)==3,
                   bodies=list(bodies),ir_bytes=len(text.encode()))
            nested=[name for name in bodies if name.startswith('cod.method.7.')]
            record(mode+'/capture_type_instances',len(nested)==(1 if general else 3),bodies=nested)
            record(mode+'/image_services',all('@'+name in text for name in ('cod_lang_callable_create','cod_lang_callable_copy','cod_lang_callable_free')),
                   ir_bytes=len(text.encode()))
            source.write_text(SOURCE.replace('base,30','base,300'))
            command=[str(COD),'--check',str(source),'--emit-llvm',str(ir)]
            if general:command.append('--no-specialize')
            deeper=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            deep=ir.read_text() if ir.exists() else ''
            record(mode+'/runtime_growth_does_not_expand_methods',deeper.returncode==0 and not deeper.stdout and not deeper.stderr
                   and inventory(deep)==bodies and abs(len(deep)-len(text))<20,
                   command=command,exit=deeper.returncode,stdout=deeper.stdout,stderr=deeper.stderr,ir_bytes=len(deep.encode()))
    passed=sum(x['passed'] for x in results)
    print(f'Captured callable staging: {passed}/{len(results)} passed')
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod'/name for name in ('main.cc','lang_checked.hh','lang_completion.hh','lang_diagnostics.hh','lang_frontend.hh',
                 'lang_llvm.hh','lang_syntax.hh','lang_exceptions.hh','lang_native_runtime.hh')]
        sources += [Path(__file__),Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),toolchain=MODE,
            passed=passed,total=len(results),results=results,executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}),indent=2)+'\n')
    return passed!=len(results)

if __name__=='__main__':
    raise SystemExit(main())
