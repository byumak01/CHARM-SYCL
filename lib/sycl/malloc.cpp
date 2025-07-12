#include <charm/sycl.hpp>
#include <iostream>
#include "iris/iris_interface.hpp"
CHARM_SYCL_BEGIN_NAMESPACE

namespace runtime {

void *new_malloc(size_t count, size_t type_size){

    auto result = iris_interface_20000::init();

    if (!result) {
        std::cout << "IRIS init failed!";
        return nullptr;
    }
    
    iris_interface_20000::mem_t mem;
    iris_interface_20000::iris_mem_create(type_size * count, &mem);
    std::cout << "I am called";
    return &mem;
}

}

CHARM_SYCL_END_NAMESPACE