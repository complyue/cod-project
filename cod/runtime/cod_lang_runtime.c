#include "cod_lang_runtime.h"

#include "../../include/cod_region_storage.h"
#include "../lang_intrinsic_catalog.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum cod_exception_category {
#define COD_RUNTIME_CATEGORY(key, spelling, id, checked) COD_EXCEPTION_##key = id,
    COD_LANG_INTRINSIC_EXCEPTIONS(COD_RUNTIME_CATEGORY)
#undef COD_RUNTIME_CATEGORY
};

static const char *const cod_exception_names[] = {
#define COD_RUNTIME_NAME(key, spelling, id, checked) #spelling,
    COD_LANG_INTRINSIC_EXCEPTIONS(COD_RUNTIME_NAME)
#undef COD_RUNTIME_NAME
};

struct cod_cause {
    uint64_t category, site, requested, available, cause;
};

struct cod_cause_history {
    struct cod_cause *records;
    size_t size, capacity;
};

/* One-based cause handles remain valid across backing allocation and relocation. */
#ifdef __cplusplus
static thread_local struct cod_cause_history cod_causes;
#else
static _Thread_local struct cod_cause_history cod_causes;
#endif

static void print_site(uint64_t site) {
    const char *context = cod_lang_exception_site(site);
    if (context)
        fprintf(stderr, "%s: ", context);
}

void cod_lang_exception_finish(void)
#ifdef __cplusplus
    noexcept
#endif
{
    free(cod_causes.records);
    cod_causes.records = NULL;
    cod_causes.size = 0;
    cod_causes.capacity = 0;
}

void cod_lang_fail(const char *message) {
    fprintf(stderr, "%s\n", message);
    cod_lang_exception_finish();
    exit(70);
}

static void *allocate(uint64_t bytes, uint64_t *status) {
    void *backing = bytes <= SIZE_MAX ? malloc((size_t)bytes) : NULL;
    if (!backing)
        status[0] = COD_EXCEPTION_out_of_memory;
    return backing;
}

static uint64_t callable_bytes(const void *image) {
    uint64_t bytes;
    memcpy(&bytes, image, sizeof(bytes));
    return bytes;
}

void *cod_lang_callable_create(uint64_t fixed, uint64_t count, void *const *children,
                               const uint64_t *offsets, uint64_t *status) {
    if (fixed > SIZE_MAX) {
        status[0] = COD_EXCEPTION_out_of_memory;
        return NULL;
    }
    uint64_t bytes = fixed;
    for (uint64_t i = 0; i < count; ++i) {
        if (!children[i])
            continue;
        uint64_t child_bytes = callable_bytes(children[i]);
        if (child_bytes > SIZE_MAX - bytes) {
            status[0] = COD_EXCEPTION_out_of_memory;
            return NULL;
        }
        bytes += child_bytes;
    }
    unsigned char *image = (unsigned char *)allocate(bytes, status);
    if (!image)
        return NULL;
    memcpy(image, &bytes, sizeof(bytes));

    uint64_t position = fixed;
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t child_position = 0;
        if (children[i]) {
            uint64_t child_bytes = callable_bytes(children[i]);
            child_position = position;
            memcpy(image + position, children[i], (size_t)child_bytes);
            position += child_bytes;
        }
        memcpy(image + offsets[i], &child_position, sizeof(child_position));
    }
    return image;
}

void *cod_lang_callable_copy(const void *source, uint64_t *status) {
    uint64_t bytes = callable_bytes(source);
    void *image = allocate(bytes, status);
    if (image)
        memcpy(image, source, (size_t)bytes);
    return image;
}

void cod_lang_callable_free(void *image) {
    free(image);
}

void *cod_lang_defer_allocate(uint64_t bytes, uint64_t *status) {
    return allocate(bytes, status);
}

void cod_lang_defer_free(void *record) {
    free(record);
}

struct cod_exception_owner {
    cod_region_storage storage;
    cod_region_extent root;
    struct cod_exception_owner *release_next;
    int aggregate;
};

struct cod_retained_exception {
    uint64_t category, site, requested, available, cause;
    struct cod_exception_owner *owner;
};

struct cod_deferred_aggregate {
    size_t retained;
    struct cod_retained_exception body;
};

