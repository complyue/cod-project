#!/usr/bin/env python3
"""Exception facts distinguish saved values from later selected-target reads."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" Exception;using "std:exceptions" MissingReturnError;using "std:exceptions" NullPointerException;'
ERROR = TYPES+'func missing()->u64{}func unchecked()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};return unchecked();}'
ERROR += 'func checked()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};return checked();}'
BASE = 'val payload:Exception<-unchecked();ptr selected=payload;'
WARN = '[-Wmissing-return]'
COVER = 'requires a covering catch or throws declaration'
SELECT = 'try{payload}finally{payload<-checked();{}}'
runner.CASES = {}
for name, expression in {
    'finally': 'selected='+SELECT,
    'scope_cleanup': 'selected={{defer{payload<-checked();{}};payload}}',
    'conditional': 'selected=if(true){'+SELECT+'}else{payload}',
    'block': 'selected={payload<-checked();payload}',
    'nested_assignment': 'selected=(selected='+SELECT+')',
    'explicit_call': '(=)(selected,'+SELECT+')',
    'intrinsic_call': 'intrinsic (=)(selected,'+SELECT+')',
}.items():
    runner.CASES[name+'_requires_cover'] = (ERROR+BASE+'throw ('+expression+');',2,'',COVER)
    runner.CASES[name+'_covered'] = (IO+ERROR+BASE+'try{throw ('+expression+');}catch(e:const Exception){behave @print(e.category);};',0,'4\n',WARN)

runner.CASES.update({
    'saved_guarded_value': (IO+ERROR+BASE+'try{throw '+SELECT+';}catch(e:const Exception){behave @print(e.category);};behave @print(payload.category);',0,'5\n4\n',WARN),
    'saved_assignment_value': (IO+ERROR+BASE+'try{throw try{selected=payload}finally{payload<-checked();{}};}catch(e:const Exception){behave @print(e.category);};behave @print(payload.category);',0,'5\n4\n',WARN),
    'saved_scope_value': (IO+ERROR+BASE+'try{throw {{defer{payload<-checked();{}};payload}};}catch(e:const Exception){behave @print(e.category);};behave @print(payload.category);',0,'5\n4\n',WARN),
    'finally_removes_checked_category': (IO+ERROR+'val payload:Exception<-checked();ptr selected=payload;try{throw (selected=try{payload}finally{payload<-unchecked();{}});}catch(e:const Exception){behave @print(e.category);};',0,'5\n',WARN),
    'selected_target_survives_rebind': (IO+ERROR+'val first:Exception<-unchecked(),second:Exception<-checked();ptr source=first,target=first;try{throw (target=try{source}finally{source=second;{}});}catch(e:const Exception){behave @print(e.category);};behave @print(target.category);behave @print(source.category);',0,'5\n5\n4\n',WARN),
    'selected_target_mutated_after_rebind': (ERROR+'val first:Exception<-unchecked(),second:Exception<-unchecked();ptr source=first,target=first;throw (target=try{source}finally{source=second;first<-checked();{}});',2,'',COVER),
    'selected_target_survives_null': (IO+ERROR+BASE+'ptr source=payload;try{throw (selected=try{source}finally{source=null;{}});}catch(e:const Exception){behave @print(e.category);};behave @print(source is null);',0,'5\ntrue\n',WARN),
    'caught_snapshot_rebind': (IO+ERROR+'try{{try{throw unchecked();}catch(caught){defer{{ptr saved=caught;throw(saved=caught);}};};}}catch(e:const Exception){behave @print(e.category);};',0,'5\n',WARN),
    'const_owned_delayed': (IO+ERROR+'try{{const val saved:Exception<-unchecked();defer{throw saved;};}}catch(e:const Exception){behave @print(e.category);};',0,'5\n',WARN),
    'const_copy_delayed': (IO+ERROR+'try{{val payload:Exception<-unchecked();const val saved<-payload;defer{throw saved;};payload<-checked();}}catch(e:const Exception){behave @print(e.category);};',0,'5\n',WARN),
    'readonly_path_is_not_immutable': (ERROR+'{{val payload:Exception<-unchecked();ptr view:const Exception=payload;defer{throw view;};payload<-checked():void;}};',2,'',COVER),
    'readonly_borrow_is_not_immutable': (ERROR+'func f(ref value:const Exception in O)->void where {owner O}{defer{throw value;};}',2,'',COVER),
    'assignment_still_has_no_slot': (ERROR+BASE+'(selected=payload)=payload;',2,'','reference operation requires a binding slot'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
