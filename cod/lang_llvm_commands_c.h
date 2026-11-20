/*===--- lang_llvm_commands_c.h - Private word and block construction -----*/
#ifndef COD_LANG_LLVM_COMMANDS_C_H
#define COD_LANG_LLVM_COMMANDS_C_H

#include "lang_llvm_c.h"

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keep this compiler-private operation table aligned with NativeCommands.cod. */
typedef enum cod_llvm_command_kind {
  COD_LLVM_DECLARE = 1, COD_LLVM_BEGIN, COD_LLVM_POSITION, COD_LLVM_JUMP,
  COD_LLVM_BRANCH_EQUAL, COD_LLVM_WORD, COD_LLVM_COPY, COD_LLVM_ADDRESS,
  COD_LLVM_LOAD_TARGET, COD_LLVM_STORE_TARGET, COD_LLVM_ADD, COD_LLVM_MULTIPLY,
  COD_LLVM_PARAMETER, COD_LLVM_ARGUMENT, COD_LLVM_INVOKE, COD_LLVM_PRINT,
  COD_LLVM_RETURN, COD_LLVM_FINISH, COD_LLVM_COMPARE
} cod_llvm_command_kind;

typedef struct cod_llvm_command {
  uint8_t opcode;
  uint64_t a, b, c, d;
} cod_llvm_command;

typedef struct cod_llvm_construction {
  cod_llvm_session *session;
  LLVMTypeRef word, result;
  LLVMValueRef *functions, *slots, *arguments;
  LLVMBasicBlockRef *blocks;
  LLVMValueRef current, print, format;
  LLVMBasicBlockRef entry;
  size_t label_count, slot_count;
  unsigned argument_count, argument_capacity;
} cod_llvm_construction;

static inline LLVMValueRef cod_llvm_word(cod_llvm_construction *state, uint64_t number) {
  return LLVMConstInt(state->word, number, 0);
}

static inline LLVMValueRef cod_llvm_slot(cod_llvm_construction *state, uint64_t index) {
  if (!state->slots[index]) {
    LLVMBuilderRef builder = state->session->builder;
    LLVMBasicBlockRef current = LLVMGetInsertBlock(builder);
    LLVMValueRef first = LLVMGetFirstInstruction(state->entry);
    if (first) LLVMPositionBuilderBefore(builder, first);
    else LLVMPositionBuilderAtEnd(builder, state->entry);
    state->slots[index] = LLVMBuildAlloca(builder, state->word, "");
    LLVMPositionBuilderAtEnd(builder, current);
  }
  return state->slots[index];
}

static inline LLVMValueRef cod_llvm_read_word(cod_llvm_construction *state, uint64_t index) {
  return LLVMBuildLoad2(state->session->builder, state->word, cod_llvm_slot(state, index), "");
}

static inline void cod_llvm_write_word(cod_llvm_construction *state, uint64_t index, LLVMValueRef value) {
  LLVMBuildStore(state->session->builder, value, cod_llvm_slot(state, index));
}

static inline LLVMValueRef cod_llvm_read_address(cod_llvm_construction *state, uint64_t index) {
  LLVMTypeRef pointer = LLVMPointerTypeInContext(state->session->context, 0);
  return LLVMBuildIntToPtr(state->session->builder, cod_llvm_read_word(state, index), pointer, "");
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
  LLVMValueRef unhandled = LLVMBuildGlobalStringPtr(builder, "byte %" PRIu64 ": unhandled CoD exception %" PRIu64 "\n", "");
  LLVMValueRef addition = LLVMBuildGlobalStringPtr(builder, "byte %" PRIu64 ": u64 addition overflow\n", "");
  LLVMValueRef multiplication = LLVMBuildGlobalStringPtr(builder, "byte %" PRIu64 ": u64 multiplication overflow\n", "");
  LLVMValueRef is_add = LLVMBuildICmp(builder, LLVMIntEQ, category, cod_llvm_word(state, UINT64_MAX), "");
  LLVMValueRef is_mul = LLVMBuildICmp(builder, LLVMIntEQ, category, cod_llvm_word(state, UINT64_MAX - 1), "");
  LLVMValueRef other = LLVMBuildSelect(builder, is_mul, multiplication, unhandled, "");
  LLVMValueRef format = LLVMBuildSelect(builder, is_add, addition, other, "");
  LLVMValueRef args[] = {LLVMConstInt(i32, 2, 0), format, cod_llvm_read_word(state, site_slot), category};
  LLVMBuildCall2(builder, printer, dprintf_function, args, 4, "");
  LLVMBuildRet(builder, LLVMConstInt(i32, 70, 0));
}

