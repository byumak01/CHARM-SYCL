#include <charm/sycl.hpp>
#include "rt.hpp"

CHARM_SYCL_BEGIN_NAMESPACE

namespace runtime::impl {

local_accessor_impl::local_accessor_impl(intrusive_ptr<handler_impl> const& handler, int dim,
                                         size_t elem_size, size_t align, range<3> range)
    : local_accessor(range),
      off_(handler->alloc_smem(elem_size * range[0] * range[1] * range[2], align, dim >= 1)) {}

size_t local_accessor_impl::get_offset() const {
    return off_;
}

/* local accessor iris start*/

local_accessor_iris_impl::local_accessor_iris_impl(intrusive_ptr<handler_impl> const& handler, int dim,
                                         size_t elem_size, size_t align, range<3> range)
    : local_accessor_iris(range),
      off_(handler->alloc_smem(elem_size * range[0] * range[1] * range[2], align, dim >= 1)) {}

size_t local_accessor_iris_impl::get_offset() const {
    return off_;
}

/* local accessor iris end*/

}  // namespace runtime::impl

namespace runtime {

intrusive_ptr<local_accessor> make_local_accessor(intrusive_ptr<handler> const& handler,
                                                  int dim, size_t elem_size, size_t align,
                                                  range<3> range) {
    return make_intrusive<impl::local_accessor_impl>(
        static_pointer_cast<impl::handler_impl>(handler), dim, elem_size, align, range);
}

/* local accessor iris start*/

intrusive_ptr<local_accessor_iris> make_local_accessor_iris(intrusive_ptr<handler> const& handler,
                                                  int dim, size_t elem_size, size_t align,
                                                  range<3> range) {
    return make_intrusive<impl::local_accessor_iris_impl>(
        static_pointer_cast<impl::handler_impl>(handler), dim, elem_size, align, range);
}

/* local accessor iris end*/

}  // namespace runtime

CHARM_SYCL_END_NAMESPACE
