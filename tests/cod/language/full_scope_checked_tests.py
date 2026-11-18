#!/usr/bin/env python3
"""Checked obligations cross immediate full-scope calls after local handling."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
TYPES = 'using "std:exceptions" Exception;using "std:exceptions" NullPointerException;'
PAYLOAD = ('func sample_error()->const NullPointerException{ptr absent:u64=null in here;'
           'try{absent;{}}catch(caught:const NullPointerException){return caught;};return sample_error();}')
FORWARD = 'proc forward(val e:NullPointerException)->void throws NullPointerException{throw e;}'
DECLARED = 'proc declared()->void throws NullPointerException{}'
ROOT = 'proc broad(val e:NullPointerException)->void throws Exception{throw e;}'
NPE = "checked exception 'NullPointerException' requires a covering catch or throws declaration"
BROAD = "declared checked exception 'Exception' requires a covering catch or throws declaration"
DEFER = 'checked failures involving deferred execution require coverage within the owning pass'


def run(body, output, declarations=''):
    return IO + TYPES + PAYLOAD + declarations + body, 0, output, ''


def reject(source, diagnostic=NPE):
    return TYPES + PAYLOAD + source, 2, '', diagnostic


def query(body):
    return TYPES + 'val result<-make local typeof({{' + body + '}}){4u64};'


runner.CASES = {
    'throw_only_typed': run('val e<-sample_error();try{ {{throw e;}}; }'
                            'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n'),
    'throw_only_void_result': reject('val e<-sample_error();val result<-make local typeof({{throw e;}}){7u64};',
                                     'initializer does not satisfy local storage type'),
    'throw_only_query_has_no_reads': run('val e<-make local NullPointerException;'
                                         'val result<-make local typeof({ {{throw e;}};7u64 }){9u64};behave @print(result);', '9\n'),
    'throw_only_unchecked_query_validity': reject('val result<-make local typeof({{throw 7u64;}});',
                                                  'throw requires an intrinsic exception value'),
    'query_checks_catch_category': (query('try{}catch(e:const u64){};return 7u64;'),
        2, '', 'catch requires a selected intrinsic exception type'),
    'query_rejects_repeated_catch_category': (query('try{}catch(e:const NullPointerException){}'
        'catch(f:const NullPointerException){};return 7u64;'), 2, '', 'duplicate exception category in catch clauses'),
    'query_matches_root_and_untyped_catch_category': (query('try{}catch(e:const Exception){}catch(f){};return 7u64;'),
        2, '', 'duplicate exception category in catch clauses'),
    'query_checks_deferred_catch_category': (query('defer catch(e:const u64){};return 7u64;'),
        2, '', 'catch requires a selected intrinsic exception type'),
    'query_rejects_repeated_deferred_catch_category': (query('defer catch(e:const NullPointerException){}'
        'catch(f:const NullPointerException){};return 7u64;'), 2, '', 'duplicate exception category in catch clauses'),
    'query_checks_guarded_arm_types': (query('try{true}catch(e:const NullPointerException){7u64};return 6u64;'),
        2, '', 'try body and catch arms must have the same result type'),
    'query_uses_deferred_catch_binding_within_handler': (IO +
        query('func e()->u64{return 7u64;};defer catch(e:const NullPointerException){e.category;};return e();') +
        'behave @print(result);', 0, '4\n', ''),
    'query_checks_handlers_without_running_cleanup': (IO +
        query('defer{behave @print(88u64);};defer catch(e:const NullPointerException){};'
              'try{behave @print(99u64);}catch(e:const NullPointerException){}catch(e:const Exception){}'
              'finally{behave @print(77u64);};return 7u64;') +
        'behave @print(result);', 0, '4\n', ''),
    'ordinary_throw_only_requires_result': reject('proc plain(val e:NullPointerException) throws NullPointerException{throw e;}',
                                                  'unproved callable result'),
    'throw_only_requires_coverage': reject('val e<-sample_error();{{throw e;}};'),
    'bare_throw_remains_invalid': reject('try{sample_error();{}}catch(e:const Exception){ {{throw;}}; };',
                                         'bare throw requires an active catch'),
    'nested_checked_calls': run('proc pass(val e:NullPointerException)->void throws NullPointerException'
                                '{ {{ {{forward(e);}}; }}; }'
                                'try{pass(sample_error());}catch(caught:const NullPointerException){behave @print(caught.category);};',
                                '4\n', FORWARD),
    'checked_call_only_inside_scope': reject(DECLARED + '{{declared();}};'),
    'checked_call_covered_inside_scope': run('{{try{declared();}catch(e:const NullPointerException){};'
                                             'behave @print(7u64);}};', '7\n', DECLARED),
    'handler_output_is_checked': reject('val e<-sample_error();{{try{throw e;}catch(caught:const NullPointerException){throw caught;};}};'),
    'finally_output_is_checked': reject('val e<-sample_error();{{try{}finally{throw e;};}};'),
    'finally_replaces_checked': run('val e<-sample_error();val result<-{{try{throw e;}finally{return 9u64;};}};'
                                    'behave @print(result);', '9\n'),
    'checked_operation_in_scope': run('try{ {{ptr absent:u64=null in here;absent;{}}}; }'
                                      'catch(caught:const NullPointerException){behave @print(caught.category);};', '4\n'),
    'scope_local_catch': run('val e<-sample_error();{{try{throw e;}catch(caught:const NullPointerException)'
                            '{behave @print(caught.category);};}};', '4\n'),
    'root_throw_survives_narrower_evidence': reject('val e:Exception<-sample_error();'
                                                   'try{ {{throw e;}}; }catch(caught:const NullPointerException){};', BROAD),
    'root_call_survives_narrower_evidence': reject(ROOT + 'val e<-sample_error();'
                                                 'try{ {{broad(e);}}; }catch(caught:const NullPointerException){};', BROAD),
    'root_covered_in_enclosing_caller': run('val payload<-sample_error();'
                                           'try{ {{broad(payload);}}; }catch(caught:const Exception){behave @print(caught.category);};',
                                           '4\n', ROOT),
    'root_covered_locally': run('val e:Exception<-sample_error();{{try{throw e;}catch(caught:const Exception)'
                               '{behave @print(caught.category);};}};', '4\n'),
    'ordinary_defer_runs_before_propagation': run('val e<-sample_error();try{ {{defer{behave @print(7u64);};throw e;}}; }'
                                                 'catch(caught:const NullPointerException){behave @print(caught.category);};', '7\n4\n'),
    'ordinary_catch_within_defer': run('val e<-sample_error();'
                                      'defer {try{ {{throw e;}}; }catch(caught:const NullPointerException)'
                                      '{behave @print(caught.category);};};', '4\n'),
    'enclosing_throws_cannot_cover_deferred_scope': reject('proc sample(val e:NullPointerException)->void throws NullPointerException'
                                                         '{defer {{throw e;}};}', DEFER),
    'owning_scope_cannot_export_checked_defer': reject('proc sample(val e:NullPointerException)->void throws NullPointerException'
                                                      '{ {{defer {throw e;};}}; }', DEFER),
    'ordinary_outer_catch_cannot_cover_checked_defer': reject('val e<-sample_error();'
                                                             'try{ {{defer {throw e;};}}; }'
                                                             'catch(caught:const NullPointerException){};', DEFER),
    'legacy_deferred_handler_cannot_consume_body_failure': reject('val e<-sample_error();'
                                                                '{{defer catch(caught:const NullPointerException){};throw e;}};', DEFER),
    'legacy_deferred_handler_in_caller_cannot_consume_body_failure': reject('val e<-sample_error();'
        'defer catch(caught:const NullPointerException){};try{ {{throw e;}};}finally{};', DEFER),
    'eager_defer_preparation_uses_ordinary_catch': run('val flag<-true;val e<-sample_error();'
        'try{defer report({{if(flag){throw e;}else{return 7u64;};}});}'
        'catch(caught:const NullPointerException){behave @print(caught.category);};',
        '4\n', 'proc report(value:u64)->void{behave @print(value);}'),
    'generic_checked_interface': run('proc run(val e:T)->void where T{'
        'try{ {{throw e;}}; }catch(caught:const NullPointerException){behave @print(caught.category);};}'
        'run(sample_error());', '4\n'),
    'separate_scope_interfaces': (IO + TYPES + PAYLOAD +
        'using "std:exceptions" MissingReturnError;func missing()->u64{}'
        'func unchecked()->const MissingReturnError{try{missing();{}}catch(caught:const MissingReturnError)'
        '{return caught;};return unchecked();}'
        'proc checked_path(val e:NullPointerException)->void throws NullPointerException{ {{throw e;}}; }'
        'proc unchecked_path(val e:MissingReturnError)->void{ {{throw e;}}; }'
        'try{checked_path(sample_error());}catch(checked:const NullPointerException){behave @print(checked.category);};'
        'try{unchecked_path(unchecked());}catch(failed:const MissingReturnError){behave @print(failed.category);};',
        0, '4\n5\n', '[-Wmissing-return]'),
}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
