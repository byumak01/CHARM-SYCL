#include <charm/sycl.hpp>
#include "iris/iris_interface.hpp"

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

using IRIS = iris_interface_20000;

void *new_malloc(size_t numBytes){

    auto result = IRIS::init();
   
    auto* mem = new IRIS::mem_t;
    
    if (IRIS::iris_mem_create(numBytes, mem) != IRIS::SUCCESS){
        delete mem;
        throw std::runtime_error("iris_mem_create inside usm.cpp failed.");
    }

    IRIS::usm_iris_mem_map[static_cast<void*>(mem)] = IRIS::rw;
    return mem;
}

void memcpy(void*& dest, void*& src, size_t numBytes){
    IRIS::init();

    auto* task = new IRIS::task_t;
    
    bool is_d2h = true;
    
    if(auto it = IRIS::usm_iris_mem_map.find(src); it != IRIS::usm_iris_mem_map.end()){
        is_d2h = true;       
    } else if (auto it = IRIS::usm_iris_mem_map.find(dest); it != IRIS::usm_iris_mem_map.end()){
        is_d2h = false;
    } else {
        throw std::runtime_error("Neither src nor dest is a device target.");
    }

    if (IRIS::iris_task_create(task) != IRIS::SUCCESS){
        throw std::runtime_error("iris_task_create inside usm.cpp failed.");
    }

    if(is_d2h){
        if (IRIS::iris_task_d2h(*task, *(static_cast<IRIS::mem_t*>(src)), 0, numBytes, dest) != IRIS::SUCCESS){
            throw std::runtime_error("iris_task_d2h inside usm.cpp failed");
        }
    }
    else {
        if (IRIS::iris_task_h2d(*task, *(static_cast<IRIS::mem_t*>(dest)), 0, numBytes, src) != IRIS::SUCCESS){
            throw std::runtime_error("iris_task_h2d inside usm.cpp failed");
        }
    }

    if (IRIS::iris_task_submit(*task, IRIS::gpu, nullptr, 0) != IRIS::SUCCESS){
        throw std::runtime_error("iris_task_submit inside usm.cpp failed");
    }

    if (IRIS::iris_synchronize() != IRIS::SUCCESS) {
        throw std::runtime_error("iris_synchronize inside usm.cpp failed");
    }
}
}
CHARM_SYCL_END_NAMESPACE
