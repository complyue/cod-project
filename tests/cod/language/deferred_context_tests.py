#!/usr/bin/env python3
"""Check every source cleanup context before executing or emitting artifacts."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

from language_test_support import ROOT, COD

BASE = 'proc work()->void{}proc consume(x:u64)->void{}\n'
INVALID = {
    'block': 'defer { defer work(); };',
    'catch': 'defer {} catch(e) { defer work(); };',
    'finally': 'defer {} finally { defer work(); };',
    'bodyless_catch': 'defer catch(e) { defer work(); };',
    'bodyless_finally': 'defer finally { defer work(); };',
    'group': 'defer { { defer work(); }; };',
    'conditional': 'defer { if(false) { defer work(); } else {}; };',
    'loop': 'defer { for(i in 0..1) { defer work(); }; };',
    'unused': 'proc unused()->void { defer { defer work(); }; }',
    'generic': 'func unused(x:T)->void where T { defer { defer work(); }; }',
    'anonymous': 'proc saved=proc()->void { defer { defer work(); }; };',
    'unreachable': 'proc unused()->void { return; defer { defer work(); }; }',
    'query': 'defer { make local typeof({ defer work(); 1u64 }); };',
    'initializer': 'defer { make local u64 { { defer work(); 1u64 } }; };',
    'new_scope_own_drain': 'defer { {{ defer { defer work(); }; }}; };',
    'helper_own_drain': 'proc helper()->void { defer { defer work(); }; } defer helper();',
    'eager_preparation_inside_delayed': 'defer { consume({ defer work(); 1u64 }); };',
    'imported_unused': 'import "./invalid.cod" unused;',
}
VALID = {
    'ordinary': 'defer work(); defer work();',
    'eager_preparation': 'defer consume({ defer work(); 1u64 });',
    'independent_helper': 'proc helper()->void { defer work(); } defer helper();',
    'independent_scope': 'defer { {{ defer work(); }}; };',
    'independent_local_body': 'defer { {{ proc helper()->void { defer work(); }; helper(); }}; };',
    'independent_anonymous_body': 'defer { (proc()->void { defer work(); })(); };',
    'restore_outer_context': '{{ defer work(); }}; defer work();',
}


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-deferred-context-') as directory:
        scratch = Path(directory)
        imported = scratch / 'invalid.cod'
        imported.write_text('proc work()->void{}proc unused()->void{defer{defer work();};}')
        for name, source in {**INVALID, **VALID}.items():
            entry = scratch / (name + '.cod')
            prefix = 'using symbol "std:io" @print;behave @print(99u64);\n' if name in INVALID else ''
            entry.write_text(BASE + prefix + source)
            for mode in ('--check', '--run'):
                for general in (False, True):
                    ir, bc = scratch / 'output.ll', scratch / 'output.bc'
                    command = [str(COD), mode, str(entry), '--emit-llvm', str(ir), '--emit-bc', str(bc)]
                    if general:
                        command.append('--no-specialize')
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                    if name in INVALID:
                        passed = (run.returncode == 2 and run.stdout == '' and not ir.exists() and not bc.exists()
                                  and 'currently draining scope' in run.stderr
                                  and ': note: enclosing deferred execution begins here' in run.stderr
                                  and (str(imported) + ':1:' if name == 'imported_unused' else str(entry) + ':3:') in run.stderr)
                    else:
                        passed = run.returncode == 0 and run.stdout == run.stderr == '' and ir.exists() and bc.exists()
                    label = f'{name}/{mode}/{"general" if general else "default"}'
                    results.append({'name': label, 'passed': passed, 'exit': run.returncode, 'stdout': run.stdout,
                                    'stderr': run.stderr, 'command': command})
                    print(('PASS ' if passed else 'FAIL ') + label, flush=True)
                    ir.unlink(missing_ok=True)
                    bc.unlink(missing_ok=True)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        Path(report).write_text(json.dumps({'passed': sum(r['passed'] for r in results), 'total': len(results),
                                         'executable_sha256': hashlib.sha256(COD.read_bytes()).hexdigest(),
                                         'results': results}, indent=2) + '\n')
    return int(not all(result['passed'] for result in results))


if __name__ == '__main__':
    raise SystemExit(main())