void *cod_lang_acquire_exception_region(uint64_t capacity, uint64_t root_size, uint64_t root_alignment) {
    if (capacity > PTRDIFF_MAX || root_size > capacity || root_alignment > SIZE_MAX)
        return NULL;
    size_t used = 0;
    cod_region_extent root;
    if (cod_region_layout_reserve(&used, (size_t)capacity, (size_t)root_size, 1,
                                  (size_t)root_alignment, &root) != COD_REGION_STORAGE_OK)
        return NULL;
    struct cod_exception_owner *owner = (struct cod_exception_owner *)malloc(sizeof(*owner));
    if (!owner)
        return NULL;
    owner->storage.bytes = NULL;
    if (cod_region_storage_open(&owner->storage, (size_t)capacity) != COD_REGION_STORAGE_OK) {
        free(owner);
        return NULL;
    }
    owner->storage.used = used;
    owner->root = root;
    owner->aggregate = 0;
    return owner;
}

void *cod_lang_exception_payload(void *handle) {
    struct cod_exception_owner *owner = (struct cod_exception_owner *)handle;
    return cod_region_storage_address(&owner->storage, owner->root);
}

static struct cod_retained_exception *deferred_members(struct cod_deferred_aggregate *aggregate) {
    return (struct cod_retained_exception *)(aggregate + 1);
}

void *cod_lang_acquire_deferred_aggregate(uint64_t members, uint64_t *failure_category) {
    const size_t header = sizeof(struct cod_deferred_aggregate);
    if (members > ((size_t)PTRDIFF_MAX - header) / sizeof(struct cod_retained_exception)) {
        *failure_category = COD_EXCEPTION_invalid_capacity;
        return NULL;
    }
    const size_t bytes = header + (size_t)members * sizeof(struct cod_retained_exception);
    const size_t capacity = bytes > COD_LANG_EXCEPTION_REGION_DEFAULT ? bytes : COD_LANG_EXCEPTION_REGION_DEFAULT;
#ifdef __cplusplus
    const size_t alignment = alignof(struct cod_deferred_aggregate);
#else
    const size_t alignment = _Alignof(struct cod_deferred_aggregate);
#endif
    struct cod_exception_owner *owner = (struct cod_exception_owner *)cod_lang_acquire_exception_region(
        capacity, bytes, alignment);
    if (!owner) {
        *failure_category = COD_EXCEPTION_out_of_memory;
        return NULL;
    }
    owner->aggregate = 1;
    struct cod_deferred_aggregate *aggregate = (struct cod_deferred_aggregate *)cod_lang_exception_payload(owner);
    aggregate->retained = 0;
    aggregate->body.owner = NULL;
    aggregate->body.category = 0;
    *failure_category = 0;
    return owner;
}

void cod_lang_retain_deferred_failure(void *handle, uint64_t position, uint64_t category, uint64_t site,
                                      uint64_t requested, uint64_t available, uint64_t cause, void *owner) {
    // The emitter reserves the exact member array, writes it in order and
    // hands off each original region once. Position zero is the body field.
    struct cod_deferred_aggregate *aggregate = (struct cod_deferred_aggregate *)cod_lang_exception_payload(handle);
    struct cod_retained_exception *entry = position ? deferred_members(aggregate) + aggregate->retained++ : &aggregate->body;
    struct cod_retained_exception value = {category, site, requested, available, cause, (struct cod_exception_owner *)owner};
    *entry = value;
}

static void schedule_exception_release(struct cod_exception_owner **pending, struct cod_exception_owner *owner) {
    if (owner) {
        owner->release_next = *pending;
        *pending = owner;
    }
}

void cod_lang_release_exception_region(void *handle) {
    struct cod_exception_owner *pending = NULL;
    schedule_exception_release(&pending, (struct cod_exception_owner *)handle);
    while (pending) {
        struct cod_exception_owner *owner = pending;
        pending = owner->release_next;
        if (owner->aggregate) {
            struct cod_deferred_aggregate *aggregate = (struct cod_deferred_aggregate *)cod_lang_exception_payload(owner);
            for (size_t i = 0; i < aggregate->retained; ++i)
                schedule_exception_release(&pending, deferred_members(aggregate)[i].owner);
            schedule_exception_release(&pending, aggregate->body.owner);
        }
        cod_region_storage_release(&owner->storage);
        free(owner);
    }
}

struct cod_text_view {
    const uint8_t *bytes;
    uint64_t size, extent;
};

struct cod_lexical_allocation {
    struct cod_lexical_allocation *previous;
    void (*release)(void *);
};

struct cod_lexical_extent {
    struct cod_lexical_allocation allocation;
    cod_region_storage backing;
};

struct cod_lexical_text {
    struct cod_lexical_allocation allocation;
    struct cod_text_view text;
    uint8_t bytes[];
};

