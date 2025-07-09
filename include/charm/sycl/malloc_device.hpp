#pragma once
#include <charm/sycl.hpp>

// bymk: malloc_device functions are only implemented to make 
// benchmarks with these functions work. These functions are
// only implemented for IRIS RTS, it will not work with other
// RTS systems. Also they are obviously not exactly SYCL 
// Specification compliant.

CHARM_SYCL_BEGIN_NAMESPACE

void* malloc_device(size_t numBytes,
                          const device& syclDevice,
                          const context& syclContext,
                          const property_list& propList = {});

template <typename T>
T* malloc_device(size_t count,
                       const device& syclDevice,
                       const context& syclContext,
                       const property_list& propList = {});

void* malloc_device(size_t numBytes,
                          const queue& syclQueue,
                          const property_list& propList = {});

template <typename T>
T* malloc_device(size_t count, const  queue& syclQueue,
                       const property_list& propList = {});


// bymk:
// aligned_alloc_* functions does not have a counterpart in
// IRIS rts. They will just call iris_mem_create in background.
// They are implemented so that benchmarks will compile and
// run without any errors.
void* aligned_alloc_device(size_t alignment, size_t numBytes,
                                 const device& syclDevice,
                                 const context& syclContext,
                                 const property_list& propList = {});

template <typename T>
T* aligned_alloc_device(size_t alignment, size_t count,
                              const device& syclDevice,
                              const context& syclContext,
                              const property_list& propList = {});

void* aligned_alloc_device(size_t alignment, size_t numBytes,
                                 const queue& syclQueue,
                                 const property_list& propList = {});

template <typename T>
T* aligned_alloc_device(size_t alignment, size_t count,
                              const queue& syclQueue,
                              const property_list& propList = {});


CHARM_SYCL_END_NAMESPACE