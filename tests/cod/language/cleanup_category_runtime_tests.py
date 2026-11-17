#!/usr/bin/env python3
"""Resolved cleanup categories preserve handler order and native resource counts."""
from pathlib import Path
from native_test_support import run_native
import loop_exception_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
{{
 val stable_cleanup_value:Exception<-missing_value();
 for(i in 0u64..2u64){ {{
   defer{try{throw stable_cleanup_value;}catch(e:const Exception){observe(e.category);};};
 }};};
 {{for(i in 0u64..2u64){
   defer{try{throw stable_cleanup_value;}catch(e:const Exception){observe(e.category);};};
 };}};
 try{ {{defer{throw stable_cleanup_value;};}}; }catch(e:const Exception){observe(e.category);};
 try{ {{
   defer catch(e:const MissingReturnError){observe(e.category);}finally{observe(9u64);};
   defer{throw stable_cleanup_value;};
 }}; }catch(e:const Exception){observe(99u64);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5','5','5','5','5','9'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
