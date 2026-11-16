#!/usr/bin/env python3
"""Aliased exception writes preserve native recovery and resource accounting."""
from pathlib import Path
import exception_borrow_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
proc change_alias(ref x:Exception in O,ref y:Exception in P,a:MissingReturnError,b:NullPointerException)->void throws Exception where {owner O,owner P}{
 x<-a;y<-b;try{throw x;}finally{observe(71u64);};
}
{{
 val x:Exception<-missing_value(),y:Exception<-missing_value();
 try{change_alias(x,y,missing_value(),null_value());}catch(e:const MissingReturnError){observe(72u64);}catch(e:const NullPointerException){observe(73u64);}catch(e:const Exception){observe(99u64);};
 try{change_alias(x,x,missing_value(),null_value());}catch(e:const MissingReturnError){observe(72u64);}catch(e:const NullPointerException){observe(73u64);}catch(e:const Exception){observe(99u64);};
}};
'''
runner.EXPECTED_VALUES += ['71','72','71','73']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
