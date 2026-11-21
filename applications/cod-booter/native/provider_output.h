#ifndef COD_BOOTER_PROVIDER_OUTPUT_H
#define COD_BOOTER_PROVIDER_OUTPUT_H

#include "cod_providers.h"
#include <stddef.h>

enum {
    provider_output_unsupported = 1,
    provider_output_conflict,
    provider_output_invalid,
    provider_output_io,
    provider_output_memory,
    provider_output_reserved_name,
};
enum { provider_output_stage = 8 };

uint32_t cod_booter_write_c_providers(const char *directory,
    const cod_native_graph_frame_use_row9 *rows, size_t row_count,
    const unsigned char *spellings, size_t spelling_count);

#endif
