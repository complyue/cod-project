#!/usr/bin/env python3
"""Category predicates preserve exact borrowed/deferred resource release."""
from pathlib import Path
import call_initialization_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
using "std:exceptions" Exception;
func failure()->const MissingReturnError{
 try{missing();}catch(e:const MissingReturnError){return e;};return failure();
}
func classify(value:Exception)->bool{
 try{throw value;}catch(e:const MissingReturnError){return true;}catch(e:const Exception){return false;};
}
proc fill(ptr output:u64 in O,value:u64,payload:Exception)->void throws NullPointerException where {owner O}{
 if(classify(payload)){output<-value;{}}else{};
 try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){output<-value;{}};
}
'''
for before, after in (
    ('x<-71u64;', 'fill(x,71u64,failure());'),
    ('cleanup(ptr x:u64 in O)', 'cleanup(ptr x:u64 in O,payload:Exception)'),
    ('defer{x<-74u64;};', 'defer{if(classify(payload)){x<-74u64;{}}else{};try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){x<-74u64;{}};};'),
    ('cleanup(b);', 'cleanup(b,failure());'),
    ('alias(ptr x:u64 in O,ptr y:u64 in O)', 'alias(ptr x:u64 in O,ptr y:u64 in O,payload:Exception)'),
    ('defer{x<-75u64;observe(y);};', 'defer{if(classify(payload)){x<-75u64;{}}else{};try{throw payload;}catch(e:const MissingReturnError){}catch(e:const Exception){x<-75u64;{}};observe(y);};'),
    ('alias(c,c);', 'alias(c,c,failure());'),
):
    assert before in runner.SOURCE
    runner.SOURCE = runner.SOURCE.replace(before, after)
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
