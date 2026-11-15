#!/usr/bin/env python3
"""Canonical failure names preserve semantic identity and common-root handling."""
from pathlib import Path
import foundation_tests as runner

IO = 'using symbol "std:io" @print;'
OUT = lambda value: 'behave @print('+value+');'
ROOT = 'using "std:exceptions" Exception;'
ERRORS = ROOT+'using "std:exceptions" MissingReturnError;using "std:exceptions" NullPointerException;func missing()->i64{}func missing_error()->const MissingReturnError{try{missing();}catch(e:const MissingReturnError){return e;};}func null_error()->const NullPointerException{ptr absent:i64=null in here;try{absent+0;}catch(e:const NullPointerException){return e;};}'
WARN = '[-Wmissing-return]'
runner.CASES = {
 'root_catches_unchecked': (IO+ERRORS+'try{missing();{}}catch(e:const Exception){'+OUT('e.category')+'};',0,'5\n',WARN),
 'root_catches_checked': (IO+ROOT+'ptr absent:i64=null in here;try{absent+0;{}}catch(e:const Exception){'+OUT('e.category')+'};',0,'4\n',''),
 'root_alias': (IO+ERRORS+'using "std:exceptions" Exception as RootError;try{throw null_error();}catch(e:const RootError){'+OUT('e.category')+'};',0,'4\n',WARN),
 'specific_then_root': (IO+ERRORS+'proc run(e:Exception)->void{try{throw e;}catch(x:const NullPointerException){'+OUT('1')+'}catch(x:const Exception){'+OUT('2')+'};}run(null_error());run(missing_error());',0,'1\n2\n',WARN),
 'root_before_specific': (IO+ERRORS+'proc run(e:Exception)->void{try{throw e;}catch(x:const Exception){'+OUT('1')+'}catch(x:const NullPointerException){'+OUT('2')+'};}run(null_error());run(missing_error());',0,'1\n1\n',WARN),
 'root_rethrow': (IO+ERRORS+'try{try{throw null_error();}catch(e:const Exception){throw;}}catch(e:const NullPointerException){'+OUT('e.category')+'};',0,'4\n',WARN),
 'sibling_exclusion': (IO+ERRORS+'try{try{throw null_error();}catch(e:const NullPointerException){throw;}catch(e:const Exception){'+OUT('99')+'};}catch(e:const Exception){'+OUT('e.category')+'};',0,'4\n',WARN),
 'root_cause_value': (IO+ERRORS+'val first<-missing_error(),cause<-null_error();val saved<-try{throw first from cause;}catch(e:const Exception){e};'+OUT('saved.category')+OUT('saved.cause_count')+OUT('cause_at(saved,0u64).category'),0,'5\n1\n4\n',WARN),
 'root_readonly': (ERRORS+'try{throw missing_error();}catch(e:const Exception){e<-null_error();};',2,'','readonly destination'),
 'duplicate_root_alias': (ROOT+'using "std:exceptions" Exception as Root;try{}catch(e:const Exception){}catch(e:const Root){};',2,'','duplicate exception category'),
 'duplicate_root_untyped': (ROOT+'try{}catch(e:const Exception){}catch(e){};',2,'','duplicate exception category'),
 'generic_root_and_concrete': (IO+ERRORS+'func recover(e:T,n:u64)->u64 where T{if(n==0u64){try{throw e;}catch(x:const T){return x.category;};}else{return recover(e,n-1u64);};}val common:Exception<-null_error();'+OUT('recover(common,30u64)')+OUT('recover(missing_error(),30u64)'),0,'4\n5\n',WARN),
 'retained_generic_root': (IO+ERRORS+'func factory(sample:T)->func(Exception)->const u64 where T{return func[](value:Exception)->const u64{try{throw value;}catch(e:const T){return e.category;};};}val common:Exception<-null_error();func handle=factory(common);'+OUT('handle(missing_error())'),0,'5\n',WARN),
 'deferred_root_recovery': (IO+ERRORS+'func recover(value:Exception)->i64{defer catch(e:const Exception){return 7;}finally{};defer{throw value;};return 1;}'+OUT('recover(missing_error())')+OUT('recover(null_error())'),0,'7\n7\n',WARN),
 'deferred_root_preserves_return': (IO+ERRORS+'func recover(value:Exception)->i64{defer catch(e:const Exception){}finally{};defer{throw value;};return 7;}'+OUT('recover(missing_error())')+OUT('recover(null_error())'),0,'7\n7\n',WARN),
 'root_initialization': (IO+ERRORS+'func recover(value:Exception)->i64{val result<-make local i64;try{throw value;}catch(e:const Exception){result<-9;{}};return result;}'+OUT('recover(missing_error())')+OUT('recover(null_error())'),0,'9\n9\n',WARN),
 'root_declared_failures': (ROOT+'proc declared()->void throws Exception{}try{declared();}catch(e:const Exception){};',0,'',''),
 'root_still_requires_checked_obligations': (ROOT+'proc failed(value:Exception)->void{throw value;}',2,'','requires a covering catch or throws declaration'),
 'checked_alias_error_suffix': ('using "std:exceptions" NullPointerException as NullableError;proc failed(value:NullableError)->void{throw value;}',2,'',"checked exception 'NullPointerException'"),
 'unchecked_alias_exception_suffix': ('using "std:exceptions" OutOfMemoryError as AllocationException;proc failed(value:AllocationException)->void{throw value;}',0,'',''),
 'root_alias_exception_suffix': ('using "std:exceptions" Exception as Failure;proc failed(value:Failure)->void throws Failure{throw value;}',0,'',''),
}
# These are rejected import inputs, not supported compatibility names.
for name in ['SystemException','OutOfMemory','RegionCapacityExceeded','InvalidCapacity','MissingReturnException','UnsoundRuntimeTyping','StaticallyUnknownType','IndexOutOfBoundsException']:
    runner.CASES['obsolete_'+name] = ('using "std:exceptions" '+name+';',2,'',"unknown intrinsic exception selection '"+name+"'")
for name,checked in [('OutOfMemoryError',False),('RegionCapacityExceededError',False),('InvalidCapacityError',False),('NullPointerException',True),('MissingReturnError',False),('UnsoundRuntimeTypingError',False),('StaticallyUnknownTypeException',True),('IndexOutOfBoundsError',False)]:
    runner.CASES['classification_'+name] = ('using "std:exceptions" '+name+';proc failed(value:'+name+')->void{throw value;}',2 if checked else 0,'',"checked exception '"+name+"'" if checked else '')
runner.CASE_ARGUMENTS = {}
runner.CASE_SHADOWS = {}
runner.EXTRA_SOURCES = [Path(__file__)]
if __name__ == '__main__':
    raise SystemExit(runner.main())
