#!/usr/bin/env python3
"""Loop exception invariants retain native recovery and exact resource release."""
from pathlib import Path
from native_test_support import run_native
import cleanup_payload_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
{{
 val invariant:Exception<-missing_value(),changing:Exception<-missing_value();
 for(i in 0u64..2u64){
   try{throw invariant;}catch(e:const Exception){observe(e.category);};
 };
 try{for(i in 0u64..2u64){
   try{throw changing;}catch(e:const MissingReturnError){observe(e.category);};
   changing<-null_value();
 };}catch(e:const Exception){observe(e.category);};
 for(i in 0u64..2u64){ {{defer{observe(invariant.category);};}}; };
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5','5','4','5','5'])

if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
