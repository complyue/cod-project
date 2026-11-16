#!/usr/bin/env python3
"""Nested deferred catches retain distinct saved and arriving exception values."""
from pathlib import Path
import exception_return_runtime_tests as fixture

runner = fixture.runner
runner.SOURCE += '''
func caught_copy(input:Exception)->const Exception{try{throw input;}catch(e){return e;};}
func catch_saved(input:const Exception,replacement:const Exception)->const Exception throws Exception{
 defer catch(last){return last;};
 defer catch(original){defer catch(current){throw original;};throw replacement;};
 throw input;
}
func catch_current(input:const Exception,replacement:const Exception)->const Exception throws Exception{
 defer catch(original){defer catch(current){return current;};throw replacement;};
 throw input;
}
func catch_mutation(ref target:Exception in O,replacement:Exception)->const Exception where {owner O}{
 try{ {{defer{target<-replacement;{}};throw target;}}; }catch(e){return e;};
}
func catch_nested_call(input:const Exception,replacement:const Exception)->const Exception throws Exception{
 defer catch(original){ {{defer catch(current){return caught_copy(original);};throw replacement;}};};
 throw input;
}
{{
 val payload:Exception<-missing_value(),replacement:Exception<-null_value();
 try{throw caught_copy(payload);}catch(e:const Exception){observe(e.category);};
 try{throw catch_saved(payload,replacement);}catch(e:const Exception){observe(e.category);};
 try{throw catch_current(payload,replacement);}catch(e:const Exception){observe(e.category);};
 try{throw catch_mutation(payload,replacement);}catch(e:const Exception){observe(e.category);};
 observe(payload.category);
 payload<-missing_value();
 try{throw catch_nested_call(payload,replacement);}catch(e:const Exception){observe(e.category);};
}};
'''
runner.EXPECTED_VALUES += ['5','5','4','5','4','5']
runner.EXTRA_SOURCES += [Path(fixture.__file__),Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
