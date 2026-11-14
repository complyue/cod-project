#!/usr/bin/env python3
"""Structural callable arguments retain finite shared native implementations."""
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
using "std:exceptions" MissingReturnException;
func first(x:i64)->const i64{return x+1;}
func second(x:i64)->const i64{return x+2;}
func truth(x:bool)->const bool{return x;}
func wrap(f:func(i64)->const i64)->func(i64)->const i64{
  return func[f](x:i64)->const i64{return f(x)+1;};
}
func relay(f:func(T)->const T,x:T,n:u64)->const T where T{
  if(n==0u64){return f(x);}else{return relay(f,x,n-1u64);};
}
func empty()->const void{}
func number()->const i64{return 1;}
func finish(f:func()->const T)->const T where T{f();}
behave @print(relay(first,10,30u64));behave @print(relay(second,10,30u64));
behave @print(relay(wrap(first),10,30u64));behave @print(relay(truth,true,30u64));
finish(empty);try{finish(number);{}}catch(e:const MissingReturnException){behave @print(99);};
'''


def main():
    results=[]
    def record(name,passed,**details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-generic-callable-staging-') as directory:
        root=Path(directory)
        for general in (False,True):
            mode='general' if general else 'default'
            source=root/'main.cod';source.write_text(SOURCE)
            ir=root/'main.ll'
            command=[str(COD),'--run',str(source),'--emit-llvm',str(ir)]
            if general:command.append('--no-specialize')
            run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            warning='callable may finish without an explicit return'
            record(mode+'/native',run.returncode==0 and run.stdout=='11\n12\n12\ntrue\n99\n'
                   and warning in run.stderr and 'error:' not in run.stderr,
                   command=command,exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
            text=ir.read_text() if ir.exists() else ''
            bodies=inventory(text)
            record(mode+'/shared_methods',len(bodies)==(9 if general else 14)
                   and sum(x.endswith('.general') for x in bodies)==3,
                   bodies=list(bodies),ir_bytes=len(text.encode()))
            relays=[name for name in bodies if '.relay.' in name]
            record(mode+'/identity_independent_contexts',len(relays)==(1 if general else 3)
                   and 'call %cod.outcome @cod.method.4.relay.general' in text,
                   bodies=relays)
            finishes=[name for name in bodies if '.finish.' in name]
            record(mode+'/void_and_value_contexts',len(finishes)==(1 if general else 3),bodies=finishes)
            source.write_text(SOURCE.replace('30u64','300u64'))
            deep_ir=root/'deep.ll';deep_command=[str(COD),'--run',str(source),'--emit-llvm',str(deep_ir)]
            if general:deep_command.append('--no-specialize')
            deep=subprocess.run(deep_command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            deep_text=deep_ir.read_text() if deep_ir.exists() else ''
            record(mode+'/runtime_depth_reuses_methods',deep.returncode==0 and deep.stdout==run.stdout
                   and warning in deep.stderr and 'error:' not in deep.stderr and inventory(deep_text)==bodies,
                   command=deep_command,exit=deep.returncode,stdout=deep.stdout,stderr=deep.stderr,
                   ir_bytes=len(deep_text.encode()))
    passed=sum(item['passed'] for item in results)
    print(f'Generic callable staging: {passed}/{len(results)} passed')
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod'/name for name in ('main.cc','lang_checked.hh','lang_completion.hh','lang_diagnostics.hh',
                 'lang_frontend.hh','lang_llvm.hh','lang_syntax.hh','lang_exceptions.hh','lang_native_runtime.hh')]
        sources += [Path(__file__),Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE,passed=passed,total=len(results),results=results,
            executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}),indent=2)+'\n')
    return passed!=len(results)

if __name__=='__main__':
    raise SystemExit(main())
