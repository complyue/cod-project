/*===--- lang_intrinsic_catalog.h - Built-in public selections ------------*/
#ifndef COD_LANG_INTRINSIC_CATALOG_H
#define COD_LANG_INTRINSIC_CATALOG_H

#include <stddef.h>
#include <stdint.h>

#define COD_LANG_EXCEPTION_MODULE "std:exceptions"
#define COD_LANG_IO_MODULE "std:io"
#define COD_LANG_PRINT_NAME "print"
#define COD_LANG_TEXT_STORAGE_MODULE "std:text-storage"
#define COD_LANG_BUILDER_STORAGE_NAME "Utf8BuilderStorage"
#define COD_LANG_BUILDER_APPEND_NAME "append_utf8"
#define COD_LANG_LEXICAL_RESULT_NAME "result_in_scope"

/* Keep runtime categories and their public type facts in one definition. */
#define COD_LANG_INTRINSIC_EXCEPTIONS(ENTRY) \
  ENTRY(system, Exception, 0, 0) \
  ENTRY(out_of_memory, OutOfMemoryError, 1, 0) \
  ENTRY(region_capacity_exceeded, RegionCapacityExceededError, 2, 0) \
  ENTRY(invalid_capacity, InvalidCapacityError, 3, 0) \
  ENTRY(null_pointer, NullPointerException, 4, 1) \
  ENTRY(missing_return, MissingReturnError, 5, 0) \
  ENTRY(unsound_runtime_typing, UnsoundRuntimeTypingError, 6, 0) \
  ENTRY(statically_unknown_type, StaticallyUnknownTypeException, 7, 1) \
  ENTRY(index_out_of_bounds, IndexOutOfBoundsError, 8, 0) \
  ENTRY(unexpected_case, UnexpectedCaseError, 9, 0) \
  ENTRY(type_assertion_failed, TypeAssertionFailedError, 10, 0)

/* An identity is read with its kind. These records describe publications
   during checking, independently of a compiler's runtime symbol table. */
enum cod_lang_export_kind {
  COD_LANG_EXCEPTION_TYPE_EXPORT = 1,
  COD_LANG_SYMBOL_EXPORT = 2,
  COD_LANG_BUILDER_STORAGE_EXPORT = 11,
  COD_LANG_BUILDER_APPEND_EXPORT = 12,
  COD_LANG_LEXICAL_RESULT_EXPORT = 14
};

enum cod_lang_intrinsic_symbol {
  COD_LANG_PRINT_SYMBOL = 0
};

typedef struct cod_lang_intrinsic_export {
  uint64_t module_first, module_past;
  uint64_t name_first, name_past;
  uint64_t identity;
  uint8_t kind, checked;
  uint64_t publishing_module, origin_module, owner_module, owner_identity;
  uint8_t shape;
} cod_lang_intrinsic_export;

/* Byte spans keep the immutable descriptor table free of process pointers.
   Each span excludes the C string terminator. */
typedef struct cod_lang_intrinsic_spellings {
  char exceptions[sizeof(COD_LANG_EXCEPTION_MODULE)];
  char io[sizeof(COD_LANG_IO_MODULE)];
  char print[sizeof(COD_LANG_PRINT_NAME)];
  char text_storage[sizeof(COD_LANG_TEXT_STORAGE_MODULE)];
  char builder_storage[sizeof(COD_LANG_BUILDER_STORAGE_NAME)];
  char builder_append[sizeof(COD_LANG_BUILDER_APPEND_NAME)];
  char lexical_result[sizeof(COD_LANG_LEXICAL_RESULT_NAME)];
#define COD_LANG_SPELLING_FIELD(key, spelling, id, checked) char key[sizeof(#spelling)];
  COD_LANG_INTRINSIC_EXCEPTIONS(COD_LANG_SPELLING_FIELD)
#undef COD_LANG_SPELLING_FIELD
} cod_lang_intrinsic_spellings;

static const cod_lang_intrinsic_spellings cod_lang_intrinsic_names = {
    COD_LANG_EXCEPTION_MODULE, COD_LANG_IO_MODULE, COD_LANG_PRINT_NAME,
    COD_LANG_TEXT_STORAGE_MODULE, COD_LANG_BUILDER_STORAGE_NAME, COD_LANG_BUILDER_APPEND_NAME, COD_LANG_LEXICAL_RESULT_NAME,
#define COD_LANG_SPELLING_VALUE(key, spelling, id, checked) #spelling,
    COD_LANG_INTRINSIC_EXCEPTIONS(COD_LANG_SPELLING_VALUE)
#undef COD_LANG_SPELLING_VALUE
};

#define COD_LANG_SPELLING_SPAN(field, spelling) \
  offsetof(cod_lang_intrinsic_spellings, field), \
  offsetof(cod_lang_intrinsic_spellings, field) + sizeof(spelling) - 1

static const cod_lang_intrinsic_export cod_lang_intrinsic_exports[] = {
#define COD_LANG_EXCEPTION_EXPORT(key, spelling, id, checked) \
  {COD_LANG_SPELLING_SPAN(exceptions, COD_LANG_EXCEPTION_MODULE), COD_LANG_SPELLING_SPAN(key, #spelling), \
      id, COD_LANG_EXCEPTION_TYPE_EXPORT, checked, 0, 0, 0, 0, 0},
    COD_LANG_INTRINSIC_EXCEPTIONS(COD_LANG_EXCEPTION_EXPORT)
#undef COD_LANG_EXCEPTION_EXPORT
    {COD_LANG_SPELLING_SPAN(io, COD_LANG_IO_MODULE), COD_LANG_SPELLING_SPAN(print, COD_LANG_PRINT_NAME),
        COD_LANG_PRINT_SYMBOL, COD_LANG_SYMBOL_EXPORT, 0, 0, 0, 0, 0, 0},
    {COD_LANG_SPELLING_SPAN(text_storage, COD_LANG_TEXT_STORAGE_MODULE), COD_LANG_SPELLING_SPAN(builder_storage, COD_LANG_BUILDER_STORAGE_NAME),
        0, COD_LANG_BUILDER_STORAGE_EXPORT, 0, 0, 0, 0, 0, 0},
    {COD_LANG_SPELLING_SPAN(text_storage, COD_LANG_TEXT_STORAGE_MODULE), COD_LANG_SPELLING_SPAN(builder_append, COD_LANG_BUILDER_APPEND_NAME),
        0, COD_LANG_BUILDER_APPEND_EXPORT, 0, 0, 0, 0, 0, 0},
    {COD_LANG_SPELLING_SPAN(text_storage, COD_LANG_TEXT_STORAGE_MODULE), COD_LANG_SPELLING_SPAN(lexical_result, COD_LANG_LEXICAL_RESULT_NAME),
        0, COD_LANG_LEXICAL_RESULT_EXPORT, 0, 0, 0, 0, 0, 0},
};

#undef COD_LANG_SPELLING_SPAN
#endif
