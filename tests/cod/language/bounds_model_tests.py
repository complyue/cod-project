#!/usr/bin/env python3
"""Compare cause indexing with independent finite-sequence bounds semantics."""
from pathlib import Path
import bounds_tests as fixture
runner = fixture.runner
runner.CASES = {}
for length in (0,1,2,3,17,64):
    values = list(range(length-1,-1,-1))
    indices = sorted({0,length//2,max(0,length-1),length,length+1,2**64-1})
    source = fixture.PRELUDE+'val e<-seed();val history<-e;for(i in 0u64..'+str(length)+'u64){history<-link(e,history);};'
    source += 'func read(e:SystemException,i:u64)->u64{try{return cause_at(e,i).cause_count;}catch(x:const IndexOutOfBoundsException){return 100u64+x.available;};}'
    expected = []
    for index in indices:
        source += fixture.OUT('read(history,'+str(index)+'u64)')
        expected.append(str(values[index] if index<len(values) else 100+len(values)))
    runner.CASES['length_'+str(length)] = (source,0,'\n'.join(expected)+'\n',fixture.WARN)
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
