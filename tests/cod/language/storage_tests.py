#!/usr/bin/env python3
"""Source checks for scalar storage identity, access and control-flow facts."""
from pathlib import Path
import sys
import foundation_tests as runner

PRELUDE = 'using symbol "std:io" @print;\n'
SHADOW = PRELUDE + 'val x=1u64; {{val x=x+1u64; behave @print(x);}};'
runner.CASES = {
    "storage": (None, 0, "3\n4\n4\n3\n9\n3\n3\n2\n15\n7.5\n5\n6\n7\n8\n11\ntrue\n3\n12\n3\n2\n13\n", ""),
    "storage_shadow": (None, 0, "12\n11\n10\n", "[-Wshadow]"),
    "shadow_error": (SHADOW, 2, "", "treated as error"),
    "unrelated_warning_error": (SHADOW, 0, "2\n", "[-Wshadow]"),
    "uninitialized": ('val x=make local u64; x;', 2, "", "before definite initialization"),
    "branch_initialization": ('proc f(flag:bool)->void { val x=make local u64; if(flag){x<-1u64;{}}else{}; x; } f(false);', 2, "", "before definite initialization"),
    "conditional_declaration": ('proc f(flag:bool)->void { if(flag){val x=1u64;}else{}; x; } f(false);', 2, "", "not available on every reaching path"),
    "short_circuit_initialization": ('val x=make local u64; false && {x<-1u64;true}; x;', 2, "", "before definite initialization"),
    "short_circuit_declaration": ('false && {val x=1u64;true}; x;', 2, "", "not available on every reaching path"),
    "const_storage": ('const val x=1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_target": ('val x:const u64=1u64; x<-2u64;', 2, "", "readonly destination"),
    "const_parameter": ('func f(x:const u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_val_parameter": ('func f(const val x:u64)->const u64 {x<-2u64;return x;} f(1u64);', 2, "", "readonly destination"),
    "const_copy": (PRELUDE + 'const val x=1u64; val copy=x; copy<-2u64; behave @print(x); behave @print(copy);', 0, "1\n2\n", ""),
    "val_rebind": ('val x=1u64,y=2u64; x=y;', 2, "", "val storage has no reference binding"),
    "transfer_result_readonly": ('val x=1u64; (x<-2u64)<-3u64;', 2, "", "readonly destination"),
    "ended_destination": ('({{ val x=1u64; x }}) <- 2u64;', 2, "", "lexical owner has ended"),
    "duplicate_group": ('val x=1u64; {val x=2u64;};', 2, "", "duplicate declaration"),
    "duplicate_branch": ('if(true){val x=1u64;}else{val x=2u64;};', 2, "", "duplicate declaration"),
    "duplicate_parameter": ('func f(x:u64)->const u64 {val x=2u64;return x;} f(1u64);', 2, "", "duplicate declaration"),
    "initializer_lookup": ('val x=x;', 2, "", "unknown scalar value"),
    "local_callable_shadow": ('func f()->void {} val f=1u64; f();', 2, "", "nearest binding 'f' is not callable"),
    "local_symbol_shadow": (PRELUDE + 'val print=1u64; behave @print(1u64);', 2, "", "nearest binding 'print' is not a symbol"),
    "narrow_store": ('val x:u8=1u8; x<-256u64;', 2, "", "does not satisfy destination type"),
    "copy_type_mismatch": ('val x:u64=true;', 2, "", "does not satisfy declared val type"),
    "generic_storage_error": ('func f(x:T)->const T where T {val y:T=true;return y;} f(1u64);', 2, "", "does not satisfy declared val type"),
    "module_value_capture": ('val x=1u64; func f()->const u64 {return x;} f();', 2, "", "module-local binding lookup and capture"),
    "module_symbol_shadow": (PRELUDE + 'val print=1u64; proc f()->void {behave @print(1u64);} f();', 2, "", "module-local binding lookup and capture"),
}
runner.CASE_ARGUMENTS = {"shadow_error": ["-Werror=shadow"], "unrelated_warning_error": ["-Werror=missing-return"]}
runner.EXTRA_SOURCES = [Path(__file__)]

if __name__ == "__main__":
    sys.exit(runner.main())
