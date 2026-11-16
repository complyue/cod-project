#!/usr/bin/env python3
"""Borrowed exception postconditions preserve native failure and cleanup effects."""
from pathlib import Path
import exception_catch_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func write_payload(ref target:Exception in O,item:Exception,n:u64)->void where {owner O}{
 if(n==0u64){target<-item;{}}else{write_payload(target,item,n-1u64);};
}
func write_then_fail(ref target:Exception in O,item:MissingReturnError)->void where {owner O}{
 target<-item;throw item;
}
func write_checked(ref target:Exception in O)->void where {owner O}{target<-null_value();}
func write_finally(ref target:Exception in O)->void where {owner O}{
 try{return;}finally{target<-missing_value();{}};
}
{{
 val payload:Exception<-null_value(),failure<-missing_value();
 write_payload(payload,failure,40u64);
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 try{write_then_fail(payload,failure);}catch(e:const MissingReturnError){observe(payload.category);};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 write_checked(payload);
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 write_finally(payload);
 try{throw payload;}catch(e:const Exception){observe(e.category);};
}};
'''
runner.EXPECTED_VALUES += ['5','5','5','4','5']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
