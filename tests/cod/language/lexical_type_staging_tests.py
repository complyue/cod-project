#!/usr/bin/env python3
"""Lexical type context preserves shared methods and unused-context elision."""
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
func factory(x:T)->func(T)->const T where T{return func[](y:T)->const T{return y;};}
func hidden(x:T)->func()->const i64 where T{return func[]()->const i64{val y:T<-1u8;return 2;};}
func wrap(f:func(T)->const T)->func(T)->const T where T{return func[f](x:T)->const T{return f(x);};}
func recurse(f:func(T)->const T,x:T,n:u64)->const T where T{
  if(n==0u64){return f(x);}else{return recurse(f,x,n-1u64);};
}
func unused(x:T)->func()->const i64 where T{return func[]()->const i64{return 3;};}
behave @print(recurse(wrap(factory(1)),7,30u64));
behave @print(recurse(wrap(factory(1.0f64)),2.5f64,30u64));
behave @print(hidden(1)());behave @print(hidden(1.0f64)());
behave @print(unused(1)());behave @print(unused(1.0f64)());
'''


def main():
    results=[]
    def record(name,passed,**details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed:print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-lexical-type-staging-') as directory:
        scratch=Path(directory)
        for general in (False,True):
            mode='general' if general else 'default'
            source=scratch/'main.cod';source.write_text(SOURCE)
            ir=scratch/'main.ll';command=[str(COD),'--run',str(source),'--emit-llvm',str(ir)]
            if general:command.append('--no-specialize')
            run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            record(mode+'/native',run.returncode==0 and run.stdout=='7\n2.5\n2\n2\n3\n3\n' and not run.stderr,
                   command=command,exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
            text=ir.read_text() if ir.exists() else '';bodies=inventory(text)
            record(mode+'/shared_methods',len(bodies)==(9 if general else 25)
                   and sum(name.endswith('.general') for name in bodies)==8,bodies=list(bodies),ir_bytes=len(text.encode()))
            retained={name:args for name,args in bodies.items() if name.startswith(('cod.method.5.','cod.method.6.','cod.method.7.'))}
            record(mode+'/retained_type_evidence',len(retained)==(3 if general else 9)
                   and all('i32 ' in args for args in retained.values()),signatures=retained)
            unused={name:args for name,args in bodies.items() if name.startswith('cod.method.8.')}
            record(mode+'/unused_type_context',len(unused)==1 and all(name.endswith('.native') and not args for name,args in unused.items()),signatures=unused)
            source.write_text(SOURCE.replace('30u64','300u64'))
            deep_ir=scratch/'deep.ll';deep_command=[str(COD),'--run',str(source),'--emit-llvm',str(deep_ir)]
            if general:deep_command.append('--no-specialize')
            deep=subprocess.run(deep_command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            deep_text=deep_ir.read_text() if deep_ir.exists() else ''
            record(mode+'/runtime_depth_reuses_methods',deep.returncode==0 and deep.stdout==run.stdout and not deep.stderr
                   and inventory(deep_text)==bodies,command=deep_command,exit=deep.returncode,stdout=deep.stdout,
                   stderr=deep.stderr,ir_bytes=len(deep_text.encode()))
            source.write_text(SOURCE.replace('recurse(wrap(factory(1)),7,30u64)', 'factory(1)(7)')
                              .replace('recurse(wrap(factory(1.0f64)),2.5f64,30u64)', 'factory(1.0f64)(2.5f64)'))
            plain=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            plain_text=ir.read_text() if ir.exists() else ''
            record(mode+'/type_only_context_needs_no_image',plain.returncode==0 and plain.stdout==run.stdout and not plain.stderr
                   and bool(plain_text) and not any(name in plain_text for name in
                       ('@cod_lang_callable_create','@cod_lang_callable_copy','@cod_lang_callable_free','@cod_lang_defer_allocate')),
                   command=command,exit=plain.returncode,stdout=plain.stdout,stderr=plain.stderr,ir_bytes=len(plain_text.encode()))
    passed=sum(item['passed'] for item in results)
    print(f'Lexical type staging: {passed}/{len(results)} passed')
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod'/name for name in ('main.cc','lang_checked.hh','lang_completion.hh','lang_diagnostics.hh',
                 'lang_frontend.hh','lang_llvm.hh','lang_syntax.hh','lang_exceptions.hh','lang_native_runtime.hh')]
        sources += [Path(__file__),Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),toolchain=MODE,
            passed=passed,total=len(results),results=results,executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}),indent=2)+'\n')
    return passed!=len(results)

if __name__=='__main__':
    raise SystemExit(main())
