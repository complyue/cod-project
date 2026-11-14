#!/usr/bin/env python3
"""Callable identities share checked method bodies and caller result storage."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile

from foundation_tests import ROOT, MODE, COD

SOURCE = '''
using symbol "std:io" @print;
func id(x:T)->const T where T{return x;}
func alternate(x:T)->const T where T{return x;}
func maker(sample:T)->func(T)->const T where T{return if(true){id}else{alternate};}
func relay(f:F,x:T,n:u64)->const T where {F,T}{
  if(n==0u64){return f(x);}else{return relay(f,x,n-1u64);};
}
func first=maker(1);func second:func(i64)->const i64=alternate;func third=maker(false);
behave @print(relay(first,60,3u64));behave @print(relay(second,61,300u64));behave @print(relay(third,true,20u64));
'''

def inventory(ir):
    return {quoted or plain: args for quoted, plain, args in re.findall(
        r'^define internal %cod.outcome @(?:"([^"]+)"|([^ (]+))\((.*?)\) \{', ir, re.M)}

def main():
    results = []
    def record(name, passed, **detail):
        results.append(dict(name=name, passed=passed, **detail))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(detail, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-callable-staging-') as temporary:
        scratch = Path(temporary)
        source = scratch / 'values.cod'
        deeper = scratch / 'deeper.cod'
        source.write_text(SOURCE)
        deeper.write_text(SOURCE.replace('300u64', '3000u64'))
        for general in (False, True):
            mode = 'general' if general else 'default'
            ir = scratch / (mode + '.ll')
            command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            record(mode+'/native', run.returncode == 0 and run.stdout == '60\n61\ntrue\n' and not run.stderr,
                   command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr)
            text = ir.read_text() if ir.exists() else ''
            bodies = inventory(text)
            shared = [name for name in bodies if name.endswith('.general')]
            record(mode+'/shared_methods', len(shared) == 4 and len(bodies) == (4 if general else 12),
                   bodies=list(bodies), ir_bytes=len(text.encode()))
            relays = [name for name in bodies if '.relay.specialized.' in name]
            record(mode+'/identity_independent_instances', len(relays) == (0 if general else 2) and
                   'call %cod.outcome @cod.method.3.relay.general' in text and 'callable.selected' in text,
                   specializations=relays)
            makers = [args for name, args in bodies.items() if '.maker.' in name]
            record(mode+'/caller_destinations', bool(makers) and all(re.search(r'\bptr\b', args) for args in makers),
                   signatures=makers)
            record(mode+'/empty_environment_storage', bool(text) and not any(name in text for name in
                   ('@malloc', '@realloc', '@cod_lang_defer_allocate', '@cod_lang_callable_create', '@cod_lang_callable_copy')),
                   ir_bytes=len(text.encode()))
            deep_ir = scratch / (mode+'-deep.ll')
            deep_command = [str(COD), '--check', str(deeper), '--emit-llvm', str(deep_ir)]
            if general:
                deep_command.append('--no-specialize')
            deep = subprocess.run(deep_command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            deep_text = deep_ir.read_text() if deep_ir.exists() else ''
            record(mode+'/runtime_depth_does_not_expand_methods', deep.returncode == 0 and not deep.stdout and not deep.stderr
                   and inventory(deep_text) == bodies, command=deep_command, exit=deep.returncode,
                   stdout=deep.stdout, stderr=deep.stderr, ir_bytes=len(deep_text.encode()))
    passed = sum(item['passed'] for item in results)
    print(f'Callable staging: {passed}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod'/name for name in ('main.cc','lang_checked.hh','lang_completion.hh','lang_diagnostics.hh',
                   'lang_frontend.hh','lang_llvm.hh','lang_syntax.hh','lang_exceptions.hh','lang_native_runtime.hh')]
        sources += [Path(__file__), Path(__file__).with_name('foundation_tests.py')]
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results,
            executable_sha256=hashlib.sha256(COD.read_bytes()).hexdigest(),
            sources={str(path.relative_to(ROOT)):hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}), indent=2)+'\n')
    return passed != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
