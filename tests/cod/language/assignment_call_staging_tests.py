#!/usr/bin/env python3
"""Assignment spellings share native bodies and preserve recursive code size."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

SOURCE = '''
using symbol "std:io" @print;
func step(ref target:T in O,next:T,n:u64)->const T where {T,owner O}{
 if(n==0u64){
  WRITE;
 }else{step(target,next,n-1u64);};
 return target;
}
val a<-1,b<-2u64,c<-9;
ptr selected=a;
REBIND;
behave @print(step(a,7,DEPTHu64));
behave @print(step(b,8u64,DEPTHu64));
behave @print(selected);
'''
SPELLINGS = {
    'infix': ('target<-next', 'selected=c'),
    'call': ('(<-)(target,next)', '(=)(selected,c)'),
    'intrinsic': ('intrinsic (<-)(target,next)', 'intrinsic (=)(selected,c)'),
}
EXTRA_SOURCES = []

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed: print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-assignment-call-') as directory:
        source, ir_path = Path(directory)/'main.cod', Path(directory)/'main.ll'
        for general in (False, True):
            mode = 'general' if general else 'default'
            baseline = None
            for name, (write, rebind) in SPELLINGS.items():
                bodies = None
                for depth in (30, 300):
                    source.write_text(SOURCE.replace('WRITE',write).replace('REBIND',rebind).replace('DEPTH',str(depth)))
                    command = [str(COD),'--run',str(source),'--emit-llvm',str(ir_path)]
                    if general: command.append('--no-specialize')
                    run = subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
                    ir = ir_path.read_text() if run.returncode == 0 and ir_path.exists() else ''
                    output = dict(exit=run.returncode,stdout=run.stdout,stderr=run.stderr)
                    record(mode+'/'+name+'/'+str(depth),output==dict(exit=0,stdout='7\n8\n9\n',stderr=''),output=output)
                    methods = set(inventory(ir))
                    if depth == 30:
                        bodies = methods
                        if baseline is None: baseline = ir
                        else: record(mode+'/'+name+'/identical_ir',bool(ir) and ir==baseline)
                        record(mode+'/'+name+'/shared_body',sum('.step.' in method and method.endswith('.general') for method in methods)==1,methods=sorted(methods))
                        record(mode+'/'+name+'/no_images','cod_lang_callable_create' not in ir and 'cod_lang_callable_copy' not in ir)
                    else: record(mode+'/'+name+'/stable_inventory',bool(methods) and methods==bodies,methods=sorted(methods))
    count = sum(item['passed'] for item in results)
    print(f'Assignment call staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py'),*EXTRA_SOURCES]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE,passed=count,total=len(results),results=results,executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}),indent=2)+'\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
