#pragma once
#include <charm/sycl.hpp>

// bymk: malloc_device functions are only implemented to make
// benchmarks with these functions work. These functions are
// only implemented for IRIS RTS, it will not work with other
// RTS systems. Also they are obviously not exactly SYCL
// Specification compliant. 
// [[maybe_unused]]'s are used because we only care about IRIS backend.

CHARM_SYCL_BEGIN_NAMESPACE

// TODO: runtimedaki new_malloc fonksiyonuna direkt
// numBytes verirsek tek bi new_malloc ile isi cozeriz

void* malloc_device(size_t numBytes, [[maybe_unused]] const device& syclDevice,
                    [[maybe_unused]] const context& syclContext,
                    [[maybe_unused]] const property_list& propList = {}) {
    auto* result = runtime::new_malloc(numBytes);
    return static_cast<void*>(result);
}

template <typename T>
T* malloc_device(size_t count, [[maybe_unused]] const device& syclDevice,
                 [[maybe_unused]] const context& syclContext,
                 [[maybe_unused]] const property_list& propList = {}) {
    auto* result = runtime::new_malloc(count * sizeof(T));
    return static_cast<T*>(result);
}

void* malloc_device(size_t numBytes, [[maybe_unused]] const queue& syclQueue,
                    [[maybe_unused]] const property_list& propList = {}) {
    auto* result = runtime::new_malloc(numBytes);
    return static_cast<void*>(result);
}

template <typename T>
T* malloc_device(size_t count, const queue& syclQueue) {
    auto* result = runtime::new_malloc(count * sizeof(T));
    return static_cast<T*>(result);
}

// bymk:
// aligned_alloc_* functions does not have a counterpart in
// IRIS rts. They will just call iris_mem_create in background.
// They are implemented so that benchmarks will compile and
// run without any errors.
void* aligned_alloc_device(size_t alignment, size_t numBytes,
                           [[maybe_unused]] const device& syclDevice,
                           [[maybe_unused]] const context& syclContext,
                           [[maybe_unused]] const property_list& propList = {});

template <typename T>
T* aligned_alloc_device(size_t alignment, size_t count,
                        [[maybe_unused]] const device& syclDevice,
                        [[maybe_unused]] const context& syclContext,
                        [[maybe_unused]] const property_list& propList = {});

void* aligned_alloc_device(size_t alignment, size_t numBytes,
                           [[maybe_unused]] const queue& syclQueue,
                           [[maybe_unused]] const property_list& propList = {});

template <typename T>
T* aligned_alloc_device(size_t alignment, size_t count, [[maybe_unused]] const queue& syclQueue,
                        [[maybe_unused]] const property_list& propList = {});

CHARM_SYCL_END_NAMESPACE