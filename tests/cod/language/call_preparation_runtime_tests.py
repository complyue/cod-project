#!/usr/bin/env python3
"""Adapted calls preserve fresh ownership and release failed preparations."""
from pathlib import Path
import generic_callable_runtime_tests as fixture
from generic_catch_tests import IO, ERRORS

runner = fixture.runner
runner.SOURCE = IO + ERRORS + '''
using "std:exceptions" OutOfMemory;
proc observe(x:u64)->void{behave @print(x);}
func number()->const u64{return 41u64;}
func wrap(f:func()->const u64)->func()->const u64{return func[f]()->const u64{return f();};}
func make_callback()->const func()->const u64{return wrap(number);}
func read_callback(f:func()->const u64)->const u64{return f();}
func read_error(e:SystemException)->const u64{return e.category;}
func apply(make_value:func()->const U,read:func(T)->const u64)->const u64 where {T,U}{return read(make_value());}
func wrap_error(f:func(SystemException)->const u64)->func(SystemException)->const u64{
 return func[f](e:SystemException)->const u64{return f(e);};
}
proc sink(e:SystemException,f:func()->const u64)->void{observe(e.category);observe(f());}
proc late(e:SystemException,f:func()->const u64,n:i64)->void{observe(99u64);}
observe(9300u64);
observe(apply(missing_error,read_error));observe(apply(make_callback,read_callback));
observe(9301u64);
{{
 func f=wrap_error(read_error);observe(9000u64);
 try{observe(f(missing_error()));}catch(e:const OutOfMemory){observe(9001u64);observe(1u64);};
 observe(f(null_error()));
}};
observe(9002u64);
proc value=late;
try{value(missing_error(),wrap(number),missing());}catch(e:const MissingReturnException){observe(2u64);};
observe(9002u64);
proc deferred=sink;
{{defer deferred(missing_error(),wrap(number));}};
observe(9002u64);
observe(9004u64);
try{{defer deferred(missing_error(),wrap(number));}}catch(e:const OutOfMemory){observe(9005u64);observe(3u64);};
observe(9002u64);
'''
runner.HARNESS = r'''
#undef cod_lang_callable_create
#undef cod_lang_callable_copy
#undef cod_lang_callable_free
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
#include <set>
static std::set<void *> images,records;
static std::uint64_t created=0,copied=0,released=0,registered=0,freed=0,copy_failures=0,record_failures=0;
static std::uint64_t live=0,before_copied=0,checkpoints=0;
static bool deny_copy=false,deny_record=false;
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,void *const *children,
                                         const std::uint64_t *offsets,std::uint64_t *status){
  auto *image=cod_image_create(fixed,count,children,offsets,status);
  assert(image && images.insert(image).second);++created;return image;
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){
  if(deny_copy){++copy_failures;status[0]=1;return nullptr;}
  auto *image=cod_image_copy(source,status);assert(image && images.insert(image).second);++copied;return image;
}
extern "C" void cod_lang_callable_free(void *image){assert(images.erase(image)==1);++released;cod_image_free(image);}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
  if(deny_record){++record_failures;status[0]=1;return nullptr;}
  auto *record=cod_record_allocate(size,status);assert(record && records.insert(record).second);++registered;return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(records.erase(record)==1);++freed;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9300){assert(images.empty());before_copied=copied;return;}
  if(value==9301){assert(images.empty() && copied==before_copied);return;}
  if(value==9000){assert(!deny_copy && images.size()==1);live=images.size();deny_copy=true;return;}
  if(value==9001){assert(deny_copy && images.size()==live && records.empty());deny_copy=false;return;}
  if(value==9002){assert(images.empty() && records.empty());++checkpoints;return;}
  if(value==9004){assert(images.empty() && records.empty());deny_record=true;return;}
  if(value==9005){assert(deny_record && images.empty() && records.empty());deny_record=false;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
  assert(!deny_copy && !deny_record && checkpoints==4 && images.empty() && records.empty());
  assert(created==5 && copied==1 && released==created+copied && registered==1 && freed==registered);
  assert(copy_failures==1 && record_failures==1);
  cod_finish();
  std::printf("{\"created\":%llu,\"copied\":%llu,\"released\":%llu,\"records\":%llu,\"freed\":%llu,\"copy_failures\":%llu,\"record_failures\":%llu}\n",
      (unsigned long long)created,(unsigned long long)copied,(unsigned long long)released,
      (unsigned long long)registered,(unsigned long long)freed,(unsigned long long)copy_failures,(unsigned long long)record_failures);
}
'''
runner.EXPECTED_VALUES = ['5','41','1','4','2','5','41','3']
runner.EXTRA_SOURCES = [Path(__file__),Path(__file__).with_name('generic_catch_tests.py'),Path(__file__).with_name('generic_callable_runtime_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
