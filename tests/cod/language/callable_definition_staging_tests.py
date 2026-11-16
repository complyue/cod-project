#!/usr/bin/env python3
"""Definition-only paths allocate no callable image; recursive coverage is finite."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from callable_value_staging_tests import inventory
from foundation_tests import ROOT, MODE, COD

DIRECT = '''
using symbol "std:io" @print;
symbol @bias:u64;
proc run()->void{
 proc unused()->u64{return @bias;};
 func leaf(x:u64)->u64{return x;};
 func direct[leaf](x:u64)->u64{return leaf(x);};
 val query<-make local typeof(unused()){7u64};
 behave @print(direct(query));
}
run();
'''
RECURSIVE = '''
using symbol "std:io" @print;
func one()->u64{return 1u64;}
func factory(source:func()->u64)->func(u64)->u64{
 func recur[source](n:u64)->u64{
  if(n==0u64){return source();}else{};
  func next=recur;return next(n-1u64);
 };return recur;
}
func value=factory(one);behave @print(value(DEPTHu64));
'''

def main():
    results=[]
    def record(name,passed,**details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-definition-staging-') as directory:
        root=Path(directory)
        for general in (False,True):
            mode='general' if general else 'default'
            source=root/'main.cod';ir=root/'main.ll'
            previous=None
            for case,text,expected in [('direct',DIRECT,'7\n'),('recursive',RECURSIVE.replace('DEPTH','2'),'1\n'),('deep',RECURSIVE.replace('DEPTH','200'),'1\n')]:
                source.write_text(text)
                command=[str(COD),'--run',str(source),'--emit-llvm',str(ir)]
                if general:command.append('--no-specialize')
                run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
                emitted=ir.read_text() if ir.exists() else ''
                record(mode+'/'+case,run.returncode==0 and run.stdout==expected and not run.stderr,
                       command=command,exit=run.returncode,stdout=run.stdout,stderr=run.stderr,ir_bytes=len(emitted))
                if case=='direct':
                    record(mode+'/no_definition_image','@cod_lang_callable_create' not in emitted)
                elif case=='recursive':previous=(inventory(emitted),len(emitted))
                else:record(mode+'/finite_recursive_coverage',inventory(emitted)==previous[0] and abs(len(emitted)-previous[1])<32,
                            bodies=list(inventory(emitted)),shallow_bytes=previous[1],deep_bytes=len(emitted))
    passed=sum(r['passed'] for r in results)
    print(f'Callable definition staging: {passed}/{len(results)} passed')
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                 Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),toolchain=MODE,
            passed=passed,total=len(results),results=results,executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}),indent=2)+'\n')
    return passed!=len(results)

if __name__=='__main__':
    raise SystemExit(main())
