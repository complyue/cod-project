#!/usr/bin/env python3
"""Common exception borrows retain payloads and cleanup under native failures."""
from pathlib import Path
import captured_callable_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
using "std:exceptions" Exception;
using "std:exceptions" NullPointerException;
func missing_value()->const MissingReturnError{try{missing();}catch(caught:const MissingReturnError){return caught;};}
func null_value()->const NullPointerException{ptr absent:u64=null in here;try{absent;{}}catch(caught:const NullPointerException){return caught;};return null_value();}
func category(ref input:const Exception in O)->u64 where {owner O}{return input.category;}
func identity(ref input:Exception in O)->ref Exception in O where {owner O}{return input;}
proc overwrite(ptr target:Exception in O,value:Exception)->void throws NullPointerException where {owner O}{target<-value;}
proc show(snapshot:Exception,ref live:const Exception in O)->void where {owner O}{observe(snapshot.category);observe(live.category);}
{{
 val common:Exception<-missing_value();
 func held=category;func select=identity;
 func captured=func[held](ref input:const Exception in O)->u64 where {owner O}{return held(input);};
 observe(9010u64);val total<-0u64;
 for(i in 0u64..10000u64){total<-total+captured(common);};
 observe(9011u64);observe(total);
 select(common)<-null_value();observe(captured(common));
 observe(9020u64);
 try{captured<-func[held](ref input:const Exception in O)->u64 where {owner O}{return held(input);};{}}
 catch(caught:const OutOfMemoryError){observe(9021u64);};
 observe(captured(common));
 observe(9030u64);
 try{captured({observe(9999u64);common});{}}catch(caught:const OutOfMemoryError){observe(9031u64);};
 observe(captured(common));
 observe(9050u64);
 try{defer captured(common);}catch(caught:const OutOfMemoryError){observe(9051u64);};
 proc writer=overwrite;
 try{writer(common,missing_value());observe(common.category);}catch(caught:const NullPointerException){};
 proc display=show;
 {{defer display(common,common);select(common)<-null_value();}};
 observe(common.category);
 val chain:Exception<-try{throw missing_value() from null_value();}catch(caught:const MissingReturnError){caught};
 observe(captured(chain));observe(chain.cause_count);observe(cause_at(chain,0u64).category);
}};
observe(9000u64);
'''
runner.HARNESS = runner.HARNESS.replace('checkpoints==5','checkpoints==6').replace('creation_failures==1','creation_failures==2').replace('copy_failures==3','copy_failures==4').replace('record_failures==1','record_failures==2')
runner.EXPECTED_VALUES += ['50000','4','4','4','5','5','4','4','5','1','4']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
