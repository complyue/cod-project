#include "../../lang_llvm_commands_c.h"
#include "../../lang_text_literal.h"

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline uint64_t cod_llvm_fault_category(cod_llvm_arithmetic_fault fault) {
  return UINT64_MAX - (uint64_t)fault;
}

typedef struct cod_llvm_function {
  LLVMValueRef value;
  bool begun;
} cod_llvm_function;

typedef struct cod_llvm_construction {
  cod_llvm_session *session;
  LLVMTypeRef word, result, text, pointer;
  cod_llvm_function *functions;
  LLVMValueRef *slots, *arguments;
  LLVMBasicBlockRef *blocks;
  LLVMValueRef current, print, format;
  LLVMBasicBlockRef entry;
  size_t label_count, slot_count, functions_to_begin;
  const uint8_t *source;
  size_t source_size;
  unsigned argument_count, argument_capacity;
} cod_llvm_construction;

static inline LLVMValueRef cod_llvm_word(cod_llvm_construction *state, uint64_t number) {
  return LLVMConstInt(state->word, number, 0);
}

static inline LLVMValueRef cod_llvm_allocate_words(cod_llvm_construction *state, uint64_t count) {
  LLVMBuilderRef builder = state->session->builder;
  LLVMBasicBlockRef current = LLVMGetInsertBlock(builder);
  LLVMValueRef first = LLVMGetFirstInstruction(state->entry);
  if (first) LLVMPositionBuilderBefore(builder, first);
  else LLVMPositionBuilderAtEnd(builder, state->entry);
  LLVMValueRef address = LLVMBuildArrayAlloca(builder, state->word, cod_llvm_word(state, count), "");
  LLVMPositionBuilderAtEnd(builder, current);
  return address;
}

static inline LLVMValueRef cod_llvm_slot(cod_llvm_construction *state, uint64_t index) {
  if (!state->slots[index]) state->slots[index] = cod_llvm_allocate_words(state, 1);
  return state->slots[index];
}

static inline LLVMValueRef cod_llvm_read_word(cod_llvm_construction *state, uint64_t index) {
  return LLVMBuildLoad2(state->session->builder, state->word, cod_llvm_slot(state, index), "");
}

static inline void cod_llvm_write_word(cod_llvm_construction *state, uint64_t index, LLVMValueRef value) {
  LLVMBuildStore(state->session->builder, value, cod_llvm_slot(state, index));
}

static inline LLVMValueRef cod_llvm_read_address(cod_llvm_construction *state, uint64_t index) {
  return LLVMBuildIntToPtr(state->session->builder, cod_llvm_read_word(state, index), state->pointer, "");
}

static LLVMValueRef cod_llvm_text_member(cod_llvm_construction *state, uint64_t slot, unsigned member) {
  const LLVMBuilderRef builder = state->session->builder;
  LLVMValueRef descriptor = cod_llvm_read_address(state, slot);
  LLVMValueRef address = LLVMBuildStructGEP2(builder, state->text, descriptor, member, "");
  return LLVMBuildLoad2(builder, member ? state->word : state->pointer, address, "");
}

static LLVMValueRef cod_llvm_text_global(cod_llvm_construction *state, LLVMValueRef value) {
  LLVMValueRef global = LLVMAddGlobal(state->session->module, LLVMTypeOf(value), "text");
  LLVMSetInitializer(global, value);
  LLVMSetGlobalConstant(global, true);
  LLVMSetLinkage(global, LLVMPrivateLinkage);
  return global;
}

static LLVMValueRef cod_llvm_text_literal(cod_llvm_construction *state, uint64_t first, uint64_t past) {
  if (!state->source || past > state->source_size || first > past || past - first < 2 ||
      state->source[first] != '"' || state->source[past - 1] != '"') return NULL;
  const size_t width = (size_t)(past - first) - 2;
  uint8_t *bytes = malloc(width ? width : 1);
  if (!bytes) return NULL;
  size_t length;
  if (cod_decode_text_literal(state->source + first + 1, width, bytes, &length)) {
    free(bytes);
    return NULL;
  }
  LLVMValueRef contents = LLVMConstStringInContext2(state->session->context, (const char *)bytes, length, false);
  free(bytes);
  LLVMValueRef fields[] = {cod_llvm_text_global(state, contents), cod_llvm_word(state, length), cod_llvm_word(state, length + 1)};
  return cod_llvm_text_global(state, LLVMConstNamedStruct(state->text, fields, 3));
}

