#!/usr/bin/env python3
"""Cleanup preserves saved exception values and reads current selected targets."""
from pathlib import Path
from native_test_support import run_native
import exception_alias_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
{{
 val payload:Exception<-missing_value();ptr selected=payload;
 try{throw (selected=try{payload}finally{payload<-null_value();{}});}catch(e:const Exception){observe(e.category);};
 payload<-missing_value();
 try{throw try{payload}finally{payload<-null_value();{}};}catch(e:const Exception){observe(e.category);};
 observe(payload.category);
 payload<-missing_value();
 try{throw (selected={{defer{payload<-null_value();{}};payload}});}catch(e:const Exception){observe(e.category);};
 try{{const val saved:Exception<-missing_value();defer{throw saved;};}}catch(e:const Exception){observe(e.category);};
 try{{try{throw missing_value();}catch(caught){defer{{ptr saved=caught;throw(saved=caught);}};};}}catch(e:const Exception){observe(e.category);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['4','5','4','4','5','5'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
