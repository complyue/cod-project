#!/usr/bin/env python3
"""Exception-domain overlap preserves independent categories, results and failures."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" SystemException;using "std:exceptions" NullPointerException;using "std:exceptions" MissingReturnException;'
ERROR = TYPES+'func missing()->u64{}func missing_value()->const MissingReturnException{try{missing();}catch(caught:const MissingReturnException){return caught;};}'
ERROR += 'func null_value()->const NullPointerException{ptr absent:u64=null in here;try{absent;{}}catch(caught:const NullPointerException){return caught;};return null_value();}'
PAIR = 'func f(a:T,b:T)->bool where T;'
BODY = 'func f(a:SystemException,b:SystemException)->bool{return true;}'
FAILURE = 'func f(input:MissingReturnException)->bool throws NullPointerException;func f(input:SystemException)->bool{return true;}'
OUT = lambda text: 'behave @print('+text+');'
runner.CASES = {
 'same_category': (IO+ERROR+PAIR+BODY+OUT('f(missing_value(),missing_value())'),0,'true\n',''),
 'distinct_categories': (IO+ERROR+PAIR+BODY+OUT('f(missing_value(),null_value())'),0,'true\n',''),
 'independent_common_inputs': (IO+ERROR+PAIR+BODY+'val left:SystemException<-missing_value(),right:SystemException<-null_value();'+OUT('f(left,right)'),0,'true\n',''),
 'independent_body_result': (IO+ERROR+PAIR+'func f(a:SystemException,b:SystemException){return true;}'+OUT('f(missing_value(),null_value())'),0,'true\n',''),
 'partial_cannot_anchor_cycle': (TYPES+PAIR+'func f(a:SystemException,b:SystemException){return f(a,b);}',2,'','unproved callable result'),
 'covering_result_anchors_cycle': (TYPES+PAIR+'func f(a:SystemException,b:SystemException)->bool;func f(a:SystemException,b:SystemException){return f(a,b);}func selected=f;',0,'',''),
 'dependent_result_remains_unproved': (TYPES+'func f(a:T,b:T)->const T where T;func f(a:SystemException,b:SystemException)->const SystemException{return a;}',2,'','declared result type parameter is not determined'),
 'narrow_result_promise': (TYPES+'func f(input:MissingReturnException)->const MissingReturnException;func f(input:SystemException)->const SystemException{return input;}',2,'','does not satisfy callable declaration'),
 'conflicting_results': (TYPES+'func f(input:MissingReturnException)->bool;func f(input:NullPointerException)->u64;func f(input:SystemException)->bool{return true;}',2,'','does not satisfy callable declaration'),
 'concrete_input_establishes_result': (IO+ERROR+'func f(a:T,b:T)->const T where T;func f(a:SystemException,b:MissingReturnException)->const MissingReturnException{return b;}val common:SystemException<-null_value();'+OUT('f(common,missing_value()).category'),0,'5\n',''),
 'later_common_input_establishes_result': (IO+ERROR+'func f(a:T,b:T)->const T where T;func f(a:MissingReturnException,b:SystemException)->const MissingReturnException{return a;}val common:SystemException<-null_value();'+OUT('f(missing_value(),common).category'),0,'5\n',''),
 'independent_method_binders': (IO+ERROR+PAIR+'func f(a:A,b:B)->bool where {A,B}{return true;}func relay(a:SystemException,b:SystemException)->bool{return f(a,b);}'+OUT('relay(missing_value(),null_value())'),0,'true\n',''),
 'independent_binders_cannot_anchor_cycle': (TYPES+PAIR+'func f(a:A,b:B) where {A,B}{return f(a,b);}func relay(a:SystemException,b:SystemException)->bool{return f(a,b);}',2,'','unproved callable result'),
 'repeated_binder_anchors_cycle': (TYPES+PAIR+'func f(a:U,b:U) where U{return f(a,b);}func relay(input:SystemException)->bool{return f(input,input);}',0,'',''),
 'repeated_method_binder': (IO+ERROR+'func f(a:T,b:T)->const T where T;func f(a:U,b:U)->const U where U{return b;}func relay(input:SystemException)->const SystemException{return f(input,input);}'+OUT('relay(missing_value()).category'),0,'5\n',''),
 'checked_failure': (ERROR+FAILURE+'f(missing_value());',2,'','requires a covering catch or throws'),
 'disjoint_checked_failure': (IO+ERROR+FAILURE+OUT('f(null_value())'),0,'true\n',''),
 'unknown_category_retains_failure': (ERROR+FAILURE+'val common:SystemException<-null_value();f(common);',2,'','requires a covering catch or throws'),
 'caught_failure_contract': (IO+ERROR+FAILURE+'try{'+OUT('f(missing_value())')+'}catch(caught:const NullPointerException){};',0,'true\n',''),
 'same_category_failure': (ERROR+'func f(a:T,b:T)->bool throws NullPointerException where T;'+BODY+'f(missing_value(),missing_value());',2,'','requires a covering catch or throws'),
 'distinct_category_failure': (IO+ERROR+'func f(a:T,b:T)->bool throws NullPointerException where T;'+BODY+OUT('f(missing_value(),null_value())'),0,'true\n',''),
 'unknown_relation_retains_failure': (ERROR+'func f(a:T,b:T)->bool throws NullPointerException where T;'+BODY+'val left:SystemException<-missing_value(),right:SystemException<-null_value();f(left,right);',2,'','requires a covering catch or throws'),
 'query_does_not_invoke': (IO+ERROR+FAILURE+'val result<-make local typeof(f(missing_value())){true};'+OUT('result'),0,'true\n',''),
 'inferred_complete_failure': (IO+ERROR+FAILURE+'func selected=f;try{'+OUT('selected(null_value())')+'}catch(caught:const NullPointerException){};',0,'true\n',''),
 'complete_requires_failure': (ERROR+FAILURE+'func selected=f;selected(null_value());',2,'','requires a covering catch or throws'),
 'narrow_complete_excludes_failure': (IO+ERROR+FAILURE+'func selected:func(NullPointerException)->bool=f;'+OUT('selected(null_value())'),0,'true\n',''),
 'narrow_complete_requires_failure': (ERROR+FAILURE+'func selected:func(MissingReturnException)->bool=f;',2,'','callable view omits a required checked exception'),
 'returned_complete_failure': (IO+ERROR+FAILURE+'func factory(){return f;}func selected=factory();try{'+OUT('selected(null_value())')+'}catch(caught:const NullPointerException){};',0,'true\n',''),
 'nested_common_relation_unproved': (TYPES+'func f(input:T,callback:func(T)->u64)->u64 where T;func f(input:SystemException,callback:func(SystemException)->u64)->u64{return 7u64;}',2,'','borrowed or nested common type'),
 'nested_concrete_relation': (IO+ERROR+'func f(input:T,callback:func(T)->u64)->const T where T;func f(input:SystemException,callback:func(MissingReturnException)->u64)->const MissingReturnException{return missing_value();}func tag(value:MissingReturnException)->u64{return value.category;}val common:SystemException<-null_value();'+OUT('f(common,tag).category'),0,'5\n',''),
 'borrowed_common_overlap_unproved': (TYPES+'func f(ref a:const T in A,ref b:const T in B)->bool where {T,owner A,owner B};func f(ref a:const SystemException in A,ref b:const SystemException in B)->bool where {owner A,owner B}{return true;}',2,'','independent borrowed input-domain proof'),
 'borrowed_unrelated_result': (IO+ERROR+'func f(a:T,b:T,ref slot:i64 in O)->ref i64 in O where {T,owner O};func f(a:SystemException,b:SystemException,ref slot:i64 in O)->ref i64 in O where {owner O}{return slot;}val target<-3;f(missing_value(),null_value(),target)<-7;'+OUT('target'),0,'7\n',''),
 'variadic_categories': (IO+ERROR+'func f(rest:T...)->bool where T;func f(a:SystemException,b:SystemException,c:SystemException)->bool{return true;}'+OUT('f(missing_value(),null_value(),missing_value())'),0,'true\n',''),
 'deferred_snapshot': (IO+ERROR+'proc show(a:T,b:T)->void where T;proc show(a:SystemException,b:SystemException)->void{'+OUT('a.category')+OUT('b.category')+'}val left:SystemException<-missing_value(),right:SystemException<-null_value();{{defer show(left,right);left<-right;}};',0,'5\n4\n',''),
 'deferred_disjoint_failure': (ERROR+FAILURE+'{{defer f(null_value());}};',0,'',''),
 'deferred_failure_required': (ERROR+FAILURE+'defer f(missing_value());',2,'','requires a covering catch or throws'),
 'operator_overlap': (IO+ERROR+'func (+)(a:T,b:T)->bool where T;func (+)(a:SystemException,b:SystemException)->bool{return true;}'+OUT('missing_value()+null_value()'),0,'true\n',''),
 'imported_declaration': ({'main.cod':IO+ERROR+'import "./lib.cod" f;func f(a:SystemException,b:SystemException)->bool{return true;}'+OUT('f(missing_value(),null_value())'),'lib.cod':'func f(a:T,b:T)->bool where T;export f;'},0,'true\n','[-Wmissing-return]'),
}
parameters = [f'item{i}' for i in range(12)]
declaration = 'func many('+','.join(name+':T' for name in parameters)+')->bool where T;'
definition = 'func many('+','.join(name+':SystemException' for name in parameters)+')->bool{return true;}'
inputs = [f'input{i}' for i in range(12)]
values = ''.join('val '+name+':SystemException<-'+('missing_value()' if i % 2 else 'null_value()')+';' for i,name in enumerate(inputs))
runner.CASES['many_independent_categories'] = (IO+ERROR+declaration+definition+values+OUT('many('+','.join(inputs)+')'),0,'true\n','')
runner.CASES['many_unresolved_dependent_categories'] = (
    TYPES+declaration.replace('->bool','->const T')+
    definition.replace('->bool{return true;}', '->const SystemException{return item0;}'),
    2,'','declared result type parameter is not determined')
for name, (source, status, output, diagnostic) in list(runner.CASES.items()):
    if isinstance(source, str) and ERROR in source and status == 0:
        runner.CASES[name] = (source,status,output,'[-Wmissing-return]')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
