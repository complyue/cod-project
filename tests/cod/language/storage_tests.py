#!/usr/bin/env python3
"""Source checks for storage identity, access and control-flow facts."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = 'using symbol "std:io" @print;\n'
SHADOW = PRELUDE + 'val x<-1u64; {{val x<-2u64; behave @print(x);}};'
PRODUCT = 'data Window {minimum:u64; maximum:u64;}'
runner.CASES = {
    "storage": (None, 0, "3\n4\n4\n3\n9\n3\n3\n2\n15\n7.5\n5\n6\n7\n8\n11\ntrue\n3\n12\n3\n2\n13\n", ""),
    "storage_shadow": (None, 0, "12\n11\n10\n", "[-Wshadow]"),
    "shadow_error": (SHADOW, 2, "", "treated as error"),
    "unrelated_warning_error": (SHADOW, 0, "2\n", "[-Wshadow]"),
    "uninitialized": ('val x<-make local u64; x;', 2, "", "before definite initialization"),
    "branch_initialization": ('proc f(flag:bool)->void { val x<-make local u64; if(flag){x<-1u64;{}}else{}; x; } f(false);', 2, "", "before definite initialization"),
    "conditional_declaration": ('proc f(flag:bool)->void { if(flag){val x<-1u64;}else{}; x; } f(false);', 2, "", "not available on every reaching path"),
    "short_circuit_initialization": ('val x<-make local u64; false && {x<-1u64;true}; x;', 2, "", "before definite initialization"),
    "short_circuit_declaration": ('false && {val x<-1u64;true}; x;', 2, "", "not available on every reaching path"),
    "const_storage": ('const val x<-1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_target": ('val x:const u64<-1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_parameter": ('func f(x:const u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_val_parameter": ('func f(const val x:u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_copy": (PRELUDE + 'const val x<-1u64; val copy<-x; copy<-2u64; behave @print(x); behave @print(copy);', 0, "1\n2\n", ""),
    "val_rebind": ('val x<-1u64,y<-2u64; x=y;', 2, "", "val storage has no reference binding"),
    "transfer_result_readonly": ('val x<-1u64; {x<-2u64}<-3u64;', 2, "", "readonly destination"),
    "full_scope_value_is_not_destination": ('{{ val x<-1u64; return x; }} <- 2u64;', 2, "", "assignment requires a storage destination"),
    "duplicate_group": ('val x<-1u64; {val x<-2u64;};', 2, "", "duplicate declaration"),
    "duplicate_branch": ('if(true){val x<-1u64;}else{val x<-2u64;};', 2, "", "duplicate declaration"),
    "duplicate_parameter": ('func f(x:u64)->const u64 {val x<-2u64;return x;} f(1u64);', 2, "", "duplicate declaration"),
    "initializer_lookup": ('val x<-x;', 2, "", "unknown scalar value"),
    "local_callable_shadow": ('func f()->void {} val f<-1u64; f();', 2, "", "nearest binding 'f' is not callable"),
    "local_symbol_shadow": (PRELUDE + 'val print<-1u64; behave @print(1u64);', 2, "", "nearest binding 'print' is not a symbol"),
    "narrow_store": ('val x:u8<-1u8; x<-256u64;', 2, "", "does not satisfy destination type"),
    "copy_type_mismatch": ('val x:u64<-true;', 2, "", "does not satisfy declared val type"),
    "generic_storage_error": ('func f(x:T)->const T where T {val y:T<-true;return y;} f(1u64);', 2, "", "does not satisfy declared val type"),
    "module_value_capture": ('val x<-1u64; func f()->const u64 {return x;} f();', 2, "", "module-local binding lookup and capture"),
    "module_symbol_shadow": (PRELUDE + 'val print<-1u64; proc f()->void {behave @print(1u64);} f();', 2, "", "module-local binding lookup and capture"),
    "lexical_product_initializer_order_and_borrow": (PRELUDE + PRODUCT +
        'proc mark(n:u64)->u64{behave @print(n);return n;}'
        'proc write(ref target:u64 in O) where {owner O}{defer (<-)(target,21u64);return;}'
        'ref w:Window in here = make Window {maximum<-mark(30u64);minimum<-mark(20u64);};'
        'ref selected=w.minimum;behave @print(w.maximum);write(w.minimum);'
        'behave @print(selected);behave @print((selected is w.minimum));',
        0, "30\n20\n30\n21\ntrue\n", ""),
    "lexical_product_distinct_subobjects": (PRELUDE + PRODUCT +
        'ref w=make Window in here {minimum<-7u64;maximum<-7u64;};'
        'behave @print((w is w.minimum));behave @print((w.minimum is w.minimum));'
        'behave @print((w.minimum is w.maximum));', 0, "false\ntrue\nfalse\n", ""),
    "lexical_product_partial_and_nested_initialization": (PRELUDE + PRODUCT +
        'data Outer {window:Window;}ptr p=make Outer;behave @print((p is not null));'
        'p.window.minimum<-20u64;behave @print(p.window.minimum);'
        'p.window.maximum<-30u64;ref ready=p;behave @print(ready.window.maximum);',
        0, "true\n20\n30\n", ""),
    "lexical_product_missing_field_at_finish": (
        PRODUCT + 'ref w=make Window {minimum<-20u64;};', 2, "", "missing initialization of field 'maximum'"),
    "lexical_product_uninitialized_field_read": (PRELUDE + PRODUCT +
        'ptr w=make Window;w.minimum<-20u64;behave @print(w.maximum);',
        2, "", "before definite initialization"),
    "lexical_product_incomplete_ref_after_branch": (PRODUCT +
        'proc check(flag:bool)->void{ptr w=make Window;w.minimum<-20u64;'
        'if(flag){w.maximum<-30u64;{}}else{};ref ready=w;return;}check(false);',
        2, "", "read of 'maximum' before definite initialization"),
    "lexical_product_repeated_outer_owner_allocation": (PRODUCT +
        'ptr w=make Window;for(i in 0u64..2u64){w=make Window;};',
        2, "", "requires a separate full lexical scope"),
    "lexical_product_readonly_projection": (PRODUCT +
        'ref w:const Window=make Window {minimum<-20u64;maximum<-30u64;};w.minimum<-21u64;',
        2, "", "readonly destination"),
    "lexical_product_null_before_projection": (PRELUDE + PRODUCT +
        'using "std:exceptions" NullPointerException;ptr w:Window=null in here;'
        'try{behave @print(w.minimum);}catch(e:const NullPointerException){behave @print(9u64);};',
        0, "9\n", ""),
    "borrowed_product_and_projected_results": (PRELUDE + PRODUCT +
        'data Policy {window:Window;}'
        'func identity(ref item:T in O)->ref T in O where {T,owner O}{return item;}'
        'func floor(ref rule:Policy in O)->ref u64 in O where {owner O}{return rule.window.minimum;}'
        'func select(ref rule:Policy in O)->ref u64 in O where {owner O}{return floor(identity(rule));}'
        'ptr policy=make Policy;policy.window.minimum<-20u64;policy.window.maximum<-30u64;'
        'ref current=identity(policy);ref retained=select(current);retained<-21u64;'
        'behave @print(policy.window.minimum);behave @print((retained is policy.window.minimum));'
        'behave @print((current is policy));', 0, "21\ntrue\ntrue\n", ""),
    "borrowed_product_defer_keeps_selected_input": (PRELUDE +
        'data Settings {enabled:bool;limit:u64;}'
        'proc configure(ref target:Settings in O) where {owner O}{target.enabled<-true;target.limit<-33u64;return;}'
        'proc run()->void{ref a=make Settings {enabled<-false;limit<-5u64;};'
        'ref b=make Settings {enabled<-false;limit<-6u64;};ptr current=a;'
        'defer{behave @print(a.enabled);behave @print(a.limit);behave @print(b.enabled);behave @print(b.limit);};'
        'defer configure(current);current=b;return;}run();', 0, "true\n33\nfalse\n6\n", ""),
    "borrowed_product_checks_complete_input": (PRODUCT +
        'proc borrow(ref target:Window in O) where {owner O}{return;}'
        'ptr window=make Window;window.minimum<-20u64;borrow(window);', 2, "", "read of 'maximum' before definite initialization"),
    "borrowed_product_write_updates_caller_boolean_proof": (
        'data Settings {enabled:bool;}'
        'proc arm(ref target:Settings in O) where {owner O}{target.enabled<-true;return;}'
        'ref state=make Settings {enabled<-false;};val unread<-make local u64;arm(state);'
        'if(state.enabled){unread;{}}else{};', 2, "", "read of 'unread' before definite initialization"),
    "const_product_owner_keeps_writable_target_access": (PRELUDE + PRODUCT +
        'proc update(ref target:Window in O) where {const owner O}{target.minimum<-7u64;return;}'
        'func inspect(target:const Window in O)->u64 where {const owner O}{return target.minimum;}'
        'ref window=make Window {minimum<-5u64;maximum<-6u64;};update(window);behave @print(inspect(window));',
        0, "7\n", ""),
    "const_product_owner_cannot_escalate_at_call": (PRODUCT +
        'proc update(ref target:Window in O) where {owner O}{target.minimum<-7u64;return;}'
        'proc appoint(ref target:Window in O) where {const owner O}{update(target);return;}'
        'ref window=make Window {minimum<-5u64;maximum<-6u64;};appoint(window);',
        2, "", "const owner cannot supply allocation access"),
    "readonly_product_parameters_require_const_owner": (PRODUCT +
        'func inspect(target:const Window in O)->u64 where {owner O}{return target.minimum;}',
        2, "", "readonly product inputs require 'const owner O'"),
    "nullable_product_argument_reads_only_a_present_field": (PRELUDE + PRODUCT +
        'func inspect(ptr window:const Window in O)->u64 where {const owner O}{'
        'if(window is null){return 9u64;}else{};return window.minimum;}'
        'ptr absent:Window=null in here;ptr reserved=make Window;reserved.minimum<-7u64;'
        'behave @print(inspect(absent));behave @print(inspect(reserved));', 0, "9\n7\n", ""),
    "nullable_product_argument_requires_the_read_field": (PRODUCT +
        'func inspect(ptr window:const Window in O)->u64 where {const owner O}{'
        'if(window is null){return 9u64;}else{};return window.maximum;}'
        'ptr reserved=make Window;reserved.minimum<-7u64;inspect(reserved);',
        2, "", "for deferred or borrowed read"),
    "nullable_product_result_preserves_the_projected_target": (PRELUDE + PRODUCT +
        'func floor(ptr window:Window in O)->ptr u64 in O where {owner O}{'
        'if(window is null){return null in O;}else{};return window.minimum;}'
        'ptr absent:Window=null in here;ptr present=make Window {minimum<-3u64;maximum<-11u64;};'
        'ptr missing=floor(absent);ptr minimum=floor(present);'
        'behave @print((missing is null));behave @print((minimum is present.minimum));', 0, "true\ntrue\n", ""),
    "nullable_product_call_preserves_partial_initialization": (PRELUDE + PRODUCT +
        'proc establish(ptr window:Window in O) where {owner O}{'
        'if(window is null){return;}else{};window.minimum<-17u64;return;}'
        'ptr reserved=make Window;establish(reserved);behave @print(reserved.minimum);', 0, "17\n", ""),
    "nullable_product_defer_retains_its_selected_target": (PRELUDE +
        'using "std:exceptions" OutOfMemoryError;data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-27u64;return;}'
        'proc run()->void{ptr reserved=make Counter;ptr selection=reserved;'
        'defer{behave @print(reserved.count);}catch(e:const OutOfMemoryError){return;};'
        'defer establish(selection);selection=null;return;}run();', 0, "27\n", ""),
    "nullable_product_failed_call_does_not_initialize_later_fields": (
        'using "std:exceptions" NullPointerException;data Counter {count:u64;}'
        'proc establish(ptr first:Counter in O,ptr second:Counter in O) throws NullPointerException where {owner O}{'
        'first.count<-1u64;second.count<-2u64;return;}'
        'ptr absent:Counter=null in here;ptr reserved=make Counter;'
        'try{establish(absent,reserved);}catch(e:const NullPointerException){};reserved.count;',
        2, "", "read of 'count' before definite initialization"),
    "product_call_initializes_the_dynamically_selected_field": (PRELUDE +
        'data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-17u64;return;}'
        'proc run(flag:bool)->void{ptr first=make Counter;ptr second=make Counter;'
        'ptr chosen=if(flag){first}else{second};establish(chosen);behave @print(chosen.count);return;}'
        'run(true);run(false);', 0, "17\n17\n", ""),
    "product_field_proof_does_not_follow_pointer_rebinding": (
        'data Counter {count:u64;}'
        'proc establish(ptr target:Counter in O) where {owner O}{'
        'if(target is null){return;}else{};target.count<-17u64;return;}'
        'proc run(flag:bool)->void{ptr first=make Counter;ptr second=make Counter;'
        'ptr chosen=if(flag){first}else{second};establish(chosen);chosen=second;chosen.count;return;}run(false);',
        2, "", "read of 'count' before definite initialization"),
}
runner.CASE_ARGUMENTS = {"shadow_error": ["-Werror=shadow"], "unrelated_warning_error": ["-Werror=missing-return"]}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
