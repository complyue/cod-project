#!/usr/bin/env python3
"""Normalized formal inputs share native bodies across lossless argument types."""
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
func real(x:f64)->f64{return x;}
func relay(x:T,n:u64) where T{
  if(n==0u64){return real(x);}else{return relay(x,n-1u64);};
}
func mixed(tag:T,x:f64,n:u64) where T{
  if(n==0u64){return x;}else{return mixed(tag,x,n-1u64);};
}
behave @print(real(1u8));behave @print(real(16777217u32));behave @print(real(3f32));
behave @print(relay(1u8,30u8));behave @print(relay(16777217u32,30u8));behave @print(relay(3f32,30u8));
behave @print(mixed(true,1u8,30u8));behave @print(mixed(true,2u32,30u8));
behave @print(mixed(1u64,3f32,30u8));behave @print(mixed(1u64,4f64,30u8));
'''

def main():
    results = []
    def record(name, passed, **details):
        results.append(dict(name=name,passed=passed,**details))
        print(('PASS ' if passed else 'FAIL ')+name,flush=True)
        if not passed:print(json.dumps(details,indent=2),flush=True)
    with tempfile.TemporaryDirectory(prefix='cod-numeric-dispatch-') as directory:
        root=Path(directory)
        def run(text,general):
            source,ir=root/'main.cod',root/'main.ll';source.write_text(text)
            command=[str(COD),'--run',str(source),'--emit-llvm',str(ir)]
            if general:command.append('--no-specialize')
            completed=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=60)
            return dict(exit=completed.returncode,stdout=completed.stdout,stderr=completed.stderr),(
                ir.read_text() if completed.returncode==0 and ir.exists() else '')
        for general in (False,True):
            mode='general' if general else 'default'
            base,ir=run(SOURCE,general)
            expected=dict(exit=0,stdout='1\n16777217\n3\n1\n16777217\n3\n1\n2\n3\n4\n',stderr='')
            bodies=inventory(ir)
            record(mode+'/native_results',base==expected,output=base)
            names=[name for name in bodies if 'cod.method.0.real.' in name]
            record(mode+'/one_concrete_method',len(names)==1 and names[0].endswith('.native'),methods=names)
            for method,count in (('1.relay',4),('2.mixed',3)):
                names=[name for name in bodies if 'cod.method.'+method+'.' in name]
                record(mode+'/'+method+'_shared',len(names)==(1 if general else count)
                       and sum(name.endswith('.general') for name in names)==1,methods=names)
            record(mode+'/native_conversions',bool(re.search(r'uitofp i32 [^\n]+ to double',ir))
                   and bool(re.search(r'fpext float [^\n]+ to double',ir)))
            deep,deep_ir=run(SOURCE.replace('30u8','300u32'),general)
            record(mode+'/depth_and_count_type',deep==expected and set(inventory(deep_ir))==set(bodies),output=deep)
            first='behave @print(relay(1u8,30u8));';second='behave @print(relay(16777217u32,30u8));'
            swapped,swapped_ir=run(SOURCE.replace(first+second,second+first),general)
            reordered=dict(expected,stdout='1\n16777217\n3\n16777217\n1\n3\n1\n2\n3\n4\n')
            record(mode+'/demand_order',swapped==reordered and set(inventory(swapped_ir))==set(bodies),output=swapped)
    count=sum(item['passed'] for item in results)
    print(f'Numeric dispatch staging: {count}/{len(results)} passed')
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),
                 Path(__file__).with_name('foundation_tests.py'),Path(__file__).with_name('callable_value_staging_tests.py')]
        digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
            toolchain=MODE,passed=count,total=len(results),results=results,executable_sha256=digest(COD),
            sources={str(p.relative_to(ROOT)):digest(p) for p in sources}),indent=2)+'\n')
    return count!=len(results)

if __name__=='__main__':
    raise SystemExit(main())
