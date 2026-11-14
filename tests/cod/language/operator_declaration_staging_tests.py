#!/usr/bin/env python3
"""Operator contracts preserve native bodies, shared generic execution and operand timing."""
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
EXTRA_SOURCES = []
CASES = (
    ('builtin', 'func (+)(x:i64,y:i64)->i64;', '', '', IO+'behave @print(2+3);', '5\n'),
    ('generic', 'func (+)(x:T,y:T)->const T where T;', '->const U', ' where U{',
     IO+'func (+)(x:U,y:U)->const U where U{return y;}behave @print(1+2);behave @print(false+true);', '2\ntrue\n'),
    ('borrowed', 'func (+)(ref x:i64 in O,ref y:i64 in P)->ref i64 in O where {owner O,owner P};',
     '->ref i64 in A', ' where {owner A,owner B}{',
     IO+'func (+)(ref x:i64 in A,ref y:i64 in B)->ref i64 in A where {owner A,owner B}{defer{x<-7;};return x;}val x<-1,y<-2;behave @print(x+y);', '7\n'),
    ('complete', 'func (+)(f:func(u64)->u64)->func(u64)->u64;', '->func(u64)->u64', '{',
     IO+'func inc(x:u64)->u64{return x+1u64;}func (+)(f:func(u64)->u64)->func(u64)->u64{return func[f](x:u64)->u64{return f(x);};}func f=(+)(inc);behave @print(f(2u64));', '3\n'),
)
RECURSIVE = IO+'''
func (+)(x:T,y:T)->const T where T;
func (+)(x:U,y:U) where U{return y;}
func relay(x:T,n:u64)->const T where T{if(n==0u64){return x;}else{return relay(x,n-1u64)+x;};}
behave @print(relay(7u64,30u64));behave @print(relay(true,30u64));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-operator-declarations-') as directory:
        root = Path(directory)
        def run(text, general):
            source, ir = root/'main.cod', root/'main.ll'
            source.write_text(text)
            command = [str(COD), '--run', str(source), '--emit-llvm', str(ir)]
            if general:
                command.append('--no-specialize')
            completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=60)
            return dict(exit=completed.returncode, stdout=completed.stdout, stderr=completed.stderr), (
                ir.read_text() if completed.returncode == 0 and ir.exists() else '')
        for general in (False, True):
            mode = 'general' if general else 'default'
            for name, declaration, result, suffix, source, expected in CASES:
                explicit = ' '*len(declaration)+source
                inferred = declaration+source.replace(result+suffix, ' '*len(result)+suffix, 1) if result else declaration+source
                left, left_ir = run(explicit, general)
                right, right_ir = run(inferred, general)
                record(mode+'/'+name+'_equivalence', left == right and left == dict(exit=0, stdout=expected, stderr='')
                       and bool(left_ir) and left_ir == right_ir, explicit=left, declared=right, identical_ir=left_ir == right_ir)
            base, base_ir = run(RECURSIVE, general)
            bodies = inventory(base_ir)
            operators = [name for name in bodies if '.0.' in name]
            record(mode+'/shared_operator', base == dict(exit=0, stdout='7\ntrue\n', stderr='')
                   and len(operators) == (1 if general else 3) and sum(name.endswith('.general') for name in operators) == 1,
                   output=base, operators=operators)
            deep, deep_ir = run(RECURSIVE.replace('30u64','300u64'), general)
            record(mode+'/runtime_depth', deep == base and bool(deep_ir) and set(inventory(deep_ir)) == set(bodies), output=deep)
    count = sum(item['passed'] for item in results)
    print(f'Operator declaration staging: {count}/{len(results)} passed')
    if report := os.environ.get('COD_LANGUAGE_REPORT'):
        sources = [ROOT/'cod/main.cc', *sorted((ROOT/'cod').glob('lang_*.hh')), Path(__file__),
                   Path(__file__).with_name('foundation_tests.py'), Path(__file__).with_name('callable_value_staging_tests.py'), *EXTRA_SOURCES]
        digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE, passed=count, total=len(results), results=results, executable_sha256=digest(COD),
            sources={str(path.relative_to(ROOT)):digest(path) for path in sources}), indent=2)+'\n')
    return count != len(results)

if __name__ == '__main__':
    raise SystemExit(main())
