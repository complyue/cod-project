#!/usr/bin/env python3
"""Generic callback completion owns prepared images through cleanup failures."""
from pathlib import Path
import exception_value_runtime_tests as runner

runner.SOURCE = '''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemory;
using "std:exceptions" MissingReturnException;
proc observe(x:u64)->void{behave @print(x);}
func empty()->const void{}
func number()->const u64{return 41u64;}
func wrap(f:func()->const u64)->func()->const u64{
  return func[f]()->const u64{return f();};
}
proc finish(f:func()->const T)->const T where T{defer observe(1u64);f();}
func recover(f:func()->const T)->const T where T{defer{return f();};}
finish(empty);
try{finish(wrap(number));{}}catch(e:const MissingReturnException){observe(2u64);};
observe(recover(wrap(number)));recover(empty);
{{
  func f=wrap(number);observe(recover(f));observe(9000u64);
  try{finish(f);{}}catch(e:const OutOfMemory){observe(9001u64);observe(3u64);};
  observe(f());
}};
observe(9002u64);
'''
runner.WRAPPERS = '''
#define cod_lang_callable_create cod_image_create
#define cod_lang_callable_copy cod_image_copy
#define cod_lang_callable_free cod_image_free
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
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
static std::uint64_t created=0,copied=0,released=0,registered=0,freed=0,denied=0,live=0;
static bool deny=false,checkpoint=false;
extern "C" void *cod_lang_callable_create(std::uint64_t fixed,std::uint64_t count,void *const *children,
                                         const std::uint64_t *offsets,std::uint64_t *status){
  auto *image=cod_image_create(fixed,count,children,offsets,status);
  assert(image && images.insert(image).second);++created;return image;
}
extern "C" void *cod_lang_callable_copy(const void *source,std::uint64_t *status){
  if(deny){++denied;status[0]=1;return nullptr;}
  auto *image=cod_image_copy(source,status);assert(image && images.insert(image).second);++copied;return image;
}
extern "C" void cod_lang_callable_free(void *image){assert(images.erase(image)==1);++released;cod_image_free(image);}
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
  auto *record=cod_record_allocate(size,status);assert(record && records.insert(record).second);++registered;return record;
}
extern "C" void cod_lang_defer_free(void *record){assert(records.erase(record)==1);++freed;cod_record_free(record);}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9000){assert(!deny && !images.empty());live=images.size();deny=true;return;}
  if(value==9001){assert(deny && images.size()==live && records.empty());deny=false;return;}
  if(value==9002){assert(images.empty() && records.empty());checkpoint=true;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
  assert(!deny && checkpoint && images.empty() && records.empty());
  assert(created>0 && copied>0 && released==created+copied && registered==5 && freed==registered && denied==1);
  cod_finish();
  std::printf("{\"created\":%llu,\"copied\":%llu,\"released\":%llu,\"records\":%llu,\"freed\":%llu,\"denied\":%llu}\n",
      (unsigned long long)created,(unsigned long long)copied,(unsigned long long)released,
      (unsigned long long)registered,(unsigned long long)freed,(unsigned long long)denied);
}
'''
runner.EXPECTED_VALUES = ['1','1','2','41','41','3','41']
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
