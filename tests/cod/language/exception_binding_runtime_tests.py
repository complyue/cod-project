#!/usr/bin/env python3
"""Exception TLS and replay retain cause handles without copying cause records."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" MissingReturnError;
using "std:exceptions" Exception;
func missing()->u64{}
func error()->const MissingReturnError{try{missing():void;}catch(e:const MissingReturnError){return e;};}
func chain(e:MissingReturnError,c:Exception)->const MissingReturnError{
 return try{throw e from c;}catch(x:const MissingReturnError){x};
}
proc observe(x:u64)->void{behave @print(x);return;}
proc show(e:Exception)->void{observe(e.cause_count);return;}
symbol @saved:Exception;
symbol @p:proc()->u64;
proc depth()->u64{return @saved.cause_count;}
proc build()->proc()->u64{effect @saved<-chain(error(),error());return depth;}
effect @saved<-error();
{{
 effect @saved<-chain(error(),error());effect @p=depth;effect @saved<-error();
 observe(9010u64);val total<-0u64;
 for(i in 0u64..1000u64){effect @saved<-error();total<-total+perform @p()+behave @p();};
 observe(total);observe(@saved.cause_count);observe(9011u64);
}};
observe(9000u64);
{{
 effect @saved<-chain(error(),error());effect @p=depth;observe(9030u64);
 try{effect @p=@p;}catch(e:const OutOfMemoryError){observe(9031u64);};
 observe(perform @p());observe(@saved.cause_count);
}};
observe(9000u64);
{{
 effect @saved<-chain(error(),error());effect @p=depth;observe(9070u64);
 try{effect @saved<-chain(error(),@saved);}catch(e:const OutOfMemoryError){observe(9071u64);observe(e.cause_count);};
 observe(@saved.cause_count);observe(perform @p());
}};
observe(9000u64);
{{effect @p=build();observe(perform @p());observe(@saved.cause_count);}};
observe(9000u64);
{{
 effect @saved<-chain(error(),error());observe(9050u64);
 try{defer show(@saved);}catch(e:const OutOfMemoryError){observe(9051u64);};
 observe(@saved.cause_count);
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(wrappers=FIXTURE.wrappers + '\n#define cod_lang_exception_chain cod_chain\n')
FIXTURE = FIXTURE.updated(harness='#undef cod_lang_exception_chain\n' + FIXTURE.harness)
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('static bool deny_create=', 'static bool deny_chain=false;\nstatic unsigned chain_calls=0,chain_failures=0;\nstatic bool deny_create='))
needle = 'extern "C" void cod_lang_print_u64(std::uint64_t value){'
chain = '''extern "C" std::uint64_t cod_lang_exception_chain(std::uint64_t category,std::uint64_t site,
    std::uint64_t requested,std::uint64_t available,std::uint64_t cause,std::uint64_t *status){
  assert(!measuring);++chain_calls;
  if(deny_chain){++chain_failures;status[0]=1;return 0;}
  return cod_chain(category,site,requested,available,cause,status);
}
'''
assert needle in FIXTURE.harness
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace(needle, chain+needle))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('if(value==9000)', '''if(value==9070){assert(cod_causes && cod_causes->size()==3);deny_chain=true;return;}
  if(value==9071){assert(deny_chain && chain_failures==1 && cod_causes->size()==3);deny_chain=false;return;}
  if(value==9000)'''))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('creation_failures==1', 'creation_failures==0').replace('copy_failures==3', 'copy_failures==1').replace('largest>4000', 'largest>0'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('  cod_finish();', '  assert(!deny_chain && chain_calls==6 && chain_failures==1 && cod_causes->size()==5);\n  cod_finish();assert(!cod_causes);'))
FIXTURE = FIXTURE.updated(expected_values=['1000', '0', '1', '1', '0', '1', '1', '1', '0', '1'])
FIXTURE = FIXTURE.updated(expected_shadows=[])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
