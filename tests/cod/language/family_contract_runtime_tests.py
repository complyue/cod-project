#!/usr/bin/env python3
"""Check family argument ownership and cleanup under native allocation failures."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" SystemException;
using "std:exceptions" MissingReturnException;
using "std:exceptions" NullPointerException;
using "std:exceptions" UnsoundRuntimeTyping;
func missing()->u64{}
func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func null_error()->const NullPointerException{val x<-1;ptr p=x;p=null;try{p;{}}catch(e:const NullPointerException){return e;};}
proc observe(x:u64)->void{behave @print(x);}
func one()->u64{return 1u64;}
func wrap(f:func()->u64)->func()->u64{return func[f]()->u64{return f();};}
symbol @bias:u64,@select:proc(SystemException)->u64;
proc @select(e:MissingReturnException)->u64{return @bias+7u64;}
proc @select(e:T)->u64 where T{return @bias+e.category;}
proc @apply(f:func()->u64,n:u64)->u64{return f();}
proc @use(f:func()->u64,ref x:u64 in O)->void where {owner O}{observe(f()+x);}
proc @update(e:SystemException,ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-1u64;}
proc @update(e:MissingReturnException,ptr x:u64 in O)->void throws NullPointerException where {owner O}{x<-7u64;}
effect @bias<-1u64;
val first:SystemException<-error(),second:SystemException<-null_error();
{{
 effect @select;effect @bias<-9u64;
 val chosen:SystemException<-first;observe(9010u64);val sum<-0u64;
 for(i in 0u64..1000u64){chosen<-if(i%2u64==0u64){first}else{second};sum<-sum+perform @select(chosen);};
 observe(sum);observe(@bias);observe(9011u64);
}};
observe(9000u64);
{{
 effect @apply;func held=wrap(one);
 observe(9020u64);
 try{perform @apply(wrap(one),0u64);{}}catch(e:const OutOfMemory){observe(9021u64);};
 observe(9030u64);
 try{perform @apply(held,{observe(9999u64);0u64});{}}catch(e:const OutOfMemory){observe(9031u64);};
 observe(9030u64);
 try{defer perform @apply(held,0u64);}catch(e:const OutOfMemory){observe(9031u64);};
 observe(perform @apply(held,0u64));
}};
observe(9000u64);
{{
 effect @use;func held=wrap(one);val x<-2u64;observe(9050u64);
 try{defer perform @use(held,x);}catch(e:const OutOfMemory){observe(9051u64);};
 defer perform @use(held,x);x<-7u64;
}};
observe(9000u64);
{{
 effect @update;val x<-make local u64;
 try{behave @update(first,x);observe(x);behave @update(second,x);observe(x);
  {{defer behave @update(first,x);first<-second;x<-0u64;}};
  observe(x);
 }catch(e:const NullPointerException){};
}};
observe(9000u64);
{{
 effect @select;effect @bias<-9u64;
 for(i in 9060u64..9063u64){
  observe(i);
  try{perform @select(cause_at(first,0u64));{}}
  catch(e:const UnsoundRuntimeTyping){observe(e.category);observe(@bias);};
 };
}};
observe(9000u64);
'''
runner.WRAPPERS += '#define cod_lang_exception_cause cod_original_cause\n'
runner.HARNESS = runner.HARNESS.replace('copy_failures==3', 'copy_failures==2').replace('largest>4000', 'largest>0')
runner.HARNESS = runner.HARNESS.replace('static bool deny_create=', '''static std::uint64_t invalid_category=0, breaches=0;
static bool deny_cause=false;
#undef cod_lang_exception_cause
extern "C" void cod_lang_exception_cause(std::uint64_t cause,std::uint64_t index,std::uint64_t *result){
  assert(deny_cause);deny_cause=false;++breaches;
  for(unsigned i=0;i<4;++i)result[i]=0;
  result[4]=invalid_category;
}
static bool deny_create=''')
runner.HARNESS = runner.HARNESS.replace('if(value==9000)', '''if(value>=9060 && value<9063){
    deny_cause=true;invalid_category=value==9060?0:value==9061?8:(std::uint64_t{1}<<32)+5;return;
  }
  if(value==9000)''').replace('assert(created+copied==released', 'assert(!deny_cause && breaches==3);\n  assert(created+copied==released')
runner.EXPECTED_VALUES = ['6500', '9', '1', '8', '7', '1', '7', '6', '9', '6', '9', '6', '9']
runner.EXTRA_SOURCES += [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
