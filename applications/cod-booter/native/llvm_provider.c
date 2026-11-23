#include "cod_providers.h"

#include "compile_diagnostic.h"
#include "lang_llvm_commands_c.h"
#include "lang_llvm_object_c.h"

#include <stdlib.h>
#include <string.h>

_Static_assert(UINTPTR_MAX <= UINT64_MAX, "LLVM handles require a pointer that fits in a handle word");
_Static_assert(sizeof(cod_native_llvm_construct_row1) == sizeof(cod_llvm_command), "unexpected command width");
_Static_assert(sizeof(cod_native_llvm_construct_units_row1) == sizeof(cod_llvm_command), "unexpected command width");
#define COMMAND_OFFSET(field) \
  _Static_assert(offsetof(cod_native_llvm_construct_row1, field) == offsetof(cod_llvm_command, field), \
                 "unexpected command field position"); \
  _Static_assert(offsetof(cod_native_llvm_construct_units_row1, field) == offsetof(cod_llvm_command, field), \
                 "unexpected command field position")
COMMAND_OFFSET(opcode);
COMMAND_OFFSET(a);
COMMAND_OFFSET(b);
COMMAND_OFFSET(c);
COMMAND_OFFSET(d);
#undef COMMAND_OFFSET

cod_native_status cod_native_LLVMSession_construct_impl(uint64_t *resource, cod_native_LLVMSession_failure *failure) {
  cod_llvm_session *session = calloc(1, sizeof(*session));
  if (!session || !cod_llvm_session_open(session, "cod-booter")) {
    free(session);
    failure->status = llvm_construction_failed;
    return COD_NATIVE_FAILURE;
  }
  *resource = (uint64_t)(uintptr_t)session;
  return COD_NATIVE_OK;
}

void cod_native_LLVMSession_destroy_impl(uint64_t resource) {
  cod_llvm_session *session = (cod_llvm_session *)(uintptr_t)resource;
  cod_llvm_session_release(session);
  free(session);
}

static cod_llvm_session *llvm_session(cod_native_LLVMSession_borrow handle) {
  return handle.payload == handle.owner ? (cod_llvm_session *)(uintptr_t)handle.payload : NULL;
}

cod_native_status cod_native_llvm_construct_impl(cod_native_LLVMSession_borrow resource,
                                                cod_native_llvm_construct_view1 commands,
                                                cod_native_llvm_construct_view2 source,
                                                uint64_t used, uint64_t functions, uint64_t labels,
                                                uint64_t slots, uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  if (used > commands.count || (uint64_t)(size_t)used != used || (uint64_t)(size_t)functions != functions ||
      (uint64_t)(size_t)labels != labels || (uint64_t)(size_t)slots != slots ||
      (uint64_t)(size_t)source.count != source.count) {
    *status = llvm_invalid_command_range;
    return COD_NATIVE_OK;
  }
  const cod_llvm_command_unit unit = {
    .records = commands.base, .count = (size_t)used, .labels = (size_t)labels, .slots = (size_t)slots,
    .source = source.base, .source_size = (size_t)source.count
  };
  *status = cod_llvm_construct_units(session, &unit, 1, (size_t)functions) ? llvm_ok : llvm_construction_failed;
  return COD_NATIVE_OK;
}

static bool llvm_prefix(uint64_t used, uint64_t capacity, size_t width, const void *base) {
  return used <= capacity && used <= (uint64_t)PTRDIFF_MAX / width && (!used || base);
}

static bool llvm_range(uint64_t first, uint64_t past, uint64_t used) {
  return first <= past && past <= used;
}

