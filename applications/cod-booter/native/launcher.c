#include "cod_providers.h"

#include "cod_region_storage.h"
#include "compile_diagnostic.h"
#include "module_graph.h"
#include "provider_output.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
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
    case 38: return "field selection";
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
    case 108: return "direct String place lowering";
    case 109: return "reference assignment lowering";
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
    else if (diagnostic->stage == 1 && diagnostic->code == 113)
        fputs(": error: invalid UTF-8 text literal or escape\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 77)
        fputs(": error: catch annotation must name an exception type\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 7)
        fputs(": error: native handle binding is unproved on this path\n", stderr);
    else if (diagnostic->stage == 3 && diagnostic->code == 13)
        fputs(": error: this module declaration is outside the supported compiler profile\n", stderr);
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
        fputs(": error: a function cannot call a C provider\n", stderr);
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
    else if (diagnostic->stage == 2 && diagnostic->code == module_cycle_unproved)
        fputs(": error: cyclic module construction remains unproved\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_dependency_rejected)
        fputs(": error: selected source dependency could not be checked\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_graph_storage_failed)
        fputs(": error: could not allocate dependency graph storage\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_depth_unproved)
        fputs(": error: dependency depth exceeds the current compiler profile\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 12)
        fputs(": error: this import is outside the supported enum import profile\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 13)
        fputs(": error: member selection requires a checked enum type\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 14)
        fputs(": error: no such member of the selected enum type\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == 15)
        fputs(": error: conflicting enum member selections\n", stderr);
    else
        fprintf(stderr, ": error: %s failed (code %" PRIu32 ")\n", diagnostic_phase(diagnostic->stage), diagnostic->code);
    if (diagnostic->related.line)
        fprintf(stderr, "%s:%" PRIu64 ":%" PRIu64 ": related location\n",
                input, diagnostic->related.line, diagnostic->related.column);
}

typedef cod_enum_CompilerStatusx2ecod_14_CompilerStatus compiler_status;

static cod_native_status compile(const char *source, const char *object, cod_booter_diagnostic *diagnostic,
                                 compiler_status *status) {
    *diagnostic = (cod_booter_diagnostic){0};
    const char *arguments[] = {source, object};
    const cod_native_loan argument_owner = {arguments, (uint64_t)(uintptr_t)arguments};
    const cod_native_loan destination = {diagnostic, (uint64_t)(uintptr_t)diagnostic};
    const cod_native_Path_borrow input = {(uint64_t)(uintptr_t)source, argument_owner.owner};
    const size_t output_size = strlen(object);
    const cod_native_text output = {(const uint8_t *)object, output_size, output_size + 1};
    const cod_native_DiagnosticDestination_borrow record = {destination.owner, destination.owner};
    return cod_native_compile_cli_entry(&input, &output, &record, &argument_owner, &destination, status);
}

int main(int argc, char **argv) {
    const bool providers = argc > 1 && strcmp(argv[1], "--emit-c-provider") == 0;
    if (argc != (providers ? 4 : 3)) {
        fputs("usage: cod-booter SOURCE OUTPUT\n       cod-booter --check-declarations SOURCE\n"
              "       cod-booter --emit-c-provider SOURCE DIRECTORY\n", stderr);
        return 64;
    }
    if (providers) {
        cod_booter_diagnostic diagnostic;
        const uint8_t status = cod_booter_emit_c_providers(argv[2], argv[3], &diagnostic);
        if (status) report(argv[2], status, &diagnostic);
        return status;
    }
    if (strcmp(argv[1], "--check-declarations") == 0) {
        cod_booter_diagnostic diagnostic;
        const uint8_t status = cod_booter_check_declarations(argv[2], &diagnostic);
        if (status) report(argv[2], status, &diagnostic);
        return status;
    }
    if (LLVMInitializeNativeTarget() || LLVMInitializeNativeAsmPrinter()) {
        fputs("cod-booter: native LLVM target unavailable\n", stderr);
        return 70;
    }

    cod_booter_diagnostic diagnostic;
    compiler_status status;
    if (compile(argv[1], argv[2], &diagnostic, &status) != COD_NATIVE_OK) {
        fputs("cod-booter: invalid native entry\n", stderr);
        return 70;
    }
    if (status != cod_enum_CompilerStatusx2ecod_14_CompilerStatus_member_Complete)
        report(argv[1], status, &diagnostic);
    return status;
}
