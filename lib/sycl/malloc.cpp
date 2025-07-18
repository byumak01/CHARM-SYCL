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
    
    std::cout << "Memory created successfully" << std::endl;
    return mem;
}

void memcpy(void* dest, void* src, size_t numBytes){

    // burada bir if check olmasi lazim, h2d copy mi
    // d2h copy mi oldugunun anlasilmasi icin, cunku d2h
    // copy islemi aslinda h2d de yapiliyor olacak iriste
    // d2h copy tarafinda zaten olan pointeri assign edicez

    auto init_iris = IRIS::init();

    auto* task = new IRIS::task_t;

    std::cout << "device pointer: " << dest << std::endl;
    std::cout << "host pointer: " << src << std::endl;

    // d2h olmasi gereken memcpy d2h void pointerini dondurecek
    // global bir map gibi bir sey olacak, oradan bakip pointeri dondurecek
    // ilk callda h2d ve d2h handle edilecek
    // ikinci callda verilen pointera mapten assign edilecek ptr
    // map void* to void* olacak buyuk ihtimalle
    // key olan void* device a verilen kisim olmasi lazim gibi
    // yada pair olarak vector olarak tutulabilir ama map biraz daha
    // cost effective olur gibi 
    // shared_ptr vs kullanmak pek makul gibi durmuyor memory31

    auto mem_task = IRIS::iris_task_create(task);

    auto h2d_obj = *((IRIS::mem_t*)dest);
    auto h2d = IRIS::iris_task_h2d(*task, h2d_obj, 0, numBytes, src);
    // src data structure a eklenecek?

    void* d2h_ptr;
    auto d2h_obj = *((IRIS::mem_t*)dest);
    auto d2h = IRIS::iris_task_d2h(*task, d2h_obj, 0, numBytes, d2h_ptr);
    // d2h_ptr data structure a eklenecek.
    // ikinci asamada bu d2h_ptr memcpy e verilen destinationa verilecek (d2h icin)
    
    auto submit = IRIS::iris_task_submit(*task, IRIS::cpu, nullptr, 0);

}

}
CHARM_SYCL_END_NAMESPACE