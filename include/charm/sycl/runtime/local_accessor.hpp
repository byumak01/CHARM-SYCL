#pragma once
#include <charm/sycl.hpp>

CHARM_SYCL_BEGIN_NAMESPACE
namespace runtime {

struct local_accessor : refcnt_base {
    virtual ~local_accessor() = default;

    virtual size_t get_offset() const = 0;

    size_t size() const {
        return range_[0] * range_[1] * range_[2];
    }

    range<3> const& get_range() const {
        return range_;
    }

    void* get_pointer() const noexcept {
        return nullptr;
    }

protected:
    explicit local_accessor(range<3> const& range) : range_(range) {}

    range<3> range_;
};

/* local accessor iris start*/

struct local_accessor_iris : refcnt_base {
    virtual ~local_accessor_iris() = default;

    virtual size_t get_offset() const = 0;

    size_t size() const {
        return range_[0] * range_[1] * range_[2];
    }

    range<3> const& get_range() const {
        return range_;
    }

    void* get_pointer() const noexcept {
        return nullptr;
    }

protected:
    explicit local_accessor_iris(range<3> const& range) : range_(range) {}

    range<3> range_;
};

/* local accessor iris end*/

}  // namespace runtime
CHARM_SYCL_END_NAMESPACE
