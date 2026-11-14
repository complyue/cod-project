#!/usr/bin/env python3
"""Expected result contexts retain shared native bodies and fixed-contract equivalence."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

IO = 'using symbol "std:io" @print;'
SHARED = IO + '''
func factory(x:T)->func()->const T where T{return func(){return 1u32;};}
func f=factory(1u64);func g=factory(1f64);
val total<-0u64;val real_total<-0f64;
for(i in 0u64..30u64){total<-total+f();real_total<-real_total+g();};
behave @print(total);behave @print(real_total==30f64);
'''
FIXED = IO + 'func f:func()->u64=func()->u64{return 7u32;};behave @print(f());'
BORROWED = IO + 'func anchor(ref x:i64 in A)->ref i64 in A where {owner A}{return x;}func f=anchor;f<-func(ref x:i64 in B)->ref i64 in B where {owner B}{defer{x<-8;};return x;};val x<-1;behave @print(f(x));'


def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-contextual-result-staging-') as directory:
        root = Path(directory)
        source = root / 'main.cod'
        def run(text, general):
            source.write_text(text)
            ir = root / 'main.ll'
            command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr,
                        ir=ir.read_text() if completed.returncode == 0 and ir.exists() else '')
        for general in (False, True):
            mode = 'general' if general else 'default'
            for name, explicit, annotation, expected in (
                ('fixed', FIXED, '->u64{', '7\n'),
                ('borrowed_cleanup', BORROWED, '->ref i64 in B where', '8\n'),
            ):
                replacement = ' ' * (len(annotation) - 1) + '{' if name == 'fixed' else ' ' * len('->ref i64 in B ') + 'where'
                inferred = explicit.replace(annotation, replacement)
                left, right = run(explicit, general), run(inferred, general)
                record(mode + '/' + name + '_identical_ir', left == right and left['exit'] == 0
                       and left['stdout'] == expected and not left['stderr'] and bool(left['ir']),
                       explicit={k:v for k,v in left.items() if k!='ir'}, contextual={k:v for k,v in right.items() if k!='ir'},
                       identical_ir=left['ir'] == right['ir'])
            base = run(SHARED, general)
            bodies = inventory(base['ir'])
            anonymous = {name: body for name, body in bodies.items() if 'cod.method.1..' in name}
            record(mode + '/shared_definition_contexts', base['exit'] == 0 and base['stdout'] == '30\ntrue\n'
                   and not base['stderr'] and len(anonymous) == (1 if general else 3)
                   and sum(name.endswith('.general') for name in anonymous) == 1,
                   output={k:v for k,v in base.items() if k!='ir'}, anonymous=list(anonymous))
            record(mode + '/no_context_image_allocation', bool(base['ir']) and not any(name in base['ir'] for name in
                   ('@cod_lang_callable_create', '@cod_lang_callable_copy', '@cod_lang_callable_free', '@cod_lang_defer_allocate')))
            for label, text, expected in (
                ('runtime_depth', SHARED.replace('30u64','300u64').replace('30f64','300f64'), '300\ntrue\n'),
                ('demand_order', SHARED.replace('func f=factory(1u64);func g=factory(1f64);',
                                              'func g=factory(1f64);func f=factory(1u64);'), '30\ntrue\n'),
            ):
                other = run(text, general)
                record(mode + '/' + label, other['exit'] == 0 and other['stdout'] == expected and not other['stderr']
                       and set(inventory(other['ir'])) == set(bodies),
                       output={k:v for k,v in other.items() if k!='ir'}, bodies=list(inventory(other['ir'])))
    count = sum(item['passed'] for item in results)
    print(f'Contextual result staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}), indent=2)+'\n')
    return count != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
