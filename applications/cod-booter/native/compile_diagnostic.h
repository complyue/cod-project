#ifndef COD_BOOTER_COMPILE_DIAGNOSTIC_H
#define COD_BOOTER_COMPILE_DIAGNOSTIC_H

#include <stdbool.h>
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
    module_package_diagnostic = 19,
    module_package_delivery_failed = 20,
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

enum {
    package_shadow = 1,
    package_ambiguity,
    package_missing,
    package_lookup_failure,
    package_input_failure,
    invalid_module_path,
    package_process_failure,
};

typedef enum {
    cod_booter_source_reports,
    cod_booter_package_report,
} cod_booter_diagnostic_kind;

typedef struct cod_booter_dependency_note {
    struct cod_booter_dependency_note *next;
    uint64_t finding;
    cod_booter_position first;
    cod_booter_position past;
    char path[];
} cod_booter_dependency_note;

typedef struct cod_booter_diagnostic_event {
    struct cod_booter_diagnostic_event *next;
    cod_booter_diagnostic_kind kind;
    uint64_t first_finding;
    cod_booter_dependency_note *first_note;
    cod_booter_dependency_note *last_note;
} cod_booter_diagnostic_event;

typedef struct {
    cod_booter_diagnostic_event event;
    size_t count;
    cod_booter_diagnostic reports[];
} cod_booter_source_diagnostics;

typedef struct {
    cod_booter_diagnostic_event event;
    uint8_t kind;
    uint64_t os_error;
    cod_booter_position first;
    cod_booter_position past;
    size_t importer_size;
    size_t spelling_size;
    size_t candidate_size;
    size_t other_size;
    unsigned char bytes[];
} cod_booter_package_diagnostic;

typedef struct {
    cod_booter_diagnostic_event *first_event;
    cod_booter_diagnostic_event *last_event;
    bool snapshot_failed;
    bool primary_retained;
    cod_booter_diagnostic primary;
    bool package_delivery_failed;
    uint64_t finding_limit;
} cod_booter_diagnostic_sink;

#endif
