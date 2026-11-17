#!/usr/bin/env python3
"""Borrowed exception postconditions preserve native failure and cleanup effects."""
from pathlib import Path
from native_test_support import run_native
import exception_catch_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
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
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5','5','4','5'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