static LLVMValueRef cod_llvm_runtime_call(cod_llvm_construction *state, const char *name, LLVMTypeRef result,
                                         LLVMTypeRef *parameters, LLVMValueRef *arguments, unsigned count) {
  LLVMModuleRef module = state->session->module;
  LLVMTypeRef type = LLVMFunctionType(result, parameters, count, false);
  LLVMValueRef function = LLVMGetNamedFunction(module, name);
  if (!function) function = LLVMAddFunction(module, name, type);
  return LLVMBuildCall2(state->session->builder, type, function, arguments, count, "");
}

/* This object reports byte positions directly in its own entry function.
   The shared runtime's optional source-context hook has no prefix here. */
static void cod_llvm_supply_exception_site(cod_llvm_construction *state) {
  LLVMModuleRef module = state->session->module;
  if (LLVMGetNamedFunction(module, "cod_lang_exception_site")) return;
  LLVMTypeRef type = LLVMFunctionType(state->pointer, &state->word, 1, false);
  LLVMValueRef function = LLVMAddFunction(module, "cod_lang_exception_site", type);
  LLVMBuilderRef builder = state->session->builder;
  LLVMBasicBlockRef current = LLVMGetInsertBlock(builder);
  LLVMPositionBuilderAtEnd(builder, LLVMAppendBasicBlockInContext(state->session->context, function, ""));
  LLVMBuildRet(builder, LLVMConstNull(state->pointer));
  LLVMPositionBuilderAtEnd(builder, current);
}

static inline bool cod_llvm_local_block(cod_llvm_construction *state, uint64_t index) {
  return index < state->label_count && state->blocks[index] &&
      LLVMGetBasicBlockParent(state->blocks[index]) == state->current;
}

static inline void cod_llvm_print_word(cod_llvm_construction *state, uint64_t index) {
  cod_llvm_session *session = state->session;
  if (!state->print) {
    LLVMTypeRef pointer = LLVMPointerTypeInContext(session->context, 0);
    LLVMTypeRef type = LLVMFunctionType(LLVMInt32TypeInContext(session->context), &pointer, 1, 1);
    state->print = LLVMAddFunction(session->module, "printf", type);
    state->format = LLVMBuildGlobalStringPtr(session->builder, "%" PRIu64 "\n", "format.word");
  }
  LLVMValueRef args[] = {state->format, cod_llvm_read_word(state, index)};
  LLVMBuildCall2(session->builder, LLVMGlobalGetValueType(state->print), state->print, args, 2, "");
}

