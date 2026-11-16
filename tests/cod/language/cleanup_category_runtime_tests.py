#!/usr/bin/env python3
"""Resolved cleanup categories preserve handler order and native resource counts."""
from pathlib import Path
import loop_exception_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
{{
 val stable_cleanup_value:Exception<-missing_value();
 for(i in 0u64..2u64){ {{
   defer{try{throw stable_cleanup_value;}catch(e:const MissingReturnError){observe(e.category);};};
 }};};
 {{for(i in 0u64..2u64){
   defer{try{throw stable_cleanup_value;}catch(e:const MissingReturnError){observe(e.category);};};
 };}};
 try{ {{defer{throw stable_cleanup_value;};}}; }catch(e:const MissingReturnError){observe(e.category);};
 {{
   defer catch(e:const MissingReturnError){observe(e.category);}finally{observe(9u64);};
   defer{throw stable_cleanup_value;};
 }};
}};
'''
runner.EXPECTED_VALUES += ['5','5','5','5','5','5','9']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
