#!/usr/bin/env python3
"""Cleanup composes physical targets with ordered initialization obligations."""
from pathlib import Path
import cleanup_selection_tests as fixture
runner = fixture.runner
good, bad, run = fixture.good, fixture.bad, fixture.run
IO, NPE, ERROR, WARN = fixture.IO, fixture.NPE, fixture.ERROR, fixture.WARN
runner.CASES = {
    'rebind_then_physical_write': good('{{defer{z<-1;};defer{p=z;};}};behave @print(p);'),
    'physical_write_then_rebind': good('{{defer{p=z;};defer{z<-1;};}};behave @print(p);'),
    'known_target_selected_write': good('{{defer{p<-1;};defer{p=z;};}};behave @print(z);'),
    'entry_rebind_then_selected_write': good('p=z;{{defer{p<-1;};}};behave @print(z);'),
    'entry_target_written_by_cleanup': good('p=z;{{defer{z<-1;};}};behave @print(p);'),
    'entry_initialization_then_rebind': good('z<-1;{{defer{p=z;};}};behave @print(p);'),
    'guarded_read_after_rebind_and_write': good('defer{behave @print(p);}finally{};defer{z<-1;};defer{p=z;};'),
    'guarded_read_after_local_write': good('defer{z<-1;behave @print(p);}finally{};defer{p=z;};'),
    'guarded_physical_read_after_known_write': good('defer{behave @print(z);}finally{};defer{p=z;p<-1;};'),
    'separated_selected_write_physical_read_unproved': bad('defer{behave @print(z);}finally{};defer{p<-1;};defer{p=z;};', 'cannot prove initialization'),
    'early_read_precedes_target_write': bad('defer{z<-1;};defer{behave @print(p);}finally{};defer{p=z;};', 'cannot prove initialization'),
    'early_read_precedes_target_rebind': bad('defer{p=z;};defer{behave @print(p);}finally{};defer{z<-1;};', 'cannot prove initialization'),
    'registration_failure_keeps_read_obligation': bad('defer{behave @print(p);};defer{z<-1;};defer{p=z;};', 'cannot prove initialization'),
    'failed_writer_registration_keeps_handler_obligation': (fixture.OOM + run('p=z;{{defer catch(e:const OutOfMemory){behave @print(z);};defer{p<-1;};}};'), 2, '', 'cannot prove initialization'),
    'write_before_repeated_rebind_unproved': bad('{{defer{p=z;};defer{p<-1;};defer{p=z;};}};behave @print(p);'),
    'unrelated_physical_write': bad('{{defer{x<-1;};defer{p=z;};}};behave @print(p);'),
    'old_target_not_initialized': bad('{{defer{p<-1;};defer{p=z;};}};behave @print(x);'),
    'all_branch_targets_initialized': good('{{defer{if(flag){p=x;}else{p=y;};};defer{x<-1;y<-1;};}};behave @print(p);'),
    'one_branch_target_uninitialized': bad('{{defer{if(flag){p=x;}else{p=y;};};defer{x<-1;};}};behave @print(p);'),
    'conditional_rebind_preserves_entry_target': good('p=z;{{defer{if(flag){p=x;{}}else{};};defer{x<-1;z<-1;};}};behave @print(p);'),
    'conditional_rebind_keeps_unknown_entry': bad('{{defer{if(flag){p=z;{}}else{};};defer{z<-1;};}};behave @print(p);'),
    'overlapping_alias_target_write': good('var other->if(flag){b}else{a};{{defer{z<-1;};defer{other=z;p=z;};}};behave @print(p);'),
    'weak_rebind_preserves_initialized_selection': good('p<-1;{{defer{a=z;};defer{z<-1;};}};behave @print(p);'),
    'nested_target_relation': good('{{defer{ {{defer{p=z;};}}; };defer{z<-1;};}};behave @print(p);'),
    'nested_physical_write': good('{{defer{ {{defer{z<-1;};}}; };defer{p=z;};}};behave @print(p);'),
    'borrowed_setter_known_target': (fixture.SETTER + run('{{defer{set(p);};defer{p=z;};}};behave @print(z);', throws='throws NullPointerException'), 0, '21\n21\n', ''),
    'borrowed_reader_resolves_target': (fixture.READER + run('defer{show(p);}finally{};defer{z<-1;};defer{p=z;};', throws='throws NullPointerException'), 0, '1\n1\n', ''),
    'prepared_reader_keeps_saved_target': (fixture.READER + run('defer show(p);p=z;z<-1;', throws='throws NullPointerException'), 2, '', 'cannot prove initialization'),
    'known_target_failure_after_write': (ERROR + run('val e=error();p=z;try{{defer{p<-1;throw e;};}}catch(e:const MissingReturnException){behave @print(z);};'), 0, '1\n1\n', WARN),
    'failed_rebind_registration_keeps_old_target': (ERROR + run('val e=error();try{{defer{p<-1;throw e;};defer{p=z;};}}catch(e:const MissingReturnException){behave @print(z);};'), 2, '', 'before definite initialization'),
    'known_target_failure_before_write': (ERROR + run('val e=error();try{{defer{throw e;p<-1;};defer{p=z;};}}catch(e:const MissingReturnException){behave @print(z);};'), 2, '', 'before definite initialization'),
    'nullable_selection_does_not_initialize_cell': bad('{{defer{if(!isnull(p)){p<-1;{}}else{};};defer{p=if(flag){z}else{null};};}};behave @print(z);'),
    'null_branch_does_not_initialize_cell': bad('ptr p=if(flag){z}else{null};if(isnull(p)){}else{p<-1;{}};behave @print(z);', selection=''),
    'nullable_initialized_target_read': good('ptr p=if(flag){z}else{null};{{defer{z<-1;};}};if(isnull(p)){behave @print(2);}else{behave @print(p);};', output='1\n2\n', selection=''),
    'null_refinement_inside_cleanup': good('ptr p=if(flag){z}else{null};defer{if(isnull(p)){behave @print(2);}else{behave @print(p);};}finally{};defer{z<-1;};', output='1\n2\n', selection=''),
    'inner_target_owner_mismatch': bad('{{val t=make local i64;p=t;defer{t<-1;};}};behave @print(p);', 'initializer-fixed owner'),
}
runner.CASE_ARGUMENTS = {}
runner.EXTRA_SOURCES = [Path(fixture.__file__), Path(__file__)]
if __name__ == '__main__': raise SystemExit(runner.main())
