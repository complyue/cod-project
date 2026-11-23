#ifndef COD_BOOTER_COMPILE_DIAGNOSTIC_H
#define COD_BOOTER_COMPILE_DIAGNOSTIC_H

#include <stddef.h>
#include <stdint.h>

enum {
    llvm_ok, llvm_invalid_command_range, llvm_construction_failed, llvm_verification_failed,
    llvm_emission_failed, llvm_publication_failed
};

/* Private diagnostic codes for the CoD module-selection stage. */
enum {
    module_cycle_unproved = 6,
    module_dependency_rejected = 7,
    module_graph_storage_failed = 8,
    module_depth_unproved = 9,
    module_dependency_unsupported = 16,
};

typedef struct {
    uint64_t line;
    uint64_t column;
} cod_booter_position;

typedef struct {
    uint8_t stage;
    uint32_t code;
    cod_booter_position first;
    cod_booter_position past;
    cod_booter_position related;
    uint64_t os_error;
} cod_booter_diagnostic;

typedef struct {
    cod_booter_diagnostic *reports;
    char *path;
    size_t count;
    cod_booter_diagnostic primary;
} cod_booter_diagnostic_sink;

#endif
