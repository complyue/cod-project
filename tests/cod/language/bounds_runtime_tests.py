#!/usr/bin/env python3
"""Bounds recovery preserves native allocation and deferred-record cleanup."""
from pathlib import Path
from native_test_support import run_native
import captured_callable_runtime_tests as fixture
FIXTURE = fixture.FIXTURE.updated(sources=[Path(__file__)])
FIXTURE = FIXTURE.updated(source='using "std:exceptions" IndexOutOfBoundsError;using "std:exceptions" Exception;'+FIXTURE.source)
FIXTURE = FIXTURE.updated(source=FIXTURE.source + '''
func seed()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}
func read(e:Exception,i:u64)->u64{
 try{return cause_at(e,i).cause_count;}catch(x:const IndexOutOfBoundsError){return x.available;};
}
{{
 func kept=wrap(base);
 val e<-seed();val history<-try{throw e from e;}catch(x:const MissingReturnError){x};
 val total<-0u64;
 observe(9010u64);
 for(i in 0u64..30u64){total<-total+read(history,i%3u64);};
 observe(9011u64);observe(total);
 {{
  observe(9050u64);
  try{defer cause_at(history,1u64);}catch(x:const OutOfMemoryError){observe(9051u64);};
 }};
 try{ {{defer{observe(9u64);};defer cause_at(history,1u64);}};{}}
 catch(x:const IndexOutOfBoundsError){observe(x.available);observe(x.requested);};
}};
observe(9000u64);
''')
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2'))
FIXTURE = FIXTURE.updated(harness='static std::size_t causes_before_measurement=0;\n'+FIXTURE.harness)
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('baseline=created+copied;measuring=true;',
    'baseline=created+copied;causes_before_measurement=cod_causes?cod_causes->size():0;measuring=true;'))
FIXTURE = FIXTURE.updated(harness=FIXTURE.harness.replace('assert(measuring && created+copied==baseline);',
    'assert(measuring && created+copied==baseline);assert((cod_causes?cod_causes->size():0)==causes_before_measurement);'))
FIXTURE = FIXTURE.updated(expected_values=list(FIXTURE.expected_values) + ['20','9','1','1'])
if __name__ == '__main__':
    raise SystemExit(run_native(FIXTURE))
