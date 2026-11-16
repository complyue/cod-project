#!/usr/bin/env python3
"""Recovery writes preserve native completion ordering and exact release."""
from pathlib import Path
import cleanup_payload_runtime_tests as fixture

runner=fixture.runner
runner.SOURCE+='''
func recover_payload(ref target:Exception in O,item:MissingReturnError,n:u64)->void where {owner O}{
 defer catch(e:const MissingReturnError){target<-e;{}};
 if(n==0u64){throw item;}else{recover_payload(target,item,n-1u64);};
}
{{
 val payload:Exception<-null_value(),item:Exception<-null_value(),failure<-missing_value();
 {{defer catch(e:const MissingReturnError){payload<-e;{}};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 {{defer{payload<-null_value();{}}catch(e:const MissingReturnError){payload<-e;{}}finally{item<-null_value();{}};defer{throw failure;};}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();
 {{defer catch(e:const MissingReturnError){ {{defer (<-)(payload,e);}};};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 payload<-null_value();recover_payload(payload,failure,20u64);
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer catch(e:const MissingReturnError){payload<-null_value();{}};}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
 {{defer catch(e:const MissingReturnError){payload<-e;{}}finally{payload<-null_value();{}};throw failure;}};
 try{throw payload;}catch(e:const Exception){observe(e.category);};
}};
'''
runner.EXPECTED_VALUES+=['5','5','5','5','5','4']
runner.EXTRA_SOURCES+=[Path(fixture.__file__),Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
