#!/usr/bin/env python3
"""Nested handler queues preserve ordering, failure cleanup and native release."""
from pathlib import Path
from native_test_support import run_native
import handler_payload_runtime_tests as fixture

FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func recover_queued(ref target:Exception in O,item:MissingReturnError,n:u64)->void where {owner O}{
 defer catch(e:const MissingReturnError){defer write_payload(target,e,10u64);};
 if(n==0u64){throw item;}else{recover_queued(target,item,n-1u64);};
}
proc failed_nested_queue(ref target:Exception in O)->void where {owner O}{
 defer catch(e:const MissingReturnError){
  defer{target<-e;{}};
  observe(9050u64);
  defer{target<-null_value();{}};
 };
 throw missing_value();
}
{{
 val payload:Exception<-null_value(),item:Exception<-null_value(),failure<-missing_value();
 {{defer catch(e:const MissingReturnError){defer (<-)(payload,e);};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 {{defer catch(e:const MissingReturnError){defer write_payload(payload,e,10u64);}finally{defer (<-)(payload,item);};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 {{defer catch(e:const MissingReturnError){for(i in 0u64..3u64){defer (<-)(payload,e);};};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer catch(e:const MissingReturnError){defer (<-)(payload,item);};}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();recover_queued(payload,failure,20u64);
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 try{failed_nested_queue(payload);}catch(e:const OutOfMemoryError){observe(9051u64);observe(payload.category);};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
}};
''')
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['5','5','5','5','5','5','5'])
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('record_failures==2','record_failures==3'))
if __name__=='__main__':
    raise SystemExit(run_native(FIXTURE))