static inline void cod_llvm_finish_program(cod_llvm_construction *state, uint64_t category_slot, uint64_t site_slot) {
  cod_llvm_session *session = state->session;
  LLVMBuilderRef builder = session->builder;
  LLVMValueRef category = cod_llvm_read_word(state, category_slot);
  LLVMTypeRef i32 = LLVMInt32TypeInContext(session->context);
  LLVMBasicBlockRef success = LLVMAppendBasicBlockInContext(session->context, state->current, "program.success");
  LLVMBasicBlockRef failure = LLVMAppendBasicBlockInContext(session->context, state->current, "program.failure");
  LLVMValueRef normal = LLVMBuildICmp(builder, LLVMIntEQ, category, cod_llvm_word(state, 0), "");
  LLVMBuildCondBr(builder, normal, success, failure);
  LLVMPositionBuilderAtEnd(builder, success);
  LLVMBuildRet(builder, LLVMConstInt(i32, 0, 0));

  LLVMPositionBuilderAtEnd(builder, failure);
  LLVMTypeRef params[] = {i32, LLVMPointerTypeInContext(session->context, 0)};
  LLVMTypeRef printer = LLVMFunctionType(i32, params, 2, 1);
  LLVMValueRef dprintf_function = LLVMAddFunction(session->module, "dprintf", printer);
  const char *const fault_formats[COD_LLVM_FAULT_COUNT] = {
    [COD_LLVM_FAULT_ADD] = "byte %" PRIu64 ": u64 addition overflow\n",
    [COD_LLVM_FAULT_MULTIPLY] = "byte %" PRIu64 ": u64 multiplication overflow\n",
    [COD_LLVM_FAULT_SUBTRACT] = "byte %" PRIu64 ": u64 subtraction overflow\n",
    [COD_LLVM_FAULT_ZERO_DIVISOR] = "byte %" PRIu64 ": division by zero\n",
  };
  LLVMValueRef format = LLVMBuildGlobalStringPtr(builder, "byte %" PRIu64 ": unhandled CoD exception %" PRIu64 "\n", "");
  for (int fault = 0; fault < COD_LLVM_FAULT_COUNT; ++fault) {
    LLVMValueRef fault_format = LLVMBuildGlobalStringPtr(builder, fault_formats[fault], "");
    LLVMValueRef matches = LLVMBuildICmp(builder, LLVMIntEQ, category,
        cod_llvm_word(state, cod_llvm_fault_category(fault)), "");
    format = LLVMBuildSelect(builder, matches, fault_format, format, "");
  }
  LLVMValueRef args[] = {LLVMConstInt(i32, 2, 0), format, cod_llvm_read_word(state, site_slot), category};
  LLVMBuildCall2(builder, printer, dprintf_function, args, 4, "");
  LLVMBuildRet(builder, LLVMConstInt(i32, 70, 0));
}

static inline bool cod_llvm_checked_arithmetic(cod_llvm_construction *state, cod_llvm_command command) {
  const size_t slots = state->slot_count;
  if (command.a >= slots || command.b >= slots || command.c >= slots || command.d >= slots)
    return false;

  LLVMBuilderRef builder = state->session->builder;
  LLVMValueRef left = cod_llvm_read_word(state, command.b);
  LLVMValueRef right = cod_llvm_read_word(state, command.c);
  LLVMValueRef value, failed;
  cod_llvm_arithmetic_fault fault;
  if (command.opcode == COD_LLVM_DIVIDE || command.opcode == COD_LLVM_REMAINDER) {
    fault = COD_LLVM_FAULT_ZERO_DIVISOR;
    failed = LLVMBuildICmp(builder, LLVMIntEQ, right, cod_llvm_word(state, 0), "");
    /* The CoD command stream takes the fault exit before using this value. */
    LLVMValueRef divisor = LLVMBuildSelect(builder, failed, cod_llvm_word(state, 1), right, "");
    value = command.opcode == COD_LLVM_DIVIDE ? LLVMBuildUDiv(builder, left, divisor, "")
                                             : LLVMBuildURem(builder, left, divisor, "");
  } else {
    const char *name;
    switch (command.opcode) {
      case COD_LLVM_ADD: fault = COD_LLVM_FAULT_ADD; name = "llvm.uadd.with.overflow"; break;
      case COD_LLVM_MULTIPLY: fault = COD_LLVM_FAULT_MULTIPLY; name = "llvm.umul.with.overflow"; break;
      case COD_LLVM_SUBTRACT: fault = COD_LLVM_FAULT_SUBTRACT; name = "llvm.usub.with.overflow"; break;
      default: return false;
    }
    unsigned intrinsic = LLVMLookupIntrinsicID(name, strlen(name));
    if (!intrinsic) return false;
    LLVMValueRef outcome = cod_llvm_build_checked_integer(builder, state->session->module, intrinsic, left, right);
    value = LLVMBuildExtractValue(builder, outcome, 0, "");
    failed = LLVMBuildExtractValue(builder, outcome, 1, "");
  }
  LLVMValueRef category = LLVMBuildSelect(builder, failed, cod_llvm_word(state, cod_llvm_fault_category(fault)),
      cod_llvm_word(state, 0), "");
  cod_llvm_write_word(state, command.a, value);
  cod_llvm_write_word(state, command.d, category);
  return true;
}

