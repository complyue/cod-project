#ifndef COD_BOOTER_MODULE_GRAPH_H
#define COD_BOOTER_MODULE_GRAPH_H

#include "compile_diagnostic.h"

/* Private module-selection diagnostic codes following the binder's 1–5. */
enum {
    module_source_unavailable = 1,
    module_cycle_unproved = 6,
    module_dependency_rejected = 7,
    module_graph_storage_failed = 8,
    module_depth_unproved = 9,
};

uint8_t cod_booter_check_declarations(const char *input, cod_booter_diagnostic *diagnostic);
uint8_t cod_booter_emit_c_providers(const char *input, const char *directory, cod_booter_diagnostic *diagnostic);

#endif
