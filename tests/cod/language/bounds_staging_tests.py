#!/usr/bin/env python3
"""Bounds policy survives shared code, imports and a separately built runtime."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from language_test_support import runtime_archive
from callable_value_staging_tests import inventory
from bounds_tests import TYPES, SEED, LINK

LIBRARY = TYPES+SEED+LINK+'''
func at(e:T,i:u64,n:u64)->const Exception where T{
 if(n==0u64){return cause_at(e,i);}else{return at(e,i,n-1u64);};
}
export at;export seed;export link;
'''
SOURCE = TYPES+'''
using symbol "std:io" @print;
using "lib.cod" at;using "lib.cod" seed;using "lib.cod" link;
val e<-seed();val history<-link(e,e);val common:Exception<-history;
behave @print(at(history,0u64,DEPTHu64).category);
behave @print(at(common,0u64,DEPTHu64).category);
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-bounds-staging-') as directory:
        root = Path(directory); (root/'lib.cod').write_text(LIBRARY)
        source, ir_path = root/'main.cod',root/'main.ll'
        archive = runtime_archive(ROOT / MODE)
        runtime = archive.read_bytes()
        for general in (False,True):
            for checked in (True,False):
                mode = ('general' if general else 'default')+('/checked' if checked else '/unchecked')
                previous = None
                for depth in (30,300):
                    source.write_text(SOURCE.replace('DEPTH',str(depth)))
                    command = [str(COD),'--run',str(source),'--emit-llvm',str(ir_path)]
                    if general: command.append('--no-specialize')
                    if not checked: command.append('--no-bounds-checks')
                    run = subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
                    label = mode+'/'+str(depth)
                    record(label,run.returncode==0 and run.stdout=='5\n5\n' and '[-Wmissing-return]' in run.stderr,
                           exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
                    ir = ir_path.read_text() if run.returncode==0 else ''
                    calls = re.findall(r'call i64 @cod_lang_exception_cause\([^\n]+, i64 ([01])\)',ir)
                    record(label+'/policy',bool(calls) and set(calls)=={str(int(checked))} and ('bounds.failure' in ir)==checked)
                    methods = inventory(ir)
                    record(label+'/shared_method',sum('.at.' in method and method.endswith('.general') for method in methods)==1)
                    if previous is not None: record(label+'/stable_inventory',bool(methods) and methods==previous)
                    previous = methods
                    record(label+'/common_runtime',archive.read_bytes()==runtime)
        run = subprocess.run([str(COD),'--no-bounds-checks'],cwd=ROOT,capture_output=True,text=True,timeout=30)
        record('option_requires_source',run.returncode==1 and 'require --run or --check' in run.stderr)
        run = subprocess.run([str(COD),'--help'],cwd=ROOT,capture_output=True,text=True,timeout=30)
        record('option_help',run.returncode==0 and '--no-bounds-checks' in run.stdout)
    count = sum(item['passed'] for item in results)
    print(f'Bounds staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [Path(__file__).with_name("language_test_support.py"), ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py'),Path(__file__).with_name('bounds_tests.py')]
        digest = lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE,passed=count,total=len(results),results=results,executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}),indent=2)+'\n')
    return count!=len(results)

if __name__ == '__main__':
    raise SystemExit(main())
