#include "cod_providers.h"

#include "cod_region_storage.h"
#include "compile_diagnostic.h"
#include "module_graph.h"

#include <inttypes.h>
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

static const char *unsupported_source_form(uint32_t code) {
    switch (code) {
    case 28: return "owning exception local";
    case 29: return "owning exception parameter";
    case 30: return "owning exception result";
    case 33: return "owning make exception expression";
    case 34: return "borrowed reference local";
    case 35: return "borrowed reference result";
    case 36: return "writable owning exception catch";
    case 37: return "ordinary construction";
    case 38: return "field selection";
    case 39: return "readonly reference parameter or const callable owner";
    case 40: return "bodyless deferred exception handler";
    case 41: return "lexical callable value";
    case 42: return "throw with an explicit cause";
    case 43: return "braced exception construction in throw";
    case 44: return "rethrow with an explicit cause";
    case 45: return "named call argument";
    case 46: return "reference assignment in a construction initializer";
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
    else if (unsupported)
        fprintf(stderr, ": error: %s is outside the supported compiler profile\n", unsupported);
    else if (diagnostic->stage == 3 && diagnostic->code == 13)
        fputs(": error: this module declaration is outside the supported compiler profile\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 12)
        fputs(": error: ref argument requires a proven non-null target\n", stderr);
    else if (diagnostic->stage == 5 && diagnostic->code == 13)
        fputs(": error: target must be initialized before this read or borrow\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_cycle_unproved)
        fputs(": error: cyclic module construction remains unproved\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_dependency_rejected)
        fputs(": error: selected source dependency could not be checked\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_graph_storage_failed)
        fputs(": error: could not allocate dependency graph storage\n", stderr);
    else if (diagnostic->stage == 2 && diagnostic->code == module_depth_unproved)
        fputs(": error: dependency depth exceeds the current compiler profile\n", stderr);
    else
        fprintf(stderr, ": error: %s failed (code %" PRIu32 ")\n", diagnostic_phase(diagnostic->stage), diagnostic->code);
    if (diagnostic->related.line)
        fprintf(stderr, "%s:%" PRIu64 ":%" PRIu64 ": related location\n",
                input, diagnostic->related.line, diagnostic->related.column);
}

static cod_native_status compile(const char *source, const char *object, cod_booter_diagnostic *diagnostic, uint8_t *status) {
    *diagnostic = (cod_booter_diagnostic){0};
    const char *arguments[] = {source, object};
    const cod_native_loan paths = {arguments, (uint64_t)(uintptr_t)arguments};
    const cod_native_loan destination = {diagnostic, (uint64_t)(uintptr_t)diagnostic};
    const cod_native_Path_borrow input = {(uint64_t)(uintptr_t)source, paths.owner};
    const cod_native_Path_borrow output = {(uint64_t)(uintptr_t)object, paths.owner};
    const cod_native_DiagnosticDestination_borrow record = {destination.owner, destination.owner};
    return cod_native_compile_cli_entry(&input, &output, &record, &paths, &destination, status);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fputs("usage: cod-booter SOURCE OUTPUT\n       cod-booter --check-declarations SOURCE\n", stderr);
        return 64;
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
    uint8_t status = 0;
    if (compile(argv[1], argv[2], &diagnostic, &status) != COD_NATIVE_OK) {
        fputs("cod-booter: invalid native entry\n", stderr);
        return 70;
    }
    if (status) report(argv[1], status, &diagnostic);
    return status;
}
