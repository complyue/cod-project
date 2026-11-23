#!/usr/bin/env python3
"""Eager aliases share native code across source spellings and recursive depth."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

OPS = 'func (-)(a:bool,b:bool)->bool{return a&&b;}export (-);'
SOURCE = '''
using symbol "std:io" @print;
using "./ops.cod" (-);using "./ops.cod" (-) as calculate;using "./ops.cod" (-) as (+);
func step(value:T,next:T,n:u64)->const T where T{
 if(n==0u64){
  WRITE
 }else{return step(value,next,n-1u64);};
}
behave @print(step(9,2,DEPTHu64));behave @print(step(10u64,2u64,DEPTHu64));
behave @print(step(false,true,DEPTHu64));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-eager-alias-') as directory:
        root = Path(directory); (root/'ops.cod').write_text(OPS)
        source, ir_path = root/'main.cod', root/'main.ll'
        for general in (False,True):
            for deferred in (False,True):
                mode = ('general' if general else 'default')+('/deferred' if deferred else '/direct')
                baseline = None
                for name, call in (('original','(-)(value,next)'),('named','calculate(value,next)'),('operator','(+)(value,next)')):
                    # Operator tokens start inside parentheses; align diagnostic locations too.
                    if name == 'named': call = ' '+call
                    write = '{{defer '+call+';}};return value;' if deferred else 'return '+call+';'
                    bodies = None
                    for depth in (30,300):
                        source.write_text(SOURCE.replace('WRITE',write).replace('DEPTH',str(depth)))
                        command = [str(COD),'--run',str(source),'--emit-llvm',str(ir_path)]
                        if general: command.append('--no-specialize')
                        run = subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
                        ir = ir_path.read_text() if run.returncode == 0 else ''
                        output = dict(exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
                        label = mode+'/'+name+'/'+str(depth)
                        expected = '9\n10\nfalse\n' if deferred else '7\n8\nfalse\n'
                        record(label,output==dict(exit=0,stdout=expected,stderr=''),output=output)
                        methods = inventory(ir)
                        if depth == 30:
                            bodies = methods
                            if baseline is None: baseline = ir
                            else: record(label+'/identical_ir',bool(ir) and ir==baseline)
                            record(label+'/shared_step',sum('.step.' in method and method.endswith('.general') for method in methods)==1,methods=list(methods))
                            record(label+'/no_callable_images',bool(ir) and 'cod_lang_callable_create' not in ir and 'cod_lang_callable_copy' not in ir)
                        else: record(label+'/stable_inventory',bool(methods) and methods==bodies,methods=list(methods))
    count = sum(item['passed'] for item in results)
    print(f'Eager alias staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [Path(__file__).with_name("language_test_support.py"), ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE,passed=count,total=len(results),results=results,executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}),indent=2)+'\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
