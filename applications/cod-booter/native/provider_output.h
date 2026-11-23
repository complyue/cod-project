#ifndef COD_BOOTER_PROVIDER_OUTPUT_H
#define COD_BOOTER_PROVIDER_OUTPUT_H

enum {
    provider_output_conflict = 1,
    provider_output_invalid,
    provider_output_io,
    provider_output_memory,
    provider_output_reserved_name,
};
enum { provider_output_stage = 8 };

#endif