struct cod_lexical_builder {
    struct cod_lexical_allocation allocation;
    uint8_t *bytes;
    size_t size, capacity;
};

static void publish_lexical_allocation(void **allocations, struct cod_lexical_allocation *allocation,
                                       void (*release)(void *)) {
    allocation->previous = *allocations;
    allocation->release = release;
    *allocations = allocation;
}

static void release_lexical_extent(void *allocation) {
    struct cod_lexical_extent *extent = allocation;
    cod_region_storage_release(&extent->backing);
    free(extent);
}

void *cod_lang_lexical_extent_allocate(void **allocations, uint64_t bytes, uint64_t alignment) {
    if (!allocations || bytes > SIZE_MAX || bytes > PTRDIFF_MAX || !alignment ||
        alignment > _Alignof(max_align_t) || (alignment & (alignment - 1)))
        return NULL;

    struct cod_lexical_extent *extent = malloc(sizeof(*extent));
    if (!extent) return NULL;
    extent->backing = (cod_region_storage){0};
    if (cod_region_storage_open_layout(&extent->backing, (size_t)bytes) != COD_REGION_STORAGE_OK) {
        free(extent);
        return NULL;
    }
    publish_lexical_allocation(allocations, &extent->allocation, release_lexical_extent);
    return extent->backing.bytes;
}

void *cod_lang_lexical_text_create(void **allocations, const void *input) {
    const struct cod_text_view *source = input;
    const uint64_t size = source->size;
    const uint64_t backing = size ? size : 1;
    if (backing > SIZE_MAX - sizeof(struct cod_lexical_text)) return NULL;
    struct cod_lexical_text *object = malloc(sizeof(*object) + (size_t)backing);
    if (!object) return NULL;
    if (size)
        memcpy(object->bytes, source->bytes, (size_t)size);
    object->text = (struct cod_text_view){object->bytes, size, size};
    publish_lexical_allocation(allocations, &object->allocation, free);
    return &object->text;
}

static int utf8_boundary(const struct cod_text_view *text, uint64_t at) {
    return at == text->size || (text->bytes[at] & 0xc0) != 0x80;
}

int cod_lang_lexical_text_slice(void **allocations, const void *input, uint64_t first, uint64_t past,
                               void **result) {
    const struct cod_text_view *source = input;
    if (first > past || past > source->size || !utf8_boundary(source, first) || !utf8_boundary(source, past))
        return COD_TEXT_SLICE_INVALID;
    const uint64_t size = past - first;
    const struct cod_text_view slice = {size ? source->bytes + first : NULL, size, size};
    void *text = cod_lang_lexical_text_create(allocations, &slice);
    if (!text) return COD_TEXT_SLICE_OUT_OF_MEMORY;
    *result = text;
    return COD_TEXT_SLICE_READY;
}

static void release_builder(void *object) {
    struct cod_lexical_builder *builder = object;
    free(builder->bytes);
    free(builder);
}

static int append_builder_bytes(struct cod_lexical_builder *builder, const uint8_t *bytes, uint64_t size, int self) {
    if (size == 0)
        return 1;
    if (size > SIZE_MAX - builder->size)
        return 0;
    const size_t end = builder->size + (size_t)size;
    if (end > builder->capacity) {
        size_t capacity = builder->capacity ? builder->capacity : 16;
        if (capacity < end)
            capacity = capacity <= SIZE_MAX / 2 ? capacity * 2 : end;
        if (capacity < end)
            capacity = end;
        uint8_t *grown = realloc(builder->bytes, capacity);
        if (!grown)
            return 0;
        builder->bytes = grown;
        builder->capacity = capacity;
    }
    memcpy(builder->bytes + builder->size, self ? builder->bytes : bytes, (size_t)size);
    builder->size = end;
    return 1;
}

int cod_lang_builder_append_text(void *target, const void *input) {
    const struct cod_text_view *source = input;
    return append_builder_bytes(target, source->bytes, source->size, 0);
}

int cod_lang_builder_append_builder(void *target, const void *input) {
    const struct cod_lexical_builder *source = input;
    return append_builder_bytes(target, source->bytes, source->size, target == input);
}

void *cod_lang_lexical_builder_create(void **allocations, const void *left, const void *right, uint32_t builders) {
    struct cod_lexical_builder *builder = calloc(1, sizeof(*builder));
    if (!builder)
        return NULL;
    const void *pieces[] = {left, right};
    for (unsigned i = 0; i != 2; ++i) {
        if (!pieces[i])
            continue;
        const int appended = builders & (1u << i) ? cod_lang_builder_append_builder(builder, pieces[i])
                                                 : cod_lang_builder_append_text(builder, pieces[i]);
        if (!appended) {
            release_builder(builder);
            return NULL;
        }
    }
    publish_lexical_allocation(allocations, &builder->allocation, release_builder);
    return builder;
}

