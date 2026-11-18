#!/usr/bin/env python3
"""Prepared builtin swaps preserve queue ownership and failure atomicity."""
from pathlib import Path
from native_test_support import run_native
import exception_value_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])

FIXTURE = FIXTURE.updated(source='''
using symbol "std:io" @print;
using "std:exceptions" OutOfMemoryError;
using "std:exceptions" NullPointerException;
using "std:exceptions" MissingReturnError;
proc observe(x:u64)->void{behave @print(x);}
func missing()->const u64{}
proc swap_in_helper(ref left:u64 in O,ref right:u64 in P)->void where {owner O,owner P}{defer (<->)(left,right);}
proc batch(n:u64)->void{
 val x<-11u64,y<-12u64,z<-13u64;
 defer{observe(x);observe(y);observe(z);};
 for(i in 0u64..n){
  if(i%2u64==0u64){defer (<->)(x,y);}else{defer (<->)(y,z);};
 };
 x<-21u64;
}
batch(0u64);observe(9000u64);
batch(3u64);observe(9000u64);
batch(10000u64);observe(9000u64);
proc per_iteration(n:u64)->void{
 val x<-1u64,y<-2u64;
 for(i in 0u64..n){{defer intrinsic (<->)(x,y);}};
 observe(x);observe(y);
}
per_iteration(10000u64);observe(9000u64);
{{val x<-1u64,y<-2u64;defer{observe(x);observe(y);};defer{swap_in_helper(x,y);};}};
observe(9000u64);
proc denied_registration()->void{
 val x<-1u64,y<-2u64,z<-3u64;ptr p=x,q=y;
 {{
  defer (<->)(x,y);
  observe(9050u64);
  try{defer (<->)({observe(70u64);p},{p=z;observe(71u64);q});}
  catch(e:const OutOfMemoryError){observe(9051u64);};
  observe(x);observe(y);observe(p);
 }};
 observe(x);observe(y);observe(p);
}
denied_registration();observe(9000u64);
proc null_preparation()->void{
 val x<-1u64;ptr p:u64=null in here;
 observe(9040u64);
 try{defer (<->)(p,{observe(75u64);x});}
 catch(e:const NullPointerException){observe(9041u64);};
 observe(x);
}
null_preparation();observe(9000u64);
proc partial_batch(n:u64)->void{
 val x<-1u64,y<-2u64;
 defer{observe(x);observe(y);};
 observe(9060u64);
 for(i in 0u64..n){defer (<->)(x,y);};
}
try{partial_batch(1000u64);}catch(e:const OutOfMemoryError){observe(9061u64);};
observe(9000u64);
{{
 val x<-1u64,y<-2u64;
 defer catch(e:const MissingReturnError){observe(x);observe(y);}finally{observe(78u64);};
 defer (<->)(x,y);
 missing();{}
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(wrappers='''
#define cod_lang_defer_allocate cod_record_allocate
#define cod_lang_defer_free cod_record_free
#define cod_lang_exception_finish cod_finish
#define cod_lang_print_u64 cod_trace
''')
FIXTURE = FIXTURE.updated(harness=r'''
#undef cod_lang_defer_allocate
#undef cod_lang_defer_free
#undef cod_lang_exception_finish
#undef cod_lang_print_u64
#include <cassert>
#include <map>
static std::map<void *,std::uint64_t> records;
static std::uint64_t created=0,released=0,failures=0,attempts=0,checkpoints=0,baseline=0;
static std::uint64_t bytes=0,peak_bytes=0,peak_records=0,largest=0;
static std::int64_t budget=-1;
extern "C" void *cod_lang_defer_allocate(std::uint64_t size,std::uint64_t *status){
  ++attempts;
  if(budget==0){++failures;status[0]=1;return nullptr;}
  if(budget>0)--budget;
  auto *record=cod_record_allocate(size,status);
  assert(record && records.emplace(record,size).second);
  ++created;bytes+=size;
  if(bytes>peak_bytes)peak_bytes=bytes;
  if(records.size()>peak_records)peak_records=records.size();
  if(size>largest)largest=size;
  return record;
}
extern "C" void cod_lang_defer_free(void *record){
  const auto found=records.find(record);assert(found!=records.end());
  bytes-=found->second;records.erase(found);++released;cod_record_free(record);
}
extern "C" void cod_lang_print_u64(std::uint64_t value){
  if(value==9000){assert(records.empty() && bytes==0 && budget==-1);++checkpoints;return;}
  if(value==9050){assert(budget==-1 && records.size()==1);baseline=created;budget=0;return;}
  if(value==9051){assert(budget==0 && records.size()==1 && created==baseline && failures==1);budget=-1;return;}
  if(value==9040){assert(records.empty());baseline=attempts;return;}
  if(value==9041){assert(records.empty() && attempts==baseline);return;}
  if(value==9060){assert(budget==-1 && records.size()==1);baseline=created;budget=3;return;}
  if(value==9061){assert(budget==0 && records.empty() && created==baseline+3 && failures==2);budget=-1;return;}
  cod_trace(value);
}
extern "C" void cod_lang_exception_finish() noexcept{
  assert(records.empty() && bytes==0 && budget==-1 && checkpoints==9);
  assert(created==20016 && released==created && failures==2 && attempts==created+failures);
  assert(peak_records==10001 && largest>16 && peak_bytes==10000*largest+16);
  cod_finish();
  std::printf("{\"records\":%llu,\"released\":%llu,\"failures\":%llu,\"peak_records\":%llu,\"peak_bytes\":%llu,\"largest\":%llu}\n",
      (unsigned long long)created,(unsigned long long)released,(unsigned long long)failures,
      (unsigned long long)peak_records,(unsigned long long)peak_bytes,(unsigned long long)largest);
}
''')


def batch_result(n):
    values = [21, 12, 13]
    for i in reversed(range(n)):
        a, b = (0, 1) if i % 2 == 0 else (1, 2)
        values[a], values[b] = values[b], values[a]
    return list(map(str, values))


FIXTURE = FIXTURE.updated(expected_values=batch_result(0) + batch_result(3) + batch_result(10000))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['1', '2', '2', '1', '70', '71', '1', '2', '3', '2', '1', '3', '75', '1', '2', '1', '2', '1', '78'])
FIXTURE = FIXTURE.updated(expected_shadows=[])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
