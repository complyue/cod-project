#!/usr/bin/env python3
"""Independent callable result evidence, recursive anchors and return-boundary joins."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
MISSING = 'using "std:exceptions" MissingReturnException;'
runner.CASES = {
 'self_literal_anchor': (IO+'func sum(n:u64){if(n==0u64){return 0u64;}else{};return n+sum(n-1u64);}behave @print(sum(30u64));',0,'465\n',''),
 'self_reverse_anchor': (IO+'func sum(n:u64){if(n!=0u64){return n+sum(n-1u64);}else{};return 0u64;}behave @print(sum(30u64));',0,'465\n',''),
 'mutual_forwarding': (IO+'func a(n:u64){return b(n);}func b(n:u64){if(n==0u64){return 3u64;}else{return a(n-1u64);};}behave @print(a(30u64));',0,'3\n',''),
 'parameter_anchor': (IO+'func f(x:T,n:u64) where T{if(n==0u64){return x;}else{return f(x,n-1u64);};}behave @print(f(true,30u64));behave @print(f(2u32,30u64));',0,'true\n2\n',''),
 'nonrecursive_anchor': (IO+'func plus(x:u64)->u64{return x+1u64;}func f(x:u64,n:u64){if(n==0u64){return plus(x);}else{return f(x,n-1u64);};}behave @print(f(5u64,30u64));',0,'6\n',''),
 'transitive_join': (IO+'func f(flag:bool){if(flag){return 1u32;}else{return g();};}func g(){return 2u64;}func tag(x:u32)->u64{return 32u64;}func tag(x:u64)->u64{return 64u64;}behave @print(tag(f(true)));behave @print(f(false));',0,'64\n2\n',''),
 'numeric_cross_join': (IO+'func f(flag:bool){if(flag){return 16777217u32;}else{return 1f32;};}func tag(x:f64)->u64{return 64u64;}behave @print(tag(f(true)));behave @print(f(true));behave @print(f(false));',0,'64\n16777217\n1\n',''),
 'numeric_reverse_join': (IO+'func f(flag:bool){if(flag){return 1f32;}else{return 16777217u32;};}func tag(x:f64)->u64{return 64u64;}behave @print(tag(f(false)));behave @print(f(false));',0,'64\n16777217\n',''),
 'join_preserves_earlier_overflow': (IO+'func f(flag:bool){if(flag){return 4294967295u32+1u32;}else{return 1u64;};}f(true);',70,'','u32 addition overflow'),
 'no_lossless_join': ('func f(flag:bool){if(flag){return 1;}else{return 1u64;};}',2,'','no unique least common result'),
 'incompatible_bool_join': ('func f(flag:bool){if(flag){return true;}else{return 1u64;};}',2,'','no unique least common result'),
 'unanchored_self': ('func f(n:u64){return n+f(n-1u64);}',2,'','unproved callable result'),
 'unanchored_mutual': ('func a(n:u64){return b(n);}func b(n:u64){return a(n);}',2,'','unproved callable result'),
 'recursive_void_completion_without_termination_proof': ('proc f(){f();}proc g:proc()->void=f;',0,'',''),
 'unresolved_value_cannot_be_void': ('func f(flag:bool){if(flag){return f(flag);}else{};}',2,'','unproved callable result'),
 'throw_only_requires_contract': (MISSING+'func f(e:MissingReturnException){throw e;}',2,'','unproved callable result'),
 'unreachable_anchor_cannot_prove_cycle': ('func f(){return f();return 1u64;}',2,'','unproved callable result'),
 'dependent_return_must_satisfy_anchor': ('func f(flag:bool){if(flag){return 1u32;}else{return f(true)+1u64;};}',2,'','return type'),
 'ordinary_body_has_no_implicit_value': ('func f(){1u64}func g:func()->void=f;g();',0,'',''),
 'void_call_and_return': (IO+'proc f(){behave @print(1u64);}proc g(){f();return;}g();',0,'1\n',''),
 'missing_value_keeps_runtime_rule': (IO+MISSING+'func f(flag:bool){if(flag){return 3u64;}else{};}try{f(false);{}}catch(e:const MissingReturnException){behave @print(4u64);};',0,'4\n','[-Wmissing-return]'),
 'deferred_return_anchor': (IO+'func f(){defer{return 7u64;};}behave @print(f());',0,'7\n',''),
 'deferred_replacement_join': (IO+'func f(){defer{return 8u64;};return 1u32;}behave @print(f());',0,'8\n',''),
 'deferred_recovery_anchor': (IO+MISSING+'func missing()->u64{}func f(){defer catch(e:const MissingReturnException){return 7u64;};missing();}behave @print(f());',0,'7\n','[-Wmissing-return]'),
 'finally_return_anchor': (IO+'func f(){try{}finally{return 6u64;};}behave @print(f());',0,'6\n',''),
 'value_snapshot_before_cleanup': (IO+'func f(x:i64 in O) where {owner O}{defer{x<-9;};return x;}val x<-1;ref r=f(x);behave @print(r);behave @print(x);',0,'1\n9\n',''),
 'copied_callable': (IO+'func f(x:u64){return x+1u64;}func a=f;func b=a;behave @print(b(3u64));',0,'4\n',''),
 'generic_callback': (IO+'func f(x:u64){return x+1u64;}func apply(f:func(T)->const T,x:T) where T{return f(x);}behave @print(apply(f,3u64));',0,'4\n',''),
 'captured_callable_result': (IO+'func add(x:u64){return x+1u64;}func wrap(f:func(u64)->u64){return func[f](x:u64){return f(x)+1u64;};}func f=wrap(add);behave @print(f(3u64));',0,'5\n',''),
 'retained_generic_context': (IO+'func outer(x:T) where T{func local(y:T){return y;};return local(x);}behave @print(outer(3u64));behave @print(outer(true));',0,'3\ntrue\n',''),
 'lexical_self_recursion': (IO+'func outer(x:u64){func sum(n:u64){if(n==0u64){return 0u64;}else{return n+sum(n-1u64);};};return sum(x);}behave @print(outer(30u64));',0,'465\n',''),
 'query_uses_inferred_signature': (IO+'func f(x:u64){return x;}val x<-make local typeof(f(1u64)){5u64};behave @print(x);',0,'5\n',''),
 'selected_generic_callable': (IO+'func f(x:T) where T{return x;}func g:func(u64)->u64=f;behave @print(g(9u64));',0,'9\n',''),
 'deferred_inferred_call': (IO+'proc f(x:u64){behave @print(x);}defer f(8u64);behave @print(1u64);',0,'1\n8\n',''),
 'receiving_storage_not_result_evidence': ('func f(){return 1u64;}val x:u32<-f();',2,'','initializer does not satisfy declared val type'),
 'local_annotation_anchor': (IO+'func f(n:u64){val x:u64<-if(n==0u64){1u64}else{f(n-1u64)};return x;}behave @print(f(30u64));',0,'1\n',''),
}
for kind in ('func','proc'):
 for anonymous in (False,True):
  name=kind+('_anonymous' if anonymous else '_named')
  definition=(kind+' f='+kind+'(x:u64){return x+1u64;};') if anonymous else (kind+' f(x:u64){return x+1u64;}')
  runner.CASES[name]=(IO+definition+'behave @print(f(3u64));',0,'4\n','')
  definition=(kind+' f='+kind+'(){1u64};') if anonymous else (kind+' f(){1u64}')
  runner.CASES[name+'_void']=(definition+'f();',0,'','')
runner.CASES.update({
 'logical_recursive_operand_is_not_anchor': ('func f(){return f()&&true;}',2,'','unproved callable result'),
 'logical_independent_short_circuit': (IO+'func f(flag:bool){return flag&&f(flag);}behave @print(f(false));',0,'false\n',''),
 'logical_or_independent_short_circuit': (IO+'func f(flag:bool){return flag||f(flag);}behave @print(f(true));',0,'true\n',''),
 'same_owner_selection': (IO+'func pick(x:i64 in O,y:i64 in O)->u32 where {owner O}{return 1u32;}func pick(x:i64 in O,y:i64 in P)->u64 where {owner O,owner P}{return 2u64;}func f(x:i64 in O) where {owner O}{return pick(x,x);}func tag(x:u32)->u64{return 32u64;}val x<-1;behave @print(tag(f(x)));',0,'32\n',''),
 'unknown_owner_selection': ('func pick(x:i64 in O,y:i64 in O)->u32 where {owner O}{return 1u32;}func pick(x:i64 in O,y:i64 in P)->u64 where {owner O,owner P}{return 2u64;}func f(x:i64 in O,y:i64 in P) where {owner O,owner P}{return pick(x,y);}',2,'','unproved callable result'),
})
runner.CASES['common_exception_parameter_context'] = (IO+MISSING+'using "std:exceptions" SystemException;func missing()->i64{}func error()->const MissingReturnException{try{missing();}catch(e:const MissingReturnException){return e;};}func tag(x:SystemException)->u64{return 64u64;}func tag(x:MissingReturnException)->u32{return 32u32;}func outer(e:SystemException){return tag(e);}func expect(x:u64)->u64{return x;}val e<-error();behave @print(expect(outer(e)));val v<-make local typeof(outer(e)){7u64};behave @print(v);',0,'64\n7\n','[-Wmissing-return]')
runner.CASES['recursive_void_cleanup_completion'] = ('proc f(){defer{f();};}proc g:proc()->void=f;',0,'','')
runner.CASES['recursive_void_scoped_completion'] = ('func f(){true;{{f();}};}func g:func()->void=f;',0,'','')
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
 raise SystemExit(runner.main())