static inline bool cod_llvm_invoke_words(cod_llvm_construction *state, cod_llvm_command command) {
  if (command.a >= state->label_count || !state->functions[command.a] ||
      command.b >= state->slot_count || command.c >= state->slot_count || command.d >= state->slot_count)
    return false;
  LLVMValueRef function = state->functions[command.a];
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
    if (a >= state->label_count || !state->functions[a]) return false;
    state->current = state->functions[a];
    if (!cod_llvm_local_block(state, b) || LLVMGetEntryBasicBlock(state->current) != state->blocks[b]) return false;
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
    case COD_LLVM_WORD:
      if (a >= slots) return false;
      cod_llvm_write_word(state, a, cod_llvm_word(state, b));
      return true;
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
    case COD_LLVM_MULTIPLY: {
      if (a >= slots || b >= slots || c >= slots || d >= slots) return false;
      const char *name = command.opcode == COD_LLVM_ADD ? "llvm.uadd.with.overflow" : "llvm.umul.with.overflow";
      unsigned intrinsic = LLVMLookupIntrinsicID(name, strlen(name));
      if (!intrinsic) return false;
      LLVMValueRef left = cod_llvm_read_word(state, b);
      LLVMValueRef right = cod_llvm_read_word(state, c);
      LLVMValueRef outcome = cod_llvm_build_checked_integer(builder, state->session->module, intrinsic, left, right);
      cod_llvm_write_word(state, a, LLVMBuildExtractValue(builder, outcome, 0, ""));
      LLVMValueRef overflow = LLVMBuildExtractValue(builder, outcome, 1, "");
      cod_llvm_write_word(state, d, LLVMBuildZExt(builder, overflow, state->word, ""));
      return true;
    }
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

/* Consume the private checked command stream. Own the LLVM session outside
   this call and discard it on failure. Object emission verifies the IR. */
static inline bool cod_llvm_construct_words(cod_llvm_session *session, const cod_llvm_command *commands,
    size_t count, size_t labels, size_t slots) {
  if (!session || !session->builder || !session->context || !session->module ||
      !commands || !count || !labels || !slots) return false;
  unsigned arity = 1;
  for (size_t i = 0; i < count; ++i) {
    if (commands[i].opcode != COD_LLVM_DECLARE) continue;
    if (commands[i].b > UINT_MAX || commands[i].b > count) return false;
    if (commands[i].b > arity) arity = (unsigned)commands[i].b;
  }
  cod_llvm_construction state = {
    .session = session, .word = LLVMInt64TypeInContext(session->context),
    .label_count = labels, .slot_count = slots, .argument_capacity = arity
  };
  LLVMTypeRef fields[] = {state.word, state.word, state.word};
  state.result = LLVMStructTypeInContext(session->context, fields, 3, 0);
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
    cod_llvm_command command = commands[i];
    if (command.opcode != COD_LLVM_DECLARE) continue;
    if (command.a >= labels || state.functions[command.a]) goto done;
    bool entry = command.c != 0;
    if (entry && (entry_declared || command.b != 0)) goto done;
    LLVMTypeRef result = entry ? LLVMInt32TypeInContext(session->context) : state.result;
    LLVMTypeRef signature = LLVMFunctionType(result, parameters, (unsigned)command.b, 0);
    char name[32];
    snprintf(name, sizeof(name), "cod.proc.%" PRIu64, command.a);
    LLVMValueRef function = LLVMAddFunction(session->module, entry ? "main" : name, signature);
    if (!entry) LLVMSetLinkage(function, LLVMInternalLinkage);
    state.functions[command.a] = function;
    entry_declared = entry_declared || entry;
  }
  if (!entry_declared) goto done;

  for (size_t i = 0; i < count; ++i) {
    cod_llvm_command command = commands[i];
    if (command.opcode != COD_LLVM_POSITION) continue;
    if (command.a >= labels || command.b >= labels || !state.functions[command.b] || state.blocks[command.a])
      goto done;
    state.blocks[command.a] = LLVMAppendBasicBlockInContext(session->context, state.functions[command.b], "");
  }
  for (size_t i = 0; i < count; ++i)
    if (!cod_llvm_construct_command(&state, commands[i])) goto done;
  success = state.argument_count == 0;

done:
  free(parameters);
  free(state.arguments);
  free(state.slots);
  free(state.blocks);
  free(state.functions);
  return success;
}

#endif
