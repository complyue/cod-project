#!/usr/bin/env python3
"""Predicate and call-presence proofs retain shared generic native bodies."""
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
    'identity': '(p is q)&&(q is not null)',
    'stored_identity': '{{val same<-p is q;same&&q is not null}}',
    'stored_null_view': '{{ptr empty:T=null in O;p is not empty}}',
}
SOURCES = {name: SOURCE.replace('TEST', predicate) for name, predicate in PREDICATES.items()}
CALL_SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" NullPointerException;
func checked(ptr p:T in O,n:u64)->void throws NullPointerException where {T,owner O}{
 if(n!=0u64){checked(p,n-1u64);}else{p;{}};
}
func choose(ptr p:T in O,fallback:T,n:u64)->const T where {T,owner O}{
 ptr saved=p;
 try{ INVOKE }catch(e:const NullPointerException){return fallback;};
 return saved;
}
val x<-7,y<-8u64;
ptr absent:i64=null in here;
behave @print(choose(x,9,DEPTHu64));
behave @print(choose(y,0u64,DEPTHu64));
behave @print(choose(absent,9,DEPTHu64));
'''
SOURCES['call_presence'] = CALL_SOURCE.replace('INVOKE', 'checked(p,n);')
SOURCES['call_presence_prepared'] = CALL_SOURCE.replace('INVOKE', '{{defer checked(p,n);p=null;}};')
PREDICATE_CALL_SOURCE = '''
using symbol "std:io" @print;
func checked(ptr p:T in O,n:u64)->bool where {T,owner O}{
 if(n!=0u64){return checked(p,n-1u64);}else{return p is not null;};
}
func choose(ptr p:T in O,fallback:T,n:u64)->const T where {T,owner O}{
 ptr saved=p;
 val ok<-checked(p,n);
 p=null;
 if(ok){return saved;}else{return fallback;};
}
val x<-7,y<-8u64;
ptr absent:i64=null in here;
behave @print(choose(x,9,DEPTHu64));
behave @print(choose(y,0u64,DEPTHu64));
behave @print(choose(absent,9,DEPTHu64));
'''
SOURCES['call_predicate'] = PREDICATE_CALL_SOURCE
SOURCES['call_predicate_returns'] = PREDICATE_CALL_SOURCE.replace(
    'return p is not null;', 'if(p is null){return false;}else{return true;};')
BOOLEAN_INPUT_SOURCE = PREDICATE_CALL_SOURCE.replace(
    'func checked(ptr p:T in O,n:u64)->bool where {T,owner O}{\n if(n!=0u64){return checked(p,n-1u64);}else{return p is not null;};\n}',
    'func checked(flag:T,n:u64)->const T where T{\n if(n!=0u64){return checked(flag,n-1u64);}else{return flag;};\n}')
SOURCES['boolean_input'] = BOOLEAN_INPUT_SOURCE.replace('checked(p,n)', 'checked(p is not null,n)')
SOURCES['boolean_borrowed_input'] = BOOLEAN_INPUT_SOURCE.replace(
    'func checked(flag:T,n:u64)->const T where T',
    'func checked(ref flag:const T in B,n:u64)->const T where {T,owner B}').replace(
    'val ok<-checked(p,n);', 'val input<-p is not null;val ok<-checked(input,n);')
SOURCES['boolean_borrowed_result'] = SOURCES['boolean_borrowed_input'].replace(
    '->const T where {T,owner B}', '->ref const T in B where {T,owner B}')
BOOLEAN_OUTPUT_SOURCE = PREDICATE_CALL_SOURCE.replace(
    'func checked(ptr p:T in O,n:u64)->bool where {T,owner O}{\n if(n!=0u64){return checked(p,n-1u64);}else{return p is not null;};\n}',
    'func checked(ref out:bool in B,ptr p:T in O,n:u64)->void where {T,owner B,owner O}{\n if(n!=0u64){checked(out,p,n-1u64);}else{out<-p is not null;{}};\n}').replace(
    'val ok<-checked(p,n);', 'val ok<-false;INVOKE;')
SOURCES['boolean_output'] = BOOLEAN_OUTPUT_SOURCE.replace('INVOKE', 'checked(ok,p,n)')
SOURCES['boolean_output_prepared'] = BOOLEAN_OUTPUT_SOURCE.replace('INVOKE', '{{defer checked(ok,p,n);}}')
SOURCES['boolean_output_block'] = BOOLEAN_OUTPUT_SOURCE.replace('INVOKE', '{{defer{checked(ok,p,n);};}}')
SOURCES['boolean_output_alternatives'] = BOOLEAN_OUTPUT_SOURCE.replace('val ok<-false;', 'val ok<-p is not null;').replace(
    'out<-p is not null;{}', 'if(n==0u64){out<-p is not null;{}}else{};').replace('INVOKE', 'checked(ok,p,n)')
SOURCES['boolean_selected_output'] = BOOLEAN_OUTPUT_SOURCE.replace('val ok<-false;INVOKE;',
    'val ok<-false,other<-true;ref output=other;{{defer{checked(output,p,n);};output=ok;}};')
SOURCES['boolean_selected_input'] = BOOLEAN_OUTPUT_SOURCE.replace('val ok<-false;INVOKE;',
    'val ok<-false,other<-true,source<-false;ref input=other;{{defer{ok<-input;};defer{checked(source,p,n);};input=source;}};')


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
            for name, program in SOURCES.items():
                baseline = None
                for depth, execute in ((3, True), (300, True), (3, False)):
                    source.write_text(program.replace('DEPTH', str(depth)))
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
                        if name.startswith(('call_presence', 'call_predicate', 'boolean_')):
                            record(key+'/shared_helper', sum('.checked.' in method and method.endswith('.general') for method in methods) == 1, methods=list(methods))
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
