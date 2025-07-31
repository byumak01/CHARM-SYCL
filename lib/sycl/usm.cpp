#include <charm/sycl.hpp>
#include "rt.hpp"
#include "iris/iris_interface.hpp"

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

using IRIS = iris_interface_20000;

void *new_malloc(size_t numBytes){

    IRIS::init();
   
    auto* mem = new IRIS::mem_t;
    
    if (IRIS::iris_mem_create(numBytes, mem) != IRIS::SUCCESS){
        delete mem;
        throw std::runtime_error("iris_mem_create inside usm.cpp failed.");
    }

    IRIS::usm_iris_mem_map[static_cast<void*>(mem)] = IRIS::rw;
    return mem;
}

void memcpy(void*& dest, void*& src, size_t numBytes){

    auto task = impl::global_state::get_depmgr()->new_task();
    task->usm_memcpy(dest, src, numBytes);
    task->submit();
    IRIS::iris_synchronize();
}
}
CHARM_SYCL_END_NAMESPACE
