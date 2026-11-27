#include "cod_providers.h"

#include "cod_region_storage.h"
#include "compile_diagnostic.h"
#include "path_owner.h"
#include "provider_output.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Target.h>

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "launcher handles require a pointer that fits in a handle word");

static const char *diagnostic_phase(uint8_t stage) {
    switch (stage) {
    case 0: return "native lowering";
    case 1: return "source read";
    case 2: return "module selection";
    case 3: return "name binding";
    case 4: return "operand checking";
    case 5: return "completion checking";
    default: return "compilation";
    }
}

static const char *llvm_error(uint32_t code) {
    switch (code) {
    case llvm_invalid_command_range: return "invalid command range";
    case llvm_construction_failed: return "construction failed";
    case llvm_verification_failed: return "verification failed";
    case llvm_emission_failed: return "emission failed";
    case llvm_publication_failed: return "could not publish object";
    default: return "operation failed";
    }
}

static const char *scratch_error(uint32_t code) {
    switch (code) {
    case COD_REGION_STORAGE_INVALID: return "invalid layout";
    case COD_REGION_STORAGE_TOO_LARGE: return "layout is too large";
    case COD_REGION_STORAGE_NO_MEMORY: return "out of memory";
    case COD_REGION_STORAGE_FULL: return "layout exceeds region capacity";
    default: return "allocation failed";
    }
}

static const char *provider_error(uint32_t code) {
    switch (code) {
    case provider_output_conflict: return "C provider declarations have conflicting native names";
    case provider_output_invalid: return "C provider declaration metadata is incomplete";
    case provider_output_io: return "could not publish the generated C provider files";
    case provider_output_memory: return "could not allocate workspace for C provider generation";
    case provider_output_reserved_name: return "provider field uses a reserved C name";
    default: return "C provider generation failed";
    }
}

static const char *unsupported_source_form(uint32_t code) {
    switch (code) {
    case 28: return "owning exception local";
    case 29: return "owning exception parameter";
    case 30: return "owning exception result";
    case 33: return "owning make exception expression";
    case 35: return "borrowed reference result";
    case 36: return "writable owning exception catch";
    case 37: return "ordinary construction";
    case 39: return "subscript expression";
    case 40: return "bodyless deferred exception handler";
    case 41: return "lexical callable value";
    case 42: return "throw with an explicit cause";
    case 43: return "braced exception construction in throw";
    case 44: return "rethrow with an explicit cause";
    case 45: return "named call argument";
    case 46: return "reference assignment in a construction initializer";
    case 47: return "defer accumulating in an ordinary loop body";
    case 48: return "repeated lexical construction in an ordinary loop body";
    case 51: return "repeated handle acquisition in an ordinary loop body";
    case 58: return "unproved C constructor operand type";
    case 63: return "supplier-owned allocation or defer in a single-brace here body";
    case 65: return "selection of an imported here signature";
    case 71: return "unproved native here feedback type";
    case 74: return "array length query";
    case 76: return "ordinary here parameter lowering";
    case 108: return "lexical callable definition lowering";
    default: return NULL;
    }
}

