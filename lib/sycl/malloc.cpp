#include <charm/sycl.hpp>
#include <iostream>
#include "iris/iris_interface.hpp"

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

using IRIS = iris_interface_20000;

void *new_malloc(size_t numBytes){

    auto result = IRIS::init();
    std::cout << "After init" << std::endl;
   
    auto* mem = new IRIS::mem_t;
    std::cout << "After memory object" << std::endl;
    
    auto create_result = IRIS::iris_mem_create(numBytes, mem);
    if (create_result != IRIS::SUCCESS) {
        std::cout << "Memory creation failed!" << std::endl;
        delete mem;
        return nullptr;
    }

    IRIS::usm_iris_mem_map[static_cast<void*>(mem)] = IRIS::rw;
    std::cout << "created mem: " << static_cast<void*>(mem) << std::endl; 
    return mem;
}

void memcpy(void*& dest, void*& src, size_t numBytes){
    auto init_iris = IRIS::init();
    auto* task = new IRIS::task_t;
    
    bool is_d2h = true;
    
    if(auto it = IRIS::usm_iris_mem_map.find(src); it != IRIS::usm_iris_mem_map.end()){
        std::cout << "src: " << src << " d2h" << std::endl;
        is_d2h = true;       
    } else if (auto it = IRIS::usm_iris_mem_map.find(dest); it != IRIS::usm_iris_mem_map.end()){
        std::cout << "dest: " << dest << " h2d" << std::endl;
        is_d2h = false;
    } else {
        std::runtime_error("Neither src nor dest is a device target.");
    }

    auto mem_task = IRIS::iris_task_create(task);

    if(is_d2h){
        std::cout << "device to host" << std::endl;
        auto d2h = IRIS::iris_task_d2h(*task, *(static_cast<IRIS::mem_t*>(src)), 0, numBytes, dest);
        std::cout << "d2h: " << d2h << std::endl;
    }
    else {
        std::cout << "iris success: " << IRIS::SUCCESS << std::endl;
        std::cout << "host to device" << std::endl;
        auto h2d = IRIS::iris_task_h2d(*task, *(static_cast<IRIS::mem_t*>(dest)), 0, numBytes, src);
        std::cout << "h2d: " << h2d << std::endl;
    }

    auto submit = IRIS::iris_task_submit(*task, IRIS::gpu, nullptr, 0);
}
}
CHARM_SYCL_END_NAMESPACE
