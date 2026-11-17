#!/usr/bin/env python3
"""Selected cell writes preserve exact native cleanup and registration recovery."""
from pathlib import Path
import call_initialization_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE+='''
func same(ptr left:const u64 in O,ptr right:const u64 in O)->bool where {owner O}{return left is right;}
proc verify(first:bool,second:bool,fail:bool)->void{
 val left_cell<-make local u64,right_cell<-make local u64;
 ptr selected=if(first){left_cell}else{right_cell};
 {{
  if(fail){observe(9001u64);}else{};
  try{defer{
   if(if(second){selected is left_cell}else{same(selected,left_cell)}){
    left_cell<-87u64;{}
   }else{right_cell<-88u64;{}};
  };}catch(e:const OutOfMemoryError){
   observe(9002u64);
   if(selected is left_cell){left_cell<-87u64;{}}else{right_cell<-88u64;{}};
  };
 }};
 observe(selected);
}
'''
for fail in (False,True):
    for first,second in ((False,False),(False,True),(True,False),(True,True)):
        runner.SOURCE+='verify('+','.join(str(v).lower() for v in (first,second,fail))+');\n'
        runner.EXPECTED_VALUES.append('87' if first else '88')
assert 'created==5 && peak==1 && failures==1' in runner.HARNESS
runner.HARNESS=runner.HARNESS.replace('created==5 && peak==1 && failures==1','created==9 && peak==1 && failures==5')
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
