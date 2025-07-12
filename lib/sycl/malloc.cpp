#include <charm/sycl.hpp>
#include <iostream>
#include "iris/iris_interface.hpp"

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

void *new_malloc(size_t count, size_t type_size){

    auto result = iris_interface_20000::init();
    std::cout << "After init" << std::endl;
   
    auto* mem = new iris_interface_20000::mem_t;
    std::cout << "After memory object" << std::endl;
    
    auto create_result = iris_interface_20000::iris_mem_create(type_size * count, mem);
    if (create_result != iris_interface_20000::SUCCESS) {
        std::cout << "Memory creation failed!" << std::endl;
        delete mem;
        return nullptr;
    }
    
    std::cout << "Memory created successfully" << std::endl;
    return mem;
}

}
CHARM_SYCL_END_NAMESPACE