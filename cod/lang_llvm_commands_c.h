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
  COD_LLVM_ALLOCATE_WORDS = 29, COD_LLVM_OFFSET_ADDRESS, COD_LLVM_TEXT_LITERAL,
  COD_LLVM_TEXT_SIZE, COD_LLVM_TEXT_BYTE, COD_LLVM_TEXT_CONTAINS_NUL,
  COD_LLVM_TEXT_ALLOCATE, COD_LLVM_TEXT_RELEASE,
  COD_LLVM_SOURCE_SITE = 39, COD_LLVM_HERE_ENTRY, COD_LLVM_INVOKE_HERE
} cod_llvm_command_kind;

/* DECLARE.d distinguishes an outlined here entry, whose first parameter is
   the address of a borrowed descriptor. HERE_ENTRY writes that outline's
   address to a slot. The command producer stores it in descriptor word 0
   and owns the layout of the remaining capture and destination words.

   INVOKE and INVOKE_HERE write the result payload at b and the category
   and site at c and d. The site and target immediately follow the category.
   INVOKE_HERE.a names the slot containing the descriptor address. Its
   preceding ARGUMENT records supply only the offers, in offer order.
   RETURN reads payload, category, site, and target from a through d.
   RETURN.d of zero supplies the constant zero as the target.
   Target zero denotes ordinary success or failure. A nonzero target is
   the address identifying a live lexical destination, even during unwind. */

/* Arithmetic faults occupy the top of the outcome category range. */
typedef enum cod_llvm_arithmetic_fault {
  COD_LLVM_FAULT_ADD, COD_LLVM_FAULT_MULTIPLY, COD_LLVM_FAULT_SUBTRACT,
  COD_LLVM_FAULT_ZERO_DIVISOR, COD_LLVM_FAULT_COUNT
} cod_llvm_arithmetic_fault;

typedef struct cod_llvm_command {
  uint8_t opcode;
  uint64_t a, b, c, d;
} cod_llvm_command;

typedef struct cod_llvm_literal_spelling {
  const uint8_t *bytes;
  size_t size;
} cod_llvm_literal_spelling;

/* A checked command unit borrows its records and one indexed source revision
   for construction. Block labels and slots are unit-local. Callable IDs
   are artifact-wide. Only direct INVOKE may reference another unit's definition.
   Multi-unit artifacts supply each source's display name for runtime sites. */
typedef struct cod_llvm_command_unit {
  const void *records;
  size_t count, labels, slots;
  const uint8_t *source;
  size_t source_size;
  const char *source_name;
  /* TEXT_LITERAL.d selects an optional retained spelling (one-based).
     Its byte span is still relative to the original revision. */
  const cod_llvm_literal_spelling *literals;
  size_t literal_count;
} cod_llvm_command_unit;

#ifdef __cplusplus
extern "C" {
#endif

/* Construct a closed artifact in a fresh session. All declarations must
   have bodies and exactly one is the entry. Release the session on failure. */
bool cod_llvm_construct_units(cod_llvm_session *session, const cod_llvm_command_unit *units,
                             size_t unit_count, size_t function_count);
bool cod_llvm_construct_word_records(cod_llvm_session *session, const void *records, size_t count,
                                     size_t labels, size_t slots);
bool cod_llvm_construct_source_records(cod_llvm_session *session, const void *records, size_t count,
                                       size_t labels, size_t slots, const uint8_t *source, size_t source_size);
bool cod_llvm_construct_words(cod_llvm_session *session, const cod_llvm_command *commands, size_t count,
                              size_t labels, size_t slots);

#ifdef __cplusplus
}
#endif

#endif