cod_native_status cod_native_llvm_construct_units_impl(cod_native_LLVMSession_borrow resource,
    cod_native_llvm_construct_units_view1 commands, cod_native_llvm_construct_units_view2 units,
    cod_native_llvm_construct_units_view3 literals, cod_native_llvm_construct_units_view4 spellings,
    cod_native_llvm_construct_units_view5 names, uint64_t command_count, uint64_t unit_count,
    uint64_t literal_count, uint64_t spelling_size, uint64_t name_size, uint64_t functions, uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  *status = llvm_invalid_command_range;
  if (!unit_count || !functions || functions != (size_t)functions ||
      !llvm_prefix(command_count, commands.count, sizeof(*commands.base), commands.base) ||
      !llvm_prefix(unit_count, units.count, sizeof(*units.base), units.base) ||
      !llvm_prefix(literal_count, literals.count, sizeof(*literals.base), literals.base) ||
      !llvm_prefix(spelling_size, spellings.count, 1, spellings.base) ||
      !llvm_prefix(name_size, names.count, 1, names.base) ||
      unit_count > PTRDIFF_MAX / sizeof(cod_llvm_command_unit) ||
      literal_count > PTRDIFF_MAX / sizeof(cod_llvm_literal_spelling))
    return COD_NATIVE_OK;

  cod_llvm_command_unit *native_units = calloc((size_t)unit_count, sizeof(*native_units));
  cod_llvm_literal_spelling *native_literals = NULL;
  size_t prepared_units = 0;
  if (!native_units) {
    *status = llvm_construction_failed;
    return COD_NATIVE_OK;
  }
  if (literal_count) {
    native_literals = malloc((size_t)literal_count * sizeof(*native_literals));
    if (!native_literals) {
      *status = llvm_construction_failed;
      goto done;
    }
  }
  for (size_t i = 0; i < (size_t)literal_count; ++i) {
    const cod_native_llvm_construct_units_row3 *range = &literals.base[i];
    if (!llvm_range(range->first, range->past, spelling_size) || range->past == range->first) goto done;
    native_literals[i] = (cod_llvm_literal_spelling){
      .bytes = spellings.base + (size_t)range->first, .size = (size_t)(range->past - range->first)
    };
  }
  for (size_t i = 0; i < (size_t)unit_count; ++i) {
    const cod_native_llvm_construct_units_row2 *row = &units.base[i];
    if (!llvm_range(row->command_first, row->command_past, command_count) ||
        row->command_first == row->command_past ||
        !llvm_range(row->literal_first, row->literal_past, literal_count) ||
        !llvm_range(row->name_first, row->name_past, name_size) || row->name_first == row->name_past ||
        row->source_size != (size_t)row->source_size ||
        row->labels != (size_t)row->labels || row->slots != (size_t)row->slots)
      goto done;
    const uint8_t *spelling = names.base + (size_t)row->name_first;
    const size_t length = (size_t)(row->name_past - row->name_first);
    if (memchr(spelling, 0, length)) goto done;
    char *name = malloc(length + 1);
    if (!name) {
      *status = llvm_construction_failed;
      goto done;
    }
    memcpy(name, spelling, length);
    name[length] = '\0';
    const size_t unit_literals = (size_t)(row->literal_past - row->literal_first);
    native_units[i] = (cod_llvm_command_unit){
      .records = commands.base + (size_t)row->command_first,
      .count = (size_t)(row->command_past - row->command_first),
      .labels = (size_t)row->labels, .slots = (size_t)row->slots,
      .source_size = (size_t)row->source_size, .source_name = name,
      .literals = unit_literals ? native_literals + (size_t)row->literal_first : NULL,
      .literal_count = unit_literals
    };
    ++prepared_units;
  }
  *status = cod_llvm_construct_units(session, native_units, (size_t)unit_count, (size_t)functions)
      ? llvm_ok : llvm_construction_failed;
done:
  for (size_t i = 0; i < prepared_units; ++i) free((void *)native_units[i].source_name);
  free(native_literals);
  free(native_units);
  return COD_NATIVE_OK;
}

cod_native_status cod_native_llvm_verify_impl(cod_native_LLVMSession_borrow resource, uint8_t *status) {
  const cod_llvm_session *session = llvm_session(resource);
  if (!session) return COD_NATIVE_BOUNDARY_ERROR;
  char *message = NULL;
  *status = LLVMVerifyModule(session->module, LLVMReturnStatusAction, &message) ? llvm_verification_failed : llvm_ok;
  if (message) LLVMDisposeMessage(message);
  return COD_NATIVE_OK;
}

/* The launcher registers the native target and its assembly printer. */
cod_native_status cod_native_llvm_emit_object_impl(cod_native_LLVMSession_borrow resource, const char *path,
                                                  uint8_t *status) {
  cod_llvm_session *session = llvm_session(resource);
  if (!session || !path) return COD_NATIVE_BOUNDARY_ERROR;
  cod_llvm_object_result result = cod_llvm_emit_object(session->module);
  *status = llvm_emission_failed;
  if (result.status == COD_LLVM_OBJECT_OK)
    *status = cod_llvm_publish_object(result.object, path) ? llvm_ok : llvm_publication_failed;
  cod_llvm_object_release(&result);
  return COD_NATIVE_OK;
}
