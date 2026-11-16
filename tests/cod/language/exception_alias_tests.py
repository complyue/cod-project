#!/usr/bin/env python3
"""Potentially aliased borrowed writes preserve checked exception obligations."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" Exception;using "std:exceptions" MissingReturnError;using "std:exceptions" NullPointerException;'
ERROR = TYPES+'func missing()->u64{}func unchecked()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}'
ERROR += 'func checked()->const NullPointerException{ptr p:u64=null in here;try{p;{}}catch(e:const NullPointerException){return e;};return checked();}'
WARN = '[-Wmissing-return]'
COVER = 'requires a covering catch or throws declaration'
HELPER = 'func replace(ref target:Exception in O,value:NullPointerException)->void where {owner O}{target<-value;}'
runner.CASES = {}
for owners, roles in [('same', ('O', 'O', 'owner O')), ('independent', ('O', 'P', 'owner O,owner P'))]:
    left, right, where = roles
    signature = f'func change(ref x:Exception in {left},ref y:Exception in {right},a:MissingReturnError,b:NullPointerException)'
    for name, body in {
        'write': 'x<-a;y<-b;throw x;',
        'copy_after_write': 'x<-a;y<-b;val saved<-x;throw saved;',
        'alias_write': 'var selected->y;x<-a;selected<-b;throw x;',
        'readonly_alias': 'var observed:const->x;x<-a;y<-b;throw observed;',
        'swap': 'x<-a;val other:Exception<-b;y<->other;throw x;',
        'helper': 'x<-a;replace(y,b);throw x;',
        'callable_helper': 'func writer=replace;x<-a;writer(y,b);throw x;',
        'conditional_write': 'x<-a;val flag<-true;if(flag){y<-b;{}}else{};throw x;',
        'finally_write': 'x<-a;try{}finally{y<-b;{}};throw x;',
        'prepared_write': 'x<-a;defer (<-)(y,b);throw x;',
    }.items():
        runner.CASES[owners+'_'+name] = (ERROR+HELPER+signature+'->void where {'+where+'}{'+body+'}',2,'',COVER)
    body = 'x<-a;y<-b;throw x;'
    method = signature+'->void throws NullPointerException where {'+where+'}{'+body+'}'
    invoke = 'val first:Exception<-unchecked(),second:Exception<-unchecked();try{change(first,DEST,unchecked(),checked());}catch(e:const NullPointerException){behave @print(4);}catch(e:const MissingReturnError){behave @print(5);};'
    runner.CASES[owners+'_declared_distinct'] = (IO+ERROR+method+invoke.replace('DEST','second'),0,'5\n',WARN)
    runner.CASES[owners+'_declared_same'] = (IO+ERROR+method+invoke.replace('DEST','first'),0,'4\n',WARN)
    snapshot = signature+'->void where {'+where+'}{x<-a;val saved<-x;y<-b;throw saved;}'
    runner.CASES[owners+'_copy_before_write'] = (IO+ERROR+snapshot+'val first:Exception<-unchecked();try{change(first,first,unchecked(),checked());}catch(e:const MissingReturnError){behave @print(first.category);};',0,'4\n',WARN)

runner.CASES.update({
    'independent_local_payload': (IO+ERROR+'func change(ref x:Exception in O,a:MissingReturnError,b:NullPointerException)->void where {owner O}{val local:Exception<-a;x<-b;throw local;}val first:Exception<-unchecked();try{change(first,unchecked(),checked());}catch(e:const MissingReturnError){behave @print(first.category);};',0,'4\n',WARN),
    'independent_value_parameter': (IO+ERROR+'func change(snapshot:Exception,ref target:Exception in O,a:MissingReturnError,b:NullPointerException)->void where {owner O}{snapshot<-a;target<-b;throw snapshot;}val first:Exception<-unchecked();try{change(first,first,unchecked(),checked());}catch(e:const MissingReturnError){behave @print(first.category);};',0,'4\n',WARN),
    'direct_locals_remain_precise': (IO+ERROR+'val x:Exception<-unchecked(),y:Exception<-checked();x<->y;try{throw x;}catch(e:const NullPointerException){behave @print(4);};try{throw y;}catch(e:const MissingReturnError){behave @print(5);};',0,'4\n5\n',WARN),
    'generic_borrow_alias': (ERROR+'func change(ref x:T in O,ref y:T in P,a:MissingReturnError,b:NullPointerException)->void where {T,owner O,owner P}{x<-a;y<-b;throw x;}val x:Exception<-unchecked();change(x,x,unchecked(),checked());',2,'',COVER),
    'readonly_access_not_escalated': (ERROR+'func change(ref x:const Exception in O,a:MissingReturnError)->void where {owner O}{x<-a;}',2,'','readonly destination'),
    'borrowed_upcast_still_unproved': (ERROR+'func change(ref x:Exception in O)->void where {owner O}{}val x<-unchecked();change(x);',2,'','borrowed exception upcasts'),
})
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
