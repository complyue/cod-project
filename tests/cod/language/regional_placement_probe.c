#include "cod_lang_runtime.h"
#include "cod_region_storage.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void *cod_test_region_acquire(uint64_t capacity);
uint64_t cod_test_region_reserve(void *owner, uint64_t bytes, uint64_t alignment);
void cod_test_region_release(void *owner);

/* The emitted Link layout for these fixtures is three u64 words. */
enum { link_tag = 0, link_words = 3 };
static void *graph;
static uint64_t node_offsets[2];
static size_t nodes;
static bool acquired;

void *cod_lang_region_acquire(uint64_t capacity) {
    assert(!acquired);
    assert(capacity == (RELOCATE_GRAPH ? 4096 : 64));
    graph = cod_test_region_acquire(capacity);
    assert(graph);
    acquired = true;
    return graph;
}

uint64_t cod_lang_region_reserve(void *owner, uint64_t bytes, uint64_t alignment) {
    assert(graph && owner == graph);
    const uint64_t offset = cod_test_region_reserve(owner, bytes, alignment);
    assert(offset != UINT64_MAX);
    if (bytes == link_words * sizeof(uint64_t)) {
        assert(nodes < sizeof(node_offsets) / sizeof(*node_offsets));
        node_offsets[nodes++] = offset;
    }
    return offset;
}

static void check_link(cod_region_storage *copy, uint64_t offset, uint64_t key, uint64_t next) {
    uint64_t words[link_words];
    assert(offset <= SIZE_MAX);
    const void *field = cod_region_storage_address(copy, (cod_region_extent){(size_t)offset, sizeof(words)});
    assert(field);
    memcpy(words, field, sizeof(words));
    assert(words[0] == link_tag && words[1] == key && words[2] == next);
}

void cod_lang_region_release(void *owner) {
    if (!owner) return;
    assert(graph && owner == graph);

    if (RELOCATE_GRAPH) {
        assert(nodes == 2 && node_offsets[0] != node_offsets[1]);
        const cod_region_storage *original = owner;
        cod_region_storage copy = {0};
        assert(cod_region_storage_open_layout(&copy, original->used) == COD_REGION_STORAGE_OK);
        assert(copy.bytes != original->bytes);
        memcpy(copy.bytes, original->bytes, original->used);
        cod_test_region_release(owner);
        graph = NULL;
        check_link(&copy, node_offsets[0], 1, node_offsets[1]);
        check_link(&copy, node_offsets[1], 2, node_offsets[0]);
        cod_region_storage_release(&copy);
        puts("relocated Link cycle");
    } else {
        assert(nodes == 1);
        cod_test_region_release(owner);
        graph = NULL;
        puts("released Graph");
    }
}
