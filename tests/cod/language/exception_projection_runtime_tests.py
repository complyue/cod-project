#!/usr/bin/env python3
"""Cleanup preserves saved exception values and reads current selected targets."""
from pathlib import Path
import exception_alias_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
{{
 val payload:Exception<-missing_value();ptr selected=payload;
 try{throw (selected=try{payload}finally{payload<-null_value();{}});}catch(e:const Exception){observe(e.category);};
 payload<-missing_value();
 try{throw try{payload}finally{payload<-null_value();{}};}catch(e:const MissingReturnError){observe(e.category);};
 observe(payload.category);
 payload<-missing_value();
 try{throw (selected={{defer{payload<-null_value();{}};payload}});}catch(e:const Exception){observe(e.category);};
 try{{const val saved:Exception<-missing_value();defer{throw saved;};}}catch(e:const MissingReturnError){observe(e.category);};
 try{{try{throw missing_value();}catch(caught){defer{{ptr saved=caught;throw(saved=caught);}};};}}catch(e:const MissingReturnError){observe(e.category);};
}};
'''
runner.EXPECTED_VALUES += ['4','5','4','4','5','5']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