static inline bool cod_llvm_invoke_words(cod_llvm_construction *state, cod_llvm_command command) {
  if (command.a >= state->label_count || !state->functions[command.a].value ||
      command.b >= state->slot_count || command.c >= state->slot_count || command.d >= state->slot_count)
    return false;
  LLVMValueRef function = state->functions[command.a].value;
  LLVMTypeRef type = LLVMGlobalGetValueType(function);
  if (LLVMGetReturnType(type) != state->result || state->argument_count != LLVMCountParams(function))
    return false;
  LLVMBuilderRef builder = state->session->builder;
  LLVMValueRef result = LLVMBuildCall2(builder, type, function, state->arguments, state->argument_count, "");
  state->argument_count = 0;
  cod_llvm_write_word(state, command.b, LLVMBuildExtractValue(builder, result, 0, ""));
  cod_llvm_write_word(state, command.c, LLVMBuildExtractValue(builder, result, 1, ""));
  cod_llvm_write_word(state, command.d, LLVMBuildExtractValue(builder, result, 2, ""));
  return true;
}

static inline bool cod_llvm_construct_command(cod_llvm_construction *state, cod_llvm_command command) {
  LLVMBuilderRef builder = state->session->builder;
  const uint64_t a = command.a, b = command.b, c = command.c, d = command.d;
  const size_t slots = state->slot_count;
  if (state->argument_count && command.opcode != COD_LLVM_ARGUMENT && command.opcode != COD_LLVM_INVOKE)
    return false;
  if (command.opcode == COD_LLVM_DECLARE) return true;
  if (command.opcode == COD_LLVM_BEGIN) {
    if (a >= state->label_count) return false;
    cod_llvm_function *function = &state->functions[a];
    if (!function->value || function->begun) return false;
    state->current = function->value;
    if (!cod_llvm_local_block(state, b) || LLVMGetEntryBasicBlock(state->current) != state->blocks[b]) return false;
    function->begun = true;
    --state->functions_to_begin;
    state->entry = state->blocks[b];
    memset(state->slots, 0, slots * sizeof(*state->slots));
    LLVMClearInsertionPosition(builder);
    return true;
  }
  if (command.opcode == COD_LLVM_POSITION) {
    if (!cod_llvm_local_block(state, a) || LLVMGetFirstInstruction(state->blocks[a])) return false;
    LLVMPositionBuilderAtEnd(builder, state->blocks[a]);
    return true;
  }
  LLVMBasicBlockRef current = LLVMGetInsertBlock(builder);
  if (!current || LLVMGetBasicBlockTerminator(current)) return false;

  switch (command.opcode) {
    case COD_LLVM_JUMP:
      if (!cod_llvm_local_block(state, a)) return false;
      LLVMBuildBr(builder, state->blocks[a]);
      return true;
    case COD_LLVM_BRANCH_EQUAL:
      if (a >= slots || !cod_llvm_local_block(state, c) || !cod_llvm_local_block(state, d)) return false;
      LLVMBuildCondBr(builder, LLVMBuildICmp(builder, LLVMIntEQ, cod_llvm_read_word(state, a),
          cod_llvm_word(state, b), ""), state->blocks[c], state->blocks[d]);
      return true;
    case COD_LLVM_BRANCH_CATCHABLE: {
      if (a >= slots || !cod_llvm_local_block(state, b) || !cod_llvm_local_block(state, c)) return false;
      LLVMValueRef category = cod_llvm_read_word(state, a);
      LLVMValueRef failed = LLVMBuildICmp(builder, LLVMIntNE, category, cod_llvm_word(state, 0), "");
      LLVMValueRef language_exception = LLVMBuildICmp(builder, LLVMIntULT, category,
          cod_llvm_word(state, cod_llvm_fault_category(COD_LLVM_FAULT_COUNT - 1)), "");
      LLVMBuildCondBr(builder, LLVMBuildAnd(builder, failed, language_exception, ""), state->blocks[b], state->blocks[c]);
      return true;
    }
    case COD_LLVM_WORD:
      if (a >= slots) return false;
      cod_llvm_write_word(state, a, cod_llvm_word(state, b));
      return true;
    case COD_LLVM_ALLOCATE_WORDS:
      if (a >= slots || b == 0 || b > PTRDIFF_MAX / sizeof(uint64_t)) return false;
      cod_llvm_write_word(state, a, LLVMBuildPtrToInt(builder, cod_llvm_allocate_words(state, b), state->word, ""));
      return true;
    case COD_LLVM_OFFSET_ADDRESS: {
      if (a >= slots || b >= slots || c >= PTRDIFF_MAX / sizeof(uint64_t)) return false;
      LLVMValueRef base = cod_llvm_read_address(state, b);
      LLVMValueRef offset = cod_llvm_word(state, c);
      LLVMValueRef address = LLVMBuildGEP2(builder, state->word, base, &offset, 1, "");
      cod_llvm_write_word(state, a, LLVMBuildPtrToInt(builder, address, state->word, ""));
      return true;
    }
    case COD_LLVM_TEXT_LITERAL: {
      if (a >= slots) return false;
      LLVMValueRef descriptor = cod_llvm_text_literal(state, b, c);
      if (!descriptor) return false;
      cod_llvm_write_word(state, a, LLVMConstPtrToInt(descriptor, state->word));
      return true;
    }
    case COD_LLVM_TEXT_SIZE:
      if (a >= slots || b >= slots) return false;
      cod_llvm_write_word(state, a, cod_llvm_text_member(state, b, 1));
      return true;
    case COD_LLVM_TEXT_BYTE: {
      if (a >= slots || b >= slots || c >= slots) return false;
      LLVMTypeRef byte = LLVMInt8TypeInContext(state->session->context);
      LLVMValueRef data = cod_llvm_text_member(state, b, 0);
      LLVMValueRef index = cod_llvm_read_word(state, c);
      LLVMValueRef address = LLVMBuildGEP2(builder, byte, data, &index, 1, "");
      LLVMValueRef value = LLVMBuildLoad2(builder, byte, address, "");
      cod_llvm_write_word(state, a, LLVMBuildZExt(builder, value, state->word, ""));
      return true;
    }
    case COD_LLVM_TEXT_CONTAINS_NUL: {
      if (a >= slots || b >= slots) return false;
      LLVMTypeRef integer = LLVMInt32TypeInContext(state->session->context);
      LLVMTypeRef types[] = {state->pointer, integer, state->word};
      LLVMValueRef args[] = {cod_llvm_text_member(state, b, 0), LLVMConstNull(integer), cod_llvm_text_member(state, b, 1)};
      LLVMValueRef found = cod_llvm_runtime_call(state, "memchr", state->pointer, types, args, 3);
      LLVMValueRef contains = LLVMBuildIsNotNull(builder, found, "");
      cod_llvm_write_word(state, a, LLVMBuildZExt(builder, contains, state->word, ""));
      return true;
    }
    case COD_LLVM_TEXT_ALLOCATE: {
      if (a >= slots || b >= slots || c >= slots) return false;
      cod_llvm_supply_exception_site(state);
      LLVMTypeRef types[] = {state->pointer, state->pointer};
      LLVMValueRef args[] = {cod_llvm_slot(state, c), cod_llvm_read_address(state, b)};
      LLVMValueRef text = cod_llvm_runtime_call(state, "cod_lang_lexical_text_create", state->pointer, types, args, 2);
      cod_llvm_write_word(state, a, LLVMBuildPtrToInt(builder, text, state->word, ""));
      return true;
    }
    case COD_LLVM_TEXT_RELEASE: {
      if (a >= slots) return false;
      cod_llvm_supply_exception_site(state);
      LLVMValueRef allocations = LLVMBuildLoad2(builder, state->pointer, cod_llvm_slot(state, a), "");
      cod_llvm_runtime_call(state, "cod_lang_lexical_allocations_release",
                            LLVMVoidTypeInContext(state->session->context), &state->pointer, &allocations, 1);
      return true;
    }
    case COD_LLVM_COPY:
    case COD_LLVM_ADDRESS:
    case COD_LLVM_LOAD_TARGET:
    case COD_LLVM_STORE_TARGET:
      if (a >= slots || b >= slots) return false;
      if (command.opcode == COD_LLVM_COPY) cod_llvm_write_word(state, a, cod_llvm_read_word(state, b));
      else if (command.opcode == COD_LLVM_ADDRESS)
        cod_llvm_write_word(state, a, LLVMBuildPtrToInt(builder, cod_llvm_slot(state, b), state->word, ""));
      else if (command.opcode == COD_LLVM_LOAD_TARGET)
        cod_llvm_write_word(state, a, LLVMBuildLoad2(builder, state->word, cod_llvm_read_address(state, b), ""));
      else {
        LLVMValueRef target = cod_llvm_read_address(state, a);
        LLVMBuildStore(builder, cod_llvm_read_word(state, b), target);
      }
      return true;
    case COD_LLVM_COMPARE: {
      const LLVMIntPredicate predicates[] = {
        LLVMIntEQ, LLVMIntNE, LLVMIntULT, LLVMIntULE, LLVMIntUGT, LLVMIntUGE
      };
      if (a >= slots || b >= slots || c >= slots || d >= sizeof(predicates) / sizeof(predicates[0])) return false;
      LLVMValueRef comparison = LLVMBuildICmp(builder, predicates[d], cod_llvm_read_word(state, b),
          cod_llvm_read_word(state, c), "");
      cod_llvm_write_word(state, a, LLVMBuildZExt(builder, comparison, state->word, ""));
      return true;
    }
    case COD_LLVM_ADD:
    case COD_LLVM_MULTIPLY:
    case COD_LLVM_SUBTRACT:
    case COD_LLVM_DIVIDE:
    case COD_LLVM_REMAINDER:
      return cod_llvm_checked_arithmetic(state, command);
    case COD_LLVM_PARAMETER:
      if (a >= slots || b >= LLVMCountParams(state->current)) return false;
      cod_llvm_write_word(state, a, LLVMGetParam(state->current, (unsigned)b));
      return true;
    case COD_LLVM_ARGUMENT:
      if (a >= slots || state->argument_count == state->argument_capacity) return false;
      state->arguments[state->argument_count++] = cod_llvm_read_word(state, a);
      return true;
    case COD_LLVM_INVOKE:
      return cod_llvm_invoke_words(state, command);
    case COD_LLVM_PRINT:
      if (a >= slots) return false;
      cod_llvm_print_word(state, a);
      return true;
    case COD_LLVM_RETURN: {
      if (a >= slots || b >= slots || c >= slots ||
          LLVMGetReturnType(LLVMGlobalGetValueType(state->current)) != state->result) return false;
      LLVMValueRef value = LLVMGetUndef(state->result);
      value = LLVMBuildInsertValue(builder, value, cod_llvm_read_word(state, a), 0, "");
      value = LLVMBuildInsertValue(builder, value, cod_llvm_read_word(state, b), 1, "");
      value = LLVMBuildInsertValue(builder, value, cod_llvm_read_word(state, c), 2, "");
      LLVMBuildRet(builder, value);
      return true;
    }
    case COD_LLVM_FINISH:
      if (b >= slots || c >= slots ||
          LLVMGetReturnType(LLVMGlobalGetValueType(state->current)) != LLVMInt32TypeInContext(state->session->context)) return false;
      cod_llvm_finish_program(state, b, c);
      return true;
    default:
      return false;
  }
}

