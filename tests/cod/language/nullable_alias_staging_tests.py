#!/usr/bin/env python3
"""Copied nullable selections preserve shared generic and recursive native bodies."""
from pathlib import Path
import nullable_predicate_staging_tests as fixture

runner = fixture.runner
before = 'if(classify(payload)){WRITE;{}}else{};\n try{throw payload;}catch(e:const NullPointerException){}catch(e:const Exception){WRITE;{}};'
after = '''ptr selected=if(choose){payload}else{null in here};
 ptr saved=selected;
 val ready<-classify(selected);
 selected=null;
 if(ready){WRITE;{}}else{};
 if(saved is null){WRITE;{}}else{
  try{throw payload;}catch(e:const NullPointerException){}catch(e:const Exception){WRITE;{}};
 };'''
for old, new in (
    (before, after),
    ('value:T,payload:Exception)', 'value:T,payload:Exception,choose:bool)'),
    ('n:u64,payload:Exception)', 'n:u64,payload:Exception,choose:bool)'),
    ('fill(saved,marker,payload)', 'fill(saved,marker,payload,choose)'),
    ('step(marker,n-1u64,payload)', 'step(marker,n-1u64,payload,choose)'),
    ('step(7,DEPTHu64,error)', 'step(7,DEPTHu64,error,false)'),
    ('step(8u64,DEPTHu64,error)', 'step(8u64,DEPTHu64,error,true)'),
):
    assert old in runner.SOURCE, old
    runner.SOURCE = runner.SOURCE.replace(old, new)
runner.EXTRA_SOURCES += [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
