#include <charm/sycl.hpp>
#include <iostream>
#include "iris/iris_interface.hpp"

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

using IRIS = iris_interface_20000;

void *new_malloc(size_t count, size_t type_size){

    auto result = IRIS::init();
    std::cout << "After init" << std::endl;
   
    auto* mem = new IRIS::mem_t;
    std::cout << "After memory object" << std::endl;
    
    auto create_result = IRIS::iris_mem_create(type_size * count, mem);
    if (create_result != IRIS::SUCCESS) {
        std::cout << "Memory creation failed!" << std::endl;
        delete mem;
        return nullptr;
    }
    
    std::cout << "Memory created successfully" << std::endl;
    return mem;
}

void memcpy(void* dest, void* src, size_t numBytes){

    auto result = IRIS::init();

    auto* task = new IRIS::task_t;

    auto mem = IRIS::iris_task_create(task);

    auto d2h = IRIS::iris_task_h2d(*task, *((IRIS::mem_t*)src), 0, numBytes, dest);

}

}
CHARM_SYCL_END_NAMESPACE