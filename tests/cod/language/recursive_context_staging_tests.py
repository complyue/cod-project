#!/usr/bin/env python3
"""Declaration evidence survives recursion without runtime capture or expansion."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from callable_value_staging_tests import inventory
from foundation_tests import ROOT, MODE, COD, shadow_diagnostics
from recursive_context_tests import runner

CASES = ('func_anchor_first','generic_declaration_contexts','capture_and_query_shadow','required_query_does_not_execute',
         'recursive_required_query','nested_inferred_query','nested_query_retains_parent_types','required_query_retains_owners')
DEEP_RESULTS = {
    'recursive_required_query':'809\n', 'nested_inferred_query':'809\n',
    'nested_query_retains_parent_types':'407\n409\n', 'required_query_retains_owners':'407\n',
}

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ')+name, flush=True)
        if not passed: print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-recursive-context-') as directory:
        source, ir = Path(directory)/'main.cod', Path(directory)/'main.ll'
        def run(text, name, general, execute=True):
            source.write_text(text)
            command = [str(COD),'--run' if execute else '--check',str(source),'--emit-llvm',str(ir)]
            if general: command.append('--no-specialize')
            completed = subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            shadows, errors = shadow_diagnostics(completed.stderr,runner.CASE_SHADOWS[name])
            return dict(exit=completed.returncode,stdout=completed.stdout,stderr=errors,shadows=shadows), ir.read_text() if completed.returncode == 0 else ''
        for general in (False,True):
            mode = 'general' if general else 'default'
            for name in CASES:
                program,_,expected,_ = runner.CASES[name]
                key = mode+'/'+name
                output,text = run(program,name,general)
                success = dict(exit=0,stdout=expected,stderr='',shadows=True)
                bodies = inventory(text)
                record(key+'/native',output==success,output=output)
                depth = '3u64' if name in DEEP_RESULTS else '20u64'
                deeper,deep_ir = run(program.replace(depth,'400u64'),name,general)
                deep_success = dict(success,stdout=DEEP_RESULTS.get(name,expected))
                record(key+'/depth_reuses_bodies',deeper==deep_success and inventory(deep_ir)==bodies,output=deeper,methods=list(bodies))
                checked,checked_ir = run(program,name,general,False)
                record(key+'/checking_has_no_effects',checked==dict(exit=0,stdout='',stderr='',shadows=True) and inventory(checked_ir)==bodies,output=checked)
                if name != 'capture_and_query_shadow':
                    record(key+'/no_query_environment_allocation','@cod_lang_callable_create' not in text and '@cod_lang_callable_copy' not in text)
                else:
                    record(key+'/complete_runtime_captures','@cod_lang_callable_create' in text and '@cod_lang_callable_copy' in text)
                if name in ('generic_declaration_contexts','nested_query_retains_parent_types'):
                    record(key+'/shared_recursive_body',sum('.recur.' in n and n.endswith('.general') for n in bodies)==1,methods=list(bodies))
    passed = sum(result['passed'] for result in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py'),
                   Path(__file__).with_name('recursive_context_tests.py')]
        digest = lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),toolchain=MODE,
            passed=passed,total=len(results),results=results,executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}),indent=2)+'\n')
    return passed != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
