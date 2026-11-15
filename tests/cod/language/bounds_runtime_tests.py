#!/usr/bin/env python3
"""Bounds recovery preserves native allocation and deferred-record cleanup."""
from pathlib import Path
import captured_callable_runtime_tests as fixture
runner = fixture.runner
runner.SOURCE = 'using "std:exceptions" IndexOutOfBoundsException;using "std:exceptions" SystemException;'+runner.SOURCE
runner.SOURCE += '''
func seed()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}
func read(e:SystemException,i:u64)->u64{
 try{return cause_at(e,i).cause_count;}catch(x:const IndexOutOfBoundsException){return x.available;};
}
{{
 func kept=wrap(base);
 val e<-seed();val history<-try{throw e from e;}catch(x:const MissingReturnException){x};
 val total<-0u64;
 observe(9010u64);
 for(i in 0u64..30u64){total<-total+read(history,i%3u64);};
 observe(9011u64);observe(total);
 {{
  observe(9050u64);
  try{defer cause_at(history,1u64);}catch(x:const OutOfMemory){observe(9051u64);};
 }};
 try{ {{defer{observe(9u64);};defer cause_at(history,1u64);}};{}}
 catch(x:const IndexOutOfBoundsException){observe(x.available);observe(x.requested);};
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5','checkpoints==6').replace('record_failures==1','record_failures==2')
runner.HARNESS = 'static std::size_t causes_before_measurement=0;\n'+runner.HARNESS
runner.HARNESS = runner.HARNESS.replace('baseline=created+copied;measuring=true;',
    'baseline=created+copied;causes_before_measurement=cod_causes?cod_causes->size():0;measuring=true;')
runner.HARNESS = runner.HARNESS.replace('assert(measuring && created+copied==baseline);',
    'assert(measuring && created+copied==baseline);assert((cod_causes?cod_causes->size():0)==causes_before_measurement);')
runner.EXPECTED_VALUES += ['20','9','1','1']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
