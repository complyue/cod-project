#!/usr/bin/env python3
"""Exception TLS and replay retain cause handles without copying cause records."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
using "std:exceptions" SystemException;
func missing()->u64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func chain(e:MissingReturnException,c:SystemException)->const MissingReturnException{
 return try{throw e from c;}catch(x:const MissingReturnException){x};
}
proc observe(x:u64)->void{behave @print(x);}
proc show(e:SystemException)->void{observe(e.cause_count);}
symbol @saved:SystemException;
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
 try{effect @p=@p;}catch(e:const OutOfMemory){observe(9031u64);};
 observe(perform @p());observe(@saved.cause_count);
}};
observe(9000u64);
{{
 effect @saved<-chain(error(),error());effect @p=depth;observe(9070u64);
 try{effect @saved<-chain(error(),@saved);}catch(e:const OutOfMemory){observe(9071u64);observe(e.cause_count);};
 observe(@saved.cause_count);observe(perform @p());
}};
observe(9000u64);
{{effect @p=build();observe(perform @p());observe(@saved.cause_count);}};
observe(9000u64);
{{
 effect @saved<-chain(error(),error());observe(9050u64);
 try{defer show(@saved);}catch(e:const OutOfMemory){observe(9051u64);};
 observe(@saved.cause_count);
}};
observe(9000u64);
'''
runner.WRAPPERS += '\n#define cod_lang_exception_chain cod_chain\n'
runner.HARNESS = '#undef cod_lang_exception_chain\n' + runner.HARNESS
runner.HARNESS = runner.HARNESS.replace('static bool deny_create=', 'static bool deny_chain=false;\nstatic unsigned chain_calls=0,chain_failures=0;\nstatic bool deny_create=')
needle = 'extern "C" void cod_lang_print_u64(std::uint64_t value){'
chain = '''extern "C" std::uint64_t cod_lang_exception_chain(std::uint64_t category,std::uint64_t site,
    std::uint64_t requested,std::uint64_t available,std::uint64_t cause,std::uint64_t *status){
  assert(!measuring);++chain_calls;
  if(deny_chain){++chain_failures;status[0]=1;return 0;}
  return cod_chain(category,site,requested,available,cause,status);
}
'''
assert needle in runner.HARNESS
runner.HARNESS = runner.HARNESS.replace(needle, chain+needle)
runner.HARNESS = runner.HARNESS.replace('if(value==9000)', '''if(value==9070){assert(cod_causes && cod_causes->size()==3);deny_chain=true;return;}
  if(value==9071){assert(deny_chain && chain_failures==1 && cod_causes->size()==3);deny_chain=false;return;}
  if(value==9000)''')
runner.HARNESS = runner.HARNESS.replace('creation_failures==1', 'creation_failures==0').replace('copy_failures==3', 'copy_failures==1').replace('largest>4000', 'largest>0')
runner.HARNESS = runner.HARNESS.replace('  cod_finish();', '  assert(!deny_chain && chain_calls==6 && chain_failures==1 && cod_causes->size()==5);\n  cod_finish();assert(!cod_causes);')
runner.EXPECTED_VALUES = ['1000', '0', '1', '1', '0', '1', '1', '1', '0', '1']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
