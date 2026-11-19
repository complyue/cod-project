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
}
runner.CASE_ARGUMENTS = {"shadow_error": ["-Werror=shadow"], "unrelated_warning_error": ["-Werror=missing-return"]}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
