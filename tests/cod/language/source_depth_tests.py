#!/usr/bin/env python3
"""Source-depth exhaustion reports its allowance without emitting partial code."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from language_test_support import ROOT, MODE, COD

IO = 'using symbol "std:io" @print;'
ID = 'func id(x:u64)->u64{return x;}'
EXPRESSION_LIMIT = 'compiler resource limit: 128 source structure levels while parsing an expression'
TYPE_LIMIT = 'compiler resource limit: 128 source structure levels while parsing a type'
ENTRY_LIMIT = 'compiler resource limit: 1024 active recursive parsing entries while parsing '


def nested_definition(count):
    body = ''
    for i in reversed(range(count)):
        body = f'func nested_{i}()->void{{{body}}};'
    return 'func outer()->void{' + body + '}'


CASES = {
    'deep_grouping': ('(' * 8192 + '1u64' + ')' * 8192 + ';', ENTRY_LIMIT, ''),
    'deep_left_addition': ('+'.join(['1u64'] * 512) + ';', EXPRESSION_LIMIT, ''),
    'deep_unary': ('!' * 512 + 'false;', EXPRESSION_LIMIT, ''),
    'deep_calls': (ID + 'id(' * 512 + '1u64' + ')' * 512 + ';', EXPRESSION_LIMIT, ''),
    'deep_blocks': ('{' * 2048 + '1u64' + '}' * 2048 + ';', ENTRY_LIMIT, ''),
    'structural_boundary': ('+'.join(['1u64'] * 128) + ';', '', ''),
    'structural_exhaustion': ('+'.join(['1u64'] * 129) + ';', EXPRESSION_LIMIT, ''),
    'unary_boundary': ('!' * 127 + 'false;', '', ''),
    'call_boundary': (ID + 'id(' * 127 + '1u64' + ')' * 127 + ';', '', ''),
    'grouping_boundary': ('(' * 1023 + '1u64' + ')' * 1023 + ';', '', ''),
    'grouping_exhaustion': ('(' * 1024 + '1u64' + ')' * 1024 + ';', ENTRY_LIMIT, ''),
    'type_boundary': ('func unused(x:' + 'func()->' * 127 + 'u64)->void;', '', ''),
    'type_exhaustion': ('func unused(x:' + 'func()->' * 128 + 'u64)->void;', TYPE_LIMIT, ''),
    'type_grouping_exhaustion': ('func unused(x:' + '(' * 8192 + 'u64' + ')' * 8192 + ')->void;', ENTRY_LIMIT, ''),
    'lexical_definitions': (nested_definition(80), EXPRESSION_LIMIT, ''),
    'shallow_lexical_definitions': (nested_definition(16), '', ''),
    'flat_forms': ('1u64;' * 5000 + IO + 'behave @print(7u64);', '', '7\n'),
    'comment_delimiters': (IO + '#' + '({' * 8192 + '\nbehave @print(7u64);', '', '7\n'),
    'quoted_delimiters': ('using symbol "std:' + '(' * 8192 + '" @print;', 'module declaration or selection', ''),
    'malformed_expression': ('1u64+;', 'expected an expression', ''),
    'numeric_execution': (IO + 'behave @print(' + '+'.join(['1u64'] * 64) + ');', '', '64\n'),
}


def no_core():
    import resource
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def main():
    results = []
    with tempfile.TemporaryDirectory(prefix='cod-source-depth-') as directory:
        scratch = Path(directory)
        for name, (source, diagnostic, expected) in CASES.items():
            entry = scratch / (name + '.cod')
            # No source effect may execute before a later depth diagnostic.
            entry.write_text((IO + 'behave @print(99u64);' if diagnostic.startswith('compiler resource limit:') else '') + source)
            for general in (False, True):
                ir = scratch / (name + '-' + str(general) + '.ll')
                command = [str(COD), '--run', str(entry), '--emit-llvm', str(ir)]
                if general:
                    command.append('--no-specialize')
                kwargs = dict(preexec_fn=no_core) if os.name != 'nt' else {}
                run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60, **kwargs)
                passed = run.returncode == (2 if diagnostic else 0) and run.stdout == expected
                if diagnostic:
                    passed = passed and diagnostic in run.stderr and not ir.exists()
                    if diagnostic.startswith('compiler resource limit:'):
                        passed = passed and re.match(re.escape(str(entry)) + r':\d+:\d+: compiler resource limit:', run.stderr) is not None
                        passed = passed and 'implementation gap' not in run.stderr
                else:
                    passed = passed and run.stderr == '' and ir.is_file()
                row = dict(name=name + ('/general' if general else '/default'), passed=bool(passed),
                           command=command, exit=run.returncode, stdout=run.stdout, stderr=run.stderr,
                           source_sha256=hashlib.sha256(entry.read_bytes()).hexdigest())
                results.append(row)
                print(('PASS ' if passed else 'FAIL ') + row['name'], flush=True)
                if not passed:
                    print(json.dumps(row, indent=2), flush=True)
    passed = sum(row['passed'] for row in results)
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT / 'cod/main.cc', *sorted((ROOT / 'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('language_test_support.py')]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=passed, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)): digest(path) for path in sources}), indent=2) + '\n')
    print(f'Source depth: {passed}/{len(results)} passed')
    return passed != len(results)


if __name__ == '__main__':
    raise SystemExit(main())
