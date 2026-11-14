#!/usr/bin/env python3
"""Numeric argument adaptation and mixed operations retain shared native methods."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from foundation_tests import ROOT, MODE, COD
from callable_value_staging_tests import inventory

SOURCE = '''
using symbol "std:io" @print;
func integer(x:u64)->u64{return x;}
func real(x:f64)->f64{return x;}
func small(x:f32)->f32{return x;}
func apply(f:func(T)->const T,x:U,n:u64)->const T where {T,U}{
  if(n==0u64){return f(x);}else{return apply(f,x,n-1u64);};
}
func mixed(x:T,y:U) where {T,U}{return x+y;}
behave @print(apply(integer,1u8,30u64));
behave @print(apply(real,16777217u32,30u64));
behave @print(apply(small,3u8,30u64));
behave @print(mixed(1u32,2f32));behave @print(mixed(1u8,2f32));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name, passed=passed, **details))
        print(('PASS ' if passed else 'FAIL ') + name, flush=True)
        if not passed:
            print(json.dumps(details, indent=2), flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-numeric-value-') as directory:
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
            base, ir = run(SOURCE, general)
            expected = dict(exit=0, stdout='1\n16777217\n3\n3\n3\n', stderr='')
            bodies = inventory(ir)
            record(mode+'/native_results', base == expected, output=base)
            for method, count in (('3.apply',4),('4.mixed',3)):
                names = [name for name in bodies if 'cod.method.'+method+'.' in name]
                record(mode+'/'+method+'_shared', len(names) == (1 if general else count)
                       and sum(name.endswith('.general') for name in names) == 1, methods=names)
            record(mode+'/lossless_native_conversions', bool(re.search(r'uitofp i32 [^\n]+ to double',ir))
                   and bool(re.search(r'fpext float [^\n]+ to double',ir)))
            deep, deep_ir = run(SOURCE.replace('30u64','300u64'), general)
            record(mode+'/runtime_depth', deep == expected and set(inventory(deep_ir)) == set(bodies), output=deep)
            first = 'behave @print(apply(integer,1u8,30u64));'
            second = 'behave @print(apply(real,16777217u32,30u64));'
            reordered, reordered_ir = run(SOURCE.replace(first+'\n'+second,second+'\n'+first), general)
            record(mode+'/demand_order', reordered == dict(exit=0,stdout='16777217\n1\n3\n3\n3\n',stderr='')
                   and set(inventory(reordered_ir)) == set(bodies), output=reordered)
    count = sum(item['passed'] for item in results)
    print(f'Numeric value staging: {count}/{len(results)} passed')
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
