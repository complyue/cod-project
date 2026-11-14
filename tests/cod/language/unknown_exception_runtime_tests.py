#!/usr/bin/env python3
"""Inject declared failures at native methods; no unknown-type ingress is claimed."""
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
import owner_dispatch_runtime_tests as fixture
from foundation_tests import ROOT, MODE, COD
from regional_runtime_tests import checked

runner=fixture.runner
SOURCE=runner.SOURCE+'''
using "std:exceptions" StaticallyUnknownType;
using "std:exceptions" UnsoundRuntimeTyping;
using "std:exceptions" SystemException;
func unknown_boundary()->const u64 throws StaticallyUnknownType{return 0u64;}
func typing_boundary()->const u64{return 0u64;}
func unknown_value()->const StaticallyUnknownType{
 try{unknown_boundary();}catch(e:const StaticallyUnknownType){return e;};
}
func missing_value()->const MissingReturnException{
 try{missing();}catch(e:const MissingReturnException){return e;};
}
func bounce(e:T)->const T where T{try{throw e;}catch(x:const T){return x;};}
func relay_exception(e:SystemException)->void throws SystemException{throw e;}
func unknown_wrap(f:func()->const u64 throws StaticallyUnknownType)->const (func()->const u64 throws StaticallyUnknownType){
 return func[f]()->const u64 throws StaticallyUnknownType{return f();};
}
func walk_unknown(f:F,n:u64)->const u64 throws StaticallyUnknownType where F{
 if(n==0u64){return f();}else{return walk_unknown(f,n-1u64);};
}
proc consume_unknown(f:func(u64)->const u64,n:u64)->void{observe(9999u64);}
func saved_unknown()->func(u64)->const u64{
 defer catch(e:const StaticallyUnknownType){}finally{};
 defer{unknown_boundary();};return wrap(base);
}
func replaced_unknown()->func(u64)->const u64{
 defer catch(e:const StaticallyUnknownType){return wrap(other);}finally{};
 defer{unknown_boundary();};return wrap(base);
}
proc delayed_unknown()->void{
 defer observe(300u64);
 defer catch(e:const StaticallyUnknownType){observe(e.category);}finally{observe(302u64);};
 defer{unknown_boundary();};
}
{{
 val original=unknown_value();observe(bounce(original).category);observe(bounce(missing_value()).category);
 val linked=try{throw original from original;}catch(e:const StaticallyUnknownType){e};
 observe(linked.category);observe(linked.site);observe(linked.requested);observe(linked.available);observe(linked.cause_count);
 observe(cause_at(linked,0u64).category);observe(cause_at(linked,0u64).requested);
 try{relay_exception(linked);}catch(e:const StaticallyUnknownType){defer{observe(e.category);};}catch(e){observe(9999u64);};
 func f=unknown_wrap(unknown_boundary);func ordinary=wrap(base);
 try{walk_unknown(f,40u64);{}}catch(e:const StaticallyUnknownType){observe(e.category);};
 observe(9040u64);try{consume_unknown(ordinary,unknown_boundary());}catch(e:const StaticallyUnknownType){observe(9041u64);observe(310u64);};
 observe(9030u64);try{consume_unknown(ordinary,unknown_boundary());}catch(e:const OutOfMemory){observe(9031u64);}catch(e:const StaticallyUnknownType){observe(9999u64);};
 observe(9050u64);try{0;{{defer f();}};}catch(e:const OutOfMemory){observe(9051u64);}catch(e:const StaticallyUnknownType){observe(9999u64);};
 delayed_unknown();observe(saved_unknown()(1u64));observe(replaced_unknown()(1u64));
 try{typing_boundary();{}}catch(e:const UnsoundRuntimeTyping){observe(e.category);};
 observe(9010u64);val total=0u64;
 for(i in 0u64..10000u64){try{f();}catch(e:const StaticallyUnknownType){total<-total+1u64;};};
 observe(9011u64);observe(total);
}};
observe(9000u64);
'''
WRAPPERS=runner.WRAPPERS
HARNESS=runner.HARNESS.replace('checkpoints==10','checkpoints==11').replace('copy_failures==9','copy_failures==10').replace('record_failures==6','record_failures==7')
EXPECTED_VALUES=runner.EXPECTED_VALUES+['7','5','7','1','321','123','1','7','321','7','310','7','302','300','2','12','6','10000','7']
EXTRA_SOURCES=[*runner.EXTRA_SOURCES,Path(fixture.__file__),Path(runner.__file__),Path(__file__)]


