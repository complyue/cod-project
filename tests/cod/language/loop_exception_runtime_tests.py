#!/usr/bin/env python3
"""Loop exception invariants retain native recovery and exact resource release."""
from pathlib import Path
import cleanup_payload_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
{{
 val invariant:Exception<-missing_value(),changing:Exception<-missing_value();
 for(i in 0u64..2u64){
   try{throw invariant;}catch(e:const MissingReturnError){observe(e.category);};
 };
 try{for(i in 0u64..2u64){
   try{throw changing;}catch(e:const MissingReturnError){observe(e.category);};
   changing<-null_value();
 };}catch(e:const Exception){observe(e.category);};
 for(i in 0u64..2u64){ {{defer{observe(invariant.category);};}}; };
}};
'''
runner.EXPECTED_VALUES += ['5','5','5','4','5','5']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
