#!/usr/bin/env python3
"""Complementary identity guards preserve recovery writes and exact native release."""
from pathlib import Path
from native_test_support import run_native
import call_initialization_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func same(ptr left:const u64 in O,ptr right:const u64 in O)->bool where {owner O}{return left is right;}
proc verify(first:bool,second:bool,fail:bool)->void{
 val left_cell<-make local u64,right_cell<-make local u64,output<-make local u64;
 ptr left=if(first){left_cell}else{right_cell};
 ptr right=if(second){left_cell}else{right_cell};
 {{
  if(fail){observe(9001u64);}else{};
  try{defer{
   if(same(left,right)){output<-87u64;{}}else{};
   if(!same(left,right)){output<-88u64;{}}else{};
  };}catch(e:const OutOfMemoryError){
   observe(9002u64);
   if(left is right){output<-87u64;{}}else{};
   if(left is not right){output<-88u64;{}}else{};
  };
 }};
 observe(output);
}
''')
for fail in (False,True):
    for first,second in ((False,False),(False,True),(True,False),(True,True)):
        FIXTURE = FIXTURE.updated(source=FIXTURE.source + 'verify('+','.join(str(v).lower() for v in (first,second,fail))+');\n')
        FIXTURE = FIXTURE.updated(expected_values=(*FIXTURE.expected_values, '87' if first==second else '88'))
assert 'created==5 && peak==1 && failures==1' in FIXTURE.harness
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('created==5 && peak==1 && failures==1','created==9 && peak==1 && failures==5'))
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
