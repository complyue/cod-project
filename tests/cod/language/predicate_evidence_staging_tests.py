#!/usr/bin/env python3
"""Expression predicates retain shared generic recursion without runtime state."""
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
func choose(ptr p:T in O,ptr q:T in O,fallback:T,n:u64)->const T where {T,owner O}{
 if(n!=0u64){return choose(p,q,fallback,n-1u64);}else{
  if(TEST){return p;}else{return fallback;};
 };
}
val x<-7,y<-8u64;
ptr absent:i64=null in here;
behave @print(choose(x,x,9,DEPTHu64));
behave @print(choose(y,y,0u64,DEPTHu64));
behave @print(choose(absent,x,9,DEPTHu64));
'''
PREDICATES = {
    'blocks': '{{{(p is not null)&&{q is not null}}}}',
    'negation': '!((p is null)||(q is null))',
    'guarded': 'try{!((p is null)||(q is null))}finally{}',
    'alternatives': 'if(q is null){p is not null}else{!(p is null)}',
    'stored': '{{val saved<-p is not null;saved}}',
    'copied': '{{val original<-p is not null;val saved<-original;original<-true;saved}}',
    'stored_alternatives': '{{val saved<-false;if(q is null){saved<-p is not null;}else{saved<-!(p is null);};saved}}',
}


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-predicate-evidence-') as directory:
        source, ir = Path(directory)/'main.cod', Path(directory)/'main.ll'
        for general in (False, True):
            mode = 'general' if general else 'default'
            for name, predicate in PREDICATES.items():
                baseline = None
                for depth, execute in ((3, True), (300, True), (3, False)):
                    source.write_text(SOURCE.replace('TEST', predicate).replace('DEPTH', str(depth)))
                    command = [str(COD), '--run' if execute else '--check', str(source), '--emit-llvm', str(ir)]
                    if general:
                        command.append('--no-specialize')
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                    text = ir.read_text() if run.returncode == 0 else ''
                    key = mode+'/'+name+'/'+(str(depth) if execute else 'check')
                    output = dict(exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
                    record(key, output == dict(exit=0, stdout='7\n8\n9\n' if execute else '', stderr=''), output=output)
                    methods = inventory(text)
                    if baseline is None:
                        baseline = methods
                        record(key+'/shared_body', sum('.choose.' in method and method.endswith('.general') for method in methods) == 1, methods=list(methods))
                        record(key+'/no_runtime_evidence', bool(text) and '@cod_lang_callable_create' not in text and '@cod_lang_callable_copy' not in text)
                    else:
                        record(key+'/stable_bodies', bool(methods) and methods == baseline, methods=list(methods))
    passed = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), toolchain=MODE,
            passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}), indent=2)+'\n')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
