#!/usr/bin/env python3
"""Captured calls preserve argument-indexed initialization and returned-view facts."""
from pathlib import Path
import foundation_tests as runner

PRE = 'using symbol "std:io" @print;using "std:exceptions" NullPointerException;'
VALUES = 'func seven()->i64{return 7;}func one()->i64{return 1;}'

def program(signature, body, use):
    return PRE+VALUES+'proc run()->void{func a=seven;func b=one;'+signature+'{a();b();'+body+'};'+use+'}run();'

PTR = 'func f[a,b](ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}'
SET = 'func f[a,b](ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O throws NullPointerException where {owner O}'
READ = 'func f[a,b](ptr x:i64 in O,ptr y:i64 in O)->i64 throws NullPointerException where {owner O}'
TRY = 'catch(e:const NullPointerException){};'
WRITER = 'proc f[a,b](ptr x:i64 in O)->void throws NullPointerException where {owner O}'
PLAIN_WRITER = 'proc plain(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-5;}'
PLAIN_SELECT = 'func plain(ptr x:i64 in O,ptr y:i64 in O)->ptr i64 in O where {owner O}{return x;}'
runner.CASES = {
 'selected_write_and_return':(program(SET,'x<-a();return x;','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'7\n',''),
 'write_does_not_initialize_other_input':(program(SET,'x<-a();return x;','val x<-make local i64,y<-make local i64;try{f(x,y);behave @print(y);}'+TRY),2,'','before definite initialization'),
 'aliased_read_after_write':(program(READ,'x<-a();return y;','val x<-make local i64;try{behave @print(f(x,x));}'+TRY),0,'7\n',''),
 'unaliased_read_after_write_rejected':(program(READ,'x<-a();return y;','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),2,'','cannot prove initialization'),
 'returned_first_input':(program(PTR,'return x;','val x<-8,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'8\n',''),
 'returned_second_input':(program(PTR,'return y;','val x<-make local i64,y<-9;try{behave @print(f(x,y));}'+TRY),0,'9\n',''),
 'returned_uninitialized_input_rejected':(program(PTR,'return y;','val x<-8,y<-make local i64;try{behave @print(f(x,y));}'+TRY),2,'','before definite initialization'),
 'first_nullable_source':(program(PTR,'return x;','val x<-8,y<-9;ptr p:i64=null in here;behave @print(isnull(f(p,y)));behave @print(isnull(f(x,p)));'),0,'true\nfalse\n',''),
 'second_nullable_source':(program(PTR,'return y;','val x<-8,y<-9;ptr p:i64=null in here;behave @print(isnull(f(p,y)));behave @print(isnull(f(x,p)));'),0,'false\ntrue\n',''),
 'literal_null_is_not_an_input':(program(PTR,'return null;','val x<-make local i64,y<-make local i64;behave @print(isnull(f(x,y)));'),0,'true\n',''),
 'deferred_initialization_of_return':(program(SET,'defer{x<-a();};return x;','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'7\n',''),
 'deferred_return_replacement':(program(PTR,'defer{return y;};return x;','val x<-make local i64,y<-9;try{behave @print(f(x,y));}'+TRY),0,'9\n',''),
 'deferred_replacement_does_not_invent_write':(program(PTR,'defer{return y;};return x;','val x<-8,y<-make local i64;try{behave @print(f(x,y));}'+TRY),2,'','before definite initialization'),
 'later_cleanup_initializes_replacement':(program(SET,'defer{y<-a();};defer{return y;};return x;','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'7\n',''),
 'finally_initializes_return':(program(SET,'try{return y;}finally{y<-a();{}};','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'7\n',''),
 'saved_return_not_later_rebind':(program(SET,'defer{x=y;x<-a();};return x;','val x<-8,y<-make local i64;try{behave @print(f(x,y));}'+TRY),0,'8\n',''),
 'saved_uninitialized_return_rejected':(program(SET,'defer{x=y;x<-a();};return x;','val x<-make local i64,y<-make local i64;try{behave @print(f(x,y));}'+TRY),2,'','before definite initialization'),
 'recursive_captured_writer':(program('func f[a,b](ptr x:i64 in O,n:i64)->ptr i64 in O throws NullPointerException where {owner O}',
     'if(n==0){x<-a();return x;}else{return f(x,n-1);};','val x<-make local i64;try{behave @print(f(x,30));}'+TRY),0,'7\n',''),
 'independent_owner_inputs':(program('func f[a,b](ptr x:i64 in O,ptr y:i64 in P)->ptr i64 in P where {owner O,owner P}',
     'return y;','val x<-make local i64;{{val y<-9;try{behave @print(f(x,y));}'+TRY+'}};'),0,'9\n',''),
 'copied_callable_preserves_summary':(program(SET,'y<-a();return y;','func copy=f;val x<-make local i64,y<-make local i64;try{behave @print(copy(x,y));}'+TRY),0,'7\n',''),
 'prepared_deferred_writer':(program('proc f[a,b](ptr x:i64 in O)->void throws NullPointerException where {owner O}',
     'x<-a();','val x<-make local i64;try{ {{defer f(x);}};behave @print(x);}'+TRY),0,'7\n',''),
 'different_capture_counts_share_write_summary':(program(WRITER,'x<-a();',
     'val x<-make local i64;proc writer=plain;for(i in 0..2){try{writer(x);behave @print(x);}'+TRY+'writer<-f;};')+PLAIN_WRITER,0,'5\n7\n',''),
 'nonwriting_capture_alternative_rejected':(program(WRITER,'',
     'val x<-make local i64;proc writer=plain;for(i in 0..2){try{writer(x);behave @print(x);}'+TRY+'writer<-f;};')+PLAIN_WRITER,2,'','before definite initialization'),
 'different_capture_counts_share_return_summary':(program(PTR,'return y;',
     'val x<-8,y<-9;func select=plain;for(i in 0..2){try{behave @print(select(x,y));}'+TRY+'select<-f;};')+PLAIN_SELECT,0,'8\n9\n',''),
 'return_alternative_does_not_initialize_input':(program(PTR,'return y;',
     'val x<-8,y<-make local i64;func select=plain;for(i in 0..2){try{behave @print(select(x,y));}'+TRY+'select<-f;};')+PLAIN_SELECT,2,'','before definite initialization'),
 'different_capture_counts_preserve_alias_requirements':(program(READ,'y<-a();return x;',
     'val x<-make local i64;func read=plain;for(i in 0..2){try{behave @print(read(x,x));}'+TRY+'read<-f;};')+
     'func plain(ptr x:i64 in O,ptr y:i64 in O)->i64 throws NullPointerException where {owner O}{x<-5;return y;}',0,'5\n7\n',''),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
