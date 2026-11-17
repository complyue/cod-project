#!/usr/bin/env python3
"""Aliased exception writes preserve native recovery and resource accounting."""
from pathlib import Path
from native_test_support import run_native
import exception_borrow_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
proc change_alias(ref x:Exception in O,ref y:Exception in P,a:MissingReturnError,b:NullPointerException)->void throws Exception where {owner O,owner P}{
 x<-a;y<-b;try{throw x;}finally{observe(71u64);};
}
{{
 val x:Exception<-missing_value(),y:Exception<-missing_value();
 try{change_alias(x,y,missing_value(),null_value());}catch(e:const MissingReturnError){observe(72u64);}catch(e:const NullPointerException){observe(73u64);}catch(e:const Exception){observe(99u64);};
 try{change_alias(x,x,missing_value(),null_value());}catch(e:const MissingReturnError){observe(72u64);}catch(e:const NullPointerException){observe(73u64);}catch(e:const Exception){observe(99u64);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['71','72','71','73'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
