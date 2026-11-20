#ifndef COD_BOOTER_COMPILE_DIAGNOSTIC_H
#define COD_BOOTER_COMPILE_DIAGNOSTIC_H

#include <stdint.h>

enum {
    llvm_ok, llvm_invalid_command_range, llvm_construction_failed, llvm_verification_failed,
    llvm_emission_failed, llvm_publication_failed
};

typedef struct {
    uint64_t line;
    uint64_t column;
} cod_booter_position;

typedef struct {
    uint8_t stage;
    uint8_t code;
    cod_booter_position first;
    cod_booter_position past;
    cod_booter_position related;
    uint64_t os_error;
} cod_booter_diagnostic;

#endif