void *cod_lang_lexical_builder_finish(void **allocations, const void *input) {
    const struct cod_lexical_builder *builder = input;
    const struct cod_text_view view = {builder->bytes, builder->size, builder->size};
    return cod_lang_lexical_text_create(allocations, &view);
}

void cod_lang_lexical_allocations_release(void *allocations) {
    struct cod_lexical_allocation *object = allocations;
    while (object) {
        struct cod_lexical_allocation *previous = object->previous;
        object->release(object);
        object = previous;
    }
}

static int grow_cause_history(void) {
    const size_t limit = SIZE_MAX / sizeof(*cod_causes.records);
    if (cod_causes.capacity == limit)
        return 0;
    size_t capacity = cod_causes.capacity ? cod_causes.capacity : 1;
    if (cod_causes.capacity)
        capacity = capacity <= limit / 2 ? capacity * 2 : limit;
    struct cod_cause *records = (struct cod_cause *)realloc(cod_causes.records, capacity * sizeof(*records));
    if (!records)
        return 0;
    cod_causes.records = records;
    cod_causes.capacity = capacity;
    return 1;
}

uint64_t cod_lang_exception_chain(uint64_t category, uint64_t site, uint64_t requested,
                                  uint64_t available, uint64_t cause, uint64_t *status) {
    if (cod_causes.size == cod_causes.capacity && !grow_cause_history()) {
        status[0] = COD_EXCEPTION_out_of_memory;
        return 0;
    }
    const struct cod_cause value = {category, site, requested, available, cause};
    cod_causes.records[cod_causes.size++] = value;
    return cod_causes.size;
}

static const struct cod_cause *read_cause(uint64_t cause) {
    /* Keep physical history checks active independently of source bounds policy. */
    if (!cause || cause > cod_causes.size)
        cod_lang_fail("invalid execution-local exception cause");
    return &cod_causes.records[cause - 1];
}

uint64_t cod_lang_exception_count(uint64_t cause) {
    uint64_t count = 0;
    while (cause) {
        cause = read_cause(cause)->cause;
        ++count;
    }
    return count;
}

uint64_t cod_lang_exception_cause(uint64_t cause, uint64_t index, uint64_t *result, uint64_t check_bounds) {
    const uint64_t requested = index;
    while (index && (cause || !check_bounds)) {
        cause = read_cause(cause)->cause;
        --index;
    }
    if (check_bounds && !cause) {
        result[0] = requested - index;
        return 0;
    }
    const struct cod_cause *value = read_cause(cause);
    result[0] = value->site;
    result[1] = value->requested;
    result[2] = value->available;
    result[3] = value->cause;
    result[4] = value->category;
    return 1;
}

static void print_exception(const char *prefix, const struct cod_cause *value) {
    if (!value->category || value->category >= sizeof(cod_exception_names) / sizeof(*cod_exception_names))
        cod_lang_fail("invalid intrinsic exception category");
    print_site(value->site);
    fprintf(stderr, "%s %s (site=%" PRIu64 ", requested=%" PRIu64 ", available=%" PRIu64 ")\n",
            prefix, cod_exception_names[value->category], value->site, value->requested, value->available);
}

void cod_lang_uncaught(uint64_t category, uint64_t site, uint64_t requested, uint64_t available, uint64_t cause) {
    const struct cod_cause head = {category, site, requested, available, cause};
    print_exception("uncaught", &head);
    while (cause) {
        const struct cod_cause *value = read_cause(cause);
        print_exception("caused by", value);
        cause = value->cause;
    }
    cod_lang_exception_finish();
    exit(70);
}

void cod_lang_uncaught_nominal(const char *name, uint64_t site) {
    print_site(site);
    fprintf(stderr, "uncaught %s\n", name);
    cod_lang_exception_finish();
    exit(70);
}

void cod_lang_print_i64(int64_t value) {
    printf("%" PRId64 "\n", value);
}

void cod_lang_print_u64(uint64_t value) {
    printf("%" PRIu64 "\n", value);
}

void cod_lang_print_f32(float value) {
    printf("%.9g\n", (double)value);
}

void cod_lang_print_f64(double value) {
    printf("%.17g\n", value);
}

void cod_lang_print_bool(uint32_t value) {
    puts(value ? "true" : "false");
}
