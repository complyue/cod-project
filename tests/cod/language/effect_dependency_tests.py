#!/usr/bin/env python3
"""Call dependencies preserve reads, writes, returned views and captured symbols."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
NPE = 'using "std:exceptions" NullPointerException;'
MISSING = ('using "std:exceptions" MissingReturnError;using "std:exceptions" Exception;'
           'func missing()->u64{}'
           'func error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}')
runner.CASES = {}


def chain(count, reverse, parameters, result, call, leaf, suffix='', kind='func'):
    forms = [kind+' node_'+str(i)+'('+parameters+')->'+result+suffix+'{'+
             (call.replace('NEXT', 'node_'+str(i+1)) if i+1<count else leaf)+'}'
             for i in range(count)]
    return ''.join(reversed(forms) if reverse else forms)


def caught(body):
    return 'try{'+body+'}catch(e:const NullPointerException){};'


for count in (8, 64, 256):
    for reverse in (False, True):
        key = str(count)+'_'+('reverse' if reverse else 'forward')
        borrow = 'ptr item:u64 in O'
        suffix = ' throws NullPointerException where {owner O}'
        read = chain(count, reverse, borrow, 'void', 'NEXT(item);', 'item;{}', suffix)
        runner.CASES['read_'+key] = (IO+NPE+read+'val value<-17u64;'+caught('node_0(value);behave @print(value);'), 0, '17\n', '')
        runner.CASES['uninitialized_read_'+key] = (NPE+read+'val value<-make local u64;'+caught('node_0(value);'), 2, '', 'cannot prove initialization')
        write = chain(count, reverse, borrow, 'void', 'NEXT(item);', 'item<-23u64;', suffix)
        runner.CASES['write_'+key] = (IO+NPE+write+'val value<-make local u64;'+caught('node_0(value);behave @print(value);'), 0, '23\n', '')

for reverse in (False, True):
    key = 'reverse' if reverse else 'forward'
    returned = chain(32, reverse, 'ptr item:u64 in O', 'ptr u64 in O', 'return NEXT(item);', 'return item;', ' where {owner O}')
    runner.CASES['returned_target_'+key] = (IO+NPE+returned+'val value<-make local u64;ptr selected=node_0(value);'+
        caught('selected<-29u64;behave @print(value);'), 0, '29\n', '')
    categories = chain(32, reverse, 'value:Exception', 'const Exception', 'return NEXT(value);', 'return value;')
    runner.CASES['returned_category_'+key] = (IO+MISSING+categories+
        'try{throw node_0(error());}catch(e:const Exception){behave @print(e.category);};', 0, '5\n', '[-Wmissing-return]')
    captures = chain(32, reverse, '', 'u64', 'return NEXT();', 'return @bias;', kind='proc')
    runner.CASES['captured_symbols_'+key] = (IO+'symbol @bias:u64;symbol @selected:proc()->u64;'+captures+
        'effect @bias<-31u64;proc held=node_0;effect @bias<-37u64;effect @selected=held;'
        'behave @print(perform @selected());behave @print(behave @selected());', 0, '31\n37\n', '')
    recursive = [
        'func left(ptr item:u64 in O,n:u64)->void throws NullPointerException where {owner O}{if(n==0u64){item<-41u64;{}}else{right(item,n-1u64);};}',
        'func right(ptr item:u64 in O,n:u64)->void throws NullPointerException where {owner O}{left(item,n);}']
    if reverse:
        recursive.reverse()
    runner.CASES['recursive_write_'+key] = (IO+NPE+''.join(recursive)+'val value<-make local u64;'+
        caught('left(value,20u64);behave @print(value);'), 0, '41\n', '')
    late = [
        'proc outer()->u64{proc local=leaf;return invoke(local);}',
        'proc invoke(callback:F)->u64 where F{return callback();}',
        'proc leaf()->u64{return @bias;}']
    if reverse:
        late.reverse()
    definitions = 'symbol @bias:u64;symbol @selected:proc()->u64;'+''.join(late)
    runner.CASES['late_capture_'+key] = (IO+definitions+
        'effect @bias<-43u64;proc held=outer;effect @bias<-47u64;effect @selected=held;'
        'behave @print(perform @selected());behave @print(behave @selected());', 0, '43\n47\n', '')
    runner.CASES['late_capture_requires_binding_'+key] = (definitions+'proc held=outer;', 2, '', 'cannot prove an installed binding')

runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {name: [] for name in runner.CASES}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == '__main__':
    raise SystemExit(runner.main())
