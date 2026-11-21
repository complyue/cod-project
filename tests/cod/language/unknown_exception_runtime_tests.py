#!/usr/bin/env python3
"""Inject declared failures at native methods; no unknown-type ingress is claimed."""
from pathlib import Path
import re
from language_test_support import ROOT
from native_test_support import run_native
import owner_dispatch_runtime_tests as fixture


FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
SOURCE=FIXTURE.source+'''
using "std:exceptions" StaticallyUnknownTypeException;
using "std:exceptions" UnsoundRuntimeTypingError;
using "std:exceptions" Exception;
func unknown_boundary()->const u64 throws StaticallyUnknownTypeException{return 0u64;}
func typing_boundary()->const u64{return 0u64;}
func unknown_value()->const StaticallyUnknownTypeException{
 try{unknown_boundary();}catch(e:const StaticallyUnknownTypeException){return e;};
}
func missing_value()->const MissingReturnError{
 try{missing():void;}catch(e:const MissingReturnError){return e;};
}
func bounce(e:T)->const T where T{try{throw e;}catch(x:const T){return x;};}
func relay_exception(e:Exception)->void throws Exception{throw e;}
func unknown_wrap(f:func()->const u64 throws StaticallyUnknownTypeException)->const (func()->const u64 throws StaticallyUnknownTypeException){
 return func[f]()->const u64 throws StaticallyUnknownTypeException{return f();};
}
func walk_unknown(f:F,n:u64)->const u64 throws StaticallyUnknownTypeException where F{
 if(n==0u64){return f();}else{return walk_unknown(f,n-1u64);};
}
proc consume_unknown(f:func(u64)->const u64,n:u64)->void{observe(9999u64);}
func saved_unknown()->func(u64)->const u64{
 defer catch(e:const StaticallyUnknownTypeException){}finally{};
 defer{unknown_boundary();};return wrap(base);
}
func replaced_unknown()->func(u64)->const u64{
 defer catch(e:const StaticallyUnknownTypeException){return wrap(other);}finally{};
 defer{unknown_boundary();};return wrap(base);
}
proc delayed_unknown()->void{
 defer observe(300u64);
 defer catch(e:const StaticallyUnknownTypeException){observe(e.category);}finally{observe(302u64);};
 defer{unknown_boundary();};
}
{{
 val original<-unknown_value();observe(bounce(original).category);observe(bounce(missing_value()).category);
 val linked<-try{throw original from original;}catch(e:const StaticallyUnknownTypeException){e};
 observe(linked.category);observe(linked.site);observe(linked.requested);observe(linked.available);observe(linked.cause_count);
 observe(cause_at(linked,0u64).category);observe(cause_at(linked,0u64).requested);
 try{relay_exception(linked);}catch(e:const StaticallyUnknownTypeException){defer{observe(e.category);};}catch(e){observe(9999u64);};
 func f=unknown_wrap(unknown_boundary);func ordinary=wrap(base);
 try{walk_unknown(f,40u64);{}}catch(e:const StaticallyUnknownTypeException){observe(e.category);};
 observe(9040u64);try{consume_unknown(ordinary,unknown_boundary());}catch(e:const StaticallyUnknownTypeException){observe(9041u64);observe(310u64);};
 observe(9030u64);try{consume_unknown(ordinary,unknown_boundary());}catch(e:const OutOfMemoryError){observe(9031u64);}catch(e:const StaticallyUnknownTypeException){observe(9999u64);};
 observe(9050u64);try{0;{{defer f();}};}catch(e:const OutOfMemoryError){observe(9051u64);}catch(e:const StaticallyUnknownTypeException){observe(9999u64);};
 delayed_unknown();observe(saved_unknown()(1u64));observe(replaced_unknown()(1u64));
 try{typing_boundary();{}}catch(e:const UnsoundRuntimeTypingError){observe(e.category);};
 observe(9010u64);val total<-0u64;
 for(i in 0u64..10000u64){try{f();}catch(e:const StaticallyUnknownTypeException){total<-total+1u64;};};
 observe(9011u64);observe(total);
}};
observe(9000u64);
'''
WRAPPERS=FIXTURE.wrappers
HARNESS=FIXTURE.harness.replace('checkpoints==10','checkpoints==11').replace('copy_failures==9','copy_failures==10').replace('record_failures==6','record_failures==7')
EXPECTED_VALUES=list(FIXTURE.expected_values)+['7','5','7','1','321','123','1','7','321','7','310','7','302','300','2','12','6','10000','7']
EXPECTED_SHADOWS=['other']
EXTRA_SOURCES=[*list(FIXTURE.sources),Path(fixture.__file__),Path(__file__).with_name('exception_value_runtime_tests.py'),Path(__file__)]


def inject_failures(ir):
    """Replace only declared native providers, retaining all compiled consumers."""
    catalog = (ROOT / "cod/lang_intrinsic_catalog.h").read_text()
    injected=[]
    for method,category in [('unknown_boundary','StaticallyUnknownTypeException'),('typing_boundary','UnsoundRuntimeTypingError')]:
        identity=int(re.search(r'ENTRY\(\w+, '+category+r', (\d+),',catalog)[1])
        pattern=r'(?ms)^(define internal %cod.outcome @cod\.method\.\d+\.'+method+r'\.native\(\) \{)\n.*?^}'
        body='\nentry:\n  ret %cod.outcome { %cod.scalar zeroinitializer, i64 '+str(identity)+', i64 1, ptr null, i64 321, i64 123, i64 0 }\n}'
        ir,count=re.subn(pattern,lambda match:match[1]+body,ir)
        if count!=1:raise RuntimeError(f'Expected exactly one native provider for {method}, found {count}')
        injected.append(dict(method=method,category=category,identity=identity))
    return ir, {"injected": injected}


FIXTURE = FIXTURE.updated(source=SOURCE, wrappers=WRAPPERS, harness=HARNESS, expected_values=EXPECTED_VALUES,
                          expected_shadows=EXPECTED_SHADOWS, sources=EXTRA_SOURCES, transform_ir=inject_failures)

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