static inline cod_llvm_command cod_llvm_word_record(const void *records, size_t index) {
  cod_llvm_command command;
  memcpy(&command, (const unsigned char *)records + index * sizeof(command), sizeof(command));
  return command;
}

/* The caller supplies contiguous, initialized records with the layout of
   cod_llvm_command. Access through bytes permits generated C record types.
   Construct one checked program in a fresh LLVM session. Every declared
   function has a body, including the entry. The caller releases the session
   on failure. Object emission verifies complete blocks and the resulting IR. */
bool cod_llvm_construct_source_records(cod_llvm_session *session, const void *records,
    size_t count, size_t labels, size_t slots, const uint8_t *source, size_t source_size) {
  if (!session || !session->builder || !session->context || !session->module ||
      LLVMGetFirstFunction(session->module) || !records || !count || count > PTRDIFF_MAX / sizeof(cod_llvm_command) ||
      !labels || !slots) return false;
  unsigned arity = 1;
  for (size_t i = 0; i < count; ++i) {
    const cod_llvm_command command = cod_llvm_word_record(records, i);
    if (command.opcode != COD_LLVM_DECLARE) continue;
    if (command.b > UINT_MAX || command.b > count) return false;
    if (command.b > arity) arity = (unsigned)command.b;
  }
  cod_llvm_construction state = {
    .session = session, .word = LLVMInt64TypeInContext(session->context),
    .label_count = labels, .slot_count = slots, .argument_capacity = arity,
    .pointer = LLVMPointerTypeInContext(session->context, 0), .source = source, .source_size = source_size
  };
  LLVMTypeRef fields[] = {state.word, state.word, state.word};
  state.result = LLVMStructTypeInContext(session->context, fields, 3, 0);
  fields[0] = state.pointer;
  state.text = LLVMStructTypeInContext(session->context, fields, 3, 0);
  state.functions = calloc(labels, sizeof(*state.functions));
  state.blocks = calloc(labels, sizeof(*state.blocks));
  state.slots = calloc(slots, sizeof(*state.slots));
  state.arguments = calloc(arity, sizeof(*state.arguments));
  LLVMTypeRef *parameters = calloc(arity, sizeof(*parameters));
  bool success = false;
  if (!state.functions || !state.blocks || !state.slots || !state.arguments || !parameters) goto done;

  for (unsigned i = 0; i < arity; ++i) parameters[i] = state.word;
  bool entry_declared = false;
  for (size_t i = 0; i < count; ++i) {
    const cod_llvm_command command = cod_llvm_word_record(records, i);
    if (command.opcode != COD_LLVM_DECLARE) continue;
    if (command.a >= labels || state.functions[command.a].value) goto done;
    bool entry = command.c != 0;
    if (entry && (entry_declared || command.b != 0)) goto done;
    LLVMTypeRef result = entry ? LLVMInt32TypeInContext(session->context) : state.result;
    LLVMTypeRef signature = LLVMFunctionType(result, parameters, (unsigned)command.b, 0);
    char name[32];
    snprintf(name, sizeof(name), "cod.proc.%" PRIu64, command.a);
    LLVMValueRef function = LLVMAddFunction(session->module, entry ? "main" : name, signature);
    if (!entry) LLVMSetLinkage(function, LLVMInternalLinkage);
    state.functions[command.a].value = function;
    ++state.functions_to_begin;
    entry_declared = entry_declared || entry;
  }
  if (!entry_declared) goto done;

  for (size_t i = 0; i < count; ++i) {
    const cod_llvm_command command = cod_llvm_word_record(records, i);
    if (command.opcode != COD_LLVM_POSITION) continue;
    if (command.a >= labels || command.b >= labels || !state.functions[command.b].value || state.blocks[command.a])
      goto done;
    state.blocks[command.a] = LLVMAppendBasicBlockInContext(session->context, state.functions[command.b].value, "");
  }
  for (size_t i = 0; i < count; ++i)
    if (!cod_llvm_construct_command(&state, cod_llvm_word_record(records, i))) goto done;
  success = state.argument_count == 0 && state.functions_to_begin == 0;

done:
  free(parameters);
  free(state.arguments);
  free(state.slots);
  free(state.blocks);
  free(state.functions);
  return success;
}

bool cod_llvm_construct_word_records(cod_llvm_session *session, const void *records,
    size_t count, size_t labels, size_t slots) {
  return cod_llvm_construct_source_records(session, records, count, labels, slots, NULL, 0);
}

bool cod_llvm_construct_words(cod_llvm_session *session, const cod_llvm_command *commands,
    size_t count, size_t labels, size_t slots) {
  return cod_llvm_construct_word_records(session, commands, count, labels, slots);
}
