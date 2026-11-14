#!/usr/bin/env python3
"""External scalar views across calls, recursion, returns and cleanup."""
from pathlib import Path
import sys
import foundation_tests as runner

IO = 'using symbol "std:io" @print;\n'
PRE = IO + 'using "std:exceptions" NullPointerException;\n'
ID = 'func id(ref x:T in O)->ref T in O where {T,owner O}{return x;}\n'
PTR = 'func forward(ptr x:i64 in O)->ptr i64 in O where {owner O}{return x;}\n'
runner.CASES = {
    'external_write': (IO + 'proc set(x:i64 in O)->void where {owner O}{x<-4;}val a=1;set(a);behave @print(a);', 0, '4\n', ''),
    'function_write': (IO + 'func set(ref x:i64 in O)->const i64 where {owner O}{x<-5;return x;}val a=1;behave @print(set(a));behave @print(a);', 0, '5\n5\n', ''),
    'default_copy_unchanged': (IO + 'proc set(x:i64)->void{x<-6;}val a=1;set(a);behave @print(a);', 0, '1\n', ''),
    'local_parameter_rebind': (IO + 'proc f(x:i64 in O,y:i64 in O)->void where {owner O}{x=y;x<-7;}val a=1,b=2;ptr p=a;f(p,b);behave @print(p);behave @print(b);', 0, '1\n7\n', ''),
    'shared_parameter_target': (IO + 'func f(x:i64 in O,y:i64 in O)->const i64 where {owner O}{x<-8;return y;}val a=1;behave @print(f(a,a));', 0, '8\n', ''),
    'same_owner_requirement': ('proc f(x:i64 in O,y:i64 in O)->void where {owner O}{}val a=1;{{val b=2;f(a,b);}};', 2, '', 'different actual owners'),
    'independent_owners': (IO + 'proc f(x:i64 in D,y:i64 in S)->void where {owner D,owner S}{x<-y;}val a=1;{{val b=9;f(a,b);}};behave @print(a);', 0, '9\n', ''),
    'independent_owner_rebind': ('proc f(x:i64 in D,y:i64 in S)->void where {owner D,owner S}{x=y;}', 2, '', 'initializer-fixed owner'),
    'unknown_parameter_owner': ('proc f(x:i64 in O)->void{}', 2, '', 'unknown parameter owner'),
    'unknown_result_owner': ('func f(x:i64)->ref i64 in O{return x;}', 2, '', 'unknown result owner'),
    'owner_is_not_value': ('func f(x:i64 in O)->const i64 where {owner O}{return O;}', 2, '', 'owner binding'),
    'owner_shadows_symbol': ('using symbol "std:io" @print as @O;proc f(x:i64 in O)->void where {owner O}{behave @O(1);}', 2, '', 'nearest binding'),
    'owner_duplicate_parameter': ('proc f(O:i64 in O)->void where {owner O}{}', 2, '', 'duplicate declaration'),
    'readonly_argument': ('proc f(x:i64 in O)->void where {owner O}{}const val a=1;f(a);', 2, '', 'readonly argument'),
    'readonly_parameter': ('proc f(x:const i64 in O)->void where {owner O}{x<-1;}', 2, '', 'readonly destination'),
    'readonly_read': (IO + 'func f(x:const i64 in O)->const i64 where {owner O}{return x;}const val a=10;behave @print(f(a));', 0, '10\n', ''),
    'const_parameter_binding': ('proc f(const x:i64 in O,y:i64 in O)->void where {owner O}{x=y;}', 2, '', 'const reference binding'),
    'const_binding_writable_target': (IO + 'proc f(const x:i64 in O)->void where {owner O}{x<-11;}val a=1;f(a);behave @print(a);', 0, '11\n', ''),
    'fresh_borrow_argument_gap': ('proc f(x:i64 in O)->void where {owner O}{}f(1);', 2, '', 'existing target views'),
    'uninitialized_null_test': ('func f(ptr x:i64 in O)->const bool where {owner O}{return isnull(x);}val a=make local i64;f(a);', 0, '', ''),
    'ownerless_borrow_gap': ('func f(ptr x:i64 in O)->const bool where {owner O}{return isnull(x);}ptr p:i64=null;f(p);', 2, '', 'one established owner'),
    'expired_borrow': ('proc f(x:i64 in O)->void where {owner O}{}f({{val a=1;a}});', 2, '', 'lexical owner has ended'),
    'nullable_forwarding': (IO + PTR + 'val a=12;ptr p=a;p=null;ptr q=forward(p);behave @print(isnull(q));p=a;ptr r=forward(p);if(!isnull(r)){behave @print(r);}else{};', 0, 'true\n12\n', ''),
    'nullable_callee_read': (PRE + 'func f(ptr x:i64 in O)->const i64 throws NullPointerException where {owner O}{return x;}val a=1;ptr p=a;p=null;try{behave @print(f(p));}catch(e:const NullPointerException){behave @print(4);};', 0, '4\n', ''),
    'nullable_callee_missing_throws': (PRE + 'func f(ptr x:i64 in O)->const i64 where {owner O}{return x;}', 2, '', 'requires a covering catch or throws'),
    'declared_throws_preserved': (PRE + 'func f(ptr x:i64 in O)->const bool throws NullPointerException where {owner O}{return isnull(x);}val a=1;f(a);', 2, '', 'requires a covering catch or throws'),
    'callee_null_test': (IO + 'func f(ptr x:i64 in O)->const i64 where {owner O}{if(isnull(x)){return 0;}else{return x;};}val a=13;ptr p=a;behave @print(f(p));p=null;behave @print(f(p));', 0, '13\n0\n', ''),
    'ordinary_borrow_reads_before_later_argument': (PRE + 'proc f(x:i64 in O,y:i64)->void where {owner O}{behave @print(99);}val a=1;ptr p=a;p=null;try{f(p,{behave @print(98);1});}catch(e:const NullPointerException){behave @print(4);};', 0, '4\n', ''),
    'ref_argument_nonnull': ('proc f(ref x:i64 in O)->void where {owner O}{}val a=1;ptr p=a;p=null;f(p);', 2, '', 'proven non-null target'),
    'selected_address_before_later_argument': (IO + 'proc f(ref x:i64 in O,y:bool)->void where {owner O}{x<-14;}val a=1,b=2;ptr p=a;f(p,{p=b;true});behave @print(a);behave @print(p);', 0, '14\n2\n', ''),
    'borrowed_result_write': (IO + ID + 'val a=1;id(a)<-15;behave @print(a);', 0, '15\n', ''),
    'borrowed_result_alias': (IO + ID + 'val a=1;ref q=id(a);a<-16;behave @print(q);', 0, '16\n', ''),
    'borrowed_result_copy': (IO + ID + 'val a=1;val q=id(a);a<-17;behave @print(q);', 0, '1\n', ''),
    'nullable_result_read': (PRE + PTR + 'val a=1;ptr p=a;p=null;try{behave @print(forward(p));}catch(e:const NullPointerException){behave @print(4);};', 0, '4\n', ''),
    'nullable_result_write': (PRE + PTR + 'val a=1;ptr p=a;p=null;try{forward(p)<-{behave @print(99);2};{}}catch(e:const NullPointerException){behave @print(4);};', 0, '4\n', ''),
    'null_result_retains_owner': (PTR + 'val a=1;ptr p=a;p=null;ptr q=forward(p);{{val b=2;q=b;}};', 2, '', 'initializer-fixed owner'),
    'result_scope_expiry': (ID + 'ref q={{val a=1;id(a)}};', 2, '', 'lexical owner has ended'),
    'return_local_rejected': ('func f(x:i64 in O)->ref i64 in O where {owner O}{val a=1;return a;}', 2, '', 'declared external owner'),
    'fresh_result_gap': ('func f(x:i64 in O)->ref i64 in O where {owner O}{return x+1;}', 2, '', 'fresh ordinary borrowed results'),
    'ref_result_null_rejected': ('func f(ptr x:i64 in O)->ref i64 in O where {owner O}{return x;}', 2, '', 'proven non-null target'),
    'return_null': (IO + 'func f(x:i64 in O)->ptr i64 in O where {owner O}{return null;}val a=1;behave @print(isnull(f(a)));', 0, 'true\n', ''),
    'return_readonly': ('func f(x:const i64 in O)->ref i64 in O where {owner O}{return x;}', 2, '', 'readonly target'),
    'readonly_result': ('func f(x:const i64 in O)->ref const i64 in O where {owner O}{return x;}val a=1;f(a)<-2;', 2, '', 'readonly destination'),
    'cleanup_borrowed_value': (IO + 'func f(x:i64 in O)->ref i64 in O where {owner O}{defer{x<-18;};return x;}val a=1;behave @print(f(a));', 0, '18\n', ''),
    'cleanup_copied_value': (IO + 'func f(x:i64 in O)->const i64 where {owner O}{defer{x<-19;};return x;}val a=1;behave @print(f(a));behave @print(a);', 0, '1\n19\n', ''),
    'cleanup_address_snapshot': (IO + 'func f(x:i64 in O,y:i64 in O)->ref i64 in O where {owner O}{defer{x=y;};return x;}val a=1,b=2;f(a,b)<-20;behave @print(a);behave @print(b);', 0, '20\n2\n', ''),
    'cleanup_replaces_borrowed_result': (IO + 'func f(x:i64 in O,y:i64 in O)->ref i64 in O where {owner O}{defer{return y;};return x;}val a=1,b=2;f(a,b)<-21;behave @print(a);behave @print(b);', 0, '1\n21\n', ''),
    'finally_replaces_borrowed_result': (IO + 'func f(x:i64 in O,y:i64 in O)->ref i64 in O where {owner O}{try{return x;}finally{return y;};}val a=1,b=2;f(a,b)<-22;behave @print(a);behave @print(b);', 0, '1\n22\n', ''),
    'prepared_borrow_snapshot': (IO + 'proc f(x:i64 in O)->void where {owner O}{x<-23;}val a=1,b=2;ptr p=a;{{defer f(p);p=b;}};behave @print(a);behave @print(p);', 0, '23\n2\n', ''),
    'prepared_borrow_observes_late_payload': (IO + 'proc f(x:i64 in O)->void where {owner O}{behave @print(x);}val a=1;{{defer f(a);a<-24;}};', 0, '24\n', ''),
    'prepared_nullable_borrow': (PRE + 'proc f(ptr x:i64 in O)->void throws NullPointerException where {owner O}{behave @print(x);}val a=1;ptr p=a;p=null;try{{defer f(p);p=a;{}}}catch(e:const NullPointerException){behave @print(4);};', 0, '4\n', ''),
    'prepared_borrow_expiry': ('using "std:exceptions" MissingReturnException;proc f(x:const MissingReturnException in O)->void where {owner O}{}func bad()->const i64{}try{bad();{}}catch(e:const MissingReturnException){defer f(e);};', 2, '', 'does not outlive'),
    'recursive_borrow': (IO + 'func f(x:i64 in O,n:u64)->ref i64 in O where {owner O}{if(n==0u64){return x;}else{return f(x,n-1u64);};}val a=1;f(a,5u64)<-25;behave @print(a);', 0, '25\n', ''),
    'mutual_generic_borrow': (IO + 'func f(ref x:T in O,n:u64)->ref T in O where {T,owner O}{if(n==0u64){return x;}else{return g(x,n-1u64);};}func g(ref x:T in O,n:u64)->ref T in O where {T,owner O}{return f(x,n);}' + 'val a=26,b=true,c=2.5f64;behave @print(f(a,4u64));behave @print(f(b,4u64));behave @print(f(c,4u64));', 0, '26\ntrue\n2.5\n', ''),
    'generic_nullable_forwarding': (IO + 'func f(ptr x:T in O)->ptr T in O where {T,owner O}{return x;}func g(ptr x:T in O)->const bool where {T,owner O}{return isnull(f(x));}val a=1,b=true;ptr p=a;p=null;behave @print(g(p));behave @print(g(b));', 0, 'true\nfalse\n', ''),
    'loop_owner_call': (IO + ID + 'for(i in 0..2){ {{val a=27;behave @print(id(a));}};};', 0, '27\n27\n', ''),
    'mixed_generic_value_consumption': (IO+'func pick(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func pick(ref x:bool in O)->const bool where {owner O}{return x;}func wrap(ref x:T in O)->const T where {T,owner O}{return pick(x);}val a=1,b=true;behave @print(wrap(a));behave @print(wrap(b));', 0, '1\ntrue\n', ''),
    'generic_nullable_result_guard': (PRE + 'func pick(ref x:i64 in O)->ref i64 in O where {owner O}{return x;}func pick(ref x:bool in O)->ptr bool in O where {owner O}{return null;}func wrap(ref x:T in O)->const T throws NullPointerException where {T,owner O}{return pick(x);}val a=28,b=true;try{behave @print(wrap(a));behave @print(wrap(b));}catch(e:const NullPointerException){behave @print(4);};', 0, '28\n4\n', ''),
    'different_preparation_types': (IO+'func pick(x:i64)->const i64{return x;}func pick(ref x:bool in O)->const bool where {owner O}{return x;}val a=1,b=false;behave @print(pick(a));behave @print(pick(b));', 0, '1\nfalse\n', ''),
    'owner_overload_selection': (IO+'proc f(x:i64 in O,y:i64 in O)->void where {owner O}{behave @print(1);}proc f(x:i64 in A,y:i64 in B)->void where {owner A,owner B}{behave @print(2);}val a=3,b=4;f(a,b);{{val c=5;f(a,c);}};', 0, '1\n2\n', ''),
    'borrow_exception_value': (IO + 'using "std:exceptions" MissingReturnException;func bad()->const i64{}' + 'func id(ref x:const T in O)->ref const T in O where {T,owner O}{return x;}' + 'try{bad();{}}catch(e:const MissingReturnException){behave @print(id(e).category);};', 0, '5\n', 'callable may finish without an explicit return'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    sys.exit(runner.main())
