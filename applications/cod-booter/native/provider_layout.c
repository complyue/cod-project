#include "cod_providers.h"
#include "lang_llvm_object_c.h"
#include "provider_output.h"

#include <stdlib.h>

static bool matches_host_abi(const cod_llvm_abi_layout *layout) {
    const size_t sizes[] = {sizeof(uint8_t), sizeof(uint32_t), sizeof(uint64_t), sizeof(float), sizeof(double)};
    const size_t alignments[] = {_Alignof(uint8_t), _Alignof(uint32_t), _Alignof(uint64_t), _Alignof(float), _Alignof(double)};
    struct record_witness { uint8_t byte; };
    if (layout->struct_alignment != _Alignof(struct record_witness)) return false;
    for (size_t i = 0; i < COD_LLVM_ABI_REPR_COUNT; ++i)
        if (layout->size[i] != sizes[i] || layout->alignment[i] != alignments[i]) return false;
    return true;
}

cod_native_status cod_native_ProviderLayout_construct_impl(uint64_t *resource, cod_native_ProviderLayout_failure *failure) {
    cod_llvm_abi_layout measured;
    if (!cod_llvm_measure_native_abi(&measured) || !matches_host_abi(&measured)) {
        failure->status = provider_output_invalid;
        return COD_NATIVE_FAILURE;
    }
    cod_llvm_abi_layout *layout = malloc(sizeof(*layout));
    if (!layout) {
        failure->status = provider_output_memory;
        return COD_NATIVE_FAILURE;
    }
    *layout = measured;
    *resource = (uint64_t)(uintptr_t)layout;
    return COD_NATIVE_OK;
}

void cod_native_ProviderLayout_destroy_impl(uint64_t resource) {
    free((void *)(uintptr_t)resource);
}

static const cod_llvm_abi_layout *provider_layout(cod_native_ProviderLayout_borrow handle) {
    return handle.payload == handle.owner ? (const cod_llvm_abi_layout *)(uintptr_t)handle.payload : NULL;
}

cod_native_status cod_native_provider_alloc_size_impl(cod_native_ProviderLayout_borrow handle,
    uint8_t representation, uint64_t *value) {
    const cod_llvm_abi_layout *layout = provider_layout(handle);
    if (!layout || representation >= COD_LLVM_ABI_REPR_COUNT) return COD_NATIVE_BOUNDARY_ERROR;
    *value = layout->size[representation];
    return COD_NATIVE_OK;
}

cod_native_status cod_native_provider_abi_alignment_impl(cod_native_ProviderLayout_borrow handle,
    uint8_t representation, uint64_t *value) {
    const cod_llvm_abi_layout *layout = provider_layout(handle);
    if (!layout || representation >= COD_LLVM_ABI_REPR_COUNT) return COD_NATIVE_BOUNDARY_ERROR;
    *value = layout->alignment[representation];
    return COD_NATIVE_OK;
}

cod_native_status cod_native_provider_struct_alignment_impl(cod_native_ProviderLayout_borrow handle, uint64_t *value) {
    const cod_llvm_abi_layout *layout = provider_layout(handle);
    if (!layout) return COD_NATIVE_BOUNDARY_ERROR;
    *value = layout->struct_alignment;
    return COD_NATIVE_OK;
}
