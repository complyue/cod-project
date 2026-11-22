/* Private link interface for emitted CoD programs and their runtime. */
#ifndef COD_LANG_RUNTIME_H
#define COD_LANG_RUNTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#define COD_RUNTIME_NORETURN [[noreturn]]
#define COD_RUNTIME_NOEXCEPT noexcept
#else
#define COD_RUNTIME_NORETURN _Noreturn
#define COD_RUNTIME_NOEXCEPT
#endif

/* Supplied by the emitted program. The returned spelling is program-owned. */
const char *cod_lang_exception_site(uint64_t site);

COD_RUNTIME_NORETURN void cod_lang_fail(const char *message);
void *cod_lang_callable_create(uint64_t fixed, uint64_t count, void *const *children,
                               const uint64_t *offsets, uint64_t *status);
void *cod_lang_callable_copy(const void *source, uint64_t *status);
void cod_lang_callable_free(void *image);
void *cod_lang_defer_allocate(uint64_t bytes, uint64_t *status);
void cod_lang_defer_free(void *record);
void *cod_lang_acquire_exception_region(uint64_t capacity, uint64_t root_size, uint64_t root_alignment);
void *cod_lang_exception_payload(void *owner);
void cod_lang_release_exception_region(void *owner);
void *cod_lang_lexical_text_create(void **allocations, const void *input);
enum cod_text_slice_status {
    COD_TEXT_SLICE_READY,
    COD_TEXT_SLICE_INVALID,
    COD_TEXT_SLICE_OUT_OF_MEMORY,
};
int cod_lang_lexical_text_slice(void **allocations, const void *input, uint64_t first, uint64_t past,
                               void **result);
void *cod_lang_lexical_builder_create(void **allocations, const void *left, const void *right, uint32_t builders);
int cod_lang_builder_append_text(void *target, const void *input);
int cod_lang_builder_append_builder(void *target, const void *input);
void *cod_lang_lexical_builder_finish(void **allocations, const void *input);
void cod_lang_lexical_allocations_release(void *allocations);

void cod_lang_exception_finish(void) COD_RUNTIME_NOEXCEPT;
uint64_t cod_lang_exception_chain(uint64_t category, uint64_t site, uint64_t requested,
                                  uint64_t available, uint64_t cause, uint64_t *status);
uint64_t cod_lang_exception_count(uint64_t cause);
uint64_t cod_lang_exception_cause(uint64_t cause, uint64_t index, uint64_t *result, uint64_t check_bounds);
COD_RUNTIME_NORETURN void cod_lang_uncaught(uint64_t category, uint64_t site, uint64_t requested,
                                             uint64_t available, uint64_t cause);
COD_RUNTIME_NORETURN void cod_lang_uncaught_nominal(const char *name, uint64_t site);

void cod_lang_print_i64(int64_t value);
void cod_lang_print_u64(uint64_t value);
void cod_lang_print_f32(float value);
void cod_lang_print_f64(double value);
void cod_lang_print_bool(uint32_t value);

#undef COD_RUNTIME_NOEXCEPT
#undef COD_RUNTIME_NORETURN
#ifdef __cplusplus
}
#endif

#endif
