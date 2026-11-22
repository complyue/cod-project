/*===--- lang_llvm_commands_c.h - Private word and block construction -----*/
#ifndef COD_LANG_LLVM_COMMANDS_C_H
#define COD_LANG_LLVM_COMMANDS_C_H

#include "lang_llvm_c.h"

#include <stdint.h>

/* Keep this compiler-private operation table aligned with NativeCommands.cod. */
typedef enum cod_llvm_command_kind {
  COD_LLVM_DECLARE = 1, COD_LLVM_BEGIN, COD_LLVM_POSITION, COD_LLVM_JUMP,
  COD_LLVM_BRANCH_EQUAL, COD_LLVM_WORD, COD_LLVM_COPY, COD_LLVM_ADDRESS,
  COD_LLVM_LOAD_TARGET, COD_LLVM_STORE_TARGET, COD_LLVM_ADD, COD_LLVM_MULTIPLY,
  COD_LLVM_PARAMETER, COD_LLVM_ARGUMENT, COD_LLVM_INVOKE, COD_LLVM_PRINT,
  COD_LLVM_RETURN, COD_LLVM_FINISH, COD_LLVM_COMPARE, COD_LLVM_SUBTRACT,
  COD_LLVM_DIVIDE, COD_LLVM_REMAINDER, COD_LLVM_BRANCH_CATCHABLE,
  COD_LLVM_ALLOCATE_WORDS = 29, COD_LLVM_OFFSET_ADDRESS
} cod_llvm_command_kind;

/* Arithmetic faults occupy the top of the outcome category range. */
typedef enum cod_llvm_arithmetic_fault {
  COD_LLVM_FAULT_ADD, COD_LLVM_FAULT_MULTIPLY, COD_LLVM_FAULT_SUBTRACT,
  COD_LLVM_FAULT_ZERO_DIVISOR, COD_LLVM_FAULT_COUNT
} cod_llvm_arithmetic_fault;

typedef struct cod_llvm_command {
  uint8_t opcode;
  uint64_t a, b, c, d;
} cod_llvm_command;

#ifdef __cplusplus
extern "C" {
#endif

bool cod_llvm_construct_word_records(cod_llvm_session *session, const void *records, size_t count,
                                     size_t labels, size_t slots);
bool cod_llvm_construct_words(cod_llvm_session *session, const cod_llvm_command *commands, size_t count,
                              size_t labels, size_t slots);

#ifdef __cplusplus
}
#endif

#endif