def inject_failures(ir, runtime):
    """Replace only declared native providers, retaining all compiled consumers."""
    injected=[]
    for method,category in [('unknown_boundary','StaticallyUnknownType'),('typing_boundary','UnsoundRuntimeTyping')]:
        identity=int(re.search(r'\b'+category+r' = (\d+),',runtime)[1])
        pattern=r'(?ms)^(define internal %cod.outcome @cod\.method\.\d+\.'+method+r'\.native\(\) \{)\n.*?^}'
        body='\nentry:\n  ret %cod.outcome { %cod.scalar zeroinitializer, i64 '+str(identity)+', i64 1, ptr null, i64 321, i64 123, i64 0 }\n}'
        ir,count=re.subn(pattern,lambda match:match[1]+body,ir)
        if count!=1:raise RuntimeError(f'Expected exactly one native provider for {method}, found {count}')
        injected.append(dict(method=method,category=category,identity=identity))
    return ir,injected


def main():
    tool=ROOT/MODE/'bin';results=[]
    with tempfile.TemporaryDirectory(prefix='cod-unknown-exception-') as directory:
        scratch=Path(directory);source=scratch/'main.cod';source.write_text(SOURCE)
        for general in (False,True):
            ir=scratch/'main.ll';bc=scratch/'injected.bc';runtime=scratch/'runtime.cc'
            command=[COD,'--check',source,'--emit-llvm',ir,'--emit-runtime',runtime]
            if general:command.append('--no-specialize')
            output,frontend_seconds=checked(command,ROOT,('[-Wmissing-return]',))
            if output:raise RuntimeError('Checking executed runtime effects')
            original=ir.read_text();modified,injected=inject_failures(original,runtime.read_text())
            ir.write_text(modified)
            checked([tool/'llvm-as',ir,'-o',bc],ROOT)
            checked([tool/'opt','-passes=verify','-disable-output',bc],ROOT)
            harness=scratch/'instrumented.cc';harness.write_text(WRAPPERS+runtime.read_text()+HARNESS)
            executable=scratch/'native'
            compile_command=[tool/'clang++','-std=c++20','-O2','-g','-UNDEBUG','-stdlib=libc++','-fuse-ld=lld',bc,harness,'-o',executable,
                             '-L'+str(ROOT/MODE/'lib'),'-Wl,-rpath,'+str(ROOT/MODE/'lib')]
            _,compile_seconds=checked(compile_command,ROOT)
            output,execution_seconds=checked([executable],ROOT)
            *values,metrics=output.splitlines()
            if values!=EXPECTED_VALUES:raise RuntimeError(f'Unexpected exception trace: {values}')
            results.append(dict(name='general' if general else 'default',passed=True,metrics=json.loads(metrics),stdout=values,injected=injected,
                                frontend_seconds=frontend_seconds,compile_seconds=compile_seconds,execution_seconds=execution_seconds,
                                check_command=list(map(str,command)),compile_command=list(map(str,compile_command)),
                                original_ir_sha256=hashlib.sha256(original.encode()).hexdigest(),injected_ir_sha256=hashlib.sha256(modified.encode()).hexdigest()))
            print(f"PASS {results[-1]['name']}: {metrics}",flush=True)
    if report:=os.environ.get('COD_LANGUAGE_REPORT'):
        sources=[ROOT/'cod/main.cc',*sorted((ROOT/'cod').glob('lang_*.hh')),Path(__file__),Path(__file__).with_name('foundation_tests.py'),
                 Path(__file__).with_name('regional_runtime_tests.py'),*EXTRA_SOURCES]
        digest=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
        Path(report).write_text(json.dumps(dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),toolchain=MODE,
                                               passed=len(results),total=2,results=results,executable_sha256=digest(COD),
                                               sources={str(path.relative_to(ROOT)):digest(path) for path in sources}),indent=2)+'\n')
    return 0

if __name__=='__main__':
    raise SystemExit(main())
