#!/usr/bin/env python3
"""Reaching callable values constrain initialization and native selection."""
from pathlib import Path
import foundation_tests as runner

PRE='using symbol "std:io" @print;using "std:exceptions" NullPointerException;\n'
METHODS='proc set(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-7;}proc other(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x<-8;}proc ignore(ptr x:i64 in O)->void throws NullPointerException where {owner O}{}\n'
BASE=PRE+METHODS+'proc unrelated=ignore;proc selected=set;val x=make local i64;\n'
OBS='behave @print(x);'
TRY=lambda body:'try{'+body+'}catch(e:const NullPointerException){};'
runner.CASES={
 'unrelated_implementation':(BASE+TRY('selected(x);'+OBS),0,'7\n',''),
 'expression_selection':(BASE+TRY('(set)(x);'+OBS),0,'7\n',''),
 'slot_alias':(BASE+'var alias->selected;'+TRY('alias(x);'+OBS),0,'7\n',''),
 'pointer_holder':(BASE+'ptr alias=selected;'+TRY('alias(x);'+OBS),0,'7\n',''),
 'independent_copy':(BASE+'proc copied=selected;'+TRY('copied(x);'+OBS),0,'7\n',''),
 'conditional_selection':(BASE+'proc chosen=if(false){selected}else{other};'+TRY('chosen(x);'+OBS),0,'8\n',''),
 'scoped_selection':(BASE+'proc chosen={{proc local=selected;local}};'+TRY('chosen(x);'+OBS),0,'7\n',''),
 'selected_nonwriter':(BASE+TRY('unrelated(x);'+OBS),2,'','before definite initialization'),
 'mixed_implementations_rejected':(BASE+'proc chosen=if(false){selected}else{unrelated};'+TRY('chosen(x);'+OBS),2,'','before definite initialization'),
 'selected_read_obligation':(PRE+METHODS+'proc read(ptr x:i64 in O)->void throws NullPointerException where {owner O}{x;{}}proc selected=read;val x=make local i64;'+TRY('selected(x);'),2,'','cannot prove initialization'),
 'generic_argument':(BASE+'proc invoke(f:F,ptr x:i64 in O)->void throws NullPointerException where {F,owner O}{f(x);}'+TRY('invoke(selected,x);'+OBS),0,'7\n',''),
 'recursive_argument':(BASE+'proc invoke(f:F,ptr x:i64 in O,n:i64)->void throws NullPointerException where {F,owner O}{if(n==0){f(x);}else{invoke(f,x,n-1);};}'+TRY('invoke(selected,x,40);'+OBS),0,'7\n',''),
 'mutual_argument':(BASE+'proc invoke(f:F,ptr x:i64 in O,n:i64)->void throws NullPointerException where {F,owner O}{if(n==0){f(x);}else{relay(f,x,n-1);};}proc relay(f:F,ptr x:i64 in O,n:i64)->void throws NullPointerException where {F,owner O}{invoke(f,x,n);}'+TRY('invoke(selected,x,40);'+OBS),0,'7\n',''),
 'captured_argument':(BASE+'proc wrap=proc[selected](ptr y:i64 in O)->void throws NullPointerException where {owner O}{selected(y);};'+TRY('wrap(x);'+OBS),0,'7\n',''),
 'capture_from_named_definition':(BASE+'proc wrap=proc[set](ptr y:i64 in O)->void throws NullPointerException where {owner O}{set(y);};'+TRY('wrap(x);'+OBS),0,'7\n',''),
 'returned_argument':(BASE+'func identity(f:F)->const F where F{return f;}proc chosen=identity(selected);'+TRY('chosen(x);'+OBS),0,'7\n',''),
 'recursive_return':(BASE+'func identity(f:F,n:i64)->const F where F{if(n==0){return f;}else{return identity(f,n-1);};}proc chosen=identity(selected,40);'+TRY('chosen(x);'+OBS),0,'7\n',''),
 'returned_capture':(BASE+'func wrap(f:F)->const F where F{return proc[f](ptr y:i64 in O)->void throws NullPointerException where {owner O}{f(y);};}proc chosen=wrap(selected);'+TRY('chosen(x);'+OBS),0,'7\n',''),
 'nested_captures':(BASE+'func wrap(f:F)->const F where F{return proc[f](ptr y:i64 in O)->void throws NullPointerException where {owner O}{f(y);};}proc chosen=wrap(wrap(selected));'+TRY('chosen(x);'+OBS),0,'7\n',''),
 'growing_captures':(BASE+'func wrap(f:F)->const F where F{return proc[f](ptr y:i64 in O)->void throws NullPointerException where {owner O}{f(y);};}for(i in 0..20){{selected<-wrap(selected);}};'+TRY('selected(x);'+OBS),0,'7\n',''),
 'deferred_selection':(BASE+TRY('0;{{defer selected(x);}};'+OBS),0,'7\n',''),
 'deferred_higher_order':(BASE+'proc invoke(f:F,ptr x:i64 in O)->void throws NullPointerException where {F,owner O}{defer f(x);}'+TRY('invoke(selected,x);'+OBS),0,'7\n',''),
 'delayed_selection':(BASE+TRY('0;{{defer{selected(x);};}};'+OBS),0,'7\n',''),
 'borrowed_holder_replacement':(BASE+'proc install(ref f:F in O,replacement:F)->void where {F,owner O}{f<-replacement;}proc next=other;install(selected,next);'+TRY('selected(x);'+OBS),0,'8\n',''),
 'borrowed_nonwriter_replacement_rejected':(BASE+'proc install(ref f:F in O,replacement:F)->void where {F,owner O}{f<-replacement;}install(selected,unrelated);'+TRY('selected(x);'+OBS),2,'','before definite initialization'),
 'returned_holder_replacement_rejected':(BASE+'func holder(ref f:F in O)->ref F in O where {F,owner O}{return f;}holder(selected)<-unrelated;'+TRY('selected(x);'+OBS),2,'','before definite initialization'),
 'pointer_retarget_rejected':(BASE+'ptr p=selected;p=unrelated;'+TRY('p(x);'+OBS),2,'','before definite initialization'),
 'later_loop_replacement_rejected':(BASE+'for(i in 0..2){'+TRY('selected(x);'+OBS)+'selected<-unrelated;};',2,'','before definite initialization'),
 'deferred_holder_mutation_rejected':(BASE+'proc install(ref f:F in O,replacement:F)->void where {F,owner O}{defer{f<-replacement;};}install(selected,unrelated);'+TRY('selected(x);'+OBS),2,'','before definite initialization'),
 'reference_swap_rejected':(BASE+'ptr p=selected;ptr q=unrelated;p<=>q;'+TRY('p(x);'+OBS),2,'','before definite initialization'),
 'declared_throws_remains':(BASE+'selected(x);',2,'','requires a covering catch'),
}
runner.CASE_ARGUMENTS={}
runner.EXTRA_SOURCES=[Path(__file__)]
if __name__=='__main__':
    raise SystemExit(runner.main())
