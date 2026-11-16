#!/usr/bin/env python3
"""Declared root handling obligations remain separate from actual categories."""
from pathlib import Path
import foundation_tests as runner
from exception_projection_tests import IO, ERROR, WARN

ROOT = "declared checked exception 'Exception'"
VALUE = 'val payload:Exception<-unchecked();'
runner.CASES = {}


def reject(name, body, declarations='', diagnostic=ROOT):
    runner.CASES[name] = (ERROR+declarations+body, 2, '', diagnostic)


def accept(name, body, declarations='', output='5\n'):
    runner.CASES[name] = (IO+ERROR+declarations+body, 0, output, WARN)


def covered(body):
    return 'try{ '+body+' }catch(e:const Exception){behave @print(e.category);};'


for name, setup, value in [
    ('local', VALUE, 'payload'),
    ('const_local', 'const val payload:Exception<-unchecked();', 'payload'),
    ('copied_local', VALUE+'val saved<-payload;', 'saved'),
    ('ref_view', VALUE+'ref selected=payload;', 'selected'),
    ('readonly_view', VALUE+'ref selected:const Exception=payload;', 'selected'),
    ('pointer_view', VALUE+'ptr selected=payload;', 'selected'),
    ('assignment_result', VALUE+'ref selected=payload;', '(selected=payload)'),
    ('selected_cleanup', 'val payload:Exception<-checked(),other:Exception<-unchecked();ref selected=other;{{defer{selected<-unchecked();};selected=payload;}};', 'payload'),
]:
    reject(name, setup+'throw '+value+';')
    accept(name+'_covered', setup+covered('throw '+value+';'))

reject('root_catch', 'try{throw unchecked();}catch(e:const Exception){throw e;};')
reject('implicit_root_catch', 'try{throw unchecked();}catch(e){throw;};')
reject('root_catch_bare_throw', 'try{throw unchecked();}catch(e:const Exception){throw;};')
ALIAS = 'using "std:exceptions" Exception as Failure;'
reject('root_alias', 'val value:Failure<-unchecked();throw value;', ALIAS)
accept('root_alias_covered', 'val value:Failure<-unchecked();try{throw value;}catch(e:const Failure){behave @print(e.category);};', ALIAS)
accept('nested_root_cover', covered('try{throw unchecked();}catch(inner:const Exception){throw inner;};'))
accept('concrete', 'try{throw unchecked();}catch(e:const MissingReturnError){behave @print(e.category);};')
accept('concrete_catch', 'try{try{throw unchecked();}catch(inner:const MissingReturnError){throw;};}catch(e:const MissingReturnError){behave @print(e.category);};')

HELPER = 'func keep(item:Exception)->const Exception{return item;}'
BORROW = 'func keep(ref item:const Exception in O)->ref const Exception in O where {owner O}{return item;}'
GENERIC = 'func keep(item:T)->const T where {T}{return item;}'
for name, declarations in [('helper', HELPER), ('borrowed_helper', BORROW), ('generic_helper', GENERIC)]:
    reject(name, VALUE+'throw keep(payload);', declarations)
    accept(name+'_covered', VALUE+covered('throw keep(payload);'), declarations)
accept('generic_concrete_result', 'try{throw keep(unchecked());}catch(e:const MissingReturnError){behave @print(e.category);};', GENERIC)

FORWARD = 'func forward(item:T)->void where {T}{val value:T<-unchecked();throw value;}'
reject('generic_root_operand', VALUE+'forward(payload);', FORWARD)
accept('generic_concrete_operand', 'try{forward(unchecked());}catch(e:const MissingReturnError){behave @print(e.category);};', FORWARD)
FORWARD_ROOT = 'func forward(item:Exception)->void throws Exception{throw item;}'
reject('declared_caller', 'forward(unchecked());', FORWARD_ROOT)
accept('declared_caller_covered', covered('forward(unchecked());'), FORWARD_ROOT)
EMPTY = 'func may_fail()->void throws Exception{}'
reject('empty_declared_caller', 'may_fail();', EMPTY)
accept('empty_declared_caller_covered', covered('may_fail();')+'behave @print(7);', EMPTY, '7\n')
reject('narrow_catch_unproved', VALUE+'try{throw payload;}catch(e:const MissingReturnError){};')
reject('concrete_throws_unproved', 'forward(unchecked());', 'func forward(item:Exception)->void throws MissingReturnError{val value:Exception<-unchecked();throw value;}')

reject('deferred_root', VALUE+'defer{throw payload;};')
accept('deferred_root_covered', VALUE+covered('{{defer{throw payload;};}};'))
accept('deferred_own_catch', VALUE+'defer{try{throw payload;}catch(e:const Exception){behave @print(e.category);};};')
reject('ordinary_group_does_not_cover_later_defer', VALUE+'try{defer{throw payload;};}catch(e:const Exception){};')
reject('prepared_declaration', 'defer forward(unchecked());', FORWARD_ROOT)
accept('prepared_declaration_covered', covered('{{defer forward(unchecked());}};'), FORWARD_ROOT)
reject('cross_defer_coverage_unproved', VALUE+'defer catch(e:const Exception){};defer{throw payload;};')
reject('handler_sibling_does_not_cover', VALUE+'try{throw unchecked();}catch(inner:const MissingReturnError){throw payload;}catch(e:const Exception){};')
accept('finally_replaces_exception', 'behave @print(run());', 'func run()->i64{val payload:Exception<-unchecked();try{throw payload;}finally{return 9;};}', '9\n')
reject('finally_root_throw', VALUE+'try{}finally{throw payload;};')
accept('concrete_finally_replaces_root', 'try{run();}catch(e:const MissingReturnError){behave @print(e.category);};', 'func run()->void{val payload:Exception<-unchecked();try{throw payload;}finally{throw unchecked();};}')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__), Path(__file__).with_name('exception_projection_tests.py')]
if __name__ == '__main__':
    raise SystemExit(runner.main())