static void report(const char *input, uint8_t status, const cod_booter_diagnostic *diagnostic) {
    if (diagnostic->os_error) {
        fprintf(stderr, "%s: error: source open failed (system error %" PRIu64 ")\n", input, diagnostic->os_error);
        return;
    }
    if (!diagnostic->stage && !diagnostic->code && !diagnostic->first.line) {
        fprintf(stderr, "%s: error: compilation failed (status %u)\n", input, (unsigned)status);
        return;
    }
    const char *unsupported = diagnostic->stage == 3 ? unsupported_source_form(diagnostic->code) : NULL;
    fputs(input, stderr);
    if (diagnostic->first.line)
        fprintf(stderr, ":%" PRIu64 ":%" PRIu64, diagnostic->first.line, diagnostic->first.column);
    if (diagnostic->stage == 6)
        fprintf(stderr, ": error: LLVM backend %s (code %" PRIu32 ")\n", llvm_error(diagnostic->code), diagnostic->code);
    else if (diagnostic->stage == 7)
        fprintf(stderr, ": error: compiler scratch %s (code %" PRIu32 ")\n", scratch_error(diagnostic->code), diagnostic->code);
    else if (diagnostic->stage == provider_output_stage)
        fprintf(stderr, ": error: %s\n", provider_error(diagnostic->code));
    else if (unsupported)
        fprintf(stderr, ": error: %s is outside the supported compiler profile\n", unsupported);
    else if (diagnostic->stage == 1 && diagnostic->code == 36)
        fputs(": error: expected a range header of the form 'for (name in lower .. upper)'\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 37)
        fputs(": error: expected a native handle constructor argument list\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 38)
        fputs(": error: borrowed handle construction requires 'in' followed by a live owning handle\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 39)
        fputs(": error: expected another positional here receiver\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 40)
        fputs(": error: a call can supply only one trailing here body\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 41)
        fputs(": error: a val parameter has independent storage and cannot declare 'in'\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 42)
        fputs(": error: expected ']' after the subscript expression\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 43)
        fputs(": error: unsupported intrinsic syntax, expected 'intrinsic (len)(array)'\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 44)
        fputs(": error: expected ',' or '}' after module selection\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 45)
        fputs(": error: expected an enum member or its integer code\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 46)
        fputs(": error: expected 'of' after the case subject\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 47)
        fputs(": error: expected '->' after the case arm header\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 48)
        fputs(": error: case pattern is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 49)
        fputs(": error: the unbraced throwing arm requires an unguarded catch-all\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 50)
        fputs(": error: expected ref after not init\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 51)
        fputs(": error: expected a quoted assertion diagnostic\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 113)
        fputs(": error: invalid UTF-8 text literal or escape\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 115)
        fputs(": error: expected '(expression -> capacity)' in region construction\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 77)
        fputs(": error: catch annotation must name an exception type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 7)
        fputs(": error: native handle binding is unproved on this path\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 13)
        fputs(": error: this module declaration is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 129)
        fputs(": error: a variadic method must be the only method in its callable family\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 130)
        fputs(": error: executing a variadic method body is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 131)
        fputs(": error: checking conformance to a callable promise is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 132)
        fputs(": error: a callable promise supplies no implementation on this path\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 133)
        fputs(": error: rebinding a native handle is outside the checked compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 134)
        fputs(": error: ref groups and bare ref slots are outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 135)
        fputs(": error: not init ref parameters are outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 136)
        fputs(": error: initializing a reserved target is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 137)
        fputs(": error: predicate assertion messages are outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 138)
        fputs(": error: reference identity checking is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 139)
        fputs(": error: a readable loan of the nominal sum case subject could not be proved\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 140)
        fputs(": error: constructor is not selected for the case subject's nominal sum type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 141)
        fputs(": error: case receipt or projection requires a checked field of the selected type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 149)
        fputs(": error: inline bits construction is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 150)
        fputs(": error: construction does not match the receiving finite-bits type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 151)
        fputs(": error: no such initializer field in the selected construction\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 142)
        fputs(": error: declaration conflicts with a protected symbol name\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 143)
        fputs(": error: on returning is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 146)
        fputs(": error: type synonym cycle has no defining type\n", stderr);
    else if (diagnostic->stage == 1 && diagnostic->code == 148)
        fputs(": error: grouped fixed references are outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 148)
        fputs(": error: a fixed reference cannot be rebound\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 144)
        fputs(": error: this signed integer operand requires an enum assertion\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 145)
        fputs(": error: a negative enum code requires an unsuffixed signed 64-bit operand\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 49)
        fputs(": error: a declaration in a loop body or else requires an explicit {{ ... }} scope\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 50)
        fputs(": error: a function cannot call a native handle constructor\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 52)
        fputs(": error: owning native construction requires fresh acquisition in a lexical own handle initializer or owning effect. "
              "A borrowed initializer requires a live handle\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 53)
        fputs(": error: native handle CType does not match the required type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 54)
        fputs(": error: native acquisition mode does not match the owning or borrow-only CType\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 55)
        fputs(": error: no C constructor declared for this CType\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 57)
        fputs(": error: wrong number of C constructor arguments\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 59)
        fputs(": error: native handle does not have the required actual owner\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 60)
        fputs(": error: C constructor argument has an incompatible bits type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 61)
        fputs(": error: a declaration in a here body requires an explicit {{ ... }} scope\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 62)
        fputs(": error: break or continue requires an active single-brace here body\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 64)
        fputs(": error: the selected call has no supported first here parameter\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 66)
        fputs(": error: here receiver count does not match the selected callback signature\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 67)
        fputs(": error: here receiver type does not match the selected callback signature\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 68)
        fputs(": error: native here completion requires a u8 value\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 69)
        fputs(": error: the selected C provider requires a trailing here body\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 70)
        fputs(": error: a function cannot invoke a procedure\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 72)
        fputs(": error: const owner requires an input under that owner\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 73)
        fputs(": error: owner requires a writable-target input under that owner\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 75)
        fputs(": error: conflicting public alias\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 78)
        fputs(": error: enum backing type must be a fixed-width integer\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 79)
        fputs(": error: enum member code is outside the backing type's range\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 80)
        fputs(": error: enum member identity requires an earlier member\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 81)
        fputs(": error: duplicate enum member code\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 82)
        fputs(": error: duplicate enum member name\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 83)
        fputs(": error: selected enum member conflicts with a module binding\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 84)
        fputs(": error: this type assertion is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 85)
        fputs(": error: no such member of the asserted enum type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 86)
        fputs(": error: operand does not have the asserted enum type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 87)
        fputs(": error: the enum assertion operand's type remains unproved\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 88)
        fputs(": error: native here feedback must have the declared enum type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 90)
        fputs(": error: a case-arm declaration requires an explicit {{ ... }} scope\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 91)
        fputs(": error: continue cannot cross a case body\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 92)
        fputs(": error: fallthrough requires an active case arm\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 93)
        fputs(": error: bool or nominal enum case subject type is unproved\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 94)
        fputs(": error: case pattern does not match the subject type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 95)
        fputs(": error: no such member in the case subject's enum\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 96)
        fputs(": error: case guard requires bool\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 97)
        fputs(": error: CString assertion requires String text\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 98)
        fputs(": error: text literal contains an interior NUL and cannot supply CString\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 99)
        fputs(": error: C provider requires a proved CString argument\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 100)
        fputs(": error: C provider requires a proved String argument\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 101)
        fputs(": error: text results require a readonly, non-null owner-qualified reference\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 102)
        fputs(": error: make String requires a text operand\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 103)
        fputs(": error: const owner cannot allocate storage\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 104)
        fputs(": error: effect operator does not match the declared symbol\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 105)
        fputs(": error: invalid nominal parent type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 106)
        fputs(": error: abstract parent declarations form a cycle\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 107)
        fputs(": error: a direct place parameter requires writable val var String storage\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 110)
        fputs(": error: a direct String place requires a String or CString source\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 111)
        fputs(": error: dependency initialization is outside the supported object profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 114)
        fputs(": error: nominal sum declaration checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 116)
        fputs(": error: named region checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 117)
        fputs(": error: this fixed local binding mode is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 118)
        fputs(": error: sum constructor initializers are outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 119)
        fputs(": error: owning region result checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 120)
        fputs(": error: constructor case reception checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 121)
        fputs(": error: reference-slot formal checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 122)
        fputs(": error: fixed formal binding checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 123)
        fputs(": error: symbolic procedure declaration checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 124)
        fputs(": error: scoped symbolic procedure checking is outside the current compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 18)
        fputs(": error: condition must have type bool\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 147)
        fputs(": error: assertion condition is provably false\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 20)
        fputs(": error: narrow integer execution is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 21)
        fputs(": error: const owner cannot supply allocation access to the callee\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 23)
        fputs(": error: native case requires a Boolean, enum, or null subject in this compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 24)
        fputs(": error: native case pattern is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 25)
        fputs(": error: value case may finish without a result\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 18)
        fputs(": error: a case transfer across cleanup is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 27)
        fputs(": error: this type assertion is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 28)
        fputs(": error: no such member in the enum\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 29)
        fputs(": error: incompatible enum type\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 30)
        fputs(": error: product storage form is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 31)
        fputs(": error: no such field in the product\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 32)
        fputs(": error: readonly field is initialized more than once\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 33)
        fputs(": error: product construction is missing a field initializer\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 34)
        fputs(": error: operation requires immutable String or CString text\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 35)
        fputs(": error: argument or binding requires a proved CString\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 36)
        fputs(": error: text contains an interior NUL and cannot supply CString\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 37)
        fputs(": error: this text operation is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 38)
        fputs(": error: borrowed text result does not retain its declared owner\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 39)
        fputs(": error: this transfer requires an applicable local '(<-)' policy\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 40)
        fputs(": error: a first String field write requires an 'init val var' policy\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 41)
        fputs(": error: ambiguous '(<-)' method selection\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 42)
        fputs(": error: duplicate signature in '(<-)' family\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 43)
        fputs(": error: selected transfer form is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 44)
        fputs(": error: transfer method selection requires an unproved owner equality\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 45)
        fputs(": error: selected callable has no supported native import signature\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 46)
        fputs(": error: ambiguous ordinary method selection\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 47)
        fputs(": error: duplicate signature in ordinary method family\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 48)
        fputs(": error: ordinary method family is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 4 && diagnostic->code == 49)
        fputs(": error: imported reference-slot call requires a checked rebind contract\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 12)
        fputs(": error: ref argument requires a proven non-null target\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 13)
        fputs(": error: target must be initialized before this read or borrow\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 14)
        fputs(": error: initialization proof exceeds available compiler workspace\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 16)
        fputs(": error: ref initialization requires a proven non-null target\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 17)
        fputs(": error: local binding is not established on every path to this use\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 19)
        fputs(": error: 'init val var' place must be initialized on every successful exit\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_cycle_unproved)
        fputs(": error: cyclic module construction remains unproved\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_dependency_rejected)
        fputs(": error: selected source dependency could not be checked\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_dependency_unsupported)
        fputs(": error: selected source dependency is outside the supported object profile\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_graph_storage_failed)
        fputs(": error: could not allocate dependency graph storage\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_depth_unproved)
        fputs(": error: dependency depth exceeds the current compiler profile\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 12)
        fputs(": error: this import is outside the supported enum import profile\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 13)
        fputs(": error: member selection requires a checked enum or sum type\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 14)
        fputs(": error: no such member of the selected enum type\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 15)
        fputs(": error: conflicting enum member selections\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 17)
        fputs(": error: no such constructor of the selected sum type\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 18)
        fputs(": error: conflicting sum constructor selections\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_package_delivery_failed)
        fputs(": error: could not retain package resolution diagnostics\n", stderr);
    else
        fprintf(stderr, ": error: %s failed (code %" PRIu32 ")\n", diagnostic_phase(diagnostic->stage), diagnostic->code);
    if (diagnostic->related.line)
        fprintf(stderr, "%s:%" PRIu64 ":%" PRIu64 ": related location\n",
                input, diagnostic->related.line, diagnostic->related.column);
}

static void write_path_bytes(const unsigned char *bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const unsigned char byte = bytes[i];
        if (byte < 32 || byte == 127) fprintf(stderr, "\\x%02x", (unsigned)byte);
        else if (byte == '\\' || byte == '\'') {
            fputc('\\', stderr);
            fputc(byte, stderr);
        } else fputc(byte, stderr);
    }
}

static void report_package(const cod_booter_package_diagnostic *diagnostic) {
    const unsigned char *importer = diagnostic->bytes;
    const unsigned char *spelling = importer + diagnostic->importer_size;
    const unsigned char *candidate = spelling + diagnostic->spelling_size;
    const unsigned char *other = candidate + diagnostic->candidate_size;
    write_path_bytes(importer, diagnostic->importer_size);
    fprintf(stderr, ":%" PRIu64 ":%" PRIu64 ": %s: ", diagnostic->first.line,
            diagnostic->first.column, diagnostic->kind == package_shadow ? "warning" : "error");
    switch (diagnostic->kind) {
    case package_shadow: fputs("package module '", stderr); break;
    case package_ambiguity: fputs("ambiguous package module '", stderr); break;
    case package_missing: fputs("module is absent from the project packages and active bundled library: '", stderr); break;
    case package_lookup_failure: fputs("could not inspect or open package module '", stderr); break;
    case package_input_failure: fputs("could not load package inputs for module '", stderr); break;
    case invalid_module_path: fputs("invalid source module path '", stderr); break;
    }
    write_path_bytes(spelling, diagnostic->spelling_size);
    fputc('\'', stderr);
    if (diagnostic->candidate_size) {
        fputs(diagnostic->kind == package_shadow ? " selects '" :
              diagnostic->kind == package_ambiguity ? " claimed by '" : " at '", stderr);
        write_path_bytes(candidate, diagnostic->candidate_size);
        fputc('\'', stderr);
    }
    if (diagnostic->other_size) {
        fputs(diagnostic->kind == package_shadow ? " and shadows '" : " and '", stderr);
        write_path_bytes(other, diagnostic->other_size);
        fputc('\'', stderr);
    }
    if (diagnostic->os_error)
        fprintf(stderr, " (system error %" PRIu64 ")", diagnostic->os_error);
    fputc('\n', stderr);
}

static void report_symbol(const cod_booter_symbol_diagnostic *diagnostic) {
    const unsigned char *importer = diagnostic->bytes;
    const unsigned char *spelling = importer + diagnostic->importer_size;
    const unsigned char *module = spelling + diagnostic->spelling_size;
    const unsigned char *definition = module + diagnostic->module_size;
    write_path_bytes(importer, diagnostic->importer_size);
    fprintf(stderr, ":%" PRIu64 ":%" PRIu64 ": error: declaration conflicts with a protected symbol name '",
            diagnostic->location.first.line, diagnostic->location.first.column);
    write_path_bytes(spelling, diagnostic->spelling_size);
    fputs("' (canonical '", stderr);
    write_path_bytes(module, diagnostic->module_size);
    fputs("::@", stderr);
    write_path_bytes(definition, diagnostic->definition_size);
    fputs("')\n", stderr);
}

typedef cod_enum_CompilerStatusx2ecod_14_CompilerStatus compiler_status;

static cod_native_DiagnosticDestination_borrow diagnostic_record(cod_booter_diagnostic_sink *diagnostic,
                                                                 cod_native_loan *destination) {
    *destination = (cod_native_loan){diagnostic, (uint64_t)(uintptr_t)diagnostic};
    return (cod_native_DiagnosticDestination_borrow){
        (uint64_t)(uintptr_t)&diagnostic->primary, destination->owner,
    };
}

static void report_dependency_notes(const cod_booter_diagnostic_event *event, size_t index) {
    for (const cod_booter_dependency_note *note = event->first_note; note; note = note->next) {
        if (note->finding - event->first_finding == index)
            fprintf(stderr, "%s:%" PRIu64 ":%" PRIu64 ": note: selected source dependency here\n",
                note->path, note->first.line, note->first.column);
    }
}

static int finish_command(const char *input, cod_native_status result, compiler_status status,
                          cod_booter_diagnostic_sink *diagnostic) {
    bool package_failure = false;
    bool has_source_reports = false;
    uint8_t latest_source_stage = 0;
    for (cod_booter_diagnostic_event *event = diagnostic->first_event; event;) {
        cod_booter_diagnostic_event *next = event->next;
        if (event->kind == cod_booter_package_report) {
            const cod_booter_package_diagnostic *package = (const cod_booter_package_diagnostic *)event;
            report_package(package);
            report_dependency_notes(event, 0);
            package_failure |= package->kind != package_shadow;
        } else if (event->kind == cod_booter_symbol_report) {
            const cod_booter_symbol_diagnostic *symbol = (const cod_booter_symbol_diagnostic *)event;
            report_symbol(symbol);
            report_dependency_notes(event, 0);
            has_source_reports = true;
            latest_source_stage = symbol->location.stage;
        } else {
            const cod_booter_source_diagnostics *batch = (const cod_booter_source_diagnostics *)event;
            const char *path = (const char *)(batch->reports + batch->count);
            for (size_t i = 0; i < batch->count; ++i) {
                report(path, status, &batch->reports[i]);
                report_dependency_notes(event, i);
            }
            has_source_reports = true;
            latest_source_stage = batch->reports[0].stage;
        }
        for (cod_booter_dependency_note *note = event->first_note; note;) {
            cod_booter_dependency_note *next_note = note->next;
            free(note);
            note = next_note;
        }
        free(event);
        event = next;
    }
    if (result != COD_NATIVE_OK) {
        fputs("cod-booter: invalid native entry\n", stderr);
    } else if (status != cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_Complete) {
        const bool detailed_package_failure = package_failure && diagnostic->primary.stage == 2 &&
            diagnostic->primary.code == module_package_diagnostic;
        const bool limited = status == cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_DiagnosticsIncomplete;
        if (!detailed_package_failure && !diagnostic->primary_retained &&
            (!limited || diagnostic->primary.code != 0) &&
            (!has_source_reports || diagnostic->primary.stage != latest_source_stage))
            report(input, status, &diagnostic->primary);
        if (limited)
            fprintf(stderr, "%s: diagnostic limit %" PRIu64 " reached; coverage incomplete\n",
                input, diagnostic->finding_limit);
        if (diagnostic->snapshot_failed)
            fprintf(stderr, "%s: error: could not retain the complete source diagnostics\n", input);
    }
    if (diagnostic->package_delivery_failed &&
        (diagnostic->primary.stage != 2 || diagnostic->primary.code != module_package_delivery_failed))
        fprintf(stderr, "%s: error: could not retain package resolution diagnostics\n", input);
    return result == COD_NATIVE_OK ? status : 70;
}

static cod_native_status check_declarations(const char *source, cod_booter_diagnostic_sink *diagnostic,
                                            compiler_status *status) {
    const cod_booter_path_owner root = {source, strlen(source), COD_BOOTER_PATH_LAUNCHER};
    const cod_native_loan argument_owner = {&root, (uint64_t)(uintptr_t)&root};
    cod_native_loan destination;
    const cod_native_Path_borrow input = {(uint64_t)(uintptr_t)source, argument_owner.owner};
    const cod_native_DiagnosticDestination_borrow record = diagnostic_record(diagnostic, &destination);
    return cod_native_check_cli_entry(&input, &record, &argument_owner, &destination, status);
}

static cod_native_status generate_providers(const char *source, const char *directory, cod_booter_diagnostic_sink *diagnostic,
                                           compiler_status *status) {
    const cod_booter_path_owner root = {source, strlen(source), COD_BOOTER_PATH_LAUNCHER};
    const cod_booter_path_owner output = {directory, strlen(directory), COD_BOOTER_PATH_LAUNCHER};
    const cod_native_loan input_owner = {&root, (uint64_t)(uintptr_t)&root};
    const cod_native_loan output_owner = {&output, (uint64_t)(uintptr_t)&output};
    cod_native_loan destination;
    const cod_native_Path_borrow input = {(uint64_t)(uintptr_t)source, input_owner.owner};
    const cod_native_Path_borrow target = {(uint64_t)(uintptr_t)directory, output_owner.owner};
    const cod_native_DiagnosticDestination_borrow record = diagnostic_record(diagnostic, &destination);
    return cod_native_providers_cli_entry(&input, &target, &record, &input_owner, &output_owner, &destination, status);
}

static cod_native_status compile(const char *source, const char *object, cod_booter_diagnostic_sink *diagnostic,
                                 compiler_status *status) {
    const cod_booter_path_owner root = {source, strlen(source), COD_BOOTER_PATH_LAUNCHER};
    const cod_native_loan argument_owner = {&root, (uint64_t)(uintptr_t)&root};
    cod_native_loan destination;
    const cod_native_Path_borrow input = {(uint64_t)(uintptr_t)source, argument_owner.owner};
    const size_t output_size = strlen(object);
    const cod_native_text output = {(const uint8_t *)object, output_size, output_size + 1};
    const cod_native_DiagnosticDestination_borrow record = diagnostic_record(diagnostic, &destination);
    return cod_native_compile_cli_entry(&input, &output, &record, &argument_owner, &destination, status);
}

static bool parse_finding_limit(const char *digits, uint64_t *limit) {
    uint64_t parsed = 0;
    if (!*digits) return false;
    for (; *digits; ++digits) {
        if (*digits < '0' || *digits > '9') return false;
        const uint64_t digit = (uint64_t)(*digits - '0');
        if (parsed > (UINT64_MAX - digit) / 10) return false;
        parsed = parsed * 10 + digit;
    }
    if (!parsed) return false;
    *limit = parsed;
    return true;
}

int main(int argc, char **argv) {
    const char *arguments[3];
    size_t count = 0;
    uint64_t limit = 0;
    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--max-diagnostics") == 0) {
            if (limit || ++index >= argc || !parse_finding_limit(argv[index], &limit)) {
                fputs("cod-booter: --max-diagnostics requires one positive integer\n", stderr);
                return 64;
            }
        } else {
            if (count == sizeof(arguments) / sizeof(arguments[0])) {
                count = 0;
                break;
            }
            arguments[count++] = argv[index];
        }
    }
    const bool providers = count && strcmp(arguments[0], "--emit-c-provider") == 0;
    if (count != (providers ? 3u : 2u)) {
        fputs("usage: cod-booter [--max-diagnostics N] SOURCE OUTPUT\n"
              "       cod-booter [--max-diagnostics N] --check-declarations SOURCE\n"
              "       cod-booter [--max-diagnostics N] --emit-c-provider SOURCE DIRECTORY\n", stderr);
        return 64;
    }
    cod_booter_diagnostic_sink diagnostic = {.finding_limit = limit};
    compiler_status status = 0;
    if (providers) {
        if (LLVMInitializeNativeTarget()) {
            fputs("cod-booter: native LLVM target unavailable\n", stderr);
            return 70;
        }
        const cod_native_status result = generate_providers(arguments[1], arguments[2], &diagnostic, &status);
        return finish_command(arguments[1], result, status, &diagnostic);
    }
    if (strcmp(arguments[0], "--check-declarations") == 0) {
        const cod_native_status result = check_declarations(arguments[1], &diagnostic, &status);
        return finish_command(arguments[1], result, status, &diagnostic);
    }
    if (LLVMInitializeNativeTarget() || LLVMInitializeNativeAsmPrinter()) {
        fputs("cod-booter: native LLVM target unavailable\n", stderr);
        return 70;
    }

    const cod_native_status result = compile(arguments[0], arguments[1], &diagnostic, &status);
    return finish_command(arguments[0], result, status, &diagnostic);
}
