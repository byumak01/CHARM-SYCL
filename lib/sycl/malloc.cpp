#include <charm/sycl.hpp>
#include <iostream>
#include "iris/iris_interface.hpp"
#include <unordered_map>

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

using IRIS = iris_interface_20000;

// sycl::free will deallocate this map, it should work
std::unordered_map<void*, IRIS::mem_t*> h2d_ptr_map;
std::unordered_map<void*, IRIS::mem_t*> host_to_mem_ptr_map;
std::unordered_map<void*, void*> d2h_ptr_map;

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
    return mem;
    /*
    auto* dummy_ptr = new char;

    //h2d_ptr_map[dummy_ptr] = mem;
    
    std::cout << "Memory created successfully" << std::endl;
    return dummy_ptr;
    */
}

void memcpy(void*& dest, void*& src, size_t numBytes){
    // src = host memory
    // dest = device mem (iris mem)
    auto init_iris = IRIS::init();
    auto* task = new IRIS::task_t;

    auto mem_task = IRIS::iris_task_create(task);

    auto h2d = IRIS::iris_task_h2d(*task, *(static_cast<IRIS::mem_t*>(dest)), 0, numBytes, src);

    auto submit = IRIS::iris_task_submit(*task, IRIS::gpu, nullptr, 0);
}

/*
void memcpy(void*& dest, void*& src, size_t numBytes){
 
    // burada bir if check olmasi lazim, h2d copy mi
    // d2h copy mi oldugunun anlasilmasi icin, cunku d2h
    // copy islemi aslinda h2d de yapiliyor olacak iriste
    // d2h copy tarafinda zaten olan pointeri assign edicez

    // char *dest 
    // memory free edilmeli
    // daha sonra bu dest

    // h2d device destination
    auto it = h2d_ptr_map.find(dest);
    if( it != h2d_ptr_map.end()){

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
        // shared_ptr vs kullanmak pek makul gibi durmuyor memory
        
        auto mem_task = IRIS::iris_task_create(task);
        //IRIS::mem_t* h2d_mem = it->second;
        IRIS::mem_t* h2d_mem = new IRIS::mem_t;
        //h2d_ptr_map.erase(it);
        delete dest; // it->first == dest
        dest = std::malloc(numBytes);
        std::memcpy(dest, src, numBytes);
        auto h2d = IRIS::iris_task_h2d(*task, *h2d_mem, 0, numBytes, dest);
        // src data structure a eklenecek?
        
        void* d2h_ptr = std::malloc(numBytes);
        std::cout << "d2h_ptr before: "<< d2h_ptr << std::endl;
        d2h_ptr_map[dest] = d2h_ptr;
        std::cout << "d2h_ptr_map updated: "<<  std::endl;
        //d2h ptr icin memory allocation ypailacak
        auto* d2h_obj = new IRIS::mem_t;
        std::cout << "d2h_obj created: "<< std::endl;
        auto create_result = IRIS::iris_mem_create(numBytes, d2h_obj);
        std::cout << "iris_mem created: "<< std::endl;
        auto d2h = IRIS::iris_task_d2h(*task, *d2h_obj, 0, numBytes, d2h_ptr);
        std::cout << "d2h_ptr after: "<< d2h_ptr << std::endl;
        // d2h_ptr data structure a eklenecek.
        // ikinci asamada bu d2h_ptr memcpy e verilen destinationa verilecek (d2h icin)
        
        auto submit = IRIS::iris_task_submit(*task, IRIS::cpu, nullptr, 0);
    } 
    else {
        std::cout << "else part" << std::endl;
        auto iter = d2h_ptr_map.find(src);
        if( iter != d2h_ptr_map.end()){
            std::cout << "inside" << std::endl;
            dest = src;
        }
        else {
            std::runtime_error("Neither src nor dest exists in the pointer map.");
        }
    }
}
*/

}
CHARM_SYCL_END_NAMESPACE