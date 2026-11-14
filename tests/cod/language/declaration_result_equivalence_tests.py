#!/usr/bin/env python3
"""Explicit and separately declared contracts generate the same native program."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
import generic_callable_staging_tests as staging

ORIGINAL = staging.SOURCE
import declaration_result_staging_tests as declared
EXTRA_SOURCES = []


def blank(source):
    return ''.join('\n' if ch == '\n' else ' ' for ch in source)


def cases():
    explicit = inherited = ORIGINAL
    for declaration, definition in re.findall(r'(?m)^(func .*?);(func .*?)\{', declared.runner.SOURCE):
        assert declaration + '{' in ORIGINAL
        head = definition.split(' where ')[0]
        original_head = declaration.split(' where ')[0]
        binders = declaration[len(original_head):]
        assert original_head.startswith(head)
        padding = ' ' * len(original_head[len(head):])
        explicit = explicit.replace(declaration + '{', blank(declaration + ';') + declaration + '{')
        inherited = inherited.replace(declaration + '{', declaration + ';' + head + padding + binders + '{')
    assert len(explicit) == len(inherited)
    yield 'generic_and_callable', explicit, inherited, '11\n12\n12\ntrue\n99\n'
    for name, declaration, body, output in (
        ('borrowed_cleanup', 'func f(ref x:i64 in O)->ref i64 in O where {owner O}',
         '{defer{x<-7;};return x;}val x<-1;ref y=f(x);y<-8;behave @print(x);', '8\n'),
        ('recursive', 'func f(n:u64)->u64',
         '{if(n==0u64){return 0u64;}else{return n+f(n-1u64);};}behave @print(f(30u64));', '465\n'),
        ('return_widening', 'func f()->u64',
         '{return 1u32;}func tag(x:u64)->u64{return 64u64;}behave @print(tag(f()));', '64\n'),
    ):
        start = declaration.index('->')
        end = declaration.find(' where ')
        if end < 0:
            end = len(declaration)
        header = declaration[:start] + ' ' * (end - start) + declaration[end:]
        prefix = 'using symbol "std:io" @print;'
        yield name, prefix + blank(declaration + ';') + declaration + body, prefix + declaration + ';' + header + body, output


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-declaration-equivalence-') as directory:
        root = Path(directory)
        source = root / 'main.cod'
        for name, explicit, inherited, expected in cases():
            for general in (False, True):
                mode = 'general' if general else 'default'
                outputs = []
                for label, text in (('explicit', explicit), ('inherited', inherited)):
                    source.write_text(text)
                    ir = root / (name + '-' + mode + '-' + label + '.ll')
                    command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
                    if general:
                        command.append('--no-specialize')
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
                    outputs.append(dict(exit=run.returncode, stdout=run.stdout, stderr=run.stderr,
                                        ir=ir.read_text() if ir.exists() else ''))
                left, right = outputs
                passed = left == right and left['exit'] == 0 and left['stdout'] == expected and bool(left['ir'])
                result = dict(name=name + '/' + mode, passed=passed, identical_ir=left['ir'] == right['ir'],
                              explicit={k: v for k, v in left.items() if k != 'ir'},
                              inherited={k: v for k, v in right.items() if k != 'ir'})
                results.append(result)
                print(('PASS ' if passed else 'FAIL ') + result['name'], flush=True)
                if not passed:
                    print(json.dumps(result, indent=2), flush=True)
    count = sum(item['passed'] for item in results)
    print(f'Declaration equivalence: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh'))]
        sources += [Path(__file__), Path(staging.__file__), Path(declared.__file__), Path(__file__).with_name('foundation_tests.py')]
        sources += EXTRA_SOURCES
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    return count != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
