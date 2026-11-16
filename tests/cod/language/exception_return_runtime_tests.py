#!/usr/bin/env python3
"""Returned exception evidence preserves native cleanup and independent values."""
from pathlib import Path
import exception_projection_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func forward_exception(item:Exception,n:u64)->const Exception{
 if(n==0u64){return item;}else{return forward_exception(item,n-1u64);};
}
func return_before_cleanup(ref item:Exception in O)->const Exception where {owner O}{
 try{return item;}finally{item<-null_value();{}};
}
func recover_return(item:const Exception)->const Exception{
 defer catch(e:const NullPointerException){};
 defer{throw null_value();};
 return item;
}
func replace_return(item:const Exception)->const Exception{
 defer{return item;};
 return null_value();
}
func return_target(ref item:Exception in O)->ref Exception in O where {owner O}{
 try{return item;}finally{item<-null_value();{}};
}
{{
 val payload:Exception<-missing_value();
 try{throw forward_exception(payload,20u64);}catch(e:const Exception){observe(e.category);};
 try{throw return_before_cleanup(payload);}catch(e:const Exception){observe(e.category);};
 observe(payload.category);
 payload<-missing_value();
 try{throw recover_return(payload);}catch(e:const Exception){observe(e.category);};
 try{throw replace_return(payload);}catch(e:const Exception){observe(e.category);};
 try{throw return_target(payload);}catch(e:const Exception){observe(e.category);};
}};
'''
runner.EXPECTED_VALUES += ['5','5','4','5','5','4']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
